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
BriefcaseFileNameCallback DgnDbClient::DefaultFileNameCallback = [](BeFileName baseDirectory, BeBriefcaseId briefcase, RepositoryInfoCR repositoryInfo)
    {
    baseDirectory.AppendToPath(BeFileName(repositoryInfo.GetId()));
    BeFileName briefcaseId;
    briefcaseId.Sprintf(L"%u", briefcase);
    baseDirectory.AppendToPath(briefcaseId);
    baseDirectory.AppendToPath(BeFileName(repositoryInfo.GetFileName()));
    return baseDirectory;
    };

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
DgnDbRepositoryConnectionTaskPtr DgnDbClient::ConnectToRepository(RepositoryInfoCR repository, ICancellationTokenPtr cancellationToken) const
    {
    return DgnDbRepositoryConnection::Create(repository, m_credentials, m_clientInfo, cancellationToken, m_authenticationHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionTaskPtr DgnDbClient::ConnectToRepository(Utf8StringCR repositoryId, ICancellationTokenPtr cancellationToken) const
    {
    return ConnectToRepository (RepositoryInfo(m_serverUrl, repositoryId), cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbClient::DgnDbClient(ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler)
    : m_clientInfo(clientInfo), m_authenticationHandler(authenticationHandler)
    {
    m_repositoryManager = DgnDbRepositoryManager::Create(clientInfo, authenticationHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbClientPtr DgnDbClient::Create(ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler)
    {
    return DgnDbClientPtr(new DgnDbClient(clientInfo, authenticationHandler));
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
AsyncTaskPtr<WSRepositoriesResult> DgnDbClient::GetRepositoriesByPlugin(Utf8StringCR pluginId, ICancellationTokenPtr cancellationToken) const
    {
    IWSClientPtr client = WSClient::Create(m_serverUrl, m_clientInfo, m_authenticationHandler);
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
DgnDbServerRepositoriesTaskPtr DgnDbClient::GetRepositories(ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (m_serverUrl.empty())
        {
        return CreateCompletedAsyncTask<DgnDbServerRepositoriesResult>(DgnDbServerRepositoriesResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_authenticationHandler)
        {
        return CreateCompletedAsyncTask<DgnDbServerRepositoriesResult>(DgnDbServerRepositoriesResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }

    return GetRepositoriesByPlugin(ServerSchema::Plugin::Repository, cancellationToken)->Then<DgnDbServerRepositoriesResult>([=]
        (const WSRepositoriesResult& response)
        {
        if (!response.IsSuccess())
            {
            return DgnDbServerRepositoriesResult::Error(response.GetError());
            }

        bvector<RepositoryInfoPtr> repositories;
        for (const auto& repository : response.GetValue())
            {
            repositories.push_back(std::make_shared<RepositoryInfo>(m_serverUrl, ParseRepositoryId(repository), repository.GetLabel(), repository.GetDescription()));
            }
        return DgnDbServerRepositoriesResult::Success(repositories);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Json::Value RepositoryCreationJson(Utf8StringCR repositoryName, Utf8StringCR repositoryGuid, Utf8StringCR description, Utf8StringCR localPath, bool published)
    {
    Json::Value repositoryCreation(Json::objectValue);
    JsonValueR instance = repositoryCreation[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Admin;
    instance[ServerSchema::ClassName] = ServerSchema::Class::Repository;
    JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
    properties[ServerSchema::Property::Description] = description;
    properties[ServerSchema::Property::RepositoryName] = repositoryName;
    properties[ServerSchema::Property::FileId] = repositoryGuid;
    Utf8String fileName;
    BeStringUtilities::WCharToUtf8(fileName, BeFileName(localPath).GetFileNameAndExtension().c_str());
    properties[ServerSchema::Property::FileName] = fileName;
    uint64_t size;
    BeFileName(localPath).GetFileSize(size);
    properties[ServerSchema::Property::FileSize] = size;
    properties[ServerSchema::Property::Published] = published;
    properties[ServerSchema::Property::IsUploaded] = false;
    return repositoryCreation;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::InitializeRepository(IWSRepositoryClientPtr client, Utf8StringCR repositoryId, Json::Value repositoryCreationJson,
    ObjectId repositoryObjectId, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    Json::Value repositoryProperties = Json::Value(repositoryCreationJson[ServerSchema::Instance][ServerSchema::Properties]);
    repositoryProperties[ServerSchema::Property::IsUploaded] = true;

    DgnDbServerRepositoryResultPtr finalResult = std::make_shared<DgnDbServerRepositoryResult>();
    return client->SendUpdateObjectRequest(repositoryObjectId, repositoryProperties, nullptr, callback, cancellationToken)
        ->Then([=] (const WSUpdateObjectResult& initializeRepositoryResult)
        {
        if (!initializeRepositoryResult.IsSuccess())
            {
            finalResult->SetError(initializeRepositoryResult.GetError());
            return;
            }

        ConnectToRepository(RepositoryInfo(m_serverUrl, repositoryId), cancellationToken)
            ->Then ([=] (const DgnDbRepositoryConnectionResult& result)
            {
            if (result.IsSuccess())
                {
                finalResult->SetSuccess(std::make_shared<RepositoryInfo>(result.GetValue()->GetRepositoryInfo()));
                }
            else
                finalResult->SetError (result.GetError());
            });

        })->Then<DgnDbServerRepositoryResult>([=]
            {
            return *finalResult;
            });
    }

DgnDbPtr CleanDb(DgnDbCR db)
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

    tempdb->Txns().EnableTracking(true);
    //Do cleanup
    auto revision = tempdb->Revisions().StartCreateRevision();
    if (revision.IsValid())
        tempdb->Revisions().FinishCreateRevision();
    tempdb->SaveBriefcaseLocalValue("ParentRevisionId", "");                            //Clear parent revision id
    tempdb->ChangeBriefcaseId(BeBriefcaseId(0));                                        //Set BriefcaseId to 0 (master)
    tempdb->SaveBriefcaseLocalValue(DgnDbServer::Db::Local::RepositoryURL, "");         //Set URL
    //tempdb.SaveBriefcaseLocalValue(DgnDbServer::Db::Local::RepositoryId, repositoryId);//Set repository ID, we know this only after it is pushed to the server
                                                                                        //Save changes
    tempdb->SaveChanges();
    //NEEDSWORK: end of file cleanup
    return tempdb;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::CreateNewRepository(Dgn::DgnDbCR db, Utf8StringCR repositoryName, Utf8StringCR description,
    bool published, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!db.GetFileName().DoesPathExist())
        {
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (m_serverUrl.empty())
        {
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_authenticationHandler)
        {
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    if (repositoryName.empty())
        {
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::InvalidRepostioryName));
        }

    DgnDbPtr tempdb = CleanDb(db);
    if (!tempdb.IsValid())
        CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::FileNotFound));
    Utf8String dbFileName = tempdb->GetDbFileName();
    BeFileName fileName = tempdb->GetFileName();
    Utf8String dbFileId = tempdb->GetDbGuid().ToString();
    tempdb->CloseDb();

    std::shared_ptr<DgnDbServerRepositoryResult> finalResult = std::make_shared<DgnDbServerRepositoryResult>();
    return GetRepositoriesByPlugin(ServerSchema::Plugin::Admin, cancellationToken)
        ->Then([=] (const WSRepositoriesResult& repositoriesResult)
        {
        if (!repositoriesResult.IsSuccess())
            {
            finalResult->SetError(repositoriesResult.GetError());
            return;
            }

        // Stage 1. Create repository.
        Utf8String adminRepositoryURL = repositoriesResult.GetValue()[0].GetId();
        IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, adminRepositoryURL, m_clientInfo, nullptr, m_authenticationHandler);
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
                        ->Then([=] (const DgnDbServerRepositoryResult& result)
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
                        finalResult->SetError(DgnDbServerError(result.GetError()));
                        return;
                        }

                    // Stage 3. Initialize repository.
                    InitializeRepository(client, repositoryInstanceId, repositoryCreationJson, repositoryObjectId, callback, cancellationToken)
                        ->Then([=] (const DgnDbServerRepositoryResult& result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess(result.GetValue());
                        else
                            finalResult->SetError(result.GetError());
                        });
                    });
                }
            });
        })->Then<DgnDbServerRepositoryResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::CreateNewRepository(Dgn::DgnDbCR db, bool published, HttpRequest::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!db.GetFileName().DoesPathExist())
        {
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    Utf8String name;
    db.QueryProperty(name, BeSQLite::PropertySpec(Db::Properties::Name, Db::Properties::ProjectNamespace));
    if (name.empty())
        BeStringUtilities::WCharToUtf8(name, db.GetFileName().GetFileNameWithoutExtension().c_str());
    Utf8String description;
    db.QueryProperty(description, BeSQLite::PropertySpec(Db::Properties::Description, Db::Properties::ProjectNamespace));
    return CreateNewRepository(db, name, description, published, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseTaskPtr DgnDbClient::OpenBriefcase(Dgn::DgnDbPtr db, bool doSync, HttpRequest::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_credentials.IsValid() && !m_authenticationHandler)
        {
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    RepositoryInfo repositoryInfo;
    auto readResult = RepositoryInfo::ReadRepositoryInfo(repositoryInfo, *db);
    if (!readResult.IsSuccess() || db->GetBriefcaseId().IsMasterId())
        {
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::FileIsNotBriefcase));
        }
    std::shared_ptr<DgnDbServerBriefcaseResult> finalResult = std::make_shared<DgnDbServerBriefcaseResult>();
    return ConnectToRepository(repositoryInfo, cancellationToken)->Then([=] (const DgnDbRepositoryConnectionResult& connectionResult)
        {
        if (connectionResult.IsSuccess())
            {
            DgnDbBriefcasePtr briefcase = DgnDbBriefcase::Create(db, connectionResult.GetValue());
            if (doSync)
                {
                briefcase->PullAndMerge(callback, cancellationToken)->Then ([=](const DgnDbServerRevisionsResult& result)
                    {
                    if (result.IsSuccess ())
                        finalResult->SetSuccess (briefcase);
                    else
                        finalResult->SetError (result.GetError ());
                    });
                }
            else
                {
                    finalResult->SetSuccess (briefcase);
                }
            }
        else
            finalResult->SetError(connectionResult.GetError());
        })->Then<DgnDbServerBriefcaseResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbClient::DownloadBriefcase(DgnDbRepositoryConnectionPtr connection, BeFileName filePath, BeBriefcaseId briefcaseId, Utf8StringCR url,
                                                        bool doSync, HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    auto briefcaseTask = connection->DownloadBriefcaseFile(filePath, BeBriefcaseId(briefcaseId), url, callback, cancellationToken);
    if (!doSync)
        return briefcaseTask;

    auto pullTask = connection->Pull(connection->GetRepositoryInfo().GetMergedRevisionId(), callback, cancellationToken);
    bset<std::shared_ptr<AsyncTask>> tasks;
    tasks.insert(briefcaseTask);
    tasks.insert(pullTask);
    return AsyncTask::WhenAll(tasks)->Then<DgnDbServerStatusResult>([=] ()
        {
        if (!briefcaseTask->GetResult().IsSuccess())
            {
            return DgnDbServerStatusResult::Error(briefcaseTask->GetResult().GetError());
            }
        if (!pullTask->GetResult().IsSuccess())
            {
            return DgnDbServerStatusResult::Error(pullTask->GetResult().GetError());
            }

        BeSQLite::DbResult status;
        std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
        DgnDbServerHost::Adopt(host);
        Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
        if (BeSQLite::DbResult::BE_SQLITE_OK == status)
            {
            db->Txns().EnableTracking(true);
            bvector<DgnDbServerRevisionPtr> revisions = pullTask->GetResult().GetValue();
            RevisionStatus mergeStatus = RevisionStatus::Success;
            if (!revisions.empty())
                {
                for (auto revision : revisions)
                    {
                    mergeStatus = db->Revisions().MergeRevision(*(revision->GetRevision()));
                    if (mergeStatus != RevisionStatus::Success)
                        break; // TODO: Use the information on the revision that actually failed. 
                    }
                }

            db->CloseDb();
            DgnDbServerHost::Forget(host);

            if (RevisionStatus::Success == mergeStatus)
                return DgnDbServerStatusResult::Success();

            return DgnDbServerStatusResult::Error(mergeStatus);
            }
        else
            {
            DgnDbServerStatusResult result = DgnDbServerStatusResult::Error(DgnDbServerError(*db, status));
            DgnDbServerHost::Forget(host);
            return result;
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
DgnDbServerFileNameTaskPtr DgnDbClient::AcquireBriefcaseToDir(RepositoryInfoCR repositoryInfo, BeFileNameCR baseDirectory, bool doSync, BriefcaseFileNameCallback const& fileNameCallback,
                                                              HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (repositoryInfo.GetId().empty())
        {
        return CreateCompletedAsyncTask<DgnDbServerFileNameResult>(DgnDbServerFileNameResult::Error(DgnDbServerError::Id::InvalidRepostioryName));
        }
    if (repositoryInfo.GetServerURL().empty())
        {
        return CreateCompletedAsyncTask<DgnDbServerFileNameResult>(DgnDbServerFileNameResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_authenticationHandler)
        {
        return CreateCompletedAsyncTask<DgnDbServerFileNameResult>(DgnDbServerFileNameResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    std::shared_ptr<DgnDbServerFileNameResult> finalResult = std::make_shared<DgnDbServerFileNameResult>();
    return ConnectToRepository(repositoryInfo, cancellationToken)->Then([=] (const DgnDbRepositoryConnectionResult& connectionResult)
        {
        if (!connectionResult.IsSuccess())
            {
            finalResult->SetError(connectionResult.GetError());
            return;
            }

        DgnDbRepositoryConnectionPtr connection = connectionResult.GetValue();
        connection->AcquireBriefcaseId(cancellationToken)->Then([=] (const WSCreateObjectResult& briefcaseResult)
            {
            if (!briefcaseResult.IsSuccess())
                {
                finalResult->SetError(briefcaseResult.GetError());
                return;
                }

            JsonValueCR instance = briefcaseResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            JsonValueCR properties = instance[ServerSchema::Properties];
            uint32_t briefcaseId = properties[ServerSchema::Property::BriefcaseId].asUInt();
            Utf8StringCR url = properties[ServerSchema::Property::URL].asString();

            BeFileName filePath = fileNameCallback(baseDirectory, BeBriefcaseId(briefcaseId), connection->GetRepositoryInfo());
            if (filePath.DoesPathExist())
                {
                finalResult->SetError(DgnDbServerError::Id::FileAlreadyExists);
                return;
                }
            if (!filePath.GetDirectoryName().DoesPathExist())
                {
                BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
                }

            DownloadBriefcase(connection, filePath, BeBriefcaseId(briefcaseId), url, doSync, callback, cancellationToken)->Then([=] (DgnDbServerStatusResultCR downloadResult)
                {
                if (downloadResult.IsSuccess())
                    finalResult->SetSuccess(filePath);
                else
                    finalResult->SetError(downloadResult.GetError());
                });

            });
        })->Then<DgnDbServerFileNameResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerFileNameTaskPtr DgnDbClient::AcquireBriefcase(RepositoryInfoCR repositoryInfo, BeFileNameCR localFileName, bool doSync,
                                                         HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (localFileName.DoesPathExist() && !localFileName.IsDirectory())
        {
        return CreateCompletedAsyncTask<DgnDbServerFileNameResult>(DgnDbServerFileNameResult::Error(DgnDbServerError::Id::FileAlreadyExists));
        }
    return AcquireBriefcaseToDir(repositoryInfo, localFileName, doSync, [=] (BeFileName baseDirectory, BeBriefcaseId, RepositoryInfoCR repositoryInfo)
        {
        if (baseDirectory.IsDirectory())
            baseDirectory.AppendToPath(BeFileName(repositoryInfo.GetFileName()));
        return baseDirectory;
        }, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Dgn::IRepositoryManager* DgnDbClient::GetRepositoryManagerP()
    {
    return dynamic_cast<Dgn::IRepositoryManager*>(m_repositoryManager.get());
    }

