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
#include <DgnDbServer/Client/Logging.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNPLATFORM
BriefcaseFileNameCallback DgnDbClient::DefaultFileNameCallback = [](BeFileName baseDirectory, BeBriefcaseId briefcase, RepositoryInfoCR repositoryInfo, FileInfoCR fileInfo)
    {
    baseDirectory.AppendToPath(BeFileName(repositoryInfo.GetId()));
    BeFileName briefcaseId;
    briefcaseId.Sprintf(L"%u", briefcase);
    baseDirectory.AppendToPath(briefcaseId);
    baseDirectory.AppendToPath(BeFileName(fileInfo.GetFileName()));
    return baseDirectory;
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  06/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfo::DgnDbServerBriefcaseInfo(BeFileName filePath, bool isReadOnly)
    : m_filePath(filePath), m_isReadOnly(isReadOnly)
    {
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  06/2016
//---------------------------------------------------------------------------------------
BeFileName DgnDbServerBriefcaseInfo::FilePath()     const { return m_filePath; }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  06/2016
//---------------------------------------------------------------------------------------
bool       DgnDbServerBriefcaseInfo::IsReadOnly()   const { return m_isReadOnly; }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
void DgnDbClient::Initialize()
    {
    auto host = Dgn::DgnPlatformLib::QueryHost();
    BeAssert(host);

    BeFileName temp = host->GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    BeFileName assets = host->GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    DgnDbServerHost::Initialize(temp, assets);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionTaskPtr DgnDbClient::ConnectToRepository(RepositoryInfoCR repository, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::ConnectToRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return DgnDbRepositoryConnection::Create(repository, m_credentials, m_clientInfo, cancellationToken, m_authenticationHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionTaskPtr DgnDbClient::ConnectToRepository(Utf8StringCR repositoryId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::ConnectToRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return ConnectToRepository(RepositoryInfo(m_serverUrl, repositoryId), cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbClient::DgnDbClient(ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler)
    : m_clientInfo(clientInfo), m_authenticationHandler(authenticationHandler), m_projectId("")
    {
    m_repositoryManager = DgnDbRepositoryManager::Create(clientInfo, authenticationHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
Json::Value BasicUserCreationJson(Credentials credentials, bool isAdmin = false)
    {
    Json::Value repositoryCreation(Json::objectValue);
    JsonValueR instance = repositoryCreation[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Project;
    instance[ServerSchema::ClassName] = ServerSchema::Class::UserDefinition;
    JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
    properties[ServerSchema::Property::Name] = credentials.GetUsername();
    properties[ServerSchema::Property::Password] = credentials.GetPassword();;
    properties[ServerSchema::Property::IsAdmin] = isAdmin;
    return repositoryCreation;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbClient::CreateBasicUser(Credentials credentials, ICancellationTokenPtr cancellationToken)
    {
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_authenticationHandler);  
    client->SetCredentials(m_credentials);

    Json::Value basicUserCreationJson = BasicUserCreationJson(credentials);
    return client->SendCreateObjectRequest(basicUserCreationJson, BeFileName(), nullptr, cancellationToken)
        ->Then<DgnDbServerStatusResult>([=] (const WSCreateObjectResult& result)
        {
        if (!result.IsSuccess())
            return DgnDbServerStatusResult::Error(result.GetError());

        return DgnDbServerStatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbClient::RemoveBasicUser(Credentials credentials, ICancellationTokenPtr cancellationToken)
    {
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_authenticationHandler); 
    client->SetCredentials(m_credentials);

    WSQuery query = WSQuery(ServerSchema::Schema::Project, ServerSchema::Class::UserDefinition);
    Utf8String filter;
    filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::Name, credentials.GetUsername().c_str());
    query.SetFilter(filter);

    auto finalResult = std::make_shared<DgnDbServerStatusResult>();
    //Find the desired user
    return client->SendQueryRequest(query, nullptr, nullptr, cancellationToken)
        ->Then([=] (const WSObjectsResult& result)
        {
        if (!result.IsSuccess())
            {
            finalResult->SetError(result.GetError());
            return;
            }

        auto instances = result.GetValue().GetInstances();

        if (0 == instances.Size())
            {
            finalResult->SetError({DgnDbServerError::Id::UserDoesNotExist});
            return;
            }

        if (1 < instances.Size())
            {
            finalResult->SetError({DgnDbServerError::Id::InternalServerError});
            return;
            }

        for (auto instance : instances)
            {
            client->SendDeleteObjectRequest(instance.GetObjectId())
                ->Then([=] (const WSDeleteObjectResult& deleteResult)
                {
                if (!deleteResult.IsSuccess())
                    {
                    finalResult->SetError(deleteResult.GetError());
                    return;
                    }

                finalResult->SetSuccess();
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
DgnDbClientPtr DgnDbClient::Create(ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler)
    {
    const Utf8String methodName = "DgnDbClient::Create";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
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
void DgnDbClient::SetCredentials(CredentialsCR credentials)
    {
    m_credentials = credentials;
    m_repositoryManager->SetCredentials(credentials);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  06/2016
//---------------------------------------------------------------------------------------
void DgnDbClient::SetProject(Utf8StringCR projectId)
    {
    m_projectId = projectId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRepositoriesTaskPtr DgnDbClient::GetRepositories(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::GetRepositories";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    BeAssert(DgnDbServerHost::IsInitialized());
    if (m_serverUrl.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoriesResult>(DgnDbServerRepositoriesResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_authenticationHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoriesResult>(DgnDbServerRepositoriesResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());
    ObjectId repositoriesObject(ServerSchema::Schema::Project, ServerSchema::Class::Repository, "");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_authenticationHandler);
    client->SetCredentials(m_credentials);
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting repositories from project %s.", project.c_str());
    return client->SendGetObjectRequest(repositoriesObject, nullptr, cancellationToken)->Then<DgnDbServerRepositoriesResult>
        ([=] (const WSObjectsResult& response)
        {
        if (!response.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, response.GetError().GetMessage().c_str());
            return DgnDbServerRepositoriesResult::Error(response.GetError());
            }

        bvector<RepositoryInfoPtr> repositories;
        for (const auto& repository : response.GetValue().GetJsonValue()[ServerSchema::Instances])
            {
            repositories.push_back(RepositoryInfo::FromJson(repository, m_serverUrl));
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "Success.");
        return DgnDbServerRepositoriesResult::Success(repositories);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Json::Value RepositoryCreationJson(Utf8StringCR repositoryName, Utf8StringCR description)
    {
    Json::Value repositoryCreation(Json::objectValue);
    JsonValueR instance = repositoryCreation[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Project;
    instance[ServerSchema::ClassName] = ServerSchema::Class::Repository;
    JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
    properties[ServerSchema::Property::RepositoryName] = repositoryName;
    properties[ServerSchema::Property::RepositoryDescription] = description;
    return repositoryCreation;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::CreateRepositoryInstance(Utf8StringCR repositoryName, Utf8StringCR description,
                                                               ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::CreateRepositoryInstance";
    std::shared_ptr<DgnDbServerRepositoryResult> finalResult = std::make_shared<DgnDbServerRepositoryResult>();
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());
    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_authenticationHandler);
    Json::Value repositoryCreationJson = RepositoryCreationJson(repositoryName, description);
    client->SetCredentials(m_credentials);
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Sending create repository request for project %s.", project.c_str());
    return client->SendCreateObjectRequest(repositoryCreationJson, BeFileName(), nullptr, cancellationToken)
        ->Then([=] (const WSCreateObjectResult& createRepositoryResult)
        {
        if (createRepositoryResult.IsSuccess())
            {
            JsonValueCR repositoryInstance = createRepositoryResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            auto repositoryInfo = RepositoryInfo::FromJson(repositoryInstance, m_serverUrl);
            finalResult->SetSuccess(repositoryInfo);
            return;
            }

        auto error = DgnDbServerError(createRepositoryResult.GetError());
        if (DgnDbServerError::Id::RepositoryAlreadyExists != error.GetId())
            {
            finalResult->SetError(error);
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
            return;
            }

        bool initialized = error.GetExtendedData()[ServerSchema::Property::RepositoryInitialized].asBool();

        if (initialized)
            {
            finalResult->SetError(error);
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
            return;
            }

        WSQuery repositoryQuery(ServerSchema::Schema::Project, ServerSchema::Class::Repository);
        Utf8String filter;
        filter.Sprintf("%s+eq+%s", ServerSchema::Property::RepositoryName, repositoryName.c_str());
        repositoryQuery.SetFilter(filter);
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Querying repository by name %s.", repositoryName.c_str());
        client->SendQueryRequest(repositoryQuery, nullptr, nullptr, cancellationToken)->Then([=] (WSObjectsResult const& queryResult)
            {
            if (!queryResult.IsSuccess())
                {
                finalResult->SetError(queryResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, queryResult.GetError().GetMessage().c_str());
                return;
                }
            JsonValueCR repositoryInstances = queryResult.GetValue().GetJsonValue()[ServerSchema::Instances];
            if (repositoryInstances.isArray())
                {
                finalResult->SetSuccess(RepositoryInfo::FromJson(repositoryInstances[0], m_serverUrl));
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Success.");
                }
            else
                {
                finalResult->SetError(error);
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
                }
            });

        })->Then<DgnDbServerRepositoryResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
DgnDbPtr CleanDb(DgnDbCR db)
    {
    const Utf8String methodName = "CleanDb";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
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
    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
    return tempdb;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::CreateNewRepository(Dgn::DgnDbCR db, Utf8StringCR repositoryName, Utf8StringCR description,
    Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::CreateNewRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!db.GetFileName().DoesPathExist())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (m_serverUrl.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid server URL.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_authenticationHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    if (repositoryName.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository name.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::InvalidRepositoryName));
        }

    DgnDbPtr tempdb = CleanDb(db);
    if (!tempdb.IsValid())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::FileNotFound));
        }

    Utf8String dbFileName = tempdb->GetDbFileName();
    BeFileName fileName = tempdb->GetFileName();
    Utf8String dbFileId = tempdb->GetDbGuid().ToString();
    FileInfo fileInfo = FileInfo(*tempdb, description);
    BeFileName filePath = tempdb->GetFileName();
    tempdb->CloseDb();

    std::shared_ptr<DgnDbServerRepositoryResult> finalResult = std::make_shared<DgnDbServerRepositoryResult>();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Creating repository instance. Name: %s.", repositoryName.c_str());
    return CreateRepositoryInstance(repositoryName, description, cancellationToken)
        ->Then([=] (DgnDbServerRepositoryResultCR createRepositoryResult)
        {
        if (!createRepositoryResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, createRepositoryResult.GetError().GetMessage().c_str());
            finalResult->SetError(createRepositoryResult.GetError());
            return;
            }

        auto repositoryInfo = createRepositoryResult.GetValue();
        finalResult->SetSuccess(repositoryInfo);

        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Successfully created repository instance. Instance ID: %s.", repositoryInfo->GetId().c_str());
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to created repository.");
        ConnectToRepository(repositoryInfo->GetId(), cancellationToken)->Then([=] (DgnDbRepositoryConnectionResultCR connectionResult)
            {
            if (!connectionResult.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
                finalResult->SetError(connectionResult.GetError());
                return;
                }
            DgnDbRepositoryConnectionPtr connection = connectionResult.GetValue();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Uploading new master file.");
            connection->UploadNewMasterFile(filePath, fileInfo, callback, cancellationToken)->Then([=] (DgnDbServerFileResultCR fileUploadResult)
                {
                if (!fileUploadResult.IsSuccess())
                    {
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileUploadResult.GetError().GetMessage().c_str());
                    finalResult->SetError(fileUploadResult.GetError());
                    }
                });
            });

        })->Then<DgnDbServerRepositoryResult>([=]
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::CreateNewRepository(Dgn::DgnDbCR db, Http::Request::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::CreateNewRepository";
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!db.GetFileName().DoesPathExist())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    Utf8String name;
    db.QueryProperty(name, BeSQLite::PropertySpec(Db::Properties::Name, Db::Properties::ProjectNamespace));
    if (name.empty())
        BeStringUtilities::WCharToUtf8(name, db.GetFileName().GetFileNameWithoutExtension().c_str());
    Utf8String description;
    db.QueryProperty(description, BeSQLite::PropertySpec(Db::Properties::Description, Db::Properties::ProjectNamespace));
    return CreateNewRepository(db, name, description, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseTaskPtr DgnDbClient::OpenBriefcase(Dgn::DgnDbPtr db, bool doSync, Http::Request::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::OpenBriefcase";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_credentials.IsValid() && !m_authenticationHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    RepositoryInfo repositoryInfo;
    auto readResult = RepositoryInfo::ReadRepositoryInfo(repositoryInfo, *db);
    if (!readResult.IsSuccess() || db->GetBriefcaseId().IsMasterId())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File is not a briefcase.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::FileIsNotBriefcase));
        }
    std::shared_ptr<DgnDbServerBriefcaseResult> finalResult = std::make_shared<DgnDbServerBriefcaseResult>();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to repository %s.", repositoryInfo.GetName().c_str());
    return ConnectToRepository(repositoryInfo, cancellationToken)->Then([=] (const DgnDbRepositoryConnectionResult& connectionResult)
        {
        if (connectionResult.IsSuccess())
            {
            DgnDbBriefcasePtr briefcase = DgnDbBriefcase::Create(db, connectionResult.GetValue());
            if (doSync)
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Calling PullAndMerge for briefcase %d.", briefcase->GetBriefcaseId().GetValue());
                briefcase->PullAndMerge(callback, cancellationToken)->Then([=](const DgnDbServerRevisionsResult& result)
                    {
                    if (result.IsSuccess())
                        {
                        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
                        finalResult->SetSuccess(briefcase);
                        }
                    else
                        {
                        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                        finalResult->SetError(result.GetError());
                        }
                    });
                }
            else
                {
                    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
                    finalResult->SetSuccess(briefcase);
                }
            }
        else
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
            finalResult->SetError(connectionResult.GetError());
            }
        })->Then<DgnDbServerBriefcaseResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbClient::DownloadBriefcase(DgnDbRepositoryConnectionPtr connection, BeFileName filePath, BeBriefcaseId briefcaseId, FileInfoCR fileInfo,
                                                        bool doSync, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::DownloadBriefcase";
    auto briefcaseTask = connection->DownloadBriefcaseFile(filePath, BeBriefcaseId(briefcaseId), fileInfo.GetFileURL(), callback, cancellationToken);
    if (!doSync)
        return briefcaseTask;

    auto pullTask = connection->DownloadRevisionsAfterId(fileInfo.GetMergedRevisionId(), callback, cancellationToken);
    bset<std::shared_ptr<AsyncTask>> tasks;
    tasks.insert(briefcaseTask);
    tasks.insert(pullTask);
    return AsyncTask::WhenAll(tasks)->Then<DgnDbServerStatusResult>([=] ()
        {
        if (!briefcaseTask->GetResult().IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, briefcaseTask->GetResult().GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(briefcaseTask->GetResult().GetError());
            }
        if (!pullTask->GetResult().IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, pullTask->GetResult().GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(pullTask->GetResult().GetError());
            }

        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Briefcase file and revisions after revision %s downloaded successfully.", fileInfo.GetMergedRevisionId().c_str());

        BeSQLite::DbResult status;
        std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
        DgnDbServerHost::Adopt(host);
        Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
        if (BeSQLite::DbResult::BE_SQLITE_OK == status)
            {
            db->Txns().EnableTracking(true);
            bvector<DgnDbServerRevisionPtr> revisions = pullTask->GetResult().GetValue();
            RevisionStatus mergeStatus = RevisionStatus::Success;
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Merging revisions.");
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
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Success.");
                return DgnDbServerStatusResult::Success();
                }

            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
            return DgnDbServerStatusResult::Error(mergeStatus);
            }
        else
            {
            DgnDbServerStatusResult result = DgnDbServerStatusResult::Error(DgnDbServerError(*db, status));
            DgnDbServerHost::Forget(host);
            if (!result.IsSuccess())
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return result;
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoTaskPtr DgnDbClient::AcquireBriefcaseToDir(RepositoryInfoCR repositoryInfo, BeFileNameCR baseDirectory, bool doSync, BriefcaseFileNameCallback const& fileNameCallback,
                                                                   Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::AcquireBriefcaseToDir";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    BeAssert(DgnDbServerHost::IsInitialized());
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (repositoryInfo.GetId().empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository name.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(DgnDbServerError::Id::InvalidRepositoryName));
        }
    if (repositoryInfo.GetServerURL().empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid server URL.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_authenticationHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    std::shared_ptr<DgnDbServerBriefcaseInfoResult> finalResult = std::make_shared<DgnDbServerBriefcaseInfoResult>();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to repository %s.", repositoryInfo.GetName().c_str());
    return ConnectToRepository(repositoryInfo, cancellationToken)->Then([=] (const DgnDbRepositoryConnectionResult& connectionResult)
        {
        if (!connectionResult.IsSuccess())
            {
            finalResult->SetError(connectionResult.GetError());
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
            return;
            }

        DgnDbRepositoryConnectionPtr connection = connectionResult.GetValue();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Acquiring briefcase ID.");
        connection->AcquireBriefcaseId(cancellationToken)->Then([=] (const WSCreateObjectResult& briefcaseResult)
            {
            if (!briefcaseResult.IsSuccess())
                {
                finalResult->SetError(briefcaseResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, briefcaseResult.GetError().GetMessage().c_str());
                return;
                }

            JsonValueCR instance = briefcaseResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            JsonValueCR properties = instance[ServerSchema::Properties];
            uint32_t briefcaseId = properties[ServerSchema::Property::BriefcaseId].asUInt();
            bool isReadOnly = properties[ServerSchema::Property::IsReadOnly].asBool();
            FileInfoPtr fileInfo = FileInfo::FromJson(instance);

            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Acquired briefcase ID %d.", briefcaseId);

            BeFileName filePath = fileNameCallback(baseDirectory, BeBriefcaseId(briefcaseId), connection->GetRepositoryInfo(), *fileInfo);
            if (filePath.DoesPathExist())
                {
                finalResult->SetError(DgnDbServerError::Id::FileAlreadyExists);
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File already exists.");
                return;
                }
            if (!filePath.GetDirectoryName().DoesPathExist())
                {
                BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
                }

            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Downloading briefcase with ID %d.", briefcaseId);
            DownloadBriefcase(connection, filePath, BeBriefcaseId(briefcaseId), *fileInfo, doSync, callback, cancellationToken)->Then([=] (DgnDbServerStatusResultCR downloadResult)
                {
                if (downloadResult.IsSuccess())
                    {
                    finalResult->SetSuccess(DgnDbServerBriefcaseInfo(filePath, isReadOnly));
                    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "Download successful.");
                    }
                else
                    {
                    finalResult->SetError(downloadResult.GetError());
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
                    }
                });

            });
        })->Then<DgnDbServerBriefcaseInfoResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoTaskPtr DgnDbClient::AcquireBriefcase(RepositoryInfoCR repositoryInfo, BeFileNameCR localFileName, bool doSync,
                                                              Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::AcquireBriefcase";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    BeAssert(DgnDbServerHost::IsInitialized());
    if (localFileName.DoesPathExist() && !localFileName.IsDirectory())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File already exists.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(DgnDbServerError::Id::FileAlreadyExists));
        }
    return AcquireBriefcaseToDir(repositoryInfo, localFileName, doSync, [=] (BeFileName baseDirectory, BeBriefcaseId, RepositoryInfoCR repositoryInfo, FileInfoCR fileInfo)
        {
        if (baseDirectory.IsDirectory())
            baseDirectory.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return baseDirectory;
        }, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbClient::DeleteRepository(RepositoryInfoCR repositoryInfo, ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "DgnDbClient::DeleteRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());
    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_authenticationHandler);
    client->SetCredentials(m_credentials);
    ObjectId repositoryId = ObjectId("BIMCSProject", "BIMRepository", repositoryInfo.GetId());
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Sending delete repository request. Repository ID: %s.", repositoryInfo.GetId().c_str());
    return client->SendDeleteObjectRequest(repositoryId, cancellationToken)->Then<DgnDbServerStatusResult>([=] (WSDeleteObjectResult const& result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(result.GetError());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "Success.");
            return DgnDbServerStatusResult::Success();
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Dgn::IRepositoryManager* DgnDbClient::GetRepositoryManagerP()
    {
    return dynamic_cast<Dgn::IRepositoryManager*>(m_repositoryManager.get());
    }

