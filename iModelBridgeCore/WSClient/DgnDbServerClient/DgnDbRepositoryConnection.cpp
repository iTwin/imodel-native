/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbRepositoryConnection.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
IAzureBlobStorageClientPtr DgnDbRepositoryConnection::GetAzureClient()
    {
    if (nullptr == m_azureClient)
        m_azureClient = AzureBlobStorageClient::Create();

    return m_azureClient;
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
DgnDbResult DgnDbRepositoryConnection::WriteBriefcaseIdIntoFile(BeFileName filePath, const BeSQLite::BeBriefcaseId& briefcaseId)
    {
    BeSQLite::DbResult status;

    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    DgnDbServerHost::Adopt(host);
    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));

    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
        status = RepositoryInfo::WriteRepositoryInfo(*db, *m_repositoryInfo, briefcaseId);

    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        {
        db->CloseDb();
        DgnDbServerHost::Forget(host, true);
        return DgnDbResult::Success();
        }

    Utf8String error = db->GetLastError(&status);
    db->CloseDb();
    DgnDbServerHost::Forget(host, true);
    return DgnDbResult::Error(error.c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::DownloadBriefcaseFile(BeFileName localFile, const BeSQLite::BeBriefcaseId& briefcaseId,
    Utf8StringCR url, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    if (url.empty())
        {
        Utf8String instanceId;
        instanceId.Sprintf("%u", briefcaseId.GetValue());
        ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase, instanceId);
        return m_wsRepositoryClient->SendGetFileRequest(fileObject, localFile, nullptr, callback, cancellationToken)
            ->Then<DgnDbResult>([=] (const WSFileResult& fileResult)
            {
            if (fileResult.IsSuccess())
                return WriteBriefcaseIdIntoFile(fileResult.GetValue().GetFilePath(), briefcaseId);
            else
                return DgnDbResult::Error(fileResult.GetError());
            });
        }
    else
        {
        // Download file directly from the url.
        return GetAzureClient()->SendGetFileRequest(url, localFile, callback, cancellationToken)
            ->Then<DgnDbResult>([=] (const AzureResult& result)
            {
            if (result.IsSuccess())
                return WriteBriefcaseIdIntoFile(localFile, briefcaseId);
            else
                return DgnDbResult::Error(DgnDbServerError(result.GetError().GetDisplayMessage().c_str()));
            });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::DownloadRevisionFile(DgnDbServerRevisionPtr revision, HttpRequest::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken)
    {
    ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revision->GetRevision()->GetId());
    
    if (revision->GetURL().empty())
        {
        return m_wsRepositoryClient->SendGetFileRequest(fileObject, revision->GetRevision()->GetChangeStreamFile(), nullptr, callback, cancellationToken)
            ->Then<DgnDbResult>([=] (const WSFileResult& fileResult)
            {
            if (fileResult.IsSuccess())
                return DgnDbResult::Success();
            else
                return DgnDbResult::Error(fileResult.GetError());
            });
        }
    else
        {
        // Download file directly from the url.
        return GetAzureClient()->SendGetFileRequest(revision->GetURL(), revision->GetRevision()->GetChangeStreamFile(), nullptr, cancellationToken)
            ->Then<DgnDbResult>([=] (const AzureResult& result)
            {
            if (result.IsSuccess())
                return DgnDbResult::Success();
            else
                return DgnDbResult::Error(DgnDbServerError(result.GetError().GetDisplayMessage().c_str()));
            });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Json::Value CreateLockInstanceJson(bvector<uint64_t> const& ids, const BeBriefcaseId& briefcaseId, Utf8StringCR description, Utf8StringCR releasedWithRevisionId, LockableType type, LockLevel level)
    {
    Json::Value properties;

    properties[ServerSchema::Property::Description]          = description;
    properties[ServerSchema::Property::BriefcaseId]          = briefcaseId.GetValue();
    properties[ServerSchema::Property::ReleasedWithRevision] = releasedWithRevisionId;
    DgnLocksJson::LockableTypeToJson(properties[ServerSchema::Property::LockType], type);
    DgnLocksJson::LockLevelToJson(properties[ServerSchema::Property::LockLevel], level);

    properties[ServerSchema::Property::ObjectIds] = Json::arrayValue;
    int i = 0;
    for (auto const& id : ids)
        {
        Utf8String idStr;
        idStr.Sprintf("%llu", id);
        properties[ServerSchema::Property::ObjectIds][i++] = idStr;
        }

    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
void AddToInstance(std::shared_ptr<WSChangeset> changeset, WSChangeset::ChangeState const& changeState, bvector<uint64_t> const& ids,
                   const BeBriefcaseId& briefcaseId, Utf8StringCR description, Utf8StringCR releasedWithRevisionId, LockableType type, LockLevel level)
    {
    if (ids.empty())
        return;
    ObjectId lockObject(ServerSchema::Schema::Repository, ServerSchema::Class::MultiLock, "");
    changeset->AddInstance(lockObject, changeState, std::make_shared<Json::Value>(CreateLockInstanceJson(ids, briefcaseId, description, releasedWithRevisionId, type, level)));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
std::shared_ptr<WSChangeset> LockJsonRequest(JsonValueCR locks, const BeBriefcaseId& briefcaseId, Utf8StringCR releasedWithRevisionId, const WSChangeset::ChangeState& changeState)
    {
    bvector<uint64_t> objects[6];

    for (auto& lock : locks[Locks::Locks])
        {
        LockableType type;
        DgnLocksJson::LockableTypeFromJson(type, lock[Locks::LockableId][Locks::Lockable::Type]);
        LockLevel level;
        DgnLocksJson::LockLevelFromJson(level, lock[Locks::Level]);
        uint64_t id = lock[Locks::LockableId][Locks::Lockable::Id].asUInt64();
        int index = static_cast<int32_t>(type) * 2 + static_cast<int32_t>(level) - 1;
        if (index >= 0 && index <= 6)
            objects[index].push_back(id);
        }

    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    Utf8String description = locks[Locks::Description].asString();
    AddToInstance(changeset, changeState, objects[0], briefcaseId, description, releasedWithRevisionId, LockableType::Db,      LockLevel::Shared);
    AddToInstance(changeset, changeState, objects[1], briefcaseId, description, releasedWithRevisionId, LockableType::Db,      LockLevel::Exclusive);
    AddToInstance(changeset, changeState, objects[2], briefcaseId, description, releasedWithRevisionId, LockableType::Model,   LockLevel::Shared);
    AddToInstance(changeset, changeState, objects[3], briefcaseId, description, releasedWithRevisionId, LockableType::Model,   LockLevel::Exclusive);
    AddToInstance(changeset, changeState, objects[4], briefcaseId, description, releasedWithRevisionId, LockableType::Element, LockLevel::Shared);
    AddToInstance(changeset, changeState, objects[5], briefcaseId, description, releasedWithRevisionId, LockableType::Element, LockLevel::Exclusive);

    return changeset;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<WSChangeset> LockDeleteAllJsonRequest (const BeBriefcaseId& briefcaseId, Utf8StringCR releasedWithRevisionId)
    {
    Utf8String id;
    id.Sprintf ("%s-%d", ServerSchema::DeleteAllLocks, briefcaseId.GetValue ());

    ObjectId lockObject (ServerSchema::Schema::Repository, ServerSchema::Class::Lock, id);

    Json::Value properties;
    properties[ServerSchema::Property::ReleasedWithRevision] = releasedWithRevisionId;

    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    changeset->AddInstance (lockObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value> (properties));

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
    Utf8StringCR lastRevisionId, ICancellationTokenPtr cancellationToken)
    {
    //How to set description here?
    auto changeset = LockJsonRequest(locksRequest, briefcaseId, lastRevisionId, WSChangeset::ChangeState::Created);
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
    Utf8StringCR releasedWithRevisionId, ICancellationTokenPtr cancellationToken)
    {
    //How to set description here?
    auto changeset = LockJsonRequest(locksRequest, briefcaseId, releasedWithRevisionId, WSChangeset::ChangeState::Deleted);
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
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::RelinquishLocks(const BeBriefcaseId& briefcaseId, Utf8StringCR releasedWithRevisionId, ICancellationTokenPtr cancellationToken)
    {
    auto changeset = LockDeleteAllJsonRequest (briefcaseId, releasedWithRevisionId);
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
AsyncTaskPtr<DgnDbOwnershipResult> DgnDbRepositoryConnection::QueryOwnership(LockableId lockId, ICancellationTokenPtr cancellationToken)
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Lock);
    Utf8String id, briefcase;
    id.Sprintf("%s+eq+%d+and+%s+eq+%llu", ServerSchema::Property::LockType, static_cast<int32_t>(lockId.GetType()), ServerSchema::Property::ObjectId, lockId.GetId().GetValue());
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
    id.Sprintf("%s+eq+%d+and+%s+eq+%llu+and+%s+eq+%u", ServerSchema::Property::LockType, static_cast<int32_t>(lockId.GetType()), ServerSchema::Property::ObjectId,
               lockId.GetId().GetValue(), ServerSchema::Property::BriefcaseId, briefcaseId.GetValue());
    query.SetFilter(id);
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
    briefcase.Sprintf("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId.GetValue());
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
                lockJson[Locks::Level] = properties[ServerSchema::Property::LockLevel].asInt();
                lockJson[Locks::LockableId] = Json::objectValue;
                uint64_t lockableId;
                BeStringUtilities::ParseUInt64(lockableId, properties[ServerSchema::Property::ObjectId].asCString());
                lockJson[Locks::LockableId][Locks::Lockable::Id] = lockableId;
                lockJson[Locks::LockableId][Locks::Lockable::Type] = properties[ServerSchema::Property::LockType].asInt();
                lock.FromJson(lockJson);
                if (lock.GetLevel() != LockLevel::None)
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
    briefcaseIdJson[ServerSchema::Instance] = Json::objectValue;
    briefcaseIdJson[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    briefcaseIdJson[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::Briefcase;
    return m_wsRepositoryClient->SendCreateObjectRequest(briefcaseIdJson, BeFileName(), nullptr, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionPtr ParseRevision(JsonValueCR jsonValue)
    {
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    DgnDbServerHost::Adopt(host);
    RevisionStatus status;
    DgnDbServerRevisionPtr indexedRevision = DgnDbServerRevision::Create(DgnRevision::Create(&status, jsonValue[ServerSchema::Property::Id].asString(),
    jsonValue[ServerSchema::Property::ParentId].asString(), jsonValue[ServerSchema::Property::MasterFileId].asString()));
    DgnDbServerHost::Forget(host);
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
            queryFilter.Sprintf("Index+ge+%llu", indexResult.GetValue());
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
        tasks.insert(DownloadRevisionFile(revision, callback, cancellationToken));
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
    pushRevisionJson[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::IsUploaded] = false;
    return pushRevisionJson;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> InitializeRevision(Json::Value pushJson, ObjectId revisionObjectId, IWSRepositoryClientPtr client,
    HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    Json::Value revisionProperties = Json::Value(pushJson[ServerSchema::Instance][ServerSchema::Properties]);
    revisionProperties[ServerSchema::Property::IsUploaded] = true;
    return client->SendUpdateObjectRequest(revisionObjectId, revisionProperties, nullptr, callback, cancellationToken)
        ->Then<DgnDbResult>([=] (const WSUpdateObjectResult& finishPushResult)
        {
        if (finishPushResult.IsSuccess())
            return DgnDbResult::Success();
        else
            return DgnDbResult::Error(finishPushResult.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::Push(Dgn::DgnRevisionPtr revision, uint32_t repositoryId,
    HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    // Stage 1. Create revision.
    auto pushJson = PushRevisionJson(revision, m_repositoryInfo->GetId(), repositoryId);
    std::shared_ptr<DgnDbResult> finalResult = std::make_shared<DgnDbResult>();
    return m_wsRepositoryClient->SendCreateObjectRequest(pushJson, BeFileName(), callback, cancellationToken)
        ->Then([=] (const WSCreateObjectResult& initializePushResult)
        {
        if (!initializePushResult.IsSuccess())
            {
            finalResult->SetError(initializePushResult.GetError());
            return;
            }

        // Stage 2. Upload revision file. 
        JsonValueCR revisionInstance   = initializePushResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
        Utf8String  revisionInstanceId = revisionInstance[ServerSchema::InstanceId].asString();
        ObjectId    revisionObjectId   = ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionInstanceId);
        Utf8StringCR url = revisionInstance[ServerSchema::Properties][ServerSchema::Property::URL].asString();

        if (url.empty())
            {
            m_wsRepositoryClient->SendUpdateFileRequest(revisionObjectId, revision->GetChangeStreamFile(), callback, cancellationToken)
                ->Then([=] (const WSUpdateObjectResult& uploadRevisionResult)
                {
                if (!uploadRevisionResult.IsSuccess())
                    {
                    finalResult->SetError(uploadRevisionResult.GetError());
                    return;
                    }

                // Stage 3. Initialize revision.
                InitializeRevision(pushJson, revisionObjectId, m_wsRepositoryClient, callback, cancellationToken)
                    ->Then([=] (const DgnDbResult& result)
                    {
                    if (result.IsSuccess())
                        finalResult->SetSuccess();
                    else
                        finalResult->SetError(result.GetError());
                    });
                });
            }
        else
            {
            GetAzureClient()->SendUpdateFileRequest(url, revision->GetChangeStreamFile(), callback, cancellationToken)
                ->Then([=] (const AzureResult& result)
                {
                if (!result.IsSuccess())
                    {
                    finalResult->SetError(DgnDbServerError(result.GetError().GetDisplayMessage().c_str()));
                    return;
                    }

                // Stage 3. Initialize revision.
                InitializeRevision(pushJson, revisionObjectId, m_wsRepositoryClient, callback, cancellationToken)
                    ->Then([=] (const DgnDbResult& result)
                    {
                    if (result.IsSuccess())
                        finalResult->SetSuccess();
                    else
                        finalResult->SetError(result.GetError());
                    });
                });
            }
        })->Then<DgnDbResult>([=]
            {
            return *finalResult;
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

