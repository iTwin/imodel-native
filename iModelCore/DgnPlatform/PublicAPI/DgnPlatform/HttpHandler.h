/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/HttpHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct HttpHandler;

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
enum class HttpRequestStatus
    {
    UnknownError,
    NoConnection,
    CouldNotResolveHost,
    CouldNotResolveProxy,
    Aborted,
    Success,
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
enum class HttpResponseStatus : long
    {
    Unknown = -1,

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

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct HttpRequest
{
private:
    Utf8String m_url;
    bmap<Utf8String, Utf8String> m_header;
public:
    HttpRequest(Utf8CP url, bmap<Utf8String, Utf8String> const& header) : m_url(url), m_header(header) {}
    Utf8String GetUrl() const {return m_url.c_str();}
    void SetUrl(Utf8StringCR url) {m_url = url;}
    bmap<Utf8String, Utf8String> const& GetHeader() const {return m_header;}
    void SetHeader(bmap<Utf8String, Utf8String> const& header) {m_header = header;}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct HttpResponse : RefCountedBase
{
    friend struct HttpHandler;
private:
    HttpResponseStatus m_status;
    bmap<Utf8String, Utf8String> m_header;
    bvector<Byte> m_body;
private:
    HttpResponse(HttpResponseStatus status, bmap<Utf8String, Utf8String>&& header, bvector<Byte>&& body) 
        : m_status(status), m_header(std::move(header)), m_body(std::move(body)) 
        {}
public:
    DGNPLATFORM_EXPORT HttpResponseStatus GetStatus() const;
    DGNPLATFORM_EXPORT bmap<Utf8String, Utf8String> const& GetHeader() const;
    DGNPLATFORM_EXPORT bvector<Byte> const& GetBody() const;
};
typedef RefCountedPtr<HttpResponse> HttpResponsePtr;

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            05/2015
//=======================================================================================
struct IHttpRequestCancellationToken
{
friend struct HttpHandler;
protected:
    virtual ~IHttpRequestCancellationToken() {}
    virtual bool _ShouldCancelHttpRequest() const = 0;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct HttpHandler : NonCopyableClass
{
private:
    static HttpHandler* s_instance;
    HttpHandler();

    static size_t HttpBodyParser(void* ptr, size_t size, size_t nmemb, void* userp);
    static size_t HttpHeaderParser(char* buffer, size_t size, size_t nItems, void* userData);
    static int HttpProgressCallback(void* clientp, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow);

public:
    DGNPLATFORM_EXPORT static HttpHandler& Instance();
    DGNPLATFORM_EXPORT HttpRequestStatus Request(HttpResponsePtr& response, HttpRequest const& request, IHttpRequestCancellationToken const* cancellationToken = nullptr);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
