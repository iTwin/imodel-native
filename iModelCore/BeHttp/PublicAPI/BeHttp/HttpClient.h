/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpClient.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>
#include <functional>
#include <atomic>

#include <BeHttp/Http.h>
#include <BeHttp/HttpHeaderProvider.h>
#include <BeHttp/HttpBodyCompressionOptions.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/IHttpHandler.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpClient
{
private:
    IHttpHeaderProviderPtr m_defaultHeadersProvider;
    IHttpHandlerPtr m_handler;
    Credentials m_credentials;
    HttpBodyComprressionOptions m_compressionOptions;

public:
    //! Construct new client with predefined configuration
    //! @param[in] defaultHeadersProvider - provided headers will be included by default to all requests created with this client
    //! @param[in] customHandler - custom handler to be used for requests
    BEHTTP_EXPORT HttpClient(IHttpHeaderProviderPtr defaultHeadersProvider = nullptr, IHttpHandlerPtr customHandler = nullptr);

    // Initialize HttpClient before using any function related to http requests 
    BEHTTP_EXPORT static void Initialize(BeFileNameCR assetsDirectoryPath);
    BEHTTP_EXPORT static BeFileNameCR GetAssetsDirectoryPath();

    // Methods for grouping multiple network requests to one activity
    BEHTTP_EXPORT static void BeginNetworkActivity();
    BEHTTP_EXPORT static void EndNetworkActivity();

    BEHTTP_EXPORT static bool IsNetworkActive();

    // Use percent escape for URLs
    BEHTTP_EXPORT static Utf8String EscapeString(Utf8StringCR inStr);

    //! Handle date formats specified in RFC 822 (updated by RFC 1123), RFC 850 (obsoleted by RFC 1036) and ANSI C's asctime() format.
    BEHTTP_EXPORT static BentleyStatus HttpDateToUnixMillis(uint64_t& unixMilliseconds, Utf8CP dateStr);

    // Set authorization for request
    void SetCredentials(Credentials credentials) {m_credentials = std::move(credentials);}

    // Create requests with client info
    BEHTTP_EXPORT Request CreateRequest(Utf8StringCR url, Utf8StringCR method) const;
    BEHTTP_EXPORT Request CreateGetRequest(Utf8StringCR url) const;
    BEHTTP_EXPORT Request CreateGetRequest(Utf8StringCR url, Utf8StringCR etag) const;
    BEHTTP_EXPORT Request CreateGetJsonRequest(Utf8StringCR url, Utf8StringCR etag = nullptr) const;
    BEHTTP_EXPORT Request CreatePostRequest(Utf8StringCR url) const;

    // Compression options to use when creating requests
    void SetCompressionOptions(HttpBodyComprressionOptions options) {m_compressionOptions = std::move(options);}
    HttpBodyComprressionOptions GetCompressionInfo() {return m_compressionOptions;}
};

typedef HttpClient& HttpClientR;
typedef const HttpClient& HttpClientCR;

END_BENTLEY_HTTP_NAMESPACE
