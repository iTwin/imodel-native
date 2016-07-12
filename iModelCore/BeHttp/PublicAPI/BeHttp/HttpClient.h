/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpClient.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>
#include <functional>
#include <atomic>

#include <BeHttp/Http.h>
#include <BeHttp/HttpHeaderProvider.h>
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
    static BeFileName s_assetsDirectoryPath;
    static BeAtomic<int> s_tasksInProgressCount;

public:
    //! Construct new client with predefined configuration
    //! @param[in] defaultHeadersProvider - provided headers will be included by default to all requests created with this client
    //! @param[in] customHandler - custom handler to be used for requests
    BEHTTP_EXPORT HttpClient (IHttpHeaderProviderPtr defaultHeadersProvider = nullptr, IHttpHandlerPtr customHandler = nullptr);

    // Initialize HttpClient before using any function related to http requests 
    BEHTTP_EXPORT static void Initialize(BeFileNameCR assetsDirectoryPath);
    BEHTTP_EXPORT static BeFileNameCR GetAssetsDirectoryPath();

    // Methods for grouping multiple network requests to one activity
    BEHTTP_EXPORT static void BeginNetworkActivity ();
    BEHTTP_EXPORT static void EndNetworkActivity ();

    BEHTTP_EXPORT static bool IsNetworkActive ();

    // Use percent escape for URLs
    BEHTTP_EXPORT static Utf8String EscapeString (Utf8StringCR inStr);

    // Set authorization for request
    BEHTTP_EXPORT void SetCredentials (Credentials credentials);

    // Create requests with client info
    BEHTTP_EXPORT HttpRequest CreateRequest (Utf8StringCR url, Utf8StringCR method) const;
    BEHTTP_EXPORT HttpRequest CreateGetRequest (Utf8StringCR url) const;
    BEHTTP_EXPORT HttpRequest CreateGetRequest (Utf8StringCR url, Utf8StringCR etag) const;
    BEHTTP_EXPORT HttpRequest CreateGetJsonRequest (Utf8StringCR url, Utf8StringCR etag = nullptr) const;
    BEHTTP_EXPORT HttpRequest CreatePostRequest (Utf8StringCR url) const;
    };

typedef HttpClient& HttpClientR;
typedef const HttpClient& HttpClientCR;

END_BENTLEY_HTTP_NAMESPACE
