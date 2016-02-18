/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbClient.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbClient.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <json/json.h>
#include <DgnPlatform/TxnManager.h>
#include <DgnPlatform/RevisionManager.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>
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

    auto host = Dgn::DgnPlatformLib::QueryHost();
    BeAssert(host);

    BeFileName temp = host->GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    BeFileName assets = host->GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    DgnDbServerHost::Initialize(temp, assets);
    initialized = true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbRepositoryConnectionResult> DgnDbClient::ConnectToRepository(RepositoryInfoPtr repository, ICancellationTokenPtr cancellationToken)
    {
    return DgnDbRepositoryConnection::Create(repository, m_credentials, m_clientInfo, cancellationToken, m_customHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbClient::DgnDbClient(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler)
    : m_clientInfo(clientInfo), m_customHandler (customHandler)
    {
    m_repositoryManager = DgnDbRepositoryManager::Create(clientInfo, customHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbClientPtr DgnDbClient::Create(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler)
    {
    return DgnDbClientPtr(new DgnDbClient(clientInfo, customHandler));
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
    m_repositoryManager->SetCredentials(credentials);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<WSRepositoriesResult> DgnDbClient::GetRepositoriesByPlugin(Utf8StringCR pluginId, ICancellationTokenPtr cancellationToken)
    {
    IWSClientPtr client = WSClient::Create(m_serverUrl, m_clientInfo, m_customHandler);
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
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        return CreateCompletedAsyncTask<DgnDbRepositoriesResult>(DgnDbRepositoriesResult::Error(Error::InvalidCredentials));
        }
    std::shared_ptr<DgnDbRepositoriesResult> finalResult = std::make_shared<DgnDbRepositoriesResult>();
    return GetRepositoriesByPlugin(ServerSchema::Plugin::Repository, cancellationToken)->Then([=]
        (const WSRepositoriesResult& response)
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
                    else
                        finalResult->SetError(result.GetError());
                    }
                if (!repositories.empty())
                    {
                    finalResult->SetSuccess(repositories);
                    }
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
Json::Value RepositoryCreationJson(Utf8StringCR repositoryName, Utf8StringCR repositoryGuid, Utf8StringCR description, Utf8StringCR localPath, bool published)
    {
    Json::Value repositoryCreation(Json::objectValue);
    repositoryCreation[ServerSchema::Instance] = Json::objectValue;
    repositoryCreation[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::Admin;
    repositoryCreation[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::Repository;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Description] = description;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::RepositoryName] = repositoryName;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileId] = repositoryGuid;
    Utf8String fileName;
    BeStringUtilities::WCharToUtf8(fileName, BeFileName(localPath).GetFileNameAndExtension().c_str());
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileName] = fileName;
    uint64_t size;
    BeFileName(localPath).GetFileSize(size);
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::FileSize] = size;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::Published] = published;
    repositoryCreation[ServerSchema::Instance][ServerSchema::Properties][ServerSchema::Property::IsUploaded] = false;
    return repositoryCreation;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbRepositoryResult> DgnDbClient::InitializeRepository(IWSRepositoryClientPtr client, Utf8StringCR repositoryId, Json::Value repositoryCreationJson,
    ObjectId repositoryObjectId, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    Json::Value repositoryProperties = Json::Value(repositoryCreationJson[ServerSchema::Instance][ServerSchema::Properties]);
    repositoryProperties[ServerSchema::Property::IsUploaded] = true;

    std::shared_ptr<DgnDbRepositoryResult> finalResult = std::make_shared<DgnDbRepositoryResult>();
    return client->SendUpdateObjectRequest(repositoryObjectId, repositoryProperties, nullptr, callback, cancellationToken)
        ->Then([=] (const WSUpdateObjectResult& initializeRepositoryResult)
        {
        if (!initializeRepositoryResult.IsSuccess())
            {
            finalResult->SetError(initializeRepositoryResult.GetError());
            return;
            }

        ConnectToRepository(RepositoryInfo::Create(m_serverUrl, repositoryId), cancellationToken)
            ->Then ([=] (const DgnDbRepositoryConnectionResult& result)
            {
            if (result.IsSuccess())
                finalResult->SetSuccess (std::make_shared<RepositoryInfo>(result.GetValue()->GetRepositoryInfo()));
            else
                finalResult->SetError (result.GetError());
            });
        })->Then<DgnDbRepositoryResult>([=]
            {
            return *finalResult;
            });
    }

DgnDbPtr CleanDb(DgnDbR db)
    {
    //NEEDSWORK: Make a clean copy for a server. This code should move to the server once we have long running services.
    BeFileName tempFile;
    DgnPlatformLib::QueryHost()->GetIKnownLocationsAdmin().GetLocalTempDirectory(tempFile, L"DgnDbServerClient");
    tempFile.AppendToPath(db.GetFileName().GetFileNameAndExtension().c_str());
    BeFileName::BeCopyFile(db.GetFileName(), tempFile);

    BeSQLite::DbResult status;
    Dgn::DgnDbPtr tempdb = Dgn::DgnDb::OpenDgnDb(&status, tempFile, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
    if (BeSQLite::DbResult::BE_SQLITE_OK != status)
        return nullptr;


    //Do cleanup
    TxnManager::TxnId cancelToId, tempId = tempdb->Txns().GetCurrentTxnId();            //Clear transaction table
    while (tempId.IsValid())
        {
        cancelToId = tempId;
        tempId = tempdb->Txns().QueryPreviousTxnId(tempId);
        }
    if (cancelToId.IsValid())
        tempdb->Txns().CancelTo(cancelToId, TxnManager::AllowCrossSessions::Yes);
    tempdb->SaveBriefcaseLocalValue("ParentRevisionId", "");                            //Clear parent revision id
    tempdb->ChangeBriefcaseId(BeBriefcaseId(0));                                        //Set BriefcaseId to 0 (master)
    tempdb->SaveBriefcaseLocalValue(DgnDbServer::Db::Local::RepositoryURL, "");         //Set URL
    //tempdb->SaveBriefcaseLocalValue(DgnDbServer::Db::Local::RepositoryId, repositoryId);//Set repository ID, we know this only after it is pushed to the server
                                                                                        //Save changes
    tempdb->SaveChanges();
    //NEEDSWORK: end of file cleanup
    return tempdb;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbRepositoryResult> DgnDbClient::CreateNewRepository(Dgn::DgnDbPtr db, Utf8StringCR repositoryName, Utf8StringCR description,
    bool published, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
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
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryResult>(DgnDbRepositoryResult::Error(Error::InvalidCredentials));
        }
    if (repositoryName.empty())
        {
        return CreateCompletedAsyncTask<DgnDbRepositoryResult>(DgnDbRepositoryResult::Error(Error::InvalidRepository));
        }

    DgnDbPtr tempdb = CleanDb(*db);
    if (!tempdb.IsValid())
        CreateCompletedAsyncTask<DgnDbRepositoryResult>(DgnDbRepositoryResult::Error(Error::DbNotFound));
    Utf8String dbFileName = tempdb->GetDbFileName();
    BeFileName fileName = tempdb->GetFileName();
    Utf8String dbFileId = tempdb->GetDbGuid().ToString();
    tempdb->CloseDb();

    std::shared_ptr<DgnDbRepositoryResult> finalResult = std::make_shared<DgnDbRepositoryResult>();
    return GetRepositoriesByPlugin(ServerSchema::Plugin::Admin, cancellationToken)
        ->Then([=] (const WSRepositoriesResult& repositoriesResult)
        {
        if (!repositoriesResult.IsSuccess())
            {
            finalResult->SetError(repositoriesResult.GetError());
            return;
            }

        // Stage 1. Create repository.
        Utf8String adminRepositoryURL = (*repositoriesResult.GetValue().begin()).GetId();
        IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, adminRepositoryURL, m_clientInfo, nullptr, m_customHandler);
        Json::Value repositoryCreationJson = RepositoryCreationJson(repositoryName, dbFileId, description, dbFileName, published);
        client->SetCredentials(m_credentials);
        client->SendCreateObjectRequest(repositoryCreationJson, BeFileName(), callback, cancellationToken)
            ->Then([=] (const WSCreateObjectResult& createRepositoryResult)
            {
            if (!createRepositoryResult.IsSuccess())
                {
                finalResult->SetError(createRepositoryResult.GetError());
                return;
                }

            // Stage 2. Upload master file. 
            JsonValueCR repositoryInstance   = createRepositoryResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            Utf8String  repositoryInstanceId = repositoryInstance[ServerSchema::InstanceId].asString();
            ObjectId    repositoryObjectId   = ObjectId(ServerSchema::Schema::Admin, ServerSchema::Class::Repository, repositoryInstanceId);
            Utf8StringCR url = repositoryInstance[ServerSchema::Properties][ServerSchema::Property::URL].asString();

            if (url.empty())
                {
                client->SendUpdateFileRequest(repositoryObjectId, fileName, callback, cancellationToken)
                    ->Then([=] (const WSUpdateFileResult& uploadFileResult)
                    {
                    if (!uploadFileResult.IsSuccess())
                        {
                        finalResult->SetError(uploadFileResult.GetError());
                        return;
                        }

                    // Stage 3. Initialize repository.
                    InitializeRepository(client, repositoryInstanceId, repositoryCreationJson, repositoryObjectId, callback, cancellationToken)
                        ->Then([=] (const DgnDbRepositoryResult& result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess(result.GetValue());
                        else
                            finalResult->SetError(result.GetError());
                        });
                    });
                }
            else
                {
                IAzureBlobStorageClientPtr azureClient = AzureBlobStorageClient::Create();
                azureClient->SendUpdateFileRequest(url, fileName, callback, cancellationToken)
                    ->Then([=] (const AzureResult& result)
                    {
                    if (!result.IsSuccess())
                        {
                        finalResult->SetError(DgnDbServerError(result.GetError().GetDisplayMessage().c_str()));
                        return;
                        }

                    // Stage 3. Initialize repository.
                    InitializeRepository(client, repositoryInstanceId, repositoryCreationJson, repositoryObjectId, callback, cancellationToken)
                        ->Then([=] (const DgnDbRepositoryResult& result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess(result.GetValue());
                        else
                            finalResult->SetError(result.GetError());
                        });
                    });
                }
            });
        })->Then<DgnDbRepositoryResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbRepositoryResult> DgnDbClient::CreateNewRepository(Dgn::DgnDbPtr db, bool published, HttpRequest::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken)
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
AsyncTaskPtr<DgnDbBriefcaseResult> DgnDbClient::OpenBriefcase(Dgn::DgnDbPtr db, bool doSync, HttpRequest::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        return CreateCompletedAsyncTask<DgnDbBriefcaseResult>(DgnDbBriefcaseResult::Error(Error::DbNotFound));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        return CreateCompletedAsyncTask<DgnDbBriefcaseResult>(DgnDbBriefcaseResult::Error(Error::InvalidCredentials));
        }
    if (db->GetBriefcaseId().IsMasterId())
        {
        return CreateCompletedAsyncTask<DgnDbBriefcaseResult>(DgnDbBriefcaseResult::Error(Error::MasterId));
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
AsyncTaskPtr<DgnDbFileNameResult> DgnDbClient::AquireBriefcase(Utf8StringCR repositoryId, BeFileNameCR localPath, bool doSync,
    HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (repositoryId.empty())
        {
        return CreateCompletedAsyncTask<DgnDbFileNameResult>(DgnDbFileNameResult::Error(Error::InvalidRepository));
        }
    if (m_serverUrl.empty())
        {
        return CreateCompletedAsyncTask<DgnDbFileNameResult>(DgnDbFileNameResult::Error(Error::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
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
                    JsonValueCR instance = briefcaseResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
                    JsonValueCR properties = instance[ServerSchema::Properties];
                    uint32_t briefcaseId = properties[ServerSchema::Property::BriefcaseId].asUInt();
                    Utf8StringCR url = properties[ServerSchema::Property::URL].asString();

                    BeFileName filePath(localPath);
                    if (filePath.IsDirectory())
                        {
                        Utf8String dirName;
                        dirName.Sprintf("%s%u", repository->GetId().c_str(), briefcaseId);
                        filePath.AppendToPath(BeFileName(dirName));
                        filePath.AppendToPath(BeFileName(properties[ServerSchema::Property::FileName].asString()));
                        BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
                        }

                    uint64_t fileSize;
                    BeStringUtilities::ParseUInt64(fileSize, properties[ServerSchema::Property::FileSize].asCString());

                    if (doSync)
                        {
                        auto briefcaseTask = connection->DownloadBriefcaseFile(filePath, BeBriefcaseId(briefcaseId), url, callback, cancellationToken);
                        auto pullTask = connection->Pull("", callback, cancellationToken);
                        bset<std::shared_ptr<AsyncTask>> tasks;
                        tasks.insert(briefcaseTask);
                        tasks.insert(pullTask);
                        AsyncTask::WhenAll(tasks)->Then([=] ()
                            {
                            if (!briefcaseTask->GetResult().IsSuccess())
                                {
                                finalResult->SetError(briefcaseTask->GetResult().GetError());
                                return;
                                }
                            if (!pullTask->GetResult().IsSuccess())
                                {
                                finalResult->SetError(pullTask->GetResult().GetError());
                                return;
                                }

                            BeSQLite::DbResult status;
                            DgnDbServerHost::Adopt(host);
                            Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb (&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
                            if (BeSQLite::DbResult::BE_SQLITE_OK == status)
                                {
                                db->Txns ().EnableTracking (true);
                                bvector<DgnDbServerRevisionPtr> revisions = pullTask->GetResult ().GetValue ();
                                RevisionStatus mergeStatus = RevisionStatus::Success;
                                if (!revisions.empty ())
                                    {
                                    bvector<DgnRevisionPtr> mergeRevisions;
                                    for (auto revision : revisions)
                                        mergeRevisions.push_back (revision->GetRevision ());
                                    mergeStatus = db->Revisions ().MergeRevisions (mergeRevisions);
                                    }
                                    
                                db->CloseDb ();
                                DgnDbServerHost::Forget (host);

                                if (RevisionStatus::Success != mergeStatus)
                                    finalResult->SetError (mergeStatus);
                                else
                                    finalResult->SetSuccess (filePath);
                                }
                            else
                                {
                                DgnDbServerHost::Forget (host);
                                finalResult->SetError (Error::CantWriteToDgnDb);
                                }
                            });
                        }
                    else
                        {
                        auto task = connection->DownloadBriefcaseFile(filePath, BeBriefcaseId(briefcaseId), url, callback, cancellationToken)->Then
                            ([=] (const DgnDbResult& result)
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
Dgn::IRepositoryManager* DgnDbClient::GetRepositoryManagerP()
    {
    return dynamic_cast<Dgn::IRepositoryManager*>(m_repositoryManager.get());
    }

