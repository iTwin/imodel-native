/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifndef BENTLEY_WINRT
#include <curl/curl.h>
#endif

BEGIN_IMAGEPP_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
enum class HttpRequestStatus
    {
    Success,                  
    CouldNotResolveProxy,     
    CouldNotResolveHost,      
    CouldNotConnect,          
    OperationTimedOut,        
    Aborted,            
    ResponseCodeError,      // Request was successful but response code indicate an error.
    UnknownError,
    };

//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
enum class HttpResponseStatus : int32_t
    {
    Unknown = -1,

    NoResponceCode = 0,     // Will be zero if no server response code has been received

    // Successful
    Success = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,

    // Redirection
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    UseProxy = 305,
    TemporaryRedirect = 307,

    // Client error
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    RequestEntityTooLarge = 413,
    RequestUriTooLong = 414,
    UnsupportedMediaType = 415,
    RequestedRangeNotSatisfyable = 416,
    ExpectationFailed = 417,

    // Server error
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HttpVersionNotSupported = 505,
    };

//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
struct Credentials
{
private:
    Utf8String m_username;
    Utf8String m_password;

public:
    //! Empty credentials
    Credentials (){};

    //! Create credentials with username and password.
    Credentials (Utf8CP username, Utf8CP password):m_username(username), m_password(password) {}

    Credentials (const Credentials&) = default;   
    Credentials& operator= (const Credentials&) = default;

    ~Credentials (){};

    bool IsEmpty () const{return m_username.empty() && m_password.empty();}

    void SetUsername (Utf8CP username) {m_username = username;}
    void SetPassword (Utf8CP password) {m_password = password;}

    Utf8StringCR GetUsername () const {return m_username;}
    Utf8StringCR GetPassword () const {return m_password;}  
};

//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
struct HttpRequest
{
private:
    Utf8String m_url;
    Utf8String m_proxyUrl;
    bmap<Utf8String, Utf8String> m_header;
    Utf8String m_certificateAuthoritiesFileUrl;

    Credentials m_credentials;
    Credentials m_proxyCredentials;
    uint32_t    m_timeOutMs;            // in milliseconds.
    bool        m_connectOnly;

public:
    HttpRequest(Utf8CP url) : m_url(url), m_connectOnly(false), m_timeOutMs(0){}
    HttpRequest(Utf8CP url, bmap<Utf8String, Utf8String> const& header) : m_url(url), m_header(header), m_connectOnly(false), m_timeOutMs(0){}

    Utf8StringCR        GetUrl() const {return m_url;}
    void                SetUrl(Utf8StringCR url) {m_url = url;}

    bmap<Utf8String, Utf8String> const& GetHeader() const {return m_header;}
    void                                SetHeader(bmap<Utf8String, Utf8String> const& header) {m_header = header;}

    Credentials const&  GetCredentials() const {return m_credentials;}
    void                SetCredentials(Credentials const& credentials) {m_credentials = credentials;}

    Credentials const&  GetProxyCredentials() const {return m_proxyCredentials;}
    void                SetProxyCredentials(Credentials const& credentials) {m_proxyCredentials = credentials;}
    
    Utf8StringCR        GetProxyUrl() const { return m_proxyUrl; }
    void                SetProxyUrl(Utf8StringCR proxyUrl) { m_proxyUrl = proxyUrl; }

    Utf8StringCR        GetCertificateAuthoritiesFileUrl() const { return m_certificateAuthoritiesFileUrl; }
    void                SetCertificateAuthoritiesFileUrl(Utf8StringCR certificateAuthoritiesFileUrl) { m_certificateAuthoritiesFileUrl = certificateAuthoritiesFileUrl; }
    
    uint32_t            GetTimeoutMs() const {return m_timeOutMs;}
    void                SetTimeoutMs(uint32_t timeOutInMs) {m_timeOutMs = timeOutInMs;}

    bool                GetConnectOnly() const {return m_connectOnly;}
    void                SetConnectOnly(bool connect) {m_connectOnly= connect;}
};

//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.St-Pierre   10/2017
//----------------------------------------------------------------------------------------
void SetProxyInfo(HttpRequest& request);

//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.St-Pierre   10/2017
//----------------------------------------------------------------------------------------
void SetCertificateAuthoritiesInfo(HttpRequest& request);

//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
struct HttpResponse : RefCountedBase
{
    friend struct HttpSession;
public:
    // According to Http/1.1 protocol header fields are case insensitive. 
    struct CompareILess : public std::binary_function<Utf8String, Utf8String, bool> 
        {
        bool operator()(const Utf8String &lhs, const Utf8String &rhs) const {return lhs.CompareToI(rhs) < 0;}
        };

    typedef bmap<Utf8String, Utf8String, CompareILess> HeaderMap;
private:
    HttpResponseStatus  m_status;
    HeaderMap           m_header;
    bvector<Byte>       m_body;
private:
    HttpResponse(HttpResponseStatus status, HeaderMap&& header, bvector<Byte>&& body) 
        : m_status(status), m_header(std::move(header)), m_body(std::move(body)) 
        {}
public:
    HttpResponseStatus GetStatus() const;
    HeaderMap const& GetHeader() const;
    bvector<Byte> const& GetBody() const;
    bvector<Byte>&       GetBodyR();
};
typedef RefCountedPtr<HttpResponse> HttpResponsePtr;

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            05/2015
//=======================================================================================
struct IHttpRequestCancellationToken
{
    virtual bool _ShouldCancelHttpRequest() const = 0;
    virtual ~IHttpRequestCancellationToken() {}    
};

//----------------------------------------------------------------------------------------
// Wrap a CURL handle that holds live connections, session ID cache, DNS cache, cookies...
// Can be reuse for multiple request but NOT by multiple threads at the same time.
// Because HttpSession need to initializes CURL it should not be created during static initialization.
// See curl_global_init
// @bsiclass                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
struct HttpSession
    {
public:
    HttpSession();
    ~HttpSession();

    //! Block until a response or timeout.
    HttpRequestStatus Request(HttpResponsePtr& response, HttpRequest const& request, IHttpRequestCancellationToken const* cancellationToken = nullptr);

private:
    HttpRequestStatus InternalRequest(HttpResponsePtr& response, HttpRequest const& request, IHttpRequestCancellationToken const* cancellationToken);

    // disable methods
    HttpSession(const HttpSession& object) = delete;
    HttpSession& operator=(const HttpSession&) = delete;
   
    CURL* m_curl;        // Curl session where the connection settings/authentication... are kept alive.
    };


END_IMAGEPP_NAMESPACE
