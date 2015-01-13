/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/WSError.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iostream>
#include "Bentley/bmap.h"
#include "BeJsonCpp/BeJsonUtilities.h"

#include <WebServices/Common.h>
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
        WS_EXPORT WSError ();
        WS_EXPORT WSError (MobileDgn::Utils::HttpResponseCR httpResponse);
        // For testing purposes only
        WS_EXPORT WSError (Id errorId);

        WS_EXPORT static WSError CreateServerNotSupportedError ();
        WS_EXPORT static WSError CreateFunctionalityNotSupportedError ();

        WS_EXPORT Status       GetStatus () const;
        WS_EXPORT Id           GetId () const;
        WS_EXPORT Utf8StringCR GetDisplayMessage () const;
        WS_EXPORT Utf8StringCR GetDisplayDescription () const;
    };

typedef WSError& WSErrorR;
typedef const WSError& WSErrorCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
