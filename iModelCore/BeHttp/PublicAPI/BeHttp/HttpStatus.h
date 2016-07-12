/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpStatus.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

enum class HttpStatus
{
    // Used in error handling, it is not an actual http status code.
    None                            = -1,
    
    // 1xx Informational
    Continue                        = 100,
    SwitchingProtocols              = 101,
    Processing                      = 102,
    
    // 2xx Success
    OK                              = 200,
    Created                         = 201,
    Accepted                        = 202,
    NonAuthoritativeInformation     = 203,
    NoContent                       = 204,
    ResetContent                    = 205,
    PartialContent                  = 206,
    MultiStatus                     = 207,
    AlreadyReported                 = 208,
    IMUsed                          = 226,
    
    // 3xx Redirection
    MultipleChoices                 = 300,
    MovedPermanently                = 301,
    Found                           = 302,
    SeeOther                        = 303,
    NotModified                     = 304,
    UseProxy                        = 305,
    TemporaryRedirect               = 307,
    ResumeIncomplete                = 308,
    PermanentRedirect               = 308,
    
    // 4xx Client Error
    BadRequest                      = 400,
    Unauthorized                    = 401,
    PaymentRequired                 = 402,
    Forbidden                       = 403,
    NotFound                        = 404,
    MethodNotAllowed                = 405,
    NotAcceptable                   = 406,
    ProxyAuthenticationRequired     = 407,
    ReqestTimeout                   = 408,
    Conflict                        = 409,
    Gone                            = 410,
    LengthRequired                  = 411,
    PreconditionFailed              = 412,
    RequestEntityTooLarge           = 413,
    RequestUriTooLong               = 414,
    UnsupportedMediaType            = 415,
    RequestRangeNotSatisfiable      = 416,
    ExpectationFailed               = 417,
    InsufficientSpaceOnResource     = 419,
    MethodFailure                   = 420,
    DestinationLocked               = 421,
    UnprocessableEntity             = 422,
    Locked                          = 423,
    FailedDependency                = 424,
    UpgradeRequired                 = 426,
    TooManyRequests                 = 429,
    
    // 5xx Server Error
    InternalServerError             = 500,
    NotImplemented                  = 501,
    BadGateway                      = 502,
    ServiceUnavailable              = 503,
    GatewayTimeout                  = 504,
    HttpVersionNotSupported         = 505,
    VariantAlsoNegotiates           = 506,
    InsufficientStorage             = 507,
    LoopDetected                    = 508,
    NotExtended                     = 510
};

END_BENTLEY_HTTP_NAMESPACE
