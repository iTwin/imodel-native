/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/log4net/log4net.cs $
|
|     $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System.Collections.Specialized;
using System;
using System.IO;
using System.Diagnostics;
using System.Xml;
using log4net.Config;
using log4net;

namespace Bentley.Logging.Provider
{
/*====================================================================================**/
/// <summary>Provides a  Context for Logging through Log4net API.
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*==============+===============+===============+===============+===============+======*/
class Log4netProviderContext : ILogProviderContext
{
string      m_nameSpace;
SEVERITY    m_sev;
ILog        m_log;

/*------------------------------------------------------------------------------------**/
/// <summary>Constructor</summary>
/// <param name="nameSpace">nameSpace to use for logging</param>
/// <param name="sev">base severity level that will be used for logging to this provider</param>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public Log4netProviderContext
(
string nameSpace,
SEVERITY sev,
ILog log
)
    {
    m_nameSpace = nameSpace;
    m_sev = sev;
    m_log = log;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Retrieves the Name Space assigned to this ProviderContext</summary>
/// <returns>NameSpace</returns>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public string getNameSpace
(
)
    {
    return m_nameSpace;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Retrieves the Severity Level assigned to this ProviderContext</summary>
/// <returns>Severity </returns>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public SEVERITY getSeverity
(
)
    {
    return m_sev;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Retrieves the log 4 net logger associated with the context</summary>
/// <author>Adam.Klatzkin</author>                              <date>10/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public ILog getLog
(
)
    {
    return m_log;
    }

} //Log4netProviderContext

/*====================================================================================**/
/// <summary>The Log4netProvider allows logging by use of the Log4net logging utility </summary>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*==============+===============+===============+===============+===============+======*/
public class Log4netProvider : ILogProvider
{
private static ListDictionary _severityMap; //contains mapping of Bentley log levels to log4net log levels.

/*------------------------------------------------------------------------------------**/
/// <summary>Static Constructor used for initialization</summary>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
static Log4netProvider
(
)
    {
    _severityMap = new ListDictionary();
    _severityMap.Add(SEVERITY.FATAL,log4net.Core.Level.Fatal);
    _severityMap.Add(SEVERITY.ERROR,log4net.Core.Level.Error);
    _severityMap.Add(SEVERITY.WARNING,log4net.Core.Level.Warn);
    _severityMap.Add(SEVERITY.INFO,log4net.Core.Level.Info);
    _severityMap.Add(SEVERITY.DEBUG,log4net.Core.Level.Debug);
    _severityMap.Add(SEVERITY.TRACE,log4net.Core.Level.Trace);
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Constructor</summary>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public Log4netProvider
(
)
    {
    BasicConfigurator.Configure (); // Set up a simple configuration that logs on the console.
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Constructor</summary>
/// <param name="configFile">log4net configuration file to use with this Log4netProvider</param>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public Log4netProvider
(
string configFile
)
    {
    FileInfo fileInfo = new System.IO.FileInfo (configFile);
    if (!fileInfo.Exists)
        {
        Trace.Write ("Specified log4net configuration file does not exist: ");
        Trace.WriteLine (fileInfo.FullName);
        Trace.WriteLine ("Reading log4net configuration from resource embedded in the assembly.");

        // The logging configuration file does not exist. Use the default one embedded in the assembly.
        System.Reflection.Assembly myAssembly = System.Reflection.Assembly.GetExecutingAssembly ();
        using (System.IO.Stream stream = myAssembly.GetManifestResourceStream ("default.logging.xml"))
            {
            if (stream != null)
                {
                XmlDocument doc = new XmlDocument ();
                doc.Load (stream);
                XmlElement firstElement = (XmlElement)doc.DocumentElement.FirstChild;
                XmlConfigurator.Configure (firstElement);
                }
            else
                {
                Trace.WriteLine ("default.logging.xml resource was not found in the assembly.");
                throw new Exception ("default.logging.xml resource was not found in the assembly. This resource should be embedded in the assembly.\n");
                }
            }
        }
    else
        {
        Trace.Write ("Reading log4net configuration from file: ");
        Trace.WriteLine (fileInfo.FullName);
        XmlConfigurator.ConfigureAndWatch (fileInfo);
        }
    }


/*------------------------------------------------------------------------------------**/
/// <summary>Constructor</summary>
/// <param name="configStream">
///     The configuration data must be valid XML.
///     It must contain at least one element called log4net that holds the log4net configuration data.
///      Note that this method will NOT close the stream parameter.
/// </param>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public Log4netProvider
(
Stream configStream
)
    {
    Trace.WriteLine ("Reading log4net configuration from System.IO.Stream.");
    XmlConfigurator.Configure (configStream); //use stream containing XML configuration information
    }


/*------------------------------------------------------------------------------------**/
/// <summary>Constructor</summary>
/// <param name="configUri">log4net configuration uri to use with this Log4netProvider</param>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public Log4netProvider
(
Uri configUri
)
    {
    Trace.Write ("Reading log4net configuration from uri: ");
    Trace.WriteLine (configUri.AbsoluteUri);
    XmlConfigurator.Configure (configUri);
    }


/*------------------------------------------------------------------------------------**/
/// <summary>Constructor</summary>
/// <param name="configUri">log4net configuration uri to use with this Log4netProvider</param>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public Log4netProvider
(
System.Xml.XmlElement configElem
)
    {
    Trace.Write ("Reading log4net configuration from xml element: ");
    Trace.WriteLine (configElem.BaseURI);

    XmlConfigurator.Configure (configElem);
    }


/*------------------------------------------------------------------------------------**/
/// <summary>Creates an logger</summary>
/// <param name="nameSpace">nameSpace to use for logging</param>
/// <param name="pContext">returns a ProviderContext</param>
/// <returns>status of call</returns>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public int createLogger
(
string nameSpace,
out ILogProviderContext pContext
)
    {
    if (nameSpace == null)
        nameSpace = "";

    pContext = new Log4netProviderContext ( nameSpace, SEVERITY.TRACE, LogManager.GetLogger(nameSpace) );

    return 0;
    }


/*------------------------------------------------------------------------------------**/
/// <summary>Destroys a logger</summary>
/// <param name="pContext">ProviderContext of logger to destroy</param>
/// <returns>status of call</returns>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public int destroyLogger
(
ILogProviderContext pContext
)
    {
    return 0;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>logs a message</summary>
/// <param name="context">ProviderContext of logger</param>
/// <param name="sev">Severity level to log to.</param>
/// <param name="msg">message to log</param>
/// <returns>status of call</returns>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public int logMessage
(
ILogProviderContext context,
SEVERITY sev,
string msg
)
    {
    Log4netProviderContext log4netContext = (Log4netProviderContext)context;
    log4netContext.getLog().Logger.Log(null,(log4net.Core.Level)_severityMap[sev],msg,null);
    return 0;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Is a certain severity level active in this logger.</summary>
/// <param name="context">ProviderContext of logger</param>
/// <param name="sev">Severity level to test for.</param>
/// <returns>true or false</returns>
/// <author>Robert.Priest</author>              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public bool isSeverityEnabled
(
ILogProviderContext context,
SEVERITY sev
)
    {
    Log4netProviderContext log4netContext = (Log4netProviderContext)context;
    return (( sev >= log4netContext.getSeverity())  && log4netContext.getLog().Logger.IsEnabledFor((log4net.Core.Level)_severityMap[sev]));
    }

}//Log4netProvider
}