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

        //
        // Find the framework .EXE to use for the specified program association class name
        //
        string FindiModelBridgeFwkForApp (string appName)
            {
            RegistryKey bentleySoftware = Registry.LocalMachine.OpenSubKey("SOFTWARE").OpenSubKey("Bentley");
            if (null == bentleySoftware)
                return "";

            RegistryKey converters = bentleySoftware.OpenSubKey ("BimServerSyncConverters");
            if (null == converters)
                return "";

            RegistryKey associations = converters.OpenSubKey (".associations");
            if (null == associations)
                return "";

            RegistryKey association = associations.OpenSubKey (appName);
            if (null == association)
                return "";

            string[] converterNames = (string[])association.GetValue ("Converters");

            foreach (string converterName in converterNames)
                {
                RegistryKey converter = converters.OpenSubKey (converterName);
                if (null != converter)
                    {
                    string path = (string)converter.GetValue ("ProgramPath");
                    if (path != null && path != "" && System.IO.File.Exists (path))
                        return path;
                    }
                }

            return "";
            }

        //
        // Find the framework .EXE to use, based on program association data, falling back on envvars
        //
        string GetiModelBridgeFwkProgramPath (ASContext asContext)
        {
            DocumentInfo docInfo = asContext.WorkingDocumentInfo;
            Runtime runtime = new Runtime(new ASDataSource(), asContext.JobDefinitionID);
            int programAssociationActionFlags = runtime.Extractor.GetProgramAssociationActionFlags(docInfo.VaultID, docInfo.DocumentID);
            string programAssociationClassName = runtime.Extractor.GetProgramAssociationClassName(docInfo.VaultID, docInfo.DocumentID);
            //string programAssociationExecutable = runtime.Extractor.GetProgramAssociationExecutable(docInfo.VaultID, docInfo.DocumentID);

            string converter = FindiModelBridgeFwkForApp (programAssociationClassName);
            if (converter != "")
                return converter;

            converter = FindiModelBridgeFwkForApp ("."); // Look up a default/fallback converter
            if (converter != "")
                return converter;

            return System.Environment.GetEnvironmentVariable("BENTLEY_IMODELBRIDGEFWK_EXE");
        }

        //
        // Find the bridge .DLL to use, based on program association data, falling back on envvars
        //
        string GetiModelBridgeLibPath (ASContext asContext)
            {
            DocumentInfo docInfo = asContext.WorkingDocumentInfo;
            Runtime runtime = new Runtime (new ASDataSource (), asContext.JobDefinitionID);
            int programAssociationActionFlags = runtime.Extractor.GetProgramAssociationActionFlags (docInfo.VaultID, docInfo.DocumentID);
            string programAssociationClassName = runtime.Extractor.GetProgramAssociationClassName (docInfo.VaultID, docInfo.DocumentID);
            //string programAssociationExecutable = runtime.Extractor.GetProgramAssociationExecutable(docInfo.VaultID, docInfo.DocumentID);

            /* *** TBD
            string bridgeLibPath = FindbridgeLibPath (programAssociationClassName);
            if (bridgeLibPath != "")
                return bridgeLibPath;

            bridgeLibPath = FindbridgeLibPath ("."); // Look up a default/fallback bridgeLibPath
            if (bridgeLibPath != "")
                return bridgeLibPath;
                */

            return System.Environment.GetEnvironmentVariable ("BENTLEY_IMODELBRIDGE_DLL");
            }

        //
        //  Run the framework
        //
        ProcessingResults ExecuteiModelBridgeFwk (ASContext asContext)
        {
            string iModelHubProjectName = "YII";                // TODO: Get the name of the iModel project from Connect
            string iModelHubRepoName = "BimCSTest";             // TODO: Get name of iModel repository from Connect
            string iModelHubUserName = "bentley/sam.wilson";    // TODO: Get user credentials from Connect?
            string iModelHubPassword = "Xm2.Xk>z";              //                  "
            string sourceRootGeoLocation = null;                // TODO: Get lat/lng/azimuth from Connect
            bool isThisARoot = true;                            // TODO: Get "root" status of this file from PW
            bool createRepoIfNecessary = true;                  // TODO: Should we allow the user to decide on this?
            string env = "QA";   // *** NEEDS WORK: This must be "release" in a real PRG build. It could be set to "dev" in a development build.

            string iModelBridgeFwkExeFileName = GetiModelBridgeFwkProgramPath(asContext);
            if (!File.Exists(iModelBridgeFwkExeFileName))
            {
                if (null == System.Environment.GetEnvironmentVariable ("BENTLEY_IMODELBRIDGEFWK_EXE"))
                    {
                    return new ProcessingResults (false, "You forgot to define BENTLEY_IMODELBRIDGEFWK_EXE!");
                    }
                Log.Ref.Error(DgnV8MirrorICSPluginConstants.LoggingNamespace, iModelBridgeFwkExeFileName + " - not found");
                return new ProcessingResults(false, iModelBridgeFwkExeFileName + " - not found");
            }

            string iModelBridgeLibFileName = GetiModelBridgeLibPath (asContext);
            if (!File.Exists (iModelBridgeLibFileName))
                {
                if (null == System.Environment.GetEnvironmentVariable ("BENTLEY_IMODELBRIDGE_DLL"))
                    {
                    return new ProcessingResults (false, "You forgot to define BENTLEY_IMODELBRIDGE_DLL!");
                    }
                Log.Ref.Error (DgnV8MirrorICSPluginConstants.LoggingNamespace, iModelBridgeLibFileName + " - not found");
                return new ProcessingResults (false, iModelBridgeLibFileName + " - not found");
                }

            //DgnV8MirrorICSPluginConfigData data = Util.GetDgnV8MirrorICSPluginConfigData(asContext);

            // inFile is the .dgn file that we must convert
            string inFile = asContext.WorkingDocumentInfo.FilePath;

            if (!isThisARoot)
                return new ProcessingResults(true, "(ignoring non-root file)");

            // jobWorkDir is the working directory that is specific to this job. 
            //  Note that specific input files are staged in a subdirectory of the job's workdir. That's why we 
            //  locate the job's directory in the document's parent directory.
            var docdir = Directory.GetParent(asContext.WorkingDocumentInfo.FilePath);
            string jobWorkDir = Directory.GetParent(docdir.FullName).FullName;


            // Note: the directory that contains the input files is the "staging area" for this job. It
            //      is permanent - it stays around until the job definition itself is deleted by the user. I put the briefcase there
            //      so that it will persist across pulls and pushes.
            string outFile = Path.Combine(jobWorkDir, iModelHubProjectName); 
            if (!outFile.EndsWith(".bim"))
                outFile += ".bim";

            string desc = "[" + asContext.JobDefinition.Name + "]";
            //desc = asContext. ...?

            // Generate inputs to the mirror program.
            string rspFileName = jobWorkDir + "\\" + "bridgefwk.rsp";
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(rspFileName))
            {
                file.WriteLine("--fwk-bridge-library=\"" + iModelBridgeLibFileName + "\"");
                file.WriteLine("--fwk-input=\"" + inFile + "\"");
                file.WriteLine("--fwk-staging-dir=\"" + jobWorkDir + "\"");
                if (sourceRootGeoLocation != "")
                    {
                    string lla = sourceRootGeoLocation;
                    lla.Trim();
                    file.WriteLine("--fwk-input-gcs=" + lla);
                    }
                file.WriteLine("--server-environment=" + env);
                file.WriteLine("--server-repository=" + iModelHubRepoName);
                file.WriteLine("--server-project=" + iModelHubProjectName);
                if (createRepoIfNecessary)
                    file.WriteLine("--server-create-repository-if-necessary");
                
                if (desc != "")
                    file.WriteLine ("--fwk-revision-comment=" + desc);
            }

            // Don't store username or password in rsp file and don't print them to the log!
            string userAndPassword = "--server-user=" + iModelHubUserName + " --server-password=" + iModelHubPassword;

            // Launch the mirror program. It does all the work
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.CreateNoWindow = false;
            startInfo.UseShellExecute = false;
            startInfo.FileName = iModelBridgeFwkExeFileName;
            startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            startInfo.RedirectStandardOutput = true;
            startInfo.Arguments = "@" + rspFileName + " " + userAndPassword;
            Log.Ref.Info(DgnV8MirrorICSPluginConstants.LoggingNamespace, inFile + " - Executing " + startInfo.FileName + " @" + rspFileName);


            // Start the process with the info we specified.
            // Call WaitForExit and then the using-statement will close.
            string resultMessage;
            bool isFailure = false;
            using ( Process exeProcess = Process.Start(startInfo))
            {
                string lineOfOutput;
                while ((lineOfOutput = exeProcess.StandardOutput.ReadLine()) != null)
                {
                    Log.Ref.Trace (DgnV8MirrorICSPluginConstants.LoggingNamespace, lineOfOutput);
                }
                exeProcess.WaitForExit();

                if (0 == exeProcess.ExitCode)
                {
                    resultMessage = String.Format("Updated {0} from {1}", outFile, inFile);
                    Log.Ref.Trace(DgnV8MirrorICSPluginConstants.LoggingNamespace, resultMessage);
                }
                else
                {
                    resultMessage = "Converter failed. See " + System.Environment.GetEnvironmentVariable("LOCALAPPDATA") + "\\Bentley\\Logs\\ToBIMServer.log for details";
                    isFailure = true;
                    Log.Ref.Error(DgnV8MirrorICSPluginConstants.LoggingNamespace, resultMessage);
                }
            }

            return new ProcessingResults(!isFailure, resultMessage);
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
            try
                {
                ProcessingResults results = ExecuteiModelBridgeFwk(asContext);
                currentProcessingInstruction.AddProcessingResults (results);
                }
            catch (Exception ex)
                {
                currentProcessingInstruction.AddProcessingResults (new ProcessingResults (false, ex.Message));
                Log.Ref.Error (DgnV8MirrorICSPluginConstants.LoggingNamespace, ex);
                return false;
                }
            finally
                {
                asContext.ReplaceProcessingInstructions (new[] { currentProcessingInstruction });
                }

            return true;
            }
        }
    }
