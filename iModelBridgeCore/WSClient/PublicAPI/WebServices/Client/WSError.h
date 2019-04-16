/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <BeHttp/HttpError.h>
#include <BeHttp/HttpResponse.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/bmap.h>
#include <Bentley/Tasks/AsyncError.h>
#include <iostream>
#include <BeRapidJson/BeRapidJson.h>
#include <WebServices/Azure/AzureError.h>

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

            // See WSError::Id for more information
            ReceivedError,

            // Not actual Status.
            _Last
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
            ProxyAuthenticationRequired,

            // Not actual Id.
            _Last
            };

    private:
        Status m_status = Status::None;
        Id m_id = Id::Unknown;
        JsonValueCPtr m_data;

    private:
        static bool DoesStringFieldExist(JsonValueCR json, Utf8CP name);
        static bool DoesStringFieldExist(RapidJsonValueCR json, Utf8CP name);
        static Utf8CP GetOptionalString(RapidJsonValueCR json, Utf8CP name);
        static int GetOptionalInt(RapidJsonValueCR json, Utf8CP name);
        static Id ErrorIdFromString(Utf8StringCR errorIdString);
        static Id GetErrorIdFromString(Utf8StringCR errorIdString);
        static Utf8String FormatDescription(Utf8StringCR errorMessage, Utf8StringCR errorDescription);

        BentleyStatus ParseBody(Http::ResponseCR httpResponse);
        BentleyStatus ParseJsonError(Http::ResponseCR httpResponse);
        BentleyStatus ParseJsonError(JsonValueCR jsonError, HttpStatus status);
        BentleyStatus ParseJsonError(RapidJsonValueCR jsonError, HttpStatus status);
        BentleyStatus ParseXmlError(Http::ResponseCR httpResponse);
        BentleyStatus ParseXmlAzureError(Http::ResponseCR httpResponse, struct BeXmlDom& xmlDom);

        void SetStatusServerNotSupported();
        void SetStatusReceivedError
            (
            HttpErrorCR httpError,
            Id errorId,
            Utf8StringCR errorMessage,
            Utf8StringCR errorDescription,
            JsonValueCP errorData
            );

    public:
        //! Create error with WSError::Status::None
        WSCLIENT_EXPORT WSError();
        //! Handle supported server response
        WSCLIENT_EXPORT WSError(Http::ResponseCR httpResponse);
        //! Handle $changeset error. Requires httpStatusCode field or will map to not supported error.
        WSCLIENT_EXPORT WSError(JsonValueCR jsonError);
        //! Handle $changeset error. Requires httpStatusCode field or will map to not supported error.
        WSCLIENT_EXPORT WSError(RapidJsonValueCR jsonError);
        //! Handle generic HttpError, unknow error will map to Id::Unknown
        WSCLIENT_EXPORT WSError(HttpErrorCR httpError);
        //! Handle generic AzureError, unknown error will map to Id:Unknown
        WSCLIENT_EXPORT WSError(AzureErrorCR azureError);
        //! Do not use in production code, this is for testing purposes only.
        //! Create error with WSError::Status::ReceivedError and specified Id.
        WSCLIENT_EXPORT WSError(Id errorId);

        WSCLIENT_EXPORT static WSError CreateServerNotSupportedError();
        WSCLIENT_EXPORT static WSError CreateFunctionalityNotSupportedError();

        //! Get error status describing origin of error
        WSCLIENT_EXPORT Status GetStatus() const;
        //! Get error id returned from server when GetStatus() returns WSError::Status::ReceivedError
        WSCLIENT_EXPORT Id GetId() const;
        //! Get server reported data
        WSCLIENT_EXPORT JsonValueCR  GetData() const;
        //! DEPRECATED! Use GetMessage()
        WSCLIENT_EXPORT Utf8StringCR GetDisplayMessage() const;
        //! DEPRECATED! Use GetDescription()
        WSCLIENT_EXPORT Utf8StringCR GetDisplayDescription() const;
        //! Checks if instance should be available for user
        bool IsInstanceNotAvailableError() const 
            { return m_id == Id::InstanceNotFound || m_id == Id::NotEnoughRights; };
    };

typedef WSError& WSErrorR;
typedef const WSError& WSErrorCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
