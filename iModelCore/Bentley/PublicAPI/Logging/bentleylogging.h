/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Logging/bentleylogging.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

/** @namespace Bentley::NativeLogging Cross-platform logging utilities */
#define BEGIN_BENTLEY_LOGGING_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace NativeLogging {
#define END_BENTLEY_LOGGING_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_LOGGING using namespace BentleyApi::NativeLogging;

//__PUBLISH_SECTION_END__
#define USING_NAMESPACE_BENTLEY_LOGGING_PROVIDER using namespace BentleyApi::NativeLogging::Provider;

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_LOGGING_NAMESPACE

/**
* @addtogroup LoggingGroup
* Cross-platform logging utilities.
*/

// Default message size for printf formated messages.
#define DEFAULT_MESSAGE_SIZE        1024

//=======================================================================================
//! Logger message severity levels.
//! @ingroup LoggingGroup
//=======================================================================================
typedef enum
    {
    LOG_FATAL = 0,      //!< Used for fatal errors that will terminate the application
    LOG_ERROR = (-1),   //!< Used for general errors
    LOG_WARNING = (-2), //!< Used for general warnings
    LOG_INFO = (-3),    //!< Used for general information
    LOG_DEBUG = (-4),   //!< Used for debugging information
    LOG_TRACE = (-5)    //!< Used for tracing function calls
    } SEVERITY;

//__PUBLISH_SECTION_END__
// Logger message severity levels in text form.
#define LOG_TEXT_FATAL      L"FATAL"
#define LOG_TEXT_ERROR      L"ERROR"
#define LOG_TEXT_WARNING    L"WARNING"
#define LOG_TEXT_INFO       L"INFO"
#define LOG_TEXT_DEBUG      L"DEBUG"
#define LOG_TEXT_TRACE      L"TRACE"

#define LOG_UTF8TEXT_FATAL   "FATAL"
#define LOG_UTF8TEXT_ERROR   "ERROR"
#define LOG_UTF8TEXT_WARNING "WARNING"
#define LOG_UTF8TEXT_INFO    "INFO"
#define LOG_UTF8TEXT_DEBUG   "DEBUG"
#define LOG_UTF8TEXT_TRACE   "TRACE"

// Common pre-defined configuration options.
#define CONFIG_OPTION_DEFAULT_SEVERITY      L"DEFAULT_SEVERITY"
#define CONFIG_OPTION_OUTPUT_FILE           L"OUTPUT_FILE"
#define CONFIG_OPTION_CONFIG_FILE           L"CONFIG_FILE"
#define CONFIG_OPTION_CONFIG_REFRESH        L"CONFIG_REFRESH"

namespace Provider {

//=======================================================================================
//! Logging provider context value. This type is passed to ILogProvider methods to identify the logging context.
//! @ingroup LoggingGroup
//=======================================================================================
class ILogProviderContext
{
};

//=======================================================================================
//! Logger provider interface. This class must be implemented by the any logging service
//! that provides logging functionality to the application
//! @ingroup LoggingGroup
//=======================================================================================
class ILogProvider
{
public:


    /** Initialize the provider.
        @return SUCCESS or ERROR
        @see Uninitialize */
    virtual int STDCALL_ATTRIBUTE Initialize () = 0;

    /** Uninitialize the provider.
        @return SUCCESS or ERROR
     *  @see Initialize */
    virtual int STDCALL_ATTRIBUTE Uninitialize () = 0;

    /** Create logger method. Called when user calls LoggerFactory::createLogger.
        @param nameSpace    => requested name for logger namespace
        @param pContext     <= Pointer to logger context value, passed to all subsequent provider interface method calls.
        @return SUCCESS or ERROR
        @see destroyLogger, LoggingManager::GetLogger
             LoggingManager::CreateUncachedLogger */
    virtual int STDCALL_ATTRIBUTE CreateLogger ( WCharCP nameSpace, ILogProviderContext ** ppContext ) = 0;

    /** Destroy logger method. Called when user calls LoggerFactory::destroyLogger.
        @param pContext     => Logger context value for logger to destroy.
        @return SUCCESS or ERROR
        @see createLogger, LoggingManager::ReleaseLogger,
             LoggingManager::DestroyUncachedLogger */
    virtual int STDCALL_ATTRIBUTE DestroyLogger ( ILogProviderContext * pContext ) = 0;

    /** Log message method. Called when user calls any of the logging methods.
        @remarks This method can be slightly more efficient on platforms such as Windows where UTF-16 is directly supported by the C runtime.
        @param context      => Logger context
        @param sev          => Message severity
        @param msg          => Message
        @return SUCCESS or ERROR
        @see ILogger::message */
    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, WCharCP msg ) = 0;

    /** Log message method. Called when user calls any of the logging methods.
        @remarks This method can be slightly more efficient on platforms such as Android where UTF-8 is the native string type.
        @param context      => Logger context
        @param sev          => Message severity
        @param msg          => Message
        @return SUCCESS or ERROR
        @see ILogger::message */
    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, Utf8CP msg ) = 0;

    /** Set an option for the provider.
        @note All calls to SetOption should occur prior to calling
              ActivateProvider or other activation functions
        @param attribName     => name of attribute to be set
        @param attribValue    => value of attribute to be set
        @return SUCCESS or ERROR
        @see LoggingConfig::SetOption */
    virtual int STDCALL_ATTRIBUTE SetOption ( WCharCP attribName, WCharCP attribValue ) = 0;

    /** Get an option for the provider.
        @param attribName     => name of attribute to be set
        @param attribValue    => value of attribute to be set
        @param valueSize      => Size of attribValue buffer in
                              characters
        @return SUCCESS or ERROR
        @see LoggingConfig::GetOption */
    virtual int STDCALL_ATTRIBUTE GetOption ( WCharCP attribName, WCharP attribValue, uint32_t valueSize ) = 0;

    /** Set the logging severity level for the logger with the specified nameSpace.
        @param[in] nameSpace used to identify the logger
        @param[in] severity the logging severity level */
    virtual int  STDCALL_ATTRIBUTE SetSeverity ( WCharCP nameSpace, SEVERITY severity ) = 0;

    /** Determine if logging severity is enabled. Called when user calls ILogger::isSeverityEnabled.
        @param context      => Logger context
        @param sev          => Message severity
        @return true or false
        @see ILogger::isSeverityEnabled */
    virtual bool STDCALL_ATTRIBUTE IsSeverityEnabled ( ILogProviderContext * context, SEVERITY sev ) = 0;

};

};

//__PUBLISH_SECTION_START__
//=======================================================================================
//! Logger interface. This class provides the product-agnostic logging interface.
//! @ingroup LoggingGroup
//=======================================================================================
class ILogger
{
public:
    //! Determine if logging severity is enabled.
    //! @param[in] sev Message severity
    //! @return true or false
    virtual bool isSeverityEnabled ( SEVERITY sev ) const = 0;

    //! Log a message.
    //! @param[in] sev Message severity
    //! @param[in] msg Message
    virtual void message ( SEVERITY sev, WCharCP msg ) = 0;

    //! Log a message.
    //! @param[in] sev Message severity
    //! @param[in] msg Message
    virtual void message ( SEVERITY sev, Utf8CP msg ) = 0;

    //! Log a message with printf style formating.
    //! @param[in] sev Message severity
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void messagev ( SEVERITY sev, WCharCP msg, ... ) = 0;

    //! Log a message with printf style formating.
    //! @param[in] sev Message severity
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void messagev ( SEVERITY sev, Utf8CP msg, ... ) = 0;

    //! Log a message with printf style formating.
    //! @param[in] sev Message severity
    //! @param[in] msg Message with printf style formating
    //! @param[in] args va_list arguments
    virtual void messageva ( SEVERITY sev, WCharCP msg, va_list args ) = 0;

    //! Log a message with printf style formating.
    //! @param[in] sev Message severity
    //! @param[in] msg Message with printf style formating
    //! @param[in] args va_list arguments
    virtual void messageva ( SEVERITY sev, Utf8CP msg, va_list args ) = 0;

    //! Log a message to the specified namespace.
    //! @note This method can be very expensive depending on the log provider implementation
    //! @param[in] nameSpace namespace
    //! @param[in] sev Message severity
    //! @param[in] msg Message
    virtual void message ( WCharCP nameSpace, SEVERITY sev, WCharCP msg ) = 0;

    //! Log a message to the specified namespace.
    //! @note This method can be very expensive depending on the log provider implementation
    //! @param[in] nameSpace namespace
    //! @param[in] sev Message severity
    //! @param[in] msg Message
    virtual void message ( Utf8CP nameSpace, SEVERITY sev, Utf8CP msg ) = 0;

    //! Log a message to the specified namespace with printf style formating.
    //! @note This method can be very expensive depending on the log provider implementation
    //! @param[in] nameSpace namespace
    //! @param[in] sev Message severity
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void messagev ( WCharCP nameSpace, SEVERITY sev, WCharCP msg, ... ) = 0;

    //! Log a message to the specified namespace with printf style formating.
    //! @note This method can be very expensive depending on the log provider implementation
    //! @param[in] nameSpace namespace
    //! @param[in] sev Message severity
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void messagev ( Utf8CP nameSpace, SEVERITY sev, Utf8CP msg, ... ) = 0;

    //! Log a message to the specified namespace with printf style formating.
    //! @note This method can be very expensive depending on the log provider implementation
    //! @param[in] nameSpace namespace
    //! @param[in] sev Message severity
    //! @param[in] msg Message with printf style formating
    //! @param[in] args va_list arguments
    virtual void messageva ( WCharCP nameSpace, SEVERITY sev, WCharCP msg, va_list args ) = 0;

    //! Log a message to the specified namespace with printf style formating.
    //! @note This method can be very expensive depending on the log provider implementation
    //! @param[in] nameSpace namespace
    //! @param[in] sev Message severity
    //! @param[in] msg Message with printf style formating
    //! @param[in] args va_list arguments
    virtual void messageva ( Utf8CP nameSpace, SEVERITY sev, Utf8CP msg, va_list args ) = 0;

    //! Log a message with fatal severity.
    //! @param[in] msg Message
    virtual void fatal ( WCharCP msg ) = 0;

    //! Log a message with fatal severity.
    //! @param[in] msg Message
    virtual void fatal ( Utf8CP msg ) = 0;

    //! Log a message with fatal severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void fatalv ( WCharCP msg, ... ) = 0;

    //! Log a message with fatal severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void fatalv ( Utf8CP msg, ... ) = 0;

    //! Log a message with error severity.
    //! @param[in] msg Message
    virtual void error ( WCharCP msg ) = 0;

    //! Log a message with error severity.
    //! @param[in] msg Message
    virtual void error ( Utf8CP msg ) = 0;

    //! Log a message with error severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void errorv ( WCharCP msg, ... ) = 0;

    //! Log a message with error severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void errorv ( Utf8CP msg, ... ) = 0;

    //! Log a message with warning severity.
    //! @param[in] msg Message
    virtual void warning ( WCharCP msg )= 0;

    //! Log a message with warning severity.
    //! @param[in] msg Message
    virtual void warning ( Utf8CP msg ) = 0;

    //! Log a message with warning severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void warningv ( WCharCP msg, ... )= 0;

    //! Log a message with warning severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void warningv ( Utf8CP msg, ... ) = 0;

    //! Log a message with info severity.
    //! @param[in] msg Message
    virtual void info ( WCharCP msg ) = 0;

    //! Log a message with info severity.
    //! @param[in] msg Message
    virtual void info ( Utf8CP msg ) = 0;

    //! Log a message with info severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void infov ( WCharCP msg, ... ) = 0;

    //! Log a message with info severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void infov ( Utf8CP msg, ... ) = 0;

    //! Log a message with debug severity.
    //! @param[in] msg Message
    virtual void debug ( WCharCP msg ) = 0;

    //! Log a message with debug severity.
    //! @param[in] msg Message
    virtual void debug ( Utf8CP msg ) = 0;

    //! Log a message with debug severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void debugv ( WCharCP msg, ... ) = 0;

    //! Log a message with debug severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void debugv ( Utf8CP msg, ... ) = 0;

    //! Log a message with trace severity.
    //! @param[in] msg Message
    virtual void trace ( WCharCP msg ) = 0;

    //! Log a message with trace severity.
    //! @param[in] msg Message
    virtual void trace ( Utf8CP msg ) = 0;

    //! Log a message with trace severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void tracev ( WCharCP msg, ... ) = 0;

    //! Log a message with trace severity with printf style formating.
    //! @param[in] msg Message with printf style formating
    //! @param[in] ... printf style arguments
    virtual void tracev ( Utf8CP msg, ... ) = 0;
};

//=======================================================================================
//! Known logging provider types.
//! @see LoggingConfig::ActivateProvider
//=======================================================================================
typedef enum
    {
    NULL_LOGGING_PROVIDER = 0,          //!< No logging provider activated
    CONSOLE_LOGGING_PROVIDER = 1,       //!< Console output logging provider
    SIMPLEFILE_LOGGING_PROVIDER = 2,    //!< File output logging provider with limited functionality
    LOG4CXX_LOGGING_PROVIDER = 3,       //!< Fully functional and configurable Log4cxx provider
    MANAGED_LOGGING_PROVIDER = 4,       //!< Provider to pass messages to managed application logging
    UNKNOWN_LOGGING_PROVIDER = 256,     //!< Used for custom provider
    } LoggingProviderType;

//=======================================================================================
//! Logger configuration. This class is used to configure the logging interface and register
//! the application logging provider.
//=======================================================================================
class BENTLEYDLL_EXPORT LoggingConfig
{
public:

    /** Activate one of the known logging providers. This will
        register the logging interface, only one interface can be
        active at a time.
        @param type     => logging provider type to activate
        @return SUCCESS or ERROR
        @see LoadAndActivateProvider, DeactivateProvider,
             IsProviderActive */
    static int STDCALL_ATTRIBUTE ActivateProvider ( LoggingProviderType type );


//__PUBLISH_SECTION_END__
    /** Load and activate an external logging provider. This
        will register and activate the logging provider from the
        specified model, only one interface can be active at a
        time.
        @param moduleName     => name of module that contains the
                              logging provider
        @return SUCCESS or ERROR
        @see LoadAndActivateProvider, DeactivateProvider,
             IsProviderActive */
    static int STDCALL_ATTRIBUTE ActivateProvider ( Provider::ILogProvider* pProvider );

//__PUBLISH_SECTION_START__
    /** Load and activate an external logging provider. This
        will register and activate the logging provider from the
        specified model, only one interface can be active at a
        time.
        @param moduleName     => name of module that contains the
                              logging provider
        @return SUCCESS or ERROR
        @see ActivateProvider, DeactivateProvider,
             IsProviderActive */
    static int STDCALL_ATTRIBUTE LoadAndActivateProvider ( WCharCP moduleName );

    /** Deactivate a logging provider.
        @return SUCCESS or ERROR
        @see ActivateProvider, LoadAndActivateProvider */
    static int STDCALL_ATTRIBUTE DeactivateProvider ( void );

    /** Determine if a logging provider is active, only one provider
        can be active at a time.
        @return true or false
        @see ActivateProvider, LoadAndActivateProvider,
             DeactivateProvider */
    static bool STDCALL_ATTRIBUTE IsProviderActive ( void );

    /** Set an option for the provider.
        @note All calls to SetOption should occur prior to calling
              ActivateProvider or other activation functions
        @param attribName     => name of attribute to be set. See #defines starting with CONFIG_OPTION_*, e.g. CONFIG_OPTION_DEFAULT_SEVERITY, CONFIG_OPTION_CONFIG_FILE, etc.
        @param attribValue    => value of attribute to be set
        @return SUCCESS or ERROR
        @see SetSeverity, SetMaxMessageSize 
        @private */
    static int STDCALL_ATTRIBUTE SetOption ( WCharCP attribName, WCharCP attribValue );

    /** Get an option for the provider.
        @param attribName     => name of attribute to be set
        @param attribValue    => buffer to recieve value of
                              attribute
        @param valueSize      => Size of attribValue buffer in
                              characters
        @return SUCCESS or ERROR
        @see SetOption
        @private */
    static int STDCALL_ATTRIBUTE GetOption ( WCharCP attribName, WCharP attribValue, uint32_t valueSize );

    /** Set the logging severity level for the logger with the specified nameSpace.
        @param[in] nameSpace used to identify the logger
        @param[in] severity the logging severity level */
    static int  STDCALL_ATTRIBUTE SetSeverity ( WCharCP nameSpace, SEVERITY severity );

    /** Set the logging severity level for the logger with the specified nameSpace.
    @param[in] nameSpace used to identify the logger
    @param[in] severity the logging severity level */
    static int  STDCALL_ATTRIBUTE SetSeverity ( Utf8CP nameSpace, SEVERITY severity );

    /** Set the maximum message size for a formated message. Messages are formated using
        printf style formating rules before they are sent to the provider.
        @param size     => Maximun message size
        @return previous max message size
        @see SetOption, SetSeverity 
        @private */
    static uint32_t STDCALL_ATTRIBUTE SetMaxMessageSize ( uint32_t size );

//__PUBLISH_SECTION_END__
protected:
    /** Load an external logging provider. This will instance a
        logging provider from the specified module, only one
        provider can be active at a time.
        @param moduleName     => name of module that contains the
                              logging provider
        @return Instanced provider or NULL */
    static Provider::ILogProvider* STDCALL_ATTRIBUTE LoadProvider ( WCharCP moduleName );

//__PUBLISH_SECTION_START__
private:
    /** Private constructor so class can not be instanced */
    LoggingConfig ( void );
};

//=======================================================================================
//! Logger factory. This class is used to manage loggers.
//! @ingroup LoggingGroup
//=======================================================================================
class BENTLEYDLL_EXPORT LoggingManager
{
public:
    /** Get the logger for the requested namespace from the cache or
        create a new one and add it to the cache if needed.
        Subsequent calls to this method with the same namespace will
        always return the cached copy of the logger. This method
        should always return a logger no matter if a provider is
        registered or not.
        @param[in] nameSpace Name of namespace
        @return pointer to logger interface

        @if BENTLEY_SDK_Internal
            @note If no provider is registered and the env var
              EMERGENCY_LOGGING is defined with the value CONSOLE for
              FILE,pathToOuputFile log messages will be sent to the
              emergency output
            @see ReleaseLogger 
        @endif
        */
    static ILogger* STDCALL_ATTRIBUTE GetLogger ( WCharCP nameSpace );

    /** Get the logger for the requested namespace from the cache or
        create a new one and add it to the cache if needed.
        Subsequent calls to this method with the same namespace will
        always return the cached copy of the logger. This method
        should always return a logger no matter if a provider is
        registered or not.
        @param[in] nameSpace Name of namespace
        @return pointer to logger interface
        */
    static ILogger* STDCALL_ATTRIBUTE GetLogger ( Utf8CP nameSpace );

//__PUBLISH_SECTION_END__
    /** Create an uncached logger for the requested namespace.
        Susequent calls to this method with the same namespace will
        always return new loggers.
        @param nameSpace    => Name of namespace
        @return pointer to logger interface or NULL
        @see DestroyUncachedLogger */
    static ILogger* STDCALL_ATTRIBUTE CreateUncachedLogger ( WCharCP nameSpace );

    /** Destroy the provided logger.
        @param pLogger      => pointer to logger interface to destroy
        @return SUCCESS or ERROR
        @see CreateUncachedLogger */
    static int STDCALL_ATTRIBUTE DestroyUncachedLogger ( ILogger* pLogger );

    /** Remove the provided logger from the cache and destroy it.
        This method is not typically called and should only be
        called when the logger is no longer needed by application.
        @param pLogger      => pointer to logger interface to destroy
        @return SUCCESS or ERROR
        @see GetLogger */
    static int STDCALL_ATTRIBUTE ReleaseLogger ( ILogger* pLogger );

//__PUBLISH_SECTION_START__
private:
    //! Private constructor so class can not be instanced
    LoggingManager ( void );
};

//__PUBLISH_SECTION_END__
//=======================================================================================
// This class logs the entering leaving of the current scope
//=======================================================================================
class _AutoScopeLogger_t_
    {
    public:
        _AutoScopeLogger_t_ ( ILogger* pLog, char const* scope,
                         WCharCP enterMsg = L"Entering scope: %ls",
                         WCharCP leaveMsg = L"Leaving scope: %ls" ) :
                         m_log (pLog), m_scope (scope), m_enter (enterMsg) , m_leave (leaveMsg)
            {
            m_log->tracev ( m_enter.c_str(), m_scope.c_str() );
            };

        ~_AutoScopeLogger_t_ ( void )
            {
            m_log->tracev ( m_leave.c_str(), m_scope.c_str() );
            };

    protected:
        ILogger*        m_log;
        std::wstring    m_enter;
        std::wstring    m_leave;
        std::string     m_scope;
    };

//__PUBLISH_SECTION_START__

END_BENTLEY_LOGGING_NAMESPACE

//__PUBLISH_SECTION_END__

// Macros for the java style synchronized blocks
#define BENTLEY_LOGGING_CONCAT( x, y ) x ## y

#define BENTLEY_LOGGING_VARNAME(pre,line)   BENTLEY_LOGGING_CONCAT ( pre, line )

#define LOGNAME           BENTLEY_LOGGING_VARNAME(log_,__LINE__)

/*----------------------------------------------------------------------------------*//**
This is a helper macro
*//*------------+---------------+---------------+---------------+---------------+------*/
#define ENABLE_SCOPE_LOGGING(log)   Bentley::NativeLogging::_AutoScopeLogger_t_ LOGNAME ( log, __FUNCDNAME__ )
