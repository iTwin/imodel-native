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
#include <WebServices/Connect/ConnectSignInManager.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM


//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
void DgnDbLockSetResultInfo::AddLock(const DgnLock dgnLock, BeBriefcaseId briefcaseId, Utf8StringCR repositoryId)
    {
    m_locks.insert(dgnLock);
    AddLockInfoToList(m_lockStates, dgnLock, briefcaseId, repositoryId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
const DgnLockSet& DgnDbLockSetResultInfo::GetLocks() const { return m_locks; }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
const DgnLockInfoSet& DgnDbLockSetResultInfo::GetLockStates() const { return m_lockStates; }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnection::DgnDbRepositoryConnection
(
RepositoryInfoCR           repository,
WebServices::CredentialsCR credentials,
WebServices::ClientInfoPtr clientInfo,
AuthenticationHandlerPtr   authenticationHandler
) : m_repositoryInfo(repository)
    {
    m_wsRepositoryClient = WSRepositoryClient::Create(repository.GetServerURL(), repository.GetWSRepositoryName(), clientInfo, nullptr, authenticationHandler);
    m_wsRepositoryClient->SetCredentials(credentials);
    //m_eventServiceClient = nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
void DgnDbRepositoryConnection::SetAzureClient(WebServices::IAzureBlobStorageClientPtr azureClient)
    {
    m_azureClient = azureClient;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
void RepositoryInfoParser(RepositoryInfoR repositoryInfo, Utf8StringCR repositoryUrl, Utf8StringCR repositoryId, JsonValueCR value)
    {
    DateTime createdDate = DateTime();
    DateTime::FromString(createdDate, static_cast<Utf8CP>(value[ServerSchema::Property::CreatedDate].asCString()));
    repositoryInfo = RepositoryInfo(repositoryUrl, repositoryId, value[ServerSchema::Property::RepositoryName].asString(), value[ServerSchema::Property::FileId].asString(),
                                    value[ServerSchema::Property::URL].asString(), value[ServerSchema::Property::FileName].asString(), value[ServerSchema::Property::Description].asString(),
                                    value[ServerSchema::Property::MergedRevisionId].asString(), value[ServerSchema::Property::UserCreated].asString(), createdDate);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::UpdateRepositoryInfo(ICancellationTokenPtr cancellationToken)
    {
    ObjectId repositoryObject(ServerSchema::Schema::Repository, ServerSchema::Class::File, "");
    return m_wsRepositoryClient->SendGetObjectRequest(repositoryObject, nullptr, cancellationToken)->Then<DgnDbServerStatusResult>([=] (const WSObjectsResult& response)
        {
        if (response.IsSuccess())
            {
            JsonValueCR instance = response.GetValue().GetJsonValue()[ServerSchema::Instances][0];
            RepositoryInfoParser(m_repositoryInfo, m_repositoryInfo.GetServerURL(), instance[ServerSchema::InstanceId].asString(), instance[ServerSchema::Properties]);
            return DgnDbServerStatusResult::Success();
            }
        return DgnDbServerStatusResult::Error(response.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionTaskPtr DgnDbRepositoryConnection::Create
(
RepositoryInfoCR         repository,
CredentialsCR            credentials,
ClientInfoPtr            clientInfo,
ICancellationTokenPtr    cancellationToken,
AuthenticationHandlerPtr authenticationHandler
)
    {
    if (repository.GetId().empty())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::InvalidRepostioryName));
        }
    if (repository.GetServerURL().empty())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!credentials.IsValid() && !authenticationHandler)
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }

    DgnDbRepositoryConnectionPtr repositoryConnection(new DgnDbRepositoryConnection(repository, credentials, clientInfo, authenticationHandler));
    if (!repository.GetFileId().empty())
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Success(repositoryConnection));

    return repositoryConnection->UpdateRepositoryInfo(cancellationToken)->Then<DgnDbRepositoryConnectionResult>([=] (DgnDbServerStatusResultCR result)
        {
        if (!result.IsSuccess())
            return DgnDbRepositoryConnectionResult::Error(result.GetError());

        //if (!repositoryConnection->GetRepositoryInfo().GetFileURL().empty())
        // WARNING: Temporarily commenting this out, should be uncommented
        //if (Utf8String::npos != repositoryConnection->GetRepositoryInfo().GetServerURL().rfind ("cloudapp.net"))
        repositoryConnection->SetAzureClient(AzureBlobStorageClient::Create());

        return DgnDbRepositoryConnectionResult::Success(repositoryConnection);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusResult DgnDbRepositoryConnection::WriteBriefcaseIdIntoFile
(
BeFileName                     filePath,
BeBriefcaseId                  briefcaseId
) const
    {
    BeSQLite::DbResult status;

    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    DgnDbServerHost::Adopt(host);

    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
    DgnDbServerStatusResult result;
    if (BeSQLite::DbResult::BE_SQLITE_OK == status && db.IsValid())
        {
        result = RepositoryInfo::WriteRepositoryInfo(*db, m_repositoryInfo, briefcaseId);
        db->CloseDb();
        }
    else
        {
        result = DgnDbServerStatusResult::Error(DgnDbServerError(db, status));
        if (db.IsValid())
            db->CloseDb();
        }

    DgnDbServerHost::Forget(host, true);
    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadBriefcaseFile
(
BeFileName                      localFile,
BeBriefcaseId                   briefcaseId,
Utf8StringCR                    url,
HttpRequest::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    if (url.empty())
        {
        Utf8String instanceId;
        instanceId.Sprintf("%u", briefcaseId.GetValue());
        ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase, instanceId);
        return m_wsRepositoryClient->SendGetFileRequest(fileObject, localFile, nullptr, callback, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const WSFileResult& fileResult)
            {
            if (fileResult.IsSuccess())
                return WriteBriefcaseIdIntoFile(fileResult.GetValue().GetFilePath(), briefcaseId);
            else
                return DgnDbServerStatusResult::Error(fileResult.GetError());
            });
        }
    else
        {
        // Download file directly from the url.
        return m_azureClient->SendGetFileRequest(url, localFile, callback, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const AzureResult& result)
            {
            if (result.IsSuccess())
                return WriteBriefcaseIdIntoFile(localFile, briefcaseId);
            else
                return DgnDbServerStatusResult::Error(DgnDbServerError(result.GetError()));
            });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadRevisionFile
(
DgnDbServerRevisionPtr          revision,
HttpRequest::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revision->GetRevision()->GetId());

    if (revision->GetURL().empty())
        {
        return m_wsRepositoryClient->SendGetFileRequest(fileObject, revision->GetRevision()->GetChangeStreamFile(), nullptr, callback, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const WSFileResult& fileResult)
            {
            if (fileResult.IsSuccess())
                return DgnDbServerStatusResult::Success();
            else
                return DgnDbServerStatusResult::Error(fileResult.GetError());
            });
        }
    else
        {
        // Download file directly from the url.
        return m_azureClient->SendGetFileRequest(revision->GetURL(), revision->GetRevision()->GetChangeStreamFile(), nullptr, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const AzureResult& result)
            {
            if (result.IsSuccess())
                return DgnDbServerStatusResult::Success();
            else
                return DgnDbServerStatusResult::Error(DgnDbServerError(result.GetError()));
            });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Json::Value CreateLockInstanceJson
(
bvector<uint64_t> const& ids,
BeBriefcaseId            briefcaseId,
Utf8StringCR             description,
Utf8StringCR             releasedWithRevisionId,
LockableType             type,
LockLevel                level
)
    {
    Json::Value properties;

    properties[ServerSchema::Property::Description] = description;
    properties[ServerSchema::Property::BriefcaseId] = briefcaseId.GetValue();
    properties[ServerSchema::Property::ReleasedWithRevision] = releasedWithRevisionId;
    RepositoryJson::LockableTypeToJson(properties[ServerSchema::Property::LockType], type);
    RepositoryJson::LockLevelToJson(properties[ServerSchema::Property::LockLevel], level);

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
void AddToInstance
(
WSChangeset&                     changeset,
WSChangeset::ChangeState const&  changeState,
bvector<uint64_t> const&         ids,
BeBriefcaseId                    briefcaseId,
Utf8StringCR                     description,
Utf8StringCR                     releasedWithRevisionId,
LockableType                     type,
LockLevel                        level
)
    {
    if (ids.empty())
        return;
    ObjectId lockObject(ServerSchema::Schema::Repository, ServerSchema::Class::MultiLock, "MultiLock");
    changeset.AddInstance(lockObject, changeState, std::make_shared<Json::Value>(CreateLockInstanceJson(ids, briefcaseId, description, releasedWithRevisionId, type, level)));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
void SetLocksJsonRequestToChangeSet
(
const DgnLockSet&               locks,
BeBriefcaseId                   briefcaseId,
Utf8StringCR                    releasedWithRevisionId,
WSChangeset&                    changeset,
const WSChangeset::ChangeState& changeState,
bool                            includeOnlyExclusive = false
)
    {
    bvector<uint64_t> objects[9];
    for (auto& lock : locks)
        {
        if (includeOnlyExclusive && LockLevel::Exclusive != lock.GetLevel())
            continue;

        int index = static_cast<int32_t>(lock.GetType()) * 3 + static_cast<int32_t>(lock.GetLevel());
        if (index >= 0 && index <= 8)
            objects[index].push_back(lock.GetId().GetValue());
        }

    Utf8String description = ""; //needswork: Currently DgnDb doesn't pass us a description for locks. Do we really need it?

    for (int i = 0; i < 9; ++i)
        AddToInstance(changeset, changeState, objects[i], briefcaseId, description, releasedWithRevisionId, static_cast<LockableType>(i / 3), static_cast<LockLevel>(i % 3));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<WSChangeset> LockDeleteAllJsonRequest(const BeBriefcaseId& briefcaseId)
    {
    Utf8String id;
    id.Sprintf("%s-%d", ServerSchema::DeleteAllLocks, briefcaseId.GetValue());

    ObjectId lockObject(ServerSchema::Schema::Repository, ServerSchema::Class::Lock, id);

    Json::Value properties;
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    changeset->AddInstance(lockObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value>(properties));

    return changeset;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::SendChangesetRequest(std::shared_ptr<WSChangeset> changeset, ICancellationTokenPtr cancellationToken) const
    {
    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<DgnDbServerStatusResult>
        ([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            return DgnDbServerStatusResult::Success();
        else
            return DgnDbServerStatusResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::AcquireLocks
(
LockRequestCR         locks,
BeBriefcaseId         briefcaseId,
Utf8StringCR          lastRevisionId,
ICancellationTokenPtr cancellationToken
) const
    {
    //How to set description here?
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetLocksJsonRequestToChangeSet(locks.GetLockSet(), briefcaseId, lastRevisionId, *changeset, WSChangeset::ChangeState::Modified);
    return SendChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DemoteLocks
(
const DgnLockSet&     locks,
BeBriefcaseId         briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    //How to set description here?
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetLocksJsonRequestToChangeSet(locks, briefcaseId, "", *changeset, WSChangeset::ChangeState::Modified);
    return SendChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::RelinquishLocks
(
BeBriefcaseId         briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    auto changeset = LockDeleteAllJsonRequest(briefcaseId);
    return SendChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerLockSetTaskPtr DgnDbRepositoryConnection::QueryLocks
(
BeBriefcaseId         briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryLocksInternal(nullptr, &briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerLockSetTaskPtr DgnDbRepositoryConnection::QueryLocksById
(
LockableIdSet const&  ids,
BeBriefcaseId         briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryLocksInternal(&ids, &briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerLockSetTaskPtr DgnDbRepositoryConnection::QueryLocksById
(
LockableIdSet const&  ids,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryLocksInternal(&ids, nullptr, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerLockSetTaskPtr DgnDbRepositoryConnection::QueryLocksInternal
(
LockableIdSet const*  ids,
const BeBriefcaseId*  briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Lock);
    Utf8String filter;

    //Format the filter
    if (nullptr == ids && nullptr != briefcaseId)
        {
        filter.Sprintf("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue());
        query.SetFilter(filter);
        }
    else if (nullptr != ids)
        {
        bool first = true;
        Utf8String idsString;
        for (auto id : *ids)
            {
            Utf8String idString;
            if (nullptr == briefcaseId)
                idString.Sprintf("'%d-%llu'", (int) id.GetType(), id.GetId().GetValue());
            else
                idString.Sprintf("'%d-%llu-%u'", (int) id.GetType(), id.GetId().GetValue(), briefcaseId->GetValue());

            if (!first)
                idsString.append(",");
            idsString.append(idString);

            first = false;
            }

        filter.Sprintf("$id+in+[%s]", idsString.c_str());
        query.SetFilter(filter);
        }

    //Execute query
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerLockSetResult>
        ([=] (const WSObjectsResult& result)
        {
        if (result.IsSuccess())
            {
            DgnDbLockSetResultInfo locks;
            for (auto& value : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                {
                DgnLock        lock;
                BeBriefcaseId  briefcaseId;
                Utf8String     repositoryId;
                if (!GetLockFromServerJson(value[ServerSchema::Properties], lock, briefcaseId, repositoryId))
                    continue;//NEEDSWORK: log an error

                if (lock.GetLevel() != LockLevel::None)
                    locks.AddLock(lock, briefcaseId, repositoryId);
                }
            return DgnDbServerLockSetResult::Success(locks);
            }
        else
            return DgnDbServerLockSetResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<WSCreateObjectResult> DgnDbRepositoryConnection::AcquireBriefcaseId(ICancellationTokenPtr cancellationToken) const
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
//@bsimethod                                     Eligijus.Mauragas              03/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::GetAllRevisions(ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
    return RevisionsFromQuery(query, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerUInt64TaskPtr DgnDbRepositoryConnection::GetRevisionIndex
(
Utf8StringCR          revisionId,
ICancellationTokenPtr cancellationToken
) const
    {
    if (revisionId.empty())
        {
        return CreateCompletedAsyncTask<DgnDbServerUInt64Result>(DgnDbServerUInt64Result::Success(0));
        }
    ObjectId revisionObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionId);
    return m_wsRepositoryClient->SendGetObjectRequest(revisionObject, nullptr, cancellationToken)->Then<DgnDbServerUInt64Result>
        ([=] (WSObjectsResult& revisionResult)
        {
        if (revisionResult.IsSuccess())
            {
            uint64_t index = 0;
            JsonValueCR instances = revisionResult.GetValue().GetJsonValue()[ServerSchema::Instances];
            if (instances.isValidIndex(0))
                index = instances[0][ServerSchema::Properties][ServerSchema::Property::Index].asInt64() + 1;
            return DgnDbServerUInt64Result::Success(index);
            }
        else
            return DgnDbServerUInt64Result::Error(revisionResult.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionTaskPtr DgnDbRepositoryConnection::GetRevisionById
(
Utf8StringCR          revisionId,
ICancellationTokenPtr cancellationToken
) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (revisionId.empty())
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionResult>(DgnDbServerRevisionResult::Error(DgnDbServerError::Id::InvalidRevision));
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
//@bsimethod                                    Arvind.Venkateswaran            05/2016
//---------------------------------------------------------------------------------------
EventServiceClient* DgnDbRepositoryConnection::m_eventServiceClient = nullptr;

BeMutex DgnDbRepositoryConnection::s_eventSubscriptionLock;

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
//Returns true if same, else false
bool CompareEventTypes
(
bvector<DgnDbServerEvent::DgnDbServerEventType>* newEventTypes,
bvector<DgnDbServerEvent::DgnDbServerEventType> oldEventTypes
)
    {
    //Case: newEventTypes are null or size 0 AND oldEventTypes are null or size 0
    //Case: newEventTypes have different size AND oldEventTypes are null or size 0
    //Case: newEventTypes are null or size 0 AND oldEventTypes are a different size 
    //Case: newEventTypes are a different size AND oldEventTypes are a different size
    //Case: newEventTypes and oldEventTypes are same size
    if (
        (newEventTypes == nullptr || newEventTypes->size() == 0) &&
        (oldEventTypes.size() == 0)
        )
        return true;
    else if (
        (newEventTypes != nullptr && newEventTypes->size() > 0) &&
        (oldEventTypes.size() == 0)
        )
        return false;
    else if (
        (newEventTypes == nullptr || newEventTypes->size() == 0) &&
        (oldEventTypes.size() > 0)
        )
        return false;
    else if (
        (newEventTypes != nullptr && newEventTypes->size() > 0) &&
        (oldEventTypes.size() > 0) &&
        (newEventTypes->size() != oldEventTypes.size())
        )
        return false;
    else if (
        (newEventTypes != nullptr && newEventTypes->size() > 0) &&
        (oldEventTypes.size() > 0) &&
        (newEventTypes->size() == oldEventTypes.size())
        )
        {
        bmap<DgnDbServerEvent::DgnDbServerEventType, Utf8CP> oldEventTypesMap;
        for (int i = 0; i < (int) oldEventTypes.size(); i++)
            {
            oldEventTypesMap.Insert(oldEventTypes.at(i), "");
            }
        bool isSame = true;
        for (int j = 0; j < (int) newEventTypes->size(); j++)
            {
            //Item in newEventTypes not found in map
            if (oldEventTypesMap.end() == oldEventTypesMap.find(newEventTypes->at(j)))
                {
                isSame = false;
                break;
                }
            }
        return isSame;
        }
    else 
        return false;

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::SetEventServiceSubscriptionAndSAS
(
bool isCreate,
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes, 
ICancellationTokenPtr cancellationToken
)
    {
    auto eventSubscription = (isCreate) ? GetEventServiceSubscriptionId(eventTypes, cancellationToken)->GetResult() : UpdateEventServiceSubscriptionId(eventTypes, cancellationToken)->GetResult();
    if (!eventSubscription.IsSuccess())
        return false;

    m_eventSubscription = eventSubscription.GetValue();

    auto sasToken = GetEventServiceSAS(cancellationToken)->GetResult();
    if (!sasToken.IsSuccess())
        return false;

    m_eventSAS = sasToken.GetValue();

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            05/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::SetEventServiceClient
(
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
ICancellationTokenPtr cancellationToken
)
    {
    if (m_eventServiceClient == nullptr)
        {
        if (!SetEventServiceSubscriptionAndSAS(true, eventTypes, cancellationToken))
            return false;

        EventServiceClient *eventServiceClient = new EventServiceClient
            (
            m_eventSAS->GetBaseAddress(),
            m_repositoryInfo.GetId(),
            m_eventSubscription->GetSubscriptionId()
            );
        eventServiceClient->UpdateSASToken(m_eventSAS->GetSASToken());
        m_eventServiceClient = eventServiceClient;
        return true;
        }
  
    bool isSame = CompareEventTypes(eventTypes, m_eventSubscription->GetEventTypes());
    bool isSuccess = (!isSame) ? SetEventServiceSubscriptionAndSAS(false, eventTypes, cancellationToken) : true;

    if (!isSuccess)
        return false;
    if (!isSame && isSuccess)
        m_eventServiceClient->UpdateSASToken(m_eventSAS->GetSASToken());
        
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSASTaskPtr DgnDbRepositoryConnection::GetEventServiceSAS(ICancellationTokenPtr cancellationToken) const
    {
    //POST to https://{server}/{version}/Repositories/DgnDbServer--{repoId}/DgnDbServer/EventSAS
    std::shared_ptr<DgnDbServerEventSASResult> finalResult = std::make_shared<DgnDbServerEventSASResult>();
    return m_wsRepositoryClient->SendCreateObjectRequest
        (
        DgnDbServerEventParser::GetInstance().GenerateEventSASJson(),
        BeFileName(),
        nullptr,
        cancellationToken
        )
        ->Then([=] (const WSCreateObjectResult& result)
        {
        if (!result.IsSuccess())
            finalResult->SetError(result.GetError());
        DgnDbServerEventSASPtr ptr = DgnDbServerEventParser::GetInstance().ParseEventSAS(result.GetValue().GetObject());
        if (ptr == nullptr)
            finalResult->SetError(DgnDbServerError::Id::NoSASFound);
        finalResult->SetSuccess(ptr);
        })->Then<DgnDbServerEventSASResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Caleb.Shafer		             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionTaskPtr DgnDbRepositoryConnection::SendEventChangesetRequest
(
std::shared_ptr<WSChangeset> changeset,
ICancellationTokenPtr cancellationToken
) const
    {
    //PUT to https://{server}/{version}/Repositories/DgnDbServer--{repoId}/DgnDbServer/EventSubscription
    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    std::shared_ptr<DgnDbServerEventSubscriptionResult> finalResult = std::make_shared<DgnDbServerEventSubscriptionResult>();
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then([=] (const WSChangesetResult& result)
        {
        if (!result.IsSuccess())
            finalResult->SetError(result.GetError());
        DgnDbServerEventSubscriptionPtr ptr = DgnDbServerEventParser::GetInstance().ParseEventSubscription(result.GetValue()->AsJson());
        if (ptr == nullptr)
            finalResult->SetError(DgnDbServerError::Id::NoSubscriptionFound);
        finalResult->SetSuccess(ptr);
        })->Then<DgnDbServerEventSubscriptionResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Caleb.Shafer					06/2016
//---------------------------------------------------------------------------------------
void SetEventSubscriptionJsonRequestToChangeSet
(
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
Utf8String										 eventSubscriptionId,
WSChangeset&									 changeset,
const WSChangeset::ChangeState&					 changeState
)
    {
    ObjectId eventSubscriptionObject(ServerSchema::Schema::Repository, ServerSchema::Class::EventSubscription, "EventSubscription");
    changeset.AddInstance(eventSubscriptionObject, changeState, std::make_shared<Json::Value>(DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionJson(eventTypes, eventSubscriptionId)));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionTaskPtr DgnDbRepositoryConnection::GetEventServiceSubscriptionId
(
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
ICancellationTokenPtr cancellationToken
) const
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, "", *changeset, WSChangeset::Created);
    return SendEventChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer					06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionTaskPtr DgnDbRepositoryConnection::UpdateEventServiceSubscriptionId
(
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
ICancellationTokenPtr cancellationToken
) const
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, m_eventSubscription->GetSubscriptionId(), *changeset, WSChangeset::Modified);
    return SendEventChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::GetEventServiceResponse
(
HttpResponseR returnResponse,
bool longpolling
)
    {
    HttpResponse response = m_eventServiceClient->MakeReceiveDeleteRequest(longpolling);
    HttpStatus status = response.GetHttpStatus();
    if (status == HttpStatus::OK || status == HttpStatus::NoContent)
        {
        returnResponse = response;
        return true;
        }
    else
        {
        return false;
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::GetEventServiceResponses
(
bvector<Utf8String>& responseStrings,
bvector<Utf8CP>& contentTypes,
bool longpolling
)
    {
    HttpResponse response = m_eventServiceClient->MakeReceiveDeleteRequest(longpolling);
    HttpStatus status = response.GetHttpStatus();
    if (status == HttpStatus::NoContent)
        {
        return true;
        }
    else if (status == HttpStatus::OK)
        {
        responseStrings.push_back(response.GetBody().AsString());
        contentTypes.push_back(response.GetHeaders().GetContentType());
        return GetEventServiceResponses(responseStrings, contentTypes, longpolling);
        }
    else
        {
        return false;
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventTaskPtr DgnDbRepositoryConnection::GetEvent
(
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
bool longPolling,
ICancellationTokenPtr cancellationToken
)
    {
    s_eventSubscriptionLock.lock();
    bool isSuccess = SetEventServiceClient(eventTypes, cancellationToken);
    s_eventSubscriptionLock.unlock();
    if (!isSuccess)
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::InternalServerError)); 

    /*if (!SetEventServiceClient(eventTypes, cancellationToken))
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::InternalServerError));*/

    HttpResponse response;
    if (!GetEventServiceResponse(response, longPolling))
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::InternalServerError));

    if (HttpStatus::NoContent == response.GetHttpStatus())
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::NoEventsFound));

    DgnDbServerEventPtr ptr = DgnDbServerEventParser::GetInstance().ParseEvent(response.GetHeaders().GetContentType(), response.GetBody().AsString());
    if (ptr == nullptr)
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::NoEventsFound));
    return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Success(ptr));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventCollectionTaskPtr DgnDbRepositoryConnection::GetEvents
(
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
bool longPolling,
ICancellationTokenPtr cancellationToken
)
    {
    if (!SetEventServiceClient(eventTypes, cancellationToken))
        return CreateCompletedAsyncTask<DgnDbServerEventCollectionResult>(DgnDbServerEventCollectionResult::Error(DgnDbServerError::Id::InternalServerError));

    bvector<Utf8String> responseStrings;
    bvector<Utf8CP> contentTypes;
    bvector<DgnDbServerEventPtr> events;
    if (!GetEventServiceResponses(responseStrings, contentTypes, longPolling))
        return CreateCompletedAsyncTask<DgnDbServerEventCollectionResult>(DgnDbServerEventCollectionResult::Error(DgnDbServerError::Id::InternalServerError));

    for (int i = 0; i < responseStrings.size(); i++)
        {
        DgnDbServerEventPtr ptr = DgnDbServerEventParser::GetInstance().ParseEvent(contentTypes[i], responseStrings[i]);
        if (ptr == nullptr)
            continue;
        events.push_back(ptr);
        }

    if (events.size() < 1)
        return CreateCompletedAsyncTask<DgnDbServerEventCollectionResult>(DgnDbServerEventCollectionResult::Error(DgnDbServerError::Id::NoEventsFound));

    return CreateCompletedAsyncTask<DgnDbServerEventCollectionResult>(DgnDbServerEventCollectionResult::Success(events));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCancelEventTaskPtr  DgnDbRepositoryConnection::CancelEventRequest
(
ICancellationTokenPtr cancellationToken
)
    {
    //if (m_eventServiceClient == nullptr && !SetEventServiceClient(cancellationToken))
    //    return CreateCompletedAsyncTask<DgnDbServerCancelEventResult>(DgnDbServerCancelEventResult::Error(DgnDbServerError::Id::InternalServerError));
    m_eventServiceClient->CancelRequest();
    return CreateCompletedAsyncTask<DgnDbServerCancelEventResult>(DgnDbServerCancelEventResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::RevisionsFromQuery
(
const WebServices::WSQuery& query,
ICancellationTokenPtr       cancellationToken
) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
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
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::GetRevisionsAfterId
(
Utf8StringCR          revisionId,
ICancellationTokenPtr cancellationToken
) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    std::shared_ptr<DgnDbServerRevisionsResult> finalResult = std::make_shared<DgnDbServerRevisionsResult>();
    return GetRevisionIndex(revisionId, cancellationToken)->Then([=] (const DgnDbServerUInt64Result& indexResult)
        {
        if (indexResult.IsSuccess())
            {
            WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
            Utf8String queryFilter;
            queryFilter.Sprintf("Index+ge+%llu", indexResult.GetValue());
            query.SetFilter(queryFilter);
            RevisionsFromQuery(query, cancellationToken)->Then([=] (DgnDbServerRevisionsResultCR revisionsResult)
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
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadRevisions
(
const bvector<DgnDbServerRevisionPtr>& revisions,
HttpRequest::ProgressCallbackCR        callback,
ICancellationTokenPtr                  cancellationToken
) const
    {
    bset<std::shared_ptr<AsyncTask>> tasks;
    for (auto& revision : revisions)
        tasks.insert(DownloadRevisionFile(revision, callback, cancellationToken));
    return AsyncTask::WhenAll(tasks)->Then<DgnDbServerStatusResult>([=] ()
        {
        for (auto task : tasks)
            {
            auto result = dynamic_pointer_cast<DgnDbServerStatusTask>(task)->GetResult();
            if (!result.IsSuccess())
                return DgnDbServerStatusResult::Error(result.GetError());
            }
        return DgnDbServerStatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::Pull
(
Utf8StringCR                    revisionId,
HttpRequest::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    std::shared_ptr<DgnDbServerRevisionsResult> finalResult = std::make_shared<DgnDbServerRevisionsResult>();
    return GetRevisionsAfterId(revisionId, cancellationToken)->Then([=] (DgnDbServerRevisionsResultCR revisionsResult)
        {
        if (revisionsResult.IsSuccess())
            {
            DownloadRevisions(revisionsResult.GetValue(), callback, cancellationToken)->Then([=] (DgnDbServerStatusResultCR downloadResult)
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
Json::Value PushRevisionJson
(
Dgn::DgnRevisionPtr            revision,
Utf8StringCR                   repositoryName,
BeBriefcaseId                  briefcaseId
)
    {
    Json::Value pushRevisionJson = Json::objectValue;
    JsonValueR instance = pushRevisionJson[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    instance[ServerSchema::ClassName] = ServerSchema::Class::Revision;
    instance[ServerSchema::Properties] = Json::objectValue;

    JsonValueR properties = instance[ServerSchema::Properties];
    properties[ServerSchema::Property::Id] = revision->GetId();
    properties[ServerSchema::Property::Description] = revision->GetSummary();
    uint64_t size;
    revision->GetChangeStreamFile().GetFileSize(size);
    properties[ServerSchema::Property::FileSize] = size;
    properties[ServerSchema::Property::ParentId] = revision->GetParentId();
    properties[ServerSchema::Property::MasterFileId] = revision->GetDbGuid();
    properties[ServerSchema::Property::BriefcaseId] = briefcaseId.GetValue();
    properties[ServerSchema::Property::IsUploaded] = false;
    return pushRevisionJson;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              02/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::InitializeRevision
(
Dgn::DgnRevisionPtr             revision,
BeBriefcaseId                   briefcaseId,
JsonValueR                      pushJson,
ObjectId                        revisionObjectId,
HttpRequest::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    //Set Revision initialization request to ECChangeSet
    JsonValueR revisionProperties = pushJson[ServerSchema::Instance][ServerSchema::Properties];
    revisionProperties[ServerSchema::Property::IsUploaded] = true;
    changeset->AddInstance(revisionObjectId, WSChangeset::ChangeState::Modified, std::make_shared<Json::Value>(revisionProperties));

    //Set used locks to the ECChangeSet
    LockRequest usedLocks;
    usedLocks.FromRevision(*revision);
    if (!usedLocks.IsEmpty())
        SetLocksJsonRequestToChangeSet(usedLocks.GetLockSet(), briefcaseId, revision->GetId(), *changeset, WSChangeset::ChangeState::Modified, true);

    //Push Revision initialization request and Locks update in a single batch
    return SendChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::Push
(
Dgn::DgnRevisionPtr             revision,
BeBriefcaseId                   briefcaseId,
HttpRequest::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    // Stage 1. Create revision.
    std::shared_ptr<Json::Value> pushJson = std::make_shared<Json::Value>(PushRevisionJson(revision, m_repositoryInfo.GetId(), briefcaseId));
    std::shared_ptr<DgnDbServerStatusResult> finalResult = std::make_shared<DgnDbServerStatusResult>();
    return m_wsRepositoryClient->SendCreateObjectRequest(*pushJson, BeFileName(), callback, cancellationToken)
        ->Then([=] (const WSCreateObjectResult& initializePushResult)
        {
        if (!initializePushResult.IsSuccess())
            {
            finalResult->SetError(initializePushResult.GetError());
            return;
            }

        // Stage 2. Upload revision file. 
        JsonValueCR revisionInstance = initializePushResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
        Utf8String  revisionInstanceId = revisionInstance[ServerSchema::InstanceId].asString();
        ObjectId    revisionObjectId = ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionInstanceId);
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
                InitializeRevision(revision, briefcaseId, *pushJson, revisionObjectId, callback, cancellationToken)
                    ->Then([=] (DgnDbServerStatusResultCR result)
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
            m_azureClient->SendUpdateFileRequest(url, revision->GetChangeStreamFile(), callback, cancellationToken)
                ->Then([=] (const AzureResult& result)
                {
                if (!result.IsSuccess())
                    {
                    finalResult->SetError(DgnDbServerError(result.GetError()));
                    return;
                    }

                // Stage 3. Initialize revision.
                InitializeRevision(revision, briefcaseId, *pushJson, revisionObjectId, callback, cancellationToken)
                    ->Then([=] (DgnDbServerStatusResultCR result)
                    {
                    if (result.IsSuccess())
                        finalResult->SetSuccess();
                    else
                        finalResult->SetError(result.GetError());
                    });
                });
            }
        })->Then<DgnDbServerStatusResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfoCR DgnDbRepositoryConnection::GetRepositoryInfo() const
    {
    return m_repositoryInfo;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::VerifyConnection(ICancellationTokenPtr cancellationToken) const
    {
    return m_wsRepositoryClient->VerifyAccess(cancellationToken)->Then<DgnDbServerStatusResult>([] (const AsyncResult<void, WSError>& result)
        {
        if (result.IsSuccess())
            return DgnDbServerStatusResult::Success();
        else
            return DgnDbServerStatusResult::Error(result.GetError());
        });
    }

