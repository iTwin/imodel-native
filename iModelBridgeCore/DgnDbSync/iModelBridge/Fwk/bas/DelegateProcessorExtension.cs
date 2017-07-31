/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/DelegateProcessorExtension.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|   Usings
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.IO;
using System.Xml;
using System.Threading;
using Microsoft.Win32;

using Bentley.Automation;
using Bentley.Automation.Extensions;
using Bentley.Automation.Messaging;
using Bentley.Automation.JobConfiguration;
using BSI.Orchestration.Utility;
using BSI.Automation;

//using PwApiWrapper;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
/// <summary>
    /// Called by the delegate processor to work on DgnV8MirrorICSPlugin
    /// messages sent from the SmartDispatcher.
    /// </summary>
    public class DelegateProcessorExtension: ASDelegateProcessorExtension
        {
        /// <summary>
        /// Constructor
        /// </summary>
        public DelegateProcessorExtension
        (
        )  : base (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
                   new Guid (DgnV8MirrorICSPluginConstants.DocumentProcessorGuid),
                   DgnV8MirrorICSPluginConstants.DocumentProcessorDescription)
            {
            }

        /// <summary>
        /// Called when the processor is started
        /// </summary>
        public override void OnStart ()
            {
            }

        /// <summary>
        /// Called when the processor is shut down
        /// </summary>
        public override void OnShutdown ()
            {
            }
        /// <summary>
        /// Report an error *** TODO - how to report errors to the job monitor?
        /// </summary>
        /// <param name="message"></param>
        void ReportJobError (string message)
            {
            // *** TODO - how to report errors in the Job monitor?

            Log.Ref.Error(DgnV8MirrorICSPluginConstants.LoggingNamespace, message);
            }
        /// <summary>
        /// Find iModelBridgeFwk.exe
        /// </summary>
        /// <returns></returns>
        string FindiModelBridgeFwkExe ()
            {
            RegistryKey iModelBridgeFwk = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Bentley\iModelBridgeFwk", true);
            if ( null != iModelBridgeFwk )
                {
                string path = (string) iModelBridgeFwk.GetValue("ProgramPath");
                if ( path != null && path != "" && System.IO.File.Exists(path) )
                    return path;
                }

            return "";
            }
        /// <summary>
        /// Execute the iModelBridgeFwk.exe program in standalone mode
        /// </summary>
        /// <param name="cmdLineArgs">The command-line arguments to be passed to iModelBridgeFwk</param>
        bool ExecuteiModelBridgeFwkExe (string cmdLineArgs)
            {
            string iModelBridgeFwkExeFileName = FindiModelBridgeFwkExe();
            if ( iModelBridgeFwkExeFileName == "" )
                {
                ReportJobError("iModelBridgeFwk.exe is not installed on this machine.");
                return false;
                }
            FileInfo exeFile = new FileInfo(iModelBridgeFwkExeFileName);
            if ( !exeFile.Exists )
                {
                ReportJobError(iModelBridgeFwkExeFileName + " - file not found. The registry entry for iModelBridgeFwkExe is incorrect.");
                return false;
                }

            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.CreateNoWindow = false;
            startInfo.UseShellExecute = false;
            startInfo.FileName = iModelBridgeFwkExeFileName;
            startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            startInfo.RedirectStandardOutput = true;
            startInfo.Arguments = cmdLineArgs;

            using ( Process exeProcess = Process.Start(startInfo) )
                {
                string lineOfOutput;
                while ( (lineOfOutput = exeProcess.StandardOutput.ReadLine()) != null )
                    {
                    Log.Ref.Trace(DgnV8MirrorICSPluginConstants.LoggingNamespace, lineOfOutput);
                    }
                exeProcess.WaitForExit();

                if ( 0 != exeProcess.ExitCode )
                    {
                    ReportJobError("Bridge failed. See " + System.Environment.GetEnvironmentVariable("LOCALAPPDATA") + "\\Bentley\\Logs\\iModelBridge.log for details");
                    return false;
                    }
                }

            return true;
            }
        /// <summary>
        /// Processes DgnV8MirrorICSPlugin message anyway you see fit, just no modal dialog boxes.
        /// </summary>
        /// <param name="asContext">Automation Service context object</param>
        /// <param name="currentProcessingInstruction">Currently active processing instruction.</param>
        /// <returns>true - message sucessfully processed.</returns>
        public override bool DoMessageProcessing
        (
        ASContext asContext,
        ProcessingInstruction currentProcessingInstruction
        )
            {
            XmlNode state = currentProcessingInstruction.GetCustomData(new Guid(DgnV8MirrorICSPluginConstants.DocumentProcessorGuid));
            string arg = state.InnerText;
            bool isSuccess = ExecuteiModelBridgeFwkExe (arg);
            currentProcessingInstruction.AddProcessingResults(isSuccess, arg);
            return true;
            }
        }
    }
