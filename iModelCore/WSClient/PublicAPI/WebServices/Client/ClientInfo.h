/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/ClientInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <MobileDgn/Utils/Http/IHttpHeaderProvider.h>
#include <Bentley/BeVersion.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ClientInfo> ClientInfoPtr;
struct ClientInfo : public IHttpHeaderProvider
    {
    private:
        mutable BeCriticalSection m_headersCS;
        mutable std::shared_ptr<HttpRequestHeaders> m_headers;

        Utf8String m_applicationName;
        BeVersion m_applicationVersion;
        Utf8String m_applicationGUID;
        Utf8String m_deviceId;
        Utf8String m_systemDescription;

        Utf8String m_languageTag;
        Utf8String m_fallbackLanguageTag;
        IHttpHeaderProviderPtr m_primaryHeaderProvider;

    private:
        Utf8String GetUserAgent() const;

    public:
        //! Create client info with mandatory fields and initialize other required fields automatically.
        //! There should be one ClientInfo created per application and shared to code that connects to web services.
        //! @param[in] applicationName - human readable string with company and application name. Format: "Bentley-TestApplication"
        //! @param[in] applicationVersion - major and minor numbers could be used to identify application in server side
        //! @param[in] applicationGUID - unique application GUID used for registering WSG usage
        //! @param[in] primaryHeaderProvider - [optional] provide additional headers
        WSCLIENT_EXPORT static ClientInfoPtr Create
            (
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            IHttpHeaderProviderPtr primaryHeaderProvider = nullptr
            );

        //! Consider using ClientInfo::Create() instead. Create client info with custom values, only useful for testing.
        //! @param[in] applicationName - human readable string with company and application name. Format: "Bentley-TestApplication"
        //! @param[in] applicationVersion - major and minor numbers could be used to identify application in server side
        //! @param[in] applicationGUID - unique application GUID used for registering WSG usage
        //! @param[in] deviceId - unique device ID used for licensing. Should be different for different devices
        //! @param[in] systemDescription - system desription for User-Agent header
        //! @param[in] primaryHeaderProvider - [optional] provide additional headers
        WSCLIENT_EXPORT ClientInfo
            (
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String deviceId,
            Utf8String systemDescription,
            IHttpHeaderProviderPtr primaryHeaderProvider = nullptr
            );

        WSCLIENT_EXPORT virtual ~ClientInfo();

        //! Default and fallback language tag used - "en" (English)
        WSCLIENT_EXPORT static Utf8CP DefaultLanguage;

        //! Override user language. Values must follow IETF language tag standard (RFC 1766).
        //! Examples - "en", "en-US", "nan-Hant-TW", etc.
        WSCLIENT_EXPORT void SetLanguage(Utf8StringCR languageTag);
        WSCLIENT_EXPORT Utf8String GetLanguage() const;

        //! Override default or change fallback language. See SetLanguage() for more info.
        WSCLIENT_EXPORT void SetFallbackLanguage(Utf8StringCR languageTag);
        WSCLIENT_EXPORT Utf8String GetFallbackLanguage() const;

        //! Get value for accept language header
        WSCLIENT_EXPORT Utf8String GetAcceptLanguage() const;

        WSCLIENT_EXPORT Utf8String GetApplicationName() const;
        WSCLIENT_EXPORT BeVersion GetApplicationVersion() const;
        WSCLIENT_EXPORT Utf8String GetApplicationGUID() const;
        WSCLIENT_EXPORT Utf8String GetDeviceId() const;
        WSCLIENT_EXPORT Utf8String GetSystemDescription() const;

        //! Create product token used in User-Agent header. Example: "TestApplicationName/4.2"
        WSCLIENT_EXPORT Utf8String GetProductToken() const;

        //! Fill required headers using ClientInfo values
        WSCLIENT_EXPORT virtual void FillHttpRequestHeaders(HttpRequestHeaders& headersOut) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
