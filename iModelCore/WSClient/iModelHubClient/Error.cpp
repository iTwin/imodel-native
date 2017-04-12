/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Error.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerError.h>
#include "DgnDbServerError.xliff.h"

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_WEBSERVICES

DgnDbServerError::Id DgnDbServerError::ErrorIdFromString(Utf8StringCR errorIdString)
    {
    static bmap<Utf8String, DgnDbServerError::Id> map;
    if (map.empty())
        {
        map["BIMCS.MissingRequiredProperties"]                 = Id::MissingRequiredProperties;
        map["BIMCS.InvalidPropertiesValues"]                   = Id::InvalidPropertiesValues;
        map["BIMCS.UserDoesNotHavePermission"]                 = Id::UserDoesNotHavePermission;
        map["BIMCS.InvalidBriefcase"]                          = Id::InvalidBriefcase;
        map["BIMCS.AnotherUserPushing"]                        = Id::AnotherUserPushing;
        map["BIMCS.RevisionAlreadyExists"]                     = Id::RevisionAlreadyExists;
        map["BIMCS.RevisionDoesNotExist"]                      = Id::RevisionDoesNotExist;
        map["BIMCS.FileIsNotUploaded"]                         = Id::FileIsNotUploaded;
        map["BIMCS.RevisionExistsButNoBackingFile"]            = Id::RevisionExistsButNoBackingFile;
        map["BIMCS.RepositoryIsNotInitialized"]                = Id::RepositoryIsNotInitialized;
        map["BIMCS.RevisionPointsToBadBIM"]                    = Id::RevisionPointsToBadBIM;
        map["BIMCS.OperationFailed"]                           = Id::BIMCSOperationFailed;
        map["BIMCS.PullIsRequired"]                            = Id::PullIsRequired;
        map["BIMCS.MaximumNumberOfBriefcasesPerUser"]          = Id::MaximumNumberOfBriefcasesPerUser;
        map["BIMCS.MaximumNumberOfBriefcasesPerUserPerMinute"] = Id::MaximumNumberOfBriefcasesPerUserPerMinute;
        map["BIMCS.DatabaseTemporarilyLocked"]                 = Id::DatabaseTemporarilyLocked;
        map["BIMCS.RepositoryAlreadyExists"]                   = Id::RepositoryAlreadyExists;
        map["BIMCS.RepositoryDoesNotExist"]                    = Id::RepositoryDoesNotExist;
        map["BIMCS.LockDoesNotExist"]                          = Id::LockDoesNotExist;
        map["BIMCS.LocksExist"]                                = Id::LocksExist;
        map["BIMCS.LockOwnedByAnotherBriefcase"]               = Id::LockOwnedByAnotherBriefcase;
        map["BIMCS.BriefcaseDoesNotBelongToUser"]              = Id::BriefcaseDoesNotBelongToUser;
        map["BIMCS.UserAlreadyExists"]                         = Id::UserAlreadyExists;
        map["BIMCS.CodeStateInvalid"]                          = Id::CodeStateInvalid;
        map["BIMCS.CodeStateRevisionDenied"]                   = Id::CodeStateRevisionDenied;
        map["BIMCS.CodeReservedByAnotherBriefcase"]            = Id::CodeReservedByAnotherBriefcase;
        map["BIMCS.CodeAlreadyExists"]                         = Id::CodeAlreadyExists;
        map["BIMCS.CodeDoesNotExist"]                          = Id::CodeDoesNotExist;
        map["BIMCS.CodesExist"]                                = Id::CodesExist;
        map["BIMCS.UserDoesNotExist"]                          = Id::UserDoesNotExist;
        map["BIMCS.FileDoesNotExist"]                          = Id::FileDoesNotExist;
        map["BIMCS.FileAlreadyExists"]                         = Id::FileAlreadyExists;
        map["BIMCS.RepositoryIsLocked"]                        = Id::RepositoryIsLocked;
        }

    auto it = map.find(errorIdString);
    if (it != map.end())
        {
        return it->second;
        }

    return Id::Unknown;
    }

DgnDbServerError::Id DgnDbServerError::ErrorIdFromWSError(WebServices::WSErrorCR error)
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

bool DgnDbServerError::RequiresExtendedData(Id id)
    {
    switch (id)
        {
            case Id::LockOwnedByAnotherBriefcase:
            case Id::RepositoryAlreadyExists:
            case Id::FileAlreadyExists:
                return true;
            default:
                return false;
        }
    }

DgnDbServerError::DgnDbServerError()
    {
    m_id = Id::Unknown;
    m_message = GetDefaultMessage(Id::Unknown);
    }

DgnDbServerError::DgnDbServerError(WebServices::WSErrorCR error)
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

DgnDbServerError::DgnDbServerError(DgnDbCR db, BeSQLite::DbResult result)
    {
    m_id = Id::DgnDbError;
    m_message = db.GetLastError(&result);
    }

DgnDbServerError::DgnDbServerError(DgnDbPtr db, BeSQLite::DbResult result)
    {
    m_id = Id::DgnDbError;
    if (db.IsValid())
        {
        m_message = db->GetLastError(&result);
        }
    }

DgnDbServerError::DgnDbServerError(RevisionStatus const& status)
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

DgnDbServerError::DgnDbServerError(HttpErrorCR error)
    {
    m_id = Id::AzureError;
    m_message = error.AsyncError::GetMessage();
    m_description = error.AsyncError::GetDescription();
    }

Utf8StringCR DgnDbServerError::GetDefaultMessage(DgnDbServerError::Id id)
    {
    static bmap<DgnDbServerError::Id, Utf8String> map;
    if (map.empty())
        {
        map[Id::CredentialsNotSet] = DgnDbServerErrorLocalizedString(MESSAGE_CredentialsNotSet);
        map[Id::InvalidServerURL] = DgnDbServerErrorLocalizedString(MESSAGE_InvalidServerURL);
        map[Id::InvalidRepositoryName] = DgnDbServerErrorLocalizedString(MESSAGE_InvalidRepositoryName);
        map[Id::InvalidRepositoryId] = DgnDbServerErrorLocalizedString(MESSAGE_InvalidRepositoryId);
        map[Id::InvalidRepositoryConnection] = DgnDbServerErrorLocalizedString(MESSAGE_InvalidRepositoryConnection);
        map[Id::InvalidRevision] = DgnDbServerErrorLocalizedString(MESSAGE_InvalidRevision);

        map[Id::UserDoesNotExist] = DgnDbServerErrorLocalizedString(MESSAGE_UserDoesNotExist);

        map[Id::RepositoryIsNotInitialized] = DgnDbServerErrorLocalizedString(MESSAGE_RepositoryIsNotInitialized);

        map[Id::FileIsNotYetInitialized] = DgnDbServerErrorLocalizedString(MESSAGE_FileIsNotYetInitialized);
        map[Id::FileIsOutdated] = DgnDbServerErrorLocalizedString(MESSAGE_FileIsOutdated);
        map[Id::FileHasDifferentId] = DgnDbServerErrorLocalizedString(MESSAGE_FileHasDifferentId);
        map[Id::FileInitializationFailed] = DgnDbServerErrorLocalizedString(MESSAGE_FileInitializationFailed);

        map[Id::BriefcaseIsReadOnly] = DgnDbServerErrorLocalizedString(MESSAGE_BriefcaseIsReadOnly);
        map[Id::FileNotFound] = DgnDbServerErrorLocalizedString(MESSAGE_FileNotFound);
        map[Id::PullIsRequired] = DgnDbServerErrorLocalizedString(MESSAGE_PullIsRequired);
        map[Id::TrackingNotEnabled] = DgnDbServerErrorLocalizedString(MESSAGE_TrackingNotEnabled);
        map[Id::FileAlreadyExists] = DgnDbServerErrorLocalizedString(MESSAGE_FileAlreadyExists);
        map[Id::FileIsNotBriefcase] = DgnDbServerErrorLocalizedString(MESSAGE_FileIsNotBriefcase);

        map[Id::MergeError] = DgnDbServerErrorLocalizedString(MESSAGE_MergeError);
        map[Id::RevisionManagerError] = DgnDbServerErrorLocalizedString(MESSAGE_RevisionManagerError);
        map[Id::RevisionDoesNotExist] = DgnDbServerErrorLocalizedString(MESSAGE_RevisionDoesNotExist);

        map[Id::EventCallbackAlreadySubscribed] = DgnDbServerErrorLocalizedString(MESSAGE_EventCallbackAlreadySubscribed);
        map[Id::EventServiceSubscribingError] = DgnDbServerErrorLocalizedString(MESSAGE_EventServiceSubscribingError);
        map[Id::EventCallbackNotFound] = DgnDbServerErrorLocalizedString(MESSAGE_EventCallbackNotFound);
        map[Id::EventCallbackNotSpecified] = DgnDbServerErrorLocalizedString(MESSAGE_EventCallbackNotSpecified);
        map[Id::NoEventsFound] = DgnDbServerErrorLocalizedString(MESSAGE_NoEventsFound);
        map[Id::NotSubscribedToEventService] = DgnDbServerErrorLocalizedString(MESSAGE_NotSubscribedToEventService);
        map[Id::NoSASFound] = DgnDbServerErrorLocalizedString(MESSAGE_NoSASFound);
        map[Id::NoSubscriptionFound] = DgnDbServerErrorLocalizedString(MESSAGE_NoSubscriptionFound);

        map[Id::Unknown] = DgnDbServerErrorLocalizedString(MESSAGE_Unknown);
        }

    auto it = map.find(id);
    if (it != map.end())
        {
        return it->second;
        }

    return map[Id::Unknown];
    }

DgnDbServerError::DgnDbServerError(DgnDbServerError::Id id)
    {
    m_id = id;
    m_message = GetDefaultMessage(id);
    }

END_BENTLEY_DGNDBSERVER_NAMESPACE
