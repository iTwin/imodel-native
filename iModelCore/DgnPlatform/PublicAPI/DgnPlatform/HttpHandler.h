/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/HttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct Http
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            04/2015
    //=======================================================================================
    struct Request
    {
        enum class Status
            {
            UnknownError,
            NoConnection,
            CouldNotResolveHost,
            CouldNotResolveProxy,
            Aborted,
            Success,
    };
    private:
        Utf8String m_url;
        bmap<Utf8String, Utf8String> m_header;

    public:
        Request(Utf8CP url, bmap<Utf8String, Utf8String> const& header) : m_url(url), m_header(header) {}
        Utf8String GetUrl() const {return m_url.c_str();}
        void SetUrl(Utf8StringCR url) {m_url = url;}
        bmap<Utf8String, Utf8String> const& GetHeader() const {return m_header;}
        void SetHeader(bmap<Utf8String, Utf8String> const& header) {m_header = header;}
    };


    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            04/2015
    //=======================================================================================
    struct Response
    {
        enum class Status : long
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

        Status m_status = Status::Unknown;
        bmap<Utf8String, Utf8String> m_header;
        ByteStream m_body;
    };

    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            05/2015
    //=======================================================================================
    struct CancellationToken
    {
        virtual ~CancellationToken() {}
        virtual bool _ShouldCancelHttpRequest() const = 0;
    };

    DGNPLATFORM_EXPORT static Request::Status PerformRequest(Response& response, Request const& request, CancellationToken const* cancellationToken = nullptr);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
