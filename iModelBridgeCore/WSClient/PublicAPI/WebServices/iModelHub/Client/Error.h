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
#include <WebServices/iModelHub/Common.h>

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
        InvalidBriefcase,
        BriefcaseDoesNotExist,
        BriefcaseDoesNotBelongToUser,
        AnotherUserPushing,
        ChangeSetAlreadyExists,
        ChangeSetDoesNotExist,
        FileIsNotUploaded,
        iModelIsNotInitialized,
        ChangeSetPointsToBadSeed,
        iModelHubOperationFailed,
        PullIsRequired,
        MaximumNumberOfBriefcasesPerUser,
        MaximumNumberOfBriefcasesPerUserPerMinute,
        DatabaseTemporarilyLocked,
        iModelAlreadyExists,
        iModelDoesNotExist,
        LockDoesNotExist,
        LocksExist,
        LockOwnedByAnotherBriefcase,
        UserAlreadyExists,
        UserDoesNotExist,
        CodeStateInvalid,
        CodeReservedByAnotherBriefcase,
        CodeDoesNotExist,
        CodesExist,
        FileDoesNotExist,
        iModelIsLocked,
        EventTypeDoesNotExist,
        EventSubscriptionDoesNotExist,
        EventSubscriptionAlreadyExists,
        ProjectAssociationIsNotEnabled,
        ProjectIdIsNotSpecified,
        FailedToGetProjectPermissions,
        ChangeSetAlreadyHasVersion,
        VersionAlreadyExists,

        //Long Running Processes Errors
        FileIsNotYetInitialized = 100,
        FileIsOutdated,
        FileHasDifferentId,
        FileInitializationFailed,

        //iModel Hub Client API Errors
        NoRepositoriesFound = 200,
        FileIsNotBriefcase,
        CredentialsNotSet,
        FileNotFound,
        FileAlreadyExists,
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

        //Revision Manager Errors
        MergeError = 500,
        RevisionManagerError,
        MergeSchemaChangesOnOpen,

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
    IMODELHUBCLIENT_EXPORT Error(Dgn::DgnDbCR db, BeSQLite::DbResult result);
    IMODELHUBCLIENT_EXPORT Error(Dgn::DgnDbPtr db, BeSQLite::DbResult result);
    IMODELHUBCLIENT_EXPORT Error(WebServices::WSErrorCR error);
    IMODELHUBCLIENT_EXPORT Error(Dgn::RevisionStatus const& status);

    IMODELHUBCLIENT_EXPORT Error(Http::HttpErrorCR error);

    JsonValueCR GetExtendedData() const {return m_wsError ? m_wsError->GetData() : Json::Value::GetNull();}
    Error::Id GetId() const {return m_id;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
