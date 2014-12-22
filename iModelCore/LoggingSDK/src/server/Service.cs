/*--------------------------------------------------------------------------------------+
|
|     $Source: logging/server/Service.cs $
|
|  $Copyright: (c) 2008 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|   Usings
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.ServiceProcess;
using System.Text;
using Bentley.Logging.Provider;
using Bentley.Logging;
using System.Runtime.Remoting;

namespace BentleyLoggingServer
{
    public partial class Service : ServiceBase
    {
        private Bentley.Logging.ILogger m_Logger = null;
        private string m_configFile = null;


        /// *****************************************************************************
        /// <summary>
        /// Gets the configuration file
        /// </summary>
        /// <returns>configuration file</returns>
        ///  @author                                             AnthonyFalcone 08/08
        /// *****************************************************************************
        string GetConfigFile()
        {
            if (null != m_configFile)
                return m_configFile;

            m_configFile = AppDomain.CurrentDomain.SetupInformation.ConfigurationFile;

            if (!System.IO.File.Exists(m_configFile))
            {
                m_configFile = System.IO.Path.GetDirectoryName(m_configFile) + @"\BentleyLoggingServer.exe.config";
            }
            return m_configFile;
        }

        /// *****************************************************************************
        /// <summary>
        /// Register the log provider
        /// </summary>
        ///  @author                                             AnthonyFalcone 01/08
        /// *****************************************************************************
        void RegisterProvider()
        {
            Log4netProvider pProvider = new Log4netProvider(GetConfigFile());
            LoggerConfig.registerLogInterface(pProvider);
        }

        /// *****************************************************************************
        /// <summary>
        /// Gets a logger
        /// </summary>
        ///  @author                                             AnthonyFalcone 01/08
        /// *****************************************************************************
        private Bentley.Logging.ILogger Log
        {
            get
            {
                if (null == m_Logger)
                {
                    RegisterProvider();
                    m_Logger = LoggerFactory.createLogger("Bentley.Logging.Service");
                }
                return m_Logger;
            }
        }

        /// *****************************************************************************
        /// <summary>
        /// Converts an exception to a message suitable for logging.
        /// </summary>
        /// <param name="ex"></param>
        /// <returns>message suitable for logging</returns>
        /// @author                                             AnthonyFalcone 06/07
        /// *****************************************************************************
        private static string ConvertExceptionToMessage(Exception ex)
        {
            return "The following exception was thrown:\n" + ex.Message + "\nStack Trace:\n" + ex.StackTrace;
        }

        /// *****************************************************************************
        /// <summary>
        /// Constructor
        /// </summary>
        ///  @author                                             AnthonyFalcone 01/08
        /// *****************************************************************************
        public Service()
        {
            InitializeComponent();
        }

        /// *****************************************************************************
        /// <summary>
        /// Called when service starts
        /// </summary>
        /// <param name="args">arguments</param>
        ///  @author                                             AnthonyFalcone 01/08
        /// *****************************************************************************
        protected override void OnStart(string[] args)
        {
            Log.trace("OnStart called. Bentley Logging Service is responding to an OnStart event from the Service Control Mangager.");

            try
            {
                // Configure remoting. This loads the TCP channel as specified in the .config file.
                RemotingConfiguration.Configure(GetConfigFile(), false);

                // Publish the remote logging server. This is done using the log4net plugin.
                log4net.LogManager.GetRepository().PluginMap.Add(new log4net.Plugin.RemoteLoggingServerPlugin("LoggingSink"));
            }
            catch (Exception ex)
            {
                Log.error("OnStart - An exception occured while configuring remoting\n" + ConvertExceptionToMessage(ex));
            }
        }

        /// *****************************************************************************
        /// <summary>
        /// Called when service stops
        /// </summary>
        /// <param name="args">arguments</param>
        ///  @author                                             AnthonyFalcone 01/08
        /// *****************************************************************************
        protected override void OnStop()
        {
            Log.trace("OnStop called. Bentley Logging Service is responding to an OnStop event from the Service Control Mangager.");
        }
    }
}
