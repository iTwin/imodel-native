using System;
using System.IO;
using System.Windows.Input;
using Microsoft.Win32;
using System.Windows.Forms;
using System.Diagnostics;
using System.Windows.Controls;

namespace ORDBridgeGUI.ViewModel
    {
    public enum PublisherType
        {
        Local,
        IModelHub
        }

    class BridgeGUIViewModel : ViewModelBase
        {
        private PublisherType m_selectedPublisher;

        private string m_inputFile;
        private string m_dgnInstall;
        private string m_dgnWorkspace;
        private string[] m_workspaceEntries;
        private string[] m_rootModelEntries;
        private string m_selectedRootModel;
        private string m_rootModel;

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
            m_dgnInstall = LocateDgnInstall();
            if ( m_dgnInstall != null )
                m_dgnInstall = m_dgnInstall.Substring(0, m_dgnInstall.Length - 1);
            m_bridgeDll = LocateBridgeDll();

            string workspacePath = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(@"SOFTWARE\Bentley\OpenRoadsDesigner\{D11A86DD-FF26-4139-9C79-C1ABB4C8B5BF}")?.GetValue("ConfigurationPath")?.ToString();
            workspacePath = Path.Combine(workspacePath, "Workspaces");
            WorkspaceEntries = GetWorkspaces(workspacePath);
            SelectedWorkspace = WorkspaceEntries[0];


            string[] rootModels = { "Default", "Active", "Other" };
            RootModelEntries = rootModels;
            SelectedRootModel = RootModelEntries[0];

            FindExePaths();

            m_isPublisherRunning = false;
            }

        public void FindExePaths ()
            {
            var curDir = new FileInfo(System.Reflection.Assembly.GetExecutingAssembly().Location).DirectoryName;
            var curDirPublisher = String.Format("{0}\\{1}", curDir, "PublishORDToBIM.exe");
            var curDirFramework = String.Format("{0}\\{1}", curDir, "iModelBridgeFwk.exe");

            //var publisheFromRegistry = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(@"SOFTWARE\Bentley\iModelBridges\OpenRoadsDesignerBridge")?.GetValue("PublishORDToBimExe");
            //string publisherPathFromRegistry = (publisheFromRegistry != null) ? publisheFromRegistry.ToString() : "";

            //var frameworkFromRegistry = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(@"SOFTWARE\Bentley\iModelBridges\OpenRoadsDesignerBridge")?.GetValue("iModelFrameWorkExe");
            //string frameworkPathFromRegistry = (frameworkFromRegistry != null) ? frameworkFromRegistry.ToString() : "";

            var publisherPathFromRegistry = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(@"SOFTWARE\Bentley\iModelBridges\OpenRoadsDesignerBridge")?.GetValue("PublishORDToBimExe")?.ToString();
            var frameworkPathFromRegistry = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(@"SOFTWARE\Bentley\iModelBridges\OpenRoadsDesignerBridge")?.GetValue("iModelFrameWorkExe")?.ToString();

            if ( File.Exists(curDirPublisher) )
                {
                m_publisherPath = curDirPublisher;
                }
            else if ( File.Exists(publisherPathFromRegistry) )
                {
                m_publisherPath = publisherPathFromRegistry;
                }
            else
                {
                System.Windows.MessageBox.Show("Please download the Civil iModel Bridge. \"PublishORDToBim.exe\" is missing!");
                m_publisherPath = "";
                }

            if ( File.Exists(curDirFramework) )
                {
                m_frameworkPath = curDirFramework;
                }
            else if ( File.Exists(frameworkPathFromRegistry) )
                {
                m_frameworkPath = frameworkPathFromRegistry;
                }
            else
                {
                System.Windows.MessageBox.Show("Please download the Civil iModel Bridge. \"iModelBridgeFwk.exe\" is missing!");
                m_frameworkPath = "";
                }
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
            var dgnInstall = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(@"SOFTWARE\Bentley\OpenRoadsDesigner\{D11A86DD-FF26-4139-9C79-C1ABB4C8B5BF}")?.GetValue("ProgramPath");
            return (dgnInstall != null) ? dgnInstall.ToString() : "";
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
            string workspacePath = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(@"SOFTWARE\Bentley\OpenRoadsDesigner\{D11A86DD-FF26-4139-9C79-C1ABB4C8B5BF}")?.GetValue("ConfigurationPath")?.ToString();
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

        private string LocateBridgeDll ()
            {
            var curDir = new FileInfo(System.Reflection.Assembly.GetExecutingAssembly().Location).DirectoryName;
            var curDirBridgeDll = String.Format("{0}\\{1}", curDir, "ORDBridge.dll");

            if ( File.Exists(curDirBridgeDll) )
                {
                return curDirBridgeDll;
                }
            else
                {
                return RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)?.OpenSubKey(@"SOFTWARE\Bentley\iModelBridges\OpenRoadsDesignerBridge")?.GetValue("BridgeLibraryPath")?.ToString();
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

            string publisherArgs = String.Format("--input=\"{0}\" --output=\"{1}\" --DGN-Install=\"{2}\" --DGN-Workspace=\"{3}\" --root-model=\"{4}\"",
                Input, Output, m_dgnInstall, SelectedWorkspace, rootModel);

            m_stagingDir = String.Format("{0}Staging", Path.GetDirectoryName(Input));
            if ( !runPublisher )
                {
                System.IO.Directory.CreateDirectory(m_stagingDir);
                }

            string frameworkArgs = String.Format("--server-project=\"{0}\" --server-repository=\"{1}\" --server-environment=\"{2}\" --server-user=\"{3}\" --server-password=\"{4}\" " +
                                                 "--fwk-input=\"{5}\" --fwk-staging-dir=\"{6}\" --fwk-bridge-library=\"{7}\" --fwk-skip-assignment-check --DGN-Install=\"{8}\" --DGN-Workspace=\"{9}\" --root-model=\"{10}\"",
                ConnectProject, ConnectOutput, m_connectEnv, ImsUsername, pwBox.Password, Input, m_stagingDir, m_bridgeDll, m_dgnInstall, SelectedWorkspace, rootModel);

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
                    System.Windows.MessageBox.Show("INFO: The conversion completed successfully.", "Pass");
                    break;
                case 1:
                    System.Windows.MessageBox.Show("ERROR: Entered parameters are incorrect", "Failure");
                    break;
                case 2:
                    System.Windows.MessageBox.Show("ERROR: A converter error has occurred.", "Failure");
                    break;
                case 3:
                    System.Windows.MessageBox.Show("ERROR: A server error has occurred.", "Failure");
                    break;
                case 4:
                    System.Windows.MessageBox.Show("ERROR: A local error has occurred.", "Failure");
                    break;
                default:
                    System.Windows.MessageBox.Show("ERROR: An unknown error has occurred.", "Failure");
                    break;
                }
            }

        private void CheckPublisherExitCode (int exitCode)
            {
            switch ( exitCode )
                {
                case 0:
                    System.Windows.MessageBox.Show("INFO: The conversion completed successfully.", "Pass");
                    break;
                case 1:
                    System.Windows.MessageBox.Show("INFO: This file has an Affinity Level of \"None.\"", "Affinity Check Failure");
                    break;
                case 2:
                    System.Windows.MessageBox.Show("INFO: This file has an Affinity Level of \"Low.\"", "Affinity Check Failure");
                    break;
                case 3:
                    System.Windows.MessageBox.Show("INFO: This file has an Affinity Level of \"Medium.\"", "Affinity Check Failure");
                    break;
                case 4:
                    System.Windows.MessageBox.Show("INFO: This file has an Affinity Level of \"High.\"", "Affinity Check Failure");
                    break;
                case 4097:
                    System.Windows.MessageBox.Show("ERROR: Failed to parse command line arguments.", "Failure");
                    break;
                case 4098:
                    System.Windows.MessageBox.Show("ERROR: Failed to initialize the iModel Bridge.", "Failure");
                    break;
                case 32768:
                    System.Windows.MessageBox.Show("ERROR: General failure.", "Failure");
                    break;
                }
            }

        private bool Validate (String imsPw)
            {
            if ( String.IsNullOrEmpty(Input) )
                {
                System.Windows.MessageBox.Show("Input cannot be empty");
                return false;
                }

            if ( SelectedPublisher == PublisherType.Local )
                {
                if ( String.IsNullOrEmpty(Output) )
                    {
                    System.Windows.MessageBox.Show("Local Ouptut cannot be empty");
                    return false;
                    }
                }
            else
                {
                if ( String.IsNullOrEmpty(ConnectProject) )
                    {
                    System.Windows.MessageBox.Show("CONNECT Project ID cannot be empty");
                    return false;
                    }
                else if ( String.IsNullOrEmpty(ConnectOutput) )
                    {
                    System.Windows.MessageBox.Show("iModel Name cannot be empty");
                    return false;
                    }
                else if ( String.IsNullOrEmpty(ImsUsername) )
                    {
                    System.Windows.MessageBox.Show("IMS Username cannot be empty");
                    return false;
                    }
                else if ( String.IsNullOrEmpty(imsPw) )
                    {
                    System.Windows.MessageBox.Show("IMS Passwod cannot be empty");
                    return false;
                    }
                }

            if ( String.IsNullOrEmpty(RootModel) )
                {
                RootModel = ".default";
                }

            return true;
            }

        }

    }
