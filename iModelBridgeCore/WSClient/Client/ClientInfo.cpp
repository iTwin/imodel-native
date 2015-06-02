/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/ClientInfo.h>
#include <MobileDgn/Device.h>

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES

Utf8CP ClientInfo::DefaultLanguage = "en";

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ClientInfo::ClientInfo
(
Utf8String applicationName,
BeVersion applicationVersion,
Utf8String applicationId,
Utf8String deviceId,
Utf8String systemDescription,
IHttpHeaderProviderPtr primaryHeaderProvider
) :
m_applicationName (applicationName),
m_applicationVersion (applicationVersion),
m_applicationId (applicationId),
m_deviceId (deviceId),
m_systemDescription (systemDescription),
m_languageTag (ClientInfo::DefaultLanguage),
m_fallbackLanguageTag (ClientInfo::DefaultLanguage),
m_primaryHeaderProvider (primaryHeaderProvider)
    {
    BeAssert (!applicationName.empty ());
    BeAssert (!applicationVersion.IsEmpty ());
    BeAssert (!applicationId.empty ());
    //BeAssert (!deviceId.empty ()); // Re-add when Windows device id retrieval is implemented
    BeAssert (!m_systemDescription.empty ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ClientInfoPtr ClientInfo::Create
(
Utf8String applicationName,
BeVersion applicationVersion,
Utf8String applicationId,
IHttpHeaderProviderPtr primaryHeaderProvider
)
    {
    Utf8String deviceId = Device::GetDeviceId ();

    Utf8String model = Device::GetModelName ();
    if (!model.empty ())
        {
        model += "; ";
        }

    Utf8PrintfString systemDescription ("%s%s %s",
        model.c_str (),
        Device::GetOSName ().c_str (),
        Device::GetOSVersion ().c_str ());

    return std::shared_ptr<ClientInfo> (new ClientInfo
        (
        applicationName,
        applicationVersion,
        applicationId,
        deviceId,
        systemDescription,
        primaryHeaderProvider
        ));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ClientInfo::~ClientInfo ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientInfo::SetLanguage (Utf8StringCR languageTag)
    {
    BeMutexHolder lock (m_headersCS);
    m_headers = nullptr;
    if (languageTag.EqualsI (m_fallbackLanguageTag))
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
Utf8String ClientInfo::GetLanguage () const
    {
    BeMutexHolder lock (m_headersCS);
    return m_languageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientInfo::SetFallbackLanguage (Utf8StringCR languageTag)
    {
    BeMutexHolder lock (m_headersCS);
    m_headers = nullptr;
    m_fallbackLanguageTag = languageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetFallbackLanguage () const
    {
    BeMutexHolder lock (m_headersCS);
    return m_fallbackLanguageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetApplicationName () const
    {
    BeMutexHolder lock (m_headersCS);
    return m_applicationName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersion ClientInfo::GetApplicationVersion () const
    {
    BeMutexHolder lock (m_headersCS);
    return m_applicationVersion;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetApplicationId () const
    {
    BeMutexHolder lock (m_headersCS);
    return m_applicationId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetDeviceId () const
    {
    BeMutexHolder lock (m_headersCS);
    return m_deviceId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientInfo::GetSystemDescription () const
    {
    BeMutexHolder lock (m_headersCS);
    return m_systemDescription;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  ClientInfo::FillHttpRequestHeaders (HttpRequestHeaders& headers) const
    {
    BeMutexHolder lock (m_headersCS);
    if (nullptr == m_headers)
        {
        m_headers = std::make_shared<HttpRequestHeaders> ();

        m_headers->SetUserAgent (Utf8PrintfString ("%s/%d.%d (%s)",
            m_applicationName.c_str (),
            m_applicationVersion.GetMajor (),
            m_applicationVersion.GetMinor (),
            m_systemDescription.c_str ()));

        if (m_languageTag.EqualsI (m_fallbackLanguageTag))
            {
            m_headers->SetAcceptLanguage (m_languageTag);
            }
        else
            {
            m_headers->SetAcceptLanguage (m_languageTag + ", " + m_fallbackLanguageTag + ";q=0.6");
            }

        // Licensing specific
        m_headers->SetUuid (m_deviceId);
        m_headers->SetAppGuid (m_applicationId);
        }

    if (nullptr != m_primaryHeaderProvider)
        {
        m_primaryHeaderProvider->FillHttpRequestHeaders (headers);
        }

    for (auto& pair : m_headers->GetMap ())
        {
        headers.SetValue (pair.first, pair.second);
        }
    }
