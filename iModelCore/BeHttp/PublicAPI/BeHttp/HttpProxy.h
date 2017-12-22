/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpProxy.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/Credentials.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

typedef struct HttpProxy& HttpProxyR;
typedef const struct HttpProxy& HttpProxyCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Travis.Cobbs          09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpProxy
    {
private:
    static BeMutex s_defaultProxyMutex;
    static HttpProxy s_defaultProxy;

private:
    Utf8String m_proxyUrl;
    Credentials m_credentials;
    
    mutable Utf8String m_pacUrl;
    mutable Utf8String m_pacScript;
    bvector<Utf8String> m_proxyBypassHosts;
    mutable Tasks::AsyncTaskPtr<Response> m_pacResponseTask;
    
private:
    bool LoadSystemProxySettings();
    bool ShouldBypassUrl(Utf8StringCR url) const;
    bool DownloadPacScriptIfNeeded();
    BentleyStatus GetProxyUrlsFromPacScript(Utf8StringCR requestUrl, bvector<Utf8String>& proxyUrlsOut) const;
    static Utf8String GetLastErrorAsString();

public:
    //! Create empty proxy
    HttpProxy() = default;
    //! Create proxy with URL
    HttpProxy(Utf8String proxyUrl) : HttpProxy(proxyUrl, Credentials()) {}
    //! Create proxy with URL and credentials
    BEHTTP_EXPORT HttpProxy(Utf8String proxyUrl, Credentials credentials);

    //! Check if proxy is configured
    BEHTTP_EXPORT bool IsValid() const;

    //! Get description about proxy
    BEHTTP_EXPORT Utf8String ToString() const;

    CredentialsCR GetCredentials() const { return m_credentials; }
    void SetCredentials(CredentialsCR credentials) { m_credentials = credentials; }

    Utf8StringCR GetProxyUrl() const { return m_proxyUrl; }
    BEHTTP_EXPORT void SetProxyUrl(Utf8String proxyUrl);
    BEHTTP_EXPORT void SetProxyServer(Utf8StringCR hostname, int port);

    //! Resolve proxies for given URL. Will use available information in proxy configuration - proxy URL or PAC script.
    //! @param[in] requestUrl HTTP request URL to get proxy for.
    //! @param[out] proxiesOut proxies to use. Should use first working proxy or fail request. Will override provided list.
    //! @return ERROR if proxy configuration is incorrect or could not resolve proxies. SUCCESS otherwise.
    BEHTTP_EXPORT BentleyStatus GetProxiesForUrl(Utf8StringCR requestUrl, bvector<HttpProxy>& proxiesOut) const;

    //! Set Proxy Access Configuration script URL to use for proxy URLs
    BEHTTP_EXPORT void SetPacUrl(Utf8String pacUrl);

    //! Get current system proxy configuration. Can be used with SetDefaultProxy() to set it for process.
    BEHTTP_EXPORT static HttpProxy GetSystemProxy();
    
    //! Get default proxy for whole process
    BEHTTP_EXPORT static HttpProxy GetDefaultProxy();
    //! Set default proxy for whole process
    BEHTTP_EXPORT static void SetDefaultProxy(HttpProxy proxy);
    };

END_BENTLEY_HTTP_NAMESPACE
