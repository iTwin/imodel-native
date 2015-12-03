/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientConnection.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApi/WebApiV1.h"
#include "WebApi/WebApiV1BentleyConnect.h"
#include "WebApi/WebApiV2.h"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ClientConnection::ClientConnection(std::shared_ptr<ClientConfiguration> configuration) :
m_configuration(configuration),
m_infoProvider(std::make_shared<ServerInfoProvider>(m_configuration))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const ClientConfiguration& ClientConnection::GetConfiguration() const
    {
    return *m_configuration;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ClientConfiguration& ClientConnection::GetConfiguration()
    {
    return *m_configuration;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientConnection::RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    m_infoProvider->RegisterServerInfoListener(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientConnection::UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    m_infoProvider->UnregisterServerInfoListener(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ClientConnection::GetServerInfo
(
bool forceQuery,
ICancellationTokenPtr ct
) const
    {
    auto thisPtr = shared_from_this();
    return m_infoProvider->GetServerInfo(forceQuery, ct)
        ->Then<WSInfoResult>([thisPtr] (WSInfoResult& result)
        {
        return result;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> ClientConnection::InvalidateInfo() const
    {
    return m_infoProvider->InvalidateInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WebApiResult> ClientConnection::GetWebApi
(
ICancellationTokenPtr ct
) const
    {
    auto thisPtr = shared_from_this();
    return GetServerInfo(false, ct)
        ->Then<WebApiResult>([this, thisPtr] (WSInfoResult& result)
        {
        if (!result.IsSuccess())
            {
            return WebApiResult::Error(result.GetError());
            }

        WSInfo& info = result.GetValue();
        BeCriticalSectionHolder webApiMutex(m_webApiCS);

        m_webApi = GetWebApi(info);

        if (nullptr == m_webApi)
            {
            return WebApiResult::Error(WSError::CreateServerNotSupportedError());
            }

        return WebApiResult::Success(m_webApi);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<WebApi> ClientConnection::GetWebApi(WSInfoCR info) const
    {
    if (WebApiV1::IsSupported(info))
        {
        if (nullptr != std::dynamic_pointer_cast<WebApiV1> (m_webApi))
            {
            return m_webApi;
            }
        return std::make_shared<WebApiV1>(m_configuration, info);
        }

    if (WebApiV1BentleyConect::IsSupported(info))
        {
        if (nullptr != std::dynamic_pointer_cast<WebApiV1BentleyConect> (m_webApi))
            {
            return m_webApi;
            }
        return std::make_shared<WebApiV1BentleyConect>(m_configuration, info);
        }

    if (WebApiV2::IsSupported(info))
        {
        if (nullptr != std::dynamic_pointer_cast<WebApiV2> (m_webApi))
            {
            return m_webApi;
            }
        return std::make_shared<WebApiV2>(m_configuration, info);
        }

    return nullptr;
    }
