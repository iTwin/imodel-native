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
#include <DgnPlatform/TxnManager.h>
#include <DgnPlatform/RevisionManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
void DgnDbClient::Initialize()
    {
    static bool initialized = false;
    BeAssert(!initialized);
    auto serverUrl = Dgn::DgnPlatformLib::QueryHost();
    BeAssert(serverUrl);
    DgnDbServerHost::Initialize(serverUrl->GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName(), serverUrl->GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    initialized = true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbRepositoryConnectionResult> DgnDbClient::ConnectToRepository(RepositoryInfoPtr repository, ICancellationTokenPtr cancellationToken)
    {
    return DgnDbRepositoryConnection::Create(repository, m_credentials, m_clientInfo, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbClient::DgnDbClient(ClientInfoPtr clientInfo)
    : m_clientInfo(clientInfo)
    {
    m_locks = DgnDbLocks::Create(clientInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbClientPtr DgnDbClient::Create(ClientInfoPtr clientInfo)
    {
    return DgnDbClientPtr(new DgnDbClient(clientInfo));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
void DgnDbClient::SetServerURL(Utf8StringCR serverUrl)
    {
    m_serverUrl = serverUrl;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
void DgnDbClient::SetCredentials(DgnClientFx::Utils::CredentialsCR credentials)
    {
    m_credentials = credentials;
    m_locks->SetCredentials(credentials);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<WSRepositoriesResult> GetRepositoriesByPlugin(Utf8StringCR pluginId, Utf8StringCR serverUrl, ClientInfoPtr clientInfo, ICancellationTokenPtr cancellationToken)
    {
    IWSClientPtr client = WSClient::Create(serverUrl, clientInfo);
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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8String ParseRepositoryId(const WSRepository& repository)
    {
    return repository.GetId().substr(repository.GetPluginId().size() + 2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnClientFx::Utils::AsyncTaskPtr<DgnDbRepositoriesResult> DgnDbClient::GetRepositories(ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    if (m_serverUrl.empty())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoriesResult>(DgnDbRepositoriesResult::Error(Error::InvalidServerURL));
        }
    if (!m_credentials.IsValid())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoriesResult>(DgnDbRepositoriesResult::Error(Error::InvalidCredentials));
        }
    std::shared_ptr<DgnDbRepositoriesResult> finalResult = std::make_shared<DgnDbRepositoriesResult>();
    return GetRepositoriesByPlugin(ServerSchema::Plugin::Repository, m_serverUrl, m_clientInfo, cancellationToken)->Then([=] (const WSRepositoriesResult& response)
        {
        if (response.IsSuccess())
            {
            bset<std::shared_ptr<AsyncTask>> tasks;
            for (const auto& repository : response.GetValue())
                tasks.insert(ConnectToRepository(RepositoryInfo::Create(m_serverUrl, ParseRepositoryId(repository)), cancellationToken));
            AsyncTask::WhenAll(tasks)->Then([=] ()
                {
                //Do we check whether all connections failed here?
                bvector<RepositoryInfoPtr> repositories;
                for (auto task : tasks)
                    {
                    auto result = dynamic_pointer_cast<PackagedAsyncTask<DgnDbRepositoryConnectionResult>>(task)->GetResult();
                    if (result.IsSuccess())
                        repositories.push_back(std::make_shared<RepositoryInfo>(result.GetValue()->GetRepositoryInfo()));
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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Json::Value RepositoryCreationJson(Utf8StringCR repositoryId, Utf8StringCR repositoryGuid, Utf8StringCR description, Utf8StringCR localPath, bool published)
    {
    Json::Value repositoryCreation(Json::objectValue);
    repositoryCreation[ServerSchema::Instance] = Json::objectValue;
    repositoryCreation[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::Admin;
    repositoryCreation[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::Repository;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Description] = description;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Id] = repositoryId;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileId] = repositoryGuid;
    Utf8String fileName;
    BeStringUtilities::WCharToUtf8(fileName, BeFileName(localPath).GetFileNameAndExtension().c_str());
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileName] = fileName;
    uint64_t size;
    BeFileName(localPath).GetFileSize(size);
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileSize] = size;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Published] = published;
    return repositoryCreation;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbRepositoryResult> DgnDbClient::CreateNewRepository(Dgn::DgnDbPtr db, Utf8StringCR repositoryId, Utf8StringCR description, bool published, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryResult>(DgnDbRepositoryResult::Error(Error::DbNotFound));
        }
    if (m_serverUrl.empty())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryResult>(DgnDbRepositoryResult::Error(Error::InvalidServerURL));
        }
    if (!m_credentials.IsValid())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryResult>(DgnDbRepositoryResult::Error(Error::InvalidCredentials));
        }
    if (repositoryId.empty())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryResult>(DgnDbRepositoryResult::Error(Error::InvalidRepository));
        }
    std::shared_ptr<DgnDbRepositoryResult> finalResult = std::make_shared<DgnDbRepositoryResult>();
    return GetRepositoriesByPlugin(ServerSchema::Plugin::Admin, m_serverUrl, m_clientInfo, cancellationToken)->Then([=] (const WSRepositoriesResult& repositoriesResult)
        {
        if (repositoriesResult.IsSuccess())
            {
            Utf8String adminRepositoryURL = (*repositoriesResult.GetValue().begin()).GetId();
            IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, adminRepositoryURL, m_clientInfo);
            client->SetCredentials(m_credentials);
            client->SendCreateObjectRequest(RepositoryCreationJson(repositoryId, db->GetDbGuid().ToString(), description, db->GetDbFileName(), published), db->GetFileName(), callback, cancellationToken)->Then([=] (const WSCreateObjectResult& createObjectResult)
                {
                if (createObjectResult.IsSuccess())
                    {
                    Json::Value createdObject = createObjectResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
                    Utf8String repositoryId = createdObject[ServerSchema::InstanceId].asString();
                    ConnectToRepository(RepositoryInfo::Create(m_serverUrl, repositoryId), cancellationToken)->Then([=] (const DgnDbRepositoryConnectionResult& result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess(std::make_shared<RepositoryInfo>(result.GetValue()->GetRepositoryInfo()));
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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbRepositoryResult> DgnDbClient::CreateNewRepository(Dgn::DgnDbPtr db, bool published, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryResult>(DgnDbRepositoryResult::Error(Error::DbNotFound));
        }
    Utf8String name;
    db->QueryProperty(name, BeSQLite::PropertySpec(Db::Properties::Name, Db::Properties::ProjectNamespace));
    Utf8String description;
    db->QueryProperty(description, BeSQLite::PropertySpec(Db::Properties::Description, Db::Properties::ProjectNamespace));
    return CreateNewRepository(db, name, description, published, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbBriefcaseResult> DgnDbClient::OpenBriefcase(Dgn::DgnDbPtr db, bool doSync, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        return CreateCompletedAsyncTask<DgnDbBriefcaseResult>(DgnDbBriefcaseResult::Error(Error::DbNotFound));
        }
    if (!m_credentials.IsValid())
        {
        return CreateCompletedAsyncTask<DgnDbBriefcaseResult>(DgnDbBriefcaseResult::Error(Error::InvalidCredentials));
        }
    RepositoryInfoPtr repository = RepositoryInfo::ReadRepositoryInfo(*db);
    if (!repository)
        {
        return CreateCompletedAsyncTask<DgnDbBriefcaseResult>(DgnDbBriefcaseResult::Error(Error::DbNotRepository));
        }
    std::shared_ptr<DgnDbBriefcaseResult> finalResult = std::make_shared<DgnDbBriefcaseResult>();
    return ConnectToRepository(repository)->Then([=] (const DgnDbRepositoryConnectionResult& connectionResult)
        {
        if (connectionResult.IsSuccess())
            {
            DgnDbBriefcasePtr briefcase = DgnDbBriefcase::Create(db, connectionResult.GetValue());
            AsyncTaskPtr<DgnDbResult> task;
            if (doSync)
                task = briefcase->PullAndMerge(callback, cancellationToken);
            else
                task = CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Success());
            task->Then([=] (const DgnDbResult& result)
                {
                if (result.IsSuccess())
                    finalResult->SetSuccess(briefcase);
                else
                    finalResult->SetError(result.GetError());
                });
            }
        else
            finalResult->SetError(connectionResult.GetError());
        })->Then<DgnDbBriefcaseResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbFileNameResult> DgnDbClient::AquireBriefcase(Utf8StringCR repositoryId, BeFileNameCR localPath, bool doSync, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    if (repositoryId.empty())
        {
        return CreateCompletedAsyncTask<DgnDbFileNameResult>(DgnDbFileNameResult::Error(Error::InvalidRepository));
        }
    if (m_serverUrl.empty())
        {
        return CreateCompletedAsyncTask<DgnDbFileNameResult>(DgnDbFileNameResult::Error(Error::InvalidServerURL));
        }
    if (!m_credentials.IsValid())
        {
        return CreateCompletedAsyncTask<DgnDbFileNameResult>(DgnDbFileNameResult::Error(Error::InvalidCredentials));
        }
    if (!localPath.GetDirectoryName().DoesPathExist())
        {
        BeFileName::CreateNewDirectory(localPath.GetDirectoryName());
        }
    std::shared_ptr<DgnDbFileNameResult> finalResult = std::make_shared<DgnDbFileNameResult>();
    RepositoryInfoPtr repository = RepositoryInfo::Create(m_serverUrl, repositoryId);
    return ConnectToRepository(repository, cancellationToken)->Then([=] (const DgnDbRepositoryConnectionResult& connectionResult)
        {
        if (connectionResult.IsSuccess())
            {
            DgnDbRepositoryConnectionPtr connection = connectionResult.GetValue();
            connection->AcquireBriefcaseId(cancellationToken)->Then([=] (const WSCreateObjectResult& briefcaseResult)
                {
                if (briefcaseResult.IsSuccess())
                    {
                    JsonValueCR properties = briefcaseResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties];
                    uint32_t briefcaseId = properties[ServerSchema::Property::BriefcaseId].asUInt();

                    BeFileName filePath(localPath);
                    if (filePath.IsDirectory())
                        {
                        Utf8String dirName;
                        dirName.Sprintf("%s%d", repository->GetId(), briefcaseId);
                        filePath.AppendToPath(BeFileName(dirName));
                        filePath.AppendToPath(BeFileName(properties[ServerSchema::Property::FileName].asString()));
                        BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
                        }

                    uint64_t fileSize;
                    BeStringUtilities::ParseUInt64(fileSize, properties[ServerSchema::Property::FileSize].asCString());

                    if (doSync)
                        {
                        auto briefcaseTask = connection->DownloadBriefcaseFile(filePath, BeBriefcaseId(briefcaseId), callback, cancellationToken);
                        auto pullTask = connection->Pull("", callback, cancellationToken);
                        bset<std::shared_ptr<AsyncTask>> tasks;
                        tasks.insert(briefcaseTask);
                        tasks.insert(pullTask);
                        AsyncTask::WhenAll(tasks)->Then([=] ()
                            {
                            if (!briefcaseTask->GetResult().IsSuccess())
                                finalResult->SetError(briefcaseTask->GetResult().GetError());
                            if (!pullTask->GetResult().IsSuccess())
                                finalResult->SetError(pullTask->GetResult().GetError());

                            BeSQLite::DbResult status;
                            Dgn::DgnPlatformLib::AdoptHost(DgnDbServerHost::Host());
                            Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
                            if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                                {
                                bvector<Dgn::DgnRevisionPtr> revisions = pullTask->GetResult().GetValue();
                                RevisionStatus mergeStatus = RevisionStatus::Success;
                                if (!revisions.empty())
                                    {
                                    mergeStatus = db->Revisions().MergeRevisions(revisions);
                                    db->CloseDb();
                                    }
                                Dgn::DgnPlatformLib::ForgetHost();
                                if (RevisionStatus::Success != mergeStatus)
                                    finalResult->SetError(mergeStatus);
                                else
                                    finalResult->SetSuccess(filePath);
                                }
                            else
                                {
                                Dgn::DgnPlatformLib::ForgetHost();
                                finalResult->SetError(db->GetLastError(&status).c_str());
                                }
                            });
                        }
                    else
                        {
                        auto task = connection->DownloadBriefcaseFile(filePath, BeBriefcaseId(briefcaseId), callback, cancellationToken)->Then([=] (const DgnDbResult& result)
                            {
                            if (result.IsSuccess())
                                finalResult->SetSuccess(filePath);
                            else
                                finalResult->SetError(result.GetError());
                            });
                        }
                    }
                else
                    {
                    finalResult->SetError(briefcaseResult.GetError());
                    }
                });
            }
        else
            finalResult->SetError(connectionResult.GetError());
        })->Then<DgnDbFileNameResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Dgn::ILocksServer* DgnDbClient::GetLocksServerP()
    {
    return dynamic_cast<Dgn::ILocksServer*>(m_locks.get());
    }
