/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/JobCompleteExtension.cs $
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

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
{
    /// <summary>
    /// The extension that calls iModelBridgeFwk to do the work of calling bridges, updating the briefcase, and pushing changesets to iModelHub. 
    /// </summary>
    public class JobCompleteExtension : ASJobCompleteExtension
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public JobCompleteExtension
        (
        ): base (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
            new Guid (DgnV8MirrorICSPluginConstants.DocumentProcessorGuid),
            DgnV8MirrorICSPluginConstants.DocumentProcessorDescription)
        {
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

            public string GetJobWorkDir () { return m_jobWorkDir; }

            /// <summary>
            /// Initialize the args
            /// </summary>
            /// <param name="asContext"></param>
            public void Initialize (ASContext asContext)
                {
                m_iModelHubProjectName = "iModelHubTest";       // TODO: Get the name of the iModel project from Connect
                m_iModelHubRepoName = "ToyTile7";               // TODO: Get name of iModel repository from Connect
                m_iModelHubUserName = "sam.wilson@bentley.com"; // TODO: Get user credentials from Connect?
                m_iModelHubPassword = "Xm2.Xk>z";               //                  "
                m_iModelHubEnv = "QA";

                // jobWorkDir is the working directory that is specific to this job. 
                //  Note that specific input files are staged in a subdirectory of the job's workdir. That's why we 
                //  locate the job's directory in the document's parent directory.
                var docdir = Directory.GetParent (asContext.WorkingDocumentInfo.FilePath);
                m_jobWorkDir = Directory.GetParent (docdir.FullName).FullName;
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
                string[] comments = Utility.GetDocumentComments (asContext, startTime);

                string desc = null;
                if (0 < comments.Length)
                    {
                    // Formating the comments as a number list.  We may not want to do this.  Makes sense the first time through with lots of comments.  Not so much later when there is only one comment.
                    for (int i = 0; i < comments.Length; i++)
                        comments[i] = $"{i + 1}. {comments[i]}";
                    desc = String.Join (" ", comments);
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

                if (m_createRepoIfNecessary)
                    cmdLine += "\n--fwk-create-repository-if-necessary";

                if (m_revisionComment != "")
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
            FileInfo[] files = dir.GetFiles ();
            if (null == files)
                return;

            foreach (FileInfo fi in files)
                {
                if (fi.Extension == ".prp")
                    continue;
                // TODO: Detect if this is a root file by looking at a ProjectWise document property
                rootFiles.Add (fi.FullName);
                //  Assume that every document is a potential source of sheets and drawings
                sheetFiles.Add (fi.FullName);
                }
            }

        /// <summary>
        /// Find iModelBridgeFwk.exe
        /// </summary>
        /// <returns></returns>
        string FindiModelBridgeFwkExe ()
            {
            RegistryKey iModelBridgeFwk = Registry.LocalMachine.OpenSubKey (@"SOFTWARE\Bentley\iModelBridgeFwk", true);
            if (null != iModelBridgeFwk)
                {
                string path = (string)iModelBridgeFwk.GetValue ("ProgramPath");
                if (path != null && path != "" && System.IO.File.Exists (path))
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
            string iModelBridgeFwkExeFileName = FindiModelBridgeFwkExe ();
            if (iModelBridgeFwkExeFileName == "")
                {
                ReportJobError ("iModelBridgeFwk.exe is not installed on this machine.");
                return null;
                }

            string iModelBridgeAssignExe = Path.Combine(Path.GetDirectoryName (iModelBridgeFwkExeFileName), "iModelBridgeAssign.exe");
            FileInfo exeFile = new FileInfo (iModelBridgeAssignExe);
            if (!exeFile.Exists)
                {
                ReportJobError ("iModelBridgeAssign.exe is not installed on this machine. It should be right next to iModelBridgeFwk.exe");
                return null;
                }

            ProcessStartInfo startInfo = new ProcessStartInfo ();
            startInfo.CreateNoWindow = false;
            startInfo.UseShellExecute = false;
            startInfo.FileName = iModelBridgeAssignExe;
            startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            startInfo.RedirectStandardOutput = true;
            startInfo.Arguments = "@" + rspFileName + " " + args.UserCredentialsToString ().ToString ();

            Console.WriteLine (startInfo.FileName + " " + startInfo.Arguments);
            Log.Ref.Error (DgnV8MirrorICSPluginConstants.LoggingNamespace, startInfo.FileName + " " + startInfo.Arguments);
            using (Process exeProcess = Process.Start (startInfo))
                {
                exeProcess.WaitForExit ();
                if (0 != exeProcess.ExitCode)
                    {
                    ReportJobError("Assignments failed. See " + System.Environment.GetEnvironmentVariable ("LOCALAPPDATA") + "\\Bentley\\Logs\\iModelBridge.log for details");
                    return null;
                    }
                }

            return File.ReadAllLines (Path.Combine (args.GetJobWorkDir (), "bridges.txt"));
            }

        /// <summary>
        /// Execute the iModelBridgeFwk.exe program in standalone mode
        /// </summary>
        /// <param name="cmdLineArgs">The command-line arguments to be passed to iModelBridgeFwk</param>
        void ExecuteiModelBridgeFwkExe (string cmdLineArgs)
            {
            string iModelBridgeFwkExeFileName = FindiModelBridgeFwkExe ();
            if (iModelBridgeFwkExeFileName == "")
                {
                ReportJobError ("iModelBridgeFwk.exe is not installed on this machine.");
                return;
                }
            FileInfo exeFile = new FileInfo (iModelBridgeFwkExeFileName);
            if (!exeFile.Exists)
                {
                ReportJobError (iModelBridgeFwkExeFileName + " - file not found. The registry entry for iModelBridgeFwkExe is incorrect.");
                return;
                }

            ProcessStartInfo startInfo = new ProcessStartInfo ();
            startInfo.CreateNoWindow = false;
            startInfo.UseShellExecute = false;
            startInfo.FileName = iModelBridgeFwkExeFileName;
            startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            startInfo.RedirectStandardOutput = true;
            startInfo.Arguments = cmdLineArgs;

            using (Process exeProcess = Process.Start (startInfo))
                {
                string lineOfOutput;
                while ((lineOfOutput = exeProcess.StandardOutput.ReadLine ()) != null)
                    {
                    Log.Ref.Trace (DgnV8MirrorICSPluginConstants.LoggingNamespace, lineOfOutput);
                    }
                exeProcess.WaitForExit ();

                if (0 != exeProcess.ExitCode)
                    {
                    ReportJobError ("Bridge failed. See " + System.Environment.GetEnvironmentVariable ("LOCALAPPDATA") + "\\Bentley\\Logs\\iModelBridge.log for details");
                    }
                }
            }

        /// <summary>
        /// Run iModelBridgeFwk, either standalone or as a keyin sent to a PowerProduct engine
        /// </summary>
        /// <param name="cmdLineArgs">The command-line arguments to be passed to iModelBridgeFwk</param>
        void RuniModelBridgeFwk (string cmdLineArgs)
            {
            // TODO - detect when we should run a PowerProduct + keyin
            // if (??)
            //      queue keyin request to an engine ...
            // else
            ExecuteiModelBridgeFwkExe (cmdLineArgs);
            }

        /// <summary>
        /// Run the bridges in the framework
        /// </summary>
        /// <param name="asContext"></param>
        /// <param name="args">The command-line arguments to be passed to iModelBridgeFwk.exe</param>
        /// <returns></returns>
        void RunBridgesOnFiles (ASContext asContext, FwkArgs args)
            {
            //  Detect all root files and sheet files
            DirectoryInfo workDirInfo = new DirectoryInfo (args.GetJobWorkDir ());
            System.IO.DirectoryInfo[] docDirs = workDirInfo.GetDirectories ();
            List<string> rootFiles = new List<string>();
            List<string> sheetFiles = new List<string>();
            foreach (DirectoryInfo dirInfo in docDirs)
                {
                DetectRootsAndSheets (rootFiles, sheetFiles, dirInfo);
                }

            // Store most inputs in an rsp file (in case they get large).
            // NB: Don't store username or password in rsp file and don't print them to the log!
            string rspFileName = args.GetJobWorkDir () + "\\" + "bridgefwk.rsp";
            using (System.IO.StreamWriter file = new System.IO.StreamWriter (rspFileName))
                {
                file.WriteLine (args.ToCmdLine ());
                foreach (string sheetFile in sheetFiles)
                    file.WriteLine ("--fwk-input-sheet=" + sheetFile);
                }

            //  Make sure all documents are assigned to bridges and then read back the unique set of bridges.
            string[] bridges = ExecuteiModelBridgeAssignExe (asContext, args, rspFileName);
            if (null == bridges)
                {
                ReportJobError ("iModelBridgeAssign.exe failed.");
                return;
                }
            if (bridges.Length == 0)
                {
                ReportJobError ("No bridges are installed or none could be used to convert the documents selected for this job.");
                return;
                }

            //  Run each bridge on each root file
            foreach (string bridge in bridges)
                {
                string bridgeargs = "@" + rspFileName + " " + args.UserCredentialsToString ().ToString () + " --fwk-bridge-regsubkey=" + bridge;

                foreach (string rootFile in rootFiles)
                    {
                    string bridgerunargs = bridgeargs + " --fwk-input=" + rootFile;

                    RuniModelBridgeFwk (bridgerunargs);
                    }
                }
            }

        /// <summary>
        /// Called when the job is to be completed. This is where we actually process the files.
        /// </summary>
        /// <param name="asContext">Automation Service context object</param>
        /// <returns></returns>
        public override void OnJobComplete 
        (
        ASContext asContext
        )
        {
            try
                {
                FwkArgs args = new FwkArgs ();
                args.Initialize (asContext);
                RunBridgesOnFiles (asContext, args);
                }
            catch (Exception ex)
                {
                Log.Ref.Error (DgnV8MirrorICSPluginConstants.LoggingNamespace, ex);
                ReportJobError(ex.Message);
                }
            }
        }
}