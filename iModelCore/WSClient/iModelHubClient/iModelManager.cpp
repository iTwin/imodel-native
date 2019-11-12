/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/iModelManager.h>
#include "Utils.h"
#include "Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_WEBSERVICES

iModelManager::iModelManager(iModelConnectionPtr connection) : m_connection(connection) 
    { 
    SetCancellationToken([]()->ICancellationTokenPtr { return nullptr; });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response iModelManager::_ProcessRequest(Request const& req, DgnDbR db, bool queryOnly)
    {
    const Utf8String methodName = "iModelManager::_ProcessRequest";
    auto purpose = queryOnly ? IBriefcaseManager::RequestPurpose::Query : IBriefcaseManager::RequestPurpose::Acquire;

    if (req.Locks().IsEmpty() && req.Codes().empty())
        return IBriefcaseManager::Response(purpose, req.Options(), RepositoryStatus::Success);

    if (m_connection.IsNull())
        return Response(purpose, req.Options(), RepositoryStatus::ServerUnavailable);

    if (!req.Codes().empty())
        {
        for (DgnCodeCR code : req.Codes())
            {
            CodeSpecCPtr codeSpec = db.CodeSpecs().GetCodeSpec(code.GetCodeSpecId());
            if (!codeSpec->IsManagedWithDgnDb())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Cannot request code managed by external service.");
                return Response(purpose, req.Options(), RepositoryStatus::InvalidRequest);
                }
            }
        }

    Utf8String lastChangeSetId = db.Revisions().GetParentRevisionId();

    StatusResultPtr result;
    if (queryOnly)
        result = ExecuteAsync(m_connection->QueryCodesLocksAvailability(req.Locks(), req.Codes(), db.GetBriefcaseId(), db.GetDbGuid(), 
                                                                        lastChangeSetId, req.Options(), m_cancellationToken()));
    else
        // NEEDSWORK: pass ResponseOptions to make sure we do not return locks if they are not needed. This is currently not supported by WSG.
        result = ExecuteAsync(m_connection->AcquireCodesLocks (req.Locks (), req.Codes(), db.GetBriefcaseId (), db.GetDbGuid(), lastChangeSetId, 
                                                               req.Options(), m_cancellationToken()));
    if (result->IsSuccess ())
        {
        return IBriefcaseManager::Response(purpose, req.Options(), RepositoryStatus::Success);
        }

    return ConvertErrorResponse(req, *result, purpose);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus iModelManager::_Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
    {
    if (locks.empty() && codes.empty())
        return RepositoryStatus::Success;

    if (m_connection.IsNull())
        return RepositoryStatus::ServerUnavailable;

    // NEEDSWORK_LOCKS: Handle codes
    auto result = ExecuteAsync(m_connection->DemoteCodesLocks (locks, codes, db.GetBriefcaseId (), db.GetDbGuid(), ResponseOptions::None, m_cancellationToken()));
    if (result->IsSuccess ())
        {
        return RepositoryStatus::Success;
        }
    else
        {
        return GetErrorResponseStatus(*result);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus iModelManager::_Relinquish(Resources which, DgnDbR db)
    {
    if (Resources::None == which)
        return RepositoryStatus::Success;

    if (m_connection.IsNull())
        return RepositoryStatus::ServerUnavailable;

    auto result = ExecuteAsync(m_connection->RelinquishCodesLocksInternal(which, db.GetBriefcaseId(), ResponseOptions::None, m_cancellationToken()));
    if (result->IsSuccess ())
        {
        return RepositoryStatus::Success;//NEEDSWORK: Can delete locks partially
        }
    else
        {
        return GetErrorResponseStatus(*result);//NEEDSWORK: Use appropriate status
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus iModelManager::_QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, 
                                                    DgnDbR db)
    {
    if (m_connection.IsNull())
        return RepositoryStatus::ServerUnavailable;

    auto availableTask = m_connection->QueryCodesLocks(db.GetBriefcaseId(), m_cancellationToken());
    auto unavailableTask = m_connection->QueryUnavailableCodesLocks(db.GetBriefcaseId(), db.Revisions().GetParentRevisionId(), 
                                                                    m_cancellationToken());
    bset<std::shared_ptr<AsyncTask>> tasks;
    tasks.insert(availableTask);
    tasks.insert(unavailableTask);

    AsyncTask::WhenAll(tasks)->Wait(); 
    auto availableResult = ExecuteAsync(availableTask);
    auto unavailableResult = ExecuteAsync(unavailableTask);
    if (availableResult->IsSuccess() && unavailableResult->IsSuccess())
        {
        locks = availableResult->GetValue().GetLocks ();
        codes = availableResult->GetValue().GetCodes ();
        unavailableLocks = unavailableResult->GetValue().GetLocks();
        unavailableCodes = unavailableResult->GetValue().GetCodes();
        return RepositoryStatus::Success;
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Sam.Wilson                      07/19
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus iModelManager::_QueryHeldLocks(DgnLockSet& locks, DgnDbR db)
    {
    if (m_connection.IsNull())
        return RepositoryStatus::ServerUnavailable;

    auto result = ExecuteAsync(m_connection->QueryLocksByBriefcaseId(db.GetBriefcaseId(), m_cancellationToken()));

    if (result->IsSuccess())
        {
        auto& lockInfos = result->GetValue();
        for (auto const& lockInfo : lockInfos)
            {
            locks.insert(DgnLock(lockInfo.GetLockableId(), lockInfo.GetOwnership().GetLockLevel()));
            }
        return RepositoryStatus::Success;
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus iModelManager::_QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, 
                                             DgnCodeSet const& codes)
    {
    if (locks.empty() && codes.empty())
        return RepositoryStatus::Success;

    if (m_connection.IsNull())
        return RepositoryStatus::ServerUnavailable;

    auto result = ExecuteAsync(m_connection->QueryCodesLocksById(codes, locks, m_cancellationToken()));

    if (result->IsSuccess ())
        {
        lockStates = result->GetValue ().GetLockStates ();
        codeStates = result->GetValue ().GetCodeStates ();
        return RepositoryStatus::Success;
        }
    else
        {
        return RepositoryStatus::ServerUnavailable;//NEEDSWORK: Use appropriate status
        }
    }