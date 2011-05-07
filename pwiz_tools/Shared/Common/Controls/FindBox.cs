﻿/*
 * Original author: Nicholas Shulman <nicksh .at. u.washington.edu>,
 *                  MacCoss Lab, Department of Genome Sciences, UW
 *
 * Copyright 2010 University of Washington - Seattle, WA
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
using System.Collections.Generic;
using System.Windows.Forms;

namespace pwiz.Common.Controls
{
    public partial class FindBox : UserControl
    {
        private Timer _timer;
        private bool _filtering;
        private DataGridView _dataGridView;

        public FindBox()
        {
            InitializeComponent();
        }

        public DataGridView DataGridView
        {
            get
            {
                return _dataGridView;
            }
            set
            {
                _dataGridView = value;
                if (_dataGridView != null)
                {
                    _dataGridView.RowsAdded += _dataGridView_RowsAdded;
                    _dataGridView.RowsRemoved += _dataGridView_RowsRemoved;
                }
            }
        }

        void _dataGridView_RowsRemoved(object sender, DataGridViewRowsRemovedEventArgs e)
        {
            StartTimer();
        }

        void _dataGridView_RowsAdded(object sender, DataGridViewRowsAddedEventArgs e)
        {
            StartTimer();
        }

        protected override void OnHandleDestroyed(EventArgs e)
        {
            base.OnHandleDestroyed(e);
            if (_timer != null)
            {
                _timer.Dispose();
                _timer = null;
            }
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            StartTimer();
        }

        private void StartTimer()
        {
            if (_filtering)
            {
                return;
            }
            if (_timer == null)
            {
                _timer = new Timer
                {
                    Interval = 2000,
                };
                _timer.Tick += _timer_Tick;
            }
            _timer.Start();
        }

        void _timer_Tick(object sender, EventArgs e)
        {
            if (_timer == null)
            {
                return;
            }
            _timer.Stop();
            var dataGridView = DataGridView;
            if (dataGridView == null)
            {
                return;
            }
            try
            {
                _filtering = true;
                var filteredRowIndexes = new List<int>();
                var text = textBox1.Text;
                var rows = new DataGridViewRow[dataGridView.Rows.Count];
                var rowsRemoved = false;
                dataGridView.Rows.CopyTo(rows, 0);
                for (int rowIndex = 0; rowIndex < rows.Length; rowIndex++)
                {
                    var row = rows[rowIndex];
                    var visible = false;
                    if (string.IsNullOrEmpty(text))
                    {
                        filteredRowIndexes.Add(rowIndex);
                        visible = true;
                    }
                    else
                    {
                        for (int iCol = 0; iCol < row.Cells.Count; iCol++)
                        {
                            var cell = row.Cells[iCol];
                            if (cell.Value == null)
                            {
                                continue;
                            }
                            var strValue = cell.Value.ToString();
                            if (strValue.IndexOf(text) >= 0)
                            {
                                filteredRowIndexes.Add(rowIndex);
                                visible = true;
                                break;
                            }
                        }
                    }
                    if (visible == row.Visible)
                    {
                        continue;
                    }
                    if (!rowsRemoved)
                    {
                        dataGridView.Rows.Clear();
                        rowsRemoved = true;
                    }
                    row.Visible = visible;
                }
                if (rowsRemoved)
                {
                    dataGridView.Rows.AddRange(rows);
                }
                FilteredRowIndexes = filteredRowIndexes.ToArray();
            }
            finally
            {
                _filtering = false;
            }
        }
        public int[] FilteredRowIndexes { get; private set; }
    }
}
