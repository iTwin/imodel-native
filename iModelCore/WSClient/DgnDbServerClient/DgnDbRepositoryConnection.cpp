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
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeLockSetResultInfo::AddLock(const DgnLock dgnLock, BeBriefcaseId briefcaseId, Utf8StringCR repositoryId)
    {
    m_locks.insert(dgnLock);
    AddLockInfoToList(m_lockStates, dgnLock, briefcaseId, repositoryId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
const DgnLockSet& DgnDbCodeLockSetResultInfo::GetLocks() const { return m_locks; }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
const DgnLockInfoSet& DgnDbCodeLockSetResultInfo::GetLockStates() const { return m_lockStates; }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeLockSetResultInfo::AddCode(const DgnCode dgnCode, DgnCodeState dgnCodeState, BeBriefcaseId briefcaseId, Utf8StringCR repositoryId)
    {
    m_codes.insert(dgnCode);
    AddCodeInfoToList(m_codeStates, dgnCode, dgnCodeState, briefcaseId, repositoryId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
const DgnCodeSet& DgnDbCodeLockSetResultInfo::GetCodes() const { return m_codes; }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
const DgnCodeInfoSet& DgnDbCodeLockSetResultInfo::GetCodeStates() const { return m_codeStates; }

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
        {
        #ifndef DEBUG
        if (Utf8String::npos != repositoryConnection->GetRepositoryInfo().GetServerURL().rfind("cloudapp.net"))
        #endif
            repositoryConnection->SetAzureClient(AzureBlobStorageClient::Create());
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Success(repositoryConnection));
        }

    return repositoryConnection->UpdateRepositoryInfo(cancellationToken)->Then<DgnDbRepositoryConnectionResult>([=] (DgnDbServerStatusResultCR result)
        {
        if (!result.IsSuccess())
            return DgnDbRepositoryConnectionResult::Error(result.GetError());

        #ifndef DEBUG
        if (Utf8String::npos != repositoryConnection->GetRepositoryInfo().GetServerURL().rfind("cloudapp.net"))
        #endif
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
Http::Request::ProgressCallbackCR callback,
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
Http::Request::ProgressCallbackCR callback,
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
Utf8StringCR             releasedWithRevisionId,
LockableType             type,
LockLevel                level,
bool                     queryOnly
)
    {
    Json::Value properties;

    properties[ServerSchema::Property::BriefcaseId]          = briefcaseId.GetValue();
    properties[ServerSchema::Property::ReleasedWithRevision] = releasedWithRevisionId;
    properties[ServerSchema::Property::QueryOnly]            = queryOnly;
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
Utf8StringCR                     releasedWithRevisionId,
LockableType                     type,
LockLevel                        level,
bool                             queryOnly
)
    {
    if (ids.empty ())
        return;
    ObjectId lockObject (ServerSchema::Schema::Repository, ServerSchema::Class::MultiLock, "MultiLock");
    changeset.AddInstance (lockObject, changeState, std::make_shared<Json::Value>(CreateLockInstanceJson (ids, briefcaseId, releasedWithRevisionId, type, level, queryOnly)));
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
bool                            includeOnlyExclusive = false,
bool                            queryOnly = false
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

    for (int i = 0; i < 9; ++i)
        AddToInstance(changeset, changeState, objects[i], briefcaseId, releasedWithRevisionId, static_cast<LockableType>(i / 3), static_cast<LockLevel>(i % 3), queryOnly);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
int CodeStateToInt(DgnCodeStateCR state)
    {
    //NEEDSWORK: Make DgnCodeState::Type public
    if (state.IsReserved())
        {
        return 1;
        }
    else if (state.IsUsed())
        {
        return 2;
        }
    else if (state.IsDiscarded())
        {
        return 3;
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Json::Value CreateCodeInstanceJson
(
bvector<DgnCode> const&      codes,
DgnCodeStateCR               state,
BeBriefcaseId                briefcaseId,
Utf8StringCR                 stateRevisionId,
bool                         queryOnly
)
    {
    Json::Value properties;
    DgnCode const* firstCode = codes.begin();

    properties[ServerSchema::Property::AuthorityId]   = firstCode->GetAuthority().GetValue();
    properties[ServerSchema::Property::Namespace]     = firstCode->GetNamespace();
    properties[ServerSchema::Property::BriefcaseId]   = briefcaseId.GetValue();
    properties[ServerSchema::Property::State]         = CodeStateToInt (state);
    properties[ServerSchema::Property::StateRevision] = stateRevisionId;
    properties[ServerSchema::Property::QueryOnly]     = queryOnly;

    properties[ServerSchema::Property::Values] = Json::arrayValue;
    int i = 0;
    for (auto const& code : codes)
        {
        properties[ServerSchema::Property::Values][i++] = code.GetValue();
        }

    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void EncodeIdString
(
Utf8StringR value
)
    {
    if (value.empty())
        return;

    Utf8String reservedChar("-");
    Utf8String replacement;
    replacement.Sprintf("_%2X_", (int)reservedChar[0]);

    value.ReplaceAll(reservedChar.c_str(), replacement.c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodeId
(
uint64_t                         authorityId,
Utf8StringCR                     nameSpace,
Utf8StringCR                     value
)
    {
    Utf8String idString;

    Utf8String encodedNamespace(nameSpace.c_str());
    EncodeIdString(encodedNamespace);
    Utf8String encodedValue(value.c_str());
    EncodeIdString(encodedValue);

    idString.Sprintf("%d-%s-%s", authorityId, encodedNamespace.c_str(), encodedValue.c_str());

    return idString;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodeId
(
uint64_t                         authorityId,
Utf8StringCR                     nameSpace,
Utf8StringCR                     value,
BeBriefcaseId                    briefcaseId
)
    {
    Utf8String idString;
    idString.Sprintf("%s-%d", FormatCodeId(authorityId, nameSpace, value).c_str(), briefcaseId.GetValue());

    return idString;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void AddCodeToInstance
(
WSChangeset&                     changeset,
WSChangeset::ChangeState const&  changeState,
bvector<DgnCode> const&          codes,
DgnCodeStateCR                   state,
BeBriefcaseId                    briefcaseId,
Utf8StringCR                     stateRevisionId,
bool                             queryOnly
)
    {
    if (codes.empty())
        return;

    ObjectId codeObject(ServerSchema::Schema::Repository, ServerSchema::Class::MultiCode, "MultiCode");
    JsonValueCR codeJson = CreateCodeInstanceJson(codes, state, briefcaseId, stateRevisionId, queryOnly);
    changeset.AddInstance(codeObject, changeState, std::make_shared<Json::Value>(codeJson));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void GroupCode
(
bmap<Utf8String, bvector<DgnCode>>* groupedCodes,
DgnCode searchCode
)
    {
    Utf8String searchKey = FormatCodeId(searchCode.GetAuthority().GetValue(), searchCode.GetNamespace(), "");
    auto it = groupedCodes->find(searchKey);
    if (it == groupedCodes->end())
        {
        bvector<DgnCode> codes;
        codes.push_back(searchCode);
        groupedCodes->insert({ searchKey, codes });
        return;
        }
    it->second.push_back(searchCode);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void SetCodesJsonRequestToChangeSet
(
const DgnCodeSet                codes,
DgnCodeState                    state,
BeBriefcaseId                   briefcaseId,
Utf8StringCR                    stateRevisionId,
WSChangeset&                    changeset,
const WSChangeset::ChangeState& changeState,
bool                            queryOnly = false
)
    {
    bmap<Utf8String, bvector<DgnCode>> groupedCodes;
    for (auto& code : codes)
        {
        GroupCode(&groupedCodes, code);
        }

    for (auto& group : groupedCodes)
        {
        AddCodeToInstance(changeset, changeState, group.second, state, briefcaseId, stateRevisionId, queryOnly);
        }
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
    changeset->AddInstance(lockObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value> (properties));

    return changeset;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void CodeDiscardReservedJsonRequest(std::shared_ptr<WSChangeset> changeSet, const BeBriefcaseId& briefcaseId)
    {
    Utf8String id;
    id.Sprintf("%s-%d", ServerSchema::DiscardReservedCodes, briefcaseId.GetValue());

    ObjectId codeObject(ServerSchema::Schema::Repository, ServerSchema::Class::Code, id);

    Json::Value properties;
    changeSet->AddInstance(codeObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value>(properties));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatLocksFilter
(
LockableIdSet const*  ids,
const BeBriefcaseId*  briefcaseId
)
    {
    Utf8String filter;

    //Format the filter
    if (nullptr == ids && nullptr != briefcaseId)
        {
        filter.Sprintf("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue());
        }
    else if (nullptr != ids)
        {
        bool first = true;
        Utf8String idsString;
        for (auto id : *ids)
            {
            Utf8String idString;
            if (nullptr == briefcaseId)
                idString.Sprintf("'%d-%llu'", (int)id.GetType(), id.GetId().GetValue());
            else
                idString.Sprintf("'%d-%llu-%u'", (int)id.GetType(), id.GetId().GetValue(), briefcaseId->GetValue());

            if (!first)
                idsString.append(",");
            idsString.append(idString);

            first = false;
            }

        if (idsString.empty())
            {
            return filter;
            }

        filter.Sprintf("$id+in+[%s]", idsString.c_str());
        }

    return filter;
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodesFilter
(
DgnCodeSet const*  ids,
const BeBriefcaseId*  briefcaseId
)
    {
    Utf8String filter;

    if (nullptr == ids && nullptr != briefcaseId)
        {
        filter.Sprintf("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue());
        }
    else if (nullptr != ids)
        {
        bool first = true;
        Utf8String idsString;
        for (auto id : *ids)
            {
            Utf8String idString;
            //NEEDSWORK - encode namespace and value
            if (nullptr != briefcaseId)
                idString.Sprintf("'%s'", FormatCodeId(id.GetAuthority().GetValue(), id.GetNamespace(), id.GetValue(), *briefcaseId).c_str());
            else
                idString.Sprintf("'%s'", FormatCodeId(id.GetAuthority().GetValue(), id.GetNamespace(), id.GetValue()).c_str());

            if (!first)
                idsString.append(",");
            idsString.append(idString);

            first = false;
            }

        if (idsString.empty())
            {
            return filter;
            }

        filter.Sprintf("$id+in+[%s]", idsString.c_str());
        }

    return filter;
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
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::AcquireCodesLocks
(
    LockRequestCR         locks,
    DgnCodeSet            codes,
    BeBriefcaseId         briefcaseId,
    Utf8StringCR          lastRevisionId,
    ICancellationTokenPtr cancellationToken
) const
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    
    SetLocksJsonRequestToChangeSet(locks.GetLockSet(), briefcaseId, lastRevisionId, *changeset, WSChangeset::ChangeState::Modified); 
    
    DgnCodeState state;
    state.SetReserved(briefcaseId);
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, lastRevisionId, *changeset, WSChangeset::ChangeState::Modified);

    return SendChangesetRequest(changeset, cancellationToken);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryCodesLocksAvailability
(
    LockRequestCR         locks,
    DgnCodeSet            codes,
    BeBriefcaseId         briefcaseId,
    Utf8StringCR          lastRevisionId,
    ICancellationTokenPtr cancellationToken
) const
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    SetLocksJsonRequestToChangeSet(locks.GetLockSet(), briefcaseId, lastRevisionId, *changeset, WSChangeset::ChangeState::Modified, false, true);

    DgnCodeState state;
    state.SetReserved(briefcaseId);
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, lastRevisionId, *changeset, WSChangeset::ChangeState::Modified, true);

    return SendChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DemoteCodesLocks
(
const DgnLockSet&     locks,
DgnCodeSet const&     codes,
BeBriefcaseId         briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    //How to set description here?
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    SetLocksJsonRequestToChangeSet (locks, briefcaseId, "", *changeset, WSChangeset::ChangeState::Modified);

    DgnCodeState state;
    state.SetAvailable();
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, "", *changeset, WSChangeset::ChangeState::Modified);

    return SendChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::RelinquishCodesLocks
(
BeBriefcaseId         briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    auto changeset = LockDeleteAllJsonRequest (briefcaseId);
    CodeDiscardReservedJsonRequest(changeset, briefcaseId);

    return SendChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocksById
(
DgnCodeSet const& codes, 
LockableIdSet const& locks,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryCodesLocksInternal(&codes, &locks, nullptr, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocksById
(
DgnCodeSet const& codes,
LockableIdSet const& locks,
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryCodesLocksInternal(&codes, &locks, &briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocks
(
const BeBriefcaseId  briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryCodesLocksInternal(nullptr, nullptr, &briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocksInternal
(
DgnCodeSet const* codes,
LockableIdSet const* locks,
const BeBriefcaseId*  briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    std::set<Utf8String> classes;
    classes.insert(ServerSchema::Class::Lock);
    classes.insert(ServerSchema::Class::Code);

    WSQuery query(ServerSchema::Schema::Repository, classes);
    Utf8String filter;

    Utf8String locksFilter = nullptr == locks && nullptr == briefcaseId ? "" : FormatLocksFilter(locks, briefcaseId);
    Utf8String codesFilter = nullptr == codes && nullptr == briefcaseId ? "" : FormatCodesFilter(codes, briefcaseId);

    if (!locksFilter.empty() && !codesFilter.empty())
        {
        //NEEDSWORK: EvaluationHelper defaults filters to true if properties are not found. Should be or instead of and.
        filter.Sprintf("(%s)+and+(%s)", locksFilter.c_str(), codesFilter.c_str());
        }
    else
        {
        filter = locksFilter.empty() ? codesFilter : locksFilter;
        }
    query.SetFilter(filter);

    //Execute query
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerCodeLockSetResult>
        ([=](const WSObjectsResult& result)
        {
        if (result.IsSuccess())
            {
            DgnDbCodeLockSetResultInfo codesLocks;
            for (auto& value : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                {
                DgnCode        code;
                DgnCodeState   codeState;
                DgnLock        lock;
                BeBriefcaseId  briefcaseId;
                Utf8String     repositoryId;
                if (GetLockFromServerJson(value[ServerSchema::Properties], lock, briefcaseId, repositoryId))
                    {
                    if (lock.GetLevel() != LockLevel::None)
                        codesLocks.AddLock(lock, briefcaseId, repositoryId);
                    }
                else if (GetCodeFromServerJson(value[ServerSchema::Properties], code, codeState, briefcaseId, repositoryId))
                    {
                    if (codeState.IsReserved() || codeState.IsUsed())
                        { 
                        codesLocks.AddCode(code, codeState, briefcaseId, repositoryId);
                        }
                    }
                //NEEDSWORK: log an error
                }
            return DgnDbServerCodeLockSetResult::Success(codesLocks);
            }
        else
            return DgnDbServerCodeLockSetResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
WSQuery CreateUnavailableLocksQuery(const BeBriefcaseId briefcaseId, const uint64_t lastRevisionIndex)
    {
    std::set<Utf8String> classes;
    classes.insert(ServerSchema::Class::Lock);
    classes.insert(ServerSchema::Class::Code);

    WSQuery query(ServerSchema::Schema::Repository, classes);
    Utf8String filter;
    Utf8String locksFilter, codesFilter;
    locksFilter.Sprintf("%s+ne+%u+or+%s+gt+%u", ServerSchema::Property::LockLevel, LockLevel::None,
                        ServerSchema::Property::ReleasedWithRevisionIndex, lastRevisionIndex);

    //NEEDSWORK: Make DgnCodeState::Type public
    codesFilter.Sprintf("%s+ne+%u+and+(%s+ne+%u+or+%s+gt+%u)", ServerSchema::Property::State, 0,
                        ServerSchema::Property::State, 3, ServerSchema::Property::StateRevisionIndex, lastRevisionIndex);

    //NEEDSWORK: EvaluationHelper defaults filters to true if properties are not found. Should be or instead of and.
    filter.Sprintf("%s+ne+%u+and+((%s)+and+(%s))", ServerSchema::Property::BriefcaseId,
                   briefcaseId.GetValue(), locksFilter.c_str(), codesFilter.c_str());

    query.SetFilter(filter);
    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
void AddCodeLock(DgnDbCodeLockSetResultInfo& codesLocksSet, JsonValueCR codeLockJson)
    {
    DgnCode        code;
    DgnCodeState   codeState;
    DgnLock        lock;
    BeBriefcaseId  briefcase;
    Utf8String     repository;
    if (GetLockFromServerJson(codeLockJson[ServerSchema::Properties], lock, briefcase, repository))
        {
        codesLocksSet.AddLock(lock, briefcase, repository);
        }
    else if (GetCodeFromServerJson(codeLockJson[ServerSchema::Properties], code, codeState, briefcase, repository))
        {
        codesLocksSet.AddCode(code, codeState, briefcase, repository);
        }
    //NEEDSWORK: log an error
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryUnavailableCodesLocks
(
    const BeBriefcaseId   briefcaseId,
    Utf8StringCR lastRevisionId,
    ICancellationTokenPtr cancellationToken
) const
    {
    if (briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId())
        return CreateCompletedAsyncTask<DgnDbServerCodeLockSetResult>(DgnDbServerCodeLockSetResult::Error(DgnDbServerError::Id::InvalidBriefcase));
    std::shared_ptr<DgnDbServerCodeLockSetResult> finalResult = std::make_shared<DgnDbServerCodeLockSetResult>();
    return GetRevisionById(lastRevisionId, cancellationToken)->Then([=] (DgnDbServerRevisionResultCR revisionResult)
        {
        uint64_t revisionIndex = 0;
        if (!revisionResult.IsSuccess() && revisionResult.GetError().GetId() != DgnDbServerError::Id::InvalidRevision)
            {
            finalResult->SetError(revisionResult.GetError());
            return;
            }
        else if (revisionResult.IsSuccess())
            {
            revisionIndex = revisionResult.GetValue()->GetIndex();
            }

        auto query = CreateUnavailableLocksQuery(briefcaseId, revisionIndex);
        
        m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then([=] (const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                {
                finalResult->SetError(result.GetError());
                return;
                }
            DgnDbCodeLockSetResultInfo codesLocks;
            for (auto const& value : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                {
                AddCodeLock(codesLocks, value);
                }
            finalResult->SetSuccess(codesLocks);
            });
        })->Then<DgnDbServerCodeLockSetResult>([=] ()
            {
            return *finalResult;
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
Http::Request::ProgressCallbackCR        callback,
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
Http::Request::ProgressCallbackCR callback,
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
Http::Request::ProgressCallbackCR callback,
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

    DgnCodeSet usedCodes;
    usedCodes = revision->GetAssignedCodes();
    if (!usedCodes.empty())
        {
        DgnCodeState state;
        state.SetUsed(revision->GetId());
        SetCodesJsonRequestToChangeSet(usedCodes, state, briefcaseId, revision->GetId(), *changeset, WSChangeset::ChangeState::Modified);
        }

    DgnCodeSet discardedCodes;
    discardedCodes = revision->GetDiscardedCodes();
    if (!discardedCodes.empty())
        {
        DgnCodeState state;
        state.SetDiscarded(revision->GetId());
        SetCodesJsonRequestToChangeSet(discardedCodes, state, briefcaseId, revision->GetId(), *changeset, WSChangeset::ChangeState::Modified);
        }

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
Http::Request::ProgressCallbackCR callback,
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

