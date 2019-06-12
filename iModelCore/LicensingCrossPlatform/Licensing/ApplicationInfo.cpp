/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ApplicationInfo.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Licensing/ApplicationInfo.h>
#include <Bentley/BeSystemInfo.h>

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ApplicationInfoPtr ApplicationInfo::Create
    (
    BeVersion version,
    Utf8String productId
    )
    {
    Utf8String deviceId = BeSystemInfo::GetDeviceId();

    return std::shared_ptr<ApplicationInfo>(new ApplicationInfo
        (
        version,
        deviceId,
        productId
        ));
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ApplicationInfo::ApplicationInfo
    (
    BeVersion version,
    Utf8String deviceId,
    Utf8String productId
    ) :
    m_version(version),
    m_productId(productId),
    m_deviceId(deviceId),
    m_languageTag("en") // default language English
    {
    BeAssert(!version.IsEmpty());
    //BeAssert (!deviceId.empty ()); // Re-add when Windows device id retrieval is implemented
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ApplicationInfo::~ApplicationInfo()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationInfo::SetLanguage(Utf8StringCR languageTag)
    {
    m_languageTag = languageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetLanguage() const
    {
    return m_languageTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersion ApplicationInfo::GetVersion() const
    {
    return m_version;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetProductId() const
    {
    return m_productId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ApplicationInfo::GetDeviceId() const
    {
    return m_deviceId;
    }

#if defined (__ANDROID__)
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientInfo::CacheAndroidDeviceId(Utf8String deviceId)
    {
    BeSystemInfo::CacheAndroidDeviceId(deviceId);
    }
#endif
