/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSError.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iostream>
#include <Bentley/bmap.h>
#include <Bentley/Tasks/AsyncError.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <BeHttp/HttpError.h>
#include <BeHttp/HttpResponse.h>
#include <WebServices/Client/WebServicesClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS
USING_NAMESPACE_BENTLEY_HTTP

struct WSError;
typedef WSError& WSErrorR;
typedef const WSError& WSErrorCR;
typedef std::shared_ptr<const Json::Value> JsonValueCPtr;

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

            // See error id for more information
            ReceivedError
            };

        enum class Id
            {
            // Server did not return enough information or not WSError::Status::ReceivedError
            Unknown,

            // Server error ids
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
            Conflict
            };

    private:
        Status m_status = Status::None;
        Id m_id = Id::Unknown;
        JsonValueCPtr       m_data;

    private:
        static bool IsValidErrorJson(JsonValueCR jsonError);
        static Id ErrorIdFromString(Utf8StringCR errorIdString);
        static Utf8String FormatDescription(Utf8StringCR errorMessage, Utf8StringCR errorDescription);

        BentleyStatus ParseBody(HttpResponseCR httpResponse);
        BentleyStatus ParseJsonError(HttpResponseCR httpResponse);
        BentleyStatus ParseXmlError(HttpResponseCR httpResponse);

        void SetStatusServerNotSupported();
        void SetStatusReceivedError
            (
            HttpErrorCR httpError,
            Id errorId,
            Utf8StringCR errorMessage,
            Utf8StringCR errorDescription,
            JsonValueCPtr errorData = nullptr
            );

    public:
        WSCLIENT_EXPORT WSError();
        //! Handle supported server response
        WSCLIENT_EXPORT WSError(HttpResponseCR httpResponse);
        //! Handle generic HttpError, unknow error will map to Id::Unknown
        WSCLIENT_EXPORT WSError(HttpErrorCR httpError);
        //! Do not use in production code, this is for testing purposes only
        WSCLIENT_EXPORT WSError(Id errorId);

        WSCLIENT_EXPORT static WSError CreateServerNotSupportedError();
        WSCLIENT_EXPORT static WSError CreateFunctionalityNotSupportedError();

        WSCLIENT_EXPORT Status       GetStatus() const;
        WSCLIENT_EXPORT Id           GetId() const;
        WSCLIENT_EXPORT JsonValueCR  GetData() const;
        WSCLIENT_EXPORT Utf8StringCR GetDisplayMessage() const;
        WSCLIENT_EXPORT Utf8StringCR GetDisplayDescription() const;
    };

typedef WSError& WSErrorR;
typedef const WSError& WSErrorCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
