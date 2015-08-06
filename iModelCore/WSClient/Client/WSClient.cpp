/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSClient.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApi/WebApiV1.h"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IWSClient::~IWSClient()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WSClient::WSClient(std::shared_ptr<ClientConnection> connection) :
m_connection(connection)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IWSClientPtr WSClient::Create(std::shared_ptr<ClientConnection> connection)
    {
    return std::shared_ptr<WSClient>(new WSClient(connection));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IWSClientPtr WSClient::Create(Utf8StringCR serverUrl, ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler)
    {
    BeAssert(nullptr != clientInfo);
    auto configuration = std::make_shared<ClientConfiguration>(serverUrl, "", clientInfo, nullptr, customHandler);
    return Create(std::make_shared<ClientConnection>(configuration));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSClient::GetServerUrl() const
    {
    return m_connection->GetConfiguration().GetServerUrl();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSClient::RegisterServerInfoListener(std::weak_ptr<IServerInfoListener> listener)
    {
    m_connection->RegisterServerInfoListener(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSClient::UnregisterServerInfoListener(std::weak_ptr<IServerInfoListener> listener)
    {
    m_connection->UnregisterServerInfoListener(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> WSClient::GetServerInfo
(
ICancellationTokenPtr cancellationToken
) const
    {
    return m_connection->GetServerInfo(false, cancellationToken);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> WSClient::SendGetInfoRequest
(
ICancellationTokenPtr cancellationToken
) const
    {
    return m_connection->GetServerInfo(true, cancellationToken);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSRepositoriesResult> WSClient::SendGetRepositoriesRequest
(
ICancellationTokenPtr cancellationToken
) const
    {
    return SendGetRepositoriesRequest(bvector<Utf8String>(), bvector<Utf8String>());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSRepositoriesResult> WSClient::SendGetRepositoriesRequest
(
const bvector<Utf8String>& types,
const bvector<Utf8String>& providerIds,
ICancellationTokenPtr cancellationToken
) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSRepositoriesResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendGetRepositoriesRequest(types, providerIds, cancellationToken);
        }, cancellationToken);
    }
