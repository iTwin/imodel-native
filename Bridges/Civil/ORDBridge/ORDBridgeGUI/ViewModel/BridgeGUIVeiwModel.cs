/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
using System;
using System.IO;
using System.Windows.Input;
using Microsoft.Win32;
using System.Windows.Forms;
using System.Diagnostics;
using System.Windows.Controls;
using System.Collections;

namespace ORDBridgeGUI.ViewModel
    {
    public enum PublisherType
        {
        Local,
        IModelHub
        }

    class BridgeGUIViewModel : ViewModelBase
        {
        // Using for looking up Registry Values
        private const string OPENROADS  = @"SOFTWARE\Bentley\OpenRoadsDesigner\{D11A86DD-FF26-4139-9C79-C1ABB4C8B5BF}";
        private const string OPENRAIL   = @"SOFTWARE\Bentley\OpenRailDesigner\{718972C7-007F-4F72-8AD5-5B0B750E2493}";
        private const string OPENBRIDGE = @"SOFTWARE\Bentley\OpenBridgeModeler\{81969A98-EDF7-4B21-9D23-7A15482E543F}";
        private const string ORDBRIDGE  = @"SOFTWARE\Bentley\iModelBridges\OpenRoadsDesigner";

        private PublisherType m_selectedPublisher;

        private string m_inputFile;
        private string m_dgnInstall;
        private string m_dgnWorkspace;
        private string[] m_workspaceEntries;
        private string[] m_rootModelEntries;
        private string m_selectedRootModel;
        private string m_rootModel;

        private bool m_isDgnInstalled;
        private string m_openRoadsInstall;
        private string m_openRailInstall;
        private string m_openBridgeInstall;
        private string[] m_dgnInstallEntries;
        private string m_selectedDgnInstall;

        private string m_defaultRoadsWorkspacePath;
        private string m_defaultRailWorkspacePath;
        private string m_defaultBridgeWorkspacePath;

        // publisher
        private string m_output;

        // framework
        private string m_connectProj;
        private string m_connectOutput;
        private string m_connectEnv;
        private string m_imsUsername;
        private string m_stagingDir;
        private string m_bridgeDll;

        private string m_publisherPath;
        private string m_frameworkPath;

        private bool m_isPublisherRunning;

        #region Mutators
        public PublisherType SelectedPublisher
            {
            get
                {
                return m_selectedPublisher;
                }
            set
                {
                m_selectedPublisher = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(SelectedPublisher));
                }
            }

        public string Input
            {
            get { return m_inputFile; }
            set
                {
                m_inputFile = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(Input));
                }
            }

        public string Output
            {
            get { return m_output; }
            set
                {
                m_output = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(Output));
                }
            }
        public string SelectedWorkspace
            {
            get { return m_dgnWorkspace; }
            set
                {
                m_dgnWorkspace = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(SelectedWorkspace));
                }
            }

        public string[] WorkspaceEntries
            {
            get { return m_workspaceEntries; }
            set
                {
                m_workspaceEntries = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(WorkspaceEntries));
                }
            }

        public string[] RootModelEntries
            {
            get
                {
                return m_rootModelEntries;
                }
            set
                {
                m_rootModelEntries = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(RootModelEntries));
                }
            }

        public string SelectedRootModel
            {
            get
                {
                return m_selectedRootModel;
                }
            set
                {
                m_selectedRootModel = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(SelectedRootModel));
                }
            }

        public string RootModel
            {
            get { return m_rootModel; }
            set
                {
                m_rootModel = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(RootModel));
                }
            }

        public string ConnectProject
            {
            get { return m_connectProj; }
            set
                {
                m_connectProj = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(ConnectProject));
                }
            }
        public string ConnectOutput
            {
            get { return m_connectOutput; }
            set
                {
                m_connectOutput = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(ConnectOutput));
                }
            }

        public string ImsUsername
            {
            get { return m_imsUsername; }
            set
                {
                m_imsUsername = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(ImsUsername));
                }
            }

        public bool IsPowerPlatformProductSelected
            {
            get
                {
                return m_isDgnInstalled;
                }
            set
                {
                m_isDgnInstalled = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(IsDgnInstalled));
                }
            }

        public string[] DgnInstallEntries
            {
            get
                {
                return m_dgnInstallEntries;
                }
            set
                {
                m_dgnInstallEntries = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(DgnInstallEntries));
                }
            }

        public string SelectedDGNInstall
            {
            get
                {
                return m_selectedDgnInstall;
                }
            set
                {
                m_selectedDgnInstall = value;
                RaisePropertyChangedEvent();
                //RaisePropertyChangedEvent(nameof(SelectedDGNInstall));

                GetSelectedPowerProductWorkspaces();
                IsPowerPlatformProductSelected = !SelectedDGNInstall.Equals("None");
                }
            }
        #endregion

        public BridgeGUIViewModel ()
            {
            Input = "";
            Output = "";
            ImsUsername = "";
            ConnectProject = "";
            ConnectOutput = "";

#if DEVELOPER
            m_connectEnv = "QA";
#else
            m_connectEnv = "RELEASE";
#endif
            FindORDBSpecificPaths();
            DgnInstallEntries = GetDgnInstalls();
            SelectedDGNInstall = DgnInstallEntries[0];

            string[] rootModels = { "Default", "Active", "Other" };
            RootModelEntries = rootModels;
            SelectedRootModel = RootModelEntries[0];

            m_isPublisherRunning = false;
            }

        public void FindORDBSpecificPaths ()
            {
            var curDir = new FileInfo(System.Reflection.Assembly.GetExecutingAssembly().Location).DirectoryName;
            var curDirPublisher = String.Format("{0}\\{1}", curDir, "PublishORDToBIM.exe");
            var curDirFramework = String.Format("{0}\\{1}", curDir, "iModelBridgeFwk.exe");
            var curDirBridgeDll = String.Format("{0}\\{1}", curDir, "ORDBridge.dll");

            var publisherPathFromRegistry = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(ORDBRIDGE)?.GetValue("PublishORDToBimExe")?.ToString();
            var frameworkPathFromRegistry = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(ORDBRIDGE)?.GetValue("iModelFrameWorkExe")?.ToString();
            var ordbDLLPathFromRegistry   = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(ORDBRIDGE)?.GetValue("BridgeLibraryPath")?.ToString();

            m_publisherPath = File.Exists(curDirPublisher) ? curDirPublisher : publisherPathFromRegistry;
            m_frameworkPath = File.Exists(curDirFramework) ? curDirFramework : frameworkPathFromRegistry;
            m_bridgeDll     = File.Exists(curDirBridgeDll) ? curDirBridgeDll : ordbDLLPathFromRegistry;

            if ( m_publisherPath != null && m_frameworkPath != null && m_bridgeDll != null )
                return;

            var errorMsg = "Please download the Civil iModel Bridge...";

            if ( m_publisherPath == null )
                errorMsg += "\n - \"PublishORDToBim.exe\" is missing!";
            if ( m_frameworkPath == null )
                errorMsg += "\n - \"iModelBridgeFwk.exe\" is missing!";
            if ( m_bridgeDll == null )
                errorMsg += "\n - \"ORDBridge.dll\" is missing!";

            MessageBox.Show(errorMsg, "Civil iModel Bridge", MessageBoxButtons.OK, MessageBoxIcon.Error);
            Environment.Exit(-1);
            }

        public ICommand SelectFileCommand
            {
            get
                {
                return new RelayCommand((obj) => DoSelectFileCommand());
                }
            }

        private void DoSelectFileCommand ()
            {
            Microsoft.Win32.OpenFileDialog openFileDialog = new Microsoft.Win32.OpenFileDialog
                {
                Filter = "DGN FIle (*.dgn)|*.dgn",
                };
            if ( openFileDialog.ShowDialog() == true )
                {
                Input = openFileDialog.FileName;
                Output = String.Format("{0}\\{1}.ibim", Path.GetDirectoryName(Input), Path.GetFileNameWithoutExtension(Input));
                ConnectOutput = String.Format("{0}", Path.GetFileNameWithoutExtension(Input));
                }
            }

        public ICommand SelectOutDirCommand
            {
            get
                {
                return new RelayCommand((obj) => DoSelectOutDirCommand());
                }
            }

        private void DoSelectOutDirCommand ()
            {
            if ( String.IsNullOrEmpty(Input) )
                {
                System.Windows.MessageBox.Show("Input cannot be empty");
                return;
                }
            var saveFileDialog = new System.Windows.Forms.SaveFileDialog
                {
                InitialDirectory = Path.GetDirectoryName(Input),
                Title = "Save iModel",
                CheckFileExists = false,
                CheckPathExists = true,
                DefaultExt = "ibim",
                Filter = "iModel (*.ibim)|*.ibim",
                RestoreDirectory = true
                };
            if ( saveFileDialog.ShowDialog() == DialogResult.OK )
                {
                Output = saveFileDialog.FileName;
                }
            }

        private string LocateDgnInstall ()
            {
            return RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(OPENROADS)?.GetValue("ProgramPath")?.ToString();
            }

        private void GetSelectedPowerProductWorkspaces ()
            {
            if ( SelectedDGNInstall.Equals("None") )
                {
                m_dgnInstall = null;
                WorkspaceEntries = new string[0];
                SelectedWorkspace = "";
                return;
                }
            else if ( SelectedDGNInstall.Equals("OpenRoads Designer") )
                m_dgnInstall = m_defaultRoadsWorkspacePath;
            else if ( SelectedDGNInstall.Equals("OpenRail Designer") )
                m_dgnInstall = m_defaultRailWorkspacePath;
            else if ( SelectedDGNInstall.Equals("OpenBridge Modeler") )
                m_dgnInstall = m_defaultBridgeWorkspacePath;

            WorkspaceEntries = GetWorkspaces(m_dgnInstall);
            SelectedWorkspace = WorkspaceEntries[0];
            }

        private string[] GetWorkspaces (String dir)
            {
            m_workspaceEntries = Directory.GetFiles(dir, "*.cfg");

            if ( m_workspaceEntries.Length == 0 )
                {
                System.Windows.MessageBox.Show("This directory does not contain any workspaces. Please select another directory.");
                DoSelectDgnWorkspaceCommand();
                }

            for ( int i = 0; i < m_workspaceEntries.Length; i++ )
                {
                m_workspaceEntries[i] = Path.GetFileNameWithoutExtension(m_workspaceEntries[i]);
                }

            return m_workspaceEntries;
            }


        public ICommand SelectDgnWorkspaceCommand
            {
            get
                {
                return new RelayCommand((obj) => DoSelectDgnWorkspaceCommand());
                }
            }

        private void DoSelectDgnWorkspaceCommand ()
            {
            string workspacePath = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(OPENROADS)?.GetValue("ConfigurationPath")?.ToString();
            if ( workspacePath == null)
                {
                System.Windows.MessageBox.Show("Please install Bentley OpenRoads Designer, it is missing!");
                m_frameworkPath = "";
                }
            workspacePath = Path.Combine(workspacePath, "Workspaces");

            var folderDialog = new FolderBrowserDialog
                {
                Description = "Select the root directory that you want to use as the default for Workspaces.",
                SelectedPath = workspacePath  // Set initial folder to be Workspaces dir found in ORD
                };
            if ( folderDialog.ShowDialog() == DialogResult.OK )
                {
                workspacePath = folderDialog.SelectedPath;
                WorkspaceEntries = GetWorkspaces(workspacePath);
                SelectedWorkspace = WorkspaceEntries[0];
                }
            }

        public ICommand RunBridgeCommand
            {
            get {
                return new RelayCommand((pwdBox) => DoBridgeCommand((PasswordBox) pwdBox), (obj) => !m_isPublisherRunning);
                }
            }

        private void DoBridgeCommand (PasswordBox pwBox)
            {
            bool shouldRun = Validate(pwBox.Password);
            if ( !shouldRun )
                return;

            bool runPublisher = SelectedPublisher == PublisherType.Local;

            string rootModel;
            if ( SelectedRootModel == "Default" )
                {
                rootModel = ".default";
                }
            else if ( SelectedRootModel == "Active" )
                {
                rootModel = ".active";
                }
            else
                {
                rootModel = RootModel;
                }

            string publisherArgs = String.Format("--input=\"{0}\" --output=\"{1}\" --root-model=\"{2}\"", Input, Output, rootModel);

            m_stagingDir = String.Format("{0}\\Staging", Path.GetDirectoryName(Input));
            if ( !runPublisher )
                {
                System.IO.Directory.CreateDirectory(m_stagingDir);
                }

            string frameworkArgs = String.Format("--server-project=\"{0}\" --server-repository=\"{1}\" --server-environment=\"{2}\" --server-user=\"{3}\" --server-password=\"{4}\" " +
                                                 "--fwk-input=\"{5}\" --fwk-staging-dir=\"{6}\" --fwk-bridge-library=\"{7}\" --fwk-skip-assignment-check --root-model=\"{8}\"",
                                                ConnectProject, ConnectOutput, m_connectEnv, ImsUsername, pwBox.Password, Input, m_stagingDir, m_bridgeDll, rootModel);

            if ( IsPowerPlatformProductSelected )
                {
                String optionWorkspaceArgs = String.Format(" --DGN-Install=\"{0}\" --DGN-Workspace=\"{1}\" ", m_dgnInstall, SelectedWorkspace);
                publisherArgs += optionWorkspaceArgs;
                frameworkArgs += optionWorkspaceArgs;
                }

            string exe = runPublisher ? m_publisherPath : m_frameworkPath;
            string args = runPublisher ? publisherArgs : frameworkArgs;

            var proc = new Process
                {
                StartInfo = new ProcessStartInfo
                    {
                    FileName = exe,
                    Arguments = args,
                    UseShellExecute = true,
                    RedirectStandardOutput = false,
                    RedirectStandardError = false,
                    CreateNoWindow = false
                    }
                };

            Console.WriteLine(proc.StartInfo.FileName + "\t" + proc.StartInfo.Arguments);
            proc.Start();
            m_isPublisherRunning = true;
            proc.WaitForExit();

            if ( SelectedPublisher == PublisherType.Local )
                {
                CheckPublisherExitCode(proc.ExitCode);
                }
            else
                {
                CheckFrameworkExitCode(proc.ExitCode);
                }

            m_isPublisherRunning = false;
            }

        private void CheckFrameworkExitCode (int exitCode)
            {
            switch ( exitCode )
                {
                case 0:
                    MessageBox.Show("The conversion completed successfully.", "Civil iModel Bridge - \"PublishORDToBIM.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    break;
                case 1:
                    MessageBox.Show("Entered parameters are incorrect", "Civil iModel Bridge - \"PublishORDToBIM.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                case 2:
                    MessageBox.Show("A converter error has occurred.", "Civil iModel Bridge - \"PublishORDToBIM.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                case 3:
                    MessageBox.Show("A server error has occurred.", "Civil iModel Bridge - \"PublishORDToBIM.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                case 4:
                    MessageBox.Show("A local error has occurred.", "Civil iModel Bridge - \"PublishORDToBIM.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                default:
                    MessageBox.Show("An unknown error has occurred.", "Civil iModel Bridge - \"PublishORDToBIM.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                }
            }

        private void CheckPublisherExitCode (int exitCode)
            {
            switch ( exitCode )
                {
                case 0:
                    MessageBox.Show("The conversion completed successfully.", "Civil iModel Bridge - \"iModelBridgeFwk.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    break;
                case 1:
                    MessageBox.Show("This file has an Affinity Level of \"None\".", "Civil iModel Bridge - \"iModelBridgeFwk.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    break;
                case 2:
                    MessageBox.Show("This file has an Affinity Level of \"Low\".", "Civil iModel Bridge - \"iModelBridgeFwk.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    break;
                case 3: // Not sure how this will be possible. Bridge will either determine if model has High affinity or none/low
                    MessageBox.Show("This file has an Affinity Level of \"Medium\".", "Civil iModel Bridge - \"iModelBridgeFwk.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    break;
                case 4: // I wouldn't think this would be an error but its one of the erro codes listed in DgnDbSync platform bridge code. Eitherway should never get hit
                    MessageBox.Show("This file has an Affinity Level of \"High\".", "Civil iModel Bridge - \"iModelBridgeFwk.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    break;
                case 4097:
                    MessageBox.Show("Failed to parse arguments.", "Civil iModel Bridge - \"iModelBridgeFwk.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                case 4098:
                    MessageBox.Show("Failed to initialize the iModel Bridge.", "Civil iModel Bridge - \"iModelBridgeFwk.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                case 32768:
                    MessageBox.Show("General failure.", "Civil iModel Bridge - \"iModelBridgeFwk.exe\"", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                }
            }

        private bool Validate (String imsPw)
            {
            if ( String.IsNullOrEmpty(Input) )
                {
                MessageBox.Show("Input cannot be empty", "Civil iModel Bridge", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
                }

            if ( SelectedPublisher == PublisherType.Local )
                {
                if ( String.IsNullOrEmpty(Output) )
                    {
                    MessageBox.Show("Local Ouptut cannot be empty", "Civil iModel Bridge", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                    }
                }
            else
                {
                if ( String.IsNullOrEmpty(ConnectProject) )
                    {
                    MessageBox.Show("CONNECT Project ID cannot be empty", "Civil iModel Bridge", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                    }
                else if ( String.IsNullOrEmpty(ConnectOutput) )
                    {
                    MessageBox.Show("iModel Name cannot be empty", "Civil iModel Bridge", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                    }
                else if ( String.IsNullOrEmpty(ImsUsername) )
                    {
                    MessageBox.Show("IMS Username cannot be empty", "Civil iModel Bridge", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                    }
                else if ( String.IsNullOrEmpty(imsPw) )
                    {
                    MessageBox.Show("IMS Passwod cannot be empty", "Civil iModel Bridge", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                    }
                }

            if ( String.IsNullOrEmpty(RootModel) )
                {
                RootModel = ".default";
                }

            return true;
            }

        private string[] GetDgnInstalls ()
            {
            m_openRoadsInstall = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(OPENROADS)?.GetValue("ProgramPath")?.ToString();
            m_openRailInstall = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(OPENRAIL)?.GetValue("ProgramPath")?.ToString();
            //m_openBridgeInstall = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(OPENBRIDGE)?.GetValue("ProgramPath")?.ToString();

            ArrayList dgnInstallList = new ArrayList {"None"};

            if ( m_openRoadsInstall != null )
                {
                m_openRoadsInstall = m_openRoadsInstall.Substring(0, m_openRoadsInstall.Length - 1);
                dgnInstallList.Add("OpenRoads Designer");
                string workspacePath = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(OPENROADS)?.GetValue("ConfigurationPath")?.ToString();
                m_defaultRoadsWorkspacePath = Path.Combine(workspacePath, "Workspaces");
                }

            if ( m_openRailInstall != null )
                {
                m_openRailInstall = m_openRailInstall.Substring(0, m_openRailInstall.Length - 1);
                dgnInstallList.Add("OpenRail Designer");
                string workspacePath = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(OPENRAIL)?.GetValue("ConfigurationPath")?.ToString();
                m_defaultRailWorkspacePath = Path.Combine(workspacePath, "Workspaces");
                }

            //if ( m_openBridgeInstall != null )
            //    {
            //    m_openBridgeInstall = m_openBridgeInstall.Substring(0, m_openBridgeInstall.Length - 1);
            //    dgnInstallList.Add("OpenBridge Modeler");
            //    string workspacePath = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(OPENBRIDGE)?.GetValue("ConfigurationPath")?.ToString();
            //    m_defaultBridgeWorkspacePath = Path.Combine(workspacePath, "Workspaces");
            //    }

            return dgnInstallList.ToArray(typeof(string)) as string[];
            }

        }

    }
