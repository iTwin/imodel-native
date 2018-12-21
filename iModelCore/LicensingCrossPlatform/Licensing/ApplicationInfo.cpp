/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ApplicationInfo.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeSystemInfo.h>

#include "ApplicationInfo.h"

USING_NAMESPACE_BENTLEY_LICENSING

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

#if defined (__ANDROID__)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                   Robert.Priest 06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationInfo::CacheAndroidDeviceId(Utf8String deviceId)
    {
    BeSystemInfo::CacheAndroidDeviceId(deviceId);
    }

#endif