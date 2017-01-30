/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSRepositoryClient.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApi/WebApiV1.h"

using namespace std::placeholders;

// WIP: SkipTokens disabled due to issues. To enable, set to "0".
// Problem: WebApi 2.3 304 NotModified responses do not contain SkipToken header field,
// this corrupts data refresh.
const Utf8String IWSRepositoryClient::InitialSkipToken = "";

const uint32_t IWSRepositoryClient::Timeout::Connection::Default = 30;

const uint32_t IWSRepositoryClient::Timeout::Transfer::GetObject = 30;
const uint32_t IWSRepositoryClient::Timeout::Transfer::GetObjects = 120; // Some repositories take a lot of time to create many full ECInstances
const uint32_t IWSRepositoryClient::Timeout::Transfer::FileDownload = 30;
const uint32_t IWSRepositoryClient::Timeout::Transfer::Upload = 30;
const uint32_t IWSRepositoryClient::Timeout::Transfer::Default = 60;
const uint32_t IWSRepositoryClient::Timeout::Transfer::LongUpload = 120;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IWSRepositoryClient::RequestOptions::RequestOptions() : m_transferTimeOut(IWSRepositoryClient::Timeout::Transfer::Default) {}

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
m_config(std::make_shared<ClientConfig>(ClientConfig(*m_connection)))
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
WSRepositoryClient::ClientConfig& WSRepositoryClient::Config()
    {
    return *m_config;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::SetCredentials(Credentials credentials)
    {
    return m_connection->GetConfiguration().GetHttpClient().SetCredentials(std::move(credentials));
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
Http::Request::ProgressCallbackCR downloadProgressCallback,
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
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr ct,
RequestOptionsPtr options
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSChangesetResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendChangesetRequest(changeset, uploadProgressCallback, ct, options);
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
    return m_connection->GetWebApiAndReturnResponse<WSCreateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendCreateObjectRequest(objectCreationJson, filePath, uploadProgressCallback, ct);
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
    return m_connection->GetWebApiAndReturnResponse<WSCreateObjectResult> ([=] (WebApiPtr webApi)
        {
        return webApi->SendCreateObjectRequest (objectId, objectCreationJson, filePath, uploadProgressCallback, ct);
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
    return m_connection->GetWebApiAndReturnResponse<WSUpdateObjectResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendUpdateObjectRequest(objectId, propertiesJson, eTag, filePath, uploadProgressCallback, ct);
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
        return webApi->SendDeleteObjectRequest(objectId, ct);
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
    return m_connection->GetWebApiAndReturnResponse<WSUpdateFileResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendUpdateFileRequest(objectId, filePath, uploadProgressCallback, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::ClientConfig::SetCompressionOptions(CompressionOptions options)
    {
    m_connection.GetConfiguration().GetHttpClient().SetCompressionOptions(options);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepositoryClient::ClientConfig::SetMaxUrlLength(size_t length)
    {
    m_connection.GetConfiguration().SetMaxUrlLength(length);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CompressionOptionsCR WSRepositoryClient::ClientConfig::GetCompressionOptions() const
    {
    return m_connection.GetConfiguration().GetHttpClient().GetCompressionOptions();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSRepositoryClient::ClientConfig::GetMaxUrlLength() const
    {
    return m_connection.GetConfiguration().GetMaxUrlLength();
    }