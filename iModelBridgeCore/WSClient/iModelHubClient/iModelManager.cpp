/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/iModelManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/iModelManager.h>
#include "Utils.h"
#include "Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SetCodesLocksStates (IBriefcaseManager::Response& response, IBriefcaseManager::ResponseOptions options, JsonValueCR deniedLocks, JsonValueCR deniedCodes)
    {
    if (IBriefcaseManager::ResponseOptions::None != (IBriefcaseManager::ResponseOptions::LockState & options))
        {
        for (auto const& lockJson : deniedLocks)
            {
            auto rapidJson = ToRapidJson(lockJson);
            if (!AddLockInfoToListFromErrorJson (response.LockStates(), rapidJson))
                continue;//NEEDSWORK: log an error
            }
        }
    if (IBriefcaseManager::ResponseOptions::None != (IBriefcaseManager::ResponseOptions::CodeState & options))
        {
        for (auto const& codeJson : deniedCodes)
            {
            DgnCode                  code;
            DgnCodeState             codeState;
            BeSQLite::BeBriefcaseId  briefcaseId;
            auto rapidJson = ToRapidJson(codeJson);
            if (!GetCodeFromServerJson(rapidJson, code, codeState, briefcaseId))
                continue;//NEEDSWORK: log an error

            AddCodeInfoToList(response.CodeStates(), code, codeState, briefcaseId);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Algirdas.Mikoliunas             07/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus iModelManager::GetResponseStatus(Result<void> result)
    {
    static bmap<Error::Id, RepositoryStatus> map;
    if (map.empty())
        {
        map[Error::Id::LockOwnedByAnotherBriefcase]    = RepositoryStatus::LockAlreadyHeld;
        map[Error::Id::PullIsRequired]                 = RepositoryStatus::RevisionRequired;
        map[Error::Id::ChangeSetDoesNotExist]          = RepositoryStatus::InvalidRequest;
        map[Error::Id::CodeStateInvalid]               = RepositoryStatus::InvalidRequest;
        map[Error::Id::CodeReservedByAnotherBriefcase] = RepositoryStatus::CodeUnavailable;
        map[Error::Id::CodeDoesNotExist]               = RepositoryStatus::CodeNotReserved;
        map[Error::Id::InvalidPropertiesValues]        = RepositoryStatus::InvalidRequest;
        map[Error::Id::iModelIsLocked]                 = RepositoryStatus::RepositoryIsLocked;
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
IBriefcaseManager::Response iModelManager::HandleError(Request const& request, Result<void> result, IBriefcaseManager::RequestPurpose purpose)
    {
    Response           response(purpose, request.Options(), RepositoryStatus::ServerUnavailable);
    Error&  error = result.GetError();

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
    else if (RepositoryStatus::InvalidRequest == responseStatus || RepositoryStatus::RepositoryIsLocked == responseStatus)
        {
        response.SetResult(responseStatus);
        }
    else if (RepositoryStatus::CodeUnavailable == responseStatus)
        {
        response.SetResult(responseStatus);
        JsonValueCR errorData = error.GetExtendedData();
        auto errorPropertyName = Error::Id::CodeStateInvalid == error.GetId()
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
IBriefcaseManager::Response iModelManager::_ProcessRequest (Request const& req, DgnDbR db, bool queryOnly)
    {
    auto purpose = queryOnly ? IBriefcaseManager::RequestPurpose::Query : IBriefcaseManager::RequestPurpose::Acquire;

    if (req.Locks ().IsEmpty () && req.Codes().empty())
        return IBriefcaseManager::Response (purpose, req.Options(), RepositoryStatus::Success);

    if (m_connection.IsNull())
        return Response(purpose, req.Options(), RepositoryStatus::ServerUnavailable);
    Utf8String lastChangeSetId = db.Revisions ().GetParentRevisionId ();

    StatusResult result;
    if (queryOnly)
        result = m_connection->QueryCodesLocksAvailability(req.Locks(), req.Codes(), db.GetBriefcaseId(), db.GetDbGuid(), lastChangeSetId, req.Options(), m_cancellationToken)->GetResult();
    else
        // NEEDSWORK: pass ResponseOptions to make sure we do not return locks if they are not needed. This is currently not supported by WSG.
        result = m_connection->AcquireCodesLocks (req.Locks (), req.Codes(), db.GetBriefcaseId (), db.GetDbGuid(), lastChangeSetId, req.Options(), m_cancellationToken)->GetResult ();
    if (result.IsSuccess ())
        {
        return IBriefcaseManager::Response (purpose, req.Options(), RepositoryStatus::Success);
        }
    
    return HandleError(req, result, purpose);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus iModelManager::_Demote (DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
    {
    if (locks.empty () && codes.empty())
        return RepositoryStatus::Success;

    if (m_connection.IsNull())
        return RepositoryStatus::ServerUnavailable;

    // NEEDSWORK_LOCKS: Handle codes
    auto result = m_connection->DemoteCodesLocks (locks, codes, db.GetBriefcaseId (), db.GetDbGuid(), ResponseOptions::None, m_cancellationToken)->GetResult ();
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
RepositoryStatus iModelManager::_Relinquish (Resources which, DgnDbR db)
    {
    if (Resources::None == which) 
        return RepositoryStatus::Success;

    if (m_connection.IsNull())
        return RepositoryStatus::ServerUnavailable;

    auto result = m_connection->RelinquishCodesLocksInternal (which, db.GetBriefcaseId (), ResponseOptions::None, m_cancellationToken)->GetResult ();
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
RepositoryStatus iModelManager::_QueryHeldResources (DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db)
    {
    if (m_connection.IsNull())
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
RepositoryStatus iModelManager::_QueryStates (DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes)
    {
    if (locks.empty () && codes.empty())
        return RepositoryStatus::Success;

    if (m_connection.IsNull())
        return RepositoryStatus::ServerUnavailable;

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
