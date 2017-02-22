/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbClient.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbClient.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <json/json.h>
#include <DgnPlatform/RevisionManager.h>
#include <DgnDbServer/Client/Logging.h>
#include "DgnDbServerUtils.h"
#include <DgnDbServer/Client/DgnDbServerBreakHelper.h>

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
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionResult DgnDbClient::CreateRepositoryConnection(RepositoryInfoCR repositoryInfo) const
    {
    return DgnDbRepositoryConnection::Create(repositoryInfo, m_credentials, m_clientInfo, m_customHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionTaskPtr DgnDbClient::ConnectToRepository(RepositoryInfoCR repositoryInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::ConnectToRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty() || m_serverUrl != repositoryInfo.GetServerURL())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }

    return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(CreateRepositoryConnection(repositoryInfo));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionTaskPtr DgnDbClient::ConnectToRepository(Utf8StringCR repositoryId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::ConnectToRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbRepositoryConnectionResult>(DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }

    return GetRepositoryById(repositoryId, cancellationToken)
        ->Then<DgnDbRepositoryConnectionResult>([=] (DgnDbServerRepositoryResultCR result)
        {
        if (!result.IsSuccess())
            {
            return DgnDbRepositoryConnectionResult::Error(result.GetError());
            }
        return CreateRepositoryConnection(*result.GetValue());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbClient::DgnDbClient(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler)
    : m_clientInfo(clientInfo), m_customHandler(customHandler), m_projectId(""), m_repositoryAdmin(this)
    {
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
    properties[ServerSchema::Property::Password] = credentials.GetPassword();
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

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_customHandler);
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

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_customHandler);
    client->SetCredentials(m_credentials);

    WSQuery query = WSQuery(ServerSchema::Schema::Project, ServerSchema::Class::UserDefinition);
    Utf8String filter;
    filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::Name, credentials.GetUsername().c_str());
    query.SetFilter(filter);

    auto finalResult = std::make_shared<DgnDbServerStatusResult>();
    //Find the desired user
    return client->SendQueryRequest(query, nullptr, nullptr, cancellationToken)
        ->Then([=](const WSObjectsResult& result)
        {
        if (!result.IsSuccess())
            {
            finalResult->SetError(result.GetError());
            return;
            }

        auto instances = result.GetValue().GetInstances();

        if (0 == instances.Size())
            {
            finalResult->SetError({ DgnDbServerError::Id::UserDoesNotExist });
            return;
            }

        if (1 < instances.Size())
            {
            finalResult->SetError({ DgnDbServerError::Id::InternalServerError });
            return;
            }

        for (auto instance : instances)
            {
            client->SendDeleteObjectRequest(instance.GetObjectId())
                ->Then([=](const WSDeleteObjectResult& deleteResult)
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
DgnDbClientPtr DgnDbClient::Create(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler)
    {
    const Utf8String methodName = "DgnDbClient::Create";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
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
void DgnDbClient::SetCredentials(CredentialsCR credentials)
    {
    m_credentials = credentials;
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
    if (m_serverUrl.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoriesResult>(DgnDbServerRepositoriesResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoriesResult>(DgnDbServerRepositoriesResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());
    ObjectId repositoriesObject(ServerSchema::Schema::Project, ServerSchema::Class::Repository, "");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_customHandler);
    client->SetCredentials(m_credentials);
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting repositories from project %s.", project.c_str());
    return client->SendGetObjectRequest(repositoriesObject, nullptr, cancellationToken)->Then<DgnDbServerRepositoriesResult>
        ([=](const WSObjectsResult& response)
        {
        if (!response.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, response.GetError().GetMessage().c_str());
            return DgnDbServerRepositoriesResult::Error(response.GetError());
            }

        bvector<RepositoryInfoPtr> repositories;
        for (const auto& repository : response.GetValue().GetInstances())
            {
            repositories.push_back(RepositoryInfo::Parse(repository, m_serverUrl));
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "Success.");
        return DgnDbServerRepositoriesResult::Success(repositories);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::GetRepositoryByName(Utf8StringCR repositoryName, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::GetRepositoryByName";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting repository with name %s.", repositoryName.c_str());
    WSQuery query = WSQuery(ServerSchema::Schema::Project, ServerSchema::Class::Repository);
    Utf8String filter;
    filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::RepositoryName, repositoryName.c_str());
    query.SetFilter(filter);

    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());
    ObjectId repositoriesObject(ServerSchema::Schema::Project, ServerSchema::Class::Repository, "");
    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_customHandler);
    client->SetCredentials(m_credentials);

    return client->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerRepositoryResult>([=] (WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerRepositoryResult::Error(result.GetError());
            }

        RepositoryInfoPtr repositoryInfo = RepositoryInfo::Parse(result.GetValue().GetJsonValue()[ServerSchema::Instances][0], m_serverUrl);
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
        return DgnDbServerRepositoryResult::Success(repositoryInfo);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::GetRepositoryById(Utf8StringCR repositoryId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::GetRepositoryById";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerRepositoryResult>(DgnDbServerRepositoryResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting repository with id %s.", repositoryId.c_str());
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());
    ObjectId repositoriesObject(ServerSchema::Schema::Project, ServerSchema::Class::Repository, repositoryId);

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_customHandler);
    client->SetCredentials(m_credentials);

    return client->SendGetObjectRequest(repositoriesObject, nullptr, cancellationToken)->Then<DgnDbServerRepositoryResult>([=] (WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerRepositoryResult::Error(result.GetError());
            }

        RepositoryInfoPtr repositoryInfo = RepositoryInfo::Parse(result.GetValue().GetJsonValue()[ServerSchema::Instances][0], m_serverUrl);
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
        return DgnDbServerRepositoryResult::Success(repositoryInfo);
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
    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_customHandler);
    Json::Value repositoryCreationJson = RepositoryCreationJson(repositoryName, description);
    client->SetCredentials(m_credentials);
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Sending create repository request for project %s.", project.c_str());
    return client->SendCreateObjectRequest(repositoryCreationJson, BeFileName(), nullptr, cancellationToken)
        ->Then([=](const WSCreateObjectResult& createRepositoryResult)
        {
#if defined (ENABLE_BIM_CRASH_TESTS)
        DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbClient_AfterCreateRequest);
#endif
        if (createRepositoryResult.IsSuccess())
            {
            Json::Value json;
            createRepositoryResult.GetValue().GetJson(json);
            JsonValueCR repositoryInstance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            auto repositoryInfo = RepositoryInfo::Parse(repositoryInstance, m_serverUrl);
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
        filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::RepositoryName, repositoryName.c_str());
        repositoryQuery.SetFilter(filter);
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Querying repository by name %s.", repositoryName.c_str());
        client->SendQueryRequest(repositoryQuery, nullptr, nullptr, cancellationToken)->Then([=](WSObjectsResult const& queryResult)
            {
            if (!queryResult.IsSuccess())
                {
                finalResult->SetError(queryResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, queryResult.GetError().GetMessage().c_str());
                return;
                }

            if (queryResult.GetValue().GetRapidJsonDocument().IsNull())
                {
                finalResult->SetError(error);
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
                return;
                }

            finalResult->SetSuccess(RepositoryInfo::Parse(*queryResult.GetValue().GetInstances().begin(), m_serverUrl));
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Success.");
            });

        })->Then<DgnDbServerRepositoryResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
void DgnDbClient::SetHttpHandler(IHttpHandlerPtr customHandler)
    {
    m_customHandler = customHandler;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
IHttpHandlerPtr DgnDbClient::GetHttpHandler()
    {
    return m_customHandler;
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
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
    return tempdb;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::CreateNewRepository(Dgn::DgnDbCR db, Utf8StringCR repositoryName, Utf8StringCR description, bool waitForInitialized,
    Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::CreateNewRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
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
    if (!m_credentials.IsValid() && !m_customHandler)
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

    FileInfoPtr fileInfo = FileInfo::Create(*tempdb, description);
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
            connection->UploadNewMasterFile(filePath, *fileInfo, waitForInitialized, callback, cancellationToken)->Then([=] (DgnDbServerFileResultCR fileUploadResult)
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
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerRepositoryTaskPtr DgnDbClient::CreateNewRepository(Dgn::DgnDbCR db, bool waitForInitialized, Http::Request::ProgressCallbackCR callback,
    ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::CreateNewRepository";
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
    return CreateNewRepository(db, name, description, waitForInitialized, callback, cancellationToken);
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
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    auto readResult = RepositoryInfo::ReadRepositoryInfo(*db);
    BeBriefcaseId briefcaseId = db->GetBriefcaseId();
    if (!readResult.IsSuccess() || briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File is not a briefcase.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::FileIsNotBriefcase));
        }
    RepositoryInfoPtr repositoryInfo = readResult.GetValue();
    if (repositoryInfo->GetServerURL() != m_serverUrl)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Briefcase belongs to another server.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseResult>(DgnDbServerBriefcaseResult::Error(DgnDbServerError::Id::InvalidServerURL));
        }
    std::shared_ptr<DgnDbServerBriefcaseResult> finalResult = std::make_shared<DgnDbServerBriefcaseResult>();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to repository %s.", repositoryInfo->GetName().c_str());
    return ConnectToRepository(*repositoryInfo, cancellationToken)->Then([=] (DgnDbRepositoryConnectionResultCR connectionResult)
        {
        if (!connectionResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
            finalResult->SetError(connectionResult.GetError());
            return;
            }

        FileInfoPtr fileInfo = FileInfo::Create(*db, "");
        auto connection = connectionResult.GetValue();
        connection->ValidateBriefcase(fileInfo->GetFileId(), briefcaseId, cancellationToken)
            ->Then([=] (DgnDbServerStatusResultCR validationResult)
            {
            if (!validationResult.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, validationResult.GetError().GetMessage().c_str());
                finalResult->SetError(validationResult.GetError());
                return;
                }
            DgnDbBriefcasePtr briefcase = DgnDbBriefcase::Create(db, connection);
            if (doSync)
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Calling PullAndMerge for briefcase %d.", briefcase->GetBriefcaseId().GetValue());
                briefcase->PullAndMerge(callback, cancellationToken)->Then([=] (const DgnRevisionsResult& result)
                    {
                    if (result.IsSuccess())
                        {
                        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
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
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                finalResult->SetSuccess(briefcase);
                }
            });
        })->Then<DgnDbServerBriefcaseResult>([=] ()
            {
            return *finalResult;
            });
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbClient::RecoverBriefcase(Dgn::DgnDbPtr db, Http::Request::ProgressCallbackCR callback,
                                                       ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::RefreshBriefcase";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    auto readResult = RepositoryInfo::ReadRepositoryInfo(*db);
    BeBriefcaseId briefcaseId = db->GetBriefcaseId();
    if (!readResult.IsSuccess() || briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File is not a briefcase.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::FileIsNotBriefcase));
        }
    RepositoryInfoPtr repositoryInfo = readResult.GetValue();
    BeFileName originalFilePath = db->GetFileName();

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to repository %s.", repositoryInfo->GetName().c_str());
    auto connectionResult = CreateRepositoryConnection(*repositoryInfo);
    if (!connectionResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(connectionResult.GetError()));
        }

    DgnDbRepositoryConnectionPtr connection = connectionResult.GetValue();
        
    auto fileResult = connection->GetBriefcaseFileInfo(briefcaseId, cancellationToken)->GetResult();
    if (!fileResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(fileResult.GetError()));
        }

    auto newFileInfo = fileResult.GetValue();
    BeFileName downloadPath = originalFilePath.GetDirectoryName();
    downloadPath = downloadPath.AppendToPath(BeFileName(newFileInfo->GetFileId().ToString()));
    downloadPath.AppendExtension(originalFilePath.GetExtension().c_str());

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Downloading briefcase with ID %d.", briefcaseId.GetValue());
    auto downloadResult = connection->DownloadBriefcaseFile(downloadPath, briefcaseId, callback, cancellationToken);
#if defined (ENABLE_BIM_CRASH_TESTS)
    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbClient_AfterDownloadBriefcaseFile);
#endif

    if (!downloadResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(downloadResult.GetError()));
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "Download successful.");

    db->CloseDb();
                
    BeFileName backupPath(originalFilePath.GetName());
    backupPath.AppendExtension(L"back");

    BeFileNameStatus status = BeFileName::BeMoveFile(originalFilePath, backupPath);

#if defined (ENABLE_BIM_CRASH_TESTS)
    try {
        DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbClient_AfterDeleteBriefcase);
        }
    catch (...)
        {
        BeFileName::BeMoveFile(backupPath, originalFilePath);
        throw;
        }
#endif 
    if (BeFileNameStatus::Success != status)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::Unknown));
        }
    status = BeFileName::BeMoveFile(downloadPath, originalFilePath);
    if (BeFileNameStatus::Success != status)
        {
        BeFileName::BeMoveFile(backupPath, originalFilePath);
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::Unknown));
        }

    backupPath.BeDeleteFile();
    return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusResult DgnDbClient::DownloadBriefcase(DgnDbRepositoryConnectionPtr connection, BeFileName filePath, BeBriefcaseId briefcaseId, FileInfoCR fileInfo,
                                                        bool doSync, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::DownloadBriefcase";
    if (!doSync)
        return connection->DownloadBriefcaseFile(filePath, BeBriefcaseId(briefcaseId), callback, cancellationToken);

    DgnDbServerStatusResult briefcaseResult = connection->DownloadBriefcaseFile(filePath, BeBriefcaseId(briefcaseId), callback, cancellationToken);
    if (!briefcaseResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, briefcaseResult.GetError().GetMessage().c_str());
        return briefcaseResult;
        }

    BeSQLite::DbResult status;
    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
    if (BeSQLite::DbResult::BE_SQLITE_OK != status)
        {
        DgnDbServerStatusResult result = DgnDbServerStatusResult::Error(DgnDbServerError(*db, status));
        if (!result.IsSuccess())
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
        return result;
        }

    DgnRevisionsTaskPtr pullTask = connection->DownloadRevisionsAfterId(db->Revisions().GetParentRevisionId(), fileInfo.GetFileId(), callback, cancellationToken);
    DgnRevisionsResult pullResult = pullTask->GetResult();
    if (!pullResult.IsSuccess())
        {
        if (db.IsValid())
            db->CloseDb();

        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, pullResult.GetError().GetMessage().c_str());
        return DgnDbServerStatusResult::Error(pullResult.GetError());
        }

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Briefcase file and revisions after revision %s downloaded successfully.", fileInfo.GetMergedRevisionId().c_str());

    db->Txns().EnableTracking(true);
#if defined (ENABLE_BIM_CRASH_TESTS)
    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbClient_AfterOpenBriefcaseForMerge);
#endif
    DgnRevisions revisions = pullTask->GetResult().GetValue();
    RevisionStatus mergeStatus = RevisionStatus::Success;
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Merging revisions.");
    if (!revisions.empty())
        {
        for (auto revision : revisions)
            {
            mergeStatus = db->Revisions().MergeRevision(*revision);
            if (mergeStatus != RevisionStatus::Success)
                break; // TODO: Use the information on the revision that actually failed. 
            }
        }
#if defined (ENABLE_BIM_CRASH_TESTS)
    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbClient_AfterMergeRevisions);
#endif
    db->CloseDb();

    if (RevisionStatus::Success == mergeStatus)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Success.");
        return DgnDbServerStatusResult::Success();
        }

    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
    return DgnDbServerStatusResult::Error(mergeStatus);
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
    if (repositoryInfo.GetId().empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository name.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(DgnDbServerError::Id::InvalidRepositoryName));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(DgnDbServerError::Id::CredentialsNotSet));
        }
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to repository %s.", repositoryInfo.GetName().c_str());

    auto connectionResult = CreateRepositoryConnection(repositoryInfo);
    if (!connectionResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(connectionResult.GetError()));
        }

    DgnDbRepositoryConnectionPtr connection = connectionResult.GetValue();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Acquiring briefcase ID.");
    auto briefcaseResult = connection->CreateBriefcaseInstance(cancellationToken)->GetResult();
#if defined (ENABLE_BIM_CRASH_TESTS)
    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbClient_AfterCreateBriefcaseInstance);
#endif
    if (!briefcaseResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, briefcaseResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(briefcaseResult.GetError()));
        }

    Json::Value json;
    briefcaseResult.GetValue().GetJson(json);
    JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
    DgnDbServerBriefcaseInfoPtr briefcaseInfo = DgnDbServerBriefcaseInfo::Parse(instance);
    FileInfoPtr fileInfo = FileInfo::Parse(instance);

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Acquired briefcase ID %d.", briefcaseInfo->GetId());

    BeFileName filePath = fileNameCallback(baseDirectory, briefcaseInfo->GetId(), connection->GetRepositoryInfo(), *fileInfo);
    if (filePath.DoesPathExist())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File already exists.");
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(DgnDbServerError::Id::FileAlreadyExists));
        }
    if (!filePath.GetDirectoryName().DoesPathExist())
        {
        BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
        }

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Downloading briefcase with ID %d.", briefcaseInfo->GetId());
    DgnDbServerStatusResult downloadResult = DownloadBriefcase(connection, filePath, briefcaseInfo->GetId(), *fileInfo, doSync, callback, cancellationToken);
    if (!downloadResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Error(downloadResult.GetError()));
        }

    briefcaseInfo->SetLocalPath(filePath);
    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "Download successful.");
    return CreateCompletedAsyncTask<DgnDbServerBriefcaseInfoResult>(DgnDbServerBriefcaseInfoResult::Success(briefcaseInfo));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoTaskPtr DgnDbClient::AcquireBriefcase(RepositoryInfoCR repositoryInfo, BeFileNameCR localFileName, bool doSync,
                                                              Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::AcquireBriefcase";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
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
//@bsimethod                                     Eligijus.Mauragas              12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbClient::AbandonBriefcase(RepositoryInfoCR repositoryInfo, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::AbandonBriefcase";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, repositoryInfo.GetWSRepositoryName(), m_clientInfo, nullptr, m_customHandler);
    client->SetCredentials(m_credentials);

    Utf8String briefcaseIdString;
    briefcaseIdString.Sprintf("%u", briefcaseId);
    ObjectId repositoryId = ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase, briefcaseIdString);

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Sending abandon briefcase request. Repository ID: %s.", repositoryInfo.GetId().c_str());
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
//@bsimethod                                     Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbClient::DeleteRepository(RepositoryInfoCR repositoryInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbClient::DeleteRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Schema::Project, m_projectId.c_str());
    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_customHandler);
    client->SetCredentials(m_credentials);
    ObjectId repositoryId = ObjectId("BIMCSProject", "BIMRepository", repositoryInfo.GetId());
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Sending delete repository request. Repository ID: %s.", repositoryInfo.GetId().c_str());
    return client->SendDeleteObjectRequest(repositoryId, cancellationToken)->Then<DgnDbServerStatusResult>([=](WSDeleteObjectResult const& result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(result.GetError());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "Success.");
            return DgnDbServerStatusResult::Success();
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::RepositoryAdmin* DgnDbClient::GetRepositoryAdmin()
    {
    return dynamic_cast<DgnPlatformLib::Host::RepositoryAdmin*>(&m_repositoryAdmin);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2016
//---------------------------------------------------------------------------------------
DgnDbRepositoryManagerTaskPtr DgnDbClient::CreateRepositoryManager(RepositoryInfoCR repositoryInfo, FileInfoCR fileInfo, DgnDbServerBriefcaseInfoCR briefcaseInfo, ICancellationTokenPtr cancellationToken)
    {
    DgnDbRepositoryManagerResultPtr finalResult = std::make_shared<DgnDbRepositoryManagerResult>();
    return ConnectToRepository(repositoryInfo, cancellationToken)->Then([=] (DgnDbRepositoryConnectionResultCR connectionResult)
        {
        if (!connectionResult.IsSuccess())
            {
            finalResult->SetError(connectionResult.GetError());
            return;
            }
        auto connection = connectionResult.GetValue();
        connection->ValidateBriefcase(fileInfo.GetFileId(), briefcaseInfo.GetId(), cancellationToken)->Then([=] (DgnDbServerStatusResultCR result)
            {
            if (!result.IsSuccess())
                {
                finalResult->SetError(result.GetError());
                }
            else
                {
                finalResult->SetSuccess(DgnDbRepositoryManager::Create(connection));
                }
            });
        })->Then<DgnDbRepositoryManagerResult>([=] ()
                {
                return *finalResult;
                });
    }

