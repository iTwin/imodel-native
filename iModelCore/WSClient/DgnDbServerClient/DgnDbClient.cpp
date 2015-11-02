/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbClient.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbClient.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <json/json.h>
#include <DgnPlatform/DgnCore/TxnManager.h>
#include <DgnPlatform/DgnCore/RevisionManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define DGNDBSERVER_PLUGIN_NAME "Bentley.DgnDbServerECPlugin"
#define DGNDBSERVERADMIN_PLUGIN_NAME "Bentley.DgnDbServerAdminECPlugin"
#define DGNDBSERVERADMIN_SCHEMA_NAME "DgnDbServerAdminSchema"
#define DGNDBSERVERADMIN_REPOSITORY_CLASS "DgnDbRepository"

#define DGNDBSERVER_LOCAL_REPOSITORY_URL "dgndbserver_serverUrl"
#define DGNDBSERVER_LOCAL_REPOSITORY_ID "dgndbserver_repositoryId"
#define DGNDBSERVER_SCHEMA_NAME "DgnDbServerSchema"
#define DGNDBSERVER_REPOSITORY_CLASS "DgnDbFile"

DgnDbRepositoryConnectionPtr DgnDbClient::ConnectToRepository(RepositoryInfoPtr repository)
    {
    return DgnDbRepositoryConnection::Create(repository, m_host, m_credentials, m_clientInfo);
    }

DgnDbClient::DgnDbClient(Utf8StringCR host, CredentialsCR credentials, ClientInfoPtr clientInfo)
    : m_host(host), m_credentials(credentials), m_clientInfo(clientInfo)
    { 
    }

DgnDbClientPtr DgnDbClient::Create(Utf8StringCR host, CredentialsCR credentials, ClientInfoPtr clientInfo)
    {
    return DgnDbClientPtr(new DgnDbClient(host, credentials, clientInfo));
    }

RepositoryInfoPtr RepositoryInfoParser(Utf8StringCR repositoryUrl, JsonValueCR value)
    {
    DateTime uploadedDate = DateTime();
    DateTime::FromString(uploadedDate, static_cast<Utf8CP>(value["UploadedDate"].asCString()));
    uint64_t fileSize;
    BeStringUtilities::ParseUInt64(fileSize, value["FileSize"].asCString());
    return RepositoryInfo::Create(FileInfo(value["FileName"].asString(), value["URL"].asString(), fileSize), repositoryUrl, value["Description"].asString(), value["Id"].asString(), value["UserUploaded"].asString(), uploadedDate);
    }


AsyncTaskPtr<DgnDbRepositoryResult> GetRepositoryInfo(Utf8StringCR host, Utf8StringCR repository, ClientInfoPtr clientInfo, CredentialsCR credentials, ICancellationTokenPtr cancellationToken = nullptr)
    {
    IWSRepositoryClientPtr client = WSRepositoryClient::Create(host, repository, clientInfo);
    client->SetCredentials(credentials);
    return client->SendGetObjectRequest(ObjectId(DGNDBSERVER_SCHEMA_NAME, DGNDBSERVER_REPOSITORY_CLASS, ""), nullptr, cancellationToken)->Then<DgnDbRepositoryResult>([=] (WSObjectsResult& response)
        {
        if (response.IsSuccess())
            {
            return DgnDbRepositoryResult::Success(RepositoryInfoParser(repository, response.GetValue().GetJsonValue()["instances"][0]["properties"]));
            }
        else
            {
            return DgnDbRepositoryResult::Error(response.GetError());
            }
        });
    }

AsyncTaskPtr<WSRepositoriesResult> GetRepositoriesByPlugin(Utf8StringCR pluginId, Utf8StringCR host, ClientInfoPtr clientInfo, ICancellationTokenPtr cancellationToken)
    {
    IWSClientPtr client = WSClient::Create(host, clientInfo);
    return client->SendGetRepositoriesRequest(cancellationToken)->Then<WSRepositoriesResult>([=] (const WSRepositoriesResult& response)
        {
        if (response.IsSuccess())
            {
            bvector<WSRepository> repositories;
            for (auto& repository : response.GetValue())
                {
                if (pluginId == repository.GetPluginId())
                    repositories.push_back(repository);
                }
            return WSRepositoriesResult::Success(repositories);
            }
        else
            return WSRepositoriesResult::Error(response.GetError());
        });
    }

DgnClientFx::Utils::AsyncTaskPtr<DgnDbRepositoriesResult> DgnDbClient::GetRepositories(ICancellationTokenPtr cancellationToken)
    {
    std::shared_ptr<DgnDbRepositoriesResult> finalResult = std::make_shared<DgnDbRepositoriesResult>();
    return GetRepositoriesByPlugin(DGNDBSERVER_PLUGIN_NAME, m_host, m_clientInfo, cancellationToken)->Then([=] (const WSRepositoriesResult& response)
        {
        if (response.IsSuccess())
            {
            bset<std::shared_ptr<AsyncTask>> tasks;
            for (const auto& repository : response.GetValue())
                tasks.insert(GetRepositoryInfo(m_host, repository.GetId(), m_clientInfo, m_credentials, cancellationToken));
            AsyncTask::WhenAll(tasks)->Then([=] ()
                {
                //Do we check whether all connections failed here?
                bvector<RepositoryInfoPtr> repositories;
                for (auto task : tasks)
                    {
                    auto result = dynamic_pointer_cast<PackagedAsyncTask<DgnDbRepositoryResult>>(task)->GetResult();
                    if (result.IsSuccess())
                        repositories.push_back(result.GetValue());
                    }
                finalResult->SetSuccess(repositories);
                });
            }
        else
            finalResult->SetError(response.GetError());
        })->Then<DgnDbRepositoriesResult>([=] ()
            {
            return *finalResult;
            });
    }

Json::Value RepositoryCreationJson(Utf8StringCR repositoryId, Utf8StringCR description, Utf8StringCR localPath, bool published)
    {
    Json::Value repositoryCreation(Json::objectValue);
    repositoryCreation["instance"] = Json::objectValue;
    repositoryCreation["instance"]["schemaName"] = DGNDBSERVERADMIN_SCHEMA_NAME;
    repositoryCreation["instance"]["className"] = DGNDBSERVERADMIN_REPOSITORY_CLASS;
    repositoryCreation["instance"]["properties"]["Description"] = description;
    repositoryCreation["instance"]["properties"]["Id"] = repositoryId;
    Utf8String fileName;
    BeStringUtilities::WCharToUtf8(fileName, BeFileName(localPath).GetFileNameAndExtension().c_str());
    repositoryCreation["instance"]["properties"]["FileName"] = fileName;
    uint64_t size;
    BeFileName(localPath).GetFileSize(size);
    repositoryCreation["instance"]["properties"]["FileSize"] = size;
    repositoryCreation["instance"]["properties"]["Published"] = published;
    return repositoryCreation;
    }


AsyncTaskPtr<DgnDbRepositoryResult> DgnDbClient::CreateNewRepository(Utf8StringCR repositoryId, Utf8StringCR description, Utf8StringCR localPath, bool published, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    std::shared_ptr<DgnDbRepositoryResult> finalResult = std::make_shared<DgnDbRepositoryResult>();
    return GetRepositoriesByPlugin(DGNDBSERVERADMIN_PLUGIN_NAME, m_host, m_clientInfo, cancellationToken)->Then([=] (const WSRepositoriesResult& repositoriesResult)
        {
        if (repositoriesResult.IsSuccess())
            {
            Utf8String adminRepositoryURL = (*repositoriesResult.GetValue().begin()).GetId();
            IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_host, adminRepositoryURL, m_clientInfo);
            client->SetCredentials(m_credentials);
            client->SendCreateObjectRequest(RepositoryCreationJson(repositoryId, description, localPath, published), BeFileName(localPath), callback, cancellationToken)->Then([=] (const WSCreateObjectResult& createObjectResult)
                {
                if (createObjectResult.IsSuccess())
                    {
                    Json::Value createdObject = createObjectResult.GetValue().GetObject()["changedInstance"]["instanceAfterChange"];
                    Utf8String repositoryURL = DGNDBSERVER_PLUGIN_NAME "--" + createdObject["instanceId"].asString();
                    GetRepositoryInfo(m_host, repositoryURL, m_clientInfo, m_credentials, cancellationToken)->Then([=] (const DgnDbRepositoryResult& result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess(result.GetValue());
                        else
                            finalResult->SetError(result.GetError());
                        });
                    }
                else
                    finalResult->SetError(createObjectResult.GetError());
                });
            }
        else
            finalResult->SetError(repositoriesResult.GetError());
        })->Then<DgnDbRepositoryResult>([=] ()
            {
            return *finalResult;
            });
    }

AsyncTaskPtr<DgnDbBriefcaseResult> DgnDbClient::OpenBriefcase(Dgn::DgnDbPtr db, bool doSync, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    DgnDbRepositoryConnectionPtr connection = ConnectToRepository(RepositoryInfo::ReadRepositoryInfo(*db));
    DgnDbBriefcasePtr briefcase = DgnDbBriefcase::Create(db, connection);
    AsyncTaskPtr<DgnDbResult> task;
    if (doSync)
        task = briefcase->Sync(callback, cancellationToken);
    else
        task = CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Success());
    return task->Then<DgnDbBriefcaseResult>([=] (const DgnDbResult& result)
        {
        if (result.IsSuccess())
            return DgnDbBriefcaseResult::Success(briefcase);
        else
            return DgnDbBriefcaseResult::Error(result.GetError());
        });
    }

AsyncTaskPtr<DgnDbBriefcaseResult> DgnDbClient::OpenBriefcase(Dgn::DgnDbPtr db, CredentialsCR credentials, ClientInfoPtr clientInfo, bool doSync, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    Utf8String host;
    db->QueryBriefcaseLocalValue(DGNDBSERVER_LOCAL_REPOSITORY_URL, host);
    DgnDbClientPtr client = Create(host, credentials, clientInfo);
    return client->OpenBriefcase(db, doSync, callback, cancellationToken);
    }

AsyncTaskPtr<DgnDbFileNameResult> DgnDbClient::AquireBriefcase(RepositoryInfoPtr repository, BeFileNameCR localPath, bool doSync, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    std::shared_ptr<DgnDbFileNameResult> finalResult = std::make_shared<DgnDbFileNameResult>();
    DgnDbRepositoryConnectionPtr connection = ConnectToRepository(repository);
    return connection->AcquireBriefcaseId(cancellationToken)->Then([=] (const WSObjectsResult& briefcaseResult)
        {
        if (briefcaseResult.IsSuccess())
            {
            JsonValueCR properties = briefcaseResult.GetValue().GetJsonValue()["instances"][0]["properties"];
            uint32_t briefcaseId = properties["BriefcaseId"].asUInt();

            BeFileName filePath(localPath);
            if (filePath.IsDirectory())
                {
                Utf8String dirName;
                dirName.Sprintf("%s%d", repository->GetId(), briefcaseId);
                filePath.AppendToPath(BeFileName(dirName));
                filePath.AppendToPath(BeFileName(properties["FileName"].asString()));
                BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
                }

            uint64_t fileSize;
            BeStringUtilities::ParseUInt64(fileSize, properties["FileSize"].asCString());
            FileInfo resultFile(filePath.GetNameUtf8(), properties["FileName"].asString(), properties["URL"].asString(), fileSize);

            if (doSync)
                {
                CallbackQueue callbackQueue(callback);
                auto briefcaseTask = connection->DownloadBriefcaseFile(filePath, callbackQueue.NewCallback(), cancellationToken);
                auto pullTask = connection->Pull("", callbackQueue.NewCallback(), cancellationToken);
                bset<std::shared_ptr<AsyncTask>> tasks;
                tasks.insert(briefcaseTask);
                tasks.insert(pullTask);
                AsyncTask::WhenAll(tasks)->Then([=] () {
                    if (!briefcaseTask->GetResult().IsSuccess())
                        finalResult->SetError(briefcaseTask->GetResult().GetError());
                    if (!pullTask->GetResult().IsSuccess())
                        finalResult->SetError(pullTask->GetResult().GetError());

                    BeSQLite::DbResult status;
                    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
                    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                        {
                        status = db->ChangeBriefcaseId(BeBriefcaseId(briefcaseId));
                        if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                            {
                            bvector<Dgn::DgnRevisionPtr> revisions = pullTask->GetResult().GetValue();
                            BentleyStatus mergeStatus = db->Revisions().MergeRevisions(revisions);
                            if (BentleyStatus::SUCCESS != status)
                                finalResult->SetError(mergeStatus);
                            else
                                finalResult->SetSuccess(filePath);
                            }
                        else
                            finalResult->SetError(status);
                        }
                    else
                        finalResult->SetError(status);
                    });
                }
            else
                {
                auto task = connection->DownloadBriefcaseFile(filePath, callback, cancellationToken)->Then([=] (const DgnDbResult& result)
                    {
                    if (result.IsSuccess())
                        {
                        BeSQLite::DbResult status;
                        Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
                        if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                            {
                            status = db->ChangeBriefcaseId(BeBriefcaseId(briefcaseId));
                            if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                                finalResult->SetSuccess(filePath);
                            else
                                finalResult->SetError(status);
                            }
                        else
                            finalResult->SetError(status);
                        }
                    else
                        finalResult->SetError(result.GetError());
                    });
                }

            }
        else
            {
            finalResult->SetError(briefcaseResult.GetError());
            }
        })->Then<DgnDbFileNameResult>([=] ()
            {
            return *finalResult;
            });
    }
