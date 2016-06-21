/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Log.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.ECSystem.Logging;
using Bentley.Logging;

namespace IndexECPlugin.Source
    {
    internal static class Log
        {
        private static ILogger s_log = LoggerManager.CreateLogger("Bentley.RealityDbECPlugin");

        static internal ILogger Logger
            {
            get
                {
                return s_log;
                }
            }
        }
    }
