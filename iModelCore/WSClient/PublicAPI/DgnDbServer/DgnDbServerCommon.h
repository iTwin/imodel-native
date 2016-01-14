/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/DgnDbServerCommon.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <WebServices/WebServices.h>
#include <WebServices/Client/WSError.h>
#include <DgnClientFx/Utils/Threading/AsyncResult.h>
#include <DgnClientFx/Utils/Threading/AsyncTask.h>

#define BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace DgnDbServer {
#define END_BENTLEY_DGNDBSERVER_NAMESPACE      } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DGNDBSERVER    using namespace BentleyApi::DgnDbServer;

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
struct DgnDbServerError : public AsyncError
    {
public:
    enum class Id
        {
        Unknown,
        //DgnDbServer Errors
        MissingRequiredProperties,
        InvalidBriefcase,
        AnotherUserPushing,
        RevisionAlreadyExists,
        RevisionDoesNotExist,
        FileIsNotUploaded,
        RevisionExistsButNoBackingFile,
        RepositoryIsNotInitialized,
        RevisionPointsToBadDgnDb,
        DgnDbServerOperationFailed,
        PullIsRequired,
        MaximumNumberOfBriefcasesPerUser,
        MaximumNumberOfBriefcasesPerUserPerMinute,
        DatabaseTemporarilyLocked,
        RepositoryAlreadyExists,
        RepositoryDoesNotExist,
        LockDoesNotExist,
        LockOwnedByAnotherBriefcase,

        //WebServices Errors
        LoginFailed,
        SslRequired,
        NotEnoughRights,
        NoServerLicense,
        NoClientLicense,
        TooManyBadLoginAttempts,
        InternalServerError,
        WebServicesError,
        ConnectionError,
        Canceled,

        //Revision Manager Errors
        MergeError,
        RevisionManagerError
        };

private:
    Id m_id;
    Id ErrorIdFromString(Utf8StringCR errorIdString);
    Id ErrorIdFromWSError(WebServices::WSErrorCR error);

public:
    DgnDbServerError();
    DgnDbServerError(Utf8CP message, Utf8CP description = nullptr);
    DgnDbServerError(WebServices::WSErrorCR error);
    DgnDbServerError(Dgn::RevisionStatus const& status);

    Id GetId();
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE


#ifdef __DgnDbServerClient_DLL_BUILD__
#define DGNDBSERVERCLIENT_EXPORT EXPORT_ATTRIBUTE
#else
#define DGNDBSERVERCLIENT_EXPORT IMPORT_ATTRIBUTE
#endif
