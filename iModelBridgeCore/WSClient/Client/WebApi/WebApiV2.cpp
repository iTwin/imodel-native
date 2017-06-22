/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV2.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApiV2.h"
#include <WebServices/Client/Response/WSObjectsReaderV2.h>

#define HEADER_SkipToken                "SkipToken"
#define HEADER_MasAllowRedirect         "Mas-Allow-Redirect"
#define HEADER_MasFileAccessUrlType     "Mas-File-Access-Url-Type"
#define HEADER_MasUploadConfirmationId  "Mas-Upload-Confirmation-Id"
#define HEADER_MasFileETag              "Mas-File-ETag"

#define VALUE_FileAccessUrlType_Azure   "AzureBlobSasUrl"
#define VALUE_True                      "true"

const BeVersion WebApiV2::s_maxTestedWebApi(2, 5);

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WebApiV2::WebApiV2(std::shared_ptr<const ClientConfiguration> configuration, WSInfo info) :
WebApi(configuration),
m_info(info),
m_azureClient(AzureBlobStorageClient::Create(configuration->GetHttpHandler()))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WebApiV2::~WebApiV2()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool WebApiV2::IsSupported(WSInfoCR info)
    {
    return
        info.GetWebApiVersion() >= BeVersion(2, 0) &&
        info.GetWebApiVersion() < BeVersion(3, 0) &&
        info.GetType() == WSInfo::Type::BentleyWSG;
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BeVersion WebApiV2::GetMaxWebApiVersion() const
    {
    BeVersion webApiVersion = m_info.GetWebApiVersion();
    if (webApiVersion > s_maxTestedWebApi)
        webApiVersion = s_maxTestedWebApi; // Limit queries to tested WebApi version
    return webApiVersion;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetWebApiUrl(BeVersion webApiVersion) const
    {
    if (webApiVersion.IsEmpty())
        webApiVersion = GetMaxWebApiVersion();

    Utf8PrintfString url
        (
        "%s/v%d.%d/",
        m_configuration->GetServerUrl().c_str(),
        webApiVersion.GetMajor(),
        webApiVersion.GetMinor()
        );

    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetRepositoryUrl(Utf8StringCR repositoryId, BeVersion webApiVersion) const
    {
    return GetWebApiUrl(webApiVersion) + "Repositories/" + HttpClient::EscapeString(repositoryId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetUrl(Utf8StringCR path, Utf8StringCR queryString, BeVersion webApiVersion) const
    {
    Utf8String url = GetRepositoryUrl(m_configuration->GetRepositoryId(), webApiVersion);

    if (!path.empty())
        {
        url += "/" + path;
        }

    if (!queryString.empty())
        {
        url += "?" + queryString;
        }

    BeAssert(url.size() < 2000 && "<Warning> Url length might be problematic as it is longer than most default settings");
    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateObjectSubPath(ObjectIdCR objectId) const
    {
    Utf8String param;
    if (!objectId.IsEmpty())
        {
        param.Sprintf("%s/%s/%s", objectId.schemaName.c_str(), objectId.className.c_str(), objectId.remoteId.c_str());
        }
    return param;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateFileSubPath(ObjectIdCR objectId) const
    {
    return CreateObjectSubPath(objectId) + +"/$file";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateClassSubPath(Utf8StringCR schemaName, Utf8StringCR className) const
    {
    return schemaName + "/" + className;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateNavigationSubPath(ObjectIdCR parentId) const
    {
    if (parentId.IsEmpty())
        {
        return "Navigation/NavNode";
        }
    return CreateObjectSubPath(parentId) + "/NavNode";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateSelectPropertiesQuery(const bset<Utf8String>& properties) const
    {
    Utf8String value = StringUtils::Join(properties.begin(), properties.end(), ",");
    Utf8String query;
    if (!value.empty())
        {
        query.Sprintf("$select=%s", value.c_str());
        }
    return query;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
std::shared_ptr<WSObjectsReader> WebApiV2::CreateJsonInstancesReader() const
    {
    bool quoteInstanceETags =
        m_info.GetWebApiVersion() == BeVersion(2, 0) ||
        m_info.GetWebApiVersion() == BeVersion(2, 1);

    return WSObjectsReaderV2::Create(quoteInstanceETags);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetNullableString(RapidJsonValueCR jsonValue)
    {
    if (jsonValue.IsString())
        {
        return jsonValue.GetString();
        }
    BeAssert(jsonValue.IsNull());
    return "";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSRepositoriesResult WebApiV2::ResolveGetRepositoriesResponse(HttpResponse& response) const
    {
    if (HttpStatus::OK != response.GetHttpStatus())
        {
        return WSRepositoriesResult::Error(response);
        }

    auto repositoriesJson = std::make_shared<rapidjson::Document>();
    response.GetBody().AsRapidJson(*repositoriesJson);

    auto reader = CreateJsonInstancesReader();
    WSObjectsReader::Instances instances = reader->ReadInstances(repositoriesJson);
    if (!instances.IsValid())
        {
        return WSRepositoriesResult::Error(WSError::CreateServerNotSupportedError());
        }

    bvector<WSRepository> repositories;
    for (WSObjectsReader::Instance instance : instances)
        {
        if (!instance.IsValid() ||
            !instance.GetObjectId().schemaName.Equals("Repositories") ||
            !instance.GetObjectId().className.Equals("RepositoryIdentifier"))
            {
            return WSRepositoriesResult::Error(WSError::CreateServerNotSupportedError());
            }

        WSRepository repository;

        repository.SetId(instance.GetObjectId().remoteId);
        repository.SetLocation(GetNullableString(instance.GetProperties()["Location"]));
        repository.SetPluginId(GetNullableString(instance.GetProperties()["ECPluginID"]));
        repository.SetLabel(GetNullableString(instance.GetProperties()["DisplayLabel"]));
        repository.SetDescription(GetNullableString(instance.GetProperties()["Description"]));

        repositories.push_back(repository);
        }

    return WSRepositoriesResult::Success(repositories);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSCreateObjectResult WebApiV2::ResolveCreateObjectResponse(HttpResponse& response) const
    {
    if (HttpStatus::Created == response.GetHttpStatus())
        return WSCreateObjectResult::Success(ResolveUploadResponse(response));
    return WSCreateObjectResult::Error(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSUpdateObjectResult WebApiV2::ResolveUpdateObjectResponse(HttpResponse& response) const
    {
    if (HttpStatus::OK == response.GetHttpStatus())
        return WSUpdateObjectResult::Success(ResolveUploadResponse(response));
    return WSUpdateObjectResult::Error(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSUploadResponse WebApiV2::ResolveUploadResponse(HttpResponse& response) const
    {
    auto body = response.GetContent()->GetBody();
    auto eTag = response.GetHeaders().GetValue(HEADER_MasFileETag);
    return WSUploadResponse(body, eTag);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSObjectsResult WebApiV2::ResolveObjectsResponse(HttpResponse& response, bool requestHadSkipToken, const ObjectId* objectId) const
    {
    HttpStatus status = response.GetHttpStatus();
    if (HttpStatus::OK == status ||
        HttpStatus::NotModified == status)
        {
        auto reader = CreateJsonInstancesReader();

        auto body = response.GetContent()->GetBody();
        auto eTag = response.GetHeaders().GetETag();

        Utf8String skipToken;
        if (requestHadSkipToken)
             skipToken = response.GetHeaders().GetValue(HEADER_SkipToken);

        return WSObjectsResult::Success(WSObjectsResponse(reader, body, status, eTag, skipToken));
        }
    return WSObjectsResult::Error(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSRepositoriesResult> WebApiV2::SendGetRepositoriesRequest
(
const bvector<Utf8String>& types,
const bvector<Utf8String>& providerIds,
ICancellationTokenPtr ct
) const
    {
    // TODO: implement filtering query by PluginId if needed
    Utf8String url = GetRepositoryUrl("");
    HttpRequest request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    request.SetCancellationToken(ct);
    return request.PerformAsync()->Then<WSRepositoriesResult>([this] (HttpResponse& httpResponse)
        {
        return ResolveGetRepositoriesResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetObjectRequest
(
ObjectIdCR objectId,
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    HttpRequest request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    request.SetRetryOptions(HttpRequest::ResetTransfer, 1);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObject);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this, objectId] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse(httpResponse, false, &objectId);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetChildrenRequest
(
ObjectIdCR parentObjectId,
const bset<Utf8String>& propertiesToSelect,
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    Utf8String url = GetUrl(CreateNavigationSubPath(parentObjectId), CreateSelectPropertiesQuery(propertiesToSelect));
    HttpRequest request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSFileResult> WebApiV2::SendGetFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
Utf8StringCR eTag,
HttpRequest::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (!objectId.IsValid() || filePath.empty())
        return CreateCompletedAsyncTask(WSFileResult::Error(WSError::CreateFunctionalityNotSupportedError()));

    bool isExternalFileAccessSupported = GetMaxWebApiVersion() >= BeVersion(2, 4);

    Utf8String url = GetUrl(CreateFileSubPath(objectId));
    HttpRequest request = CreateFileDownloadRequest(url, filePath, eTag, downloadProgressCallback, ct);

    if (isExternalFileAccessSupported)
        {
        request.GetHeaders().SetValue(HEADER_MasAllowRedirect, VALUE_True);
        request.SetFollowRedirects(false);
        }

    auto finalResult = std::make_shared<WSFileResult>();
    return request.PerformAsync()->Then([=] (HttpResponse& response)
        {
        if (HttpStatus::TemporaryRedirect != response.GetHttpStatus())
            {
            *finalResult = ResolveFileDownloadResponse(response, filePath);
            return;
            }

        Utf8String redirectUrl = response.GetHeaders().GetLocation();
        HttpRequest request = CreateFileDownloadRequest(redirectUrl, filePath, eTag, downloadProgressCallback, ct);
        request.PerformAsync()->Then([=] (HttpResponse& response)
            {
            *finalResult = ResolveFileDownloadResponse(response, filePath);
            });
        })
            ->Then<WSFileResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
HttpRequest WebApiV2::CreateFileDownloadRequest
(
Utf8StringCR url,
BeFileNameCR filePath,
Utf8StringCR eTag,
HttpRequest::ProgressCallbackCR onProgress,
ICancellationTokenPtr ct
) const
    {
    HttpRequest request = m_configuration->GetHttpClient().CreateGetRequest(url);
    request.SetResponseBody(HttpFileBody::Create(filePath));
    request.SetRetryOptions(HttpRequest::ResumeTransfer, 0);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::FileDownload);
    request.SetDownloadProgressCallback(onProgress);
    request.SetCancellationToken(ct);
    request.GetHeaders().SetIfNoneMatch(eTag);
    return request;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSFileResult WebApiV2::ResolveFileDownloadResponse(HttpResponse& response, BeFileName filePath) const
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetSchemasRequest
(
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    Utf8String url = GetUrl(CreateClassSubPath("MetaSchema", "ECSchemaDef"));
    HttpRequest request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    request.GetHeaders().SetIfNoneMatch(eTag);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendQueryRequest
(
WSQueryCR query,
Utf8StringCR eTag,
Utf8StringCR skipToken,
ICancellationTokenPtr ct
) const
    {
    Utf8String classes = StringUtils::Join(query.GetClasses().begin(), query.GetClasses().end(), ",");
    Utf8String url = GetUrl(CreateClassSubPath(query.GetSchemaName(), classes), query.ToQueryString());
    HttpRequest request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    bool requestHasSkipToken = false;
    if (GetMaxWebApiVersion() >= BeVersion(2, 5) && !skipToken.empty())
        {
        request.GetHeaders().SetValue(HEADER_SkipToken, skipToken);
        requestHasSkipToken = true;
        }

    request.GetHeaders().SetIfNoneMatch(eTag);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this, requestHasSkipToken] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse(httpResponse, requestHasSkipToken);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSChangesetResult> WebApiV2::SendChangesetRequest
(
HttpBodyPtr changeset,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (GetMaxWebApiVersion() < BeVersion(2, 1))
        {
        return CreateCompletedAsyncTask(WSChangesetResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    Utf8String url = GetUrl("$changeset");
    HttpRequest request = m_configuration->GetHttpClient().CreatePostRequest(url);

    request.GetHeaders().SetContentType("application/json");

    request.SetRequestBody(changeset);
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSChangesetResult>([this] (HttpResponse& response)
        {
        if (HttpStatus::OK != response.GetHttpStatus())
            {
            return WSChangesetResult::Error(response);
            }
        return WSChangesetResult::Success(response.GetContent()->GetBody());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSCreateObjectResult> WebApiV2::SendCreateObjectRequest
(
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return SendCreateObjectRequest(ObjectId(), objectCreationJson, filePath, uploadProgressCallback, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSCreateObjectResult> WebApiV2::SendCreateObjectRequest
(
ObjectIdCR relatedObjectId,
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    Utf8String url;
    if (relatedObjectId.IsValid())
        {
        Utf8String schemaName = relatedObjectId.schemaName;
        Utf8String className = relatedObjectId.className;
        Utf8String instanceId = relatedObjectId.remoteId;

        url = GetUrl(CreateClassSubPath(schemaName, className));
        if (!instanceId.empty())
            url += "/" + instanceId;

        auto createdClassName = objectCreationJson["instance"]["className"];

        auto createdClassSchema = objectCreationJson["instance"]["schemaName"];

        if (createdClassName.empty() || createdClassSchema.empty())
            {
            BeAssert(false && "Invalid object creation JSON: no class name or schema name defined");
            return CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));
            }

        url += "/";

        if (schemaName.compare(createdClassSchema.asCString()) != 0 )
            url += createdClassSchema.asString() + ".";

        url += createdClassName.asString();
        }
    else
        {
        Utf8String schemaName = objectCreationJson["instance"]["schemaName"].asString();
        Utf8String className = objectCreationJson["instance"]["className"].asString();
        Utf8String instanceId = objectCreationJson["instance"]["instanceId"].asString();

        url = GetUrl(CreateClassSubPath(schemaName, className));
        if (!instanceId.empty())
            url += "/" + instanceId;
        }
    ChunkedUploadRequest request("POST", url, m_configuration->GetHttpClient());

    request.SetHandshakeRequestBody(HttpStringBody::Create(Json::FastWriter().write(objectCreationJson)), "application/json");
    if (!filePath.empty())
        {
        request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));
        }
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSCreateObjectResult>([this] (HttpResponse& httpResponse)
        {
        return ResolveCreateObjectResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSUpdateObjectResult> WebApiV2::SendUpdateObjectRequest
(
ObjectIdCR objectId,
JsonValueCR propertiesJson,
Utf8String eTag,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (!filePath.empty() && GetMaxWebApiVersion() < BeVersion(2, 4))
        {
        BeAssert(false && "SendUpdateObjectRequest() supports file upload from WebApi 2.4 only. Update server or use seperate file upload");
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    ChunkedUploadRequest request("POST", url, m_configuration->GetHttpClient());

    // WSG 2.x does not support instance validation in update request
    // TODO: implement WSG side or Client side validation
    //if (!eTag.empty ())
    //    {
    //    request.GetHeaders ().SetIfMatch (eTag);
    //    }

    Json::Value updateJson;
    JsonValueR instanceJson = updateJson["instance"];
    instanceJson["schemaName"] = objectId.schemaName;
    instanceJson["className"] = objectId.className;
    instanceJson["instanceId"] = objectId.remoteId;
    instanceJson["properties"] = propertiesJson;

    request.SetHandshakeRequestBody(HttpStringBody::Create(Json::FastWriter().write(updateJson)), "application/json");
    if (!filePath.empty())
        {
        request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));
        }
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSUpdateObjectResult>([this] (HttpResponse& httpResponse)
        {
        return ResolveUpdateObjectResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSDeleteObjectResult> WebApiV2::SendDeleteObjectRequest
(
ObjectIdCR objectId,
ICancellationTokenPtr ct
) const
    {
    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    HttpRequest request = m_configuration->GetHttpClient().CreateRequest(url, "DELETE");

    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSDeleteObjectResult>([] (HttpResponse& httpResponse)
        {
        if (HttpStatus::OK == httpResponse.GetHttpStatus())
            {
            return WSDeleteObjectResult::Success();
            }
        return WSDeleteObjectResult::Error(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSUpdateFileResult> WebApiV2::SendUpdateFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    bool isExternalFileAccessSupported = GetMaxWebApiVersion() >= BeVersion(2, 4);

    BeFile beFile;
    beFile.Open(filePath, BeFileAccess::Read);

    Utf8String url = GetUrl(CreateFileSubPath(objectId));
    ChunkedUploadRequest request("PUT", url, m_configuration->GetHttpClient());

    request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    if (isExternalFileAccessSupported)
        {
        request.GetHandshakeRequest().GetHeaders().SetValue(HEADER_MasAllowRedirect, VALUE_True);
        request.GetHandshakeRequest().SetFollowRedirects(false);
        }

    auto finalResult = std::make_shared<WSUpdateFileResult>();
    return request.PerformAsync()->Then([=] (HttpResponse& response)
        {
        if (HttpStatus::OK == response.GetHttpStatus())
            {
            finalResult->SetSuccess(ResolveUploadResponse(response));
            return;
            }

        if (HttpStatus::TemporaryRedirect != response.GetHttpStatus())
            {
            finalResult->SetError(response);
            return;
            }

        Utf8String redirectUrl = response.GetHeaders().GetLocation();
        Utf8String redirectType = response.GetHeaders().GetValue(HEADER_MasFileAccessUrlType);
        Utf8String confirmationId = response.GetHeaders().GetValue(HEADER_MasUploadConfirmationId);

        if (redirectType != VALUE_FileAccessUrlType_Azure)
            {
            LOG.errorv("Header field '%s' contains not supported value: '%s'", HEADER_MasFileAccessUrlType, redirectType.c_str());
            finalResult->SetError(WSError::CreateServerNotSupportedError());
            return;
            }

        m_azureClient->SendUpdateFileRequest(redirectUrl, filePath, uploadProgressCallback, ct)->Then([=] (AzureResult azureResult)
            {
            if (!azureResult.IsSuccess())
                {
                finalResult->SetError(azureResult.GetError());
                return;
                }
                
            finalResult->SetSuccess({nullptr, azureResult.GetValue().GetETag()});
            if (confirmationId.empty())
                return;

            HttpRequest request = m_configuration->GetHttpClient().CreateRequest(url, "PUT");
            request.GetHeaders().SetValue(HEADER_MasUploadConfirmationId, confirmationId);
            request.PerformAsync()->Then([=] (HttpResponse& response)
                {
                if (HttpStatus::OK != response.GetHttpStatus())
                    finalResult->SetError(response);
                });
            });
        })
            ->Then<WSUpdateFileResult>([=]
            {
            return *finalResult;
            });
    }
