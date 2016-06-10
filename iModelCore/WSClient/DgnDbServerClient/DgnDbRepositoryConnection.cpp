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
void DgnDbLockSetResultInfo::AddLock (const DgnLock dgnLock, BeBriefcaseId briefcaseId, Utf8StringCR repositoryId)
    {
    m_locks.insert (dgnLock);
    AddLockInfoToList (m_lockStates, dgnLock, briefcaseId, repositoryId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
const DgnLockSet& DgnDbLockSetResultInfo::GetLocks () const { return m_locks; }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
const DgnLockInfoSet& DgnDbLockSetResultInfo::GetLockStates () const { return m_lockStates; }


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

EventServiceClient* DgnDbRepositoryConnection::m_eventServiceClient = nullptr;

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            03/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::SetEventServiceClient(ICancellationTokenPtr cancellationToken)
    {
    //Get Event Service SASToken and Set EventService Client
    if (m_eventServiceClient == nullptr)
        {
        //DgnDbServerEventConnectionTaskPtr taskPtr1, taskPtr2;
        auto sasToken = GetEventServiceSAS(cancellationToken)->GetResult();
        if (!sasToken.IsSuccess())
            return false;
        auto subscriptionId = GetEventServiceSubscriptionId(cancellationToken)->GetResult();
        if (!subscriptionId.IsSuccess())
            return false;

        EventServiceClient *eventServiceClient = new EventServiceClient(sasToken.GetValue()->GetNamespace(), m_repositoryInfo.GetId(), subscriptionId.GetValue()->GetSubscriptionId());
        eventServiceClient->UpdateSASToken(sasToken.GetValue()->GetSasToken());
        m_eventServiceClient = eventServiceClient;
        }
    return true; 
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
void RepositoryInfoParser (RepositoryInfoR repositoryInfo, Utf8StringCR repositoryUrl, Utf8StringCR repositoryId, JsonValueCR value)
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
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::UpdateRepositoryInfo (ICancellationTokenPtr cancellationToken)
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

        //Set EventServiceClient
        if (!repositoryConnection->SetEventServiceClient(cancellationToken))
            return DgnDbRepositoryConnectionResult::Error(result.GetError());

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
    DgnDbServerHost::Adopt (host);

    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb (&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
    DgnDbServerStatusResult result;
    if (BeSQLite::DbResult::BE_SQLITE_OK == status && db.IsValid())
        {
        result = RepositoryInfo::WriteRepositoryInfo (*db, m_repositoryInfo, briefcaseId);
        db->CloseDb ();
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

    properties[ServerSchema::Property::Description]          = description;
    properties[ServerSchema::Property::BriefcaseId]          = briefcaseId.GetValue();
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
    if (ids.empty ())
        return;
    ObjectId lockObject (ServerSchema::Schema::Repository, ServerSchema::Class::MultiLock, "MultiLock");
    changeset.AddInstance (lockObject, changeState, std::make_shared<Json::Value>(CreateLockInstanceJson (ids, briefcaseId, description, releasedWithRevisionId, type, level)));
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
        if (includeOnlyExclusive && LockLevel::Exclusive != lock.GetLevel ())
            continue;

        int index = static_cast<int32_t>(lock.GetType ()) * 3 + static_cast<int32_t>(lock.GetLevel ());
        if (index >= 0 && index <= 8)
            objects[index].push_back (lock.GetId ().GetValue ());
        }

    Utf8String description = ""; //needswork: Currently DgnDb doesn't pass us a description for locks. Do we really need it?

    for (int i = 0; i < 9; ++i)
        AddToInstance(changeset, changeState, objects[i], briefcaseId, description, releasedWithRevisionId, static_cast<LockableType>(i / 3), static_cast<LockLevel>(i % 3));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<WSChangeset> LockDeleteAllJsonRequest (const BeBriefcaseId& briefcaseId)
    {
    Utf8String id;
    id.Sprintf ("%s-%d", ServerSchema::DeleteAllLocks, briefcaseId.GetValue ());

    ObjectId lockObject (ServerSchema::Schema::Repository, ServerSchema::Class::Lock, id);

    Json::Value properties;
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    changeset->AddInstance (lockObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value> (properties));

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
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    SetLocksJsonRequestToChangeSet (locks.GetLockSet (), briefcaseId, lastRevisionId, *changeset, WSChangeset::ChangeState::Modified);
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
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    SetLocksJsonRequestToChangeSet (locks, briefcaseId, "", *changeset, WSChangeset::ChangeState::Modified);
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
    auto changeset = LockDeleteAllJsonRequest (briefcaseId);
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
    return QueryLocksInternal (nullptr, &briefcaseId, cancellationToken);
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
    return QueryLocksInternal (&ids, &briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::GetEventServiceResponse(HttpResponseR returnResponse, bool longpolling)
    {
    HttpResponse response = m_eventServiceClient->Receive(longpolling);
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
bool DgnDbRepositoryConnection::GetEventServiceResponses(bvector<Utf8String>& responseStrings, bvector<Utf8CP>& contentTypes, bool longpolling)
    {
    HttpResponse response = m_eventServiceClient->Receive(longpolling);
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
bool EventServiceStringHelper(bmap<Utf8String, Utf8String>& entitymap, Utf8String original)
    {
    bmap<Utf8String, Utf8String>::iterator data = entitymap.begin();
    int i = 0;
    while (data != entitymap.end())
        {
        Utf8String entity = data->first;
        size_t entityPos = (0 == (BeStringUtilities::Stricmp("Date", entity.c_str()))) ? original.find_first_of(entity.c_str()) : original.rfind(entity.c_str());

        if (Utf8String::npos == entityPos)
            return false;
        Utf8String sub1 = original.substr(entityPos, original.length());
        size_t entityValueStart = sub1.find_first_of(':');
        Utf8String sub2 = sub1.substr(entityValueStart + 2, sub1.length());
        size_t entityValueStop = sub2.find_first_of('"');
        Utf8String entityValue = sub2.substr(0, entityValueStop);
        data->second = entityValue;
        ++data;
        ++i;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
IDgnDbServerEventPtr DgnDbRepositoryConnection::BuildDgnDbServerEventFromString(Utf8CP contentType, Utf8String jsonString)
    {
    size_t jsonPart1 = jsonString.find_first_of('{');
    size_t jsonPart2 = jsonString.find_last_of('}');
    Utf8String actualJsonPart = jsonString.substr(jsonPart1, jsonPart2);
    if (0 == (BeStringUtilities::Stricmp("Bentley.DgnDbServer.Common.LockEvent", contentType)))
        {
        bmap<Utf8String, Utf8String> entitymap;
        entitymap.Insert(DgnDbServerEvent::RepoId, "");
        entitymap.Insert(DgnDbServerEvent::UserId, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::LockId, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::LockType, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::Date, "");

        if (!EventServiceStringHelper(entitymap, actualJsonPart))
            return nullptr;
        return DgnDbServerLockEvent::Create
            (
            entitymap[DgnDbServerEvent::RepoId],
            entitymap[DgnDbServerEvent::UserId],
            entitymap[DgnDbServerEvent::LockEvent::LockId],
            entitymap[DgnDbServerEvent::LockEvent::LockType],
            entitymap[DgnDbServerEvent::LockEvent::Date]
            );
        }
    else if (0 == (BeStringUtilities::Stricmp("Bentley.DgnDbServer.Common.RevisionEvent", contentType)))
        {
        bmap<Utf8String, Utf8String> entitymap;
        entitymap.Insert(DgnDbServerEvent::RepoId, "");
        entitymap.Insert(DgnDbServerEvent::UserId, "");
        entitymap.Insert(DgnDbServerEvent::RevisionEvent::RevisionId, "");
        entitymap.Insert(DgnDbServerEvent::RevisionEvent::Date, "");

        if (!EventServiceStringHelper(entitymap, actualJsonPart))
            return nullptr;
        return DgnDbServerRevisionEvent::Create
            (
            entitymap[DgnDbServerEvent::RepoId],
            entitymap[DgnDbServerEvent::UserId],
            entitymap[DgnDbServerEvent::RevisionEvent::RevisionId],
            entitymap[DgnDbServerEvent::RevisionEvent::Date]
            );
        }
    else
        {
        return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
//Todo:: Needs completion
IDgnDbServerEventPtr DgnDbRepositoryConnection::BuildDgnDbServerEventFromJson(Utf8CP contentType, Utf8String jsonString)
    {
    size_t jsonPosStart = jsonString.find_first_of('{');
    size_t jsonPosEnd = jsonString.find_last_of('}');
    Utf8String actualJsonPart = jsonString.substr(jsonPosStart, jsonPosEnd);
    if (0 == (BeStringUtilities::Stricmp("Bentley.DgnDbServer.Common.LockEvent", contentType)))
        {
        Json::Reader reader;
        Json::Value data(Json::objectValue);
        if (
            reader.parse(actualJsonPart, data) &&
            !data.isArray() &&
            5 == data.size() &&
            data.isMember(DgnDbServerEvent::RepoId) &&
            data.isMember(DgnDbServerEvent::UserId) &&
            data.isMember(DgnDbServerEvent::LockEvent::LockId) &&
            data.isMember(DgnDbServerEvent::LockEvent::LockType) &&
            data.isMember(DgnDbServerEvent::LockEvent::Date)
            )
            return DgnDbServerLockEvent::Create
            (
            data[DgnDbServerEvent::RepoId].asString(),
            data[DgnDbServerEvent::UserId].asString(),
            data[DgnDbServerEvent::LockEvent::LockId].asString(),
            data[DgnDbServerEvent::LockEvent::LockType].asString(),
            data[DgnDbServerEvent::LockEvent::Date].asString()
            );
        return nullptr;
        }
    else if (0 == (BeStringUtilities::Stricmp("Bentley.DgnDbServer.Common.RevisionEvent", contentType)))
        {
        Json::Reader reader;
        Json::Value data(Json::objectValue);
        if (
            reader.parse(actualJsonPart, data) &&
            !data.isArray() &&
            data.isMember(DgnDbServerEvent::RepoId) &&
            data.isMember(DgnDbServerEvent::UserId) &&
            data.isMember(DgnDbServerEvent::RevisionEvent::RevisionId) &&
            data.isMember(DgnDbServerEvent::RevisionEvent::Date)
            )
            return DgnDbServerRevisionEvent::Create
            (
            data[DgnDbServerEvent::RepoId].asString(),
            data[DgnDbServerEvent::UserId].asString(),
            data[DgnDbServerEvent::RevisionEvent::RevisionId].asString(),
            data[DgnDbServerEvent::RevisionEvent::Date].asString()
            );
        return nullptr;
        }
    else
        return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
IDgnDbServerEventTaskPtr DgnDbRepositoryConnection::GetEvent(bool longPolling, ICancellationTokenPtr cancellationToken)
    {
    HttpResponse response;
    if (!GetEventServiceResponse(response, longPolling))
        return CreateCompletedAsyncTask<IDgnDbServerEventResult>(IDgnDbServerEventResult::Error(DgnDbServerError::Id::InternalServerError));

    if (HttpStatus::NoContent == response.GetHttpStatus())
        return CreateCompletedAsyncTask<IDgnDbServerEventResult>(IDgnDbServerEventResult::Error(DgnDbServerError::Id::NoEventsFound));

    //IDgnDbServerEventPtr ptr = BuildDgnDbServerEventFromString(response.GetHeaders().GetContentType(), response.GetBody().AsString());
    IDgnDbServerEventPtr ptr = BuildDgnDbServerEventFromJson(response.GetHeaders().GetContentType(), response.GetBody().AsString());
    if (ptr == nullptr)
        return CreateCompletedAsyncTask<IDgnDbServerEventResult>(IDgnDbServerEventResult::Error(DgnDbServerError::Id::NoEventsFound));
    return CreateCompletedAsyncTask<IDgnDbServerEventResult>(IDgnDbServerEventResult::Success(ptr));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
//Not working yet. Json deserilaization not working yet. Need to do string manipulation for now
IDgnDbServerEventCollectionTaskPtr DgnDbRepositoryConnection::GetEvents(bool longPolling, ICancellationTokenPtr cancellationToken)
    {
    bvector<Utf8String> responseStrings;
    bvector<Utf8CP> contentTypes;
    bvector<IDgnDbServerEventPtr> events;
    if (!GetEventServiceResponses(responseStrings, contentTypes, longPolling))
        return CreateCompletedAsyncTask<IDgnDbServerEventCollectionResult>(IDgnDbServerEventCollectionResult::Error(DgnDbServerError::Id::InternalServerError));

    for (int i = 0; i < responseStrings.size(); i++)
        {
        IDgnDbServerEventPtr ptr = BuildDgnDbServerEventFromJson(contentTypes[i], responseStrings[i]);
        if (ptr == nullptr)
            continue;
        events.push_back(ptr);
        }

    if (events.size()<1)
        return CreateCompletedAsyncTask<IDgnDbServerEventCollectionResult>(IDgnDbServerEventCollectionResult::Error(DgnDbServerError::Id::NoEventsFound));
    return CreateCompletedAsyncTask<IDgnDbServerEventCollectionResult>(IDgnDbServerEventCollectionResult::Success(events));
    }

////---------------------------------------------------------------------------------------
////@bsimethod									Arvind.Venkateswaran            05/2016
////@bsimethod									Caleb.Shafer					06/2016
////---------------------------------------------------------------------------------------
//EventServiceReceiveTaskPtr DgnDbRepositoryConnection::GetEvents (bool longPolling, ICancellationTokenPtr cancellationToken)
//    {
//	if (m_eventServiceClient == nullptr)
//		{
//		return CreateCompletedAsyncTask<EventServiceReceiveResult>(EventServiceReceiveResult::Error(DgnDbServerError::Id::InternalServerError));
//		}
//
//	Utf8String msg = "";
//	bool rtnVal = m_eventServiceClient->Receive(msg, longPolling);
//    
//	if (!rtnVal)
//		{
//		return CreateCompletedAsyncTask<EventServiceReceiveResult>(EventServiceReceiveResult::Error(DgnDbServerError::Id::InternalServerError));
//		}
//	if (msg.empty())
//		{
//		return CreateCompletedAsyncTask<EventServiceReceiveResult>(EventServiceReceiveResult::Error(DgnDbServerError::Id::NoEventsFound));
//		}
//
//	return CreateCompletedAsyncTask<EventServiceReceiveResult>(EventServiceReceiveResult::Success(EventServiceReceive::Create(rtnVal, msg)));    
//    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerLockSetTaskPtr DgnDbRepositoryConnection::QueryLocksById
(
LockableIdSet const&  ids,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryLocksInternal (&ids, nullptr, cancellationToken);
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
    WSQuery query (ServerSchema::Schema::Repository, ServerSchema::Class::Lock);
    Utf8String filter;

    //Format the filter
    if (nullptr == ids && nullptr != briefcaseId)
        {
        filter.Sprintf ("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue ());
        query.SetFilter (filter);
        }
    else if (nullptr != ids)
        {
        bool first = true;
        Utf8String idsString;
        for (auto id : *ids)
            {
            Utf8String idString;
            if (nullptr == briefcaseId)
                idString.Sprintf ("'%d-%llu'", (int)id.GetType (), id.GetId().GetValue());
            else
                idString.Sprintf ("'%d-%llu-%u'", (int)id.GetType (), id.GetId().GetValue(), briefcaseId->GetValue ());

            if (!first)
                idsString.append (",");
            idsString.append (idString);

            first = false;
            }

        filter.Sprintf ("$id+in+[%s]", idsString.c_str());
        query.SetFilter (filter);
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
                if (!GetLockFromServerJson (value[ServerSchema::Properties], lock, briefcaseId, repositoryId))
                    continue;//NEEDSWORK: log an error

                if (lock.GetLevel() != LockLevel::None)
                    locks.AddLock (lock, briefcaseId, repositoryId);
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
AsyncTaskPtr<WSCreateObjectResult> DgnDbRepositoryConnection::AcquireBriefcaseId (ICancellationTokenPtr cancellationToken) const
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
DgnDbServerRevisionPtr ParseRevision (JsonValueCR jsonValue)
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
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::GetAllRevisions (ICancellationTokenPtr cancellationToken) const
    {
    BeAssert (DgnDbServerHost::IsInitialized ());
    WSQuery query (ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
    return RevisionsFromQuery (query, cancellationToken);
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
//@bsimethod                                     Arvind.Venkateswaran           05/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventConnectionTaskPtr DgnDbRepositoryConnection::GetEventServiceSubscriptionId(ICancellationTokenPtr cancellationToken) const
    {
    //Query for https://{server}/{version}/Repositories/DgnDbServer--{repoId}/DgnDbServer/EventSubscription

    //Todo: Find a better way to handle error

	ObjectId eventServiceObject(ServerSchema::Schema::Repository, ServerSchema::Class::EventSubscription, "");
	return m_wsRepositoryClient->SendGetObjectRequest(eventServiceObject, nullptr, cancellationToken)->Then<DgnDbServerEventConnectionResult>
        ([=] (WSObjectsResult& eventServiceResult)
        {
        if (eventServiceResult.IsSuccess())
            {
            bvector<WSObjectsReader::Instance> jsoninstances;
            for (WSObjectsReader::Instance instance : eventServiceResult.GetValue().GetInstances())
                {
                jsoninstances.push_back(instance);
                }

            
            if (jsoninstances.size() < 1)
                //return DgnDbServerEventConnectionResult::Success(DgnDbServerEventConnection::CreateDefaultInfo());
                return DgnDbServerEventConnectionResult::Error(eventServiceResult.GetError());

            //Get json values 
            RapidJsonValueCR instanceProperties = jsoninstances[0].GetProperties();
            if (!instanceProperties.HasMember(ServerSchema::Property::Id))
                return DgnDbServerEventConnectionResult::Error(eventServiceResult.GetError());
            auto info = DgnDbServerEventConnection::Create(nullptr, nullptr, instanceProperties[ServerSchema::Property::Id].GetString());

            if (Utf8String::IsNullOrEmpty(info->GetSubscriptionId().c_str()))
                return DgnDbServerEventConnectionResult::Error(eventServiceResult.GetError());
            return DgnDbServerEventConnectionResult::Success(info);
            }
        else
            {
            return DgnDbServerEventConnectionResult::Error(eventServiceResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran           05/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventConnectionTaskPtr DgnDbRepositoryConnection::GetEventServiceSAS(ICancellationTokenPtr cancellationToken) const
    {
    //Query for https://{server}/{version}/Repositories/DgnDbServer--{repoId}/DgnDbServer/EventSAS

    //Todo: Find a better way to handle error

	ObjectId eventServiceObject(ServerSchema::Schema::Repository, ServerSchema::Class::EventSAS, "");
	return m_wsRepositoryClient->SendGetObjectRequest(eventServiceObject, nullptr, cancellationToken)->Then<DgnDbServerEventConnectionResult>
        ([=] (WSObjectsResult& eventServiceResult)
        {
        if (eventServiceResult.IsSuccess())
            {
            bvector<WSObjectsReader::Instance> jsoninstances;
            for (WSObjectsReader::Instance instance : eventServiceResult.GetValue().GetInstances())
                {
                jsoninstances.push_back(instance);
                }

            if (jsoninstances.size() < 1)
                return DgnDbServerEventConnectionResult::Error(eventServiceResult.GetError());

            //Get json values 
            RapidJsonValueCR instanceProperties = jsoninstances[0].GetProperties();

            if (
                !instanceProperties.HasMember(ServerSchema::Property::EventServiceSASToken) ||
                !instanceProperties.HasMember(ServerSchema::Property::EventServiceNameSpace)
                )
                return DgnDbServerEventConnectionResult::Error(eventServiceResult.GetError());

            auto info = DgnDbServerEventConnection::Create(
                instanceProperties[ServerSchema::Property::EventServiceSASToken].GetString(),
                instanceProperties[ServerSchema::Property::EventServiceNameSpace].GetString()
                );

            if (Utf8String::IsNullOrEmpty(info->GetSasToken().c_str()) ||
                Utf8String::IsNullOrEmpty(info->GetNamespace().c_str())
                )
                return DgnDbServerEventConnectionResult::Error(eventServiceResult.GetError());
            return DgnDbServerEventConnectionResult::Success(info);
            }
        else
            {
            return DgnDbServerEventConnectionResult::Error(eventServiceResult.GetError());
            }
        });
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
            DownloadRevisions(revisionsResult.GetValue(), callback, cancellationToken)->Then([=](DgnDbServerStatusResultCR downloadResult)
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
    properties[ServerSchema::Property::BriefcaseId] = briefcaseId.GetValue ();
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
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());

    //Set Revision initialization request to ECChangeSet
    JsonValueR revisionProperties = pushJson[ServerSchema::Instance][ServerSchema::Properties];
    revisionProperties[ServerSchema::Property::IsUploaded] = true;
    changeset->AddInstance (revisionObjectId, WSChangeset::ChangeState::Modified, std::make_shared<Json::Value> (revisionProperties));

    //Set used locks to the ECChangeSet
    LockRequest usedLocks;
    usedLocks.FromRevision (*revision);
    if (!usedLocks.IsEmpty ())
        SetLocksJsonRequestToChangeSet (usedLocks.GetLockSet (), briefcaseId, revision->GetId (), *changeset, WSChangeset::ChangeState::Modified, true);

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

