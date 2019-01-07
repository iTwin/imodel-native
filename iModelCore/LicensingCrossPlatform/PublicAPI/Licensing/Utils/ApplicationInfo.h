/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Utils/ApplicationInfo.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <BeHttp/IHttpHeaderProvider.h>
#include <Bentley/BeVersion.h>
#include <Bentley/BeThread.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

#define HEADER_MasUuid      "Mas-Uuid"
#define HEADER_MasAppGuid   "Mas-App-Guid"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Carlos.Thomas    12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ApplicationInfo> ApplicationInfoPtr;
typedef const struct  ApplicationInfo& ApplicationInfoCR;
typedef struct ApplicationInfo& ApplicationInfoR;

struct ApplicationInfo : public IHttpHeaderProvider
    {
    private:
		mutable BeMutex m_headersCS;
		mutable std::shared_ptr<HttpRequestHeaders> m_headers;

        Utf8String m_applicationName;
        BeVersion m_applicationVersion;
        Utf8String m_applicationGUID;
        Utf8String m_applicationProductId;
        Utf8String m_deviceId;
        Utf8String m_systemDescription;

        Utf8String m_languageTag;
        Utf8String m_fallbackLanguageTag;

		Utf8String GetUserAgent() const;
		Utf8String GetAcceptLanguage() const;

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

		//! Create product token used in User-Agent header. Example: "TestApplicationName/4.2"
		LICENSING_EXPORT Utf8String GetProductToken() const;

		//! Fill required headers using ClientInfo values
		LICENSING_EXPORT virtual void FillHttpRequestHeaders(HttpRequestHeaders& headersOut) const override;

#if defined (__ANDROID__)

        LICENSING_EXPORT static void CacheAndroidDeviceId(Utf8String deviceId);

#endif
    };

 END_BENTLEY_LICENSING_NAMESPACE
