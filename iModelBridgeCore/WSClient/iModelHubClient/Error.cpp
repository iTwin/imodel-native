/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Error.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        map["BIMCS.MissingRequiredProperties"]                 = Id::MissingRequiredProperties;
        map["BIMCS.InvalidPropertiesValues"]                   = Id::InvalidPropertiesValues;
        map["BIMCS.UserDoesNotHavePermission"]                 = Id::UserDoesNotHavePermission;
        map["BIMCS.InvalidBriefcase"]                          = Id::InvalidBriefcase;
        map["BIMCS.BriefcaseDoesNotExist"]                     = Id::BriefcaseDoesNotExist;
        map["BIMCS.BriefcaseDoesNotBelongToUser"]              = Id::BriefcaseDoesNotBelongToUser;
        map["BIMCS.AnotherUserPushing"]                        = Id::AnotherUserPushing;
        map["BIMCS.ChangeSetAlreadyExists"]                    = Id::ChangeSetAlreadyExists;
        map["BIMCS.UserAlreadyExists"]                         = Id::UserAlreadyExists;
        map["BIMCS.UserDoesNotExist"]                          = Id::UserDoesNotExist;
        map["BIMCS.ChangeSetDoesNotExist"]                     = Id::ChangeSetDoesNotExist;
        map["BIMCS.FileIsNotUploaded"]                         = Id::FileIsNotUploaded;
        map["BIMCS.iModelIsNotInitialized"]                    = Id::iModelIsNotInitialized;
        map["BIMCS.ChangeSetPointsToBadSeed"]                  = Id::ChangeSetPointsToBadSeed;
        map["BIMCS.iModelHubOperationFailed"]                  = Id::iModelHubOperationFailed;
        map["BIMCS.PullIsRequired"]                            = Id::PullIsRequired;
        map["BIMCS.MaximumNumberOfBriefcasesPerUser"]          = Id::MaximumNumberOfBriefcasesPerUser;
        map["BIMCS.MaximumNumberOfBriefcasesPerUserPerMinute"] = Id::MaximumNumberOfBriefcasesPerUserPerMinute;
        map["BIMCS.DatabaseTemporarilyLocked"]                 = Id::DatabaseTemporarilyLocked;
        map["BIMCS.iModelIsLocked"]                            = Id::iModelIsLocked;
        map["BIMCS.CodesExist"]                                = Id::CodesExist;
        map["BIMCS.LocksExist"]                                = Id::LocksExist;
        map["BIMCS.iModelAlreadyExists"]                       = Id::iModelAlreadyExists;
        map["BIMCS.iModelDoesNotExist"]                        = Id::iModelDoesNotExist;
        map["BIMCS.FileDoesNotExist"]                          = Id::FileDoesNotExist;
        map["BIMCS.FileAlreadyExists"]                         = Id::FileAlreadyExists;
        map["BIMCS.LockDoesNotExist"]                          = Id::LockDoesNotExist;
        map["BIMCS.LockOwnedByAnotherBriefcase"]               = Id::LockOwnedByAnotherBriefcase;
        map["BIMCS.CodeStateInvalid"]                          = Id::CodeStateInvalid;
        map["BIMCS.CodeReservedByAnotherBriefcase"]            = Id::CodeReservedByAnotherBriefcase;
        map["BIMCS.CodeDoesNotExist"]                          = Id::CodeDoesNotExist;
        map["BIMCS.EventTypeDoesNotExist"]                     = Id::EventTypeDoesNotExist;
        map["BIMCS.EventSubscriptionDoesNotExist"]             = Id::EventSubscriptionDoesNotExist;
        map["BIMCS.EventSubscriptionAlreadyExists"]            = Id::EventSubscriptionAlreadyExists;
        map["BIMCS.ProjectAssociationIsNotEnabled"]            = Id::ProjectAssociationIsNotEnabled;
        map["BIMCS.ProjectIdIsNotSpecified"]                   = Id::ProjectIdIsNotSpecified;
        map["BIMCS.FailedToGetProjectPermissions"]             = Id::FailedToGetProjectPermissions;
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
    if (customId.StartsWith ("BIMCS."))
        {
        m_id = ErrorIdFromString(customId);
        if (RequiresExtendedData(m_id))
            m_wsError = std::make_shared<WSError>(error);
        }
    else
        {
        m_id = ErrorIdFromWSError(error.GetId());
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
    if (RevisionStatus::MergeError == status)
        {
        m_id = Id::MergeError;
        m_message = GetDefaultMessage(Id::MergeError);
        }
    else
        {
        m_id = Id::RevisionManagerError;
        m_message = GetDefaultMessage(Id::RevisionManagerError);
        }
    }

Error::Error(HttpErrorCR error)
    {
    m_id = Id::AzureError;
    m_message = error.AsyncError::GetMessage();
    m_description = error.AsyncError::GetDescription();
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
        map[Id::FileHasDifferentId] = ErrorLocalizedString(MESSAGE_FileHasDifferentId);
        map[Id::FileInitializationFailed] = ErrorLocalizedString(MESSAGE_FileInitializationFailed);

        map[Id::BriefcaseIsReadOnly] = ErrorLocalizedString(MESSAGE_BriefcaseIsReadOnly);
        map[Id::FileNotFound] = ErrorLocalizedString(MESSAGE_FileNotFound);
        map[Id::PullIsRequired] = ErrorLocalizedString(MESSAGE_PullIsRequired);
        map[Id::TrackingNotEnabled] = ErrorLocalizedString(MESSAGE_TrackingNotEnabled);
        map[Id::FileAlreadyExists] = ErrorLocalizedString(MESSAGE_FileAlreadyExists);
        map[Id::FileIsNotBriefcase] = ErrorLocalizedString(MESSAGE_FileIsNotBriefcase);

        map[Id::MergeError] = ErrorLocalizedString(MESSAGE_MergeError);
        map[Id::RevisionManagerError] = ErrorLocalizedString(MESSAGE_RevisionManagerError);
        map[Id::ChangeSetDoesNotExist] = ErrorLocalizedString(MESSAGE_ChangeSetDoesNotExist);

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
