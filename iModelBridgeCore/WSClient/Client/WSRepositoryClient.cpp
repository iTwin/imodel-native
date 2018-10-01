/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSRepositoryClient.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApi/WebApiV1.h"
#include <regex>

#define HEADER_MasConnectionInfo "Mas-Connection-Info"

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
IWSRepositoryClient::RequestOptions::RequestOptions() : m_transferTimeOut(IWSRepositoryClient::Timeout::Transfer::Default) 
    {
    m_jobOptions = std::make_shared<JobOptions>();
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
m_infoProvider(std::make_shared<RepositoryInfoProvider>(m_connection)),
m_config(std::make_shared<Configuration>(Configuration(*m_connection)))
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
    BeAssert(!serverUrl.empty());
    BeAssert(!repositoryId.empty());
    BeAssert(nullptr != clientInfo);
    auto configuration = std::make_shared<ClientConfiguration>(serverUrl, repositoryId, clientInfo, schemaProvider, customHandler);
    configuration->SetPersistenceProviderId(ParsePluginIdFromRepositoryId(repositoryId));
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
    return m_infoProvider->GetInfo(ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSVoidResult> WSRepositoryClient::VerifyAccess(ICancellationTokenPtr ct) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSVoidResult>([=] (WebApiPtr webApi)
        {
        ObjectId fakeObject("NonExistingSchema.NonExistingClassForCredentialChecking", "nonId");

        return
            webApi->SendGetObjectRequest(fakeObject, "", ct)
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
    return m_connection->GetWebApiAndReturnResponse<WSObjectsResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendGetObjectRequest(objectId, eTag, ct);
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
    bset<Utf8String> properties;
    return SendGetChildrenRequest(parentObjectId, properties, eTag, ct);
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
    return m_connection->GetWebApiAndReturnResponse<WSObjectsResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendGetChildrenRequest(parentObjectId, propertiesToSelect, eTag, ct);
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
HttpRequest::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return m_fileDownloadQueue.Push([=]
        {
        return m_connection->GetWebApiAndReturnResponse<WSFileResult>([=] (WebApiPtr webApi)
            {
            return webApi->SendGetFileRequest(objectId, filePath, eTag, downloadProgressCallback, ct);
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
    return m_connection->GetWebApiAndReturnResponse<WSObjectsResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendGetSchemasRequest(eTag, ct);
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
    return m_connection->GetWebApiAndReturnResponse<WSObjectsResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendQueryRequest(query, eTag, skipToken, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSChangesetResult> WSRepositoryClient::SendChangesetRequest
(
HttpBodyPtr changeset,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSChangesetResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendChangesetRequest(changeset, uploadProgressCallback, nullptr, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSChangesetResult> WSRepositoryClient::SendChangesetRequestWithOptions
(
HttpBodyPtr changeset,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
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
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSCreateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendCreateObjectRequest(objectCreationJson, filePath, uploadProgressCallback, nullptr, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WSRepositoryClient::SendCreateObjectRequestWithOptions
(
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
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
* @bsimethod                                                    Petras.Sukys    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WSRepositoryClient::SendCreateObjectRequest
(
ObjectIdCR relatedObjectId,
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSCreateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendCreateObjectRequest(relatedObjectId, objectCreationJson, filePath, uploadProgressCallback, nullptr, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Petras.Sukys    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSCreateObjectResult> WSRepositoryClient::SendCreateObjectRequestWithOptions
(
ObjectIdCR relatedObjectId,
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
RequestOptionsPtr options,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSCreateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendCreateObjectRequest(relatedObjectId, objectCreationJson, filePath, uploadProgressCallback, options, ct);
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
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSUpdateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendUpdateObjectRequest(objectId, propertiesJson, eTag, filePath, uploadProgressCallback, nullptr, ct);
        }, ct);
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
HttpRequest::ProgressCallbackCR uploadProgressCallback,
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
    return m_connection->GetWebApiAndReturnResponse<WSDeleteObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendDeleteObjectRequest(objectId, nullptr, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
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
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSUpdateFileResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendUpdateFileRequest(objectId, filePath, uploadProgressCallback, nullptr, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSUpdateFileResult> WSRepositoryClient::SendUpdateFileRequestWithOptions
(
ObjectIdCR objectId,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
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
    // v\d+\.\d+ - Web API version (e.g. "v2.5")
    //     v\d+ - first character must be "v" and then one or more digits - Web API version first part (e.g. "v2")
    //     \.\d+ - "." and then one or more digits
    // \/repositories\/ - "/repositories/"

    std::regex regex(R"((https?:\/\/.+)\/v\d+\.\d+\/repositories\/)", std::regex_constants::icase);
    std::cmatch matches;
    std::regex_search(url.c_str(), matches, regex);
    if (matches.empty() && matches.size() != 1)
        return WSRepository();

    WSRepository repository;
    repository.SetServerUrl(matches[1].str().c_str()); // [0] element is overall match 

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
    // First replace the percent sign so that it is not used in decoding, as it is a value in this encoder, not an escape symbol.
    url.ReplaceAll("%", percentReplace.c_str());
    url.ReplaceAll("~25", percentReplace.c_str());
    // Then use % instead of ~ for the real decoding algorithm.
    url.ReplaceAll("~", "%");
    // URL decode
    auto decodedUrl = HttpClient::UnescapeString(url);
    // Restore the values that failed to decode, i.e. where % was not used.
    decodedUrl.ReplaceAll("%", "~");
    // Restore the backed up percent signs.
    decodedUrl.ReplaceAll(percentReplace.c_str(), "%");

    return decodedUrl;
    }
