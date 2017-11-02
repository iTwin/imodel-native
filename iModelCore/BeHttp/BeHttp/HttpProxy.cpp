/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpProxy.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/HttpProxy.h>

#include <BeHttp/HttpHeaderProvider.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/BeUri.h>

#include "Curl/ThreadCurlHttpHandler.h"

#include "WebLogging.h"

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>

static Utf8String GetProxyUrlIos(CFDictionaryRef proxyDict, CFStringRef proxyKey, CFStringRef portKey);
static void LoadSystemProxySettingsIos(CFDictionaryRef systemProxySettings, Utf8StringR proxyUrl, Utf8StringR pacUrl);

#endif // __APPLE__

#if defined (BENTLEY_WIN32)
#include <windows.h>
#include <winhttp.h>

static Utf8String GetProxyUrlWin32(Utf8StringCR proxyServerSetting);
static Utf8String GetProxyServerWin32(bvector<Utf8String> const& hosts, Utf8CP prefix);
static void LoadSystemProxySettingsWin32(Utf8StringR proxyUrl, Utf8StringR pacUrl, bvector<Utf8String>& proxyBypassHosts);
static void FreeProxyString(LPWSTR proxyString);
static void SplitHosts(Utf8StringCR hostsString, bvector<Utf8String>& hosts);

#endif // BENTLEY_WIN32

USING_NAMESPACE_BENTLEY_HTTP

BeMutex HttpProxy::s_defaultProxyMutex;
HttpProxy HttpProxy::s_defaultProxy;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HttpProxy HttpProxy::GetDefaultProxy()
    {
    BeMutexHolder lock(s_defaultProxyMutex);
    return s_defaultProxy;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpProxy::SetDefaultProxy(HttpProxy proxy)
    {
    BeMutexHolder lock(s_defaultProxyMutex);
    s_defaultProxy = proxy;
    LOG.infov("HttpProxy: Default proxy changed to: %s", s_defaultProxy.ToString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HttpProxy HttpProxy::GetSystemProxy()
    {
    HttpProxy proxy;
    if (!proxy.LoadSystemProxySettings())
        {
        LOG.infov("HttpProxy: System proxy information not loaded");
        return proxy;
        }
    LOG.infov("HttpProxy: System proxy information loaded: %s", proxy.ToString().c_str());
    return proxy;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpProxy::IsValid() const
    {
    if (!m_pacUrl.empty() || !m_pacScript.empty())
        return true;
    if (!m_proxyUrl.empty())
        return true;
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpProxy::ToString() const
    {
    Utf8String str;

    if (!m_pacUrl.empty())
        str += "PAC URL:" + m_pacUrl + " ";

    if (!m_proxyUrl.empty())
        str += "URL:" + m_proxyUrl + " ";

    if (m_credentials.IsValid())
        str += "With user:'" + m_credentials.GetUsername();

    if (str.empty())
        str = "EMPTY";

    return str;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HttpProxy> HttpProxy::GetProxiesForUrl(Utf8String url) const
    {
    bvector<HttpProxy> proxies;
    if (ShouldBypassUrl(url))
        return proxies;
    if (m_pacResponseTask)
        {
        Response& pacResponse = m_pacResponseTask->GetResult();
        if (HttpStatus::OK == pacResponse.GetHttpStatus())
            {
            HttpStringBody const* body = static_cast<HttpStringBody const*>(&pacResponse.GetBody());
            if (nullptr != body)
                m_pacScript = body->AsString();
            m_pacResponseTask = nullptr;
            LOG.infov("HttpProxy: PAC downloaded and loaded");
            }
        else
            {
            Utf8String status = Response::ToStatusString(pacResponse.GetConnectionStatus(), pacResponse.GetHttpStatus());
            LOG.infov("HttpProxy: PAC did not download %s", status.c_str());
            }
        if (m_pacScript.empty())
            m_pacUrl.clear();
        }
    bvector<Utf8String> proxyUrls;
    if (!m_pacUrl.empty())
        proxyUrls = GetProxyUrlsFromPacScript(url);
    if (proxyUrls.empty() && !m_proxyUrl.empty())
        proxyUrls.push_back(m_proxyUrl);
    proxies.reserve(proxyUrls.size());
    for (auto const& proxyUrl : proxyUrls)
        {
        proxies.push_back(HttpProxy(proxyUrl, m_credentials));
        }
    return proxies;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpProxy::ShouldBypassUrl(Utf8StringCR url) const
    {
    if (!m_pacUrl.empty() && url == m_pacUrl)
        return true; // We're in the process of fetching the PAC script; don't use a proxy for that.
    if (m_proxyBypassHosts.empty())
        return false;
    Utf8String host = BeUri(url).GetHost();
    for (auto const& proxyBypassHost : m_proxyBypassHosts)
        {
        if (host == proxyBypassHost)
            return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> HttpProxy::GetProxyUrlsFromPacScript(Utf8StringCR url) const
    {
    bvector<Utf8String> proxyUrls;
#if defined(__APPLE__)
    CFStringRef pacScript = CFStringCreateWithCString(nullptr, m_pacScript.c_str(), kCFStringEncodingUTF8);
    CFStringRef urlString = CFStringCreateWithCString(nullptr, url.c_str(), kCFStringEncodingUTF8);
    CFURLRef urlRef = CFURLCreateWithString(nullptr, urlString, nullptr);
    CFArrayRef proxies = CFNetworkCopyProxiesForAutoConfigurationScript(pacScript, urlRef, nullptr);
    CFRelease(pacScript);
    CFRelease(urlString);
    CFRelease(urlRef);
    if (proxies == nullptr)
        {
        return proxyUrls;
        }
    for (CFIndex i = 0; i < CFArrayGetCount(proxies); ++i)
        {
        CFDictionaryRef proxyDict = (CFDictionaryRef)CFArrayGetValueAtIndex(proxies, i);
        CFStringRef proxyType = (CFStringRef)CFDictionaryGetValue(proxyDict, kCFProxyTypeKey);
        if (CFStringCompare(proxyType, kCFProxyTypeHTTP, 0) == kCFCompareEqualTo ||
            CFStringCompare(proxyType, kCFProxyTypeHTTPS, 0) == kCFCompareEqualTo)
            {
            Utf8String proxyUrl = GetProxyUrlIos(proxyDict, kCFProxyHostNameKey, kCFProxyPortNumberKey);
            if (!proxyUrl.empty())
                proxyUrls.push_back(proxyUrl);
            }
        else if (CFStringCompare(proxyType, kCFProxyTypeNone, 0) == kCFCompareEqualTo)
            proxyUrls.push_back("");
        }
    CFRelease(proxies);
#else // __APPLE__
#if defined(BENTLEY_WIN32)
    HINTERNET hSession = WinHttpOpen(L"User", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    if (nullptr != hSession)
        {
        WString urlw(url.c_str(), true);
        WString pacUrlw(m_pacUrl.c_str(), true);
        WINHTTP_AUTOPROXY_OPTIONS pacOptions;
        WINHTTP_PROXY_INFO proxyInfo;
        memset(&pacOptions, 0, sizeof(pacOptions));
        pacOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
        pacOptions.lpszAutoConfigUrl = pacUrlw.c_str();
        pacOptions.fAutoLogonIfChallenged = TRUE;
        if (WinHttpGetProxyForUrl(hSession, urlw.c_str(), &pacOptions, &proxyInfo))
            {
            if (nullptr != proxyInfo.lpszProxy)
                {
                Utf8String proxyServerSetting(proxyInfo.lpszProxy);
                bvector<Utf8String> hosts;
                SplitHosts(proxyServerSetting, hosts);
                if (!hosts.empty())
                    {
                    for (Utf8StringCR host : hosts)
                        {
                        Utf8String scheme = BeUri(host).GetScheme();
                        if (scheme.empty())
                            {
                            proxyUrls.push_back("http://" + host);
                            }
                        else if (scheme == "http" || scheme == "https")
                            {
                            proxyUrls.push_back(host);
                            }
                        }
                    }
                }
            FreeProxyString(proxyInfo.lpszProxy);
            FreeProxyString(proxyInfo.lpszProxyBypass);
            }
        else
            {
            LOG.errorv("HttpProxy::GetProxyUrlsFromPacScript() WinHttpGetProxyForUrl() failed: %d", GetLastError());
            }
        WinHttpCloseHandle(hSession);
        }
    else
        {
        LOG.errorv("HttpProxy::GetProxyUrlsFromPacScript() WinHttpOpen() failed: %d", GetLastError());
        }
#endif // BENTLEY_WIN32
#endif // !__APPLE__
    return proxyUrls;
    }

#if defined(__APPLE__)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetProxyUrlIos(CFDictionaryRef proxyDict, CFStringRef proxyKey, CFStringRef portKey)
    {
    Utf8String proxyUrl;
    CFStringRef httpProxy = (CFStringRef)CFDictionaryGetValue(proxyDict, proxyKey);
    CFNumberRef httpPortNumber = (CFNumberRef)CFDictionaryGetValue(proxyDict, portKey);
    if (httpProxy != NULL && httpPortNumber != NULL)
        {
        CFStringRef url = CFStringCreateWithFormat(NULL, NULL, CFSTR("http://%@:%@"), httpProxy, httpPortNumber);
        proxyUrl = CFStringGetCStringPtr(url, kCFStringEncodingUTF8);
        CFRelease(url);
        }
    return proxyUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void LoadSystemProxySettingsIos(CFDictionaryRef systemProxySettings, Utf8StringR proxyUrl, Utf8StringR pacUrl)
    {
    CFNumberRef proxyEnabledNum = (CFNumberRef)CFDictionaryGetValue(systemProxySettings, kCFNetworkProxiesHTTPEnable);
    if (proxyEnabledNum != NULL)
        {
        SInt8 proxyEnabled;
        CFNumberGetValue(proxyEnabledNum, kCFNumberSInt8Type, &proxyEnabled);
        if (proxyEnabled != 0)
            proxyUrl = GetProxyUrlIos(systemProxySettings, kCFNetworkProxiesHTTPProxy, kCFNetworkProxiesHTTPPort);
        }
    CFNumberRef pacEnabledNum = (CFNumberRef)CFDictionaryGetValue(systemProxySettings, kCFNetworkProxiesProxyAutoConfigEnable);
    if (pacEnabledNum != NULL)
        {
        SInt8 pacEnabled;
        CFNumberGetValue(pacEnabledNum, kCFNumberSInt8Type, &pacEnabled);
        if (pacEnabled != 0)
            {
            CFStringRef pacString = (CFStringRef)CFDictionaryGetValue(systemProxySettings, kCFNetworkProxiesProxyAutoConfigURLString);
            if (pacString != NULL)
                {
                pacUrl = CFStringGetCStringPtr(pacString, kCFStringEncodingUTF8);
                }
            }
        }
    // Note: the iOS dictionary includes an entry for kCFNetworkProxiesExceptionsList. However,
    // the iOS API doesn't support this entry (it is Mac-only), so we are forced to ignore it.
    }

#endif // __APPLE__

#if defined(BENTLEY_WIN32)

static void FreeProxyString(LPWSTR proxyString)
    {
    if (nullptr != proxyString)
        GlobalFree(proxyString);
    }

static void SplitHosts(Utf8StringCR hostsString, bvector<Utf8String>& hosts)
    {
    BeStringUtilities::Split(hostsString.c_str(), ";", hosts);
    for (auto& host : hosts)
        {
        host.Trim();
        host.ToLower();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void LoadSystemProxySettingsWin32(Utf8StringR proxyUrl, Utf8StringR pacUrl, bvector<Utf8String>& proxyBypassHosts)
    {
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ipc;
    if (!WinHttpGetIEProxyConfigForCurrentUser(&ipc))
        return;

    if (ipc.lpszProxyBypass != nullptr)
        {
        Utf8String hostsString(ipc.lpszProxyBypass);
        SplitHosts(hostsString, proxyBypassHosts);
        }

    if (ipc.lpszAutoConfigUrl != nullptr)
        {
        pacUrl = Utf8String(ipc.lpszAutoConfigUrl);
        }
    else if (ipc.lpszProxy != nullptr)
        {
        Utf8String proxyServerSetting(ipc.lpszProxy);
        proxyUrl = GetProxyUrlWin32(proxyServerSetting);
        }
    FreeProxyString(ipc.lpszProxy);
    FreeProxyString(ipc.lpszProxyBypass);
    FreeProxyString(ipc.lpszAutoConfigUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetProxyUrlWin32(Utf8StringCR proxyServerSetting)
    {
    bvector<Utf8String> hosts;
    SplitHosts(proxyServerSetting, hosts);
    if (hosts.empty())
        return nullptr;
    Utf8String proxyServer(GetProxyServerWin32(hosts, "https="));
    if (!proxyServer.empty())
        return "http://" + proxyServer;
    proxyServer = GetProxyServerWin32(hosts, "http=");
    if (!proxyServer.empty())
        return "http://" + proxyServer;
    return "http://" + hosts[0];
    }

// There is a bug in Microsoft's compiler that claims that the function below is
// unreachable code.
#pragma warning(push)
#pragma warning(disable:4702)
/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetProxyServerWin32(bvector<Utf8String> const& hosts, Utf8CP prefix)
    {
    for (Utf8StringCR host : hosts)
        {
        size_t index = host.find(prefix);
        if (index >= host.size())
            return nullptr;
        Utf8String proxyServer = host.substr(index + strlen(prefix));
        Utf8String scheme = BeUri(proxyServer).GetScheme();
        if (scheme.empty())
            {
            return proxyServer;
            }
        else if (scheme == "http" || scheme == "https")
            {
            return proxyServer.substr(scheme.size() + 3);
            }
        }
    return nullptr;
    }
#pragma warning(pop)

#endif // BENTLEY_WIN32

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpProxy::SetPacUrl(Utf8StringCR pacUrl)
    {
    if (pacUrl != m_pacUrl)
        {
        m_pacScript.clear();
        m_pacUrl = pacUrl;
        m_pacResponseTask = nullptr;
        DownloadPacScriptIfNeeded();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpProxy::SetProxyServer(Utf8StringCR server, int port)
    {
    SetProxyUrl(Utf8PrintfString("http://%s:%d", server.c_str(), port));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpProxy::LoadSystemProxySettings()
    {
    m_proxyUrl.clear();
    m_pacUrl.clear();
    m_pacScript.clear();
    m_proxyBypassHosts.clear();

#if defined(__APPLE__)
    CFDictionaryRef systemProxySettings = CFNetworkCopySystemProxySettings();
    LoadSystemProxySettingsIos(systemProxySettings, m_proxyUrl, m_pacUrl);
    CFRelease(systemProxySettings);
#elif defined(BENTLEY_WIN32)
    LoadSystemProxySettingsWin32(m_proxyUrl, m_pacUrl, m_proxyBypassHosts);
#endif

    if (m_pacUrl.empty())
        return !m_proxyUrl.empty();
    return DownloadPacScriptIfNeeded();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpProxy::DownloadPacScriptIfNeeded()
    {
    if (m_pacUrl.empty())
        return false;
#if defined (BENTLEY_WIN32)
    // Note: the Windows version uses .Net functionality for PAC proxy script execution,
    // and that .Net functionality downloads the PAC script internally.
    return true;
#else // BENTLEY_WIN32
    // We have a PAC URL, so we need to download the PAC script from the specified URL.
    Utf8String scheme = BeUri(m_pacUrl).GetScheme();
    if (scheme == "file")
        {
        // The PAC URL is a file:// URL, so load the file here.
        BeFileName filename(m_pacUrl.substr(scheme.size() + 3));
        BeFile file;
        if (BeFileStatus::Success == file.Open(filename.GetName(), BeFileAccess::Read))
            {
            bvector<Byte> buffer;
            if (BeFileStatus::Success == file.ReadEntireFile(buffer))
                {
                m_pacScript.insert(0, (const char*)&buffer[0], buffer.size());
                return !m_proxyUrl.empty() || !m_pacScript.empty();
                }
            }
        }
    else if (scheme == "http" || scheme == "https")
        {
        // The PAC URL is an http:// URL or https:// URL, so request a download
        // of the URL.
        std::shared_ptr<ThreadCurlHttpHandler> pacHandler = std::make_shared<ThreadCurlHttpHandler>();
        // Custom handler needed to make sure our PAC download request happens on its
        // own thread, to avoid deadlocking the shared HttpHandler thread.
        std::shared_ptr<Request> pacRequest = std::make_shared<Request>(m_pacUrl, "GET", pacHandler);
        if (m_credentials.IsValid())
            pacRequest->SetCredentials(m_credentials);
        pacRequest->SetResponseBody(HttpStringBody::Create());
        m_pacResponseTask = pacHandler->PerformThreadedRequest(*pacRequest);
        return true;
        }
    else
        {
        // The PAC URL isn't file://, http://, or https://, so we don't know what
        // to do with it. Ignore it.
        m_pacUrl.clear();
        }
    return !m_proxyUrl.empty();
#endif // !BENTLEY_WIN32
    }
