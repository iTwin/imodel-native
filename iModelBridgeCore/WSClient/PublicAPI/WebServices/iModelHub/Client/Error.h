/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/Error.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSClient.h>
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
struct DgnDbServerError : public Tasks::AsyncError
{
public:
    enum class Id
        {
        Unknown = -1,
        //DgnDbServer Errors
        MissingRequiredProperties = 1,
        InvalidPropertiesValues,
        UserDoesNotHavePermission,
        InvalidBriefcase,
        BriefcaseDoesNotBelongToUser,
        AnotherUserPushing,
        RevisionAlreadyExists,
        RevisionDoesNotExist,
        FileIsNotUploaded,
        RevisionExistsButNoBackingFile,
        RepositoryIsNotInitialized,
        RevisionPointsToBadBIM,
        BIMCSOperationFailed,
        PullIsRequired,
        MaximumNumberOfBriefcasesPerUser,
        MaximumNumberOfBriefcasesPerUserPerMinute,
        DatabaseTemporarilyLocked,
        RepositoryAlreadyExists,
        RepositoryDoesNotExist,
        LockDoesNotExist,
        LocksExist,
        LockOwnedByAnotherBriefcase,
        UserAlreadyExists,
        UserDoesNotExist,
        CodeStateInvalid,
        CodeStateRevisionDenied,
        CodeReservedByAnotherBriefcase,
        CodeAlreadyExists,
        CodeDoesNotExist,
        CodesExist,
        FileDoesNotExist,
        RepositoryIsLocked,

        //Long Running Processes Errors
        FileIsNotYetInitialized,
        FileIsOutdated,
        FileHasDifferentId,
        FileInitializationFailed,

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

        //DgnDbServer Client API Errors
        NoRepositoriesFound,
        FileIsNotBriefcase,
        CredentialsNotSet,
        FileNotFound,
        FileAlreadyExists,
        DgnDbServerClientNotInitialized,
        InvalidServerURL,
        InvalidRepositoryName,
        InvalidRepositoryId,
        InvalidRepositoryConnection,
        InvalidRevision,
        BriefcaseIsReadOnly,
        TrackingNotEnabled,

        //Revision Manager Errors
        MergeError,
        RevisionManagerError,

        //Event Errors
        NoEventsFound,
        NoSubscriptionFound,
        NoSASFound,
        NotSubscribedToEventService,
        EventServiceSubscribingError,
        EventCallbackNotFound,
        EventCallbackAlreadySubscribed,
        EventCallbackNotSpecified,
			
        AzureError,
        DgnDbError
        };

private:
    DgnDbServerError::Id m_id;
    std::shared_ptr<WebServices::WSError> m_wsError;
    bool RequiresExtendedData(DgnDbServerError::Id id);
    DgnDbServerError::Id ErrorIdFromString(Utf8StringCR errorIdString);
    DgnDbServerError::Id ErrorIdFromWSError(WebServices::WSErrorCR error);
    Utf8StringCR GetDefaultMessage(DgnDbServerError::Id id);

public:
    DGNDBSERVERCLIENT_EXPORT DgnDbServerError();
    DGNDBSERVERCLIENT_EXPORT DgnDbServerError(DgnDbServerError::Id id);
    DgnDbServerError(DgnDbServerError::Id id, Utf8StringCR message) {m_id = id; m_message = message;}
    DGNDBSERVERCLIENT_EXPORT DgnDbServerError(Dgn::DgnDbCR db, BeSQLite::DbResult result);
    DGNDBSERVERCLIENT_EXPORT DgnDbServerError(Dgn::DgnDbPtr db, BeSQLite::DbResult result);
    DGNDBSERVERCLIENT_EXPORT DgnDbServerError(WebServices::WSErrorCR error);
    DGNDBSERVERCLIENT_EXPORT DgnDbServerError(Dgn::RevisionStatus const& status);

    DGNDBSERVERCLIENT_EXPORT DgnDbServerError(Http::HttpErrorCR error);

    JsonValueCR GetExtendedData() const {return m_wsError ? m_wsError->GetData() : Json::Value::GetNull();}
    DgnDbServerError::Id GetId() const {return m_id;}
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
