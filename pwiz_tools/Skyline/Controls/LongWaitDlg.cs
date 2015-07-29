﻿/*
 * Original author: Brendan MacLean <brendanx .at. u.washington.edu>,
 *                  MacCoss Lab, Department of Genome Sciences, UW
 *
 * Copyright 2009 University of Washington - Seattle, WA
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;
using System.IO;
using System.Reflection;
using System.Windows.Forms;
using pwiz.Common.SystemUtil;
using pwiz.Skyline.Model;
using pwiz.Skyline.Properties;
using pwiz.Skyline.Util;

namespace pwiz.Skyline.Controls
{
    public partial class LongWaitDlg : FormEx, ILongWaitBroker
    {
        private readonly string _cancelMessage = string.Format(" ({0})", Resources.LongWaitDlg_PerformWork_canceled); // Not L10N

        private Control _parentForm;
        private Exception _exception;
        private bool _clickedCancel;
        private int _progressValue = -1;
        private string _message;
        private int _tickCount;
        private DateTime _startTime;

        private IAsyncResult _result;

        // these members should only be accessed in a block which locks on _lock
        #region synchronized members
        private readonly object _lock = new object();
        private bool _finished;
        private bool _windowShown;
        #endregion

        /// <summary>
        /// For operations where a change in the active document should
        /// cause the operation to fail.
        /// </summary>
        private readonly IDocumentContainer _documentContainer;

        public LongWaitDlg(IDocumentContainer documentContainer = null, bool allowCancel = true)
        {
            InitializeComponent();

            Icon = Resources.Skyline;

            _documentContainer = documentContainer;

            btnCancel.Visible = btnCancel.Enabled = IsCancellable = allowCancel;

            if (!IsCancellable)
                Height -= Height - btnCancel.Bottom;
        }

        public string Message
        {
            get { return _message; }
            set { _message = value; }
        }

        public int ProgressValue
        {
            get { return _progressValue; }
            set { _progressValue = value; }
        }

        public bool IsCancellable { get; private set; }

        public bool IsDocumentChanged(SrmDocument docOrig)
        {
            return _documentContainer != null && !ReferenceEquals(docOrig, _documentContainer.Document);
        }

        public DialogResult ShowDialog(Func<IWin32Window, DialogResult> show)
        {
            // If the window handle is created, show the message in its thread,
            // parented to it.  Otherwise, use the intended parent of this form.
            var parent = (IsHandleCreated ? this : _parentForm);
            DialogResult result = DialogResult.OK;
            parent.Invoke((Action) (() => result = show(parent)));
            return result;
        }

        public void SetProgressCheckCancel(int step, int totalSteps)
        {
            if (IsCanceled)
                throw new OperationCanceledException();
            ProgressValue = 100 * step / totalSteps;
        }

        public void PerformWork(Control parent, int delayMillis, Action performWork)
        {
            var indefiniteWaitBroker = new IndefiniteWaitBroker(performWork);
            PerformWork(parent, delayMillis, indefiniteWaitBroker.PerformWork);
        }

        public ProgressStatus PerformWork(Control parent, int delayMillis, Action<IProgressMonitor> performWork)
        {
            var progressWaitBroker = new ProgressWaitBroker(performWork);
            PerformWork(parent, delayMillis, progressWaitBroker.PerformWork);
            return progressWaitBroker.Status;
        }

        public void PerformWork(Control parent, int delayMillis, Action<ILongWaitBroker> performWork)
        {
            _startTime = DateTime.Now;
            _parentForm = parent;
            try
            {
                Action<Action<ILongWaitBroker>> runner = RunWork;
                _result = runner.BeginInvoke(performWork, runner.EndInvoke, null);

                // Wait as long as the caller wants before showing the progress
                // animation to the user.
                _result.AsyncWaitHandle.WaitOne(delayMillis);

                // Return without notifying the user, if the operation completed
                // before the wait expired.
                if (_result.IsCompleted)
                    return;

                progressBar.Value = Math.Max(0, _progressValue);
                if (_message != null)
                    labelMessage.Text = _message;

                _tickCount = 0;
                ShowDialog(parent);
            }
            finally
            {
                var x = _exception;

                // Get rid of this window before leaving this function
                Dispose();

                if (IsCanceled && null != x)
                {
                    if (x is OperationCanceledException || x.InnerException is OperationCanceledException)
                    {
                        x = null;
                    }
                }

                if (x != null)
                {
                    
                    // TODO: Clean this up.  The thrown exception needs to be preserved to preserve
                    //       the original stack trace from which it was thrown.  In some cases,
                    //       its type must also be preserved, because existing code handles certain
                    //       exception types.  If this case threw only TargetInvocationException,
                    //       then more frequently the code would just have to have a blanket catch
                    //       of the base exception type, which could hide coding errors.
                    if (x is InvalidDataException)
                        throw new InvalidDataException(x.Message, x);
                    if (x is IOException)
                        throw new IOException(x.Message, x);
                    if (x is OperationCanceledException)
                        throw new OperationCanceledException(x.Message, x);
                    throw new TargetInvocationException(x.Message, x);
                }
            }
        }

        /// <summary>
        /// When this dialog is shown, it is necessary to check whether the background job has completed.
        /// If it has, then this dialog needs to be closed right now.
        /// If the background job has not yet completed, then this dialog will be closed by the finally
        /// block in <see cref="RunWork"/>.
        /// </summary>
        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            lock (_lock)
            {
                if (_finished)
                {
                    Close();
                }
                else
                {
                    _windowShown = true;
                }
            }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            lock (_lock)
            {
                _windowShown = false;
            }
            base.OnFormClosing(e);
        }


        private void RunWork(Action<ILongWaitBroker> performWork)
        {
            try
            {
                // Called in a UI thread
                LocalizationHelper.InitThread();
                performWork(this);
            }
            catch (Exception x)
            {
                _exception = x;
            }
            finally
            {
                lock (_lock)
                {
                    _finished = true;
                    if (_windowShown)
                    {
                        BeginInvoke(new Action(FinishDialog));
                    }
                }
            }
        }

        private void FinishDialog()
        {
            if (!_clickedCancel)
            {
                var runningTime = DateTime.Now.Subtract(_startTime);
                // Show complete status before returning.
                progressBar.Value = _progressValue = 100;
                labelMessage.Text = _message;
                // Display the final complete status for one second, or 10% of the time the job ran for,
                // whichever is shorter
                int finalDelayTime = Math.Min(1000, (int) (runningTime.TotalMilliseconds/10));
                if (finalDelayTime > 0)
                {
                    timerClose.Interval = finalDelayTime;
                    timerClose.Enabled = true;
                    return;
                }
            }
            Close();
        }

        public bool IsCanceled
        {
            get { return _clickedCancel; }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            labelMessage.Text += _cancelMessage;
            _clickedCancel = true;
        }

        private void timerUpdate_Tick(object sender, EventArgs e)
        {
            _tickCount++;
            if (_progressValue == -1)
            {
                progressBar.Value = (_tickCount * 10) % 110;
            }
            else
            {
                progressBar.Value = _progressValue;
            }
            if (_message != null && !Equals(_message, labelMessage.Text))
                labelMessage.Text = _message + (_clickedCancel ? _cancelMessage : string.Empty);
        }

        private void timerClose_Tick(object sender, EventArgs e)
        {
            Close();
        }

        private sealed class IndefiniteWaitBroker
        {
            private readonly Action _performWork;

            public IndefiniteWaitBroker(Action performWork)
            {
                _performWork = performWork;
            }

            public void PerformWork(ILongWaitBroker broker)
            {
                _performWork();
            }
        }

    }
}