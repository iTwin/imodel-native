/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSClient.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/UniqueTaskHolder.h>
#include <map>
#include <set>

#include "ClientConfiguration.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef AsyncResult<WSInfo, Http::Response> WSInfoHttpResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ServerInfoProvider
    {
    private:
        static std::map<Utf8String, std::pair<WSInfo, uint64_t>> s_serverInfo;
        static BeMutex s_mutex;

        BeMutex m_mutex;
        UniqueTaskHolder<WSInfoResult> m_getInfoExecutor;

        std::shared_ptr<const ClientConfiguration> m_configuration;
        std::vector<std::weak_ptr<IWSClient::IServerInfoListener>> m_listeners;

    private:
        bool CanUseCachedInfo() const;
        void UpdateInfo(WSInfoCR info) const;
        void NotifyServerInfoUpdated(WSInfoCR info) const;

        AsyncTaskPtr<WSInfoResult> GetInfo(ICancellationTokenPtr ct) const;
        AsyncTaskPtr<WSInfoResult> GetInfo(BeVersionCR serviceVersion, ICancellationTokenPtr ct) const;
        AsyncTaskPtr<WSInfoHttpResult> GetInfoFromPage(Utf8StringCR page, BeVersionCR serviceVersion, ICancellationTokenPtr ct) const;

    public:
        ServerInfoProvider(std::shared_ptr<const ClientConfiguration> configuration);
        ~ServerInfoProvider();

        void RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);
        void UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);

        AsyncTaskPtr<WSInfoResult> GetServerInfo(bool forceQuery, ICancellationTokenPtr ct);
        void InvalidateInfo() const;
        WSCLIENT_EXPORT static void InvalidateAllInfo();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
