/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/interface/LoggerRegistry.cs $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Reflection;
using System.Collections;


namespace Bentley.Logging
{
public static class LoggerRegistry
{
private static Hashtable m_htNamespaceToInstance = new Hashtable ();
private static Hashtable m_htAssemblyToInstance = new Hashtable ();
private static EmptyLogger m_defaultLogger = new EmptyLogger ();

/*------------------------------------------------------------------------------------**/
/// <summary>Gets or creates a Bentley.Logging.ILogger for the specified namespace</summary>
/// <remarks> checks if a logger already exists for the namespace by checking against the respective hashtable.  If so, just return it.  If not, create the logger and add it to the hashtable.
/// Also, figure out what assembly is making the request to create the logger and add that to the second hashtable..</remarks>
/// <param name="loggerNamespace">namespace</param>
/// <author>Robert.Priest</author>                              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public static ILogger CreateLogger
(
string loggerNamespace
)
    {
    ILogger logger = m_htNamespaceToInstance[loggerNamespace] as ILogger;

    if ((logger != null) && (!(logger is EmptyLogger)))
        {
        return logger;
        }

    lock ( m_htNamespaceToInstance.SyncRoot )
        {
        logger = m_htNamespaceToInstance[loggerNamespace] as ILogger;

        //couldn't find one in the Hashtable, so create one. and add it.
        if (logger == null && LoggerConfig.isInterfaceRegistered() )
            {
            logger = LoggerFactory.createLogger (loggerNamespace);
            m_htNamespaceToInstance.Add (loggerNamespace, logger);
            //also add it to the assembly hashtable
            m_htAssemblyToInstance.Add (Assembly.GetCallingAssembly().GetName(), logger);
            }
        }

    if (logger == null)
        return m_defaultLogger;

    return logger;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Returns a logger for the specified namespace</summary>
/// <remarks> if a logger for the namespace already exists, that one will be returned.
/// If not, one will be created.</remarks>
/// <param name="assembly">assembly name</param>
/// <author>Robert.Priest</author>                              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public static ILogger GetLogger
(
string assembly
)
    {
    if (m_htAssemblyToInstance.ContainsKey (assembly))
        {
        return m_htAssemblyToInstance[assembly] as ILogger;
        }

    return m_defaultLogger;
    }

//public static Hashtable NameSpaceToInstance
//    {
//    get { return m_htNamespaceToInstance; }
//    set { m_htNamespaceToInstance = value; }
//    }

//public static Hashtable AssemblyToInstance
//    {
//    get { return m_htAssemblyToInstance; }
//    set { m_htAssemblyToInstance = value; }
//    }

//public static EmptyLogger DefaultLogger
//    {
//    get { return m_defaultLogger; }
//    //set { m_defaultLogger = value; }
//    }
}
}
