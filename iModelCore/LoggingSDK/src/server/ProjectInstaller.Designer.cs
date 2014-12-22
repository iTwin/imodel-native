/*--------------------------------------------------------------------------------------+
|
|     $Source: logging/server/ProjectInstaller.Designer.cs $
|
|  $Copyright: (c) 2008 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|   Usings
|
+--------------------------------------------------------------------------------------*/
namespace BentleyLoggingServer
{
    partial class ProjectInstaller
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// *****************************************************************************
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        ///  @author                                             AnthonyFalcone 01/08
        /// *****************************************************************************
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.ServiceProcessInstaller = new System.ServiceProcess.ServiceProcessInstaller();
            this.ServiceInstaller = new System.ServiceProcess.ServiceInstaller();
            //
            // ServiceProcessInstaller
            //
            this.ServiceProcessInstaller.Password = null;
            this.ServiceProcessInstaller.Username = null;
            //
            // ServiceInstaller
            //
            this.ServiceInstaller.Description = "Receives logging messages from multiple sources and consolidates them into its ap" +
                "penders.";
            this.ServiceInstaller.DisplayName = "Bentley Logging Server";
            this.ServiceInstaller.ServiceName = "BentleyLoggingServer";
            this.ServiceInstaller.StartType = System.ServiceProcess.ServiceStartMode.Automatic;
            this.ServiceInstaller.AfterInstall += new System.Configuration.Install.InstallEventHandler(this.serviceInstaller1_AfterInstall);
            //
            // ProjectInstaller
            //
            this.Installers.AddRange(new System.Configuration.Install.Installer[] {
            this.ServiceProcessInstaller,
            this.ServiceInstaller});

        }

        #endregion

        private System.ServiceProcess.ServiceProcessInstaller ServiceProcessInstaller;
        private System.ServiceProcess.ServiceInstaller ServiceInstaller;
    }
}