/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApiV2.h"
#include <WebServices/Client/Response/WSObjectsReaderV2.h>
#include <BeHttp/ProxyHttpHandler.h>

#define HEADER_SkipToken                   "SkipToken"
#define HEADER_MasAllowRedirect            "Mas-Allow-Redirect"
#define HEADER_MasFileAccessUrlType        "Mas-File-Access-Url-Type"
#define HEADER_MasUploadConfirmationId     "Mas-Upload-Confirmation-Id"
#define HEADER_MasFileETag                 "Mas-File-ETag"
#define HEADER_MasServerHeader             "Mas-Server"

#define VALUE_FileAccessUrlType_Azure      "AzureBlobSasUrl"
#define VALUE_True                         "true"

#define WARNING_UrlLengthLimitations       "<Warning> Url length might be problematic as it is longer than expected"

#define SCHEMA_Policies                    "Policies"
#define CLASS_PolicyAssertion              "PolicyAssertion"
#define INSTANCE_PersistenceFileBackable   "Persistence.FileBackable"
#define INSTANCE_PersistenceStreamBackable "Persistence.StreamBackable"
#define PROPERTY_AdhocProperties           "AdhocProperties"

const BeVersion WebApiV2::s_maxTestedWebApi(2, 8);

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
WebApiV2::~WebApiV2() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool WebApiV2::IsSupported(WSInfoCR info)
    {
    if (info.GetWebApiVersion() < BeVersion(2, 0))
        return false;

    if (!info.GetServiceVersion().IsEmpty() && info.GetWebApiVersion() > s_maxTestedWebApi)
        return false;

    if (info.GetWebApiVersion() > BeVersion(3, 0))
        return false;

    if (info.GetType() != WSInfo::Type::BentleyWSG)
        return false;

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
uint64_t WebApiV2::GetMaxUploadSize(Http::Response& response, ActivityLoggerR activityLogger, uint64_t defaultMaxUploadSize) const
    {
    if (!m_info.IsMaxUploadSizeSupported())
        return defaultMaxUploadSize;

    auto policiesJson = std::make_shared<rapidjson::Document>();
    policiesJson->Parse<0>(response.GetBody().AsString().c_str());

    auto reader = CreateJsonInstancesReader();
    WSObjectsReader::Instances instances = reader->ReadInstances(policiesJson);

    if (instances.IsEmpty())
        {
        activityLogger.error("GetMaxUploadSize: Response was empty");
        return defaultMaxUploadSize;
        }

    WSObjectsReader::Instance instance = reader->GetInstance(0);
    if (!instance.IsValid() ||
        !instance.GetObjectId().schemaName.Equals(SCHEMA_Policies) ||
        !instance.GetObjectId().className.Equals(CLASS_PolicyAssertion))
        {
        activityLogger.errorv("GetMaxUploadSize: Expected '%s' schema and '%s' class. Actually it was: '%s' schema and '%s' class", SCHEMA_Policies, CLASS_PolicyAssertion,
                   instance.GetObjectId().schemaName.c_str(), instance.GetObjectId().className.c_str());
        return defaultMaxUploadSize;
        }

    if(!instance.GetObjectId().GetRemoteId().Equals(INSTANCE_PersistenceFileBackable) &&
       !instance.GetObjectId().GetRemoteId().Equals(INSTANCE_PersistenceStreamBackable))
        {
        activityLogger.errorv("GetMaxUploadSize: InstanceId was expected to be '%s' or '%s'. Actually it was: '%s'", INSTANCE_PersistenceFileBackable,
                   INSTANCE_PersistenceStreamBackable, instance.GetObjectId().GetRemoteId().c_str());
        return defaultMaxUploadSize;
        }

    if (!instance.GetProperties().HasMember(PROPERTY_AdhocProperties) ||
        !instance.GetProperties()[PROPERTY_AdhocProperties].IsArray())
        {
        activityLogger.error("GetMaxUploadSize: AdhocProperties did not exist in the response");
        return defaultMaxUploadSize;
        }

    auto adhocProperties = instance.GetProperties()[PROPERTY_AdhocProperties].GetArray();

    for (auto itr = adhocProperties.Begin(); itr != adhocProperties.End(); ++itr)
        {
        auto adhocPropertyName = GetNullableString((*itr), "Name");
        if (adhocPropertyName.Equals("MaxUploadSize"))
            return BeRapidJsonUtilities::UInt64FromValue((*itr)["Value"], defaultMaxUploadSize);
        }

    activityLogger.error("GetMaxUploadSize: MaxUploadSize property did not exist in the response");
    return defaultMaxUploadSize;
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
Utf8String WebApiV2::GetVersionedUrl() const
    {
    Utf8String version;
    if (!m_configuration->GetServiceVersion().IsEmpty())
        {
        version = "sv" + m_configuration->GetServiceVersion().ToMajorMinorString();
        }
    else
        {
        version = "v" + GetMaxWebApiVersion().ToMajorMinorString();
        }

    Utf8PrintfString url
        (
        "%s/%s/",
        m_configuration->GetServerUrl().c_str(),
        version.c_str()
        );

    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetRepositoryUrl(Utf8StringCR repositoryId) const
    {
    return GetVersionedUrl() + "Repositories/" + BeUri::EscapeString(repositoryId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetUrl(Utf8StringCR path, Utf8StringCR queryString) const
    {
    Utf8String url = GetUrlWithoutLengthWarning(path, queryString);

    BeAssert(url.size() < m_configuration->GetMaxUrlLength() && WARNING_UrlLengthLimitations);
    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetUrlWithoutLengthWarning(Utf8StringCR path, Utf8StringCR queryString) const
    {
    Utf8String url = GetRepositoryUrl(m_configuration->GetRepositoryId());

    if (!path.empty())
        url += "/" + path;

    if (!queryString.empty())
        url += "?" + queryString;

    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateObjectSubPath(ObjectIdCR objectId) const
    {
    Utf8String param;
    if (!objectId.IsEmpty())
        param.Sprintf("%s/%s/%s", objectId.schemaName.c_str(), objectId.className.c_str(), objectId.remoteId.c_str());

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
        return "Navigation/NavNode";

    return CreateObjectSubPath(parentId) + "/NavNode";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ActivityLogger WebApiV2::CreateActivityLogger(Utf8StringCR activityName, IWSRepositoryClient::RequestOptionsPtr options) const
    {
    if (m_info.GetWebApiVersion() < BeVersion(2, 7))
        {
        auto activityLogger = ActivityLogger(LOG, activityName);
        if (nullptr != options && options->GetActivityOptions()->HasActivityId())
            activityLogger.warning("Specified activity id will be ignored, because it's supported from WebApi 2.7 only");
        return activityLogger;
        }

    Utf8String activityId;
    if (nullptr != options && options->GetActivityOptions()->HasActivityId())
        {
        activityId = options->GetActivityOptions()->GetActivityId();
        }
    else
        {
        activityId = m_configuration->GetActivityIdGenerator().GenerateNextId();
        }

    auto headerName = IWSRepositoryClient::ActivityOptions::HeaderName::Default;
    if (nullptr != options)
        headerName = options->GetActivityOptions()->GetHeaderName();

    Utf8String headerNameString = IWSRepositoryClient::ActivityOptions::HeaderNameToString(headerName);

    return ActivityLogger(LOG, activityName, headerNameString, activityId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void WebApiV2::SetActivityIdToRequest(ActivityLoggerR activityLogger, Http::RequestR request) const
    {
    if (!activityLogger.HasValidActivityInfo())
        return;

    request.GetHeaders().AddValue(activityLogger.GetHeaderName(), activityLogger.GetActivityId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void WebApiV2::SetActivityIdToRequest(ActivityLoggerR activityLogger, ChunkedUploadRequestR request) const
    {
    if (!activityLogger.HasValidActivityInfo())
        return;

    request.GetRequestsHeaders().AddValue(activityLogger.GetHeaderName(), activityLogger.GetActivityId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                  Simonas.Mulevicius 08/2019
+--------------------------------------------------------------------------------------*/
void WebApiV2::SetActivityIdToWSResponse(WSResponseR response, Utf8StringCR activityId) const
    {
    response.SetActivityId(activityId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    simonas.mulevicius
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetResponseActivityId(Http::Response& response, ActivityLoggerR activityLogger) const
    {
    if (!activityLogger.HasValidActivityInfo())
        return "";

    Utf8String actualActivityHeaderName = activityLogger.GetHeaderName();
    Utf8String actualActivityId = activityLogger.GetActivityId();
    Utf8String responseActivityId = response.GetHeaders().GetValue(actualActivityHeaderName);

    Utf8CP idsMismatchMessagePrefix = "Response activity IDs do not match";

    if (responseActivityId.empty())
        {
        activityLogger.warningv("%s: response ActivityId is empty.", idsMismatchMessagePrefix);
        return "";
        }

    if (!actualActivityId.Equals(responseActivityId)) 
        activityLogger.warningv("%s: response ActivityId is %s.", idsMismatchMessagePrefix, responseActivityId.c_str());

    return responseActivityId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateSelectPropertiesQuery(const bset<Utf8String>& properties) const
    {
    Utf8String value = StringUtils::Join(properties.begin(), properties.end(), ",");
    Utf8String query;
    if (!value.empty())
        query.Sprintf("$select=%s", value.c_str());

    return query;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Http::Request WebApiV2::CreateGetRepositoryRequest() const
    {
    WSQuery query(SCHEMA_Policies, CLASS_PolicyAssertion);
    query.SetFilter("Supported+eq+true");

    std::deque<ObjectId> queryIds;
    queryIds.push_back(ObjectId(SCHEMA_Policies, CLASS_PolicyAssertion, INSTANCE_PersistenceFileBackable));
    queryIds.push_back(ObjectId(SCHEMA_Policies, CLASS_PolicyAssertion, INSTANCE_PersistenceStreamBackable));
    query.AddFilterIdsIn(queryIds);

    query.SetTop(1);
    return CreateQueryRequest(query);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Http::Request WebApiV2::CreateQueryRequest(WSQueryCR query) const
    {
    Utf8String classes = StringUtils::Join(query.GetClasses().begin(), query.GetClasses().end(), ",");
    Utf8String url = GetUrl(CreateClassSubPath(query.GetSchemaName(), classes), query.ToQueryString());
    return m_configuration->GetHttpClient().CreateGetJsonRequest(url);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Simonas.Mulevicius          08/2019
+--------------------------------------------------------------------------------------*/
WSError WebApiV2::CreateError(Http::Response& response, Utf8StringCR activityId) const
    {
    if (Utf8String::IsNullOrEmpty(activityId.c_str()))
        return WSError(response);

    return WSError::CreateErrorUsingActivityId(response, activityId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Simonas.Mulevicius          08/2019
+--------------------------------------------------------------------------------------*/
WSError WebApiV2::CreateServerNotSupportedError(Http::Response& response, Utf8StringCR activityId) const
    {
    if (Utf8String::IsNullOrEmpty(activityId.c_str()))
        return WSError::CreateServerNotSupportedError();

    return WSError::CreateServerNotSupportedErrorWithActivityId(activityId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Simonas.Mulevicius          08/2019
+--------------------------------------------------------------------------------------*/
WSError WebApiV2::CreateErrorFromAzzureError(AzureErrorCR azureError, Utf8StringCR activityId) const
    {
    if (Utf8String::IsNullOrEmpty(activityId.c_str()))
        return WSError(azureError);

    return WSError::CreateErrorUsingActivityId(azureError, activityId);
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
Utf8String WebApiV2::GetNullableString(RapidJsonValueCR object, Utf8CP member) const
    {
    if (!object.HasMember(member))
        return "";

    auto& value = object[member];
    if (value.IsString())
        return value.GetString();

    BeAssert(value.IsNull() && "Should be string or null");
    return "";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Simonas.Mulevicius 08/2019
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetActivityIdFromResponse(Http::Response& response, Utf8StringCR activityHeaderName) const
    {
    return Utf8String(response.GetHeaders().GetValue(activityHeaderName));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSRepositoriesResult WebApiV2::ResolveGetRepositoriesResponse(Http::Response& response, ActivityLoggerCR activityLogger) const
    {
    auto activityId = GetActivityIdFromResponse(response, activityLogger.GetHeaderName());
    if (HttpStatus::OK != response.GetHttpStatus())
        return WSRepositoriesResult::Error(CreateError(response, activityId));

    auto repositoriesJson = std::make_shared<rapidjson::Document>();
    repositoriesJson->Parse<0>(response.GetBody().AsString().c_str());

    auto reader = CreateJsonInstancesReader();
    WSObjectsReader::Instances instances = reader->ReadInstances(repositoriesJson);
    if (!instances.IsValid())
        {
        return WSRepositoriesResult::Error(CreateServerNotSupportedError(response, activityId));
        }

    bvector<WSRepository> repositories;
    for (WSObjectsReader::Instance instance : instances)
        {
        if (!instance.IsValid() ||
            !instance.GetObjectId().schemaName.Equals("Repositories") ||
            !instance.GetObjectId().className.Equals("RepositoryIdentifier"))
            {
            return WSRepositoriesResult::Error(CreateServerNotSupportedError(response, activityId));
            }

        WSRepository repository;

        repository.SetId(instance.GetObjectId().remoteId);
        repository.SetLocation(GetNullableString(instance.GetProperties(), "Location"));
        repository.SetPluginId(GetNullableString(instance.GetProperties(), "ECPluginID"));
        repository.SetLabel(GetNullableString(instance.GetProperties(), "DisplayLabel"));
        repository.SetDescription(GetNullableString(instance.GetProperties(), "Description"));
        repository.SetServerUrl(m_configuration->GetServerUrl().c_str());

        repositories.push_back(repository);
        }

    return WSRepositoriesResult::Success(repositories);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BeVersion WebApiV2::GetRepositoryPluginVersion(Http::Response& response, Utf8StringCR pluginId) const
    {
    Utf8CP serverHeader = response.GetHeaders().GetValue(HEADER_MasServerHeader);
    if (!serverHeader)
        return BeVersion();

    bvector<Utf8String> servers;
    BeStringUtilities::Split(serverHeader, ",", servers);

    for (Utf8String& server : servers)
        {
        server.Trim();
        bvector<Utf8String> values;
        BeStringUtilities::Split(server.c_str(), "/", values);
        if (values[0] != pluginId)
            continue;

        auto format = values[0] + "/%d.%d.%d.%d";
        return BeVersion(server.c_str(), format.c_str());
        }

    return BeVersion();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSUpdateObjectResult WebApiV2::ResolveUpdateObjectResponse(Http::Response& response, ActivityLoggerCR activityLogger) const
    {
    auto activityId = GetActivityIdFromResponse(response, activityLogger.GetHeaderName());
    if (HttpStatus::OK == response.GetHttpStatus())
        {
        auto uploadResponse = ResolveUploadResponse(response);
        SetActivityIdToWSResponse(uploadResponse, activityId);
        return WSUpdateObjectResult::Success(uploadResponse);
        }

    return WSUpdateObjectResult::Error(CreateError(response, activityId));
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
* @bsimethod                                            simonas.mulevicius        08/2019
+--------------------------------------------------------------------------------------*/
WSObjectsResult WebApiV2::ResolveObjectsResponse(Http::Response& response, ActivityLoggerCR activityLogger) const
    {
    auto activityId = GetActivityIdFromResponse(response, activityLogger.GetHeaderName());
    HttpStatus status = response.GetHttpStatus();
    if (HttpStatus::OK == status ||
        HttpStatus::NotModified == status)
        {
        auto reader = CreateJsonInstancesReader();

        auto body = response.GetContent()->GetBody();
        auto eTag = response.GetHeaders().GetETag();
        auto skipToken = response.GetHeaders().GetValue(HEADER_SkipToken);

        auto objectResponse = WSObjectsResponse(reader, body, status, eTag, skipToken);
        SetActivityIdToWSResponse(objectResponse, activityId);
        return WSObjectsResult::Success(objectResponse);
        }

    return WSObjectsResult::Error(CreateError(response, activityId));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSRepositoryResult> WebApiV2::SendGetRepositoryInfoRequest
(
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    auto activityLogger = CreateActivityLogger("Get Repository Info", options);
    activityLogger.debug("Started");

    auto repository = WSRepositoryClient::ParseRepositoryUrl(GetUrl("/"));

    const uint64_t defaultMaxUploadSize = 0;
    repository.SetMaxUploadSize(defaultMaxUploadSize);

    if (!repository.IsValid())
        {
        activityLogger.error("Configured repository is not valid");
        return CreateCompletedAsyncTask(WSRepositoryResult::Error(WSError::CreateServerNotSupportedError()));
        }

    if (GetMaxWebApiVersion() < BeVersion(2, 8))
        return CreateCompletedAsyncTask(WSRepositoryResult::Success(repository));

    Http::Request request = CreateGetRepositoryRequest();
    SetActivityIdToRequest(activityLogger, request);
    request.SetCancellationToken(ct);
    return request.PerformAsync()->Then<WSRepositoryResult>([=] (Http::Response& response) mutable
        {
        auto responseActivityId = GetResponseActivityId(response, activityLogger);

        if (!response.IsSuccess() && HttpStatus::InternalServerError != response.GetHttpStatus())
            return WSRepositoryResult::Error(CreateError(response, responseActivityId));

        repository.SetMaxUploadSize(GetMaxUploadSize(response, activityLogger, defaultMaxUploadSize));
        repository.SetPluginVersion(GetRepositoryPluginVersion(response, m_configuration->GetPersistenceProviderId()));
        return WSRepositoryResult::Success(repository);
        });
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
    auto activityLogger = CreateActivityLogger("Get Repositories");
    activityLogger.debug("Started");

    // TODO: implement filtering query by PluginId if needed
    Utf8String url = GetRepositoryUrl("");
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    SetActivityIdToRequest(activityLogger, request);
    request.SetCancellationToken(ct);
    return request.PerformAsync()->Then<WSRepositoriesResult>([this, activityLogger] (Http::Response& response) mutable
        {
        return ResolveGetRepositoriesResponse(response, activityLogger);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetObjectRequest
(
ObjectIdCR objectId,
Utf8StringCR eTag,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    auto activityLogger = CreateActivityLogger("Get Object", options);
    activityLogger.debug("Started");

    if (!objectId.IsValid())
        {
        activityLogger.errorv("Specified 'objectId' is not valid. 'objectId':'%s'", objectId.ToString().c_str());
        return CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    SetActivityIdToRequest(activityLogger, request);
    request.SetRetryOptions(Http::Request::RetryOption::ResetTransfer, 1);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObject);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this, activityLogger] (Http::Response& response) mutable
        {
        return ResolveObjectsResponse(response, activityLogger);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    simonas.mulevicius
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetChildrenRequest
(
ObjectIdCR parentObjectId,
const bset<Utf8String>& propertiesToSelect,
Utf8StringCR eTag,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    auto activityLogger = CreateActivityLogger("Get Children", options);
    activityLogger.debug("Started");

    Utf8String url = GetUrl(CreateNavigationSubPath(parentObjectId), CreateSelectPropertiesQuery(propertiesToSelect));
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url, eTag);

    SetActivityIdToRequest(activityLogger, request);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this, activityLogger](Http::Response& response) mutable
        {
        return ResolveObjectsResponse(response, activityLogger);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSResult> WebApiV2::SendGetFileRequest
(
ObjectIdCR objectId,
HttpBodyPtr bodyResponseOut,
Utf8StringCR eTag,
Http::Request::ProgressCallbackCR downloadProgressCallback,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    auto activityLogger = CreateActivityLogger("Get File", options);
    activityLogger.debug("Started");

    if (!objectId.IsValid())
        {
        activityLogger.errorv("Specified 'objectId' is not valid. 'objectId':'%s'", objectId.ToString().c_str());
        return CreateCompletedAsyncTask(WSResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    bool isExternalFileAccessSupported = GetMaxWebApiVersion() >= BeVersion(2, 4);

    Utf8String url = GetUrl(CreateFileSubPath(objectId));
    Http::Request request = CreateFileDownloadRequest(url, bodyResponseOut, eTag, activityLogger, downloadProgressCallback, ct);

    if (isExternalFileAccessSupported)
        {
        request.GetHeaders().SetValue(HEADER_MasAllowRedirect, VALUE_True);
        request.SetFollowRedirects(false);
        }

    auto finalResult = std::make_shared<WSResult>();
    return request.PerformAsync()->Then([=] (Http::Response& response) mutable
        {
        if (HttpStatus::TemporaryRedirect != response.GetHttpStatus())
            {
            *finalResult = ResolveFileDownloadResponse(response, activityLogger);
            return;
            }

        Utf8String redirectUrl = response.GetHeaders().GetLocation();
        Http::Request request = CreateFileDownloadRequest(redirectUrl, bodyResponseOut, eTag, activityLogger, downloadProgressCallback, ct);
        request.PerformAsync()->Then([=] (Http::Response& response) mutable
            {
            *finalResult = ResolveFileDownloadResponse(response, activityLogger);
            });
        })->Then<WSResult>([=]
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
HttpBodyPtr responseBody,
Utf8StringCR eTag,
ActivityLoggerR activityLogger,
Http::Request::ProgressCallbackCR onProgress,
ICancellationTokenPtr ct
) const
    {
    Http::Request request = m_configuration->GetHttpClient().CreateGetRequest(url);
    SetActivityIdToRequest(activityLogger, request);
    request.SetResponseBody(responseBody);
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
WSResult WebApiV2::ResolveFileDownloadResponse(Http::Response& response, ActivityLoggerCR activityLogger) const
    {
    auto activityId = GetActivityIdFromResponse(response, activityLogger.GetHeaderName());
    HttpStatus status = response.GetHttpStatus();
    if (HttpStatus::OK == status ||
        HttpStatus::NotModified == status)
        {
        auto wsResponse = WSResponse(status, response.GetHeaders().GetETag());
        SetActivityIdToWSResponse(wsResponse, activityId);
        return WSResult::Success(wsResponse);
        }
    return WSResult::Error(CreateError(response, activityId));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetSchemasRequest
(
Utf8StringCR eTag,
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    auto activityLogger = CreateActivityLogger("Get Schemas", options);
    activityLogger.debug("Started");

    Utf8String url = GetUrl(CreateClassSubPath("MetaSchema", "ECSchemaDef"));
    Http::Request request = m_configuration->GetHttpClient().CreateGetJsonRequest(url);

    SetActivityIdToRequest(activityLogger, request);
    request.GetHeaders().SetIfNoneMatch(eTag);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this, activityLogger] (Http::Response& response) mutable
        {
        return ResolveObjectsResponse(response, activityLogger);
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
IWSRepositoryClient::RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    auto activityLogger = CreateActivityLogger("Send Query", options);
    activityLogger.debug("Started");

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

    if (GetMaxWebApiVersion() >= BeVersion(2, 5) && !skipToken.empty())
        request.GetHeaders().SetValue(HEADER_SkipToken, skipToken);

    SetActivityIdToRequest(activityLogger, request);
    request.GetHeaders().SetIfNoneMatch(eTag);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSObjectsResult>([this, activityLogger] (Http::Response& response) mutable
        {
        return ResolveObjectsResponse(response, activityLogger);
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
    auto activityLogger = CreateActivityLogger("Send Changeset", options);
    activityLogger.debug("Started");

    if (GetMaxWebApiVersion() < BeVersion(2, 1))
        {
        activityLogger.error("Supported from WebApi 2.1 only");
        return CreateCompletedAsyncTask(WSChangesetResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    Utf8String url = GetUrl("$changeset");
    Http::Request request = m_configuration->GetHttpClient().CreatePostRequest(url);
    SetActivityIdToRequest(activityLogger, request);
    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::Upload);
    if (nullptr != options) 
        request.SetTransferTimeoutSeconds(options->GetTransferTimeOut());

    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);

    request.SetRequestBody(changeset);
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    return m_jobApi->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then<WSChangesetResult>([=] (HttpJobResult& response) mutable
        {
        if (!response.IsSuccess())
            return WSChangesetResult::Error(response.GetError());

        auto httpResponse = response.GetValue();
        auto responseActivityId = GetResponseActivityId(httpResponse, activityLogger);

        if (HttpStatus::OK != httpResponse.GetHttpStatus())
            return WSChangesetResult::Error(CreateError(httpResponse, responseActivityId));

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
    auto activityLogger = CreateActivityLogger("Create Object", options);
    activityLogger.debug("Started");

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
    SetActivityIdToRequest(activityLogger, request);
    if (nullptr != options)
        request.SetUploadTransferTime(options->GetTransferTimeOut());

    request.SetHandshakeRequestBody(HttpStringBody::Create(Json::FastWriter().write(objectCreationJson)), "application/json");
    if (!filePath.empty())
        request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));

    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);
    return m_jobApi->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then<WSCreateObjectResult>([=] (HttpJobResult& response) mutable
        {
        if (!response.IsSuccess())
            return WSCreateObjectResult::Error(response.GetError());

        auto httpResponse = response.GetValue();
        auto responseActivityId = GetResponseActivityId(httpResponse, activityLogger);

        if (HttpStatus::Created != httpResponse.GetHttpStatus() && HttpStatus::OK != httpResponse.GetHttpStatus())
            return WSCreateObjectResult::Error(CreateError(httpResponse, responseActivityId));

        auto uploadResponse = ResolveUploadResponse(httpResponse);
        SetActivityIdToWSResponse(uploadResponse, responseActivityId);
        return WSCreateObjectResult::Success(uploadResponse);
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
    auto activityLogger = CreateActivityLogger("Update Object", options);
    activityLogger.debug("Started");

    if (!objectId.IsValid())
        {
        activityLogger.errorv("Specified 'objectId' is not valid. 'objectId':'%s'", objectId.ToString().c_str());
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    if (!filePath.empty() && GetMaxWebApiVersion() < BeVersion(2, 4))
        {
        BeAssert(false && "SendUpdateObjectRequest() supports file upload from WebApi 2.4 only. Update server or use seperate file upload");
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    ChunkedUploadRequest request("POST", url, m_configuration->GetHttpClient());
    SetActivityIdToRequest(activityLogger, request);
    if (nullptr != options)
        request.SetUploadTransferTime(options->GetTransferTimeOut());
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

        return ResolveUpdateObjectResponse(response.GetValue(), activityLogger);
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
    auto activityLogger = CreateActivityLogger("Delete Object", options);
    activityLogger.debug("Started");

    if (!objectId.IsValid())
        {
        activityLogger.errorv("Specified 'objectId' is not valid. 'objectId':'%s'", objectId.ToString().c_str());
        return CreateCompletedAsyncTask(WSDeleteObjectResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    Utf8String url = GetUrl(CreateObjectSubPath(objectId));
    Http::Request request = m_configuration->GetHttpClient().CreateRequest(url, "DELETE");
    SetActivityIdToRequest(activityLogger, request);
    request.SetCancellationToken(ct);
    if (nullptr != options)
        request.SetTransferTimeoutSeconds(options->GetTransferTimeOut());
        
    return m_jobApi->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then<WSDeleteObjectResult>([=] (HttpJobResult& response) mutable
        {
        if (!response.IsSuccess())
            return WSDeleteObjectResult::Error(response.GetError());

        auto httpResponse = response.GetValue();
        auto responseActivityId = GetResponseActivityId(httpResponse, activityLogger);
        if (HttpStatus::OK == httpResponse.GetHttpStatus())
            return WSDeleteObjectResult::Success();

        return WSDeleteObjectResult::Error(CreateError(httpResponse, responseActivityId));
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
    auto activityLogger = CreateActivityLogger("Update File", options);
    activityLogger.debug("Started");

    if (!objectId.IsValid())
        {
        activityLogger.errorv("Specified 'objectId' is not valid. 'objectId':'%s'", objectId.ToString().c_str());
        return CreateCompletedAsyncTask(WSUpdateFileResult::Error(WSError::CreateFunctionalityNotSupportedError()));
        }

    bool isExternalFileAccessSupported = GetMaxWebApiVersion() >= BeVersion(2, 4);

    BeFile beFile;
    beFile.Open(filePath, BeFileAccess::Read);
    Utf8String url = GetUrl(CreateFileSubPath(objectId));
    ChunkedUploadRequest request("PUT", url, m_configuration->GetHttpClient());
    SetActivityIdToRequest(activityLogger, request);
    if (nullptr != options)
        request.SetUploadTransferTime(options->GetTransferTimeOut());

    request.SetRequestBody(HttpFileBody::Create(filePath), Utf8String(filePath.GetFileNameAndExtension()));
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(uploadProgressCallback);

    if (isExternalFileAccessSupported)
        {
        request.GetHandshakeRequest().GetHeaders().SetValue(HEADER_MasAllowRedirect, VALUE_True);
        request.GetHandshakeRequest().SetFollowRedirects(false);
        }

    //TODO TFS#866928 SendUpdateFileRequest temporary disabled
    if (options && options->GetJobOptions()->IsJobsApiEnabled())
        options->GetJobOptions()->DisableJobs();

    auto finalResult = std::make_shared<WSUpdateFileResult>();
    return m_jobApi
        ->ExecuteViaJob(request, m_info, options ? options->GetJobOptions() : nullptr, ct)
        ->Then([=] (HttpJobResult& response) mutable
        {
        if (!response.IsSuccess())
            {
            finalResult->SetError(response.GetError());
            return;
            }

        auto httpResponse = response.GetValue();

        ResolveUpdateFileResponse(httpResponse, url, filePath, activityLogger, uploadProgressCallback, ct)
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
ActivityLoggerR activityLogger,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    auto responseActivityId = GetResponseActivityId(httpResponse, activityLogger);
    if (HttpStatus::OK == httpResponse.GetHttpStatus())
        {
        auto uploadResponse = ResolveUploadResponse(httpResponse);
        SetActivityIdToWSResponse(uploadResponse, responseActivityId);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Success(uploadResponse));
        }

    if (HttpStatus::TemporaryRedirect != httpResponse.GetHttpStatus())
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(CreateError(httpResponse, responseActivityId)));

    Utf8String redirectUrl = httpResponse.GetHeaders().GetLocation();
    Utf8String redirectType = httpResponse.GetHeaders().GetValue(HEADER_MasFileAccessUrlType);
    Utf8String confirmationId = httpResponse.GetHeaders().GetValue(HEADER_MasUploadConfirmationId);

    if (redirectType != VALUE_FileAccessUrlType_Azure)
        {
        activityLogger.errorv("Header field '%s' contains not supported value: '%s'", HEADER_MasFileAccessUrlType, redirectType.c_str());
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(CreateServerNotSupportedError(httpResponse, responseActivityId)));
        }

    auto azureRequestOptions = std::make_shared<IAzureBlobStorageClient::RequestOptions>();
    azureRequestOptions->GetActivityOptions()->SetActivityId(activityLogger.GetActivityId());

    auto finalResult = std::make_shared<WSUpdateFileResult>();
    return m_azureClient->SendUpdateFileRequest(redirectUrl, filePath, uploadProgressCallback, azureRequestOptions, ct)->Then([=] (AzureResult azureResult) mutable
        {
        auto secondResponseActivityId = GetResponseActivityId(httpResponse, activityLogger);
        if (!azureResult.IsSuccess())
            {
            finalResult->SetError(CreateErrorFromAzzureError(azureResult.GetError(), secondResponseActivityId));
            return;
            }

        WSUploadResponse uploadResponse = {nullptr, azureResult.GetValue().GetETag()};
        SetActivityIdToWSResponse(uploadResponse, secondResponseActivityId);
        finalResult->SetSuccess(uploadResponse);
        if (confirmationId.empty())
            return;

        Http::Request request = m_configuration->GetHttpClient().CreateRequest(url, "PUT");
        SetActivityIdToRequest(activityLogger, request);
        request.GetHeaders().SetValue(HEADER_MasUploadConfirmationId, confirmationId);
        request.PerformAsync()->Then([=] (Http::Response& response) mutable
            {
            if (HttpStatus::OK != response.GetHttpStatus())
                finalResult->SetError(CreateError(response, GetResponseActivityId(response, activityLogger)));
            });
        })
            ->Then<WSUpdateFileResult>([=]
            {
            return *finalResult;
            });
    }
