/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <regex>

#define HEADER_MasConnectionInfo "Mas-Connection-Info"
#define HEADER_MasRequestId      "Mas-Request-Id"
#define HEADER_XCorrelationId    "X-Correlation-Id"

using namespace std::placeholders;

const Utf8String IWSRepositoryClient::InitialSkipToken = "0";

const uint32_t IWSRepositoryClient::Timeout::Connection::Default = 30;

const uint32_t IWSRepositoryClient::Timeout::Transfer::GetObject = 30;
const uint32_t IWSRepositoryClient::Timeout::Transfer::GetObjects = 120; // Some repositories take a lot of time to create many full ECInstances
const uint32_t IWSRepositoryClient::Timeout::Transfer::FileDownload = 30;
const uint32_t IWSRepositoryClient::Timeout::Transfer::Upload = 30;
const uint32_t IWSRepositoryClient::Timeout::Transfer::Default = 60;
const uint32_t IWSRepositoryClient::Timeout::Transfer::LongUpload = 120;
const uint32_t IWSRepositoryClient::Timeout::Transfer::UploadProcessing = 300; // Longer timeout for server file processing to finish

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IWSRepositoryClient::RequestOptions::RequestOptions() :
m_transferTimeOut(IWSRepositoryClient::Timeout::Transfer::Default),
m_activityOptions(std::make_shared<ActivityOptions>()),
m_jobOptions(std::make_shared<JobOptions>())
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IWSRepositoryClient::ActivityOptions::HeaderNameToString(HeaderName headerName)
    {
    switch (headerName)
        {
        case HeaderName::MasRequestId:
            return HEADER_MasRequestId;
        case HeaderName::XCorrelationId:
            return HEADER_XCorrelationId;
        }

    BeAssert(false && "Unknown header name for ActivityId");
    return Utf8String();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IWSRepositoryClient::~IWSRepositoryClient()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepositoryClient::WSRepositoryClient(std::shared_ptr<struct ClientConnection> connection) :
m_connection(connection),
m_serverClient(WSClient::Create(m_connection)),
m_config(std::make_shared<Configuration>(Configuration(*m_connection))),
m_infoProvider(std::make_shared<RepositoryInfoProvider>(m_connection))
    {} 

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<WSRepositoryClient> WSRepositoryClient::Create
(
Utf8StringCR serverUrl,
Utf8StringCR repositoryId,
ClientInfoPtr clientInfo,
IWSSchemaProviderPtr schemaProvider,
IHttpHandlerPtr customHandler
)
    {
    return Create(serverUrl, BeVersion(), repositoryId, clientInfo, schemaProvider, customHandler);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<WSRepositoryClient> WSRepositoryClient::Create
(
Utf8StringCR serverUrl,
BeVersionCR serviceVersion,
Utf8StringCR repositoryId,
ClientInfoPtr clientInfo,
IWSSchemaProviderPtr schemaProvider,
IHttpHandlerPtr customHandler
)
    {
    BeAssert(!serverUrl.empty());
    BeAssert(!repositoryId.empty());
    BeAssert(nullptr != clientInfo);
    auto configuration = std::make_shared<ClientConfiguration>(serverUrl, repositoryId, clientInfo, schemaProvider, customHandler);
    configuration->SetPersistenceProviderId(ParsePluginIdFromRepositoryId(repositoryId));
    configuration->SetServiceVersion(serviceVersion);
    return std::shared_ptr<WSRepositoryClient>(new WSRepositoryClient(std::make_shared<ClientConnection>(configuration)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::SetFileDownloadLimit(size_t limit)
    {
    m_fileDownloadQueue.SetLimit(limit);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IWSClientPtr WSRepositoryClient::GetWSClient() const
    {
    return m_serverClient;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSRepositoryClient::GetRepositoryId() const
    {
    return m_connection->GetConfiguration().GetRepositoryId();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepositoryClient::Configuration& WSRepositoryClient::Config()
    {
    return *m_config;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::SetCredentials(Credentials credentials, AuthenticationType type)
    {
    m_connection->GetConfiguration().GetHttpClient().SetCredentials(std::move(credentials));
    if (AuthenticationType::Windows == type)
        m_connection->GetConfiguration().GetDefaultHeaders().SetValue(HEADER_MasConnectionInfo, "CredentialType=Windows");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas   5/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::RegisterRepositoryInfoListener(std::weak_ptr<IRepositoryInfoListener> listener)
    {
    m_infoProvider->RegisterInfoListener(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas   5/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::UnregisterRepositoryInfoListener(std::weak_ptr<IRepositoryInfoListener> listener)
    {
    m_infoProvider->UnregisterInfoListener(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas   5/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSRepositoryResult> WSRepositoryClient::GetInfo(ICancellationTokenPtr ct) const
    {
    return GetInfoWithOptions(nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Mantas.Smicius   11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSRepositoryResult> WSRepositoryClient::GetInfoWithOptions(RequestOptionsPtr options, ICancellationTokenPtr ct) const
    {
    return m_infoProvider->GetInfo(options, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSVoidResult> WSRepositoryClient::VerifyAccess(ICancellationTokenPtr ct) const
    {
    return VerifyAccessWithOptions(nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Mantas.Smicius   11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSVoidResult> WSRepositoryClient::VerifyAccessWithOptions(RequestOptionsPtr options, ICancellationTokenPtr ct) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSVoidResult>([=] (WebApiPtr webApi)
        {
        ObjectId fakeObject("NonExistingSchema.NonExistingClassForCredentialChecking", "nonId");

        return
            webApi->SendGetObjectRequest(fakeObject, "", options, ct)
            ->Then<WSVoidResult>([=] (WSObjectsResult& result)
            {
            if (WSError::Id::ClassNotFound == result.GetError().GetId() ||
                WSError::Id::SchemaNotFound == result.GetError().GetId())
                {
                return WSVoidResult::Success();
                }
            return WSVoidResult::Error(result.GetError());
            });
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendGetObjectRequest
(
ObjectIdCR objectId,
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    return SendGetObjectRequestWithOptions(objectId, eTag, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Mantas.Smicius   11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendGetObjectRequestWithOptions
(
ObjectIdCR objectId,
Utf8StringCR eTag,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSObjectsResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendGetObjectRequest(objectId, eTag, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendGetChildrenRequest
(
ObjectIdCR parentObjectId,
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    return SendGetChildrenRequestWithOptions(parentObjectId, eTag, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Mantas.Smicius   11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendGetChildrenRequestWithOptions
(
ObjectIdCR parentObjectId,
Utf8StringCR eTag,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    bset<Utf8String> properties;
    return SendGetChildrenRequestWithOptions(parentObjectId, properties, eTag, options, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendGetChildrenRequest
(
ObjectIdCR parentObjectId,
const bset<Utf8String>& propertiesToSelect,
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    return SendGetChildrenRequestWithOptions(parentObjectId, propertiesToSelect, eTag, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Mantas.Smicius   11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendGetChildrenRequestWithOptions
(
ObjectIdCR parentObjectId,
const bset<Utf8String>& propertiesToSelect,
Utf8StringCR eTag,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSObjectsResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendGetChildrenRequest(parentObjectId, propertiesToSelect, eTag, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSFileResult> WSRepositoryClient::SendGetFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
Utf8StringCR eTag,
Http::Request::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
	if (filePath.empty())
		return CreateCompletedAsyncTask(WSFileResult::Error(WSError::CreateFunctionalityNotSupportedError()));

    return m_fileDownloadQueue.Push([=]
        {
        return m_connection->GetWebApiAndReturnResponse<WSResult>([=] (WebApiPtr webApi)
            {
            return webApi->SendGetFileRequest(objectId, HttpFileBody::Create(filePath), eTag, downloadProgressCallback, nullptr, ct);
            }, ct);
        }, ct)
	->Then<WSFileResult>([=] (WSResult response)
        {
        if (!response.IsSuccess())
            return WSFileResult::Error(response.GetError());

		auto status = response.GetValue().IsModified() ? Http::HttpStatus::OK : Http::HttpStatus::BadRequest;
        return WSFileResult::Success(WSFileResponse(filePath, status, response.GetValue().GetETag()));
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSResult> WSRepositoryClient::SendGetFileRequest
(
ObjectIdCR objectId,
Http::HttpBodyPtr responseBodyOut,
Utf8StringCR eTag,
Http::Request::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return SendGetFileRequestWithOptions(objectId, responseBodyOut, eTag, downloadProgressCallback, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Mantas.Smicius   11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSResult> WSRepositoryClient::SendGetFileRequestWithOptions
(
ObjectIdCR objectId,
Http::HttpBodyPtr responseBodyOut,
Utf8StringCR eTag,
Http::Request::ProgressCallbackCR downloadProgressCallback,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_fileDownloadQueue.Push([=]
        {
        return m_connection->GetWebApiAndReturnResponse<WSResult>([=] (WebApiPtr webApi)
            {
            return webApi->SendGetFileRequest(objectId, responseBodyOut, eTag, downloadProgressCallback, options, ct);
            }, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendGetSchemasRequest
(
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    return SendGetSchemasRequestWithOptions(eTag, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Mantas.Smicius   11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendGetSchemasRequestWithOptions
(
Utf8StringCR eTag,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSObjectsResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendGetSchemasRequest(eTag, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendQueryRequest
(
WSQueryCR query,
Utf8StringCR eTag,
Utf8StringCR skipToken,
ICancellationTokenPtr ct
) const
    {
    return SendQueryRequestWithOptions(query, eTag, skipToken, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> WSRepositoryClient::SendQueryRequestWithOptions
(
WSQueryCR query,
Utf8StringCR eTag,
Utf8StringCR skipToken,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSObjectsResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendQueryRequest(query, eTag, skipToken, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSChangesetResult> WSRepositoryClient::SendChangesetRequest
(
HttpBodyPtr changeset,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return SendChangesetRequestWithOptions(changeset, uploadProgressCallback, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSChangesetResult> WSRepositoryClient::SendChangesetRequestWithOptions
(
HttpBodyPtr changeset,
Http::Request::ProgressCallbackCR uploadProgressCallback,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSChangesetResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendChangesetRequest(changeset, uploadProgressCallback, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WSRepositoryClient::SendCreateObjectRequest
(
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return SendCreateObjectRequestWithOptions(objectCreationJson, filePath, uploadProgressCallback, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WSRepositoryClient::SendCreateObjectRequestWithOptions
(
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSCreateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendCreateObjectRequest(objectCreationJson, filePath, uploadProgressCallback, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WSRepositoryClient::SendCreateObjectRequest
(
ObjectIdCR objectId,
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return SendCreateObjectRequestWithOptions(objectId, objectCreationJson, filePath, uploadProgressCallback, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WSRepositoryClient::SendCreateObjectRequestWithOptions
(
ObjectIdCR objectId,
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSCreateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendCreateObjectRequest(objectId, objectCreationJson, filePath, uploadProgressCallback, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSUpdateObjectResult> WSRepositoryClient::SendUpdateObjectRequest
(
ObjectIdCR objectId,
JsonValueCR propertiesJson,
Utf8StringCR eTag,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return SendUpdateObjectRequestWithOptions(objectId, propertiesJson, eTag, filePath, uploadProgressCallback, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSUpdateObjectResult> WSRepositoryClient::SendUpdateObjectRequestWithOptions
(
ObjectIdCR objectId,
JsonValueCR propertiesJson,
Utf8StringCR eTag,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSUpdateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendUpdateObjectRequest(objectId, propertiesJson, eTag, filePath, uploadProgressCallback, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSDeleteObjectResult> WSRepositoryClient::SendDeleteObjectRequest
(
ObjectIdCR objectId,
ICancellationTokenPtr ct
) const
    {
    return SendDeleteObjectRequestWithOptions(objectId, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSDeleteObjectResult> WSRepositoryClient::SendDeleteObjectRequestWithOptions
(
ObjectIdCR objectId,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSDeleteObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendDeleteObjectRequest(objectId, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSUpdateFileResult> WSRepositoryClient::SendUpdateFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return SendUpdateFileRequestWithOptions(objectId, filePath, uploadProgressCallback, nullptr, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSUpdateFileResult> WSRepositoryClient::SendUpdateFileRequestWithOptions
(
ObjectIdCR objectId,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR uploadProgressCallback,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSUpdateFileResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendUpdateFileRequest(objectId, filePath, uploadProgressCallback, options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::Configuration::SetCompressionOptions(CompressionOptions options)
    {
    if (options.IsRequestCompressionEnabled())
        {
        options.AddSupportedType(REQUESTHEADER_ContentType_ApplicationJson);
        options.AddSupportedType(REQUESTHEADER_ContentType_ApplicationXml);
        options.AddSupportedType(REQUESTHEADER_ContentType_TextHtml);
        }

    m_connection.GetConfiguration().GetHttpClient().SetCompressionOptions(options);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::Configuration::SetMaxUrlLength(size_t length)
    {
    m_connection.GetConfiguration().SetMaxUrlLength(length);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CompressionOptionsCR WSRepositoryClient::Configuration::GetCompressionOptions() const
    {
    return m_connection.GetConfiguration().GetHttpClient().GetCompressionOptions();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSRepositoryClient::Configuration::GetMaxUrlLength() const
    {
    return m_connection.GetConfiguration().GetMaxUrlLength();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::Configuration::SetPersistenceProviderId(Utf8StringCR provider)
    {
    m_connection.GetConfiguration().SetPersistenceProviderId(provider);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSRepositoryClient::Configuration::GetPersistenceProviderId() const
    {
    return m_connection.GetConfiguration().GetPersistenceProviderId();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vilius.Kazlauskas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepository WSRepositoryClient::ParseRepositoryUrl(Utf8StringCR url, Utf8StringP remainingPathOut)
    {
    //---------------------------------ServerUrl parsing---------------------------------

    // Matches that URL would start with "http" or "https" and that after server URL are Web API version and "/repositories/"
    // Captures ServerUrl, excludes rest part of url starting from Web API version
    // E.g. if URL is "https://foo.com/ws250/v2.5/repositories/Foo--Boo.com~3AFoo/Schema/Project/RemoteId"
    //      it will capture "https://foo.com/ws250/"
    //
    // (https?:\/\/.+\/) - captures ServerUrl:
    //     https?:\/\/ - "http://" or "https://"
    //     .+\/ - any character one or more times untill "/"
    // (\/v|\/sv)\d+\.\d+ - Web API version (e.g. "v2.5") or Service Version (e.g. "sv4.2")
    // \/repositories\/ - "/repositories/"

    std::regex regex(R"((https?:\/\/.+)(\/v|\/sv)(\d+\.\d+)\/repositories\/)", std::regex_constants::icase);
    std::cmatch matches;
    std::regex_search(url.c_str(), matches, regex);
    if (matches.size() < 2)
        return WSRepository();

    WSRepository repository;
    repository.SetServerUrl(matches[1].str().c_str()); // [0] element is overall match 

    if (matches.size() == 4 && matches[2] == "/sv")
        repository.SetServiceVersion(BeVersion(matches[3].str().c_str()));

    //--------------------------------RepositoryId and PluginId parsing-------------------------------  

    // Matches PluginId, then "--", and then Location till "/", "?" or "#"
    // [^\/\?#]+ - any character except "/", "?" or "#"
    // E.g. if remaining URL part is "PluginId--Boo.com~3AFoo/Schema/Class/RemoteId?query=foo"
    //      it will capture PluginId - "PluginId" and RepositoryId - "PluginId--Boo.com~3AFoo"
    Utf8String remainingUrlPart(matches[0].second);
    regex = R"(((.+?)--[^\/\?#]+))";
    std::regex_search(remainingUrlPart.c_str(), matches, regex);
    if (matches.empty() && matches.size() != 3)
        return WSRepository();
    
    Utf8String repositoryId(matches[1].str().c_str());
    repository.SetId(repositoryId);
    repository.SetPluginId(matches[2].str().c_str());

    //-------------------------------Remaining part of url-------------------------------
    if (nullptr != remainingPathOut && Utf8String(matches[0].second).length() > 1) // Skip if there is just a seperator "/"
        *remainingPathOut = matches[0].second;

    //---------------------------------Location parsing----------------------------------

    // Matches that after PluginId goes "--" and after that any character one or more times
    // Captures Location
    // .+?-- - any character one or more times that goes untill first "--"
    // (.+) - captures Location - any character that appears one or more times
    // E.g. if RepositoryId is "PluginId--Boo.com~3AFoo"
    //      it will capture "Boo.com~3AFoo"
    regex = R"(.+?--(.+))";
    std::regex_search(repositoryId.c_str(), matches, regex);
    if (matches.empty() && matches.size() != 2)
        return WSRepository();

    auto location = UrlDecode(matches[1].str().c_str()); // [0] element is overall match 
    repository.SetLocation(location);

    return repository;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSRepositoryClient::ParsePluginIdFromRepositoryId(Utf8StringCR repositoryId)
    {
    std::regex regex(R"(((.+?)--[^\/\?#]+))");
    std::cmatch matches;
    std::regex_search(repositoryId.c_str(), matches, regex);
    if (matches.empty() && matches.size() != 3)
        return "";

    return (matches[2].str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSRepositoryClient::UrlDecode(Utf8String url)
    {
    Utf8String percentReplace("-PeRCenT_");
    // First replace the percent sign so that it is not used in decoding, as it is a value in this encoder, not an escape symbol.
    url.ReplaceAll("%", percentReplace.c_str());
    url.ReplaceAll("~25", percentReplace.c_str());
    // Then use % instead of ~ for the real decoding algorithm.
    url.ReplaceAll("~", "%");
    // URL decode
    auto decodedUrl = BeUri::UnescapeString(url);
    // Restore the values that failed to decode, i.e. where % was not used.
    decodedUrl.ReplaceAll("%", "~");
    // Restore the backed up percent signs.
    decodedUrl.ReplaceAll(percentReplace.c_str(), "%");

    return decodedUrl;
    }
