/*--------------------------------------------------------------------------------------+
|
|     $Source: LoggingSDK/src/native/interface/bsilogprivate.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning (disable:4251)
#endif // _MSC_VER

#include <Bentley/Bentley.h>
#include <Bentley/BeAssert.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>
#include <Bentley/bvector.h>
#include <Bentley/BeThread.h>
#include <Bentley/ScopedArray.h>

#include <Logging/bentleylogging.h>

#if defined(BENTLEY_WIN32) && defined (NOT_NOW)
#define BSILOG_TRY      __try
#define BSILOG_CATCH    __except ( EXCEPTION_EXECUTE_HANDLER )
#else
#define BSILOG_TRY      try
#define BSILOG_CATCH    catch ( ... )
#endif

BEGIN_BENTLEY_LOGGING_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
class BsiLogger : public ILogger
{
protected:
    WString                    m_namespace;
    Provider::ILogProviderContext * m_context;

public:
    BsiLogger ( WCharCP nameSpace, Provider::ILogProviderContext * context ) :
        m_namespace (nameSpace), m_context(context)
        {
        };

    virtual ~BsiLogger ( void )
        {
        };

    Provider::ILogProviderContext *  getContext ( void ) const { return m_context; };
    void  setContext ( Provider::ILogProviderContext * context ) { m_context = context; };
    WStringCR  getNamespace ( void ) const { return m_namespace; };
    void  setNamespace ( WStringCR ns ) { m_namespace = ns; };

    virtual bool isSeverityEnabled ( SEVERITY sev ) const override;
    virtual void message ( SEVERITY sev, WCharCP msg ) override;
    virtual void message ( SEVERITY sev, Utf8CP msg ) override;
    virtual void messagev ( SEVERITY sev, WCharCP msg, ... ) override;
    virtual void messagev ( SEVERITY sev, Utf8CP msg, ... ) override;
    virtual void messageva ( SEVERITY sev, WCharCP msg, va_list args ) override;
    virtual void messageva ( SEVERITY sev, Utf8CP msg, va_list args ) override;
    virtual void message ( WCharCP nameSpace, SEVERITY sev, WCharCP msg ) override;
    virtual void message ( Utf8CP nameSpace, SEVERITY sev, Utf8CP msg ) override;
    virtual void messagev ( WCharCP nameSpace, SEVERITY sev, WCharCP msg, ... ) override;
    virtual void messagev ( Utf8CP nameSpace, SEVERITY sev, Utf8CP msg, ... ) override;
    virtual void messageva ( WCharCP nameSpace, SEVERITY sev, WCharCP msg, va_list args ) override;
    virtual void messageva ( Utf8CP nameSpace, SEVERITY sev, Utf8CP msg, va_list args ) override;
    virtual void fatal ( WCharCP msg ) override;
    virtual void fatal ( Utf8CP msg ) override;
    virtual void fatalv ( WCharCP msg, ... ) override;
    virtual void fatalv ( Utf8CP msg, ... ) override;
    virtual void error ( WCharCP msg ) override;
    virtual void error ( Utf8CP msg ) override;
    virtual void errorv ( WCharCP msg, ... ) override;
    virtual void errorv ( Utf8CP msg, ... ) override;
    virtual void warning ( WCharCP msg ) override;
    virtual void warning ( Utf8CP msg ) override;
    virtual void warningv ( WCharCP msg, ... ) override;
    virtual void warningv ( Utf8CP msg, ... ) override;
    virtual void info ( WCharCP msg ) override;
    virtual void info ( Utf8CP msg ) override;
    virtual void infov ( WCharCP msg, ... ) override;
    virtual void infov ( Utf8CP msg, ... ) override;
    virtual void debug ( WCharCP msg ) override;
    virtual void debug ( Utf8CP msg ) override;
    virtual void debugv ( WCharCP msg, ... ) override;
    virtual void debugv ( Utf8CP msg, ... ) override;
    virtual void trace ( WCharCP msg ) override;
    virtual void trace ( Utf8CP msg ) override;
    virtual void tracev ( WCharCP msg, ... ) override;
    virtual void tracev ( Utf8CP msg, ... ) override;

};

typedef bmap<WString,ILogger*>    LoggerMap;
typedef bvector<BsiLogger*>       LoggerVector;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
class MaxMessageSize
{
protected:
    BeAtomic<uint32_t> m_maxMessageSize;

public:
    MaxMessageSize () :
        m_maxMessageSize (DEFAULT_MESSAGE_SIZE)
        {
        };

    virtual ~MaxMessageSize ( void )
        {
        };

    uint32_t Get ();

    uint32_t Set ( uint32_t value );

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
class LoggerCache
{
public:

    LoggerCache ( void );

    virtual ~LoggerCache ( void );

    ILogger* GetLogger ( WCharCP nameSpace );

    ILogger* CreateLogger ( WCharCP nameSpace );

    int      DestroyLogger ( ILogger* pLogger );

    bool     IsLoggerCached ( ILogger* pLogger );

    bool     IsLoggerCached ( WCharCP nameSpace );

    int      RemoveLogger ( ILogger* pLogger );

    ILogger* RemoveLogger ( WCharCP nameSpace );

    void     RemoveProvider ( Provider::ILogProvider* pProvider );

    int      AddProvider ( Provider::ILogProvider* pProvider );

protected:

    LoggerMap            m_loggers;
    BeMutex m_lock;
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
namespace Provider
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
class SeverityMap
{
public:
    SeverityMap ();

    SeverityMap ( SEVERITY defaultSeverity );

    virtual ~SeverityMap ( void );

    bool IsSeverityEnabled ( WCharCP nameSpace, SEVERITY sev );

    int SetSeverity ( WCharCP nameSpace, SEVERITY severity );

    SEVERITY GetDefaultSeverity ();

    void SetDefaultSeverity (SEVERITY severity);

protected:
    typedef bmap<WString,SEVERITY>     NamespaceSeverityMap;

    NamespaceSeverityMap        m_severity;
    SEVERITY                    m_defaultSeverity;
    BeMutex m_lock;
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
class EmergencyLogProvider
{
public:
    EmergencyLogProvider();

    virtual ~EmergencyLogProvider();

    bool IsActive();

public:
    int STDCALL_ATTRIBUTE Uninitialize ();

    int STDCALL_ATTRIBUTE CreateLogger ( WCharCP nameSpace, ILogProviderContext ** ppContext );

    int STDCALL_ATTRIBUTE DestroyLogger ( ILogProviderContext * pContext );

    void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, WCharCP msg );
    void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, Utf8CP msg );

    bool STDCALL_ATTRIBUTE IsSeverityEnabled ( ILogProviderContext * context, SEVERITY sev );

protected:
    LoggingProviderType m_emergencyProviderType;
    WString        m_emergencyProviderOutputPath;
    ILogProvider*       m_emergencyProvider;

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
class LogProviderProxy : public ILogProvider
{
public:
    LogProviderProxy ();
    virtual ~LogProviderProxy ( void );
    bool isProviderRegistered ();
    bool isEmergencyProvider ();
    ILogProvider* registerProvider ( ILogProvider* pProvider );
    ILogProvider* unregisterProvider ();
    ILogProvider* GetProvider ();

public:
    virtual int STDCALL_ATTRIBUTE Initialize () override;
    virtual int STDCALL_ATTRIBUTE Uninitialize () override;
    virtual int STDCALL_ATTRIBUTE CreateLogger ( WCharCP nameSpace, ILogProviderContext ** ppContext ) override;
    virtual int STDCALL_ATTRIBUTE DestroyLogger ( ILogProviderContext * pContext ) override;
    virtual bool STDCALL_ATTRIBUTE IsSeverityEnabled ( ILogProviderContext * context, SEVERITY sev ) override;
    virtual int STDCALL_ATTRIBUTE SetOption ( WCharCP attribName, WCharCP attribValue ) override;
    virtual int STDCALL_ATTRIBUTE GetOption ( WCharCP attribName, WCharP attribValue, uint32_t valueSize ) override;
    virtual int STDCALL_ATTRIBUTE SetSeverity ( WCharCP nameSpace, SEVERITY severity ) override;

    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, WCharCP msg ) override;
    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, Utf8CP msg ) override;

protected:
    typedef bmap<WString,WString>    OptionMap;

    ILogProvider*           m_pLogProvider;
    OptionMap               m_optionCache;
    EmergencyLogProvider    m_emergencyProvider;

};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
class  ConsoleProvider : public ILogProvider
{
public:
    ConsoleProvider ( void );

    virtual ~ConsoleProvider ( void );

// ILogProvider methods
public:
    virtual int STDCALL_ATTRIBUTE Initialize () override;
    virtual int STDCALL_ATTRIBUTE Uninitialize () override;
    virtual int STDCALL_ATTRIBUTE CreateLogger ( WCharCP nameSpace, ILogProviderContext ** ppContext ) override;
    virtual int STDCALL_ATTRIBUTE DestroyLogger ( ILogProviderContext * pContext ) override;
    virtual int STDCALL_ATTRIBUTE SetOption ( WCharCP attribName, WCharCP attribValue ) override;
    virtual int STDCALL_ATTRIBUTE GetOption ( WCharCP attribName, WCharP attribValue, uint32_t valueSize ) override;
    virtual int STDCALL_ATTRIBUTE SetSeverity ( WCharCP _namespace, SEVERITY severity ) override;
    virtual bool STDCALL_ATTRIBUTE IsSeverityEnabled ( ILogProviderContext * context, SEVERITY sev ) override;

    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, WCharCP msg ) override;
    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, Utf8CP msg ) override;       // we have an optimized version for Android and iOS

protected:

    SeverityMap          m_severity;

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
class SimpleFileProvider : public ILogProvider
    {
public:
    SimpleFileProvider ();

    virtual ~SimpleFileProvider ( void );

// ILogProvider methods
public:
    virtual int STDCALL_ATTRIBUTE Initialize () override;
    virtual int STDCALL_ATTRIBUTE Uninitialize () override;
    virtual int STDCALL_ATTRIBUTE CreateLogger ( WCharCP nameSpace, ILogProviderContext ** pContext ) override;
    virtual int STDCALL_ATTRIBUTE DestroyLogger ( ILogProviderContext * pContext ) override;
    virtual bool STDCALL_ATTRIBUTE IsSeverityEnabled ( ILogProviderContext * context, SEVERITY sev ) override;
    virtual int STDCALL_ATTRIBUTE SetOption ( WCharCP attribName, WCharCP attribValue ) override;
    virtual int STDCALL_ATTRIBUTE GetOption ( WCharCP attribName, WCharP attribValue, uint32_t valueSize ) override;
    virtual int STDCALL_ATTRIBUTE SetSeverity ( WCharCP _namespace, SEVERITY severity ) override;

    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, WCharCP msg ) override;
    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, Utf8CP msg ) override {LogMessage(context,sev,WString(msg, true).c_str());}

protected:
    SeverityMap     m_severity;
    WString         m_name;
    FILE*           m_file;

    int openlog ( void );
    void closelog ( void );

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP GetSeverityText
(
SEVERITY sev
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GetSeverityUtf8Text
(
SEVERITY sev
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SEVERITY GetSeverityFromText
(
WCharCP  sev
);


}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
extern MaxMessageSize               g_maxMessageSize;
Provider::LogProviderProxy&   GetLogProvider();
LoggerCache&                  GetLoggerCache();

#ifdef _MSC_VER
    #pragma warning(pop)
#endif //_MSC_VER

END_BENTLEY_LOGGING_NAMESPACE
