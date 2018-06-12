/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpClient.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include <BeHttp/CompressionOptions.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/IHttpHandler.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpClient
    {
public:
    struct Options;

private:
    IHttpHeaderProviderPtr m_defaultHeadersProvider;
    IHttpHandlerPtr m_handler;
    Credentials m_credentials;
    CompressionOptions m_compressionOptions;


public:
    //! Construct new client with predefined configuration
    //! @param[in] defaultHeadersProvider - provided headers will be included by default to all requests created with this client
    //! @param[in] customHandler - custom handler to be used for requests
    BEHTTP_EXPORT HttpClient(IHttpHeaderProviderPtr defaultHeadersProvider = nullptr, IHttpHandlerPtr customHandler = nullptr);
    
    //! Initialize platform specific HTTP framework elements. Call once when starting up application.
    //! [Android] - Required for SSL support. Initialize with JNIEnv*
    //! [Windows/iOS/etc.] - Not required, does nothing.
    //! @param[in] arg initialization argument
    BEHTTP_EXPORT static void InitializePlatform(void* arg);

    // Initialize HttpClient before using any function related to http requests 
    BEHTTP_EXPORT static void Initialize(const Options& options);

    //! Reinitialize HTTP framework with original options after uninitialization.
    BEHTTP_EXPORT static void Reinitialize();

    //! Uninitialize HTTP framework before process is shutting down to release any resources.
    //! It is recommended to call this from main thread. Should never be called from web threads or callbacks.
    //! New requests will fail as cancelled unless Reinitialize() is called.
    BEHTTP_EXPORT static void Uninitialize();
    
    BEHTTP_EXPORT static BeFileNameCR GetAssetsDirectoryPath();

    //! Options provided at initialization
    BEHTTP_EXPORT static const Options& GetOptions();

    // Methods for grouping multiple network requests to one activity
    BEHTTP_EXPORT static void BeginNetworkActivity();
    BEHTTP_EXPORT static void EndNetworkActivity();

    BEHTTP_EXPORT static bool IsNetworkActive();

    // DEPRECATED, use BeUri::EscapeString()! Use percent escape for URLs
    static Utf8String EscapeString(Utf8StringCR str) { return BeUri::EscapeString(str); } 
    // DEPRECATED, use BeUri::UnescapeString()! Unsecape percent encoding
    static Utf8String UnescapeString (Utf8StringCR str) { return BeUri::UnescapeString(str); }

    //! Handle date formats specified in RFC 822 (updated by RFC 1123), RFC 850 (obsoleted by RFC 1036) and ANSI C's asctime() format.
    BEHTTP_EXPORT static BentleyStatus HttpDateToUnixMillis(uint64_t& unixMilliseconds, Utf8CP dateStr);

    //! Enable full logging that could contain sensitive information. Enabled on DEBUG builds by default, disabled otherwise.
    BEHTTP_EXPORT static void EnableFullLogging(bool enable);
    BEHTTP_EXPORT static bool IsFullLoggingEnabled();

    // Set authorization for request
    void SetCredentials(Credentials credentials) {m_credentials = std::move(credentials);}

    // Create requests with client info
    BEHTTP_EXPORT Request CreateRequest(Utf8StringCR url, Utf8StringCR method) const;
    BEHTTP_EXPORT Request CreateGetRequest(Utf8StringCR url) const;
    BEHTTP_EXPORT Request CreateGetRequest(Utf8StringCR url, Utf8StringCR etag) const;
    BEHTTP_EXPORT Request CreateGetJsonRequest(Utf8StringCR url, Utf8StringCR etag = "") const;
    BEHTTP_EXPORT Request CreatePostRequest(Utf8StringCR url) const;

    // Compression options to use when creating requests
    BEHTTP_EXPORT void SetCompressionOptions(CompressionOptions options) {m_compressionOptions = std::move(options);}
    BEHTTP_EXPORT CompressionOptionsCR GetCompressionOptions() const {return m_compressionOptions;}
    };

typedef HttpClient& HttpClientR;
typedef const HttpClient& HttpClientCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass Initialization options
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpClient::Options
    {
private:
    BeFileName m_assetsDirectoryPath;
    uint32_t m_maxConnectionsPerHost = 10;
    uint32_t m_maxTotalConnections = 0;

public:
    Options() {};
    Options(BeFileName assetsDir, uint32_t maxConnectionsPerHost = 10, uint32_t maxTotalConnections = 0) :
        m_assetsDirectoryPath(assetsDir), m_maxConnectionsPerHost(maxConnectionsPerHost), m_maxTotalConnections(maxTotalConnections) {}

    BeFileNameCR GetAssetsDirectoryPath() const { return m_assetsDirectoryPath; }
    uint32_t GetMaxConnectionsPerHost() const { return m_maxConnectionsPerHost; }
    uint32_t GetMaxTotalConnections() const { return m_maxTotalConnections; }
    };

END_BENTLEY_HTTP_NAMESPACE
