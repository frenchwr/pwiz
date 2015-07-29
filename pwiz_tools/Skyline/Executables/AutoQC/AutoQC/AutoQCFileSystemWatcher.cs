﻿/*
 * Original author: Vagisha Sharma <vsharma .at. uw.edu>,
 *                  MacCoss Lab, Department of Genome Sciences, UW
 * Copyright 2015 University of Washington - Seattle, WA
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
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;

namespace AutoQC
{

    public class AutoQCFileSystemWatcher
    {
        private readonly IAutoQCLogger Logger;

        private IResultFileStatus _fileStatusChecker;

        // Collection of new mass spec files to be processed.
        private ConcurrentQueue<string> _dataFiles;

        private readonly FileSystemWatcher _fileWatcher;

        private const int WAIT_60SEC = 60000;

        // TODO: We need to support other instrument vendors
        private const string THERMO_EXT = ".raw";

        public AutoQCFileSystemWatcher(IAutoQCLogger logger)
        {
            _fileWatcher = new FileSystemWatcher();
            _fileWatcher.Created += (s, e) => FileAdded(e);

            Logger = logger;
        }

        public void Start(MainSettings mainSettings)
        {
            _dataFiles = new ConcurrentQueue<string>();

            _fileStatusChecker = GetFileStatusChecker(mainSettings);

            _fileWatcher.EnableRaisingEvents = false;

            _fileWatcher.Filter = GetFileFilter(mainSettings.InstrumentType);

            _fileWatcher.Path = mainSettings.FolderToWatch;

            // Begin watching.
            _fileWatcher.EnableRaisingEvents = true;
        }

        private static IResultFileStatus GetFileStatusChecker(MainSettings mainSettings)
        {
            // TODO: We need to support other instrument vendors
            if (mainSettings.InstrumentType.Equals(MainSettings.THERMO))
            {
                return new XRawFileStatus(mainSettings.AcquisitionTime);
            }
            // TODO: We need to support other instrument vendors
            return new AcquisitionTimeFileStatus(mainSettings.AcquisitionTime);
        }

        private static string GetFileFilter(string instrument)
        {
            if (instrument.Equals(MainSettings.THERMO))
            {
                return "*" + THERMO_EXT;
            }
            else
            {
                // TODO: We need to support other instrument vendors
                return "*.*";
            }
        }

        public void Stop()
        {
            _fileWatcher.EnableRaisingEvents = false;
        }

        void FileAdded(FileSystemEventArgs e)
        {
            Logger.Log("File {0} added to directory.", e.Name);
            _dataFiles.Enqueue(e.FullPath);
        }

        public void WaitForFileReady(string filePath)
        {
            var counter = 0;
            while (true)
            {
                var fileStatus= _fileStatusChecker.CheckStatus(filePath);
                if (fileStatus.Equals(Status.Ready))
                {
                    break;
                }

                if (fileStatus.Equals(Status.ExceedMaximumAcquiTime))
                {
                    throw new FileStatusException("The data acquistion has exceeded the expected acquistion time." +
                                        "The instument probably encountered an error.");
                }

                if (counter % 10 == 0)
                {
                    Logger.Log("File is being acquired. Waiting...");
                }
                counter++;
                // Wait for 60 seconds.
                Thread.Sleep(WAIT_60SEC);
            }
            Logger.Log("File {0} is ready", Path.GetFileName(filePath));
        }

        public string GetFile()
        {
            if (_dataFiles.IsEmpty)
            {
                return null;
            }
                
            string filePath;
            _dataFiles.TryDequeue(out filePath);
            return filePath;
        }

        public List<string> GetAllFiles()
        {
            return Directory.GetFiles(_fileWatcher.Path, _fileWatcher.Filter).ToList();
        }

        public string GetDirectory()
        {
            return _fileWatcher.Path;
        }
    }

    public class FileStatusException : Exception
    {
        public FileStatusException(string message) : base(message)
        {
        }

        public FileStatusException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}