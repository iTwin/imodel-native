/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbLocks.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnDbServer/Client/DgnDbLocks.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

DgnDbRepositoryConnectionPtr DgnDbLocks::Connect(DgnDbCR db)
    {
    auto repository = RepositoryInfo::ReadRepositoryInfo(db);
    if (m_connection)
        {
        if (*repository == m_connection->GetRepositoryInfo())
            return m_connection;
        }

    auto result = DgnDbRepositoryConnection::Create(RepositoryInfo::ReadRepositoryInfo(db), m_credentials, m_clientInfo, m_cancellationToken)->GetResult();
    if (result.IsSuccess())
        {
        DgnDbRepositoryConnectionPtr connection = result.GetValue();
        m_connection = connection;
        return connection;
        }
    else
        return nullptr;
    }

LockStatus DgnDbLocks::_QueryLocksHeld(bool& held, LockRequestCR locks, DgnDbR db)
    {
    auto connection = Connect(db);
    if (connection)
        {
        auto result = connection->QueryLocksHeld(held, locks, db.GetBriefcaseId(), m_cancellationToken)->GetResult();
        if (result.IsSuccess())
            {
            return LockStatus::Success;
            }
        else
            {
            return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
            }
        }
    else
        return LockStatus::ServerUnavailable;//NEEDSWORK: Should this be false?
    }

LockRequest::Response DgnDbLocks::_AcquireLocks(LockRequestCR locks, DgnDbR db)
    {
    auto connection = Connect(db);
    if (connection)
        {
        Json::Value locksRequest;
        locks.ToJson(locksRequest);
        locksRequest["Description"] = "";
        auto result = connection->AcquireLocks(locksRequest, db.GetBriefcaseId(), m_cancellationToken)->GetResult();
        if (result.IsSuccess())
            {
            return result.GetValue();
            }
        else
            {
            return LockRequest::Response(LockStatus::ServerUnavailable);//NEEDSWORK: Use appropriate status
            }
        }
    else
        return LockRequest::Response(LockStatus::ServerUnavailable);
    }

Dgn::LockStatus DgnDbLocks::_ReleaseLocks(Dgn::DgnLockSet const& locks, Dgn::DgnDbR db)
    {
    auto connection = Connect(db);
    if (connection)
        {
        Json::Value locksRequest;
        locksRequest["Locks"] = Json::arrayValue;
        int i = 0;
        for (auto& lock : locks)
            lock.ToJson(locksRequest["Locks"][i++]);
        locksRequest["Description"] = "";
        auto result = connection->ReleaseLocks(locksRequest, db.GetBriefcaseId(), m_cancellationToken)->GetResult();
        if (result.IsSuccess())
            {
            return LockStatus::Success;
            }
        else
            {
            return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
            }
        }
    else
        return LockStatus::ServerUnavailable;
    }

LockStatus DgnDbLocks::_RelinquishLocks(DgnDbR db)
    {
    auto connection = Connect(db);
    if (connection)
        {
        auto result = connection->RelinquishLocks(db.GetBriefcaseId(), m_cancellationToken)->GetResult();
        if (result.IsSuccess())
            {
            return LockStatus::Success;//NEEDSWORK: Can delete locks partially
            }
        else
            {
            return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
            }
        }
    else
        return LockStatus::ServerUnavailable;
    }

LockStatus DgnDbLocks::_QueryLockLevel(LockLevel& level, LockableId lockId, DgnDbR db)
    {
    auto connection = Connect(db);
    if (connection)
        {
        auto result = connection->QueryLockLevel(lockId, db.GetBriefcaseId(), m_cancellationToken)->GetResult();
        if (result.IsSuccess())
            {
            level = result.GetValue();
            return LockStatus::Success;
            }
        else
            {
            return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
            }
        }
    else
        return LockStatus::ServerUnavailable;
    }

LockStatus DgnDbLocks::_QueryLocks(DgnLockSet& locks, DgnDbR db)
    {
    auto connection = Connect(db);
    if (connection)
        {
        auto result = connection->QueryLocks(db.GetBriefcaseId(), m_cancellationToken)->GetResult();
        if (result.IsSuccess())
            {
            locks = result.GetValue();
            return LockStatus::Success;
            }
        else
            {
            return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
            }
        }
    else
        return LockStatus::ServerUnavailable;
    }

LockStatus DgnDbLocks::_QueryOwnership(DgnLockOwnershipR ownership, Dgn::LockableId lockId, DgnDbR db)
    {
    auto connection = Connect(db);
    if (connection)
        {
        auto result = connection->QueryOwnership(lockId, db.GetBriefcaseId(), m_cancellationToken)->GetResult();
        if (result.IsSuccess())
            {
            ownership = result.GetValue();
            return LockStatus::Success;
            }
        else
            {
            return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
            }
        }
    else
        return LockStatus::ServerUnavailable;
    }

DgnDbLocks::DgnDbLocks(WebServices::ClientInfoPtr clientInfo)
    {
    m_clientInfo = clientInfo;
    }

std::shared_ptr<DgnDbLocks> DgnDbLocks::Create(WebServices::ClientInfoPtr clientInfo)
    {
    return std::shared_ptr<DgnDbLocks>(new DgnDbLocks(clientInfo));
    }

void DgnDbLocks::SetCancellationToken(ICancellationTokenPtr cancellationToken)
    {
    m_cancellationToken = cancellationToken;
    }
