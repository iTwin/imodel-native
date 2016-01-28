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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
LockRequest::Response DgnDbLocks::_AcquireLocks(LockRequestCR locks, DgnDbR db)
    {
    if (!m_connection)
        return LockRequest::Response (LockStatus::ServerUnavailable);

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
            DgnLocksJson::LockStatusToJson(deniedLocks[Locks::Status], LockStatus::AlreadyHeld);
            deniedLocks[Locks::DeniedLocks] = Json::arrayValue;
            JsonValueCR errorData = error.GetExtendedData();
            uint32_t i = 0;
            for (auto const& lock : errorData[ServerSchema::Property::ExistingLocks])
                FormatLockFromServer(deniedLocks[Locks::DeniedLocks][i++], lock);
            LockRequest::Response response;
            response.FromJson(deniedLocks);
            return response;
            }
        return LockRequest::Response(LockStatus::ServerUnavailable);//NEEDSWORK: Use appropriate status
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Dgn::LockStatus DgnDbLocks::_DemoteLocks(Dgn::DgnLockSet const& locks, Dgn::DgnDbR db)
    {
    if (!m_connection)
        return LockStatus::ServerUnavailable;

    Utf8String lastRevisionId;
    db.QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);
    auto result = m_connection->DemoteLocks (locks, db.GetBriefcaseId(), lastRevisionId, m_cancellationToken)->GetResult();
    if (result.IsSuccess())
        {
        return LockStatus::Success;
        }
    else
        {
        return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
LockStatus DgnDbLocks::_RelinquishLocks(DgnDbR db)
    {
    if (!m_connection)
        return LockStatus::ServerUnavailable;

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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
LockStatus DgnDbLocks::_QueryLocks(DgnLockSet& locks, DgnDbR db)
    {
    if (!m_connection)
        return LockStatus::ServerUnavailable;

    auto result = m_connection->QueryLocks(db.GetBriefcaseId(), m_cancellationToken)->GetResult();
    if (result.IsSuccess())
        {
        locks = result.GetValue ().GetLocks ();
        return LockStatus::Success;
        }
    else
        {
        return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
LockStatus DgnDbLocks::_QueryLocksHeld(bool& held, LockRequestCR locks, DgnDbR db)
    {
    held = false;

    if (!m_connection)
        return LockStatus::ServerUnavailable;

    LockableIdSet ids;
    for (auto& lock : locks)
        ids.insert (lock.GetLockableId ());

    auto result = m_connection->QueryLocksById (ids, db.GetBriefcaseId (), m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        held = result.GetValue ().GetLocks ().size () == locks.Size ();
        return LockStatus::Success;
        }
    else
        {
        return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
LockStatus DgnDbLocks::_QueryLockLevels(DgnLockSet& levels, LockableIdSet const& lockIds, DgnDbR db)
    {
    if (!m_connection)
        return LockStatus::ServerUnavailable;

    auto result = m_connection->QueryLocksById (lockIds, db.GetBriefcaseId (), m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        levels = result.GetValue ().GetLocks ();
        return LockStatus::Success;
        }
    else
        {
        return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
LockStatus DgnDbLocks::_QueryOwnerships(DgnOwnedLockSet& ownerships, LockableIdSet const& ids)
    {
    if (!m_connection)
        return LockStatus::ServerUnavailable;

    auto result = m_connection->QueryLocksById (ids, m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        ownerships = result.GetValue ().GetOwners ();
        return LockStatus::Success;
        }
    else
        {
        return LockStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbLocks::DgnDbLocks(WebServices::ClientInfoPtr clientInfo)
    {
    m_clientInfo = clientInfo;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
std::shared_ptr<DgnDbLocks> DgnDbLocks::Create(WebServices::ClientInfoPtr clientInfo)
    {
    return std::shared_ptr<DgnDbLocks>(new DgnDbLocks(clientInfo));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
void DgnDbLocks::SetCancellationToken(ICancellationTokenPtr cancellationToken)
    {
    m_cancellationToken = cancellationToken;
    }
