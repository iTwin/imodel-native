/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ApplicationInfo.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>

#include <Bentley/BeVersion.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ApplicationInfo> ApplicationInfoPtr;
typedef const struct  ApplicationInfo& ApplicationInfoCR;
typedef struct ApplicationInfo& ApplicationInfoR;

struct ApplicationInfo
    {
    private:
        Utf8String m_applicationName;
        BeVersion m_applicationVersion;
        Utf8String m_applicationGUID;
        Utf8String m_applicationProductId;
        Utf8String m_deviceId;
        Utf8String m_systemDescription;

        Utf8String m_languageTag;
        Utf8String m_fallbackLanguageTag;

    public:
        LICENSING_EXPORT static ApplicationInfoPtr Create
        (
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String applicationProductId = nullptr
        );

        LICENSING_EXPORT ApplicationInfo
        (
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String deviceId,
            Utf8String systemDescription,
            Utf8String applicationProductId = nullptr
        );

        LICENSING_EXPORT virtual ~ApplicationInfo();

        //! Override user language. Values must follow IETF language tag standard (RFC 1766).
        //! Examples - "en", "en-US", "nan-Hant-TW", etc.
        LICENSING_EXPORT void SetLanguage(Utf8StringCR languageTag);
        LICENSING_EXPORT Utf8String GetLanguage() const;

        //! Override default or change fallback language. See SetLanguage() for more info.
        LICENSING_EXPORT void SetFallbackLanguage(Utf8StringCR languageTag);
        LICENSING_EXPORT Utf8String GetFallbackLanguage() const;

        LICENSING_EXPORT Utf8String GetApplicationName() const;
        LICENSING_EXPORT BeVersion GetApplicationVersion() const;
        LICENSING_EXPORT Utf8String GetApplicationGUID() const;
        LICENSING_EXPORT Utf8String GetApplicationProductId() const;
        LICENSING_EXPORT Utf8String GetDeviceId() const;
        LICENSING_EXPORT Utf8String GetSystemDescription() const;

#if defined (__ANDROID__)

        LICENSING_EXPORT static void CacheAndroidDeviceId(Utf8String deviceId);

#endif
    };

 END_BENTLEY_LICENSING_NAMESPACE
