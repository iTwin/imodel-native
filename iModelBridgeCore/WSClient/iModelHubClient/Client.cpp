/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Client.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/Client.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <json/json.h>
#include <DgnPlatform/RevisionManager.h>
#include "Logging.h"
#include "Utils.h"
#include <WebServices/iModelHub/Client/BreakHelper.h>
#include "MultiProgressCallbackHandler.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGN

BriefcaseFileNameCallback Client::DefaultFileNameCallback = [](BeFileName baseDirectory, iModelInfoCR iModelInfo, BriefcaseInfoCR briefcaseInfo)
    {
    baseDirectory.AppendToPath(BeFileName(iModelInfo.GetId()));
    BeFileName briefcaseId;
    briefcaseId.Sprintf(L"%u", briefcaseInfo.GetId());
    baseDirectory.AppendToPath(briefcaseId);
    baseDirectory.AppendToPath(BeFileName(briefcaseInfo.GetFileName()));
    return baseDirectory;
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
IWSRepositoryClientPtr Client::CreateProjectConnection(Utf8StringCR projectId) const
    {
    Utf8String project;
    project.Sprintf("%s--%s", ServerSchema::Plugin::Project, projectId.c_str());
    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, project, m_clientInfo, nullptr, m_customHandler);
    client->SetCredentials(m_credentials);
    return client;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
iModelConnectionTaskPtr Client::ConnectToiModel(iModelInfoCR iModelInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::ConnectToiModel";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty() || m_serverUrl != iModelInfo.GetServerURL())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<iModelConnectionResult>(iModelConnectionResult::Error(Error::Id::InvalidServerURL));//NEEDSWORK: different message?
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<iModelConnectionResult>(iModelConnectionResult::Error(Error::Id::CredentialsNotSet));
        }

    return CreateCompletedAsyncTask<iModelConnectionResult>(CreateiModelConnection(iModelInfo));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
iModelConnectionTaskPtr Client::ConnectToiModel(Utf8StringCR projectId, Utf8StringCR iModelId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::ConnectToiModel";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<iModelConnectionResult>(iModelConnectionResult::Error(Error::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<iModelConnectionResult>(iModelConnectionResult::Error(Error::Id::CredentialsNotSet));
        }

    return GetiModelById(projectId, iModelId, cancellationToken)
        ->Then<iModelConnectionResult>([=](iModelResultCR result)
        {
        if (!result.IsSuccess())
            {
            return iModelConnectionResult::Error(result.GetError());
            }
        return CreateiModelConnection(*result.GetValue());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ClientPtr Client::Create(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler)
    {
    const Utf8String methodName = "Client::Create";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return new Client(clientInfo, customHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
iModelsTaskPtr Client::GetiModels(Utf8StringCR projectId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::GetiModels";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<iModelsResult>(iModelsResult::Error(Error::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<iModelsResult>(iModelsResult::Error(Error::Id::CredentialsNotSet));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    WSQuery query = WSQuery(ServerSchema::Schema::Project, ServerSchema::Class::iModel);

    //Always select HasCreatorInfo relationship
    Utf8String select = "*";
    iModelInfo::AddHasCreatorInfoSelect(select);
    query.SetSelect(select);

    IWSRepositoryClientPtr client = CreateProjectConnection(projectId);
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting iModels from project %s.", projectId.c_str());

    return client->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<iModelsResult>([=] (WSObjectsResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return iModelsResult::Error(result.GetError());
            }
        bvector<iModelInfoPtr> iModels;
        for (const auto& iModel : result.GetValue().GetInstances())
            {
            iModels.push_back(iModelInfo::Parse(iModel, m_serverUrl));
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "Success.");
        return iModelsResult::Success(iModels);
        });
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::GetiModelByName(Utf8StringCR projectId, Utf8StringCR iModelName, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::GetiModelByName";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::CredentialsNotSet));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting iModel with name %s.", iModelName.c_str());
    WSQuery query = WSQuery(ServerSchema::Schema::Project, ServerSchema::Class::iModel);
    Utf8String filter;
    Utf8String updatedName = BeUri::EscapeString(iModelName);
    filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::iModelName, updatedName.c_str());
    query.SetFilter(filter);

    //Always select HasCreatorInfo relationship
    Utf8String select = "*";
    iModelInfo::AddHasCreatorInfoSelect(select);
    query.SetSelect(select);

    IWSRepositoryClientPtr client = CreateProjectConnection(projectId);
    return client->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<iModelResult>([=] (WSObjectsResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
            return iModelResult::Error(result.GetError());
            }
        auto iModelInfoInstances = result.GetValue().GetInstances();
        if (iModelInfoInstances.Size() == 0)
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "iModel does not exist.");
            return iModelResult::Error(Error::Id::iModelDoesNotExist);
            }
        iModelInfoPtr iModelInfo = iModelInfo::Parse(*iModelInfoInstances.begin(), m_serverUrl);
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
        return iModelResult::Success(iModelInfo);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::GetiModelById(Utf8StringCR projectId, Utf8StringCR iModelId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::GetiModelById";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_serverUrl.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::CredentialsNotSet));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting iModel with id %s.", iModelId.c_str());

    WSQuery query = WSQuery(ServerSchema::Schema::Project, ServerSchema::Class::iModel);
    Utf8String filter;
    filter.Sprintf("$id+eq+'%s'", iModelId.c_str());
    query.SetFilter(filter);

    //Always select HasCreatorInfo relationship
    Utf8String select = "*";
    iModelInfo::AddHasCreatorInfoSelect(select);
    query.SetSelect(select);

    IWSRepositoryClientPtr client = CreateProjectConnection(projectId);
    return client->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<iModelResult>([=] (WSObjectsResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
            return iModelResult::Error(result.GetError());
            }

        auto iModelInfoInstances = result.GetValue().GetInstances();
        if (iModelInfoInstances.Size() == 0)
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "iModel does not exist.");
            return iModelResult::Error(Error::Id::iModelDoesNotExist);
            }
        iModelInfoPtr iModelInfo = iModelInfo::Parse(*iModelInfoInstances.begin(), m_serverUrl);
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "");
        return iModelResult::Success(iModelInfo);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Json::Value iModelCreationJson(Utf8StringCR iModelName, Utf8StringCR description)
    {
    Json::Value iModelCreation(Json::objectValue);
    JsonValueR instance = iModelCreation[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Project;
    instance[ServerSchema::ClassName] = ServerSchema::Class::iModel;
    JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
    properties[ServerSchema::Property::iModelName] = iModelName;
    properties[ServerSchema::Property::iModelDescription] = description;
    return iModelCreation;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateiModelInstance(Utf8StringCR projectId, Utf8StringCR iModelName, Utf8StringCR description,
                                           ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::CreateiModelInstance";
    std::shared_ptr<iModelResult> finalResult = std::make_shared<iModelResult>();

    Json::Value imodelCreationJson = iModelCreationJson(iModelName, description);
    IWSRepositoryClientPtr client = CreateProjectConnection(projectId);
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Sending create iModel request for project %s.", projectId.c_str());
    return client->SendCreateObjectRequest(imodelCreationJson, BeFileName(), nullptr, cancellationToken)
        ->Then([=](const WSCreateObjectResult& createiModelResult)
        {
#if defined (ENABLE_BIM_CRASH_TESTS)
        BreakHelper::HitBreakpoint(Breakpoints::Client_AfterCreateRequest);
#endif
        if (createiModelResult.IsSuccess())
            {
            Json::Value json;
            createiModelResult.GetValue().GetJson(json);
            JsonValueCR iModelInstance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            auto iModelInfo = iModelInfo::Parse(ToRapidJson(iModelInstance[ServerSchema::Properties]), 
                                                iModelInstance[ServerSchema::InstanceId].asString(), nullptr, m_serverUrl);
            finalResult->SetSuccess(iModelInfo);
            return;
            }

        auto error = Error(createiModelResult.GetError());
        if (Error::Id::iModelAlreadyExists != error.GetId())
            {
            finalResult->SetError(error);
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, error.GetMessage().c_str());
            return;
            }

        bool initialized = error.GetExtendedData()[ServerSchema::Property::iModelInitialized].asBool();

        if (initialized)
            {
            finalResult->SetError(error);
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, error.GetMessage().c_str());
            return;
            }

        WSQuery iModelQuery(ServerSchema::Schema::Project, ServerSchema::Class::iModel);
        Utf8String filter;
        filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::iModelName, iModelName.c_str());
        iModelQuery.SetFilter(filter);
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Querying iModel by name %s.", iModelName.c_str());
        client->SendQueryRequest(iModelQuery, nullptr, nullptr, cancellationToken)->Then([=](WSObjectsResult const& queryResult)
            {
            if (!queryResult.IsSuccess())
                {
                finalResult->SetError(queryResult.GetError());
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, queryResult.GetError().GetMessage().c_str());
                return;
                }

            if (queryResult.GetValue().GetRapidJsonDocument().IsNull())
                {
                finalResult->SetError(error);
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, error.GetMessage().c_str());
                return;
                }

            finalResult->SetSuccess(iModelInfo::Parse(*queryResult.GetValue().GetInstances().begin(), m_serverUrl));
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Success.");
            });

        })->Then<iModelResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateNewiModel(Utf8StringCR projectId, Dgn::DgnDbCR db, Utf8StringCR iModelName, Utf8StringCR description, 
                                      bool waitForInitialized, Http::Request::ProgressCallbackCR callback, 
                                      ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::CreateNewiModel";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    if (!db.GetFileName().DoesPathExist())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::FileNotFound));// Fixed
        }
    if (m_serverUrl.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid server URL.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::InvalidServerURL));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::CredentialsNotSet));
        }
    if (iModelName.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid iModel name.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::InvalidiModelName));
        }

    FileInfoPtr fileInfo = FileInfo::Create(db, description);
    BeFileName filePath = db.GetFileName();

    std::shared_ptr<iModelResult> finalResult = std::make_shared<iModelResult>();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Creating iModel instance. Name: %s.", iModelName.c_str());
    return CreateiModelInstance(projectId, iModelName, description, cancellationToken)
        ->Then([=](iModelResultCR createiModelResult)
        {
        if (!createiModelResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, createiModelResult.GetError().GetMessage().c_str());
            finalResult->SetError(createiModelResult.GetError());
            return;
            }

        auto iModelInfo = createiModelResult.GetValue();
        finalResult->SetSuccess(iModelInfo);

        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Successfully created iModel instance. Instance ID: %s.", iModelInfo->GetId().c_str());
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to created iModel.");
        ConnectToiModel(*iModelInfo, cancellationToken)->Then([=](iModelConnectionResultCR connectionResult)
            {
            if (!connectionResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
                finalResult->SetError(connectionResult.GetError());
                return;
                }
            iModelConnectionPtr connection = connectionResult.GetValue();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Uploading new seed file.");
            connection->UploadNewSeedFile(filePath, *fileInfo, waitForInitialized, callback, cancellationToken)
                ->Then([=](FileResultCR fileUploadResult)
                {
                if (!fileUploadResult.IsSuccess())
                    {
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileUploadResult.GetError().GetMessage().c_str());
                    finalResult->SetError(fileUploadResult.GetError());
                    }
                });
            });

        })->Then<iModelResult>([=]
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateNewiModel(Utf8StringCR projectId, Dgn::DgnDbCR db, bool waitForInitialized, Http::Request::ProgressCallbackCR callback,
                                      ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::CreateNewiModel";
    if (!db.GetFileName().DoesPathExist())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::FileNotFound));
        }
    Utf8String name;
    db.QueryProperty(name, BeSQLite::PropertySpec(Db::Properties::Name, Db::Properties::ProjectNamespace));
    if (name.empty())
        BeStringUtilities::WCharToUtf8(name, db.GetFileName().GetFileNameWithoutExtension().c_str());
    Utf8String description;
    db.QueryProperty(description, BeSQLite::PropertySpec(Db::Properties::Description, Db::Properties::ProjectNamespace));
    return CreateNewiModel(projectId, db, name, description, waitForInitialized, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
BriefcaseTaskPtr Client::OpenBriefcase(Dgn::DgnDbPtr db, bool doSync, Http::Request::ProgressCallbackCR callback,
                                       ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::OpenBriefcase";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<BriefcaseResult>(BriefcaseResult::Error(Error::Id::FileNotFound));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<BriefcaseResult>(BriefcaseResult::Error(Error::Id::CredentialsNotSet));
        }
    auto readResult = iModelInfo::ReadiModelInfo(*db);
    BeBriefcaseId briefcaseId = db->GetBriefcaseId();
    if (!readResult.IsSuccess() || briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File is not a briefcase.");
        return CreateCompletedAsyncTask<BriefcaseResult>(BriefcaseResult::Error(Error::Id::FileIsNotBriefcase));
        }
    iModelInfoPtr iModelInfo = readResult.GetValue();
    if (iModelInfo->GetServerURL() != m_serverUrl)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Briefcase belongs to another server.");
        return CreateCompletedAsyncTask<BriefcaseResult>(BriefcaseResult::Error({Error::Id::InvalidServerURL, 
                                                                                ErrorLocalizedString(MESSAGE_BriefcaseWrongURL)}));
        }
    std::shared_ptr<BriefcaseResult> finalResult = std::make_shared<BriefcaseResult>();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to iModel %s.", iModelInfo->GetName().c_str());
    return ConnectToiModel(*iModelInfo, cancellationToken)->Then([=](iModelConnectionResultCR connectionResult)
        {
        if (!connectionResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
            finalResult->SetError(connectionResult.GetError());
            return;
            }

        FileInfoPtr fileInfo = FileInfo::Create(*db, "");
        auto connection = connectionResult.GetValue();
        connection->ValidateBriefcase(fileInfo->GetFileId(), briefcaseId, cancellationToken)
            ->Then([=](StatusResultCR validationResult)
            {
            if (!validationResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, validationResult.GetError().GetMessage().c_str());
                finalResult->SetError(validationResult.GetError());
                return;
                }
            BriefcasePtr briefcase = Briefcase::Create(db, connection);
            if (doSync)
                {
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Calling PullAndMerge for briefcase %d.", briefcase->GetBriefcaseId().GetValue());
                briefcase->PullAndMerge(callback, cancellationToken)->Then([=](const ChangeSetsResult& result)
                    {
                    if (result.IsSuccess())
                        {
                        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                        finalResult->SetSuccess(briefcase);
                        }
                    else
                        {
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                        finalResult->SetError(result.GetError());
                        }
                    });
                }
            else
                {
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                finalResult->SetSuccess(briefcase);
                }
            });
        })->Then<BriefcaseResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr Client::RecoverBriefcase(Dgn::DgnDbPtr db, Http::Request::ProgressCallbackCR callback,
                                       ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::RecoverBriefcase";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    if (!db.IsValid() || !db->GetFileName().DoesPathExist())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::FileNotFound));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::CredentialsNotSet));
        }
    auto readResult = iModelInfo::ReadiModelInfo(*db);
    BeBriefcaseId briefcaseId = db->GetBriefcaseId();
    if (!readResult.IsSuccess() || briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File is not a briefcase.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::FileIsNotBriefcase));
        }
    iModelInfoPtr iModelInfo = readResult.GetValue();
    BeFileName originalFilePath = db->GetFileName();

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to iModel %s.", iModelInfo->GetName().c_str());
    auto connectionResult = CreateiModelConnection(*iModelInfo);
    if (!connectionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(connectionResult.GetError()));
        }

    iModelConnectionPtr connection = connectionResult.GetValue();

    auto briefcaseResult = ExecuteAsync(connection->QueryBriefcaseInfo(briefcaseId, cancellationToken));
    if (!briefcaseResult->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, briefcaseResult->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(briefcaseResult->GetError()));
        }

    BeFileName downloadPath = originalFilePath.GetDirectoryName();
    downloadPath = downloadPath.AppendToPath(BeFileName(briefcaseResult->GetValue()->GetFileId().ToString()));
    downloadPath.AppendExtension(originalFilePath.GetExtension().c_str());

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Downloading briefcase with ID %d.", briefcaseId.GetValue());
    auto downloadResult = connection->DownloadBriefcaseFile(downloadPath, briefcaseId, nullptr, callback, cancellationToken);
#if defined (ENABLE_BIM_CRASH_TESTS)
    BreakHelper::HitBreakpoint(Breakpoints::Client_AfterDownloadBriefcaseFile);
#endif

    if (!downloadResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(downloadResult.GetError()));
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "Download successful.");

    db->CloseDb();

    BeFileName backupPath(originalFilePath.GetName());
    backupPath.AppendExtension(L"back");

    BeFileNameStatus status = BeFileName::BeMoveFile(originalFilePath, backupPath);

#if defined (ENABLE_BIM_CRASH_TESTS)
    try
        {
        BreakHelper::HitBreakpoint(Breakpoints::Client_AfterDeleteBriefcase);
        }
    catch (...)
        {
        BeFileName::BeMoveFile(backupPath, originalFilePath);
        throw;
        }
#endif 
    if (BeFileNameStatus::Success != status)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error()));
        }
    status = BeFileName::BeMoveFile(downloadPath, originalFilePath);
    if (BeFileNameStatus::Success != status)
        {
        BeFileName::BeMoveFile(backupPath, originalFilePath);
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error()));
        }

    backupPath.BeDeleteFile();
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Atiqa.Zafar            08/2017
//---------------------------------------------------------------------------------------
BriefcaseInfoTaskPtr Client::RestoreBriefcase(iModelInfoCR iModelInfo, BeSQLite::BeBriefcaseId briefcaseID, BeFileNameCR baseDirectory,
                                              bool doSync, BriefcaseFileNameCallback const& fileNameCallback,
                                              Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::RestoreBriefcase";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    CHECK_BRIEFCASEID(briefcaseID, BriefcaseInfoResult);

    if (iModelInfo.GetId().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid iModel id.");
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(Error::Id::InvalidiModelId));
        }

    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(Error::Id::CredentialsNotSet));
        }

    //get iModelConnection
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to iModel %s.", iModelInfo.GetName().c_str());
    auto connectionResult = CreateiModelConnection(iModelInfo);
    if (!connectionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(connectionResult.GetError()));
        }

    iModelConnectionPtr connection = connectionResult.GetValue();

    //get BriefcaseInfo
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting BriefcaseInfo of Briefcase ID %d.", briefcaseID);
    BriefcaseInfoResultPtr briefcaseInfoResult = ExecuteAsync(connection->QueryBriefcaseInfo(briefcaseID, nullptr));
    if (!briefcaseInfoResult->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, briefcaseInfoResult->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(briefcaseInfoResult->GetError()));
        }
    BriefcaseInfoPtr briefcaseInfo = briefcaseInfoResult->GetValue();
    BeFileName filePath = fileNameCallback(baseDirectory, connection->GetiModelInfo(), *briefcaseInfo);
    if (filePath.DoesPathExist())
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "File already exists.");
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(Error::Id::FileAlreadyExists));
        }
    if (!filePath.GetDirectoryName().DoesPathExist())
        {
        BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
        }

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Downloading briefcase with ID %d.", briefcaseID);
    StatusResult downloadResult = DownloadBriefcase(connection, filePath, *briefcaseInfo, doSync, callback, cancellationToken);
    if (!downloadResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(downloadResult.GetError()));
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "Download successful.");

    briefcaseInfo->SetLocalPath(filePath);
    return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Success(briefcaseInfo));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikolinuas            07/2017
//---------------------------------------------------------------------------------------
DgnDbPtr Client::OpenWithSchemaUpgradeInternal(BeSQLite::DbResult* status, BeFileName filePath, ChangeSets changeSets, 
                                               SchemaUpgradeOptions::DomainUpgradeOptions domainUpgradeOptions, 
                                               RevisionProcessOption processOption)
    {
    bvector<DgnRevisionCP> changeSetsToMerge;
    ConvertToChangeSetPointersVector(changeSets, changeSetsToMerge);

    if (RevisionProcessOption::Reverse == processOption)
        std::reverse(changeSetsToMerge.begin(), changeSetsToMerge.end());

    auto upgradeOptions = SchemaUpgradeOptions(domainUpgradeOptions);
    upgradeOptions.SetUpgradeFromRevisions(changeSetsToMerge, processOption);
    return Dgn::DgnDb::OpenDgnDb(status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, upgradeOptions));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikolinuas            07/2017
//---------------------------------------------------------------------------------------
DgnDbPtr Client::OpenWithSchemaUpgrade(BeSQLite::DbResult* status, BeFileName filePath, ChangeSets changeSets, RevisionProcessOption processOption)
    {
    return OpenWithSchemaUpgradeInternal(status, filePath, changeSets, SchemaUpgradeOptions::DomainUpgradeOptions::ValidateOnly, processOption);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
StatusResult Client::DownloadBriefcase(iModelConnectionPtr connection, BeFileName filePath, BriefcaseInfoCR briefcaseInfo,
                                       bool doSync, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::DownloadBriefcase";
    if (!doSync)
        return connection->DownloadBriefcaseFile(filePath, briefcaseInfo.GetId(), briefcaseInfo.GetFileAccessKey(), callback, cancellationToken);

    MultiProgressCallbackHandler handler(callback);
    Http::Request::ProgressCallback briefcaseCallback, changeSetsCallback, changeSetsFallbackCallback;
    handler.AddCallback(briefcaseCallback, 70.0f);
    handler.AddCallback(changeSetsCallback, 20.0f);
    handler.AddCallback(changeSetsFallbackCallback, 10.0f);

    Utf8String mergedChangeSetId = briefcaseInfo.GetMergedChangeSetId();
    ChangeSetsTaskPtr pullChangeSetsTask = connection->DownloadChangeSetsAfterId(mergedChangeSetId, briefcaseInfo.GetFileId(), changeSetsCallback, cancellationToken);

    StatusResult briefcaseResult = connection->DownloadBriefcaseFile(filePath, briefcaseInfo.GetId(), briefcaseInfo.GetFileAccessKey(), briefcaseCallback, cancellationToken);
    if (!briefcaseResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, briefcaseResult.GetError().GetMessage().c_str());
        return briefcaseResult;
        }

    BeSQLite::DbResult status;
    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipUpgrade)));
    if (BeSQLite::DbResult::BE_SQLITE_OK != status)
        {
        StatusResult result = StatusResult::Error(Error(db, status));
        if (!result.IsSuccess())
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
        return result;
        }

    ChangeSetsResult pullChangeSetsResult = pullChangeSetsTask->GetResult();
    if (!pullChangeSetsResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, pullChangeSetsResult.GetError().GetMessage().c_str());
        return StatusResult::Error(pullChangeSetsResult.GetError());
        }

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Briefcase file and changeSets after changeSet %s downloaded successfully.", mergedChangeSetId.c_str());

    // If MergedChangeSetId and ParentChangeSetId do not match, query new changeSets
    Utf8String parentRevisionId = db->Revisions().GetParentRevisionId();
    if (!parentRevisionId.Equals(mergedChangeSetId))
        {
        // Override task and result so that previously downloaded changeSets will be removed. Otherwise they are overridden later and required changeSets are deleted.
        pullChangeSetsTask = CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Success(ChangeSets()));
        pullChangeSetsResult = pullChangeSetsTask->GetResult();

        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "MergedChangeSetId '%s' and ParentChangeSetId '%s' do not match.", mergedChangeSetId.c_str(), parentRevisionId.c_str());
        pullChangeSetsTask = connection->DownloadChangeSetsAfterId(parentRevisionId, briefcaseInfo.GetFileId(), changeSetsFallbackCallback, cancellationToken);
        pullChangeSetsResult = pullChangeSetsTask->GetResult();
        if (!pullChangeSetsResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, pullChangeSetsResult.GetError().GetMessage().c_str());
            return StatusResult::Error(pullChangeSetsResult.GetError());
            }
        }

    db->Txns().EnableTracking(true);
#if defined (ENABLE_BIM_CRASH_TESTS)
    BreakHelper::HitBreakpoint(Breakpoints::Client_AfterOpenBriefcaseForMerge);
#endif
    ChangeSets changeSets = pullChangeSetsResult.GetValue();
    handler.SetFinished();

    return MergeChangeSetsIntoDgnDb(db, changeSets, filePath);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
StatusResult Client::MergeChangeSetsIntoDgnDb(Dgn::DgnDbPtr db, const ChangeSets changeSets, BeFileNameCR filePath, 
                                              ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "Client::MergeChangeSetsIntoDgnDb";

    RevisionStatus mergeStatus = RevisionStatus::Success;
    if (ContainsSchemaChanges(changeSets, *db))
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Merging changeSets with DgnDb reopen.");
        db->CloseDb();

        BeSQLite::DbResult status;
        db = OpenWithSchemaUpgradeInternal(&status, filePath, changeSets, SchemaUpgradeOptions::DomainUpgradeOptions::SkipUpgrade);
        if (BeSQLite::DbResult::BE_SQLITE_OK != status)
            {
            StatusResult result = StatusResult::Error(Error(db, status));
            if (!result.IsSuccess())
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return result;
            }
        }
    else
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Merging changeSets.");
        if (!changeSets.empty())
            {
            for (auto changeSet : changeSets)
                {
                mergeStatus = db->Revisions().MergeRevision(*changeSet);
                if (mergeStatus != RevisionStatus::Success)
                    break; // TODO: Use the information on the changeSet that actually failed. 
                }
            }
        }

#if defined (ENABLE_BIM_CRASH_TESTS)
    BreakHelper::HitBreakpoint(Breakpoints::Client_AfterMergeChangeSets);
#endif
    db->CloseDb();

    if (RevisionStatus::Success == mergeStatus)
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Success.");
        return StatusResult::Success();
        }

    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
    return StatusResult::Error(mergeStatus);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoTaskPtr Client::AcquireBriefcaseToDir(iModelInfoCR iModelInfo, BeFileNameCR baseDirectory, bool doSync, 
                                                   BriefcaseFileNameCallback const& fileNameCallback, Http::Request::ProgressCallbackCR callback, 
                                                   ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::AcquireBriefcaseToDir";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    if (iModelInfo.GetId().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid iModel id.");
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(Error::Id::InvalidiModelId));
        }
    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(Error::Id::CredentialsNotSet));
        }
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to iModel %s.", iModelInfo.GetName().c_str());

    auto connectionResult = CreateiModelConnection(iModelInfo);
    if (!connectionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(connectionResult.GetError()));
        }

    iModelConnectionPtr connection = connectionResult.GetValue();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Acquiring briefcase ID.");
    auto briefcaseResult = ExecuteAsync(connection->CreateBriefcaseInstance(cancellationToken));
#if defined (ENABLE_BIM_CRASH_TESTS)
    BreakHelper::HitBreakpoint(Breakpoints::Client_AfterCreateBriefcaseInstance);
#endif
    if (!briefcaseResult->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, briefcaseResult->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(briefcaseResult->GetError()));
        }

    Json::Value json;
    briefcaseResult->GetValue().GetJson(json);
    JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
    BriefcaseInfoPtr briefcaseInfo = BriefcaseInfo::ParseRapidJson(ToRapidJson(instance[ServerSchema::Properties]));

    FileAccessKeyPtr fileAccessKey = FileAccessKey::ParseFromRelated(instance);
    briefcaseInfo->SetFileAccessKey(fileAccessKey);

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Acquired briefcase ID %d.", briefcaseInfo->GetId());

    BeFileName filePath = fileNameCallback(baseDirectory, iModelInfo, *briefcaseInfo);
    if (filePath.DoesPathExist())
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "File already exists.");
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(Error::Id::FileAlreadyExists));
        }
    if (!filePath.GetDirectoryName().DoesPathExist())
        {
        BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
        }

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Downloading briefcase with ID %d.", briefcaseInfo->GetId());
    StatusResult downloadResult = DownloadBriefcase(connection, filePath, *briefcaseInfo, doSync, callback, cancellationToken);
    if (!downloadResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(downloadResult.GetError()));
        }

    briefcaseInfo->SetLocalPath(filePath);
    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "Download successful.");
    return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Success(briefcaseInfo));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
BriefcaseInfoTaskPtr Client::AcquireBriefcase(iModelInfoCR iModelInfo, BeFileNameCR localFileName, bool doSync,
                                              Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::AcquireBriefcase";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (localFileName.DoesPathExist() && !localFileName.IsDirectory())
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "File already exists.");
        return CreateCompletedAsyncTask<BriefcaseInfoResult>(BriefcaseInfoResult::Error(Error::Id::FileAlreadyExists));
        }
    return AcquireBriefcaseToDir(iModelInfo, localFileName, doSync, [=](BeFileName baseDirectory, iModelInfoCR iModelInfo, BriefcaseInfoCR briefcaseInfo)
        {
        if (baseDirectory.IsDirectory())
            baseDirectory.AppendToPath(BeFileName(briefcaseInfo.GetFileName()));
        return baseDirectory;
        }, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr Client::AbandonBriefcase(iModelInfoCR iModelInfo, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::AbandonBriefcase";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    CHECK_BRIEFCASEID(briefcaseId, StatusResult);

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, iModelInfo.GetWSRepositoryName(), m_clientInfo, nullptr, 
                                                               m_customHandler);
    client->SetCredentials(m_credentials);

    Utf8String briefcaseIdString;
    briefcaseIdString.Sprintf("%u", briefcaseId);
    ObjectId iModelId = ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase, briefcaseIdString);

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Sending abandon briefcase request. iModel ID: %s.", iModelInfo.GetId().c_str());
    return client->SendDeleteObjectRequest(iModelId, cancellationToken)->Then<StatusResult>([=](WSDeleteObjectResult const& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, "Success.");
            return StatusResult::Success();
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr Client::DeleteiModel(Utf8StringCR projectId, iModelInfoCR iModelInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::DeleteiModel";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    IWSRepositoryClientPtr client = CreateProjectConnection(projectId);
    ObjectId iModelId = ObjectId(ServerSchema::Schema::Project, ServerSchema::Class::iModel, iModelInfo.GetId());
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Sending delete iModel request. iModel ID: %s.", iModelInfo.GetId().c_str());
    return client->SendDeleteObjectRequest(iModelId, cancellationToken)->Then<StatusResult>([=](WSDeleteObjectResult const& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "Success.");
            return StatusResult::Success();
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2016
//---------------------------------------------------------------------------------------
iModelManagerTaskPtr Client::CreateiModelManager(iModelInfoCR iModelInfo, FileInfoCR fileInfo, BriefcaseInfoCR briefcaseInfo, 
                                                 ICancellationTokenPtr cancellationToken)
    {
    iModelManagerResultPtr finalResult = std::make_shared<iModelManagerResult>();
    return ConnectToiModel(iModelInfo, cancellationToken)->Then([=](iModelConnectionResultCR connectionResult)
        {
        if (!connectionResult.IsSuccess())
            {
            finalResult->SetError(connectionResult.GetError());
            return;
            }
        auto connection = connectionResult.GetValue();
        connection->ValidateBriefcase(fileInfo.GetFileId(), briefcaseInfo.GetId(), cancellationToken)->Then([=](StatusResultCR result)
            {
            if (!result.IsSuccess())
                {
                finalResult->SetError(result.GetError());
                }
            else
                {
                finalResult->SetSuccess(iModelManager::Create(connection));
                }
            });
        })->Then<iModelManagerResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
BeFileNameTaskPtr Client::DownloadStandaloneBriefcaseUpdatedToVersion
(
iModelInfoCR iModelInfo, Utf8String versionId, 
LocalBriefcaseFileNameCallback const & fileNameCallBack,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "Client::DownloadStandaloneBriefcaseUpdatedToVersion";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    //double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    if (iModelInfo.GetId().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid iModel id.");
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(Error::Id::InvalidiModelId));
        }
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to iModel %s.", iModelInfo.GetName().c_str());

    auto connectionResult = CreateiModelConnection(iModelInfo);
    if (!connectionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(connectionResult.GetError()));
        }

    iModelConnectionPtr connection = connectionResult.GetValue();

    auto seedFileInfoResult = ExecuteAsync(connection->GetLatestSeedFile(cancellationToken));
    if (!seedFileInfoResult->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, seedFileInfoResult->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(seedFileInfoResult->GetError()));
        }

    auto versionManager = connection->GetVersionsManager();
    auto result = ExecuteAsync(versionManager.GetVersionChangeSets(versionId, seedFileInfoResult->GetValue()->GetFileId(), cancellationToken));
    if (!result->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(result->GetError()));
        }
    if (result->GetValue().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid version id.");
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(Error::Id::InvalidVersion));
        }

    return DownloadStandaloneBriefcaseInternal(connection, iModelInfo, *(seedFileInfoResult->GetValue()), result->GetValue(), fileNameCallBack, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
BeFileNameTaskPtr Client::DownloadStandaloneBriefcaseUpdatedToChangeSet
(
iModelInfoCR iModelInfo, 
Utf8String changeSetId, 
LocalBriefcaseFileNameCallback const & fileNameCallback, 
Http::Request::ProgressCallbackCR callback, 
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "Client::DownloadStandaloneBriefcaseUpdatedToChangeSet";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    //double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    if (iModelInfo.GetId().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid iModel id.");
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(Error::Id::InvalidiModelId));
        }
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to iModel %s.", iModelInfo.GetName().c_str());

    auto connectionResult = CreateiModelConnection(iModelInfo);
    if (!connectionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(connectionResult.GetError()));
        }

    iModelConnectionPtr connection = connectionResult.GetValue();

    auto seedFileInfoResult = ExecuteAsync(connection->GetLatestSeedFile(cancellationToken));
    if (!seedFileInfoResult->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, seedFileInfoResult->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(seedFileInfoResult->GetError()));
        }

    auto result = ExecuteAsync(connection->GetChangeSetById(changeSetId, cancellationToken));
    if (!result->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result->GetError().GetMessage().c_str());
        ChangeSetsInfoResult::Error(result->GetError());
        }

    uint64_t changeSetIndex = result->GetValue()->GetIndex();

    Utf8String filter;
    filter.Sprintf("%s+le+%I64d", ServerSchema::Property::Index, changeSetIndex);

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    query.SetFilter(filter);

    auto changeSetsResult = ExecuteAsync(connection->ChangeSetsFromQueryInternal(query, false, cancellationToken));

    return DownloadStandaloneBriefcaseInternal(connection, iModelInfo, *(seedFileInfoResult->GetValue()), changeSetsResult->GetValue(), fileNameCallback, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
BeFileNameTaskPtr Client::DownloadStandaloneBriefcase(iModelInfoCR iModelInfo, LocalBriefcaseFileNameCallback const & fileNameCallback, 
                                                      Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::DownloadStandaloneBriefcase";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    //double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    if (iModelInfo.GetId().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid iModel id.");
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(Error::Id::InvalidiModelId));
        }
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Connecting to iModel %s.", iModelInfo.GetName().c_str());

    auto connectionResult = CreateiModelConnection(iModelInfo);
    if (!connectionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(connectionResult.GetError()));
        }

    iModelConnectionPtr connection = connectionResult.GetValue();

    auto seedFileInfoResult = ExecuteAsync(connection->GetLatestSeedFile(cancellationToken));
    if (!seedFileInfoResult->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, seedFileInfoResult->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(seedFileInfoResult->GetError()));
        }

    auto result = ExecuteAsync(connection->GetAllChangeSets());
    if (!result->IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(result->GetError()));
        }

    return DownloadStandaloneBriefcaseInternal(connection, iModelInfo, *(seedFileInfoResult->GetValue()), result->GetValue(), fileNameCallback, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
BeFileNameTaskPtr Client::DownloadStandaloneBriefcaseInternal
(
iModelConnectionPtr connection, 
iModelInfoCR iModelInfo, 
FileInfoCR fileInfo, 
bvector<ChangeSetInfoPtr> changeSetsToMerge, 
LocalBriefcaseFileNameCallback const & fileNameCallback, 
Http::Request::ProgressCallback callback, 
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "Client::DownloadStandaloneBriefcaseInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    //double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    BeFileName filePath = fileNameCallback(iModelInfo, fileInfo);
    if (filePath.DoesPathExist())
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "File already exists.");
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(Error::Id::FileAlreadyExists));
        }
    if (!filePath.GetDirectoryName().DoesPathExist())
        {
        BeFileName::CreateNewDirectory(filePath.GetDirectoryName());
        }

    MultiProgressCallbackHandler handler(callback);
    Http::Request::ProgressCallback changeSetsCallback, seedFileCallback;
    handler.AddCallback(changeSetsCallback, 20.0f);
    handler.AddCallback(seedFileCallback, 80.0f);

    auto SeedFileResult = connection->DownloadSeedFile(filePath, fileInfo.GetFileId().ToString(), seedFileCallback, cancellationToken)->GetResult();
    if (!SeedFileResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, SeedFileResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(SeedFileResult.GetError()));
        }

    auto briefcaseId = BeBriefcaseId(BeBriefcaseId::Standalone());
    auto result = connection->WriteBriefcaseIdIntoFile(filePath, briefcaseId);
    if (!result.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(result.GetError()));
        }

    auto changeSetsResult = connection->DownloadChangeSetsInternal(changeSetsToMerge, changeSetsCallback, cancellationToken)->GetResult();
    if (!changeSetsResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetsResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(changeSetsResult.GetError()));
        }

    BeSQLite::DbResult status;
    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, 
                                                                                       SchemaUpgradeOptions::DomainUpgradeOptions::SkipUpgrade));
    if (BeSQLite::DbResult::BE_SQLITE_OK != status)
        {
        StatusResult result = StatusResult::Error(Error(db, status));
        if (!result.IsSuccess())
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(result));
        }

    auto margeStatus = MergeChangeSetsIntoDgnDb(db, changeSetsResult.GetValue(), filePath);
    if (!margeStatus.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, margeStatus.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(margeStatus.GetError()));
        }

    handler.SetFinished();
    BeFileName::SetFileReadOnly(filePath, true);

    return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Success(filePath));
    }


