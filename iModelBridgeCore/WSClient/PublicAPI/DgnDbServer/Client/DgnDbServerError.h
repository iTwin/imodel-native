/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerError.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSClient.h>
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
struct DgnDbServerError : public DgnClientFx::Utils::AsyncError
    {
    public:
        enum class Id
            {
            Unknown,
            //DgnDbServer Errors
            MissingRequiredProperties,
            InvalidBriefcase,
            BriefcaseDoesNotBelongToUser,
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
        std::shared_ptr<WebServices::WSError> m_wsError;
        bool RequiresExtendedData(Id id);
        Id ErrorIdFromString(Utf8StringCR errorIdString);
        Id ErrorIdFromWSError(WebServices::WSErrorCR error);

    public:
        DGNDBSERVERCLIENT_EXPORT DgnDbServerError();
        DGNDBSERVERCLIENT_EXPORT DgnDbServerError(Utf8CP message, Utf8CP description = nullptr);
        DGNDBSERVERCLIENT_EXPORT DgnDbServerError(WebServices::WSErrorCR error);
        DGNDBSERVERCLIENT_EXPORT DgnDbServerError(Dgn::RevisionStatus const& status);

        JsonValueCR GetExtendedData();
        DGNDBSERVERCLIENT_EXPORT Id GetId();
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE
