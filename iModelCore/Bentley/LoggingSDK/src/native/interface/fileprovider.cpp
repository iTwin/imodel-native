/*--------------------------------------------------------------------------------------+
|
|     $Source: LoggingSDK/src/native/interface/fileprovider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "bsilogprivate.h"
#include <Bentley/BeFile.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_LOGGING_PROVIDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleFileProvider::SimpleFileProvider() : m_severity ( LOG_ERROR ), m_file ( NULL )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleFileProvider::~SimpleFileProvider ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SimpleFileProvider::Initialize ( void )
    {
    return openlog();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SimpleFileProvider::Uninitialize ( void )
    {
    closelog();
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int SimpleFileProvider::openlog ( void )
    {
    closelog();

    Utf8String aname (m_name);
    FILE* pFile = fopen ( aname.c_str(), "a");

    if ( NULL == pFile  )
        {
        return ERROR;
        }

    m_file = pFile;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleFileProvider::closelog ( void )
    {
    if ( NULL != m_file )
        {
        fclose ( m_file );
        m_file = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SimpleFileProvider::CreateLogger
(
WCharCP                 nameSpace,
ILogProviderContext**   ppContext
)
    {
    BeAssert ( NULL != nameSpace );
    BeAssert ( NULL != ppContext );

    *ppContext = reinterpret_cast<ILogProviderContext*>(new WString ( nameSpace ));

    if ( NULL == *ppContext )
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SimpleFileProvider::DestroyLogger
(
ILogProviderContext * pContext
)
    {
    if ( NULL == pContext )
        {
        return ERROR;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    if ( NULL == pNs )
        {
        return ERROR;
        }

    delete pNs;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE SimpleFileProvider::LogMessage
(
ILogProviderContext*    pContext,
SEVERITY                sev,
WCharCP                 msg
)
    {
    BeAssert ( NULL != pContext );
    BeAssert ( NULL != msg );

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    if ( true == m_severity.IsSeverityEnabled (pNs->c_str(), sev) )
        {
        // Because it is so easy for developers to screw up printf format strings, we're
        // going to protect ourselves from an exception and log a message saying that
        // our developers are stupid and to please contact support
        BSILOG_TRY
            {
            fwprintf ( m_file, L"%-8ls %-20ls %ls\n", GetSeverityText(sev), pNs->c_str(), msg );
            }
        BSILOG_CATCH
            {
            fwprintf ( m_file, L"Formating output string caused an exception!!!" );
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SimpleFileProvider::SetOption
(
WCharCP attribName,
WCharCP attribValue
)
    {
    BeAssert ( NULL != attribName );
    BeAssert ( NULL != attribValue );

    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_DEFAULT_SEVERITY ) )
        {
        m_severity.SetDefaultSeverity ( GetSeverityFromText(attribValue) );
        }
    else if ( 0 == wcscmp ( attribName, CONFIG_OPTION_OUTPUT_FILE ) )
        {
        m_name = attribValue;
        }
    else
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SimpleFileProvider::GetOption
(
WCharCP attribName,
WCharP  attribValue,
uint32_t valueSize
)
    {
    BeAssert ( NULL != attribName );
    BeAssert ( NULL != attribValue );

    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_DEFAULT_SEVERITY ) )
        {
        BeStringUtilities::Wcsncpy ( attribValue, valueSize, GetSeverityText(m_severity.GetDefaultSeverity()) );
        }
    else if ( 0 == wcscmp ( attribName, CONFIG_OPTION_OUTPUT_FILE ) )
        {
        BeStringUtilities::Wcsncpy ( attribValue, valueSize, m_name.c_str() );
        }
    else
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SimpleFileProvider::SetSeverity
(
WCharCP     nameSpace,
SEVERITY    severity
)
    {
    BeAssert ( NULL != nameSpace );

    return m_severity.SetSeverity (nameSpace,severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimpleFileProvider::IsSeverityEnabled
(
ILogProviderContext*    pContext,
SEVERITY                severity
)
    {
    BeAssert ( NULL != pContext );

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    return m_severity.IsSeverityEnabled (pNs->c_str(),severity);
    }

