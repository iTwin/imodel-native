/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/ImageViewerPackage.cs $
|    $RCSfile: ImageViewerPackage.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.ComponentModel.Design;
using Microsoft.Win32;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using EnvDTE;
using EnvDTE80;

namespace Bentley.ImageViewer
{

    [Guid("5452AFEA-3DF6-46BB-9177-C0B08F318025")]
    public interface IBeImageDebuggerService { }
    
    [PackageRegistration(UseManagedResourcesOnly = true)]
    // This attribute is used to register the information needed to show this package
    // in the Help/About dialog of Visual Studio.
    [InstalledProductRegistration("#110", "#112", "1.0", IconResourceID = 400)]
    // This attribute is needed to let the shell know that this package exposes some menus.
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [ProvideService(typeof(IBeImageDebuggerService) , ServiceName="DebuggerConnector")]
    // This attribute registers a tool window exposed by this package.
    [ProvideToolWindow(typeof(ImageDebuggerToolWindow))]
    [ProvideToolWindow(typeof(WatchWindowToolWindow))]
    [Guid("17d5d5e0-4d45-4df2-b9bf-19fb036ba99d")]
    [ProvideAutoLoad(VSConstants.UICONTEXT.Debugging_string)]
    public sealed class ImageViewerPackage : Package
    {
        /// <summary>
        /// Default constructor of the package.
        /// Inside this method you can place any initialization code that does not require 
        /// any Visual Studio service because at this point the package object is created but 
        /// not sited yet inside Visual Studio environment. The place to do all the other 
        /// initialization is the Initialize method.
        /// </summary>
        /// 
        OleMenuCommandService m_MenuService;
        DebuggerConnector m_DebuggerConnector;
        bool m_HasHexaDisplay;

        public ImageViewerPackage()
        {
            Debug.WriteLine(string.Format(CultureInfo.CurrentCulture, "Entering constructor for: {0}", this.ToString()));
        }

        public bool HasHexaDisplay
        {
            get { return m_HasHexaDisplay;}
            set { m_HasHexaDisplay = value; }
        }
        /// <summary>
        /// This function is called when the user clicks the menu item that shows the 
        /// tool window. See the Initialize method to see how the menu item is associated to 
        /// this function using the OleMenuCommandService service and the MenuCommand class.
        /// </summary>
        private void ShowToolWindow(object sender, EventArgs e)
        {
            ShowControl();
        }
        public void ChangeButtonEnabled(bool state)
        {
            m_MenuService.FindCommand(new CommandID(GuidList.guidMenuAddWatchAndVisualizeCmdSet , (int) PkgCmdIDList.addWatch)).Enabled = state;
            m_MenuService.FindCommand(new CommandID(GuidList.guidMenuAddWatchAndVisualizeCmdSet , (int) PkgCmdIDList.visualize)).Enabled = state;
        }
        private void ShowWatchToolWindow(object sender , EventArgs e)
        {
            ShowWatch();
        }
        public void ShowWatch()
        {
            ToolWindowPane window = this.FindToolWindow(typeof(WatchWindowToolWindow) , 0 , true);
            if( ( null == window ) || ( null == window.Frame ) )
            {
                throw new NotSupportedException(Resources.CanNotCreateWindow);
            }
            IVsWindowFrame windowFrame = (IVsWindowFrame) window.Frame;
            Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(windowFrame.Show());
        }
        private void AddWatch(object sender , EventArgs e)
        {
            m_DebuggerConnector.AddWatch(GetSelectionString());
        }

        private void Visualize(object sender , EventArgs e)
        {
            m_DebuggerConnector.Visualize(GetSelectionString());
        }
        private string GetSelectionString()
        {
            DTE2 dte = (DTE2) GetService(typeof(DTE));
            TextDocument document =  (TextDocument) dte.ActiveDocument.Object("TextDocument");
            string selection = document.Selection.Text;
            if( selection == "" )
            {
                TextPoint point = document.Selection.ActivePoint;
                EditPoint editPoint = point.CreateEditPoint();
                string line = editPoint.GetLines(point.Line , point.Line+1);
                int startPos = 0;
                int endPos = 0;
                for( int i = point.DisplayColumn-1 ; i < line.Length ; i++ )
                {
                    endPos = i;
                    if( !Char.IsLetterOrDigit(line[i]))
                    {
                       break;
                    }
                }
                for( int i = point.DisplayColumn-1 ; i >= 0 ; i-- )
                {
                    if( !Char.IsLetterOrDigit(line[i]) )
                    {
                        break;
                    }
                    startPos = i;
                }
                selection = line.Substring(startPos , endPos-startPos);
            }
            return selection;
        }
        public bool GetHexaDisplay()
        {
            DTE2 dte = (DTE2) GetService(typeof(DTE));
            return dte.Debugger.HexDisplayMode;
        }
        public void ShowControl()
        {
            // Get the instance number 0 of this tool window. This window is single instance so this instance
            // is actually the only one.
            // The last flag is set to true so that if the tool window does not exists it will be created.
            ToolWindowPane window = this.FindToolWindow(typeof(ImageDebuggerToolWindow) , 0 , true);
            if( ( null == window ) || ( null == window.Frame ) )
            {
                throw new NotSupportedException(Resources.CanNotCreateWindow);
            }
            IVsWindowFrame windowFrame = (IVsWindowFrame) window.Frame;
            Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(windowFrame.Show());
        }
        public VisualizerView GetControl()
        {
            ToolWindowPane window = this.FindToolWindow(typeof(ImageDebuggerToolWindow) , 0 , true);
            return (VisualizerView) window.Content;
        }


        /////////////////////////////////////////////////////////////////////////////
        // Overridden Package Implementation
        #region Package Members

        /// <summary>
        /// Initialization of the package; this method is called right after the package is sited, so this is the place
        /// where you can put all the initialization code that rely on services provided by VisualStudio.
        /// </summary>
        protected override void Initialize()
        {
            Debug.WriteLine (string.Format(CultureInfo.CurrentCulture, "Entering Initialize() of: {0}", this.ToString()));
            base.Initialize();

            IServiceContainer serviceContainer = (IServiceContainer) this;

            VisualizerModel imageDebugger = new VisualizerModel(this);
            WatchWindowModel watchWindowModel = new WatchWindowModel(imageDebugger,this);
            m_HasHexaDisplay = GetHexaDisplay();
            if( serviceContainer!= null )
            {
                m_DebuggerConnector =  new DebuggerConnector(imageDebugger ,watchWindowModel, this);
                imageDebugger.Connector = m_DebuggerConnector;
                watchWindowModel.SetConnector(m_DebuggerConnector);
                serviceContainer.AddService(typeof(IBeImageDebuggerService) , m_DebuggerConnector , true);
            }

            // Add our command handlers for menu (commands must exist in the .vsct file)
            m_MenuService = GetService(typeof(IMenuCommandService)) as OleMenuCommandService;
            if( null != m_MenuService )
            {
                // Create the command for the tool window
                CommandID toolwndCommandID = new CommandID(GuidList.guidImageViewerCmdSet, (int)PkgCmdIDList.beImageDbg);
                MenuCommand menuToolWin = new MenuCommand(ShowToolWindow, toolwndCommandID);
                m_MenuService.AddCommand(menuToolWin);
                CommandID WatchtoolwndCommandID = new CommandID(GuidList.guidImageViewerCmdSet , (int)PkgCmdIDList.beImageDbgWatch);
                MenuCommand WatchmenuToolWin = new MenuCommand(ShowWatchToolWindow , WatchtoolwndCommandID);
                m_MenuService.AddCommand(WatchmenuToolWin);
                CommandID WatchCommandID = new CommandID(GuidList.guidMenuAddWatchAndVisualizeCmdSet , (int) PkgCmdIDList.addWatch);
                MenuCommand watchCmd = new MenuCommand(AddWatch , WatchCommandID);
                m_MenuService.AddCommand(watchCmd);
                CommandID VisualizeCommandID = new CommandID(GuidList.guidMenuAddWatchAndVisualizeCmdSet , (int) PkgCmdIDList.visualize);
                MenuCommand visualizeCmd = new MenuCommand(Visualize, VisualizeCommandID);
                m_MenuService.AddCommand(visualizeCmd);
            }
        }
        #endregion


        public WatchWindow GetWatchControl()
        {
            ToolWindowPane window = this.FindToolWindow(typeof(WatchWindowToolWindow) , 0 , true);
            return (WatchWindow) window.Content;
        }

        public bool VisualizerIsOpen()
        {
            ToolWindowPane window = this.FindToolWindow(typeof(ImageDebuggerToolWindow) , 0 , true);
            return ( (VisualizerView) window.Content).IsVisible;
        }
    }
}
