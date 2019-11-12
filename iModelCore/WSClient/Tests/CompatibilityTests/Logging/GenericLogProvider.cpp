/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "GenericLogProvider.h"

std::shared_ptr<GenericLogProvider> s_provider;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP GetSeverityText(SEVERITY sev)
    {
    switch (sev)
        {
        case LOG_FATAL:     return LOG_TEXT_FATAL;
        case LOG_ERROR:     return LOG_TEXT_ERROR;
        case LOG_WARNING:   return LOG_TEXT_WARNING;
        case LOG_INFO:      return LOG_TEXT_INFO;
        case LOG_DEBUG:     return LOG_TEXT_DEBUG;
        case LOG_TRACE:     return LOG_TEXT_TRACE;
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GetSeverityUtf8Text(SEVERITY sev)
    {
    switch (sev)
        {
        case LOG_FATAL:     return LOG_UTF8TEXT_FATAL;
        case LOG_ERROR:     return LOG_UTF8TEXT_ERROR;
        case LOG_WARNING:   return LOG_UTF8TEXT_WARNING;
        case LOG_INFO:      return LOG_UTF8TEXT_INFO;
        case LOG_DEBUG:     return LOG_UTF8TEXT_DEBUG;
        case LOG_TRACE:     return LOG_UTF8TEXT_TRACE;
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SEVERITY GetSeverityFromText
(
WCharCP sev
)
    {
    SEVERITY  severity = LOG_WARNING;

    if (NULL == sev)
        {
        return severity;
        }

    if (0 == BeStringUtilities::Wcsicmp(sev, LOG_TEXT_FATAL))
        {
        severity = LOG_FATAL;
        }
    else if (0 == BeStringUtilities::Wcsicmp(sev, LOG_TEXT_ERROR))
        {
        severity = LOG_ERROR;
        }
    else if (0 == BeStringUtilities::Wcsicmp(sev, LOG_TEXT_WARNING))
        {
        severity = LOG_WARNING;
        }
    else if (0 == BeStringUtilities::Wcsicmp(sev, LOG_TEXT_INFO))
        {
        severity = LOG_INFO;
        }
    else if (0 == BeStringUtilities::Wcsicmp(sev, LOG_TEXT_DEBUG))
        {
        severity = LOG_DEBUG;
        }
    else if (0 == BeStringUtilities::Wcsicmp(sev, LOG_TEXT_TRACE))
        {
        severity = LOG_TRACE;
        }

    return severity;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::SeverityMap() : m_defaultSeverity(LOG_ERROR)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::SeverityMap(SEVERITY defaultSeverity) : m_defaultSeverity(defaultSeverity)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::~SeverityMap()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SEVERITY SeverityMap::GetDefaultSeverity()
    {
    return m_defaultSeverity;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void SeverityMap::SetDefaultSeverity
(
SEVERITY    severity
)
    {
    m_defaultSeverity = severity;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int SeverityMap::SetSeverity
(
WCharCP     nameSpace,
SEVERITY    severity
)
    {
    BeAssert(NULL != nameSpace);

    BeSharedMutexHolder lock(m_lock);

    bmap<WString, SEVERITY>::iterator it = m_severity.find(nameSpace);

    if (it != m_severity.end())
        {
        (*it).second = severity;
        }
    else
        {
        m_severity.insert(bmap<WString, SEVERITY>::value_type(nameSpace, severity));
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool SeverityMap::IsSeverityEnabled
(
WCharCP     nameSpace,
SEVERITY    sev
)
    {
    BeAssert(NULL != nameSpace);

    BeSharedMutexHolder lock(m_lock);

    SEVERITY    severity = m_defaultSeverity;

    bmap<WString, SEVERITY>::iterator it = m_severity.find(nameSpace);

    if (it != m_severity.end())
        {
        severity = (*it).second;
        }

    if (sev < severity)
        {
        return false;
        }

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GenericLogProvider::GenericLogProvider(GenericLogWriter writer) :
m_severity(LOG_ERROR),
m_writer(writer)
    {
    if (m_writer)
        return;

    BeAssert(false);
    m_writer = [] (SEVERITY sev, WCharCP msg)
        {
        fwprintf(stdout, msg);
        };
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GenericLogProvider::~GenericLogProvider()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int GenericLogProvider::Initialize()
    {
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int GenericLogProvider::Uninitialize()
    {
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int GenericLogProvider::CreateLogger
(
WCharCP                 nameSpace,
ILogProviderContext**   ppContext
)
    {
    if (NULL == ppContext || NULL == nameSpace)
        {
        return ERROR;
        }

    *ppContext = reinterpret_cast<ILogProviderContext*>(new WString(nameSpace));

    if (NULL == *ppContext)
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int GenericLogProvider::DestroyLogger
(
ILogProviderContext*    pContext
)
    {
    if (NULL == pContext)
        {
        return ERROR;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    if (NULL == pNs)
        {
        return ERROR;
        }

    delete pNs;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void GenericLogProvider::LogMessage
(
ILogProviderContext*    pContext,
SEVERITY                sev,
WCharCP                 msg
)
    {
    if (NULL == pContext || NULL == msg)
        {
        return;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert(NULL != pNs);

    if (!m_severity.IsSeverityEnabled(pNs->c_str(), sev))
        {
        return;
        }

    try
        {
        WPrintfString message(L"%-8ls %-20ls %ls\n", GetSeverityText(sev), pNs->c_str(), msg);
        m_writer(sev, message.c_str());
        }
    catch (...)
        {
        m_writer(SEVERITY::LOG_FATAL, L"Formating output string caused an exception!!!");
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void  GenericLogProvider::LogMessage
(
ILogProviderContext*    pContext,
SEVERITY                sev,
Utf8CP                  msg
)
    {
    if (NULL == pContext || NULL == msg)
        {
        return;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert(NULL != pNs);

    if (!m_severity.IsSeverityEnabled(pNs->c_str(), sev))
        {
        return;
        }

    LogMessage(pContext, sev, WString(msg, true).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int  GenericLogProvider::SetOption
(
WCharCP attribName,
WCharCP attribValue
)
    {
    BeAssert(NULL != attribName);
    BeAssert(NULL != attribValue);

    if (0 == wcscmp(attribName, CONFIG_OPTION_DEFAULT_SEVERITY))
        {
        m_severity.SetDefaultSeverity(GetSeverityFromText(attribValue));
        }
    else
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int  GenericLogProvider::GetOption
(
WCharCP attribName,
WCharP  attribValue,
uint32_t valueSize
)
    {
    BeAssert(NULL != attribName);
    BeAssert(NULL != attribValue);

    if (0 == wcscmp(attribName, CONFIG_OPTION_DEFAULT_SEVERITY))
        {
        BeStringUtilities::Wcsncpy(attribValue, valueSize, GetSeverityText(m_severity.GetDefaultSeverity()));
        }
    else
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int  GenericLogProvider::SetSeverity
(
WCharCP     nameSpace,
SEVERITY    severity
)
    {
    BeAssert(NULL != nameSpace);

    return m_severity.SetSeverity(nameSpace, severity);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool GenericLogProvider::IsSeverityEnabled
(
ILogProviderContext*    pContext,
SEVERITY                severity
)
    {
    BeAssert(NULL != pContext);

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert(NULL != pNs);

    return m_severity.IsSeverityEnabled(pNs->c_str(), severity);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int GenericLogProviderActivator::Activate(GenericLogWriter writer)
    {
    LoggingConfig::DeactivateProvider();
    s_provider = std::make_shared<GenericLogProvider>(writer);
    return LoggingConfig::ActivateProvider(s_provider.get());
    }
