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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbResult DgnDbRepositoryManager::Connect(DgnDbCR db)
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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryManager::Response DgnDbRepositoryManager::_AcquireLocks(LockRequestCR locks, DgnDbR db)
    {
    if (!m_connection)
        return Response(RepositoryStatus::ServerUnavailable);

    Utf8String lastRevisionId;
    db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);
    auto result = m_connection->AcquireLocks (locks, db.GetBriefcaseId (), lastRevisionId, m_cancellationToken)->GetResult ();
    if (result.IsSuccess())
        {
        return result.GetValue();
        }
    else
        {
        DgnDbServerError& error = result.GetError();
        if (DgnDbServerError::Id::LockOwnedByAnotherBriefcase == error.GetId())
            {
            Json::Value deniedLocks;
            RepositoryJson::RepositoryStatusToJson(deniedLocks[Locks::Status], RepositoryStatus::LockAlreadyHeld);
            deniedLocks[Locks::DeniedLocks] = Json::arrayValue;
            JsonValueCR errorData = error.GetExtendedData();
            uint32_t i = 0;
            for (auto const& lock : errorData[ServerSchema::Property::ExistingLocks])
                FormatLockFromServer(deniedLocks[Locks::DeniedLocks][i++], lock);
            Response response;
#ifdef NEEDSWORK_LOCKS
            response.FromJson(deniedLocks);
#endif
            return response;
            }
        return Response(RepositoryStatus::ServerUnavailable);//NEEDSWORK: Use appropriate status
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbRepositoryManager::Response DgnDbRepositoryManager::_ProcessRequest(Request const& req, DgnDbR db)
    {
    // NEEDSWORK_LOCKS: Handle codes
    return _AcquireLocks(req.Locks(), db);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Dgn::RepositoryStatus DgnDbRepositoryManager::_DemoteLocks(Dgn::DgnLockSet const& locks, Dgn::DgnDbR db)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    Utf8String lastRevisionId;
    db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);
    auto result = m_connection->DemoteLocks (locks, db.GetBriefcaseId(), lastRevisionId, m_cancellationToken)->GetResult();
    if (result.IsSuccess())
        {
        return RepositoryStatus::Success;
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
    {
    // NEEDSWORK_LOCKS: Handle codes
    return _DemoteLocks(locks, db);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
RepositoryStatus DgnDbRepositoryManager::_RelinquishLocks(DgnDbR db)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    Utf8String lastRevisionId;
    db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);

    auto result = m_connection->RelinquishLocks(db.GetBriefcaseId(), lastRevisionId, m_cancellationToken)->GetResult();
    if (result.IsSuccess())
        {
        return RepositoryStatus::Success;//NEEDSWORK: Can delete locks partially
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_Relinquish(Resources which, DgnDbR db)
    {
    // NEEDSWORK_LOCKS: Handle codes
    return (Resources::Locks == (which & Resources::Locks)) ? _RelinquishLocks(db) : RepositoryStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
RepositoryStatus DgnDbRepositoryManager::_QueryLocks(DgnLockSet& locks, DgnDbR db)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    auto result = m_connection->QueryLocks(db.GetBriefcaseId(), m_cancellationToken)->GetResult();
    if (result.IsSuccess())
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
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db)
    {
    // NEEDSWORK_LOCKS: Handle codes
    return _QueryLocks(locks, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes)
    {
    // NEEDSWORK_LOCKS
    return RepositoryStatus::InvalidResponse;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryManager::DgnDbRepositoryManager(WebServices::ClientInfoPtr clientInfo)
    {
    m_clientInfo = clientInfo;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
std::shared_ptr<DgnDbRepositoryManager> DgnDbRepositoryManager::Create(WebServices::ClientInfoPtr clientInfo)
    {
    return std::shared_ptr<DgnDbRepositoryManager>(new DgnDbRepositoryManager(clientInfo));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
void DgnDbRepositoryManager::SetCancellationToken(ICancellationTokenPtr cancellationToken)
    {
    m_cancellationToken = cancellationToken;
    }

