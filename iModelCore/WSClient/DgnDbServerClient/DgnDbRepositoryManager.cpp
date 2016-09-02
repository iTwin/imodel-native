/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbRepositoryManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbRepositoryManager.h>
#include "DgnDbServerUtils.h"
#include <DgnDbServer/Client/Logging.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SetCodesLocksStates (IBriefcaseManager::Response& response, IBriefcaseManager::ResponseOptions options, JsonValueCR deniedLocks, JsonValueCR deniedCodes)
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
    if (IBriefcaseManager::ResponseOptions::None != (IBriefcaseManager::ResponseOptions::CodeState & options))
        {
        for (auto const& codeJson : deniedCodes)
            {
            DgnCode                  code;
            DgnCodeState             codeState;
            BeSQLite::BeBriefcaseId  briefcaseId;
            Utf8String               revisionId;
            if (!GetCodeFromServerJson(codeJson, code, codeState, briefcaseId, revisionId))
                continue;//NEEDSWORK: log an error

            AddCodeInfoToList(response.CodeStates(), code, codeState, briefcaseId, revisionId);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbServerStatusResult DgnDbRepositoryManager::Connect (DgnDbCR db)
    {
    const Utf8String methodName = "DgnDbRepositoryManager::Connect";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    RepositoryInfo repositoryInfo;
    auto readResult = RepositoryInfo::ReadRepositoryInfo(repositoryInfo, db);

    if (!readResult.IsSuccess())
        {
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
        return readResult;
        }

    if (m_connection)
        {
        if (repositoryInfo == m_connection->GetRepositoryInfo())
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
            return DgnDbServerStatusResult::Success();
            }
        }

    auto connectionResult = DgnDbRepositoryConnection::Create(repositoryInfo, m_credentials, m_clientInfo, m_cancellationToken, m_authenticationHandler)->GetResult();
    if (connectionResult.IsSuccess())
        {
        DgnDbRepositoryConnectionPtr connection = connectionResult.GetValue();
        m_connection = connection;
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
        return DgnDbServerStatusResult::Success();
        }
    else
        {
        m_connection = nullptr;
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return DgnDbServerStatusResult::Error(connectionResult.GetError());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Algirdas.Mikoliunas             07/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::GetResponseStatus(DgnDbServerResult<void> result)
    {
    static bmap<DgnDbServerError::Id, RepositoryStatus> map;
    if (map.empty())
        {
        map[DgnDbServerError::Id::LockOwnedByAnotherBriefcase]    = RepositoryStatus::LockAlreadyHeld;
        map[DgnDbServerError::Id::PullIsRequired]                 = RepositoryStatus::RevisionRequired;
        map[DgnDbServerError::Id::RevisionDoesNotExist]           = RepositoryStatus::InvalidRequest;
        map[DgnDbServerError::Id::CodeStateInvalid]               = RepositoryStatus::InvalidRequest;
        map[DgnDbServerError::Id::CodeReservedByAnotherBriefcase] = RepositoryStatus::CodeUnavailable;
        map[DgnDbServerError::Id::CodeDoesNotExist]               = RepositoryStatus::CodeNotReserved;
        map[DgnDbServerError::Id::InvalidPropertiesValues]        = RepositoryStatus::InvalidRequest;
        map[DgnDbServerError::Id::CodeStateRevisionDenied]        = RepositoryStatus::InvalidRequest;
        map[DgnDbServerError::Id::CodeAlreadyExists]              = RepositoryStatus::CodeUnavailable;
        }

    auto it = map.find(result.GetError().GetId());
    if (it != map.end())
        {
        return it->second;
        }

    return RepositoryStatus::ServerUnavailable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Algirdas.Mikoliunas             07/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response DgnDbRepositoryManager::HandleError(Request const& request, DgnDbServerResult<void> result, IBriefcaseManager::RequestPurpose purpose)
    {
    Response           response(purpose, request.Options(), RepositoryStatus::ServerUnavailable);
    DgnDbServerError&  error = result.GetError();

    RepositoryStatus responseStatus = GetResponseStatus(result);
    if (RepositoryStatus::LockAlreadyHeld == responseStatus)
        {
        response.SetResult(responseStatus);
        JsonValueCR errorData = error.GetExtendedData();
        SetCodesLocksStates(response, request.Options(), errorData[ServerSchema::Property::ConflictingLocks], nullptr);
        }
    else if (RepositoryStatus::RevisionRequired == responseStatus)
        {
        response.SetResult(responseStatus);
        JsonValueCR errorData = error.GetExtendedData();
        SetCodesLocksStates(response, request.Options(), errorData[ServerSchema::Property::LocksRequiresPull], errorData[ServerSchema::Property::CodesRequiresPull]);
        }
    else if (RepositoryStatus::InvalidRequest == responseStatus)
        {
        response.SetResult(responseStatus);
        }
    else if (RepositoryStatus::CodeUnavailable == responseStatus)
        {
        response.SetResult(responseStatus);
        JsonValueCR errorData = error.GetExtendedData();
        auto errorPropertyName = DgnDbServerError::Id::CodeStateInvalid == error.GetId()
            ? ServerSchema::Property::CodeStateInvalid
            : ServerSchema::Property::ConflictingCodes;
        SetCodesLocksStates(response, request.Options(), nullptr, errorData[errorPropertyName]);
        }
    else if (RepositoryStatus::CodeNotReserved == responseStatus)
        {
        response.SetResult(responseStatus);
        JsonValueCR errorData = error.GetExtendedData();
        SetCodesLocksStates(response, request.Options(), nullptr, errorData[ServerSchema::Property::CodesNotFound]);
        }

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

    DgnDbServerStatusResult result;
    if (queryOnly)
        result = m_connection->QueryCodesLocksAvailability(req.Locks(), req.Codes(), db.GetBriefcaseId(), lastRevisionId, m_cancellationToken)->GetResult();
    else
        // NEEDSWORK: pass ResponseOptions to make sure we do not return locks if they are not needed. This is currently not supported by WSG.
        result = m_connection->AcquireCodesLocks (req.Locks (), req.Codes(), db.GetBriefcaseId (), lastRevisionId, m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        return IBriefcaseManager::Response (purpose, req.Options(), RepositoryStatus::Success);
        }
    
    return HandleError(req, result, purpose);
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
        return GetResponseStatus(result);
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
        return GetResponseStatus(result);//NEEDSWORK: Use appropriate status
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnDbRepositoryManager::_QueryHeldResources (DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db)
    {
    if (!m_connection)
        return RepositoryStatus::ServerUnavailable;

    auto availableTask = m_connection->QueryCodesLocks(db.GetBriefcaseId(), m_cancellationToken);
    auto unavailableTask = m_connection->QueryUnavailableCodesLocks(db.GetBriefcaseId(), db.Revisions().GetParentRevisionId(), m_cancellationToken);
    bset<std::shared_ptr<AsyncTask>> tasks;
    tasks.insert(availableTask);
    tasks.insert(unavailableTask);
    AsyncTask::WhenAll(tasks)->Wait();
    if (availableTask->GetResult().IsSuccess() && unavailableTask->GetResult().IsSuccess())
        {
        locks = availableTask->GetResult().GetValue ().GetLocks ();
        codes = availableTask->GetResult().GetValue ().GetCodes ();
        unavailableLocks = unavailableTask->GetResult().GetValue().GetLocks();
        unavailableCodes = unavailableTask->GetResult().GetValue().GetCodes();
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
