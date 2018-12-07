/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/interface/EmptyLogger.cs $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;

namespace Bentley.Logging
{
/*====================================================================================**/
///
/// <summary>Empty Logger to be returned if LoggerManager cannot find one</summary>
/// <remarks>Empty Loggers are usually the result of a configuration problem.
/// To configure this, create tags for a LoggerProvider in the procedures config file,
/// and include the following properties.
/// Note: change paths where neccessary : <list type="bullet">
/// <item>assemblyPath="Bentley.logging.log4net.dll"</item>
/// <item>typeName="Bentley.Logging.Log4netProvider"</item>
/// <item>Parameters configFile="log4net_properties.xml"/</item>
/// </list></remarks>
/// <author>Robert.Priest</author>                              <date>08/2004</date>
/*==============+===============+===============+===============+===============+======*/
public class EmptyLogger : ILogger
    {
    #region ILogger Members
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    public int trace
    (
    string msg,
    params object[] args
    )
        {
        // TODO:  Add EmptyLogger.trace implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.trace
    (
    string msg
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.trace implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    public int info
    (
    string msg,
    params object[] args
    )
        {
        // TODO:  Add EmptyLogger.info implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.info
    (
    string msg
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.info implementation
        return 0;
        }

    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    public int message
    (
    string nameSpace,
    Bentley.Logging.SEVERITY sev,
    string msg,
    params object[] args
    )
        {
        // TODO:  Add EmptyLogger.message implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.message
    (
    string nameSpace,
    Bentley.Logging.SEVERITY sev,
    string msg
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.message implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.message
    (
    Bentley.Logging.SEVERITY sev,
    string msg,
    params object[] args
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.message implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.message
    (
    Bentley.Logging.SEVERITY sev,
    string msg
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.message implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    public int warning
    (
    string msg,
    params object[] args
    )
        {
        // TODO:  Add EmptyLogger.warning implementation
        System.Diagnostics.Trace.WriteLine (msg);
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.warning
    (
    string msg
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.warning implementation
        System.Diagnostics.Trace.WriteLine (msg);
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    public int debug
    (
    string msg,
    params object[] args
    )
        {
        // TODO:  Add EmptyLogger.debug implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.debug
    (
    string msg
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.debug implementation
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    public int error
    (
    string msg,
    params object[] args
    )
        {
        // TODO:  Add EmptyLogger.error implementation
        System.Diagnostics.Trace.WriteLine (msg);
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.error
    (
    string msg
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.error implementation
        System.Diagnostics.Trace.WriteLine (msg);
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    public int fatal
    (
    string msg,
    params object[] args
    )
        {
        // TODO:  Add EmptyLogger.fatal implementation
        System.Diagnostics.Trace.WriteLine (msg);
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    int Bentley.Logging.ILogger.fatal
    (
    string msg
    )
        {
        // TODO:  Add EmptyLogger.Bentley.Logging.ILogger.fatal implementation
        System.Diagnostics.Trace.WriteLine (msg);
        return 0;
        }
    /*------------------------------------------------------------------------------------**/
    /// <summary></summary>
    /// <author>Robert.Priest</author>                              <date>08/2004</date>
    /*--------------+---------------+---------------+---------------+---------------+------*/
    public bool isSeverityEnabled
    (
    Bentley.Logging.SEVERITY sev
    )
        {
        // TODO:  Add EmptyLogger.isSeverityEnabled implementation
        return false;
        }

    #endregion

    }

}
