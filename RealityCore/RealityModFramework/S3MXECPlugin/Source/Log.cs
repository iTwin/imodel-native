/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
using Bentley.ECSystem.Logging;
using Bentley.Logging;

namespace S3MXECPlugin.Source
    {
    internal static class Log
        {
        private static ILogger s_log = LoggerManager.CreateLogger("Bentley.S3MXECPlugin");

        static internal ILogger Logger
            {
            get
                {
                return s_log;
                }
            }
        }
    }
