/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ServerInfoProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSClient.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <set>

#include "ClientConnection.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef AsyncResult<WSInfo, Http::Response> WSInfoHttpResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ServerInfoProvider
    {
    private:
        std::shared_ptr<const ClientConfiguration> m_configuration;
        std::shared_ptr<WorkerThread> m_thread;
        std::vector<std::weak_ptr<IWSClient::IServerInfoListener>> m_listeners;

        mutable WSInfo m_serverInfo;
        mutable uint64_t m_serverInfoUpdated;

    private:
        bool CanUseCachedInfo() const;
        void UpdateInfo(WSInfoCR info) const;
        void NotifyServerInfoUpdated(WSInfoCR info) const;

        AsyncTaskPtr<WSInfoResult> GetInfo(ICancellationTokenPtr ct) const;
        AsyncTaskPtr<WSInfoHttpResult> GetInfoFromPage(Utf8StringCR page, ICancellationTokenPtr ct) const;

    public:
        ServerInfoProvider(std::shared_ptr<const ClientConfiguration> configuration);
        ~ServerInfoProvider();

        void RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);
        void UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);

        AsyncTaskPtr<WSInfoResult> GetServerInfo(bool forceQuery, ICancellationTokenPtr ct) const;
        AsyncTaskPtr<void> InvalidateInfo() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
