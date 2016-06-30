/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/ClientInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <BeHttp/IHttpHeaderProvider.h>
#include <Bentley/BeVersion.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ClientInfo> ClientInfoPtr;
typedef const struct  ClientInfo& ClientInfoCR;
typedef struct ClientInfo& ClientInfoR;

struct ClientInfo : public IHttpHeaderProvider
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
        IHttpHeaderProviderPtr m_primaryHeaderProvider;

    private:
        Utf8String GetUserAgent() const;

    public:
        //! Create client info with mandatory fields and initialize other required fields automatically.
        //! More info at WSClient wiki page.
        //! There should be one ClientInfo created per application and shared to code that connects to web services.
        //! @param[in] applicationName - human readable string with company and application name. Format: "Bentley-TestApplication"
        //! @param[in] applicationVersion - major and minor numbers could be used to identify application in server side
        //! @param[in] applicationGUID - unique application GUID used for registering WSG usage
        //! @param[in] applicationProductId - registered 4-digit application product ID (e.g. "1234") used for IMS sign-in [optional otherwise]
        //! Given product ID is used for sign-in relying-party URI that is "sso://wsfed_desktop/1234" for Windows builds and 
        //! "sso://wsfed_mobile/1234" for iOS/Android builds. Both RPs need to be registered to IMS DEV/QA/PROD environments so 
        //! that your application would be allowed to sign-in by IMS.
        //! @param[in] primaryHeaderProvider - [optional] provide additional headers
        WSCLIENT_EXPORT static ClientInfoPtr Create
            (
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String applicationProductId = nullptr,
            IHttpHeaderProviderPtr primaryHeaderProvider = nullptr
            );

        //! Consider using ClientInfo::Create() instead. Create client info with custom values, only useful for testing.
        //! More info at WSClient wiki page.
        //! @param[in] applicationName - human readable string with company and application name. Format: "Bentley-TestApplication"
        //! @param[in] applicationVersion - major and minor numbers could be used to identify application in server side
        //! @param[in] applicationGUID - unique application GUID used for registering WSG usage
        //! @param[in] deviceId - unique device ID used for licensing. Should be different for different devices
        //! @param[in] systemDescription - system desription for User-Agent header
        //! @param[in] applicationProductId - registered 4-digit application product ID (e.g. "1234") used for IMS sign-in [optional otherwise]
        //! Given product ID is used for sign-in relying-party URI that is "sso://wsfed_desktop/1234" for Windows builds and 
        //! "sso://wsfed_mobile/1234" for iOS/Android builds. Both RPs need to be registered to IMS DEV/QA/PROD environments so 
        //! that your application would be allowed to sign-in by IMS.
        //! @param[in] primaryHeaderProvider - [optional] provide additional headers
        WSCLIENT_EXPORT ClientInfo
            (
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String deviceId,
            Utf8String systemDescription,
            Utf8String applicationProductId = nullptr,
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
        WSCLIENT_EXPORT Utf8String GetApplicationProductId() const;
        WSCLIENT_EXPORT Utf8String GetDeviceId() const;
        WSCLIENT_EXPORT Utf8String GetSystemDescription() const;

        //! Create product token used in User-Agent header. Example: "TestApplicationName/4.2"
        WSCLIENT_EXPORT Utf8String GetProductToken() const;

        //! Fill required headers using ClientInfo values
        WSCLIENT_EXPORT virtual void FillHttpRequestHeaders(HttpRequestHeaders& headersOut) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
