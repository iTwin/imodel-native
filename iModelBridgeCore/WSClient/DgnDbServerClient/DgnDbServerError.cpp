/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerError.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerError.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_WEBSERVICES

DgnDbServerError::DgnDbServerError()
    {
    m_id = Id::Unknown;
    }

DgnDbServerError::Id DgnDbServerError::ErrorIdFromString(Utf8StringCR errorIdString)
    {
    static bmap<Utf8String, DgnDbServerError::Id> map;
    if (map.empty())
        {
        map["DgnDbServer.MissingRequiredProperties"]                 = Id::MissingRequiredProperties;
        map["DgnDbServer.InvalidBriefcase"]                          = Id::InvalidBriefcase;
        map["DgnDbServer.AnotherUserPushing"]                        = Id::AnotherUserPushing;
        map["DgnDbServer.RevisionAlreadyExists"]                     = Id::RevisionAlreadyExists;
        map["DgnDbServer.RevisionDoesNotExist"]                      = Id::RevisionDoesNotExist;
        map["DgnDbServer.FileIsNotUploaded"]                         = Id::FileIsNotUploaded;
        map["DgnDbServer.RevisionExistsButNoBackingFile"]            = Id::RevisionExistsButNoBackingFile;
        map["DgnDbServer.RepositoryIsNotInitialized"]                = Id::RepositoryIsNotInitialized;
        map["DgnDbServer.RevisionPointsToBadDgnDb"]                  = Id::RevisionPointsToBadDgnDb;
        map["DgnDbServer.DgnDbServerOperationFailed"]                = Id::DgnDbServerOperationFailed;
        map["DgnDbServer.PullIsRequired"]                            = Id::PullIsRequired;
        map["DgnDbServer.MaximumNumberOfBriefcasesPerUser"]          = Id::MaximumNumberOfBriefcasesPerUser;
        map["DgnDbServer.MaximumNumberOfBriefcasesPerUserPerMinute"] = Id::MaximumNumberOfBriefcasesPerUserPerMinute;
        map["DgnDbServer.DatabaseTemporarilyLocked"]                 = Id::DatabaseTemporarilyLocked;
        map["DgnDbServer.RepositoryAlreadyExists"]                   = Id::RepositoryAlreadyExists;
        map["DgnDbServer.RepositoryDoesNotExist"]                    = Id::RepositoryDoesNotExist;
        map["DgnDbServer.LockDoesNotExist"]                          = Id::LockDoesNotExist;
        map["DgnDbServer.LockOwnedByAnotherBriefcase"]               = Id::LockOwnedByAnotherBriefcase;
        map["DgnDbServer.BriefcaseDoesNotBelongToUser"]              = Id::BriefcaseDoesNotBelongToUser;
        map["DgnDbServer.UserAlreadyExists"]                         = Id::UserAlreadyExists;
        map["DgnDbServer.CodeStateInvalid"]                          = Id::CodeStateInvalid;
        map["DgnDbServer.CodeReservedByAnotherBriefcase"]            = Id::CodeReservedByAnotherBriefcase;
        map["DgnDbServer.CodeDoesNotExist"]                          = Id::CodeDoesNotExist;
        map["DgnDbServer.UserDoesNotExist"]                          = Id::UserDoesNotExist;
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
                return true;
            default:
                return false;
        }
    }

DgnDbServerError::DgnDbServerError(Id id)
    {
    m_id = id;
    }

DgnDbServerError::DgnDbServerError(WebServices::WSErrorCR error)
    {
    m_message = error.GetMessage();
    m_description = error.GetDescription();
    Utf8StringCR customId = error.GetData()["errorId"].asString();
    if (customId.StartsWith ("DgnDbServer."))
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
        m_id = Id::MergeError;
    else
        m_id = Id::RevisionManagerError;
    }

DgnDbServerError::DgnDbServerError(HttpErrorCR error)
    {
    m_id = Id::AzureError;
    m_message = error.AsyncError::GetMessage();
    m_description = error.AsyncError::GetDescription();
    }

JsonValueCR DgnDbServerError::GetExtendedData() const
    {
    return m_wsError ? m_wsError->GetData() : Json::Value::null;
    }

DgnDbServerError::Id DgnDbServerError::GetId() const
    {
    return m_id;
    }

END_BENTLEY_DGNDBSERVER_NAMESPACE
