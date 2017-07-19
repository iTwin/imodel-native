/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSError.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iostream>
#include "Bentley/bmap.h"
#include "BeJsonCpp/BeJsonUtilities.h"

#include <WebServices/Client/WebServicesClient.h>
#include <MobileDgn/Utils/Threading/AsyncError.h>
#include <MobileDgn/Utils/Http/HttpError.h>
#include <MobileDgn/Utils/Http/HttpResponse.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

struct WSError;
typedef WSError& WSErrorR;
typedef const WSError& WSErrorCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSError : public AsyncError
    {
    public:
        enum class Status
            {
            None,
            Canceled,
            ServerNotSupported,

            // Could not connect, connection lost, timeout, etc.
            ConnectionError,

            // Could not verify HTTPS certificate
            CertificateError,

            // See WSError::Id for more information
            ReceivedError
            };

        enum class Id
            {
            // Server did not return enough information or not WSError::Status::ReceivedError
            Unknown,

            // Server returned error ids
            LoginFailed,
            SslRequired,
            NotEnoughRights,
            RepositoryNotFound,
            SchemaNotFound,
            ClassNotFound,
            PropertyNotFound,
            InstanceNotFound,
            FileNotFound,
            NotSupported,
            NoServerLicense,
            NoClientLicense,
            TooManyBadLoginAttempts,

            // Additional error ids for handling specific errors
            ServerError,
            BadRequest,
            Conflict,
            ProxyAuthenticationRequired
            };

    private:
        Status m_status = Status::None;
        Id m_id = Id::Unknown;

    private:
        static bool IsValidErrorJson(JsonValueCR jsonError);
        static Id ErrorIdFromString(Utf8StringCR errorIdString);
        static Id GetErrorIdFromString(Utf8StringCR errorIdString);
        static Utf8String FormatDescription(Utf8StringCR errorMessage, Utf8StringCR errorDescription);

        BentleyStatus ParseBody(HttpResponseCR httpResponse);
        BentleyStatus ParseJsonError(HttpResponseCR httpResponse);
        BentleyStatus ParseXmlError(HttpResponseCR httpResponse);

        void SetStatusServerNotSupported();
        void SetStatusReceivedError(HttpErrorCR httpError, Id errorId, Utf8StringCR errorMessage, Utf8StringCR errorDescription);

    public:
        //! Create error with WSError::Status::None
        WSCLIENT_EXPORT WSError();
        //! Handle supported server response
        WSCLIENT_EXPORT WSError(HttpResponseCR httpResponse);
        //! Handle generic HttpError, unknow error will map to Id::Unknown
        WSCLIENT_EXPORT WSError(HttpErrorCR httpError);
        //! Do not use in production code, this is for testing purposes only.
        //! Create error with WSError::Status::ReceivedError and specified Id.
        WSCLIENT_EXPORT WSError(Id errorId);

        WSCLIENT_EXPORT static WSError CreateServerNotSupportedError();
        WSCLIENT_EXPORT static WSError CreateFunctionalityNotSupportedError();

        //! Get error status describing origin of error
        WSCLIENT_EXPORT Status GetStatus() const;
        //! Get error id returned from server when GetStatus() returns WSError::Status::ReceivedError
        WSCLIENT_EXPORT Id GetId() const;
        //! DEPRECATED! Use GetMessage()
        WSCLIENT_EXPORT Utf8StringCR GetDisplayMessage() const;
        //! DEPRECATED! Use GetDescription()
        WSCLIENT_EXPORT Utf8StringCR GetDisplayDescription() const;
    };

typedef WSError& WSErrorR;
typedef const WSError& WSErrorCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
