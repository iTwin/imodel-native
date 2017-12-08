/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ServerInfoProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSClient.h>
#include <MobileDgn/Utils/Threading/WorkerThread.h>
#include <MobileDgn/Utils/Threading/AsyncTask.h>
#include <MobileDgn/Utils/Threading/UniqueTaskHolder.h>
#include <set>

#include "ClientConnection.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

typedef AsyncResult<WSInfo, HttpResponse> WSInfoHttpResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ServerInfoProvider
    {
    private:
        BeCriticalSection m_mutex;
        UniqueTaskHolder<WSInfoResult> m_getInfoExecutor;

        std::shared_ptr<const ClientConfiguration> m_configuration;
        std::vector<std::weak_ptr<IWSClient::IServerInfoListener>> m_listeners;

        WSInfo m_serverInfo;
        uint64_t m_serverInfoUpdated;

    private:
        bool CanUseCachedInfo() const;
        void UpdateInfo(WSInfoCR info);
        void NotifyServerInfoUpdated(WSInfoCR info) const;

        AsyncTaskPtr<WSInfoResult> GetInfo(ICancellationTokenPtr ct) const;
        AsyncTaskPtr<WSInfoHttpResult> GetInfoFromPage(Utf8StringCR page, ICancellationTokenPtr ct) const;

    public:
        ServerInfoProvider(std::shared_ptr<const ClientConfiguration> configuration);
        ~ServerInfoProvider();

        void RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);
        void UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);

        AsyncTaskPtr<WSInfoResult> GetServerInfo(bool forceQuery, ICancellationTokenPtr ct);
        void InvalidateInfo();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
