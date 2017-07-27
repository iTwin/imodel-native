/*--------------------------------------------------------------------------------------+
|
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
|  Limited permission is hereby granted to reproduce and modify this copyrighted
|  material provided that the resulting code is used only in conjunction with
|  Bentley Systems products under the terms of the license agreement provided
|  therein, and that this notice is retained in its entirety in any such
|  reproduction or modification.
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|   Usings
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Xml;
using Bentley.Applications;
using Bentley.Automation;
using Bentley.Automation.Extensions;
using Bentley.Automation.Messaging;
using Bentley.Orchestration.MSProcessor;
using Bentley.Orchestration.Extensibility;
using BSI.Orchestration.Utility;
using BSI.Automation;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;
using Microsoft.Win32;
using System.Xml.Serialization;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
    /// <summary>
    /// Implements the smart dispatcher extension.  This class is
    /// responsible for creating and controlling the step sequence required
    /// to index widgets.
    /// </summary>
    public class SmartDispatcherExtension: ASSmartDispatcherExtension
        {
        private System.Guid m_guid;
        /// <summary>
        /// Constructor
        /// </summary>
        public SmartDispatcherExtension
        (
        )
            : base (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
              new Guid (DgnV8MirrorICSPluginConstants.DocumentProcessorGuid),
              DgnV8MirrorICSPluginConstants.DocumentProcessorDescription)
            {
            m_guid = new System.Guid();
            }
        /// <summary>
        /// Report an error *** TODO - how to report errors to the job monitor?
        /// </summary>
        /// <param name="message"></param>
        void ReportJobError (string message)
            {
            // *** TODO - how to report errors in the Job monitor?

            Log.Ref.Error (DgnV8MirrorICSPluginConstants.LoggingNamespace, message);
            }
        /// <summary>
        /// (Most of) the arguments that are to be passed to iModelBridgeFwk
        /// </summary>
        class FwkArgs
            {
            // The following are set for the whole job, once:
            bool m_createRepoIfNecessary = true;
            string m_jobWorkDir;
            string m_iModelHubProjectName;                //!< iModelHub project
            string m_iModelHubRepoName;                //!< A repository in the iModelHub project
            string m_iModelHubUserName;
            string m_iModelHubPassword;
            string m_iModelHubEnv;
            string m_revisionComment = "";

            public string GetJobWorkDir ()
                {
                return m_jobWorkDir;
                }

            /// <summary>
            /// Initialize the args
            /// </summary>
            /// <param name="asContext"></param>
            public void Initialize (ASContext asContext)
                {
                m_iModelHubProjectName = "RevitBridgeTest";       // TODO: Get the name of the iModel project from Connect
                m_iModelHubRepoName = asContext.JobDefinition.Name;  // TODO: Get name of iModel repository from Connect
                m_iModelHubUserName = "sam.wilson@bentley.com"; // TODO: Get user credentials from Connect?
                m_iModelHubPassword = "Xm2.Xk>z";               //                  "
                m_iModelHubEnv = "QA";

                // jobWorkDir is the working directory that is specific to this job.
                //  Note that specific input files are staged in a subdirectory of the job's workdir. That's why we
                //  locate the job's directory in the document's parent directory.
                var docdir = Directory.GetParent(asContext.WorkingDocumentInfo.FilePath);
                m_jobWorkDir = Directory.GetParent(docdir.FullName).FullName;
                }

            /// <summary>
            /// Get document change comments from ProjectWise
            /// </summary>
            /// <param name="asContext"></param>
            /// <returns></returns>
            string GetComments (ASContext asContext)
                {
                // *** TBD: get time as of last conversion
                DateTime startTime = DateTime.MinValue; // Will get all comments.  Set to the last modified date since the document last push to iModelHub
                string[] comments = Utility.GetDocumentComments(asContext, startTime);

                string desc = null;
                if ( 0 < comments.Length )
                    {
                    // Formating the comments as a number list.  We may not want to do this.  Makes sense the first time through with lots of comments.  Not so much later when there is only one comment.
                    for ( int i = 0; i < comments.Length; i++ )
                        comments[i] = $"{i + 1}. {comments[i]}";
                    desc = String.Join(" ", comments);
                    }

                return desc;
                }

            /// <summary>
            /// Convert most of the arguments to command-line argument format.
            /// </summary>
            /// <returns></returns>
            public string ToCmdLine ()
                {
                string cmdLine = "";
                //cmdLine += "\n--fwk-bridge-library=\"" + m_bridgeLibraryName + "\"";
                cmdLine += "\n--fwk-staging-dir=\"" + m_jobWorkDir + "\"";
                /*
                if (sourceRootGeoLocation != "")
                    {
                    string lla = sourceRootGeoLocation;
                    lla.Trim (;
                    cmdLine += "\n--fwk-input-gcs=" + lla;
                    }
                    */
                cmdLine += "\n--server-environment=" + m_iModelHubEnv;
                cmdLine += "\n--server-repository=" + m_iModelHubRepoName;
                cmdLine += "\n--server-project=" + m_iModelHubProjectName;

                if ( m_createRepoIfNecessary )
                    cmdLine += "\n--fwk-create-repository-if-necessary";

                if ( m_revisionComment != "" )
                    cmdLine += "\n--fwk-revision-comment=" + m_revisionComment;

                // NB! Don't store username or password in rsp file and don't print them to the log!

                return cmdLine;
                }

            /// <summary>
            /// Convert the user credentials to command-line argument format.
            /// </summary>
            /// <returns></returns>
            public string UserCredentialsToString ()
                {
                return "--server-user=\"" + m_iModelHubUserName + "\" --server-password=\"" + m_iModelHubPassword + "\"";
                }
            }

        /// <summary>
        /// Detect the spatial root files and the sheet files in the specified directory
        /// </summary>
        /// <param name="rootFiles">output - the full path names of the spatial root files</param>
        /// <param name="sheetFiles">ouput -- the full path names of the sheet files</param>
        /// <param name="dir">the directory to search</param>
        void DetectRootsAndSheets (List<string> rootFiles, List<string> sheetFiles, DirectoryInfo dir)
            {
            FileInfo[] files = dir.GetFiles();
            if ( null == files )
                return;

            foreach ( FileInfo fi in files )
                {
                if ( fi.Extension == ".prp" )
                    continue;
                // TODO: Detect if this is a root file by looking at a ProjectWise document property
                rootFiles.Add(fi.FullName);
                //  Assume that every document is a potential source of sheets and drawings
                sheetFiles.Add(fi.FullName);
                }
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
        /// Run iModelBridgeAssign.exe
        /// </summary>
        /// <param name="asContext"></param>
        /// <param name="args">The command-line arguments that will be passed to iModelBridgeFwk.exe and that should also be passed to the calculator</param>
        /// <param name="rspFileName">The name of the response file where most of command-line arguments are stored, ready to pass to the exe</param>
        /// <returns></returns>
        string[] ExecuteiModelBridgeAssignExe (ASContext asContext, FwkArgs args, string rspFileName)
            {
            string iModelBridgeFwkExeFileName = FindiModelBridgeFwkExe();
            if ( iModelBridgeFwkExeFileName == "" )
                {
                ReportJobError("iModelBridgeFwk.exe is not installed on this machine.");
                return null;
                }

            string iModelBridgeAssignExe = Path.Combine(Path.GetDirectoryName(iModelBridgeFwkExeFileName), "iModelBridgeAssign.exe");
            FileInfo exeFile = new FileInfo(iModelBridgeAssignExe);
            if ( !exeFile.Exists )
                {
                ReportJobError("iModelBridgeAssign.exe is not installed on this machine. It should be right next to iModelBridgeFwk.exe");
                return null;
                }

            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.CreateNoWindow = false;
            startInfo.UseShellExecute = false;
            startInfo.FileName = iModelBridgeAssignExe;
            startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            startInfo.RedirectStandardOutput = true;
            startInfo.Arguments = "@" + rspFileName + " " + args.UserCredentialsToString().ToString();

            Console.WriteLine(startInfo.FileName + " " + startInfo.Arguments);
            Log.Ref.Error(DgnV8MirrorICSPluginConstants.LoggingNamespace, startInfo.FileName + " " + startInfo.Arguments);
            using ( Process exeProcess = Process.Start(startInfo) )
                {
                exeProcess.WaitForExit();
                if ( 0 != exeProcess.ExitCode )
                    {
                    ReportJobError("Assignments failed. See " + System.Environment.GetEnvironmentVariable("LOCALAPPDATA") + "\\Bentley\\Logs\\iModelBridge.log for details");
                    return null;
                    }
                }

            return File.ReadAllLines(Path.Combine(args.GetJobWorkDir(), "bridges.txt"));
            }

        /*------------------------------------------------------------------------------------**/
        /// <author>Anthony.Falcone</author>                            <date>9/2014</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private MicroStationMessage AddAutomationServiceMDLApp
        (
        MicroStationMessage msm,
        string mdlApp
        )
            {
            string asInstallDir = "[PASInstallDir]";
            string ma = asInstallDir + mdlApp;
            string loadKeyin = "mdl load \"" + ma + "\"";
            msm.AddKeyin(loadKeyin);
            return msm;
            }


        /// <summary>
        /// Run iModelBridgeFwk, either standalone or as a keyin sent to a PowerProduct engine
        /// </summary>
        /// <param name="cmdLineArgs">The command-line arguments to be passed to iModelBridgeFwk</param>
        PowerPlatformProcessorInstruction GetPPInstructionOrExecute (string cmdLineArgs, string rootFile)
            {
            PowerPlatformProcessorInstruction mspi = new PowerPlatformProcessorInstruction();
            mspi.DocumentProcessorName = DocumentProcessorName;
            mspi.DocumentProcessorGuid = DocumentProcessorGuid.ToString();
            mspi.Step = DgnV8MirrorICSPluginConstants.Steps.PowerPlatformExample;
            MicroStationMessage msm = new MicroStationMessage();

            msm.RequiredProcessorName = "MicroStation";
            msm.RequiredProcessorVersionRange = "10.07.00.99";
            msm.DocumentName = rootFile;
            //TODO: How do I get the document Guid and datasource.
            //msm.DocumentID = asContext.WorkingDocumentInfo.DocumentGuid;
            //msm.Datasource = asContext.WorkingDocumentInfo.Datasource;
            msm.CustomDescription = String.Format("BAS keyins for {0}", DgnV8MirrorICSPluginConstants.DocumentProcessorName);
            msm = AddAutomationServiceMDLApp(msm, "AutomationService.ma");
            msm.AddKeyin(String.Format("automationservice setdocumentprocessor {0} {1}",
                        DgnV8MirrorICSPluginConstants.DocumentProcessorGuid, DgnV8MirrorICSPluginConstants.Steps.PowerPlatformExample));

            msm.AddKeyin("mdl load bimbridge");
            msm.AddKeyin("bimbridge init " + cmdLineArgs);
            msm.AddKeyin("bimbridge sync");

            mspi.MicroStationMessage = msm;
            //Execute this in BAS somewhow
            return mspi;


            }

        public class ProcessingState
            {

            public string[] m_bridges;
            public string m_respFileName;
            public string m_userCredentials;
            public int m_bridgeIndex;

            public ProcessingState ()
                {
                m_bridgeIndex = 0;
                }

            public string CurrentBridge ()
                {
                return m_bridges[m_bridgeIndex-1];
                }

            public bool Increment ()
                {
                if ( m_bridgeIndex < m_bridges.Length )
                    {
                    ++m_bridgeIndex;
                    return true;
                    }
                return false;
                }
            };
        /// <summary>
        /// Run the bridges in the framework
        /// </summary>
        /// <param name="asContext"></param>
        /// <param name="args">The command-line arguments to be passed to iModelBridgeFwk.exe</param>
        /// <returns></returns>
        ProcessingState GetBridgeAssignments (ASContext asContext, FwkArgs args)
            {
            //  Detect all root files and sheet files
            DirectoryInfo workDirInfo = new DirectoryInfo(args.GetJobWorkDir());
            System.IO.DirectoryInfo[] docDirs = workDirInfo.GetDirectories();
            List<string> rootFiles = new List<string>();
            List<string> sheetFiles = new List<string>();
            foreach ( DirectoryInfo dirInfo in docDirs )
                {
                DetectRootsAndSheets(rootFiles, sheetFiles, dirInfo);
                }

            // Store most inputs in an rsp file (in case they get large).
            // NB: Don't store username or password in rsp file and don't print them to the log!
            string rspFileName = args.GetJobWorkDir() + "\\" + "bridgefwk.rsp";
            using ( System.IO.StreamWriter file = new System.IO.StreamWriter(rspFileName) )
                {
                file.WriteLine(args.ToCmdLine());
                foreach ( string sheetFile in sheetFiles )
                    file.WriteLine("--fwk-input-sheet=" + sheetFile);
                }

            //  Make sure all documents are assigned to bridges and then read back the unique set of bridges.
            string[] bridges = ExecuteiModelBridgeAssignExe(asContext, args, rspFileName);
            if ( null == bridges )
                {
                ReportJobError("iModelBridgeAssign.exe failed.");
                return null;
                }
            if ( bridges.Length == 0 )
                {
                ReportJobError("No bridges are installed or none could be used to convert the documents selected for this job.");
                return null;
                }
            ProcessingState state = new ProcessingState();
            state.m_bridges = bridges;
            state.m_respFileName = rspFileName;
            return state;
            }


        /// <summary>
        /// Gives the smart dispatcher the sequence of steps needed to index widgets.
        /// </summary>
        /// <param name="asContext">Automation Service context object</param>
        /// <param name="previousPi">The previous step</param>
        /// <param name="isFirstRequestForProcessingInstructions">First step?</param>
        /// <returns>The next processing instruction to execute</returns>
        public override ProcessingInstruction GenerateProcessingInstructions
        (
        ASContext asContext,
        ProcessingInstruction previousPi,
        bool isFirstRequestForProcessingInstructions
        )
            {
            ProcessingState bridgeContext = null;
            XmlSerializer writer = null;
            try
                {
                writer = new XmlSerializer(typeof(ProcessingState));
                }
            catch(Exception ex)
                {
                Debug.Assert(false, ex.Message);
                }
            if ( isFirstRequestForProcessingInstructions )
                {
                FwkArgs args = new FwkArgs();
                args.Initialize(asContext);
                bridgeContext = GetBridgeAssignments(asContext, args);
                bridgeContext.m_userCredentials = args.UserCredentialsToString().ToString();

                }
            else
                {
                XmlNode state = previousPi.GetCustomData(m_guid);
                bridgeContext = writer.Deserialize(new XmlNodeReader(state)) as ProcessingState;
                }

            if ( !bridgeContext.Increment() )
                {
                return GenerateExitInstruction(previousPi);
                }

            string bridge = bridgeContext.CurrentBridge();

            string[] bridgeInfo = bridge.Split(';');

            string bridgeargs = "@" + bridgeContext.m_respFileName + " " + bridgeContext.m_userCredentials + " --fwk-bridge-regsubkey=" + bridgeInfo[0];

            bool isPowerPlatformBridge = Convert.ToInt16(bridgeInfo[2]) == 1;
            string rootFile = bridgeInfo[1];
            ProcessingInstruction instruction = null;
            if ( isPowerPlatformBridge )
                instruction = GetPPInstructionOrExecute(bridgeargs, rootFile);
            else
                {
                string bridgerunargs = bridgeargs + " --fwk-input=\"" + bridgeInfo[1] + "\"";
                instruction = GenerateDelegateProcessorInstruction(bridgerunargs);
                }
            if ( null != instruction )
                {
                StringWriter xout = new StringWriter();
                writer.Serialize(xout, bridgeContext);
                XmlDocument doc = new XmlDocument();
                doc.LoadXml(xout.ToString());
                instruction.SetCustomData(m_guid, doc.ChildNodes[1]);
                }
            return instruction;
            }


        /// <summary>
        /// Generates exit instruction
        /// </summary>
        /// <param name="previousPi">Previous processing instruction</param>
        /// <returns></returns>
        private ProcessingInstruction GenerateExitInstruction
        (
        ProcessingInstruction previousPi
        )
            {
            ExitInstruction ei = null;
            if (previousPi.ProcessingResults != null)
                {
                ei = new ExitInstruction (previousPi.ProcessingResults);
                }
            else
                {
                string message = ASConstants.StringResources.GetString ("ErrorEmptyResults");
                ei = new ExitInstruction (new ProcessingResults (false, message));
                Log.Ref.Error (DgnV8MirrorICSPluginConstants.LoggingNamespace, message);
                }
            ei.DocumentProcessorName = DocumentProcessorName;
            ei.DocumentProcessorGuid = DocumentProcessorGuid.ToString ();
            return ei;
            }


        /// <summary>
        /// Generates delegate processing instruction
        /// </summary>
        /// <returns>Delegate processing instruction</returns>
        private ProcessingInstruction GenerateDelegateProcessorInstruction
        (
            string bridgeArgs
        )
            {
            DelegateInstruction di = new DelegateInstruction ();
            di.DocumentProcessorName = DocumentProcessorName;
            di.DocumentProcessorGuid = DocumentProcessorGuid.ToString ();
            di.Step = DgnV8MirrorICSPluginConstants.Steps.DelegateExample;
            XmlDocument doc = new XmlDocument();
            XmlElement  argNode = doc.CreateElement("ArgumentData");
            XmlCDataSection  dataSection = doc.CreateCDataSection(bridgeArgs);
            argNode.AppendChild(dataSection);

            di.SetCustomData(new Guid(DgnV8MirrorICSPluginConstants.DocumentProcessorGuid), argNode);
            return di;
            }
        }
    }
