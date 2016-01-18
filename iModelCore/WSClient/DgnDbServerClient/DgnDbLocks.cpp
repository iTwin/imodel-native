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

DgnDbResult DgnDbLocks::Connect(DgnDbCR db)
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

LockStatus DgnDbLocks::_QueryLocksHeld(bool& held, LockRequestCR locks, DgnDbR db)
    {
    if (m_connection)
        {
        auto result = m_connection->QueryLocksHeld(held, locks, db.GetBriefcaseId(), m_cancellationToken)->GetResult();
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

LockRequest::Response DgnDbLocks::_AcquireLocks(LockRequestCR locks, DgnDbR db)
    {
    if (m_connection)
        {
        Utf8String lastRevisionId;
        db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);

        Json::Value locksRequest;
        locks.ToJson(locksRequest);
        locksRequest[Locks::Description] = "";
        auto result = m_connection->AcquireLocks (locksRequest, db.GetBriefcaseId (), lastRevisionId, m_cancellationToken)->GetResult ();
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
    if (m_connection)
        {
        Utf8String lastRevisionId;
        db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);

        Json::Value locksRequest;
        locksRequest[Locks::Locks] = Json::arrayValue;
        int i = 0;
        for (auto& lock : locks)
            lock.ToJson(locksRequest[Locks::Locks][i++]);
        locksRequest[Locks::Description] = "";
        auto result = m_connection->ReleaseLocks(locksRequest, db.GetBriefcaseId(), lastRevisionId, m_cancellationToken)->GetResult();
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
    if (m_connection)
        {
        Utf8String lastRevisionId;
        db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);

        auto result = m_connection->RelinquishLocks(db.GetBriefcaseId(), lastRevisionId, m_cancellationToken)->GetResult();
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
    if (m_connection)
        {
        auto result = m_connection->QueryLockLevel(lockId, db.GetBriefcaseId(), m_cancellationToken)->GetResult();
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
    if (m_connection)
        {
        auto result = m_connection->QueryLocks(db.GetBriefcaseId(), m_cancellationToken)->GetResult();
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

LockStatus DgnDbLocks::_QueryOwnership(DgnLockOwnershipR ownership, Dgn::LockableId lockId)
    {
    if (m_connection)
        {
        auto result = m_connection->QueryOwnership(lockId, m_cancellationToken)->GetResult();
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


LockStatus DgnDbLocks::_QueryRevisionId(WStringR, LockableId)
    {
    BeAssert(false && "Unimplemented");
    return LockStatus::Success;
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
