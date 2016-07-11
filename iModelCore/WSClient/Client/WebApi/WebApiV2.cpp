/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV2.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApiV2.h"
#include <WebServices/Client/Response/WSObjectsReaderV2.h>

#define HEADER_SkipToken                "SkipToken"
#define HEADER_MasAllowRedirect         "Mas-Allow-Redirect"
#define HEADER_MasFileAccessUrlType     "Mas-File-Access-Url-Type"
#define HEADER_MasUploadConfirmationId  "Mas-Upload-Confirmation-Id"

#define VALUE_FileAccessUrlType_Azure   "AzureBlobSasUrl"
#define VALUE_True                      "true"

const BeVersion WebApiV2::s_maxTestedWebApi(2, 4);

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
Utf8String WebApiV2::GetWebApiUrl(BeVersion webApiVersion) const
    {
    BeVersion webApiVersionToUse = m_info.GetWebApiVersion();
    if (!webApiVersion.IsEmpty())
        {
        webApiVersionToUse = webApiVersion;
        }
    else if (webApiVersionToUse > s_maxTestedWebApi)
        {
        webApiVersionToUse = s_maxTestedWebApi; // Limit queries to tested WebApi version
        }

    Utf8PrintfString url
        (
        "%s/v%d.%d/",
        m_configuration->GetServerUrl().c_str(),
        webApiVersionToUse.GetMajor(),
        webApiVersionToUse.GetMinor()
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
WSRepositoriesResult WebApiV2::ResolveGetRepositoriesResponse(Http::Response& response) const
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
WSCreateObjectResult WebApiV2::ResolveCreateObjectResponse(Http::Response& response) const
    {
    if (HttpStatus::Created == response.GetHttpStatus())
        {
        return WSCreateObjectResult::Success(response.GetBody().AsJson());
        }
    return WSCreateObjectResult::Error(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSUpdateObjectResult WebApiV2::ResolveUpdateObjectResponse(Http::Response& response) const
    {
    if (HttpStatus::OK == response.GetHttpStatus())
        {
        return WSUpdateObjectResult::Success();
        }
    return WSUpdateObjectResult::Error(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSObjectsResult WebApiV2::ResolveObjectsResponse(Http::Response& response, const ObjectId* objectId) const
    {
    HttpStatus status = response.GetHttpStatus();
    if (HttpStatus::OK == status ||
        HttpStatus::NotModified == status)
        {
        auto reader = CreateJsonInstancesReader();

        auto body = response.GetContent()->GetBody();
        auto eTag = response.GetHeaders().GetETag();
        auto skipToken = response.GetHeaders().GetValue(HEADER_SkipToken);

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
    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    request.SetRetryOptions(Http::Request::RetryOption::ResetTransfer, 1);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObject);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this, objectId] (Http::Response& httpResponse)
        {
        return ResolveObjectsResponse(httpResponse, &objectId);
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
    BeVersion webApiVersion;
    bool isExternalFileAccessSupported = m_info.GetWebApiVersion() >= BeVersion(2, 4);
    if (isExternalFileAccessSupported)
        webApiVersion = BeVersion(2, 4);

    Utf8String url = GetUrl(CreateFileSubPath(objectId), "", webApiVersion);
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
    Utf8String url = GetUrl(CreateClassSubPath(query.GetSchemaName(), classes), query.ToQueryString());
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    request.GetHeaders().SetIfNoneMatch(eTag);
    request.GetHeaders().SetValue(HEADER_SkipToken, skipToken);
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
AsyncTaskPtr<WSChangesetResult> WebApiV2::SendChangesetRequest
(
HttpBodyPtr changeset,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (m_info.GetWebApiVersion() < BeVersion(2, 1))
        {
        return CreateCompletedAsyncTask(WSChangesetResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    Utf8String url = GetUrl("$changeset");
    Http::Request request = m_configuration->GetHttpClient().CreatePostRequest(url);

    request.GetHeaders().SetContentType("application/json");

    request.SetRequestBody(changeset);
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSChangesetResult>([this] (Http::Response& response)
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
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    ObjectId objectId;
    objectId.schemaName = objectCreationJson["instance"]["schemaName"].asString();
    objectId.className = objectCreationJson["instance"]["className"].asString();
    objectId.remoteId = objectCreationJson["instance"]["instanceId"].asString();

    return SendCreateObjectRequest(objectId, objectCreationJson, filePath, uploadProgressCallback, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSCreateObjectResult> WebApiV2::SendCreateObjectRequest
(
ObjectIdCR objectId,
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (objectId.schemaName.empty() || objectId.className.empty())
        {
        BeAssert(false && "Either schemaName or className passed into WebApiV2::SendCreateObjectRequest is empty. Both are required to be valid.");
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError()));
        }

    BeAssert(objectId.schemaName.Equals(objectCreationJson["instance"]["schemaName"].asString()));
    BeAssert(objectId.className.Equals(objectCreationJson["instance"]["className"].asString()));

    Utf8String url = GetUrl(CreateClassSubPath(objectId.schemaName, objectId.className));

    if (!objectId.remoteId.empty())
        {
        url += "/" + objectId.remoteId;
        }

    ChunkedUploadRequest request("POST", url, m_configuration->GetHttpClient());

    request.SetHandshakeRequestBody(HttpStringBody::Create(Json::FastWriter().write(objectCreationJson)), "application/json");
    if (!filePath.empty())
        {
        request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));
        }
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSCreateObjectResult>([this] (Http::Response& httpResponse)
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
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    Http::Request request = m_configuration->GetHttpClient().CreatePostRequest(url);

    request.GetHeaders().SetContentType("application/json");

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

    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(updateJson)));
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return request.PerformAsync()->Then<WSUpdateObjectResult>([this] (Http::Response& httpResponse)
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
    Http::Request request = m_configuration->GetHttpClient().CreateRequest(url, "DELETE");

    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSDeleteObjectResult>([] (Http::Response& httpResponse)
        {
        if (HttpStatus::OK == httpResponse.GetHttpStatus())
            {
            return WSUpdateObjectResult::Success();
            }
        return WSUpdateObjectResult::Error(httpResponse);
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
ICancellationTokenPtr ct
) const
    {
    BeVersion webApiVersion;
    bool isExternalFileAccessSupported = m_info.GetWebApiVersion() >= BeVersion(2, 4);
    if (isExternalFileAccessSupported)
        webApiVersion = BeVersion(2, 4);

    BeFile beFile;
    beFile.Open(filePath, BeFileAccess::Read);

    Utf8String url = GetUrl(CreateFileSubPath(objectId), "", webApiVersion);
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
    return request.PerformAsync()->Then([=] (Http::Response& response)
        {
        if (HttpStatus::OK == response.GetHttpStatus())
            {
            finalResult->SetSuccess();
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

        m_azureClient->SendUpdateFileRequest(redirectUrl, filePath, uploadProgressCallback, ct)->Then([=] (AzureResult result)
            {
            if (!result.IsSuccess())
                {
                finalResult->SetError(result.GetError());
                return;
                }

            if (confirmationId.empty())
                {
                finalResult->SetSuccess();
                return;
                }

            Http::Request request = m_configuration->GetHttpClient().CreateRequest(url, "PUT");
            request.GetHeaders().SetValue(HEADER_MasUploadConfirmationId, confirmationId);
            request.PerformAsync()->Then([=] (Http::Response& response)
                {
                if (HttpStatus::OK != response.GetHttpStatus())
                    {
                    finalResult->SetError(response);
                    return;
                    }
                finalResult->SetSuccess();
                });
            });
        })
            ->Then<WSUpdateFileResult>([=]
            {
            return *finalResult;
            });
    }
