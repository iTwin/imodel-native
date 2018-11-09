/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/interface/bsilog.cs $
|
|     $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;

namespace Bentley.Logging
{
    public enum SEVERITY
        {
        FATAL = 0,      /// Used for fatal errors that will terminate the application
        LOG_FATAL = 0,      /// Used for fatal errors that will terminate the application
        ERROR = (-1),   /// Used for general errors
        LOG_ERROR = (-1),   /// Used for general errors
        WARNING = (-2), /// Used for general warnings
        LOG_WARNING = (-2), /// Used for general warnings
        INFO = (-3),    /// Used for general informatin
        LOG_INFO = (-3),    /// Used for general informatin
        DEBUG = (-4),   /// Used for debugging information
        LOG_DEBUG = (-4),   /// Used for debugging information
        TRACE = (-5),    /// Used for function calls
        LOG_TRACE = (-5)    /// Used for function calls
        }

    public interface ILogger
    {
        /** Determine if logging severity is enabled.
            @param sev          => Message severity
            @return true or false */
        bool isSeverityEnabled ( SEVERITY sev );

        /** Log a message.
            @param sev          => Message severity
            @param msg          => Message
            @return SUCCESS or ERROR */
        int message ( SEVERITY sev, string msg );

        /** Log a message with printf style formating.
            @param sev          => Message severity
            @param msg          => Message with printf style formating
            @param ...          => printf style arguments
            @return SUCCESS or ERROR */
        int message ( SEVERITY sev, string msg, params object[] args);

        /** Log a message to the specified namespace.
            @note This method can be very expensive depending on the log provider implementation
            @param nameSpace    => namespace
            @param sev          => Message severity
            @param msg          => Message
            @return SUCCESS or ERROR */
        int message ( string nameSpace, SEVERITY sev, string msg );

        /** Log a message to the specified namespace with printf style formating.
            @note This method can be very expensive depending on the log provider implementation
            @param nameSpace    => namespace
            @param sev          => Message severity
            @param msg          => Message
            @param ...          => printf style arguments
            @return SUCCESS or ERROR */
        int message ( string nameSpace, SEVERITY sev, string msg, params object[] args );

        /** Log a message with fatal severity.
            @param msg          => Message
            @return SUCCESS or ERROR */
        int fatal ( string msg );

        /** Log a message with fatal severity with printf style formating.
            @param msg          => Message
            @param ...          => printf style arguments
            @return SUCCESS or ERROR */
        int fatal ( string msg, params object[] args );

        /** Log a message with error severity.
            @param msg          => Message
            @return SUCCESS or ERROR */
        int error ( string msg );

        /** Log a message with error severity with printf style formating.
            @param msg          => Message
            @param ...          => printf style arguments
            @return SUCCESS or ERROR */
        int error ( string msg, params object[] args );

        /** Log a message with warning severity.
            @param msg          => Message
            @return SUCCESS or ERROR */
        int warning ( string msg );

        /** Log a message with warning severity with printf style formating.
            @param msg          => Message
            @param ...          => printf style arguments
            @return SUCCESS or ERROR */
        int warning ( string msg, params object[] args );

        /** Log a message with info severity.
            @param msg          => Message
            @return SUCCESS or ERROR */
        int info ( string msg );

        /** Log a message with info severity with printf style formating.
            @param msg          => Message
            @param ...          => printf style arguments
            @return SUCCESS or ERROR */
        int info ( string msg, params object[] args );

        /** Log a message with debug severity.
            @param msg          => Message
            @return SUCCESS or ERROR */
        int debug ( string msg );

        /** Log a message with debug severity with printf style formating.
            @param msg          => Message
            @param ...          => printf style arguments
            @return SUCCESS or ERROR */
        int debug ( string msg, params object[] args );

        /** Log a message with trace severity.
            @param msg          => Message
            @return SUCCESS or ERROR */
        int trace ( string msg );

        /** Log a message with trace severity with printf style formating.
            @param msg          => Message
            @param ...          => printf style arguments
            @return SUCCESS or ERROR */
        int trace ( string msg, params object[] args );
    }

namespace Provider
{

/*----------------------------------------------------------------------------------*//**
Logger provider context interface. This class must be implemented by the any logging service
that provides logging functionality to the application
@ingroup cppinterface
*//*------------+---------------+---------------+---------------+---------------+------*/
    public interface ILogProviderContext
    {
    }

/*----------------------------------------------------------------------------------*//**
Logger provider interface. This class must be implemented by the any logging service
that provides logging functionality to the application
@ingroup cppinterface
*//*------------+---------------+---------------+---------------+---------------+------*/
    public interface ILogProvider
    {
        /** Create logger method. Called when user calls LoggerFactory::createLogger.
            @param nameSpace    => requested name for logger namespace
            @param pContext     <= Pointer to logger context value, passed to all subsequent provider interface method calls.
            @return SUCCESS or ERROR
            @see destroyLogger, LoggerFactory::createLogger */
        int createLogger ( string nameSpace, out ILogProviderContext pContext );

        /** Destroy logger method. Called when user calls LoggerFactory::destroyLogger.
            @param pContext     => Logger context value for logger to destroy.
            @return SUCCESS or ERROR
            @see createLogger, LoggerFactory::destroyLogger */
        int destroyLogger ( ILogProviderContext pContext );

        /** Log message method. Called when user calls any of the logging methods.
            @param context      => Logger context
            @param sev          => Message severity
            @param msg          => Message
            @return SUCCESS or ERROR
            @see ILogger::message */
        int logMessage ( ILogProviderContext context, SEVERITY sev, string msg );

        /** Determine if logging severity is enabled. Called when user calls ILogger::isSeverityEnabled.
            @param context      => Logger context
            @param sev          => Message severity
            @return true or false
            @see ILogger::isSeverityEnabled */
        bool isSeverityEnabled ( ILogProviderContext context, SEVERITY sev );
    };

}

/*----------------------------------------------------------------------------------*//**
*//*------------+---------------+---------------+---------------+---------------+------*/
    class LoggerConfigHolder
    {
        static Provider.ILogProvider     s_provider;

        public static Provider.ILogProvider getProvider()
        {
            return s_provider;
        }

        public static Provider.ILogProvider setProvider ( Provider.ILogProvider provider )
        {
            Provider.ILogProvider save = s_provider;
            s_provider = provider;
            return save;
        }

    }

    /*----------------------------------------------------------------------------------*//**
Logger factory. This class is create and destroy loggers.
@ingroup cppinterface
*//*------------+---------------+---------------+---------------+---------------+------*/
    public class LoggerFactory
        {
        /** Create a logger with the provided namespace.
            @param nameSpace    => Name of namespace
            @return pointer to logger interface or NULL
            @see destroyLogger */
        public static ILogger createLogger ( string nameSpace )
            {
            Provider.ILogProviderContext  context = null;

            bool noProvider = true;
            Provider.ILogProvider provider = LoggerConfigHolder.getProvider();
            if (provider != null)
                {
                provider.createLogger ( nameSpace, out context );
                noProvider = false;
                }

            BsiLogger logger = new BsiLogger ( nameSpace, context );
            if ( true == noProvider )
                {
                // This logger has no provider. Store it and set it correctly when a provider is registered.
                NoProviderLoggers.AddLogger (logger);
                }
            return logger;
            }

        /** Destroy the provided logger.
            @param pLogger      => pointer to logger interface to destroy
            @return SUCCESS or ERROR
            @see createLogger */
        public static int destroyLogger ( ILogger pLogger )
            {
            Provider.ILogProvider provider = LoggerConfigHolder.getProvider();

            BsiLogger log = (BsiLogger)pLogger;

            if ((provider != null) && (log != null))
                {
                NoProviderLoggers.RemoveLogger (log);
                return provider.destroyLogger ( log.getContext() );
                }
            else
                return 0;
            }
        };

        /// <summary>
        /// Summary description for Class1.
        /// </summary>
        class BsiLogger : ILogger
        {
        string                          m_namespace;
        Provider.ILogProviderContext    m_context;

        public BsiLogger( string nameSpace, Provider.ILogProviderContext context )
        {
            m_namespace = nameSpace;
            m_context = context;//
        }

        public Provider.ILogProviderContext getContext()
        {
            return m_context;
        }

        internal void setContext (Provider.ILogProviderContext context)
        {
            m_context = context;
        }

        internal string getNamespace ()
        {
            return m_namespace;
        }

        #region ILogger Members

        public bool isSeverityEnabled(SEVERITY sev)
        {
            Provider.ILogProvider provider = LoggerConfigHolder.getProvider();

            if (provider != null)
                return provider.isSeverityEnabled ( m_context, sev );
            else
                return false;
        }

        public int message(SEVERITY sev, string msg)
        {
            Provider.ILogProvider provider = LoggerConfigHolder.getProvider();

            if (provider == null)
                return 0;

            return provider.logMessage ( m_context, sev, msg );
        }

        public int message(SEVERITY sev, string msg, params object[] args )
        {
            Provider.ILogProvider provider = LoggerConfigHolder.getProvider();

            if (provider == null)
                return 0;

            // Check the if this message is going to output before formating the string
            if ( !provider.isSeverityEnabled ( m_context, sev ) )
                return 0;

            return provider.logMessage ( m_context, sev, String.Format ( msg, args ) );
        }

        public int message(string nameSpace, SEVERITY sev, string msg)
        {
            ILogger logger = LoggerFactory.createLogger ( nameSpace );


            int status = 0;
            if (logger != null)
                status = logger.message ( sev, msg );

            LoggerFactory.destroyLogger ( logger );

            return status;
        }

        public int message(string nameSpace, SEVERITY sev, string msg, params object[] args )
        {
            ILogger logger = LoggerFactory.createLogger ( nameSpace );

            int status = 0;

            if (logger != null)
                status = logger.message ( sev, msg, args );

            LoggerFactory.destroyLogger ( logger );

            return status;
        }

        public int fatal(string msg)
        {
            return message ( SEVERITY.FATAL, msg );
        }

        public int fatal(string msg, params object[] args )
        {
            return message ( SEVERITY.FATAL, msg, args );
        }

        public int error(string msg)
        {
            return message ( SEVERITY.ERROR, msg );
        }

        public int error(string msg, params object[] args )
        {
            return message ( SEVERITY.ERROR, msg, args );
        }

        public int warning(string msg)
        {
            return message ( SEVERITY.WARNING, msg );
        }

        public int warning(string msg, params object[] args )
        {
            return message ( SEVERITY.WARNING, msg, args );
        }

        public int info(string msg)
        {
            return message ( SEVERITY.INFO, msg );
        }

        public int info(string msg, params object[] args )
        {
            return message ( SEVERITY.INFO, msg, args );
        }

        public int debug(string msg)
        {
            return message ( SEVERITY.DEBUG, msg );
        }

        public int debug(string msg, params object[] args )
        {
            return message ( SEVERITY.DEBUG, msg, args );
        }

        public int trace(string msg)
        {
            return message ( SEVERITY.TRACE, msg );
        }

        public int trace(string msg, params object[] args)
        {
            return message ( SEVERITY.TRACE, msg, args );
        }

        #endregion
    }

/*====================================================================================**/
/// <summary>
///     This class is used to store loggers that were created without a provider.
///     When a provider is registered, all the loggers stored are then initialized
///     with this provider.</summary>
///
/// <author>Eric.Paquet</author>                                <date>06/2008</date>
/*==============+===============+===============+===============+===============+======*/
internal class NoProviderLoggers
{
private static List<BsiLogger> m_loggers    = new List<BsiLogger>();
private static object          m_lock       = new Object();

/*------------------------------------------------------------------------------------**/
/// <summary>
///     Adds a logger that was created with no provider.</summary>
/// <author>Eric.Paquet</author>                                <date>06/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
internal static void AddLogger
(
BsiLogger logger
)
    {
    lock (m_lock)
        {
        m_loggers.Add (logger);
        }
    }

/*------------------------------------------------------------------------------------**/
/// <summary>
///     Removes a logger from the list.</summary>
/// <author>Eric.Paquet</author>                                <date>06/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
internal static void RemoveLogger
(
BsiLogger logger
)
    {
    lock (m_lock)
        {
        if (m_loggers.Contains (logger))
            {
            m_loggers.Remove (logger);
            }
        }
    }

/*------------------------------------------------------------------------------------**/
/// <summary>
///     Adds a provider that was just registered. All the loggers in the list are initialized
///     using this provider.</summary>
/// <author>Eric.Paquet</author>                                <date>06/2008</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
internal static void AddProvider
(
Provider.ILogProvider   provider
)
    {
    lock (m_lock)
        {
        for (int i = m_loggers.Count - 1; i >= 0; i--)
            {
            string loggerNamespace = m_loggers[i].getNamespace();
            Provider.ILogProviderContext context = null;
            provider.createLogger (loggerNamespace, out context);
            m_loggers[i].setContext (context);

            // Remove logger from the list because it is now correctly set with a valid provider.
            m_loggers.Remove (m_loggers[i]);
            }
        }
    }
}

}
