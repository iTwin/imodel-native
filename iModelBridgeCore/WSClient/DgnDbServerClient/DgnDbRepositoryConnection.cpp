/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbRepositoryConnection.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/RevisionManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define DGNDBSERVER_SCHEMA_NAME "DgnDbServerSchema"
#define DGNDBSERVER_BRIEFCASE_CLASS "DgnDbBriefcase"
#define DGNDBSERVER_REVISION_CLASS "DgnDbRevision"


DgnDbRepositoryConnection::DgnDbRepositoryConnection(RepositoryInfoPtr repository, Utf8StringCR host, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo) : m_repositoryInfo(repository)
    {
    m_wsRepositoryClient = WSRepositoryClient::Create(host, repository->GetServerURL(), clientInfo);
    m_wsRepositoryClient->SetCredentials(credentials);
    }

DgnDbRepositoryConnectionPtr DgnDbRepositoryConnection::Create(RepositoryInfoPtr repository, Utf8StringCR host, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo)
    {
    return DgnDbRepositoryConnectionPtr(new DgnDbRepositoryConnection(repository, host, credentials, clientInfo));
    }

AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::DownloadBriefcaseFile(BeFileName localFile, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    ObjectId fileObject(DGNDBSERVER_SCHEMA_NAME, DGNDBSERVER_BRIEFCASE_CLASS, m_repositoryInfo->GetFileInfo().GetFileName());
    return (m_wsRepositoryClient->SendGetFileRequest(fileObject, localFile, nullptr, callback, cancellationToken)->Then<DgnDbServerResult<void>>([=] (const WSFileResult& fileResult)
        {
        if (fileResult.IsSuccess())
            {
            BeFileName resultPath = fileResult.GetValue().GetFilePath();
            BeSQLite::DbResult status;
            Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, resultPath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));

            if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                status = RepositoryInfo::WriteRepositoryInfo(*db, *m_repositoryInfo);
            if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                return DgnDbResult::Success();
            return DgnDbResult::Error(status);
            }
        else
            {
            return DgnDbResult::Error(fileResult.GetError());
            }
        }));
    }

AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::DownloadRevisionFile(Dgn::DgnRevisionPtr revision, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    ObjectId fileObject(DGNDBSERVER_SCHEMA_NAME, DGNDBSERVER_BRIEFCASE_CLASS, revision->GetId());
    
    return m_wsRepositoryClient->SendGetFileRequest(fileObject, revision->GetChangeStreamFile(), nullptr, callback, cancellationToken)->Then<DgnDbServerResult<void>>([=] (const WSFileResult& fileResult)
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

AsyncTaskPtr<WSObjectsResult> DgnDbRepositoryConnection::AcquireBriefcaseId(ICancellationTokenPtr cancellationToken)
    {
    return m_wsRepositoryClient->SendGetObjectRequest(ObjectId(DGNDBSERVER_SCHEMA_NAME, DGNDBSERVER_BRIEFCASE_CLASS, ""), nullptr, cancellationToken);
    }

DgnRevisionPtr ParseRevision(JsonValueCR jsonValue)
    {
    DgnRevisionPtr revision = DgnRevision::Create(jsonValue["Id"].asString(), jsonValue["ParentId"].asString(), jsonValue["MasterFileId"].asString());
    revision->SetSummary(jsonValue["Description"].asCString());
    DateTime pushDate = DateTime();
    DateTime::FromString(pushDate, jsonValue["PushDate"].asCString());
    revision->SetDateTime(pushDate);
    revision->SetUserName(jsonValue["UserCreated"].asCString());
    revision->SetInitialParentId(jsonValue["MergedFromId"].asCString());
    return revision;
    }

AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::GetRevisionIndex(uint64_t& index, Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken)
    {
    return m_wsRepositoryClient->SendGetObjectRequest(ObjectId(DGNDBSERVER_SCHEMA_NAME, DGNDBSERVER_REVISION_CLASS, revisionId))->Then<DgnDbResult>([&index] (WSObjectsResult& revisionResult)
        {
        if (revisionResult.IsSuccess())
            {
            index = revisionResult.GetValue().GetJsonValue()["instances"][0]["properties"]["Index"].asInt64();
            return DgnDbResult::Success();
            }
        else
            return DgnDbResult::Error(revisionResult.GetError());
        });
    }

AsyncTaskPtr<DgnDbRevisionResult> DgnDbRepositoryConnection::GetRevisionById(Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken)
    {
    return m_wsRepositoryClient->SendGetObjectRequest(ObjectId(DGNDBSERVER_SCHEMA_NAME, DGNDBSERVER_REVISION_CLASS, revisionId))->Then<DgnDbRevisionResult>([=] (WSObjectsResult& revisionResult)
        {
        if (revisionResult.IsSuccess())
            {
            return DgnDbRevisionResult::Success(ParseRevision(revisionResult.GetValue().GetJsonValue()["instances"][0]["properties"]));
            }
        else
            return DgnDbRevisionResult::Error(revisionResult.GetError());
        });
    }

AsyncTaskPtr<DgnDbRevisionsResult> DgnDbRepositoryConnection::RevisionsFromQuery(const WebServices::WSQuery& query, ICancellationTokenPtr cancellationToken)
    {
    
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, cancellationToken)->Then<DgnDbRevisionsResult>([=] (const WSObjectsResult& revisionsInfoResult)
        {
        if (revisionsInfoResult.IsSuccess())
            {
            bvector<DgnRevisionPtr> revisions;
            for (auto value : revisionsInfoResult.GetValue().GetJsonValue()["instances"])
                revisions.push_back(ParseRevision(value["properties"]));
            return DgnDbRevisionsResult::Success(revisions);
            }
        else
            return DgnDbRevisionsResult::Error(revisionsInfoResult.GetError());
        });
    }


AsyncTaskPtr<DgnDbRevisionsResult> DgnDbRepositoryConnection::GetRevisionsFromId(Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken)
    {
    std::shared_ptr<DgnDbRevisionsResult> finalResult = std::make_shared<DgnDbRevisionsResult>();
    uint64_t index;
    return GetRevisionIndex(index, revisionId, cancellationToken)->Then([=] (const DgnDbResult& indexResult)
        {
        if (indexResult.IsSuccess())
            {
            WSQuery query(DGNDBSERVER_SCHEMA_NAME, DGNDBSERVER_REVISION_CLASS);
            query.SetFilter("Index+gt+" + index);
            RevisionsFromQuery(query, cancellationToken)->Then([=] (const DgnDbRevisionsResult& revisionsResult)
                {
                if (revisionsResult.IsSuccess())
                    finalResult->SetSuccess(revisionsResult.GetValue());
                else
                    finalResult->SetError(revisionsResult.GetError());
                });
            }
        else
            finalResult->SetError(indexResult.GetError());
        })->Then<DgnDbRevisionsResult>([=] ()
        {
        return *finalResult;
        });
    }

AsyncTaskPtr<DgnDbRevisionsResult> DgnDbRepositoryConnection::Pull(Utf8StringCR revisionId, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    std::shared_ptr<DgnDbRevisionsResult> finalResult = std::make_shared<DgnDbRevisionsResult>();
    return GetRevisionsFromId(revisionId, cancellationToken)->Then([=] (const DgnDbRevisionsResult& revisionsResult)
        {
        if (revisionsResult.IsSuccess())
            {
            CallbackQueue callbackQueue(callback);
            bset<std::shared_ptr<AsyncTask>> tasks;
            for (auto& revision : revisionsResult.GetValue())
                tasks.insert(DownloadRevisionFile(revision, callbackQueue.NewCallback(), cancellationToken));
            AsyncTask::WhenAll(tasks)->Then([=] ()
                {
                bvector<DgnRevisionPtr> revisions;
                for (auto task : tasks)
                    {
                    auto result = dynamic_pointer_cast<PackagedAsyncTask<DgnDbRevisionResult>>(task)->GetResult();
                    if (result.IsSuccess())
                        revisions.push_back(result.GetValue());
                    else
                        finalResult->SetError(result.GetError());
                    }
                finalResult->SetSuccess(revisions);
                });
            }
        else
            finalResult->SetError(revisionsResult.GetError());
        })->Then<DgnDbRevisionsResult>([=] ()
            {
            return *finalResult;
            });
    }

Json::Value PushRevisionJson(Dgn::DgnRevisionPtr revision)
    {
    Json::Value pushRevisionJson = Json::objectValue;
    pushRevisionJson["instance"] = Json::objectValue;
    pushRevisionJson["instance"]["schemaName"] = DGNDBSERVER_SCHEMA_NAME;
    pushRevisionJson["instance"]["className"] = DGNDBSERVER_REVISION_CLASS;
    pushRevisionJson["instance"]["properties"] = Json::objectValue;

    pushRevisionJson["instance"]["properties"]["Id"] = revision->GetId();
    pushRevisionJson["instance"]["properties"]["ParentId"] = revision->GetParentId();
    pushRevisionJson["instance"]["properties"]["MasterFileId"] = revision->GetDbGuid();
    Utf8String fileName;
    BeStringUtilities::WCharToUtf8(fileName, revision->GetChangeStreamFile().GetFileNameAndExtension().c_str());
    pushRevisionJson["instance"]["properties"]["FileName"] = fileName;
    uint64_t size;
    revision->GetChangeStreamFile().GetFileSize(size);
    pushRevisionJson["instance"]["properties"]["FileSize"] = size;
    pushRevisionJson["instance"]["properties"]["Description"] = revision->GetSummary();
    pushRevisionJson["instance"]["properties"]["MergedFromId"] = revision->GetInitialParentId();
    //pushRevisionJson["instance"]["properties"]["BriefcaseId"] = ;
    return pushRevisionJson;
    }

AsyncTaskPtr<DgnDbResult> DgnDbRepositoryConnection::Push(Dgn::DgnRevisionPtr revision, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    Utf8String repositoryId;
    return m_wsRepositoryClient->SendCreateObjectRequest(PushRevisionJson(revision), revision->GetChangeStreamFile(), callback, cancellationToken)->Then<DgnDbResult>([=] (const WSCreateObjectResult& result)
        {
        if (result.IsSuccess())
            return DgnDbResult::Success();
        else
            return DgnDbResult::Error(result.GetError());
        });
    }

RepositoryInfoCR DgnDbRepositoryConnection::GetRepositoryInfo()
    {
    return *m_repositoryInfo;
    }

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

