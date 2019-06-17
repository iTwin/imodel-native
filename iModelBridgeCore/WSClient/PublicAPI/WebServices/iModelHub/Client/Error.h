/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSClient.h>
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Azure/AzureError.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
struct Error : public Tasks::AsyncError
{
public:
    enum class Id
        {
        Unknown = -1,

        //iModel Hub Services Errors
        MissingRequiredProperties = 1,
        InvalidPropertiesValues,
        UserDoesNotHavePermission,
        UserDoesNotHaveAccess,
        InvalidBriefcase,
        BriefcaseDoesNotExist,
        BriefcaseDoesNotBelongToUser,
        AnotherUserPushing,
        ChangeSetAlreadyExists,
        ChangeSetDoesNotExist,
        FileIsNotUploaded,
        iModelIsNotInitialized,
        ChangeSetPointsToBadSeed,
        OperationFailed,
        PullIsRequired,
        MaximumNumberOfBriefcasesPerUser,
        MaximumNumberOfBriefcasesPerUserPerMinute,
        DatabaseTemporarilyLocked,
        iModelIsLocked,
        CodesExist,
        LocksExist,
        iModelAlreadyExists,
        iModelDoesNotExist,
        FileDoesNotExist,
        FileAlreadyExists,
        LockDoesNotExist,
        LockOwnedByAnotherBriefcase,
        CodeStateInvalid,
        CodeReservedByAnotherBriefcase,
        CodeDoesNotExist,
        EventTypeDoesNotExist,
        EventSubscriptionDoesNotExist,
        EventSubscriptionAlreadyExists,
        ProjectIdIsNotSpecified,
        FailedToGetProjectPermissions,
        FailedToGetProjectMembers,
        FailedToGetAssetPermissions,
        FailedToGetAssetMembers,
        ChangeSetAlreadyHasVersion,
        VersionAlreadyExists,
        JobSchedulingFailed,
        ConflictsAggregate,
        FailedToGetProjectById,

        DatabaseOperationFailed,
        SQLiteOperationFailed,

        //Long Running Processes Errors
        FileIsNotYetInitialized = 100,
        FileIsOutdated,
        FileCodeTooLong,
        FileInitializationFailed,
        FileIsBriefcase,

        //iModel Hub Client API Errors
        NoRepositoriesFound = 200,
        UserDoesNotExist,
        QueryIdsNotSpecified,
        FileIsNotBriefcase,
        CredentialsNotSet,
        FileNotFound,
        iModelHubClientNotInitialized,
        InvalidServerURL,
        InvalidiModelName,
        InvalidiModelId,
        InvalidiModelConnection,
        InvalidChangeSet,
        InvalidVersion,
        BriefcaseIsReadOnly,
        TrackingNotEnabled,

        //Event Errors
        NoEventsFound = 300,
        NoSubscriptionFound,
        NoSASFound,
        NotSubscribedToEventService,
        EventServiceSubscribingError,
        EventCallbackNotFound,
        EventCallbackAlreadySubscribed,
        EventCallbackNotSpecified,
        ExecutionTimeout,
        EventInvalid,

        //WebServices Errors
        LoginFailed = 400,
        SslRequired,
        NotEnoughRights,
        NoServerLicense,
        NoClientLicense,
        TooManyBadLoginAttempts,
        InternalServerError,
        WebServicesError,
        ConnectionError,
        Canceled,

        //ChangeSet Manager Errors
        ApplyError = 500,
        ChangeSetManagerError,
        MergeSchemaChangesOnOpen,
        ReverseOrReinstateSchemaChangesOnOpen,

        AzureError,
        DgnDbError
        };

private:
    Error::Id m_id;
    std::shared_ptr<WebServices::WSError> m_wsError;
    bool RequiresExtendedData(Error::Id id);
    Error::Id ErrorIdFromString(Utf8StringCR errorIdString);
    Error::Id ErrorIdFromWSError(WebServices::WSErrorCR error);
    Utf8StringCR GetDefaultMessage(Error::Id id);

public:
    IMODELHUBCLIENT_EXPORT Error();
    IMODELHUBCLIENT_EXPORT Error(Error::Id id);
    Error(Error::Id id, Utf8StringCR message) {m_id = id; m_message = message;}
    Error(Error::Id id, Utf8StringCR message, Utf8StringCR description) { m_id = id; m_message = message; m_description = description; }
    IMODELHUBCLIENT_EXPORT Error(Dgn::DgnDbCR db, BeSQLite::DbResult result);
    IMODELHUBCLIENT_EXPORT Error(Dgn::DgnDbPtr db, BeSQLite::DbResult result);
    IMODELHUBCLIENT_EXPORT Error(WebServices::WSErrorCR error);
    IMODELHUBCLIENT_EXPORT Error(Dgn::RevisionStatus const& status);

    IMODELHUBCLIENT_EXPORT Error(WebServices::AzureErrorCR azureError);

    JsonValueCR GetExtendedData() const {return m_wsError ? m_wsError->GetData() : Json::Value::GetNull();}
    Error::Id GetId() const {return m_id;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
