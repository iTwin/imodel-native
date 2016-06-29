/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbRepositoryManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbRepositoryManager.h>
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
DgnDbServerStatusResult DgnDbRepositoryManager::Connect (DgnDbCR db)
    {
    RepositoryInfo repositoryInfo;
    auto readResult = RepositoryInfo::ReadRepositoryInfo(repositoryInfo, db);

    if (!readResult.IsSuccess())
        return readResult;

    if (m_connection)
        {
        if (repositoryInfo == m_connection->GetRepositoryInfo())
            return DgnDbServerStatusResult::Success();
        }

    auto connectionResult = DgnDbRepositoryConnection::Create(repositoryInfo, m_credentials, m_clientInfo, m_cancellationToken, m_authenticationHandler)->GetResult();
    if (connectionResult.IsSuccess())
        {
        DgnDbRepositoryConnectionPtr connection = connectionResult.GetValue();
        m_connection = connection;
        return DgnDbServerStatusResult::Success();
        }
    else
        {
        m_connection = nullptr;
        return DgnDbServerStatusResult::Error(connectionResult.GetError());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Algirdas.Mikoliunas               06/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response DgnDbRepositoryManager::QueryCodesLocksAvailable(Request const& req, DgnDbR db)
    {
    LockableIdSet lockIds;
    for (auto const& lock : req.Locks().GetLockSet())
        lockIds.insert(lock.GetLockableId());

    DgnLockInfoSet lockStates;
    DgnCodeInfoSet codeStates;
    this->_QueryStates(lockStates, codeStates, lockIds, req.Codes());

    if (lockStates.empty() && codeStates.empty())
        {
        return IBriefcaseManager::Response(IBriefcaseManager::RequestPurpose::Query, req.Options(), RepositoryStatus::Success);
        }

    // NEEDSWORK - handle errors
    Response response(IBriefcaseManager::RequestPurpose::Query, req.Options(), RepositoryStatus::ServerUnavailable);
    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response DgnDbRepositoryManager::_ProcessRequest (Request const& req, DgnDbR db, bool queryOnly)
    {
    auto purpose = queryOnly ? IBriefcaseManager::RequestPurpose::Query : IBriefcaseManager::RequestPurpose::Acquire;
    if (!m_connection)
        return Response (purpose, req.Options(), RepositoryStatus::ServerUnavailable);

    if (req.Locks ().IsEmpty () && req.Codes().empty())
        return IBriefcaseManager::Response (purpose, req.Options(), RepositoryStatus::Success);

    Utf8String lastRevisionId = db.Revisions ().GetParentRevisionId ();

    if (queryOnly)
        {
        return QueryCodesLocksAvailable(req, db);
        }

    // NEEDSWORK: pass ResponseOptions to make sure we do not return locks if they are not needed. This is currently not supported by WSG.
    auto result = m_connection->AcquireCodesLocks (req.Locks (), req.Codes(), db.GetBriefcaseId (), lastRevisionId, m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        return IBriefcaseManager::Response (purpose, req.Options(), RepositoryStatus::Success);
        }
    else
        {
        Response           response (purpose, req.Options(), RepositoryStatus::ServerUnavailable);
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
        else if (DgnDbServerError::Id::RevisionDoesNotExist == error.GetId ())
            {
            response.SetResult (RepositoryStatus::InvalidRequest);
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

    if (locks.empty () && codes.empty())
        return RepositoryStatus::Success;

    // NEEDSWORK_LOCKS: Handle codes
    auto result = m_connection->DemoteCodesLocks (locks, codes, db.GetBriefcaseId (), m_cancellationToken)->GetResult ();
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

    if (Resources::None == which) 
        return RepositoryStatus::Success;

    auto result = m_connection->RelinquishCodesLocks (db.GetBriefcaseId (), m_cancellationToken)->GetResult ();
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
RepositoryStatus DgnDbRepositoryManager::_QueryHeldResources (DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    // NEEDSWORK_LOCKS: Handle unavailable locks + codes
    auto result = m_connection->QueryCodesLocks (db.GetBriefcaseId (), m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        locks = result.GetValue ().GetLocks ();
        codes = result.GetValue ().GetCodes ();
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

    if (locks.empty () && codes.empty())
        return RepositoryStatus::Success;

    auto result = m_connection->QueryCodesLocksById (codes, locks, m_cancellationToken)->GetResult ();

    if (result.IsSuccess ())
        {
        lockStates = result.GetValue ().GetLockStates ();
        codeStates = result.GetValue ().GetCodeStates ();
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
DgnDbRepositoryManager::DgnDbRepositoryManager (WebServices::ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler)
    {
    m_clientInfo = clientInfo;
    m_authenticationHandler = authenticationHandler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              12/15
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<DgnDbRepositoryManager> DgnDbRepositoryManager::Create (WebServices::ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler)
    {
    return std::shared_ptr<DgnDbRepositoryManager>(new DgnDbRepositoryManager(clientInfo, authenticationHandler));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbRepositoryManager::SetCancellationToken (ICancellationTokenPtr cancellationToken)
    {
    m_cancellationToken = cancellationToken;
    }
