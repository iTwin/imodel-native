/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSError.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

struct WSError;
typedef WSError& WSErrorR;
typedef const WSError& WSErrorCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSError : public MobileDgn::Utils::AsyncError
    {
    public:
        enum class Status
            {
            None,
            Canceled,
            ServerNotSupported,

            ConnectionError,

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
        Status              m_status;
        Id                  m_id;

    private:
        static bool IsValidErrorJson (JsonValueCR jsonError);
        static Id ErrorIdFromString (Utf8StringCR errorIdString);
        static Utf8String FormatDescription (Utf8StringCR errorMessage, Utf8StringCR errorDescription);

        BentleyStatus ParseBody (MobileDgn::Utils::HttpResponseCR httpResponse);
        BentleyStatus ParseJsonError (MobileDgn::Utils::HttpResponseCR httpResponse);
        BentleyStatus ParseXmlError (MobileDgn::Utils::HttpResponseCR httpResponse);

        void SetStatusServerNotSupported ();
        void SetStatusReceivedError (MobileDgn::Utils::HttpErrorCR httpError, Id errorId, Utf8StringCR errorMessage, Utf8StringCR errorDescription);

    public:
        WSCLIENT_EXPORT WSError ();
        WSCLIENT_EXPORT WSError (MobileDgn::Utils::HttpResponseCR httpResponse);
        // Do not use in production code, this is for testing purposes only
        WSCLIENT_EXPORT WSError (Id errorId);

        WSCLIENT_EXPORT static WSError CreateServerNotSupportedError ();
        WSCLIENT_EXPORT static WSError CreateFunctionalityNotSupportedError ();

        WSCLIENT_EXPORT Status       GetStatus () const;
        WSCLIENT_EXPORT Id           GetId () const;
        WSCLIENT_EXPORT Utf8StringCR GetDisplayMessage () const;
        WSCLIENT_EXPORT Utf8StringCR GetDisplayDescription () const;
    };

typedef WSError& WSErrorR;
typedef const WSError& WSErrorCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
