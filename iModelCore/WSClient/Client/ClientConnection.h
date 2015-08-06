/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientConnection.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <MobileDgn/Utils/Threading/AsyncResult.h>
#include "ClientConfiguration.h"
#include "WebApi/WebApi.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

typedef AsyncResult<WebApiPtr, WSError> WebApiResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClientConnection : std::enable_shared_from_this<ClientConnection>
    {
    private:
        std::shared_ptr<ClientConfiguration> m_configuration;
        std::shared_ptr<struct ServerInfoProvider> m_infoProvider;

        mutable BeCriticalSection m_webApiCS;
        mutable std::shared_ptr<WebApi> m_webApi;

    private:
        AsyncTaskPtr<void> InvalidateInfo() const;
        std::shared_ptr<WebApi> GetWebApi(WSInfoCR info) const;

    public:
        ClientConnection(std::shared_ptr<ClientConfiguration> configuration);

        const ClientConfiguration& GetConfiguration() const;
        ClientConfiguration& GetConfiguration();

        void RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);
        void UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);

        AsyncTaskPtr<WebApiResult> GetWebApi
            (
            ICancellationTokenPtr cancellationToken
            ) const;

        AsyncTaskPtr<WSInfoResult> GetServerInfo
            (
            bool forceQuery,
            ICancellationTokenPtr cancellationToken
            ) const;

        template<typename R>
        AsyncTaskPtr<R> GetWebApiAndReturnResponse
            (
            std::function<AsyncTaskPtr<R>(WebApiPtr)> requestCallback,
            ICancellationTokenPtr cancellationToken
            ) const
            {
            auto thisPtr = shared_from_this();
            auto responsePtr = std::make_shared<R>();

            return GetWebApi(cancellationToken)
                ->Then([=] (WebApiResult& webApiResult)
                {
                if (!webApiResult.IsSuccess())
                    {
                    *responsePtr = R::Error(webApiResult);
                    return;
                    }

                requestCallback(webApiResult.GetValue())
                    ->Then([=] (R& response)
                    {
                    if (!response.IsSuccess() &&
                        WSError::Status::ServerNotSupported == response.GetError().GetStatus())
                        {
                        InvalidateInfo();
                        }
                    *responsePtr = response;
                    });
                })
                    ->template Then<R>([responsePtr, thisPtr]
                    {
                    return *responsePtr;
                    });
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
