/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Utils/ApplicationInfo.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeSystemInfo.h>
#include <mutex>

#include "../../PublicAPI/Licensing/Utils/ApplicationInfo.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ApplicationInfoPtr ApplicationInfo::Create
(
    Utf8String applicationName,
    BeVersion applicationVersion,
    Utf8String applicationGUID,
    Utf8String applicationProductId
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

    return std::shared_ptr<ApplicationInfo>(new ApplicationInfo(
        applicationName,
        applicationVersion,
        applicationGUID,
        deviceId,
        systemDescription,
        applicationProductId
    ));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ApplicationInfo::ApplicationInfo
(
    Utf8String applicationName,
    BeVersion applicationVersion,
    Utf8String applicationGUID,
    Utf8String deviceId,
    Utf8String systemDescription,
    Utf8String applicationProductId
) :
    m_applicationName(applicationName),
    m_applicationVersion(applicationVersion),
    m_applicationGUID(applicationGUID),
    m_applicationProductId(applicationProductId),
    m_deviceId(deviceId),
    m_systemDescription(systemDescription)
    {
    BeAssert(!applicationName.empty());
    BeAssert(!applicationVersion.IsEmpty());
    BeAssert(!applicationGUID.empty());
    BeAssert(!deviceId.empty ());
    BeAssert(!m_systemDescription.empty());
    m_languageTag = "en";
    m_fallbackLanguageTag = "en";
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ApplicationInfo::~ApplicationInfo()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationInfo::SetLanguage(Utf8StringCR languageTag)
    {
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
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetLanguage() const
    {
    return m_languageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationInfo::SetFallbackLanguage(Utf8StringCR languageTag)
    {
    m_fallbackLanguageTag = languageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetFallbackLanguage() const
    {
    return m_fallbackLanguageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetApplicationName() const
    {
    return m_applicationName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersion ApplicationInfo::GetApplicationVersion() const
    {
    return m_applicationVersion;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetApplicationGUID() const
    {
    return m_applicationGUID;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetApplicationProductId() const
    {
    return m_applicationProductId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetDeviceId() const
    {
    return m_deviceId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetSystemDescription() const
    {
    return m_systemDescription;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jedrzej.Kosinski    01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetAcceptLanguage() const
{
	BeMutexHolder lock(m_headersCS);

	/*if (m_languageTag.empty() && m_fallbackLanguageTag.empty())
	{
		return ApplicationInfo::DefaultLanguage;
	}*/

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
Utf8String ApplicationInfo::GetUserAgent() const
{
	return Utf8PrintfString("%s (%s)", GetProductToken().c_str(), m_systemDescription.c_str());
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetProductToken() const
{
	return Utf8PrintfString("%s/%s", m_applicationName.c_str(), m_applicationVersion.ToString().c_str());
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jedrzej.Kosinski    01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationInfo::FillHttpRequestHeaders(HttpRequestHeaders& headers) const
{
	BeMutexHolder lock(m_headersCS);
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

	/*if (nullptr != m_primaryHeaderProvider)
	{
		m_primaryHeaderProvider->FillHttpRequestHeaders(headers);
	}*/

	for (auto& pair : m_headers->GetMap())
	{
		headers.SetValue(pair.first, pair.second);
	}
	}

#if defined (__ANDROID__)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                   Robert.Priest 06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationInfo::CacheAndroidDeviceId(Utf8String deviceId)
    {
    BeSystemInfo::CacheAndroidDeviceId(deviceId);
    }

#endif