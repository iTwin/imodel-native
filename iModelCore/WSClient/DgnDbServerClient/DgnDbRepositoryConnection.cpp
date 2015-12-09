/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbRepositoryConnection.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/RevisionManager.h>
#include <WebServices/Client/WSChangeset.h>
#include "DgnDbServerUtils.h"
#include <DgnDbServer/Client/DgnDbServerRevision.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnection::DgnDbRepositoryConnection(RepositoryInfoPtr repository, WebServices::CredentialsCR credentials,
    WebServices::ClientInfoPtr clientInfo) : m_repositoryInfo(repository)
    {
    m_wsRepositoryClient = WSRepositoryClient::Create(repository->GetServerURL(), repository->GetWSRepositoryName(), clientInfo);
    m_wsRepositoryClient->SetCredentials(credentials);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfoParser(Utf8StringCR repositoryUrl, JsonValueCR value)
    {
    DateTime uploadedDate = DateTime();
    DateTime::FromString(uploadedDate, static_cast<Utf8CP>(value[ServerSchema::Property::UploadedDate].asCString()));
    return RepositoryInfo::Create(repositoryUrl, value[ServerSchema::Property::Id].asString(), value[ServerSchema::Property::FileId].asString(),
                                  value[ServerSchema::Property::URL].asString(), value[ServerSchema::Property::Description].asString(),
                                  value[ServerSchema::Property::UserUploaded].asString(), uploadedDate);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::UpdateRepositoryInfo(ICancellationTokenPtr cancellationToken)
    {
    ObjectId repositoryObject(ServerSchema::Schema::Repository, ServerSchema::Class::File, "");
    return m_wsRepositoryClient->SendGetObjectRequest(repositoryObject, nullptr, cancellationToken)->Then<DgnDbResult>([=] (WSObjectsResult& response)
        {
        if (response.IsSuccess())
            {
            m_repositoryInfo = RepositoryInfoParser(m_repositoryInfo->GetServerURL(),
                                                    response.GetValue().GetJsonValue()[ServerSchema::Instances][0][ServerSchema::Properties]);
            return DgnDbResult::Success();
            }
        else
            {
            return DgnDbResult::Error(response.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbRepositoryConnectionResult> DgnDbRepositoryConnection::Create(RepositoryInfoPtr repository, CredentialsCR credentials,
    ClientInfoPtr clientInfo, ICancellationTokenPtr cancellationToken)
    {
    if (!repository || repository->GetServerURL().empty() || repository->GetId().empty())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(Error::InvalidRepository));
        }
    if (!credentials.IsValid())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(Error::InvalidCredentials));
        }
    DgnDbRepositoryConnectionPtr repositoryConnection(new DgnDbRepositoryConnection(repository, credentials, clientInfo));
    if (repository->GetFileId().empty())
        return repositoryConnection->UpdateRepositoryInfo(cancellationToken)->Then<DgnDbRepositoryConnectionResult>([=] (const DgnDbResult& result)
        {
        if (result.IsSuccess())
            return DgnDbRepositoryConnectionResult::Success(repositoryConnection);
        else
            return DgnDbRepositoryConnectionResult::Error(result.GetError());
        });
    else
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Success(repositoryConnection));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::DownloadBriefcaseFile(BeFileName localFile, const BeSQLite::BeBriefcaseId& briefcaseId,
    HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    Utf8String instanceId;
    instanceId.Sprintf("%d", briefcaseId.GetValue());
    ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase, instanceId);
    return m_wsRepositoryClient->SendGetFileRequest(fileObject, localFile, nullptr, callback, cancellationToken)->Then<DgnDbResult>
            ([=] (const WSFileResult& fileResult)
            {
            if (fileResult.IsSuccess())
                {
                BeFileName resultPath = fileResult.GetValue().GetFilePath();
                BeSQLite::DbResult status;

                Dgn::DgnPlatformLib::AdoptHost(DgnDbServerHost::Host());
                Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, resultPath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));

                if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                    status = RepositoryInfo::WriteRepositoryInfo(*db, *m_repositoryInfo, briefcaseId);

                if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
                    {
                    db->CloseDb();
                    Dgn::DgnPlatformLib::ForgetHost();
                    return DgnDbResult::Success();
                    }

                Utf8String error = db->GetLastError(&status);
                db->CloseDb();
                Dgn::DgnPlatformLib::ForgetHost();
                return DgnDbResult::Error(error.c_str());
                }
            else
                {
                return DgnDbResult::Error(fileResult.GetError());
                }
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::DownloadRevisionFile(Dgn::DgnRevisionPtr revision, HttpRequest::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken)
    {
    ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revision->GetId());
    
    return m_wsRepositoryClient->SendGetFileRequest(fileObject, revision->GetChangeStreamFile(), nullptr, callback, cancellationToken)->
        Then<DgnDbResult>([=] (const WSFileResult& fileResult)
        {
        if (fileResult.IsSuccess())
            {
            return DgnDbResult::Success();
            }
        else
            {
            return DgnDbResult::Error(fileResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
std::shared_ptr<WSChangeset> LockJsonRequest(JsonValueCR locks, const BeBriefcaseId& briefcaseId, const WSChangeset::ChangeState& changeState)
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    for (auto& lock : locks[Locks::Locks])
        {
        Json::Value properties;
        properties[ServerSchema::Property::Description] = locks[Locks::Description];
        properties[ServerSchema::Property::ObjectId] = lock[Locks::ObjectId][Locks::Object::Id];
        properties[ServerSchema::Property::LockType] = lock[Locks::ObjectId][Locks::Object::Type];
        properties[ServerSchema::Property::LockLevel] = lock[Locks::Level];
        properties[ServerSchema::Property::BriefcaseId] = briefcaseId.GetValue();
        ObjectId lockObject(ServerSchema::Schema::Repository, ServerSchema::Class::Lock, "");
        changeset->AddInstance(lockObject, changeState, std::make_shared<Json::Value>(properties));
        }
    return changeset;
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::QueryLocksHeld(bool& held, LockRequestCR locksRequest, const BeBriefcaseId& briefcaseId,
    ICancellationTokenPtr cancellationToken)
    {
    return QueryLocks(briefcaseId, cancellationToken)->Then<DgnDbResult>([=, &held] (const DgnDbLockSetResult& result)
        {
        if (result.IsSuccess())
            {
            auto& lockSet = result.GetValue();
            for (const auto& lock : locksRequest)
                {
                if (lockSet.end() == lockSet.find(lock))
                    {
                    held = false;
                    return DgnDbResult::Success();
                    }
                }
            held = true;
            return DgnDbResult::Success();
            }
        else
            return DgnDbResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnLockResponseResult> DgnDbRepositoryConnection::AcquireLocks(JsonValueCR locksRequest, const BeBriefcaseId& briefcaseId,
    ICancellationTokenPtr cancellationToken)
    {
    //How to set description here?
    auto changeset = LockJsonRequest(locksRequest, briefcaseId, WSChangeset::ChangeState::Created);
    Json::Value requestJson;
    changeset->ToRequestJson(requestJson);
    HttpStringBodyPtr request = HttpStringBody::Create(requestJson.toStyledString());
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<DgnLockResponseResult>
        ([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            {
            LockRequest::Response response;
            Json::Value responseJson;
            responseJson[Locks::Status] = static_cast<uint32_t>(LockStatus::Success);
            response.FromJson(responseJson);
            return DgnLockResponseResult::Success(response);
            }
        else
            {
            return DgnLockResponseResult::Error(result.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::ReleaseLocks(JsonValueCR locksRequest, const BeBriefcaseId& briefcaseId,
    ICancellationTokenPtr cancellationToken)
    {
    //How to set description here?
    auto changeset = LockJsonRequest(locksRequest, briefcaseId, WSChangeset::ChangeState::Deleted);
    Json::Value requestJson;
    changeset->ToRequestJson(requestJson);
    HttpStringBodyPtr request = HttpStringBody::Create(requestJson.toStyledString());
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<DgnDbResult>([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            {
            return DgnDbResult::Success();
            }
        else
            {
            return DgnDbResult::Error(result.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::RelinquishLocks(const BeBriefcaseId& briefcaseId, ICancellationTokenPtr cancellationToken)
    {
    Utf8String id;
    id.Sprintf("%s-%d", ServerSchema::DeleteAllLocks, briefcaseId.GetValue());
    ObjectId lockObject(ServerSchema::Schema::Repository, ServerSchema::Class::Lock, id);
    return m_wsRepositoryClient->SendDeleteObjectRequest(lockObject, cancellationToken)->Then<DgnDbResult>
        ([=] (const AsyncResult<void, WSError>& result)
        {
        if (result.IsSuccess())
            return DgnDbResult::Success();
        else
            return DgnDbResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbOwnershipResult> DgnDbRepositoryConnection::QueryOwnership(LockableId lockId, ICancellationTokenPtr cancellationToken)
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Lock);
    Utf8String id, briefcase;
    id.Sprintf("%s+eq+%llu", ServerSchema::Property::ObjectId, lockId.GetId().GetValue());
    query.SetFilter(id);
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbOwnershipResult>
        ([=] (const WSObjectsResult& result)
        {
        if (result.IsSuccess())
            {
            if (result.GetValue().GetJsonValue()[ServerSchema::Instances].isValidIndex(0))
                {
                JsonValueCR value = result.GetValue().GetJsonValue()[ServerSchema::Instances][0][ServerSchema::Properties];
                if (LockLevel::Exclusive == static_cast<LockLevel>(value[ServerSchema::Property::LockLevel].asUInt()))
                    return DgnDbOwnershipResult::Success(DgnLockOwnership(BeBriefcaseId(value[ServerSchema::Property::BriefcaseId].asUInt())));
                else
                    {
                    DgnLockOwnership ownership;
                    for (auto const& lock : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                        ownership.AddSharedOwner(BeBriefcaseId(lock[ServerSchema::Properties][ServerSchema::Property::BriefcaseId].asUInt()));
                    return DgnDbOwnershipResult::Success(ownership);
                    }
                }
            else
                return DgnDbOwnershipResult::Success(DgnLockOwnership());
            }
        else
            return DgnDbOwnershipResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbLockLevelResult> DgnDbRepositoryConnection::QueryLockLevel(Dgn::LockableId lockId, const BeSQLite::BeBriefcaseId& briefcaseId,
    ICancellationTokenPtr cancellationToken)
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Lock);
    Utf8String id, briefcase;
    id.Sprintf("%s+eq+%llu", ServerSchema::Property::ObjectId, lockId.GetId().GetValue());
    briefcase.Sprintf("%s+eq+%lu", ServerSchema::Property::BriefcaseId, briefcaseId.GetValue());
    query.SetFilter(id);
    query.SetFilter(briefcase);
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbLockLevelResult>
        ([=] (const WSObjectsResult& result)
        {
        if (result.IsSuccess())
            {
            JsonValueCR instances = result.GetValue().GetJsonValue()[ServerSchema::Instances];
            if (instances.isValidIndex(0))
                {
                int32_t value = instances[0][ServerSchema::Properties][ServerSchema::Property::LockLevel].asInt();
                LockLevel level = static_cast<LockLevel>(value);
                return DgnDbLockLevelResult::Success(level);
                }
            else
                return DgnDbLockLevelResult::Success(LockLevel::None);
            }
        else
            return DgnDbLockLevelResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbLockSetResult> DgnDbRepositoryConnection::QueryLocks(const BeBriefcaseId& briefcaseId, ICancellationTokenPtr cancellationToken)
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Lock);
    Utf8String briefcase;
    briefcase.Sprintf("%s+eq+%lu", ServerSchema::Property::BriefcaseId, briefcaseId.GetValue());
    query.SetFilter(briefcase);
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbLockSetResult>
        ([=] (const WSObjectsResult& result)
        {
        if (result.IsSuccess())
            {
            DgnLockSet lockSet;
            for (auto& value : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                {
                DgnLock lock;
                Json::Value lockJson;
                JsonValueCR properties = value[ServerSchema::Properties];
                lockJson[Locks::Level] = properties[ServerSchema::Property::LockLevel];
                lockJson[Locks::ObjectId] = Json::objectValue;
                lockJson[Locks::ObjectId][Locks::Object::Id] = properties[ServerSchema::Property::ObjectId];
                lockJson[Locks::ObjectId][Locks::Object::Type] = properties[ServerSchema::Property::LockType];
                lock.FromJson(lockJson);
                lockSet.insert(lock);
                }
            return DgnDbLockSetResult::Success(lockSet);
            }
        else
            return DgnDbLockSetResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<WSCreateObjectResult> DgnDbRepositoryConnection::AcquireBriefcaseId(ICancellationTokenPtr cancellationToken)
    {
    Json::Value briefcaseIdJson = Json::objectValue;
    Json::Value instance = briefcaseIdJson[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    instance[ServerSchema::ClassName] = ServerSchema::Class::Briefcase;
    return m_wsRepositoryClient->SendCreateObjectRequest(briefcaseIdJson, BeFileName(), nullptr, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionPtr ParseRevision(JsonValueCR jsonValue)
    {
    Dgn::DgnPlatformLib::AdoptHost(DgnDbServerHost::Host());
    RevisionStatus status;
    DgnDbServerRevisionPtr indexedRevision = DgnDbServerRevision::Create(DgnRevision::Create(&status, jsonValue[ServerSchema::Property::Id].asString(),
    jsonValue[ServerSchema::Property::ParentId].asString(), jsonValue[ServerSchema::Property::MasterFileId].asString()));
    Dgn::DgnPlatformLib::ForgetHost();
    if (RevisionStatus::Success == status)
        {
        indexedRevision->GetRevision()->SetSummary(jsonValue[ServerSchema::Property::Description].asCString());
        DateTime pushDate = DateTime();
        DateTime::FromString(pushDate, jsonValue[ServerSchema::Property::PushDate].asCString());
        indexedRevision->GetRevision()->SetDateTime(pushDate);
        indexedRevision->GetRevision()->SetUserName(jsonValue[ServerSchema::Property::UserCreated].asCString());
        indexedRevision->SetIndex(jsonValue[ServerSchema::Property::Index].asUInt64());
        indexedRevision->SetURL(jsonValue["URL"].asString());
        }
    return indexedRevision;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbUInt64Result> DgnDbRepositoryConnection::GetRevisionIndex(Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken)
    {
    if (revisionId.empty())
        {
        return CreateCompletedAsyncTask<DgnDbUInt64Result>(DgnDbUInt64Result::Success(0));
        }
    ObjectId revisionObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionId);
    return m_wsRepositoryClient->SendGetObjectRequest(revisionObject, nullptr, cancellationToken)->Then<DgnDbUInt64Result>
        ([=] (WSObjectsResult& revisionResult)
        {
        if (revisionResult.IsSuccess())
            {
            uint64_t index = 0;
            JsonValueCR instances = revisionResult.GetValue().GetJsonValue()[ServerSchema::Instances];
            if (instances.isValidIndex(0))
                index = instances[0][ServerSchema::Properties][ServerSchema::Property::Index].asInt64() + 1;
            return DgnDbUInt64Result::Success(index);
            }
        else
            return DgnDbUInt64Result::Error(revisionResult.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbServerRevisionResult> DgnDbRepositoryConnection::GetRevisionById(Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    if (revisionId.empty())
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionResult>(DgnDbServerRevisionResult::Error(Error::InvalidRevision));
        }
    ObjectId revisionObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionId);
    return m_wsRepositoryClient->SendGetObjectRequest(revisionObject, nullptr, cancellationToken)->Then<DgnDbServerRevisionResult>
        ([=] (WSObjectsResult& revisionResult)
        {
        if (revisionResult.IsSuccess())
            {
            auto revision = ParseRevision(revisionResult.GetValue().GetJsonValue()[ServerSchema::Instances][0][ServerSchema::Properties]);
            return DgnDbServerRevisionResult::Success(revision);
            }
        else
            return DgnDbServerRevisionResult::Error(revisionResult.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbServerRevisionsResult> DgnDbRepositoryConnection::RevisionsFromQuery(const WebServices::WSQuery& query,
    ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerRevisionsResult>
        ([=] (const WSObjectsResult& revisionsInfoResult)
        {
        if (revisionsInfoResult.IsSuccess())
            {
            bvector<DgnDbServerRevisionPtr> indexedRevisions;
            for (auto& value : revisionsInfoResult.GetValue().GetJsonValue()[ServerSchema::Instances])
                indexedRevisions.push_back(ParseRevision(value[ServerSchema::Properties]));
            std::sort(indexedRevisions.begin(), indexedRevisions.end(), [] (DgnDbServerRevisionPtr a, DgnDbServerRevisionPtr b)
                {
                return a->GetIndex() < b->GetIndex();
                });
            return DgnDbServerRevisionsResult::Success(indexedRevisions);
            }
        else
            return DgnDbServerRevisionsResult::Error(revisionsInfoResult.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbServerRevisionsResult> DgnDbRepositoryConnection::GetRevisionsAfterId(Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    std::shared_ptr<DgnDbServerRevisionsResult> finalResult = std::make_shared<DgnDbServerRevisionsResult>();
    return GetRevisionIndex(revisionId, cancellationToken)->Then([=] (const DgnDbUInt64Result& indexResult)
        {
        if (indexResult.IsSuccess())
            {
            WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
            Utf8String queryFilter;
            queryFilter.Sprintf("%s%lld", "Index+ge+", indexResult.GetValue());
            query.SetFilter(queryFilter);
            RevisionsFromQuery(query, cancellationToken)->Then([=] (const DgnDbServerRevisionsResult& revisionsResult)
                {
                if (revisionsResult.IsSuccess())
                    finalResult->SetSuccess(revisionsResult.GetValue());
                else
                    finalResult->SetError(revisionsResult.GetError());
                });
            }
        else
            finalResult->SetError(indexResult.GetError());
        })->Then<DgnDbServerRevisionsResult>([=] ()
        {
        return *finalResult;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::DownloadRevisions(const bvector<DgnDbServerRevisionPtr>& revisions,
    HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    bset<std::shared_ptr<AsyncTask>> tasks;
    for (auto& revision : revisions)
        tasks.insert(DownloadRevisionFile(revision->GetRevision(), callback, cancellationToken));
    return AsyncTask::WhenAll(tasks)->Then<DgnDbResult>([=] ()
        {
        for (auto task : tasks)
            {
            auto result = dynamic_pointer_cast<PackagedAsyncTask<DgnDbResult>>(task)->GetResult();
            if (!result.IsSuccess())
                return DgnDbResult::Error(result.GetError());
            }
        return DgnDbResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbServerRevisionsResult> DgnDbRepositoryConnection::Pull(Utf8StringCR revisionId, HttpRequest::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken)
    {
    std::shared_ptr<DgnDbServerRevisionsResult> finalResult = std::make_shared<DgnDbServerRevisionsResult>();
    return GetRevisionsAfterId(revisionId, cancellationToken)->Then([=] (const DgnDbServerRevisionsResult& revisionsResult)
        {
        if (revisionsResult.IsSuccess())
            {
            DownloadRevisions(revisionsResult.GetValue(), callback, cancellationToken)->Then([=](const DgnDbResult& downloadResult)
                {
                if (downloadResult.IsSuccess())
                    finalResult->SetSuccess(revisionsResult.GetValue());
                else
                    finalResult->SetError(downloadResult.GetError());
                });
            }
        else
            finalResult->SetError(revisionsResult.GetError());
        })->Then<DgnDbServerRevisionsResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Json::Value PushRevisionJson(Dgn::DgnRevisionPtr revision, Utf8StringCR repositoryName, uint32_t repositoryId)
    {
    Json::Value pushRevisionJson = Json::objectValue;
    pushRevisionJson[ServerSchema::Instance] = Json::objectValue;
    pushRevisionJson[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    pushRevisionJson[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::Revision;
    pushRevisionJson[ServerSchema::Instance][ServerSchema::Properties] = Json::objectValue;

    pushRevisionJson[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Id] = revision->GetId();
    pushRevisionJson[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Description] = revision->GetSummary();
    uint64_t size;
    revision->GetChangeStreamFile().GetFileSize(size);
    pushRevisionJson[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileSize] = size;
    pushRevisionJson[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::ParentId] = revision->GetParentId();
    pushRevisionJson[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::MasterFileId] = revision->GetDbGuid();
    pushRevisionJson[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::BriefcaseId] = repositoryId;
    return pushRevisionJson;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::Push(Dgn::DgnRevisionPtr revision, uint32_t repositoryId,
    HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    auto pushJson = PushRevisionJson(revision, m_repositoryInfo->GetId(), repositoryId);
    return m_wsRepositoryClient->SendCreateObjectRequest(pushJson, revision->GetChangeStreamFile(), callback, cancellationToken)->Then<DgnDbResult>
        ([=] (const WSCreateObjectResult& result)
        {
        if (result.IsSuccess())
            return DgnDbResult::Success();
        else
            return DgnDbResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfoCR DgnDbRepositoryConnection::GetRepositoryInfo()
    {
    return *m_repositoryInfo;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::VerifyConnection(ICancellationTokenPtr cancellationToken)
    {
    return m_wsRepositoryClient->VerifyAccess(cancellationToken)->Then<DgnDbResult>([] (const AsyncResult<void, WSError>& result)
        {
        if (result.IsSuccess())
            return DgnDbResult::Success();
        else
            return DgnDbResult::Error(result.GetError());
        });
    }

