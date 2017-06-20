/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/Bootstrapper/Program.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using Bentley.Automation;
using Bentley.Automation.Configuration;
using Microsoft.Win32;
using System.Windows.Forms; 

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS.BootStrapper
    {


    class Program
        {
        static void Main (string[] args)
            {
            AppDomain.CurrentDomain.AssemblyResolve += new ResolveEventHandler (MyResolveEventHandler);

            var uninstall = (from a in args
                             where a.StartsWith ("/uninstall")
                             select a).FirstOrDefault ();

        //    MessageBox.Show ("Bentley.Map.Mobile.ICS.BootStrapper " + null != uninstall ? "Uninstall" : "Install");
            string pluginAssm = AppDomain.CurrentDomain.BaseDirectory + @"\DgnV8MirrorICSPlugin.dll";


            ConfigurePlugin (pluginAssm, uninstall != null);
            }


        private static void ConfigurePlugin
        (
        string pluginAssm,
        bool uninstall
        )
            {
            if (uninstall)
                {
                ASConfig.LaunchConfigWizardForUninstall (pluginAssm);
                ASConfig.RemovePlugin (pluginAssm);
                }
            else
                {
                ASConfig.AddPlugin (pluginAssm);
                ASConfig.LaunchConfigWizard ();
                }
            }

        /*------------------------------------------------------------------------------------**/
        /// <author>Anthony.Falcone</author>                               <date>09/2012</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private static Assembly MyResolveEventHandler
        (
        object sender,
        ResolveEventArgs args
        )
            {
            //This handler is called only when the common language runtime tries to bind to the assembly and fails.
            string dllNameToResolve = args.Name;
            int commaIdx = args.Name.IndexOf (",");
            if (-1 != commaIdx)
                dllNameToResolve = (args.Name.Substring (0, commaIdx) + ".dll");
            dllNameToResolve = dllNameToResolve.ToLower ();

            var supportingPaths = new[] { ASServerInstallDir, OFServerInstallDir };

            string assmPath =
                (from p in supportingPaths
                 where File.Exists (p + dllNameToResolve)
                 select p + dllNameToResolve).
                    FirstOrDefault ();
            if (null != assmPath)
                return Assembly.LoadFrom (assmPath);
            return null;
            }

        private const string ASServerRegistryKeyName = "SOFTWARE\\Bentley\\AutomationServices\\Server";
        private const string OFServerRegistryKeyName = "SOFTWARE\\Bentley\\Orchestration Framework\\Server";

        //-------------------------------------------------------------------------------
        /// <summary>
        /// Returns the Automation server installation directory.
        /// </summary>
        //-------------------------------------------------------------------------------
        private static string ASServerInstallDir
            {
            get
                {
                return (string)ASServerRegistryKey.GetValue ("InstallDir");
                }
            }
        private static RegistryKey ASServerRegKey = null;
        /// <summary>The registry key used to store Automation Service server settings</summary>
        private static RegistryKey ASServerRegistryKey
            {
            get
                {
                if (ASServerRegKey != null)
                    return ASServerRegKey;

                ASServerRegKey = OpenRegKey (ASServerRegistryKeyName);

                return ASServerRegKey;
                }
            }

        //-------------------------------------------------------------------------------
        /// <summary>
        /// Returns the Orchestration installation directory.
        /// </summary>
        //-------------------------------------------------------------------------------
        private static string OFServerInstallDir
            {
            get
                {
                return (string)OFServerRegistryKey.GetValue ("InstallDir");
                }
            }
        private static RegistryKey OFServerRegKey = null;
        private static RegistryKey OFServerRegistryKey
            {
            get
                {
                if (OFServerRegKey != null)
                    return OFServerRegKey;

                OFServerRegKey = OpenRegKey (OFServerRegistryKeyName);

                return OFServerRegKey;
                }
            }

        public static RegistryKey OpenRegKey (string regKeyName)
            {
            RegistryKey regLocalMachine = Registry.LocalMachine;
            RegistryKey regKey = regLocalMachine.OpenSubKey (regKeyName);
            return regKey;
            }
        }
    }
