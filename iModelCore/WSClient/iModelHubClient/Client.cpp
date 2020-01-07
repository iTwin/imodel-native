/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
IWSRepositoryClientPtr Client::CreateContextConnection(Utf8StringCR contextId) const
    {
    Utf8String context;
    context.Sprintf("%s--%s", ServerSchema::Plugin::Context, contextId.c_str());
    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, ServerProperties::ServiceVersion(), context, m_clientInfo, nullptr, m_customHandler);
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
iModelConnectionTaskPtr Client::ConnectToiModel(Utf8StringCR contextId, Utf8StringCR iModelId, ICancellationTokenPtr cancellationToken) const
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

    return GetiModelById(contextId, iModelId, cancellationToken)
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
ClientPtr Client::Create(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler, Utf8CP url, IAzureBlobStorageClientFactory storageClientFactory)
    {
    const Utf8String methodName = "Client::Create";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!Utf8String::IsNullOrEmpty(url))
        return new Client(clientInfo, customHandler, url, storageClientFactory);
    Utf8String resolvedUrl = UrlProvider::Urls::iModelHubApi.Get();
    return new Client(clientInfo, customHandler, resolvedUrl.c_str(), storageClientFactory);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
iModelsTaskPtr Client::GetiModels(Utf8StringCR contextId, ICancellationTokenPtr cancellationToken, bool filterInitialized) const
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

    WSQuery query = WSQuery(ServerSchema::Schema::Context, ServerSchema::Class::iModel);

    //Always select HasCreatorInfo relationship
    Utf8String select = "*";
    iModelInfo::AddHasCreatorInfoSelect(select);
    query.SetSelect(select);
    if (filterInitialized)
        {
        query.SetFilter("Initialized+eq+true");
        }

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    IWSRepositoryClientPtr client = CreateContextConnection(contextId);
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, requestOptions, "Getting iModels from context %s.", contextId.c_str());

    return client->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)->Then<iModelsResult>([=] (WSObjectsResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return iModelsResult::Error(result.GetError());
            }
        bvector<iModelInfoPtr> iModels;
        for (const auto& iModel : result.GetValue().GetInstances())
            {
            iModels.push_back(iModelInfo::Parse(iModel, m_serverUrl));
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "Success.");
        return iModelsResult::Success(iModels);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas         10/2018
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::GetiModelInternal(Utf8StringCR contextId, WSQuery query, Utf8String methodName, ICancellationTokenPtr cancellationToken) const
    {
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

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    IWSRepositoryClientPtr client = CreateContextConnection(contextId);

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, requestOptions, "Getting iModels from context %s.", contextId.c_str());
    return client->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)->Then<iModelResult>([=] (WSObjectsResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return iModelResult::Error(result.GetError());
            }
        auto iModelInfoInstances = result.GetValue().GetInstances();
        if (iModelInfoInstances.Size() == 0)
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, "iModel does not exist.");
            return iModelResult::Error(Error::Id::iModelDoesNotExist);
            }
        iModelInfoPtr iModelInfo = iModelInfo::Parse(*iModelInfoInstances.begin(), m_serverUrl);
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, requestOptions, "");
        return iModelResult::Success(iModelInfo);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::GetiModelByName(Utf8StringCR contextId, Utf8StringCR iModelName, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::GetiModelByName";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting iModel with name %s.", iModelName.c_str());

    WSQuery query = WSQuery(ServerSchema::Schema::Context, ServerSchema::Class::iModel);
    Utf8String filter;
    Utf8String updatedName = BeUri::EscapeString(iModelName);
    filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::iModelName, updatedName.c_str());
    query.SetFilter(filter);

    //Always select HasCreatorInfo relationship
    Utf8String select = "*";
    iModelInfo::AddHasCreatorInfoSelect(select);
    query.SetSelect(select);

    return GetiModelInternal(contextId, query, methodName, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::GetiModelById(Utf8StringCR contextId, Utf8StringCR iModelId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::GetiModelById";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting iModel with id %s.", iModelId.c_str());

    WSQuery query = WSQuery(ServerSchema::Schema::Context, ServerSchema::Class::iModel);
    Utf8String filter;
    filter.Sprintf("$id+eq+'%s'", iModelId.c_str());
    query.SetFilter(filter);

    //Always select HasCreatorInfo relationship
    Utf8String select = "*";
    iModelInfo::AddHasCreatorInfoSelect(select);
    query.SetSelect(select);
    
    return GetiModelInternal(contextId, query, methodName, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            10/2018
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::GetiModel(Utf8StringCR contextId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::GetiModel";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting primary iModel for context %s.", contextId.c_str());

    WSQuery query = WSQuery(ServerSchema::Schema::Context, ServerSchema::Class::iModel);
    Utf8String orderBy;
    orderBy.Sprintf("%s+%s", ServerSchema::Property::CreatedDate, "asc");
    query.SetOrderBy(orderBy);
    query.SetTop(1);

    //Always select HasCreatorInfo relationship
    Utf8String select = "*";
    iModelInfo::AddHasCreatorInfoSelect(select);
    query.SetSelect(select);

    return GetiModelInternal(contextId, query, methodName, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  04/2018
//---------------------------------------------------------------------------------------
ThumbnailImageTaskPtr Client::GetiModelThumbnail
(
Utf8StringCR contextId, 
Utf8StringCR imodelId, 
Thumbnail::Size size, 
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "ThumbnailsManager::GetiModelThumbnail";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    ObjectId thumbnailObjectId(ServerSchema::Schema::Context, Thumbnail::GetClassName(size), imodelId);

    return ExecuteWithRetry<Render::Image>([=]()
        {
        IWSRepositoryClientPtr client = CreateContextConnection(contextId);
        HttpByteStreamBodyPtr responseBody = HttpByteStreamBody::Create();
        return client->SendGetFileRequestWithOptions(thumbnailObjectId, responseBody, nullptr, nullptr, requestOptions, cancellationToken)
            ->Then<ThumbnailImageResult>([=](const WSResult& streamResult)
            {
            if (!streamResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, streamResult.GetError().GetMessage().c_str());
                return ThumbnailImageResult::Error(streamResult.GetError());
                }

            ByteStream byteStream = responseBody->GetByteStream();
            Render::Image image = Render::Image::FromPng(byteStream.GetData(), byteStream.GetSize());

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");
            return ThumbnailImageResult::Success(image);
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
BEGIN_UNNAMED_NAMESPACE
Json::Value iModelCreationJson(iModelBaseInfoCR imodelCreateInfo)
    {
    Json::Value iModelCreation(Json::objectValue);
    JsonValueR instance = iModelCreation[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Context;
    instance[ServerSchema::ClassName] = ServerSchema::Class::iModel;
    JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
    properties[ServerSchema::Property::iModelName] = imodelCreateInfo.GetName();
    properties[ServerSchema::Property::iModelDescription] = imodelCreateInfo.GetDescription();
    properties[ServerSchema::Property::iModelTemplate] = imodelCreateInfo.GetTemplate();

    if (!imodelCreateInfo.GetExtent().empty())
        {
        properties[ServerSchema::Property::Extent] = Json::arrayValue;
        int i = 0;
        for (auto const& coordinate : imodelCreateInfo.GetExtent())
            {
            properties[ServerSchema::Property::Extent][i++] = coordinate;
            }
        }

    return iModelCreation;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Vilius.Kazlauskas             01/2019
//---------------------------------------------------------------------------------------
Utf8CP GetiModelValidationLogMessage(Error::Id errorId)
    {
    switch (errorId)
        {
        case Error::Id::InvalidiModelName:
            return "Invalid iModel name.";
        case Error::Id::InvalidiModelExtentCount:
            return "Invalid iModel extent coordinates count.";
        case Error::Id::InvalidiModelExtentCoordinate:
            return "Invalid iModel extent coordinate value out of bounds.";
        default:
            return Utf8PrintfString("Unexpected error while validating an iModel. ErrorId: %d", static_cast<int>(errorId)).c_str();
        }
    }
END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateiModelInstance(Utf8StringCR contextId, iModelCreateInfoPtr imodelCreateInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::CreateiModelInstance";
    std::shared_ptr<iModelResult> finalResult = std::make_shared<iModelResult>();

    Json::Value imodelCreationJson = iModelCreationJson(*imodelCreateInfo);
    m_globalRequestOptionsPtr->InsertRequestOptions(imodelCreationJson);

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    IWSRepositoryClientPtr client = CreateContextConnection(contextId);
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, requestOptions, "Sending create iModel request for context %s.", contextId.c_str());
    return client->SendCreateObjectRequestWithOptions(imodelCreationJson, BeFileName(), nullptr, requestOptions, cancellationToken)
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
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, error.GetMessage().c_str());
            return;
            }

        bool initialized = error.GetExtendedData()[ServerSchema::Property::iModelInitialized].asBool();

        if (initialized)
            {
            finalResult->SetError(error);
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, error.GetMessage().c_str());
            return;
            }

        GetiModelByName(contextId, imodelCreateInfo->GetName())->Then([=] (iModelResultCR queryResult)
            {
            queryResult.IsSuccess() ? finalResult->SetSuccess(queryResult.GetValue()) : finalResult->SetError(queryResult.GetError());
            });

        })->Then<iModelResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateNewiModel(Utf8StringCR contextId, Dgn::DgnDbCR db, iModelCreateInfoPtr imodelCreateInfo,
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
    if (!db.IsStandaloneBriefcase() && !db.IsMasterCopy())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Seed file is a briefcase.");
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(Error::Id::FileIsBriefcase));
        }

    StatusResult imodelValidationResult = imodelCreateInfo->Validate();
    if (!imodelValidationResult.IsSuccess())
        {
        Error::Id errorId = imodelValidationResult.GetError().GetId();
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, GetiModelValidationLogMessage(errorId));
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(errorId));
        }

    FileInfoPtr fileInfo = FileInfo::Create(db, imodelCreateInfo->GetDescription());
    BeFileName filePath = db.GetFileName();

    std::shared_ptr<iModelResult> finalResult = std::make_shared<iModelResult>();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Creating iModel instance. Name: %s.", imodelCreateInfo->GetName().c_str());
    return CreateiModelInstance(contextId, imodelCreateInfo, cancellationToken)
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
//@bsimethod                                     Vilius.Kazlauskas             08/2019
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateNewiModel(Utf8StringCR contextId, Dgn::DgnDbCR db, Utf8StringCR iModelName, Utf8StringCR description,
                                      bool waitForInitialized, Http::Request::ProgressCallbackCR callback,
                                      ICancellationTokenPtr cancellationToken) const
    {
    iModelCreateInfoPtr imodelCreateInfo = iModelCreateInfo::Create(iModelName, description);
    return CreateNewiModel(contextId, db, imodelCreateInfo, waitForInitialized, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateNewiModel(Utf8StringCR contextId, Dgn::DgnDbCR db, bool waitForInitialized, Http::Request::ProgressCallbackCR callback,
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

    iModelCreateInfoPtr imodelCreateInfo = iModelCreateInfo::Create(name, description);
    return CreateNewiModel(contextId, db, imodelCreateInfo, waitForInitialized, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2019
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateEmptyiModel(Utf8StringCR contextId, iModelCreateInfoPtr imodelCreateInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::CreateEmptyiModel";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
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

    StatusResult imodelValidationResult = imodelCreateInfo->Validate();
    if (!imodelValidationResult.IsSuccess())
        {
        Error::Id errorId = imodelValidationResult.GetError().GetId();
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, GetiModelValidationLogMessage(errorId));
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(errorId));
        }

    imodelCreateInfo->SetTemplate(ServerSchema::iModelTemplateEmpty);

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Creating empty iModel instance. Name: %s.", imodelCreateInfo->GetName().c_str());
    return CreateiModelInstance(contextId, imodelCreateInfo, cancellationToken)
        ->Then<iModelResult>([=] (iModelResultCR createiModelResult)
        {
        if (!createiModelResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, createiModelResult.GetError().GetMessage().c_str());
            return iModelResult::Error(createiModelResult.GetError());
            }

        auto iModelInfo = createiModelResult.GetValue();
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "");

        return iModelResult::Success(iModelInfo);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Vilius.Kazlauskas             01/2019
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CreateEmptyiModel(Utf8StringCR contextId, Utf8StringCR iModelName, Utf8StringCR description,
                                        ICancellationTokenPtr cancellationToken) const
    {
    iModelCreateInfoPtr imodelCreateInfo = iModelCreateInfo::Create(iModelName, description);
    return CreateEmptyiModel(contextId, imodelCreateInfo, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2019
//---------------------------------------------------------------------------------------
iModelTaskPtr Client::CloneiModel(Utf8StringCR contextId, Utf8StringCR sourceiModelId, Utf8StringCR sourceChangeSetId, 
    iModelCreateInfoPtr imodelCreateInfo, bool waitForInitialized, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::CloneiModel";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
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

    StatusResult imodelValidationResult = imodelCreateInfo->Validate();
    if (!imodelValidationResult.IsSuccess())
        {
        Error::Id errorId = imodelValidationResult.GetError().GetId();
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, GetiModelValidationLogMessage(errorId));
        return CreateCompletedAsyncTask<iModelResult>(iModelResult::Error(errorId));
        }

    imodelCreateInfo->SetTemplate(Utf8PrintfString("%s:%s", sourceiModelId.c_str(), sourceChangeSetId.c_str()));

    std::shared_ptr<iModelResult> finalResult = std::make_shared<iModelResult>();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Cloning iModel. Source iModel Id: %s.", sourceiModelId.c_str());
    return CreateiModelInstance(contextId, imodelCreateInfo, cancellationToken)
        ->Then([=](iModelResultCR createiModelResult)
        {
        if (!createiModelResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, createiModelResult.GetError().GetMessage().c_str());
            finalResult->SetError(createiModelResult.GetError());
            return;
            }

        
        iModelInfoPtr imodelInfo = createiModelResult.GetValue();
        finalResult->SetSuccess(imodelInfo);

        if (!waitForInitialized)
            return;

        ConnectToiModel(*imodelInfo, cancellationToken)->Then([=](iModelConnectionResultCR connectionResult)
            {
            if (!connectionResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, connectionResult.GetError().GetMessage().c_str());
                finalResult->SetError(connectionResult.GetError());
                return;
                }

            iModelConnectionPtr connection = connectionResult.GetValue();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Waiting for iModel initialization.");
            connection->WaitForInitialization(cancellationToken)
                ->Then([=](StatusResultCR statusResult)
                    {
                    if (!statusResult.IsSuccess())
                        {
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, statusResult.GetError().GetMessage().c_str());
                        finalResult->SetError(statusResult.GetError());
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
DgnDbPtr OpenWithSchemaUpgradeInternal(BeSQLite::DbResult* status, BeFileName filePath, ChangeSets changeSets, 
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
    return OpenWithSchemaUpgradeInternal(status, filePath, changeSets, SchemaUpgradeOptions::DomainUpgradeOptions::CheckRequiredUpgrades, processOption);
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

    Utf8String mergedChangeSetId = briefcaseInfo.GetMergedChangeSetId();
    auto changeSetsResult = ExecuteAsync(connection->GetChangeSetsAfterId(mergedChangeSetId, briefcaseInfo.GetFileId(), cancellationToken));


    uint64_t changeSetsSize = 0;
    for (auto changeSet : changeSetsResult->GetValue())
        {
        changeSetsSize += changeSet->GetFileSize();
        }

    uint64_t totalSize = 2 * changeSetsSize + briefcaseInfo.GetSize();

    MultiProgressCallbackHandler handler(callback, totalSize);
    Http::Request::ProgressCallback briefcaseCallback, changeSetsCallback, mergeCallback;
    handler.AddCallback(briefcaseCallback);
    handler.AddCallback(changeSetsCallback);
    handler.AddCallback(mergeCallback);


    auto changeSetsDownloadTask = connection->DownloadChangeSetsInternal(changeSetsResult->GetValue(), changeSetsCallback, cancellationToken);
    StatusResult briefcaseResult = connection->DownloadBriefcaseFile(filePath, briefcaseInfo.GetId(), briefcaseInfo.GetFileAccessKey(), briefcaseCallback, cancellationToken);
    if (!briefcaseResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, briefcaseResult.GetError().GetMessage().c_str());
        return briefcaseResult;
        }

    BeSQLite::DbResult status;
    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck)));
    if (BeSQLite::DbResult::BE_SQLITE_OK != status)
        {
        StatusResult result = StatusResult::Error(Error(db, status));
        if (!result.IsSuccess())
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
        return result;
        }

    ChangeSetsResult changeSetsDownloadResult = changeSetsDownloadTask->GetResult();
    if (!changeSetsDownloadResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetsDownloadResult.GetError().GetMessage().c_str());
        return StatusResult::Error(changeSetsDownloadResult.GetError());
        }

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Briefcase file and changeSets after changeSet %s downloaded successfully.", mergedChangeSetId.c_str());


    db->Txns().EnableTracking(true);
#if defined (ENABLE_BIM_CRASH_TESTS)
    BreakHelper::HitBreakpoint(Breakpoints::Client_AfterOpenBriefcaseForMerge);
#endif
    ChangeSets changeSets = changeSetsDownloadResult.GetValue();

    StatusResult mergeResult = MergeChangeSetsIntoDgnDb(db, changeSets, filePath, mergeCallback, cancellationToken);

    handler.SetFinished();
    return mergeResult;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2018
//---------------------------------------------------------------------------------------
DbResult MergeChangeSetWithReopen(Dgn::DgnDbPtr& db, BeFileNameCR filePath, DgnRevisionPtr changeSet)
    {
    db->CloseDb();

    BeSQLite::DbResult status;
    bvector<DgnRevisionPtr> changeSets;
    changeSets.push_back(changeSet);
    db = OpenWithSchemaUpgradeInternal(&status, filePath, changeSets, SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck, RevisionProcessOption::Merge);
    return status;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
StatusResult Client::MergeChangeSetsIntoDgnDb(Dgn::DgnDbPtr db, const ChangeSets changeSets, BeFileNameCR filePath,
                                              Http::Request::ProgressCallbackCR callback,
                                              ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "Client::MergeChangeSetsIntoDgnDb";

    uint64_t totalSize = 0;
    if (callback != nullptr)
        {
        for (DgnRevisionPtr changeSet : changeSets)
            {
            uint64_t fileSize;
            auto status = changeSet->GetRevisionChangesFile().GetFileSize(fileSize);
            if (BeFileNameStatus::Success != status)
                return StatusResult::Error(Error(Error::Id::FileNotFound));
            totalSize += fileSize;
            }

        callback(0.0, totalSize);
        }

    RevisionStatus mergeStatus = RevisionStatus::Success;

    uint64_t sizeMerged = 0;
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Merging changeSets.");
    if (!changeSets.empty())
        {
        for (DgnRevisionPtr changeSet : changeSets)
            {
            if (changeSet->ContainsSchemaChanges(*db))
                {
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Merging changeSets with DgnDb reopen.");
                DbResult status = MergeChangeSetWithReopen(db, filePath, changeSet);
                if (DbResult::BE_SQLITE_OK != status)
                    {
                    StatusResult result = StatusResult::Error(Error(db, status));
                    if (!result.IsSuccess())
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                    return result;
                    }
                }
            else
                {
                mergeStatus = db->Revisions().MergeRevision(*changeSet);
                if (mergeStatus != RevisionStatus::Success)
                    break; // TODO: Use the information on the changeSet that actually failed. 
                }

            if (callback != nullptr)
                {
                uint64_t fileSize;
                changeSet->GetRevisionChangesFile().GetFileSize(fileSize);
                sizeMerged += fileSize;
                callback(sizeMerged, totalSize);
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

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(m_serverUrl, ServerProperties::ServiceVersion(), iModelInfo.GetWSRepositoryName(), m_clientInfo, nullptr,
                                                               m_customHandler);
    client->SetCredentials(m_credentials);

    Utf8String briefcaseIdString;
    briefcaseIdString.Sprintf("%u", briefcaseId);
    ObjectId briefcaseObjectId = ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase, briefcaseIdString);

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, requestOptions, "Sending abandon briefcase request. iModel ID: %s.", iModelInfo.GetId().c_str());
    return client->SendDeleteObjectRequestWithOptions(briefcaseObjectId, requestOptions, cancellationToken)->Then<StatusResult>([=](WSDeleteObjectResult const& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, end - start, requestOptions, "Success.");
            return StatusResult::Success();
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Vilius.Kazlauskas              09/2018
//---------------------------------------------------------------------------------------
StatusTaskPtr Client::UpdateiModel(Utf8StringCR contextId, iModelInfoCR iModelInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::UpdateiModel";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    if (iModelInfo.GetId().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid iModel id.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::InvalidiModelId));
        }

    Json::Value iModelJson = iModelCreationJson(iModelInfo);
    IWSRepositoryClientPtr client = CreateContextConnection(contextId);

    return client->SendUpdateObjectRequestWithOptions(ObjectId(ServerSchema::Schema::Context, ServerSchema::Class::iModel, iModelInfo.GetId()),
                                               iModelJson[ServerSchema::Instance][ServerSchema::Properties], nullptr, BeFileName(), nullptr,
                                               requestOptions, cancellationToken)
        ->Then<StatusResult>([=] (const WSUpdateObjectResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), requestOptions, "Success.");
        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr Client::DeleteiModel(Utf8StringCR contextId, iModelInfoCR iModelInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Client::DeleteiModel";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    IWSRepositoryClientPtr client = CreateContextConnection(contextId);
    ObjectId iModelId = ObjectId(ServerSchema::Schema::Context, ServerSchema::Class::iModel, iModelInfo.GetId());
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    changeset->AddInstance(iModelId, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value>());
    m_globalRequestOptionsPtr->InsertRequestOptions(changeset);

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, requestOptions, "Sending delete iModel request. iModel ID: %s.", iModelInfo.GetId().c_str());
    const HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return client->SendChangesetRequestWithOptions(request, nullptr, requestOptions, cancellationToken)->Then<StatusResult>([=](WSChangesetResult const& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "Success.");
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

    auto changeSetsResult = ExecuteAsync(connection->GetChangeSetsFromQueryByChunks(query, false, cancellationToken));

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

    uint64_t changeSetsSize = 0;
    for (ChangeSetInfoPtr changeSet : changeSetsToMerge)
        {
        changeSetsSize += changeSet->GetFileSize();
        }
    uint64_t totalSize = 2 * changeSetsSize + fileInfo.GetSize();

    MultiProgressCallbackHandler handler(callback, totalSize);
    Http::Request::ProgressCallback changeSetsCallback, mergeCallback, seedFileCallback;
    handler.AddCallback(changeSetsCallback);
    handler.AddCallback(mergeCallback);
    handler.AddCallback(seedFileCallback);

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
                                                                                       SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck));
    if (BeSQLite::DbResult::BE_SQLITE_OK != status)
        {
        StatusResult result = StatusResult::Error(Error(db, status));
        if (!result.IsSuccess())
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(result));
        }

    auto margeStatus = MergeChangeSetsIntoDgnDb(db, changeSetsResult.GetValue(), filePath, mergeCallback, cancellationToken);
    if (!margeStatus.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, margeStatus.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Error(margeStatus.GetError()));
        }

    handler.SetFinished();
    BeFileName::SetFileReadOnly(filePath, true);

    return CreateCompletedAsyncTask<BeFileNameResult>(BeFileNameResult::Success(filePath));
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
GlobalConnectionTaskPtr Client::GlobalConnection()
    {
    const Utf8String methodName = "Client::GlobalEvents";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if(m_globalConnectionPtr != nullptr)
        {
        return CreateCompletedAsyncTask<GlobalConnectionResult>(GlobalConnectionResult::Success(m_globalConnectionPtr));
        }

    if (m_serverUrl.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Server URL is invalid.");
        return CreateCompletedAsyncTask<GlobalConnectionResult>(GlobalConnectionResult::Error(Error::Id::InvalidServerURL));//NEEDSWORK: different message?
        }

    if (!m_credentials.IsValid() && !m_customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return CreateCompletedAsyncTask<GlobalConnectionResult>(GlobalConnectionResult::Error(Error::Id::CredentialsNotSet));
        }

    auto globalConnectionResult = GlobalConnection::Create(m_serverUrl, m_credentials, m_clientInfo, m_customHandler);
    if (globalConnectionResult.IsSuccess())
        m_globalConnectionPtr = globalConnectionResult.GetValue();

    return CreateCompletedAsyncTask<GlobalConnectionResult>(globalConnectionResult);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             07/2018
//---------------------------------------------------------------------------------------
GlobalRequestOptionsPtr Client::GlobalRequestOptions() const
    {
    return m_globalRequestOptionsPtr;
    }
