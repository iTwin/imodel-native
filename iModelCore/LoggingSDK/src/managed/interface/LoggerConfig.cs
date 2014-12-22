/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/interface/LoggerConfig.cs $
|
|     $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Reflection;
using System.Xml;
using Microsoft.Win32;

namespace Bentley.Logging
{
/*----------------------------------------------------------------------------------*/
/**
Logger configuration. This class is used to configure the logging interface and register
the application logging provider.
@ingroup cppinterface
*/
/*------------+---------------+---------------+---------------+---------------+------*/
public class LoggerConfig
{
/** Register a logging provider with the logging interface, only one interface can
    be registered at a time.
    @param pLog     => Pointer to povider interface
    @return SUCCESS or ERROR
    @see isInterfaceRegistered */
public static int registerLogInterface (Provider.ILogProvider pLog)
    {
    if (null != LoggerConfigHolder.getProvider () || null == pLog)
        {
        return (-1);
        }

    // A Provider is now registered. Initialize correctly all loggers that are without a provider.
    NoProviderLoggers.AddProvider (pLog);
    LoggerConfigHolder.setProvider (pLog);
    return 0;
    }

/** Determine if a loggin provider has been registered, only one interface can
    be registered at a time.
    @return true or false
    @see registerLogInterface */
public static bool isInterfaceRegistered ()
    {
    if (null != LoggerConfigHolder.getProvider ())
        {
        return true;
        }

    return false;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>
/// Registers a logging provider defined in an xml node. Only does something if no 
/// logging provider has been registered yet.
/// </summary>
/// <remarks>Logging is configured with a file (typically named log4net_properties.xml in our systems).
///    The location of the logging config file is specified by a &lt;LogProvider&gt; element. &lt;LogProvider&gt; specifies the name
///    and location of the log4net configuration file:
///    <code>
///    &lt;LogProvider assemblyPath="Bentley.logging.log4net-2.0.dll" typeName="Bentley.Logging.Provider.Log4netProvider"&gt;
///        &lt;Parameters configFile="log4net_properties.xml"/&gt;
///    &lt;/LogProvider&gt;
///    </code>
///    In the logging configuration file, you configure various "appenders", e.g
///    to output to a rolling log file or stdout or send log messages to a communcations port, etc.
///    Then, in the &lt;root&gt; element you specify which appenders are currently in use.
///    Each logging message is associated with a
///    logging namespaces, and you can specify which severity level should be logged per logging namespace,
///    e.g. you can log "info" (and more severe) messages for namespace X whil logging only "error"
///    (and more severe) messages in namespace Y.
///    see example. </remarks>
/// <example>
///    <code>
///    &lt;?xml version="1.0" encoding="utf-8" ?&gt;
///    &lt;configuration&gt;
///      &lt;configSections&gt;
///        &lt;section name="log4net" type="log4net.Config.Log4NetConfigurationSectionHandler, log4net" /&gt;
///      &lt;/configSections&gt;
///      &lt;log4net&gt;
///        &lt;appender name="ConsoleAppender" type="log4net.Appender.ConsoleAppender"&gt;
///          &lt;layout type="log4net.Layout.PatternLayout"&gt;
///            &lt;param name="ConversionPattern" value="%d [%t] %-5p %c %x - %m%n" /&gt;
///          &lt;/layout&gt;
///        &lt;/appender&gt;
///        &lt;appender name="RollingFileAppender" type="log4net.Appender.RollingFileAppender"&gt;
///          &lt;file value="c:\\temp\\ecom-log-file.txt" /&gt;
///          &lt;appendToFile value="true" /&gt;
///          &lt;rollingStyle value="Size" /&gt;
///          &lt;maxSizeRollBackups value="10" /&gt;
///          &lt;maximumFileSize value="1000KB" /&gt;
///          &lt;layout type="log4net.Layout.PatternLayout"&gt;
///            &lt;param name="ConversionPattern" value="%d [%t] %-5p - %c %x - %m%n" /&gt;
///          &lt;/layout&gt;
///        &lt;/appender&gt;
///        &lt;appender name="TraceAppender" type="log4net.Appender.TraceAppender"&gt;
///          &lt;layout type="log4net.Layout.PatternLayout"&gt;
///            &lt;param name="ConversionPattern" value="%d [%t] %-5p - %c %x - %m%n" /&gt;
///          &lt;/layout&gt;
///        &lt;/appender&gt;
///        &lt;appender name="DevenvAppender" type="log4net.Appender.OutputDebugStringAppender"&gt;
///          &lt;!--  Can be used with http://www.sysinternals.com/Utilities/DebugView.html --&gt;
///          &lt;layout type="log4net.Layout.PatternLayout"&gt;
///            &lt;param name="ConversionPattern" value="%d [%t] %-5p - %c %x - %m%n" /&gt;
///          &lt;/layout&gt;
///        &lt;/appender&gt;
///        &lt;root&gt;
///          &lt;level value="ERROR" /&gt;
///          &lt;appender-ref ref="RollingFileAppender" /&gt;
///          &lt;appender-ref ref="ConsoleAppender" /&gt;
///          &lt;appender-ref ref="TraceAppender" /&gt;
///          &lt;!-- appender-ref ref="DevenvAppender" /--&gt;
///        &lt;/root&gt;
///      &lt;/log4net&gt;
///    &lt;/configuration&gt;
///    </code>
/// </example>
/// <param name="nodeContainingLogProvider">xml parent node containing a LogProvider element</param>
/// <returns>true if the provider was successfully registered. This method will never throw an exception</returns>
/// <author>Robert.Schili</author>                              <date>08/2014</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public static bool RegisterProviderIfNecessary
(
XmlNode nodeContainingLogProvider
)
    {
    if (LoggerConfig.isInterfaceRegistered ())
        return false;

    if (nodeContainingLogProvider == null)
        {
        System.Diagnostics.Trace.WriteLine ("Node containing LogProvider node was null.");
        return false;
        }

    try
        {
        const string CONFIG_ASSEMBLYPATH_ATTRIBUTE = "assemblyPath";
        const string CONFIG_FILE_ATTRIBUTE = "configFile";
        
        // is a logger configured in the config file
        XmlNode logProviderNode = nodeContainingLogProvider.SelectSingleNode ("./LogProvider");
        if (logProviderNode == null)
            {
            string message = "LogProvider not defined in application config file";
            System.Diagnostics.Trace.WriteLine(message);
            return false;
            }

        var assemblypathAttribute = logProviderNode.Attributes[CONFIG_ASSEMBLYPATH_ATTRIBUTE];
        // config node must contain an assembly path attribute
        if (assemblypathAttribute == null)
            {
            string message = "LogProvider element in config file is missing assemblyPath attribute";
            System.Diagnostics.Trace.WriteLine (message);
            System.Diagnostics.Debug.Fail (message);
            return false;
            }

        // does the path to the assembly exist
        string path = ResolvePath (assemblypathAttribute.Value);
        if (null == path)
            {
            string message = "LogProvider assemblyPath is not defined.";
            System.Diagnostics.Trace.WriteLine (message);
            System.Diagnostics.Debug.Fail (message);
            return false;
            }

        // load the assembly
        AssemblyName assemName = null;
        Assembly loadedAssem = null;

        if (System.IO.File.Exists (path))
            {
            assemName = AssemblyName.GetAssemblyName (path);
            loadedAssem = Assembly.Load (assemName);
            }
        else
            {
            string name = System.IO.Path.GetFileNameWithoutExtension (assemblypathAttribute.Value);
            loadedAssem = Assembly.Load (name);
            }

        if (null == loadedAssem)
            {
            string message = "LogProvider assembly failed to load " + path;
            System.Diagnostics.Trace.WriteLine (message);
            System.Diagnostics.Debug.Fail (message);
            return false;
            }

        // config node must contain a type attribute that specifies the type in the assembly that will be
        // used as the log provider.
        var typeNameAttribute = logProviderNode.Attributes["typeName"];

        if (typeNameAttribute == null)
            {
            string message = "LogProvider config element does not contain typeName attribute";
            System.Diagnostics.Trace.WriteLine (message);
            System.Diagnostics.Debug.Fail (message);
            return false;
            }

        Type providerType = loadedAssem.GetType (typeNameAttribute.Value, false, true);
        if (providerType == null)
            {
            string message = String.Format (CultureInfo.InvariantCulture, "LogProvider type '{0}' does not exist in assembly '{1}'",
                typeNameAttribute.Value, path);
            System.Diagnostics.Trace.WriteLine (message);
            System.Diagnostics.Debug.Fail (message);
            return false;
            }

        // config node may contain constructor parameters
        XmlNode typeConfig = logProviderNode.SelectSingleNode ("./Parameters");
        if (typeConfig != null)
            {
            XmlAttribute attribute = typeConfig.Attributes[CONFIG_FILE_ATTRIBUTE];
            if (attribute != null)
                attribute.Value = ResolvePath (attribute.Value);
            }

        // construct an instance of the provider type
        Trace.WriteLine ("Read logging configuration information from ecom configuration file LogProvider element:");
        Trace.WriteLine (logProviderNode.OuterXml);

        Bentley.Logging.Provider.ILogProvider logProvider = ConstructInstanceOf (providerType, typeConfig) as Bentley.Logging.Provider.ILogProvider;
        if (logProvider == null)
            {
            string message = String.Format (CultureInfo.InvariantCulture, "Failed to construct instance of LogProvider type '{0}' in assembly '{1}'",
                typeNameAttribute.Value, path);
            System.Diagnostics.Trace.WriteLine (message);
            System.Diagnostics.Debug.Fail (message);
            return false;
            }

        return LoggerConfig.registerLogInterface (logProvider) == 0;
        }
    catch (Exception ex)
        {
        string message = "Failed to initialize Bentley.Logger\n" + ex.ToString ();
        System.Diagnostics.Trace.WriteLine (message);
        System.Diagnostics.Debug.Fail (message);
        return false;
        }
    }


/*------------------------------------------------------------------------------------**/
/// <summary>Resolves paths by expanding environment variables and substituting
/// registry values.</summary>
/// <returns>If path is of the format
/// [registry_key:registry_value_name]additional_path_info the bracketed value will
/// be resolved by getting the value from the registry.</returns>
/// <author>Adam.Klatzkin</author>                              <date>07/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
private static string ResolvePath
(
string path
)
    {
    if (path == null)
        return null;

    if (path.IndexOf ("%", StringComparison.Ordinal) >= 0)
        path = Environment.ExpandEnvironmentVariables (path);

    // If the path is of the right format, we will obtain the path from the registry
    // [registry_key:registry_value_name]additional_path_info
    if (path.StartsWith ("[", StringComparison.Ordinal))
        {
        int regEnd = path.IndexOf ("]", StringComparison.Ordinal);
        if (regEnd > 0)
            {
            string temp = path;
            path = "";
            string[] regKey = temp.Substring (1, regEnd - 1).Split (new char[] { ':' });
            if ((regKey != null) && (regKey.Length == 2))
                {
                RegistryKey key = Registry.LocalMachine.OpenSubKey (regKey[0], false);
                if (key != null)
                    {
                    string val = key.GetValue (regKey[1]) as string;
                    if (val != null)
                        {
                        path = val;
                        }
                    }
                }

            if (path.Length == 0)
                {
                System.Diagnostics.Trace.WriteLine ("Failed to resolve path because registry value could not be found" +
                    "\n  Path: " + temp +
                    "\n  Registry Key: " + temp.Substring (1, regEnd - 1));
                }

            if (regEnd < temp.Length - 1)
                path += temp.Substring (regEnd + 1);
            }
        }

    if (path.IndexOf (":", StringComparison.Ordinal) == -1)
        {
        // this is a relative path, let's convert to an absolute path
        path = AppDomain.CurrentDomain.SetupInformation.ApplicationBase + "\\" + path;
        }

    return path;
    }


/*------------------------------------------------------------------------------------**/
/// <summary>Construct an instance of the specified type using the configuration
/// parameters under the specified XmlNode</summary>
/// <author>Adam.Klatzkin</author>                              <date>11/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
[SuppressMessage ("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")]
private static object ConstructInstanceOf
(
Type type,
XmlNode typeConfig
)
    {
    try
        {
        // is there any configuration for this log provider type, if so we will assume it contains
        // constructor arguments
        if (typeConfig == null)
            {
            // There is no configuration for the type so we will just expect and invoke
            // an empty constructor.
            ConstructorInfo constructor = type.GetConstructor (Type.EmptyTypes);
            if (constructor == null)
                {
                // the type does not contain a public parameterless constructor but it may
                // contain a GetInstance method so let's check.
                MethodInfo instanceMethod = type.GetMethod ("GetInstance", Type.EmptyTypes);
                if (instanceMethod == null)
                    {
                    System.Diagnostics.Trace.WriteLine ("Failed to initialize extension type <" + type.FullName + "> because it does not contain a " +
                        "parameterless constructor and does not have a configuration element in the config file.");
                    return null;
                    }

                return instanceMethod.Invoke (type, null);
                }
            return constructor.Invoke (null);
            }
        else
            {
            // A type configuration element was found in the configuration file so
            // we will attempt to construct an instance using those settings.
            ConstructorInfo[] constructorArray = type.GetConstructors ();
            foreach (ConstructorInfo constructor in constructorArray)
                {
                // we need to see if the element's attributes match any of the
                // constructor signatures.
                ParameterInfo[] paramArray = constructor.GetParameters ();
                if (paramArray.Length == typeConfig.Attributes.Count)
                    {
                    // attempt to build the parameters array
                    object[] constParams = new object[typeConfig.Attributes.Count];
                    int i = 0;
                    foreach (XmlAttribute attribute in typeConfig.Attributes)
                        {
                        ParameterInfo param = GetParameter (attribute.Name, paramArray);
                        if (param == null)
                            break;

                        constParams[i] = Convert.ChangeType (attribute.Value, param.ParameterType, CultureInfo.InvariantCulture);
                        i++;
                        }

                    if (i == constParams.Length)
                        {
                        // We have filled all parameters so this is a valid constructor
                        return constructor.Invoke (constParams);
                        }
                    }
                }

            string message = "Failed to initialize extension type <" + type.FullName + "> because it does not contain a " +
                "constructor that matches the parameters found in the configuration file.";
            Trace.WriteLine (message);
            Debug.Fail (message);
            return null;
            }
        }
    catch (Exception ex)
        {
        string message = "Failed to initialize extension type <" + type.FullName + "> because the following error occurred.\n" +
            ex.Message + "\n" + ex.StackTrace;
        if (ex.InnerException != null)
            message += "\nInnerException:" + ex.InnerException.Message + "\n" + ex.InnerException.StackTrace;
        Trace.WriteLine (message);
        Debug.Fail (message);
        }

    return null;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Gets the named parameter from the parameter array.</summary>
/// <returns>Null if the parameter does not exist in the array.</returns>
/// <author>Adam.Klatzkin</author>                              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
private static ParameterInfo GetParameter
(
string name,
ParameterInfo[] paramArray
)
    {
    foreach (ParameterInfo paramInfo in paramArray)
        {
        if (paramInfo.Name == name)
            return paramInfo;
        }

    return null;
    }

}
}
