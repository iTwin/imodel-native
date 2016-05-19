/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV1.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApiV1.h"
#include <WebServices/Client/Response/WSObjectsReaderV1.h>

Utf8CP const WebApiV1::SERVICE_Navigation = "Navigation";
Utf8CP const WebApiV1::SERVICE_Objects = "Objects";
Utf8CP const WebApiV1::SERVICE_Files = "Files";
Utf8CP const WebApiV1::SERVICE_Schema = "Schema";

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebApiV1::WebApiV1(std::shared_ptr<const ClientConfiguration> configuration, WSInfoCR info) :
WebApi(configuration),
m_info(info)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebApiV1::~WebApiV1()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool WebApiV1::IsSupported(WSInfoCR info)
    {
    return info.GetWebApiVersion() <= BeVersion(1, 3) && info.GetType() == WSInfo::Type::BentleyWSG;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest WebApiV1::CreateGetRepositoriesRequest(const bvector<Utf8String>& types, const bvector<Utf8String>& providerIds) const
    {
    Utf8String params;

    if (!types.empty())
        {
        params += "types=" + StringUtils::Join(types.begin(), types.end(), ',');
        }

    if (!providerIds.empty())
        {
        if (!params.empty())
            {
            params += "&";
            }
        params += "providerIds=" + StringUtils::Join(providerIds.begin(), providerIds.end(), ',');
        }

    Utf8PrintfString url
        (
        "%s/%sDataSources",
        m_configuration->GetServerUrl().c_str(),
        CreateWebApiVersionPart("v1.1").c_str()
        );

    if (!params.empty())
        {
        url += "?" + params;
        }

    return m_configuration->GetHttpClient().CreateGetRequest(url);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::CreateObjectIdParam(ObjectIdCR objectId) const
    {
    Utf8String param;
    if (!objectId.IsEmpty())
        {
        param.Sprintf("%s/%s", objectId.className.c_str(), objectId.remoteId.c_str());
        }
    return param;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vinmcas.Razma   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::CreatePropertiesQuery(const bset<Utf8String>& properties) const
    {
    return CreatePropertiesQuery(StringUtils::Join(properties.begin(), properties.end(), ","));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vinmcas.Razma   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::CreatePropertiesQuery(Utf8StringCR properties) const
    {
    Utf8String query;
    if (!properties.empty())
        {
        query.Sprintf("properties=%s", properties.c_str());
        }
    return query;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vinmcas.Razma   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::CreateParentQuery(ObjectIdCR objectId) const
    {
    Utf8String query;
    if (!objectId.IsEmpty())
        {
        query.Sprintf("parentClass=%s&parentObjectId=%s", objectId.className.c_str(), objectId.remoteId.c_str());
        }
    return query;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::GetUrl(Utf8StringCR service, Utf8StringCR params, Utf8StringCR queryString, Utf8StringCR webApiVersion) const
    {
    Utf8PrintfString url
        (
        "%s/%sDataSources/%s/%s",
        m_configuration->GetServerUrl().c_str(),
        CreateWebApiVersionPart(webApiVersion).c_str(),
        HttpClient::EscapeString(m_configuration->GetRepositoryId()).c_str(),
        service.c_str()
        );

    if (!params.empty())
        {
        url += "/" + params;
        }

    if (!queryString.empty())
        {
        url += "?" + queryString;
        }

    BeAssert(url.size() < 2000 && "<Warning> Url length might be problematic as it is longer than most default settings");
    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::CreateWebApiVersionPart(Utf8StringCR webApiVersion) const
    {
    if (m_info.GetWebApiVersion() <= BeVersion(1, 1))
        {
        return "";
        }
    return webApiVersion + "/";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::GetMaxWebApi() const
    {
    BeVersion version = m_info.GetWebApiVersion();
    return Utf8PrintfString("v%d.%d", version.GetMajor(), version.GetMinor());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebApiV1::ParseRepository(JsonValueCR dataSourceJson, WSRepository& repositoryOut)
    {
    Utf8String dataSourceId = dataSourceJson["id"].asString();
    Utf8String dataSourceType = dataSourceJson["type"].asString();
    Utf8String providerId = dataSourceJson["providerId"].asString();

    Utf8String dataSourceIdPrefix;

    if (providerId.EqualsI("ec"))
        {
        dataSourceIdPrefix.Sprintf("%s.%s--", providerId.c_str(), dataSourceType.c_str());
        repositoryOut.SetPluginId(std::move(dataSourceType));
        }
    else
        {
        dataSourceIdPrefix.Sprintf("%s.", providerId.c_str());
        repositoryOut.SetPluginId(std::move(providerId));
        }

    Utf8String location;
    if (0 == dataSourceId.compare(0, dataSourceIdPrefix.length(), dataSourceIdPrefix))
        {
        location = dataSourceId.substr(dataSourceIdPrefix.length(), dataSourceId.length() - dataSourceIdPrefix.length());
        }

    repositoryOut.SetLocation(std::move(location));
    repositoryOut.SetId(std::move(dataSourceId));
    repositoryOut.SetLabel(dataSourceJson["label"].asString());
    repositoryOut.SetDescription(dataSourceJson["description"].asString());

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepositoriesResult WebApiV1::ResolveGetRepositoriesResponse(HttpResponse& response)
    {
    if (!response.IsSuccess() || !IsJsonResponse(response))
        {
        return WSRepositoriesResult::Error(response);
        }
    Json::Value responseJson = response.GetBody().AsJson();
    if (responseJson.isNull() || !responseJson.isArray())
        {
        return WSRepositoriesResult::Error(response);
        }
    bvector<WSRepository> repositories;
    for (JsonValueCR dataSourceJson : responseJson)
        {
        WSRepository repository;
        if (SUCCESS != ParseRepository(dataSourceJson, repository))
            {
            return WSRepositoriesResult::Error(WSError::CreateServerNotSupportedError());
            }
        repositories.push_back(repository);
        }
    return WSRepositoriesResult::Success(repositories);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSCreateObjectResult WebApiV1::ResolveCreateObjectResponse(HttpResponse& response, ObjectIdCR newObjectId, ObjectIdCR relObjectId, ObjectIdCR parentObjectId)
    {
    Utf8String remoteId = response.GetBody().AsJson()["id"].asString();
    if (HttpStatus::Created == response.GetHttpStatus() && !remoteId.empty())
        {
        Json::Value createdObject;

        auto& instance = createdObject["changedInstance"]["instanceAfterChange"];
        instance["schemaName"] = newObjectId.schemaName;
        instance["className"] = newObjectId.className;
        instance["instanceId"] = remoteId;

        if (!parentObjectId.IsEmpty())
            {
            auto& relationship = createdObject["changedInstance"]["instanceAfterChange"]["relationshipInstances"][0];
            relationship["schemaName"] = relObjectId.schemaName;
            relationship["className"] = relObjectId.className;
            relationship["instanceId"] = "";

            auto& related = relationship["relatedInstance"];
            related["schemaName"] = relObjectId.schemaName;
            related["className"] = relObjectId.className;
            related["instanceId"] = "";
            }

        return WSCreateObjectResult::Success(createdObject);
        }
    return WSCreateObjectResult::Error(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jahan.Zeb    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSUpdateObjectResult WebApiV1::ResolveUpdateObjectResponse(HttpResponse& response)
    {
    if (HttpStatus::OK != response.GetHttpStatus())
        {
        return WSUpdateObjectResult::Error(response);
        }
    return WSUpdateObjectResult::Success();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsResult WebApiV1::ResolveObjectsResponse(HttpResponse& response, Utf8StringCR schemaName, Utf8StringCR objectClassName)
    {
    if (!IsValidObjectsResponse(response))
        {
        return WSObjectsResult::Error(response);
        }

    std::shared_ptr<WSObjectsReader> reader;
    if (!objectClassName.empty())
        {
        reader = WSObjectsReaderV1::Create(schemaName, objectClassName);
        }
    else
        {
        reader = WSObjectsReaderV1::Create(schemaName);
        }

    auto body = response.GetContent()->GetBody();
    auto eTag = response.GetHeaders().GetETag();

    return WSObjectsResult::Success(WSObjectsResponse(reader, body, response.GetHttpStatus(), eTag, nullptr));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool WebApiV1::IsValidObjectsResponse(HttpResponseCR response)
    {
    HttpStatus status = response.GetHttpStatus();
    if (HttpStatus::NotModified == status)
        {
        return true;
        }
    if (HttpStatus::OK == status && IsJsonResponse(response))
        {
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool WebApiV1::IsJsonResponse(HttpResponseCR response)
    {
    if (Utf8String(response.GetHeaders().GetContentType()).find("application/json") != Utf8String::npos)
        {
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool WebApiV1::IsObjectCreationJsonSupported(JsonValueCR objectCreationJson)
    {
    JsonValueCR instanceJson = objectCreationJson["instance"];

    if (instanceJson["relationshipInstances"].size() == 0)
        {
        return true;
        }
    if (instanceJson["relationshipInstances"].size() == 1)
        {
        if (instanceJson["relationshipInstances"][0]["relatedInstance"]["relationshipInstances"].empty())
            {
            return true;
            }
        else
            {
            return false;
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void WebApiV1::GetParametersFromObjectCreationJson
(
JsonValueCR objectCreationJson,
Utf8StringR propertiesOut,
ObjectIdR relObjectId,
ObjectIdR parentObjectIdOut
)
    {
    JsonValueCR instance = objectCreationJson["instance"];

    propertiesOut = Json::FastWriter::ToString(instance["properties"]);

    if (0 == instance["relationshipInstances"].size())
        {
        relObjectId = ObjectId();
        parentObjectIdOut = ObjectId();
        }
    else
        {
        auto& relationship = instance["relationshipInstances"][0];
        relObjectId.schemaName = relationship["schemaName"].asString();;
        relObjectId.className = relationship["className"].asString();
        relObjectId.remoteId = relationship["instanceId"].asString();

        auto& related = relationship["relatedInstance"];
        parentObjectIdOut.schemaName = related["schemaName"].asString();
        parentObjectIdOut.className = related["className"].asString();
        parentObjectIdOut.remoteId = related["instanceId"].asString();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSFileResult WebApiV1::ResolveFileResponse(HttpResponse& response, BeFileName filePath)
    {
    HttpStatus status = response.GetHttpStatus();
    if (HttpStatus::OK == status ||
        HttpStatus::NotModified == status)
        {
        return WSFileResult::Success(WSFileResponse(filePath, status, response.GetHeaders().GetETag()));
        }
    return WSFileResult::Error(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSRepositoriesResult> WebApiV1::SendGetRepositoriesRequest
(
const bvector<Utf8String>& types,
const bvector<Utf8String>& providerIds,
ICancellationTokenPtr ct
) const
    {
    HttpRequest request = CreateGetRepositoriesRequest(types, providerIds);
    request.SetCancellationToken(ct);
    return request.PerformAsync()->Then<WSRepositoriesResult>([] (HttpResponse& httpResponse)
        {
        return ResolveGetRepositoriesResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WebApiV1::SendGetObjectRequest
(
ObjectIdCR objectId,
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    BeAssert(!objectId.IsEmpty() && "<Error> DataSource is not object");

    Utf8String url = GetUrl(SERVICE_Objects, CreateObjectIdParam(objectId));
    HttpRequest request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    request.SetRetryOptions(HttpRequest::ResetTransfer, 1);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObject);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([objectId] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse(httpResponse, objectId.schemaName, objectId.className);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WebApiV1::SendGetChildrenRequest
(
ObjectIdCR parentObjectId,
const bset<Utf8String>& propertiesToSelect,
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    return SendGetChildrenRequest(parentObjectId, CreatePropertiesQuery(propertiesToSelect), eTag, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WebApiV1::SendGetChildrenRequest
(
ObjectIdCR parentObjectId,
Utf8StringCR propertiesQuery,
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    if (!propertiesQuery.empty() && !m_info.IsWebApiSupported(BeVersion(1, 3)))
        {
        return CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    Utf8String url = GetUrl(SERVICE_Navigation, CreateObjectIdParam(parentObjectId), propertiesQuery, propertiesQuery.empty() ? "v1.1" : "v1.3");
    HttpRequest request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    request.SetRetryOptions(HttpRequest::ResetTransfer, 1);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.SetCancellationToken(ct);

    auto thisPtr = shared_from_this();
    auto masResponse = std::make_shared<WSObjectsResult>();

    return
        request.PerformAsync()
        ->Then([=] (HttpResponse& childrenResponse)
        {
        if (childrenResponse.GetConnectionStatus() != ConnectionStatus::OK)
            {
            masResponse->SetError(childrenResponse);
            return;
            }

        GetSchemaInfo(ct)
            ->Then([=] (SchemaInfoResult& schemaInfoResult) mutable
            {
            if (!schemaInfoResult.IsSuccess())
                {
                masResponse->SetError(schemaInfoResult.GetError());
                return;
                }

            *masResponse = ResolveObjectsResponse(childrenResponse, schemaInfoResult.GetValue().name);
            });
        })
            ->Then<WSObjectsResult>([thisPtr, masResponse]
            {
            return *masResponse;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WebApiV1::SchemaInfoResult> WebApiV1::GetSchemaInfo(ICancellationTokenPtr ct) const
    {
    SchemaInfo info = GetCachedSchemaInfo();
    if (!info.name.empty())
        {
        return CreateCompletedAsyncTask(SchemaInfoResult::Success(info));
        }

    auto schemaBody = HttpStringBody::Create();

    return
        GetSchema(schemaBody, "", nullptr, ct)
        ->Then<SchemaInfoResult>([=] (SchemaResult& result) mutable
        {
        if (!result.IsSuccess())
            {
            return SchemaInfoResult::Error(result.GetError());
            }

        if (SUCCESS != ReadSchemaInfoFromXmlString(schemaBody->AsString(), info))
            {
            return SchemaInfoResult::Error(WSError::CreateFunctionalityNotSupportedError());
            }
        SetCachedSchemaInfo(info);

        return SchemaInfoResult::Success(info);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebApiV1::ReadSchemaInfoFromFile(BeFileNameCR filePath, SchemaInfo& schemaInfoOut)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filePath);
    if (SUCCESS != ReadSchemaInfoFromXmlDom(xmlStatus, xmlDom, schemaInfoOut))
        {
        LOG.errorv("Failed to get name from schema file: \"%s\"", filePath.GetNameUtf8().c_str());
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebApiV1::ReadSchemaInfoFromXmlString(Utf8StringCR xmlString, SchemaInfo& schemaInfoOut)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, xmlString.c_str(), xmlString.size());
    if (SUCCESS != ReadSchemaInfoFromXmlDom(xmlStatus, xmlDom, schemaInfoOut))
        {
        LOG.errorv("Failed to get info from schema: \"%s\"", xmlString.c_str());
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebApiV1::ReadSchemaInfoFromXmlDom(BeXmlStatus xmlDomReadStatus, BeXmlDomPtr schemaXmlDom, SchemaInfo& schemaInfoOut)
    {
    if (BeXmlStatus::BEXML_Success != xmlDomReadStatus && schemaXmlDom.IsValid())
        {
        WString message;
        schemaXmlDom->GetErrorMessage(message);
        LOG.error(message.c_str());
        return ERROR;
        }

    if (!schemaXmlDom.IsValid())
        {
        return ERROR;
        }

    BeXmlNodeP rootNode = schemaXmlDom->GetRootElement();
    if (nullptr == rootNode)
        {
        return ERROR;
        }

    rootNode->GetAttributeStringValue(schemaInfoOut.name, "schemaName");
    if (schemaInfoOut.name.empty())
        {
        return ERROR;
        }

    Utf8String versionStr;
    rootNode->GetAttributeStringValue(versionStr, "version");
    if (versionStr.empty())
        {
        return ERROR;
        }
    schemaInfoOut.version = BeVersion(versionStr.c_str());

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebApiV1::SchemaInfo WebApiV1::GetCachedSchemaInfo() const
    {
    BeMutexHolder mutex (m_schemaInfoCS);
    return m_schemaInfo;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WebApiV1::SetCachedSchemaInfo(SchemaInfo schemaInfo) const
    {
    BeMutexHolder mutex (m_schemaInfoCS);
    m_schemaInfo = std::move(schemaInfo);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSFileResult> WebApiV1::GetSchema
(
BeFileNameCR filePath,
Utf8StringCR eTag,
HttpRequest::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return
        GetSchema(HttpFileBody::Create(filePath), eTag, downloadProgressCallback, ct)
        ->Then<WSFileResult>([filePath] (SchemaResult& result)
        {
        if (!result.IsSuccess())
            {
            return WSFileResult::Error(result.GetError());
            }
        HttpStatus status = result.GetValue().isModified ? HttpStatus::OK : HttpStatus::NotModified;
        return WSFileResult::Success(WSFileResponse(filePath, status, result.GetValue().eTag));
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSFileResult> WebApiV1::SendGetFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
Utf8StringCR eTag,
HttpRequest::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (SchemaInfo::IsDummySchemaId(objectId))
        {
        return GetSchema(filePath, eTag, downloadProgressCallback, ct);
        }

    Utf8String url = GetUrl(SERVICE_Files, CreateObjectIdParam(objectId));
    HttpRequest request = m_configuration->GetHttpClient().CreateGetRequest(url, eTag);

    request.SetResponseBody(HttpFileBody::Create(filePath));
    request.SetRetryOptions(HttpRequest::ResumeTransfer, 0);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::FileDownload);
    request.SetDownloadProgressCallback(downloadProgressCallback);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSFileResult>([filePath] (HttpResponse& httpResponse)
        {
        return ResolveFileResponse(httpResponse, filePath);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WebApiV1::SchemaResult> WebApiV1::GetSchema
(
HttpBodyPtr body,
Utf8StringCR eTag,
HttpRequest::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    BeFileName defaultSchemaPath = m_configuration->GetDefaultSchemaPath(m_info);
    if (!defaultSchemaPath.empty())
        {
        SchemaInfo info;
        if (SUCCESS != ReadSchemaInfoFromFile(defaultSchemaPath, info))
            {
            return CreateCompletedAsyncTask(SchemaResult::Error(WSError::CreateFunctionalityNotSupportedError()));
            }
        SetCachedSchemaInfo(info);

        if (info.CreateDummyRemoteId() == eTag)
            {
            SchemaResponse response;
            response.isModified = false;
            return CreateCompletedAsyncTask(SchemaResult::Success(response));
            }

        if (SUCCESS != WriteFileToHttpBody(defaultSchemaPath, body))
            {
            return CreateCompletedAsyncTask(SchemaResult::Error(WSError::CreateFunctionalityNotSupportedError()));
            }

        SchemaResponse response;
        response.eTag = info.CreateDummyRemoteId();
        response.isModified = true;
        return CreateCompletedAsyncTask(SchemaResult::Success(response));
        }

    Utf8String url = GetSchemaUrl();
    if (url.empty())
        {
        return CreateCompletedAsyncTask(SchemaResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    HttpRequest request = m_configuration->GetHttpClient().CreateGetRequest(url, eTag);

    request.SetResponseBody(body);
    request.GetHeaders().SetAccept("application/xml");
    request.SetDownloadProgressCallback(downloadProgressCallback);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<SchemaResult>([] (HttpResponse& httpResponse)
        {
        HttpStatus status = httpResponse.GetHttpStatus();
        if (HttpStatus::OK == status)
            {
            SchemaResponse response;
            response.eTag = httpResponse.GetHeaders().GetETag();
            response.isModified = true;
            return SchemaResult::Success(response);
            }
        else if (HttpStatus::NotModified == status)
            {
            SchemaResponse response;
            response.isModified = false;
            return SchemaResult::Success(response);
            }
        return SchemaResult::Error(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebApiV1::WriteFileToHttpBody(BeFileNameCR filePath, HttpBodyPtr body)
    {
    BeFile file;
    BeFileStatus status;

    status = file.Open(filePath, BeFileAccess::Read);
    body->Open();

    if (BeFileStatus::Success != status)
        {
        BeAssert(false);
        return ERROR;
        }

    static const uint32_t bufferSize = 1000;
    Utf8Char buffer[static_cast<size_t>(bufferSize)];

    uint32_t bytesRead = 1;
    while (BeFileStatus::Success == status && bytesRead > 0)
        {
        status = file.Read(buffer, &bytesRead, bufferSize);
        auto written = body->Write(buffer, bytesRead);
        if (written != bytesRead)
            {
            status = BeFileStatus::UnknownError;
            }
        }

    body->Close();
    file.Close();

    if (BeFileStatus::Success != status)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::GetSchemaUrl() const
    {
    if (!m_info.IsWebApiSupported(BeVersion(1, 2)))
        {
        return "";
        }
    return GetUrl(SERVICE_Schema, "", nullptr, "v1.2");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WebApiV1::SendGetSchemasRequest
(
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    auto schemaBody = HttpStringBody::Create();

    return
        GetSchema(schemaBody, eTag, nullptr, ct)
        ->Then<WSObjectsResult>([=] (SchemaResult& result)
        {
        if (!result.IsSuccess())
            {
            return WSObjectsResult::Error(result.GetError());
            }

        HttpResponse schemaDefResponse;
        if (result.GetValue().isModified)
            {
            SchemaInfo info;
            if (SUCCESS != ReadSchemaInfoFromXmlString(schemaBody->AsString(), info))
                {
                return WSObjectsResult::Error(WSError::CreateFunctionalityNotSupportedError());
                }

            Json::Value schemaDefInstance;
            schemaDefInstance["$id"] = info.CreateDummyRemoteId();
            schemaDefInstance["Name"] = info.name;
            schemaDefInstance["VersionMajor"] = info.version.GetMajor();
            schemaDefInstance["VersionMinor"] = info.version.GetMinor();

            auto schemaDefContent = HttpResponseContent::Create(HttpStringBody::Create(schemaDefInstance.toStyledString()));
            schemaDefContent->GetHeaders().SetETag(result.GetValue().eTag);
            schemaDefContent->GetHeaders().SetContentType("application/json");

            schemaDefResponse = HttpResponse(schemaDefContent, "", ConnectionStatus::OK, HttpStatus::OK);
            }
        else
            {
            schemaDefResponse = HttpResponse(HttpResponseContent::Create(HttpStringBody::Create()), "", ConnectionStatus::OK, HttpStatus::NotModified);
            }

        return ResolveObjectsResponse(schemaDefResponse, "MetaSchema", "ECSchemaDef");
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WebApiV1::SendQueryRequest
(
WSQueryCR query,
Utf8StringCR eTag,
Utf8StringCR skipToken,
ICancellationTokenPtr ct
) const
    {
    auto it = query.GetCustomParameters().find(WSQuery_CustomParameter_NavigationParentId);
    if (it != query.GetCustomParameters().end())
        {
        ObjectId parentId;
        parentId.remoteId = it->second;

        if (!parentId.remoteId.empty())
            {
            parentId.schemaName = query.GetSchemaName();
            if (query.GetClasses().size() == 1)
                {
                parentId.className = *query.GetClasses().begin();
                }
            else
                {
                BeAssert(false && "Expected one parent instance class for navigation query");
                }
            }

        return SendGetChildrenRequest(parentId, CreatePropertiesQuery(query.GetSelect()), eTag, ct);
        }

    Utf8String schemaName = query.GetSchemaName();
    Utf8String classes = StringUtils::Join(query.GetClasses().begin(), query.GetClasses().end(), ',');

    Utf8String url = GetUrl(SERVICE_Objects, classes, query.ToQueryString(), GetMaxWebApi());
    HttpRequest request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.GetHeaders().SetIfNoneMatch(eTag);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([schemaName] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse(httpResponse, schemaName);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSChangesetResult> WebApiV1::SendChangesetRequest
(
HttpBodyPtr changeset,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return CreateCompletedAsyncTask(WSChangesetResult::Error(WSError::CreateFunctionalityNotSupportedError()));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WebApiV1::SendCreateObjectRequest
(
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    Utf8String schemaName(objectCreationJson["instance"]["schemaName"].asString());
    Utf8String className(objectCreationJson["instance"]["className"].asString());
    Utf8String remoteId(objectCreationJson["instance"]["instanceId"].asString());
    ObjectId objectId(schemaName, className, Utf8String());

    return SendCreateObjectRequest(objectId, objectCreationJson, filePath, uploadProgressCallback, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WebApiV1::SendCreateObjectRequest
(
ObjectIdCR objectId,
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (!IsObjectCreationJsonSupported(objectCreationJson) || !m_info.IsWebApiSupported(BeVersion(1, 2)))
        {
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    if (objectId.className.empty())
        {
        BeDebugLog("The className passed into WebApiV1::SendCreateObjectRequest is empty. ClassName is required to be valid.");
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError()));
        }

    BeAssert(objectId.schemaName.Equals(objectCreationJson["instance"]["schemaName"].asString()) 
             && "schemaName in objectId parameter should match objectCreationJson schemanName.");
    BeAssert(objectId.className.Equals(objectCreationJson["instance"]["className"].asString())
             && "className in objectId parameter should match objectCreationJson className.");

    Utf8String propertiesStr;
    ObjectId relObjectId, parentObjectId;

    GetParametersFromObjectCreationJson
        (objectCreationJson, propertiesStr, relObjectId, parentObjectId);

    Utf8String url = GetUrl(SERVICE_Objects, objectId.className, CreateParentQuery(parentObjectId), "v1.2");
    ChunkedUploadRequest request("POST", url, m_configuration->GetHttpClient());

    request.SetHandshakeRequestBody(HttpStringBody::Create(propertiesStr), "application/json");
    if (!filePath.empty())
        {
        request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));
        }
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSCreateObjectResult>([=] (HttpResponse& httpResponse)
        {
        return ResolveCreateObjectResponse(httpResponse, objectId, relObjectId, parentObjectId);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSUpdateObjectResult> WebApiV1::SendUpdateObjectRequest
(
ObjectIdCR objectId,
JsonValueCR propertiesJson,
Utf8String eTag,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    Utf8String url = GetUrl(SERVICE_Objects, CreateObjectIdParam(objectId));
    HttpRequest request = m_configuration->GetHttpClient().CreatePostRequest(url);

    request.GetHeaders().SetContentType("application/json");
    if (!eTag.empty())
        {
        request.GetHeaders().SetIfMatch(eTag);
        }
    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(propertiesJson)));
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSUpdateObjectResult>([] (HttpResponse& httpResponse)
        {
        return ResolveUpdateObjectResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSDeleteObjectResult> WebApiV1::SendDeleteObjectRequest
(
ObjectIdCR objectId,
ICancellationTokenPtr ct
) const
    {
    Utf8String url = GetUrl(SERVICE_Objects, CreateObjectIdParam(objectId));
    HttpRequest request = m_configuration->GetHttpClient().CreateRequest(url, "DELETE");

    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSDeleteObjectResult>([] (HttpResponse& httpResponse)
        {
        if (HttpStatus::OK == httpResponse.GetHttpStatus())
            {
            return WSUpdateObjectResult::Success();
            }
        return WSUpdateObjectResult::Error(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSUpdateFileResult> WebApiV1::SendUpdateFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    Utf8String url = GetUrl(SERVICE_Files, CreateObjectIdParam(objectId));
    ChunkedUploadRequest request("PUT", url, m_configuration->GetHttpClient());

    request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSUpdateFileResult>([] (HttpResponse& httpResponse)
        {
        if (HttpStatus::OK == httpResponse.GetHttpStatus())
            {
            return WSUpdateFileResult::Success();
            }
        return WSUpdateFileResult::Error(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1::SchemaInfo::CreateDummyRemoteId() const
    {
    return Utf8PrintfString("DUMMY_SCHEMA_OBJECT-%s.%02d.%02d", name.c_str(), version.GetMajor(), version.GetMinor());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WebApiV1::SchemaInfo::IsDummySchemaId(ObjectIdCR objectId)
    {
    if (!objectId.schemaName.Equals("MetaSchema") ||
        !objectId.className.Equals("ECSchemaDef"))
        {
        return false;
        }
    static const Utf8String prefix = "DUMMY_SCHEMA_OBJECT";
    return 0 == objectId.remoteId.compare(0, prefix.size(), prefix.c_str());
    }
