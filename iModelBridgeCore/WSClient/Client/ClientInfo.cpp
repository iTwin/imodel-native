/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/ClientInfo.h>
#include <Bentley/BeSystemInfo.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

Utf8CP ClientInfo::DefaultLanguage = "en";

#define HEADER_MasUuid      "Mas-Uuid"
#define HEADER_MasAppGuid   "Mas-App-Guid"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ClientInfoPtr ClientInfo::Create
(
Utf8String applicationName,
BeVersion applicationVersion,
Utf8String applicationGUID,
Utf8String applicationProductId,
IHttpHeaderProviderPtr primaryHeaderProvider
)
    {
    Utf8String deviceId = BeSystemInfo::GetDeviceId();

    Utf8String model = BeSystemInfo::GetMachineName();
    if (!model.empty())
        {
        model += "; ";
        }

    Utf8PrintfString systemDescription("%s%s %s",
        model.c_str(),
        BeSystemInfo::GetOSName().c_str(),
        BeSystemInfo::GetOSVersion().c_str());

    return std::shared_ptr<ClientInfo>(new ClientInfo(
        applicationName,
        applicationVersion,
        applicationGUID,
        deviceId,
        systemDescription,
        applicationProductId,
        primaryHeaderProvider
        ));
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ClientInfo::ClientInfo
(
Utf8String applicationName,
BeVersion applicationVersion,
Utf8String applicationGUID,
Utf8String deviceId,
Utf8String systemDescription,
Utf8String applicationProductId,
IHttpHeaderProviderPtr primaryHeaderProvider
) :
m_applicationName(applicationName),
m_applicationVersion(applicationVersion),
m_applicationGUID(applicationGUID),
m_applicationProductId(applicationProductId),
m_deviceId(deviceId),
m_systemDescription(systemDescription),
m_languageTag(ClientInfo::DefaultLanguage),
m_fallbackLanguageTag(ClientInfo::DefaultLanguage),
m_primaryHeaderProvider(primaryHeaderProvider)
    {
    BeAssert(!applicationName.empty());
    BeAssert(!applicationVersion.IsEmpty());
    BeAssert(!applicationGUID.empty());
    //BeAssert (!deviceId.empty ()); // Re-add when Windows device id retrieval is implemented
    BeAssert(!m_systemDescription.empty());
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ClientInfo::~ClientInfo()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientInfo::SetLanguage(Utf8StringCR languageTag)
    {
    BeMutexHolder lock (m_headersCS);
    m_headers = nullptr;
    if (languageTag.EqualsI(m_fallbackLanguageTag))
        {
        m_languageTag = m_fallbackLanguageTag;
        }
    else
        {
        m_languageTag = languageTag;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetLanguage() const
    {
    BeMutexHolder lock (m_headersCS);
    return m_languageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientInfo::SetFallbackLanguage(Utf8StringCR languageTag)
    {
    BeMutexHolder lock (m_headersCS);
    m_headers = nullptr;
    m_fallbackLanguageTag = languageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetFallbackLanguage() const
    {
    BeMutexHolder lock (m_headersCS);
    return m_fallbackLanguageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetAcceptLanguage() const
    {
    BeMutexHolder lock (m_headersCS);

    if (m_languageTag.empty() && m_fallbackLanguageTag.empty())
        {
        return ClientInfo::DefaultLanguage;
        }

    if (m_languageTag.EqualsI(m_fallbackLanguageTag))
        {
        return m_languageTag;
        }

    if (m_languageTag.empty())
        {
        return m_fallbackLanguageTag;
        }

    if (m_fallbackLanguageTag.empty())
        {
        return m_languageTag;
        }

    return m_languageTag + ", " + m_fallbackLanguageTag + ";q=0.6";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetApplicationName() const
    {
    return m_applicationName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersion ClientInfo::GetApplicationVersion() const
    {
    return m_applicationVersion;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetApplicationGUID() const
    {
    return m_applicationGUID;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetApplicationProductId() const
    {
    return m_applicationProductId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetDeviceId() const
    {
    return m_deviceId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetSystemDescription() const
    {
    return m_systemDescription;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetUserAgent() const
    {
    return Utf8PrintfString("%s (%s)", GetProductToken().c_str(), m_systemDescription.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetProductToken() const
    {
    return Utf8PrintfString("%s/%s", m_applicationName.c_str(), m_applicationVersion.ToString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientInfo::FillHttpRequestHeaders(HttpRequestHeaders& headers) const
    {
    BeMutexHolder lock (m_headersCS);
    if (nullptr == m_headers)
        {
        m_headers = std::make_shared<HttpRequestHeaders>();

        // Reporting application that connects
        m_headers->SetUserAgent(GetUserAgent());

        // Request localized response
        m_headers->SetAcceptLanguage(GetAcceptLanguage());

        // WSG feature usage tracking
        m_headers->SetValue(HEADER_MasUuid, m_deviceId);
        m_headers->SetValue(HEADER_MasAppGuid, m_applicationGUID);
        }

    if (nullptr != m_primaryHeaderProvider)
        {
        m_primaryHeaderProvider->FillHttpRequestHeaders(headers);
        }

    for (auto& pair : m_headers->GetMap())
        {
        headers.SetValue(pair.first, pair.second);
        }
    }

#if defined (__ANDROID__)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                   Robert.Priest 06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientInfo::CacheAndroidDeviceId(Utf8String deviceId)
    {
    BeSystemInfo::CacheAndroidDeviceId(deviceId);
    }

#endif