/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <BeHttp/HttpResponse.h>
#include <Bentley/BeVersion.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSInfo
    {
    public:
        enum class Type
            {
            Unknown = 0,
            BentleyWSG = 1,
            BentleyConnect = 2
            };

    private:
        static const BeVersion s_serverR1From;
        static const BeVersion s_serverR2From;
        static const BeVersion s_serverR3From;
        static const BeVersion s_serverR4From;

        Type m_type;
        BeVersion m_serverVersion;
        BeVersion m_webApiVersion;

    private:
        static void ParseHeaders(HttpResponseHeadersCR headers, Type& typeOut, BeVersion& serverVersionOut, BeVersion& webApiVersionOut);
        static void ParseInfoPage(Http::ResponseCR response, Type& typeOut, BeVersion& serverVersionOut, BeVersion& webApiVersionOut);
        static void ParseAboutPage(Http::ResponseCR response, Type& typeOut, BeVersion& serverVersionOut, BeVersion& webApiVersionOut);
        static BeVersion DeduceWebApiVersion(BeVersionCR serverVersion);

    public:
        struct WebApiVersion;

        //! Create invalid info
        WSCLIENT_EXPORT WSInfo();
        //! Construct info with values
        WSCLIENT_EXPORT WSInfo(BeVersion serverVersion, BeVersion webApiVersion, Type serverType);
        //! Create info from server response
        WSCLIENT_EXPORT WSInfo(Http::ResponseCR response);
        //! Deserialize string info
        WSCLIENT_EXPORT WSInfo(Utf8StringCR serialized);

        //! Returns true if server type and version is known
        WSCLIENT_EXPORT bool IsValid() const;

        //! Returns type of server detected
        WSCLIENT_EXPORT Type GetType() const;

        //! Returns version of WSG used
        WSCLIENT_EXPORT BeVersionCR GetVersion() const;

        WSCLIENT_EXPORT bool IsR2OrGreater() const;
        WSCLIENT_EXPORT bool IsR3OrGreater() const;

        WSCLIENT_EXPORT BeVersionCR GetWebApiVersion() const;
        WSCLIENT_EXPORT bool IsWebApiSupported(BeVersionCR version) const;

        WSCLIENT_EXPORT bool IsNavigationPropertySelectForAllClassesSupported() const;
        WSCLIENT_EXPORT bool IsSchemaDownloadFullySupported() const;

        //! Serialize info to string
        WSCLIENT_EXPORT Utf8String ToString() const;
    };

typedef WSInfo& WSInfoR;
typedef const WSInfo& WSInfoCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
