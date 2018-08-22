/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpProxy.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/HttpProxy.h>

#include <BeHttp/HttpHeaderProvider.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/BeUri.h>

#include "Curl/ThreadCurlHttpHandler.h"

#include "WebLogging.h"

#if defined(BENTLEYCONFIG_OS_APPLE)
#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>

static Utf8String GetProxyUrlIos(CFDictionaryRef proxyDict, CFStringRef proxyKey, CFStringRef portKey);
static void LoadSystemProxySettingsIos(CFDictionaryRef systemProxySettings, Utf8StringR proxyUrl, Utf8StringR pacUrl);

#endif // BENTLEYCONFIG_OS_APPLE

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HttpProxy::HttpProxy(Utf8String proxyUrl, Credentials credentials) :
    m_proxyUrl(proxyUrl),
    m_credentials(credentials)
    {
    BeUri::EscapeUnsafeCharactersInUrl(m_proxyUrl);
    }

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
        str += "With user:" + m_credentials.GetUsername();

    if (str.empty())
        str = "EMPTY";

    return str;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpProxy::GetProxiesForUrl(Utf8StringCR requestUrl, bvector<HttpProxy>& proxiesOut) const
    {
    proxiesOut.clear();

    if (ShouldBypassUrl(requestUrl))
        return SUCCESS;

    if (m_pacResponseTask)
        {
        Response& pacResponse = m_pacResponseTask->GetResult();
        if (HttpStatus::OK == pacResponse.GetHttpStatus())
            {
            HttpStringBody const* body = static_cast<HttpStringBody const*>(&pacResponse.GetBody());
            if (nullptr != body)
                m_pacScript = body->AsString();
            m_pacResponseTask = nullptr;
            LOG.infov("HttpProxy: PAC file '%s' downloaded and loaded", pacResponse.GetEffectiveUrl().c_str()); // TODO: this is shown for each HttpProxy copy
            }
        else
            {
            Utf8String status = Response::ToStatusString(pacResponse.GetConnectionStatus(), pacResponse.GetHttpStatus());
            LOG.errorv("HttpProxy: PAC file '%s' did not download: '%s'", pacResponse.GetEffectiveUrl().c_str(), status.c_str());
            return ERROR;
            }
        }

    bvector<Utf8String> proxyUrls;
    if (!m_pacUrl.empty() && SUCCESS != GetProxyUrlsFromPacScript(requestUrl, proxyUrls))
        return ERROR;
    if (proxyUrls.empty() && !m_proxyUrl.empty())
        proxyUrls.push_back(m_proxyUrl);

    proxiesOut.reserve(proxyUrls.size());
    for (auto const& proxyUrl : proxyUrls)
        proxiesOut.push_back(HttpProxy(proxyUrl, m_credentials));

    return SUCCESS;
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
    Utf8String authority = BeUri(url).GetAuthority(); // Using authority to include port
    for (auto const& proxyBypassHost : m_proxyBypassHosts)
        {
        if (authority == proxyBypassHost)
            return true;
        }
    return false;
    }

#if defined(BENTLEY_WIN32)
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpProxy::GetLastErrorAsString()
    {
    //Returns the last Win32 error, in string format. Returns an empty string if there is no error.

    //Get the error message, if any.
    DWORD errorId = ::GetLastError();
    if (errorId == 0)
        return "0"; // No error message has been recorded

    LPSTR buffer = nullptr;
    size_t size = FormatMessageA
        (
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &buffer, 0, nullptr
        );

    Utf8String message(buffer, size);
    LocalFree(buffer);

    if (message.empty())
        {
        // Handle common errors in HttpProxy code
        Utf8CP label = nullptr;
        switch (errorId)
            {
            case ERROR_WINHTTP_UNRECOGNIZED_SCHEME :
                label = "The URL specified a scheme other than HTTP or HTTPS."; break;
            case ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT :
                label = "The PAC file cannot be downloaded."; break;
            case ERROR_WINHTTP_BAD_AUTO_PROXY_SCRIPT:
                label = "An error occurred executing the script code in the Proxy Auto-Configuration (PAC) file."; break;
            default:
                label = "Windows error"; break;
            }
        message.Sprintf("%s (%d)", label, errorId);
        }

    return message;
    }
#endif // BENTLEY_WIN32

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpProxy::GetProxyUrlsFromPacScript(Utf8StringCR requestUrl, bvector<Utf8String>& proxyUrlsOut) const
    {
    BentleyStatus result = SUCCESS;
#if defined(BENTLEYCONFIG_OS_APPLE)
    CFStringRef pacScript = CFStringCreateWithCStringNoCopy(nullptr, m_pacScript.c_str(), kCFStringEncodingUTF8, kCFAllocatorNull);
    CFStringRef urlString = CFStringCreateWithCStringNoCopy(nullptr, requestUrl.c_str(), kCFStringEncodingUTF8, kCFAllocatorNull);
    CFURLRef urlRef = CFURLCreateWithString(nullptr, urlString, nullptr);

    Utf8String errorStr;
    CFErrorRef error = nullptr;
    CFArrayRef proxies = nullptr;

    if (nullptr != pacScript && nullptr != urlRef)
        {
        proxies = CFNetworkCopyProxiesForAutoConfigurationScript(pacScript, urlRef, &error);
        }
    else
        {
        errorStr = "HttpProxy: Failed to read PAC script";
        }

    if (nullptr != pacScript)
        CFRelease(pacScript);
    if (nullptr != urlString)
        CFRelease(urlString);
    if (nullptr != urlRef)
        CFRelease(urlRef);

    if (nullptr != error)
        {
        CFStringRef errorStrCf = CFErrorCopyDescription(error);
        char buffer[256];
        CFStringGetCString(errorStrCf, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        errorStr = buffer;
        if (errorStr.empty())
            errorStr = "Uknown error";
        CFRelease(errorStrCf);
        CFRelease(error);
        }

    if (errorStr.empty() && nullptr == proxies)
        {
        if (errorStr.empty())
            errorStr = "Uknown error";
        }

    if (nullptr != proxies)
        {
        for (CFIndex i = 0; i < CFArrayGetCount(proxies); ++i)
            {
            CFDictionaryRef proxyDict = (CFDictionaryRef)CFArrayGetValueAtIndex(proxies, i);
            CFStringRef proxyType = (CFStringRef)CFDictionaryGetValue(proxyDict, kCFProxyTypeKey);
            if (CFStringCompare(proxyType, kCFProxyTypeHTTP, 0) == kCFCompareEqualTo ||
                CFStringCompare(proxyType, kCFProxyTypeHTTPS, 0) == kCFCompareEqualTo)
                {
                Utf8String proxyUrl = GetProxyUrlIos(proxyDict, kCFProxyHostNameKey, kCFProxyPortNumberKey);
                if (!proxyUrl.empty())
                    proxyUrlsOut.push_back(proxyUrl);
                }
            else if (CFStringCompare(proxyType, kCFProxyTypeNone, 0) == kCFCompareEqualTo)
                proxyUrlsOut.push_back("");
            }
        }

    if (nullptr != proxies)
        CFRelease(proxies);

    if (!errorStr.empty())
        {
        LOG.errorv("HttpProxy: Failed to get proxy URL from PAC file '%s'. Error: '%s'", m_pacUrl.c_str(), errorStr.c_str());
        return ERROR;
        }

#else // BENTLEYCONFIG_OS_APPLE
#if defined(BENTLEY_WIN32)
    HINTERNET hSession = WinHttpOpen(L"User", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    if (nullptr != hSession)
        {
        WString urlw(requestUrl.c_str(), true);
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
                for (Utf8StringCR host : hosts)
                    {
                    Utf8String scheme = BeUri(host).GetScheme();
                    if (scheme.empty())
                        proxyUrlsOut.push_back("http://" + host);
                    else if ("http" == scheme || "https" == scheme)
                        proxyUrlsOut.push_back(host);
                    }
                }
            FreeProxyString(proxyInfo.lpszProxy);
            FreeProxyString(proxyInfo.lpszProxyBypass);
            }
        else
            {
            LOG.errorv("HttpProxy: Failed to get proxy URL from PAC file '%s'. Error: '%s'", m_pacUrl.c_str(), GetLastErrorAsString().c_str());
            result = ERROR;
            }
        WinHttpCloseHandle(hSession);
        }
    else
        {
        LOG.errorv("HttpProxy: WinHttpOpen failed. Error: '%s'", GetLastErrorAsString().c_str());
        result = ERROR;
        }
#endif // BENTLEY_WIN32
#endif // !BENTLEYCONFIG_OS_APPLE
    return result;
    }

#if defined(BENTLEYCONFIG_OS_APPLE)

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

#endif // BENTLEYCONFIG_OS_APPLE

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
void HttpProxy::SetPacUrl(Utf8String pacUrl)
    {
    if (pacUrl == m_pacUrl)
        return;

    m_pacScript.clear();
    m_pacUrl = pacUrl;
    m_pacResponseTask = nullptr;
    DownloadPacScriptIfNeeded();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpProxy::SetProxyServer(Utf8StringCR hostname, int port)
    {
    if (hostname.empty())
        {
        SetProxyUrl("");
        return;
        }
    SetProxyUrl(Utf8PrintfString("http://%s:%d", hostname.c_str(), port));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpProxy::SetProxyUrl(Utf8String proxyUrl)
    {
    m_proxyUrl = proxyUrl;
    BeUri::EscapeUnsafeCharactersInUrl(m_proxyUrl);
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

#if defined(BENTLEYCONFIG_OS_APPLE)
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
        LOG.infov("HttpProxy: Unknown scheme in PAC URL: '%s'", m_pacUrl.c_str());
        }
    return !m_proxyUrl.empty();
#endif // !BENTLEY_WIN32
    }
