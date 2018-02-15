/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV2.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

#define WARNING_UrlLengthLimitations    "<Warning> Url length might be problematic as it is longer than expected"

const BeVersion WebApiV2::s_maxTestedWebApi(2, 5);

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WebApiV2::WebApiV2(std::shared_ptr<const ClientConfiguration> configuration, WSInfo info) :
WebApi(configuration),
m_info(info),
m_azureClient(AzureBlobStorageClient::Create(configuration->GetHttpHandler())),
m_jobApi(JobApi::Create(configuration))
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
    return GetWebApiUrl(webApiVersion) + "Repositories/" + BeUri::EscapeString(repositoryId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetUrl(Utf8StringCR path, Utf8StringCR queryString, BeVersion webApiVersion) const
    {
    Utf8String url = GetUrlWithoutLengthWarning(path, queryString, webApiVersion);

    BeAssert(url.size() < m_configuration->GetMaxUrlLength() && WARNING_UrlLengthLimitations);
    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetUrlWithoutLengthWarning(Utf8StringCR path, Utf8StringCR queryString, BeVersion webApiVersion) const
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
Utf8String WebApiV2::CreatePostQueryPath(Utf8StringCR classSubPath) const
    {
    return classSubPath + "/$query";
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
WSRepositoriesResult WebApiV2::ResolveGetRepositoriesResponse(Http::Response& response) const
    {
    if (HttpStatus::OK != response.GetHttpStatus())
        {
        return WSRepositoriesResult::Error(response);
        }

    auto repositoriesJson = std::make_shared<rapidjson::Document>();
    repositoriesJson->Parse<0>(response.GetBody().AsString().c_str());

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
        repository.SetServerUrl(m_configuration->GetServerUrl().c_str());

        repositories.push_back(repository);
        }

    return WSRepositoriesResult::Success(repositories);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSUpdateObjectResult WebApiV2::ResolveUpdateObjectResponse(Http::Response& response) const
    {
    if (HttpStatus::OK == response.GetHttpStatus())
        return WSUpdateObjectResult::Success(ResolveUploadResponse(response));
    return WSUpdateObjectResult::Error(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSUploadResponse WebApiV2::ResolveUploadResponse(Http::Response& response) const
    {
    auto body = response.GetContent()->GetBody();
    auto eTag = response.GetHeaders().GetValue(HEADER_MasFileETag);
    return WSUploadResponse(body, eTag);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSObjectsResult WebApiV2::ResolveObjectsResponse(Http::Response& response, bool requestHadSkipToken, const ObjectId* objectId) const
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
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    request.SetCancellationToken(ct);
    return request.PerformAsync()->Then<WSRepositoriesResult>([this] (Http::Response& httpResponse)
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
    if (!objectId.IsValid())
        return CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::CreateFunctionalityNotSupportedError()));

    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    request.SetRetryOptions(Http::Request::RetryOption::ResetTransfer, 1);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObject);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this, objectId] (Http::Response& httpResponse)
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
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this] (Http::Response& httpResponse)
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
Http::Request::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (!objectId.IsValid() || filePath.empty())
        return CreateCompletedAsyncTask(WSFileResult::Error(WSError::CreateFunctionalityNotSupportedError()));

    bool isExternalFileAccessSupported = GetMaxWebApiVersion() >= BeVersion(2, 4);

    Utf8String url = GetUrl(CreateFileSubPath(objectId));
    Http::Request request = CreateFileDownloadRequest(url, filePath, eTag, downloadProgressCallback, ct);

    if (isExternalFileAccessSupported)
        {
        request.GetHeaders().SetValue(HEADER_MasAllowRedirect, VALUE_True);
        request.SetFollowRedirects(false);
        }

    auto finalResult = std::make_shared<WSFileResult>();
    return request.PerformAsync()->Then([=] (Http::Response& response)
        {
        if (HttpStatus::TemporaryRedirect != response.GetHttpStatus())
            {
            *finalResult = ResolveFileDownloadResponse(response, filePath);
            return;
            }

        Utf8String redirectUrl = response.GetHeaders().GetLocation();
        Http::Request request = CreateFileDownloadRequest(redirectUrl, filePath, eTag, downloadProgressCallback, ct);
        request.PerformAsync()->Then([=] (Http::Response& response)
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
Http::Request WebApiV2::CreateFileDownloadRequest
(
Utf8StringCR url,
BeFileNameCR filePath,
Utf8StringCR eTag,
Http::Request::ProgressCallbackCR onProgress,
ICancellationTokenPtr ct
) const
    {
    Http::Request request = m_configuration->GetHttpClient().CreateGetRequest(url);
    request.SetResponseBody(HttpFileBody::Create(filePath));
    request.SetRetryOptions(Http::Request::RetryOption::ResumeTransfer, 0);
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
WSFileResult WebApiV2::ResolveFileDownloadResponse(Http::Response& response, BeFileName filePath) const
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
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    request.GetHeaders().SetIfNoneMatch(eTag);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this] (Http::Response& httpResponse)
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
    Utf8String url = GetUrlWithoutLengthWarning(CreateClassSubPath(query.GetSchemaName(), classes), query.ToQueryString());
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    if (m_configuration->GetMaxUrlLength() < url.size())
        {
        if (m_info.GetWebApiVersion() >= BeVersion(2, 4))
            {
            url = GetUrl(CreatePostQueryPath(CreateClassSubPath(query.GetSchemaName(), classes)), "");
            request = m_configuration->GetHttpClient().CreatePostRequest(url);
            request.SetRequestBody(HttpStringBody::Create(query.ToQueryString()));
            request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);
            }
        else
            {
            BeAssert(true && WARNING_UrlLengthLimitations);
            }
        }
        
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

    return request.PerformAsync()->Then<WSObjectsResult>([this, requestHasSkipToken] (Http::Response& httpResponse)
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
Http::Request::ProgressCallbackCR uploadProgressCallback,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    if (GetMaxWebApiVersion() < BeVersion(2, 1))
        return CreateCompletedAsyncTask(WSChangesetResult::Error(WSError::CreateFunctionalityNotSupportedError()));

    Utf8String url = GetUrl("$changeset");
    Http::Request request = m_configuration->GetHttpClient().CreatePostRequest(url);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::Default);

    if (nullptr != options)
        {
        request.SetTransferTimeoutSeconds(options->GetTransferTimeOut());
        }

    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);

    request.SetRequestBody(changeset);
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return m_jobApi->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then<WSChangesetResult>([=] (HttpJobResult& response)
        {
        if (!response.IsSuccess())
            return WSChangesetResult::Error(response.GetError());

        auto httpResponse = response.GetValue();
        if (HttpStatus::OK != httpResponse.GetHttpStatus())
            {
            return WSChangesetResult::Error(httpResponse);
            }
        return WSChangesetResult::Success(httpResponse.GetContent()->GetBody());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSCreateObjectResult> WebApiV2::SendCreateObjectRequest
(
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return SendCreateObjectRequest(ObjectId(), objectCreationJson, filePath, uploadProgressCallback, options, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSCreateObjectResult> WebApiV2::SendCreateObjectRequest
(
ObjectIdCR relatedObjectId,
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    Utf8String url;
    if (!relatedObjectId.IsEmpty())
        {
        if (!relatedObjectId.IsValid())
            {
            BeAssert(false && "Invalid relatedObjectId");
            return CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));
            }

        Utf8String className = objectCreationJson["instance"]["className"].asString();
        Utf8String schemaName = objectCreationJson["instance"]["schemaName"].asString();

        if (className.empty() || schemaName.empty())
            {
            BeAssert(false && "Invalid object creation JSON: no class name or schema name defined");
            return CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));
            }

        url = GetUrl(CreateObjectSubPath(relatedObjectId));
        url += "/";

        if (relatedObjectId.schemaName != schemaName != 0)
            url += schemaName + ".";

        url += className;
        }
    else
        {
        ObjectId objectId;
        Utf8String schemaName = objectCreationJson["instance"]["schemaName"].asString();
        Utf8String className = objectCreationJson["instance"]["className"].asString();
        Utf8String instanceId = objectCreationJson["instance"]["instanceId"].asString();

        url = GetUrl(CreateClassSubPath(schemaName, className));
        if (!instanceId.empty() && objectCreationJson["instance"]["changeState"].asString() != "new")
            url += "/" + instanceId;
        }

    ChunkedUploadRequest request("POST", url, m_configuration->GetHttpClient());

    request.SetHandshakeRequestBody(HttpStringBody::Create(Json::FastWriter().write(objectCreationJson)), "application/json");
    if (!filePath.empty())
        request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));

    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);
    return m_jobApi->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then<WSCreateObjectResult>([=] (HttpJobResult& response)
        {
        if (!response.IsSuccess())
            return WSCreateObjectResult::Error(response.GetError());

        auto httpResponse = response.GetValue();
        if (HttpStatus::Created != httpResponse.GetHttpStatus() && HttpStatus::OK != httpResponse.GetHttpStatus())
            return WSCreateObjectResult::Error(httpResponse);
        return WSCreateObjectResult::Success(ResolveUploadResponse(httpResponse));
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSUpdateObjectResult> WebApiV2::SendUpdateObjectRequest
(
ObjectIdCR objectId,
JsonValueCR propertiesJson,
Utf8StringCR eTag,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    if (!objectId.IsValid())
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));

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

    request.SetHandshakeRequestBody(HttpStringBody::Create(Json::FastWriter().write(updateJson)), REQUESTHEADER_ContentType_ApplicationJson);
    if (!filePath.empty())
        request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));

    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return m_jobApi->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then<WSUpdateObjectResult>([=] (HttpJobResult& response)
        {
        if (!response.IsSuccess())
            return WSUpdateObjectResult::Error(response.GetError());

        return ResolveUpdateObjectResponse(response.GetValue());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSDeleteObjectResult> WebApiV2::SendDeleteObjectRequest
(
ObjectIdCR objectId,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    if (!objectId.IsValid())
        return CreateCompletedAsyncTask(WSDeleteObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));

    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    Http::Request request = m_configuration->GetHttpClient().CreateRequest(url, "DELETE");
    request.SetCancellationToken(ct);

    return m_jobApi->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then<WSDeleteObjectResult>([=] (HttpJobResult& response)
        {
        if (!response.IsSuccess())
            return WSDeleteObjectResult::Error(response.GetError());

        auto httpResponse = response.GetValue();
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
Http::Request::ProgressCallbackCR uploadProgressCallback,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    if (!objectId.IsValid())
        return CreateCompletedAsyncTask(WSUpdateFileResult::Error(WSError::CreateFunctionalityNotSupportedError()));

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
    return m_jobApi
        ->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then([=] (HttpJobResult& response)
        {
        if (!response.IsSuccess())
            {
            finalResult->SetError(response.GetError());
            return;
            }

        auto httpResponse = response.GetValue();
        ResolveUpdateFileResponse(httpResponse, url, filePath, uploadProgressCallback, ct)
            ->Then([=] (WSUpdateFileResult result)
            {
            *finalResult = result;
            });

        })->Then<WSUpdateFileResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSUpdateFileResult> WebApiV2::ResolveUpdateFileResponse
(
Http::Response& httpResponse,
Utf8StringCR url,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (HttpStatus::OK == httpResponse.GetHttpStatus())
        {
        return CreateCompletedAsyncTask(WSCreateObjectResult::Success(ResolveUploadResponse(httpResponse)));
        }

    if (HttpStatus::TemporaryRedirect != httpResponse.GetHttpStatus())
        {
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(httpResponse));
        }

    Utf8String redirectUrl = httpResponse.GetHeaders().GetLocation();
    Utf8String redirectType = httpResponse.GetHeaders().GetValue(HEADER_MasFileAccessUrlType);
    Utf8String confirmationId = httpResponse.GetHeaders().GetValue(HEADER_MasUploadConfirmationId);

    if (redirectType != VALUE_FileAccessUrlType_Azure)
        {
        LOG.errorv("Header field '%s' contains not supported value: '%s'", HEADER_MasFileAccessUrlType, redirectType.c_str());
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError::CreateServerNotSupportedError()));
        }

    auto finalResult = std::make_shared<WSUpdateFileResult>();
    return m_azureClient->SendUpdateFileRequest(redirectUrl, filePath, uploadProgressCallback, ct)->Then([=] (AzureResult azureResult)
        {
        if (!azureResult.IsSuccess())
            {
            finalResult->SetError(azureResult.GetError());
            return;
            }

        finalResult->SetSuccess({nullptr, azureResult.GetValue().GetETag()});
        if (confirmationId.empty())
            return;

            Http::Request request = m_configuration->GetHttpClient().CreateRequest(url, "PUT");
            request.GetHeaders().SetValue(HEADER_MasUploadConfirmationId, confirmationId);
            request.PerformAsync()->Then([=] (Http::Response& response)
                {
                if (HttpStatus::OK != response.GetHttpStatus())
                    finalResult->SetError(response);
                });
        })
            ->Then<WSUpdateFileResult>([=]
            {
            return *finalResult;
            });
    }
