/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientConnection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/Tasks/AsyncResult.h>
#include "ClientConfiguration.h"
#include "WebApi/WebApi.h"
#include "ServerInfoProvider.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef AsyncResult<WebApiPtr, WSError> WebApiResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClientConnection : std::enable_shared_from_this<ClientConnection>
    {
    private:
        std::shared_ptr<ClientConfiguration> m_configuration;
        std::shared_ptr<ServerInfoProvider> m_infoProvider;

        mutable BeMutex m_webApiCS;
        mutable std::shared_ptr<WebApi> m_webApi;

    private:
        std::shared_ptr<WebApi> GetWebApi(WSInfoCR info) const;

    public:
        ClientConnection(std::shared_ptr<ClientConfiguration> configuration);

        const ClientConfiguration& GetConfiguration() const;
        ClientConfiguration& GetConfiguration();

        //! Note: Temporary until WSG defect 651740 is fixed for BIMReviewSharing
        void EnableWsgServerHeader(bool enable);

        void RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);
        void UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);

        AsyncTaskPtr<WebApiResult> GetWebApi
            (
            ICancellationTokenPtr ct
            ) const;

        AsyncTaskPtr<WSInfoResult> GetServerInfo
            (
            bool forceQuery,
            ICancellationTokenPtr ct
            ) const;

        template<typename R>
        AsyncTaskPtr<R> GetWebApiAndReturnResponse
            (
            std::function<AsyncTaskPtr<R>(WebApiPtr)> requestCallback,
            ICancellationTokenPtr ct
            ) const
            {
            auto thisPtr = shared_from_this();
            auto responsePtr = std::make_shared<R>();

            return GetWebApi(ct)
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
                        m_infoProvider->InvalidateInfo();
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
