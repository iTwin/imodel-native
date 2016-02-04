/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbLocks.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbLocks.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SetLockStates (IBriefcaseManager::Response& response, IBriefcaseManager::ResponseOptions options, JsonValueCR deniedLocks)
    {
    if (IBriefcaseManager::ResponseOptions::None != (IBriefcaseManager::ResponseOptions::LockState & options))
        {
        for (auto const& lockJson : deniedLocks)
            {
            DgnLock                  lock;
            BeSQLite::BeBriefcaseId  briefcaseId;
            Utf8String               repositoryId;
            if (!GetLockFromServerJson (lockJson, lock, briefcaseId, repositoryId))
                continue;//NEEDSWORK: log an error

            AddLockInfoToList (response.LockStates (), lock, briefcaseId, repositoryId);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbResult DgnDbRepositoryManager::Connect (DgnDbCR db)
    {
    auto repository = RepositoryInfo::ReadRepositoryInfo(db);
    if (m_connection)
        {
        if (*repository == m_connection->GetRepositoryInfo())
            return DgnDbResult::Success();
        }

    auto result = DgnDbRepositoryConnection::Create(RepositoryInfo::ReadRepositoryInfo(db), m_credentials, m_clientInfo, m_cancellationToken)->GetResult();
    if (result.IsSuccess())
        {
        DgnDbRepositoryConnectionPtr connection = result.GetValue();
        m_connection = connection;
        return DgnDbResult::Success();
        }
    else
        {
        m_connection = nullptr;
        return DgnDbResult::Error(result.GetError());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response DgnDbRepositoryManager::_ProcessRequest (Request const& req, DgnDbR db)
    {
    if (!m_connection)
        return Response (RepositoryStatus::ServerUnavailable);

    Utf8String lastRevisionId;
    db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);

    // NEEDSWORK_LOCKS: Handle codes
    // NEEDSWORK: pass ResponseOptions to make sure we do not return locks if they are not needed. This is currently not supported by WSG.
    auto result = m_connection->AcquireLocks (req.Locks (), db.GetBriefcaseId (), lastRevisionId, m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        return IBriefcaseManager::Response (RepositoryStatus::Success);
        }
    else
        {
        Response           response (RepositoryStatus::ServerUnavailable);
        DgnDbServerError&  error = result.GetError ();

        if (DgnDbServerError::Id::LockOwnedByAnotherBriefcase == error.GetId ())
            {
            response.SetResult (RepositoryStatus::LockAlreadyHeld);
            JsonValueCR errorData = error.GetExtendedData ();
            SetLockStates (response, req.Options (), errorData[ServerSchema::Property::ConflictingLocks]);
            }
        else if (DgnDbServerError::Id::PullIsRequired == error.GetId ())
            {
            response.SetResult (RepositoryStatus::RevisionRequired);
            JsonValueCR errorData = error.GetExtendedData ();
            SetLockStates (response, req.Options (), errorData[ServerSchema::Property::LocksRequiresPull]);
            }
            
        return response;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_Demote (DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    Utf8String lastRevisionId;
    db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);

    // NEEDSWORK_LOCKS: Handle codes
    auto result = m_connection->DemoteLocks (locks, db.GetBriefcaseId (), lastRevisionId, m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        return RepositoryStatus::Success;
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_Relinquish (Resources which, DgnDbR db)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    Utf8String lastRevisionId;
    db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);

    // NEEDSWORK_LOCKS: Handle codes
    if (Resources::Locks != (which & Resources::Locks)) 
        return RepositoryStatus::Success;

    auto result = m_connection->RelinquishLocks (db.GetBriefcaseId (), lastRevisionId, m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        return RepositoryStatus::Success;//NEEDSWORK: Can delete locks partially
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_QueryHeldResources (DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    // NEEDSWORK_LOCKS: Handle codes
    auto result = m_connection->QueryLocks (db.GetBriefcaseId (), m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        locks = result.GetValue ().GetLocks ();
        return RepositoryStatus::Success;
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_QueryStates (DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    // NEEDSWORK_LOCKS: Handle codes
    auto result = m_connection->QueryLocksById (locks, m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        lockStates = result.GetValue ().GetLockStates ();
        return RepositoryStatus::Success;
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbRepositoryManager::DgnDbRepositoryManager (WebServices::ClientInfoPtr clientInfo)
    {
    m_clientInfo = clientInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              12/15
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<DgnDbRepositoryManager> DgnDbRepositoryManager::Create (WebServices::ClientInfoPtr clientInfo)
    {
    return std::shared_ptr<DgnDbRepositoryManager>(new DgnDbRepositoryManager(clientInfo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbRepositoryManager::SetCancellationToken (ICancellationTokenPtr cancellationToken)
    {
    m_cancellationToken = cancellationToken;
    }
