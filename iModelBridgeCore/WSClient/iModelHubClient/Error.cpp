/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Error.h>
#include "Error.xliff.h"

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_WEBSERVICES

Error::Id Error::ErrorIdFromString(Utf8StringCR errorIdString)
    {
    static bmap<Utf8String, Error::Id> map;
    if (map.empty())
        {
        map["iModelHub.MissingRequiredProperties"]                  = Id::MissingRequiredProperties;
        map["iModelHub.InvalidPropertiesValues"]                    = Id::InvalidPropertiesValues;
        map["iModelHub.UserDoesNotHavePermission"]                  = Id::UserDoesNotHavePermission;
        map["iModelHub.UserDoesNotHaveAccess"]                      = Id::UserDoesNotHaveAccess;
        map["iModelHub.InvalidBriefcase"]                           = Id::InvalidBriefcase;
        map["iModelHub.BriefcaseDoesNotExist"]                      = Id::BriefcaseDoesNotExist;
        map["iModelHub.BriefcaseDoesNotBelongToUser"]               = Id::BriefcaseDoesNotBelongToUser;
        map["iModelHub.AnotherUserPushing"]                         = Id::AnotherUserPushing;
        map["iModelHub.ChangeSetAlreadyExists"]                     = Id::ChangeSetAlreadyExists;
        map["iModelHub.ChangeSetDoesNotExist"]                      = Id::ChangeSetDoesNotExist;
        map["iModelHub.FileIsNotUploaded"]                          = Id::FileIsNotUploaded;
        map["iModelHub.iModelIsNotInitialized"]                     = Id::iModelIsNotInitialized;
        map["iModelHub.ChangeSetPointsToBadSeed"]                   = Id::ChangeSetPointsToBadSeed;
        map["iModelHub.OperationFailed"]                            = Id::OperationFailed;
        map["iModelHub.PullIsRequired"]                             = Id::PullIsRequired;
        map["iModelHub.MaximumNumberOfBriefcasesPerUser"]           = Id::MaximumNumberOfBriefcasesPerUser;
        map["iModelHub.MaximumNumberOfBriefcasesPerUserPerMinute"]  = Id::MaximumNumberOfBriefcasesPerUserPerMinute;
        map["iModelHub.DatabaseTemporarilyLocked"]                  = Id::DatabaseTemporarilyLocked;
        map["iModelHub.iModelIsLocked"]                             = Id::iModelIsLocked;
        map["iModelHub.CodesExist"]                                 = Id::CodesExist;
        map["iModelHub.LocksExist"]                                 = Id::LocksExist;
        map["iModelHub.iModelAlreadyExists"]                        = Id::iModelAlreadyExists;
        map["iModelHub.iModelDoesNotExist"]                         = Id::iModelDoesNotExist;
        map["iModelHub.FileDoesNotExist"]                           = Id::FileDoesNotExist;
        map["iModelHub.FileAlreadyExists"]                          = Id::FileAlreadyExists;
        map["iModelHub.LockDoesNotExist"]                           = Id::LockDoesNotExist;
        map["iModelHub.LockOwnedByAnotherBriefcase"]                = Id::LockOwnedByAnotherBriefcase;
        map["iModelHub.CodeStateInvalid"]                           = Id::CodeStateInvalid;
        map["iModelHub.CodeReservedByAnotherBriefcase"]             = Id::CodeReservedByAnotherBriefcase;
        map["iModelHub.CodeDoesNotExist"]                           = Id::CodeDoesNotExist;
        map["iModelHub.EventTypeDoesNotExist"]                      = Id::EventTypeDoesNotExist;
        map["iModelHub.EventSubscriptionDoesNotExist"]              = Id::EventSubscriptionDoesNotExist;
        map["iModelHub.EventSubscriptionAlreadyExists"]             = Id::EventSubscriptionAlreadyExists;
        map["iModelHub.ProjectIdIsNotSpecified"]                    = Id::ProjectIdIsNotSpecified;
        map["iModelHub.FailedToGetProjectPermissions"]              = Id::FailedToGetProjectPermissions;
        map["iModelHub.FailedToGetProjectMembers"]                  = Id::FailedToGetProjectMembers;
        map["iModelHub.ChangeSetAlreadyHasVersion"]                 = Id::ChangeSetAlreadyHasVersion;
        map["iModelHub.VersionAlreadyExists"]                       = Id::VersionAlreadyExists;
        map["iModelHub.JobSchedulingFailed"]                        = Id::JobSchedulingFailed;
        map["iModelHub.ConflictsAggregate"]                         = Id::ConflictsAggregate;
        map["iModelHub.FailedToGetProjectById"]                     = Id::FailedToGetProjectById;
        
        map["iModelHub.DatabaseOperationFailed"]                    = Id::DatabaseOperationFailed;
        map["iModelHub.SQLiteOperationFailed"]                      = Id::SQLiteOperationFailed;
        }

    auto it = map.find(errorIdString);
    if (it != map.end())
        {
        return it->second;
        }

    return Id::Unknown;
    }

Error::Id Error::ErrorIdFromWSError(WebServices::WSErrorCR error)
    {
    if (WSError::Status::ReceivedError == error.GetStatus())
        switch (error.GetId())
            {
            case WSError::Id::LoginFailed:
                return Id::LoginFailed;
            case WSError::Id::SslRequired:
                return Id::SslRequired;
            case WSError::Id::NotEnoughRights:
                return Id::NotEnoughRights;
            case WSError::Id::NoServerLicense:
                return Id::NoServerLicense;
            case WSError::Id::NoClientLicense:
                return Id::NoClientLicense;
            case WSError::Id::TooManyBadLoginAttempts:
                return Id::TooManyBadLoginAttempts;
            case WSError::Id::ServerError:
                return Id::InternalServerError;
            default:
                return Id::WebServicesError;
            }
    else if (WSError::Status::Canceled == error.GetStatus())
        return Id::Canceled;
    else if (WSError::Status::ConnectionError == error.GetStatus())
        return Id::ConnectionError;

    return Id::WebServicesError;
    }

bool Error::RequiresExtendedData(Id id)
    {
    switch (id)
        {
        case Id::LockOwnedByAnotherBriefcase:
        case Id::iModelAlreadyExists:
        case Id::FileAlreadyExists:
        case Id::PullIsRequired:
        case Id::CodeStateInvalid:
        case Id::CodeReservedByAnotherBriefcase:
        case Id::ConflictsAggregate:
            return true;
        default:
            return false;
        }
    }

Error::Error()
    {
    m_id = Id::Unknown;
    m_message = GetDefaultMessage(Id::Unknown);
    }

Error::Error(WebServices::WSErrorCR error)
    {
    m_message = error.GetMessage();
    m_description = error.GetDescription();
    Utf8StringCR customId = error.GetData()["errorId"].asString();
    if (customId.StartsWith("iModelHub."))
        {
        m_id = ErrorIdFromString(customId);
        if (RequiresExtendedData(m_id))
            m_wsError = std::make_shared<WSError>(error);
        }
    else
        {
        m_id = ErrorIdFromWSError(error);
        }
    }

Error::Error(DgnDbCR db, BeSQLite::DbResult result)
    {
    m_id = Id::DgnDbError;
    m_message = db.GetLastError(&result);
    }

Error::Error(DgnDbPtr db, BeSQLite::DbResult result)
    {
    m_id = Id::DgnDbError;
    if (db.IsValid())
        {
        m_message = db->GetLastError(&result);
        }
    }

Error::Error(RevisionStatus const& status)
    {
    if (RevisionStatus::ApplyError == status)
        {
        m_id = Id::ApplyError;
        m_message = GetDefaultMessage(Id::ApplyError);
        }
    else if (RevisionStatus::MergeSchemaChangesOnOpen == status)
        {
        m_id = Id::MergeSchemaChangesOnOpen;
        m_message = GetDefaultMessage(Id::MergeSchemaChangesOnOpen);
        }
    else if (RevisionStatus::ReverseOrReinstateSchemaChangesOnOpen == status)
        {
        m_id = Id::ReverseOrReinstateSchemaChangesOnOpen;
        m_message = GetDefaultMessage(Id::ReverseOrReinstateSchemaChangesOnOpen);
        }
    else
        {
        m_id = Id::ChangeSetManagerError;
        m_message = GetDefaultMessage(Id::ChangeSetManagerError);
        }
    }

Error::Error(AzureErrorCR azureError)
    {
    m_id = Id::AzureError;
    m_message = azureError.GetCode();
    m_description = azureError.GetMessage();
    }

Utf8StringCR Error::GetDefaultMessage(Error::Id id)
    {
    static bmap<Error::Id, Utf8String> map;
    if (map.empty())
        {
        map[Id::CredentialsNotSet] = ErrorLocalizedString(MESSAGE_CredentialsNotSet);
        map[Id::InvalidServerURL] = ErrorLocalizedString(MESSAGE_InvalidServerURL);
        map[Id::InvalidiModelName] = ErrorLocalizedString(MESSAGE_InvalidiModelName);
        map[Id::InvalidiModelId] = ErrorLocalizedString(MESSAGE_InvalidiModelId);
        map[Id::InvalidiModelConnection] = ErrorLocalizedString(MESSAGE_InvalidiModelConnection);
        map[Id::InvalidChangeSet] = ErrorLocalizedString(MESSAGE_InvalidChangeSet);

        map[Id::UserDoesNotExist] = ErrorLocalizedString(MESSAGE_UserDoesNotExist);

        map[Id::iModelIsNotInitialized] = ErrorLocalizedString(MESSAGE_iModelIsNotInitialized);

        map[Id::FileIsNotYetInitialized] = ErrorLocalizedString(MESSAGE_FileIsNotYetInitialized);
        map[Id::FileIsOutdated] = ErrorLocalizedString(MESSAGE_FileIsOutdated);
        map[Id::FileCodeTooLong] = ErrorLocalizedString(MESSAGE_FileCodeTooLong);
        map[Id::FileInitializationFailed] = ErrorLocalizedString(MESSAGE_FileInitializationFailed);

        map[Id::BriefcaseIsReadOnly] = ErrorLocalizedString(MESSAGE_BriefcaseIsReadOnly);
        map[Id::FileNotFound] = ErrorLocalizedString(MESSAGE_FileNotFound);
        map[Id::PullIsRequired] = ErrorLocalizedString(MESSAGE_PullIsRequired);
        map[Id::TrackingNotEnabled] = ErrorLocalizedString(MESSAGE_TrackingNotEnabled);
        map[Id::FileAlreadyExists] = ErrorLocalizedString(MESSAGE_FileAlreadyExists);

        map[Id::FileIsNotBriefcase] = ErrorLocalizedString(MESSAGE_FileIsNotBriefcase);

        map[Id::ApplyError] = ErrorLocalizedString(MESSAGE_ApplyError);
        map[Id::ChangeSetManagerError] = ErrorLocalizedString(MESSAGE_ChangeSetManagerError);
        map[Id::MergeSchemaChangesOnOpen] = ErrorLocalizedString(MESSAGE_MergeSchemaChangesOnOpen);
        map[Id::ReverseOrReinstateSchemaChangesOnOpen] = ErrorLocalizedString(MESSAGE_ReverseOrReinstateSchemaChangesOnOpen);
        map[Id::ChangeSetDoesNotExist] = ErrorLocalizedString(MESSAGE_ChangeSetDoesNotExist);
        map[Id::ConflictsAggregate] = ErrorLocalizedString(MESSAGE_ConflictsAggregate);

        map[Id::EventCallbackAlreadySubscribed] = ErrorLocalizedString(MESSAGE_EventCallbackAlreadySubscribed);
        map[Id::EventServiceSubscribingError] = ErrorLocalizedString(MESSAGE_EventServiceSubscribingError);
        map[Id::EventCallbackNotFound] = ErrorLocalizedString(MESSAGE_EventCallbackNotFound);
        map[Id::EventCallbackNotSpecified] = ErrorLocalizedString(MESSAGE_EventCallbackNotSpecified);
        map[Id::NoEventsFound] = ErrorLocalizedString(MESSAGE_NoEventsFound);
        map[Id::NotSubscribedToEventService] = ErrorLocalizedString(MESSAGE_NotSubscribedToEventService);
        map[Id::NoSASFound] = ErrorLocalizedString(MESSAGE_NoSASFound);
        map[Id::NoSubscriptionFound] = ErrorLocalizedString(MESSAGE_NoSubscriptionFound);

        map[Id::Unknown] = ErrorLocalizedString(MESSAGE_Unknown);
        }

    auto it = map.find(id);
    if (it != map.end())
        {
        return it->second;
        }

    return map[Id::Unknown];
    }

Error::Error(Error::Id id)
    {
    m_id = id;
    m_message = GetDefaultMessage(id);
    }

END_BENTLEY_IMODELHUB_NAMESPACE