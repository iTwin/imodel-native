/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ServerInfoProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSClient.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/UniqueTaskHolder.h>
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
        BeMutex m_mutex;
        UniqueTaskHolder<WSInfoResult> m_getInfoExecutor;

        std::shared_ptr<const ClientConfiguration> m_configuration;
        std::vector<std::weak_ptr<IWSClient::IServerInfoListener>> m_listeners;

        mutable WSInfo m_serverInfo;
        mutable uint64_t m_serverInfoUpdated;

        //! Note: Temporary until WSG defect 651740 is fixed for BIMReviewSharing
        bool m_enableWsgServerHeader;

    private:
        bool CanUseCachedInfo() const;
        void UpdateInfo(WSInfoCR info);
        void NotifyServerInfoUpdated(WSInfoCR info) const;

        AsyncTaskPtr<WSInfoResult> GetInfo(ICancellationTokenPtr ct) const;
        AsyncTaskPtr<WSInfoHttpResult> GetInfoFromPage(Utf8StringCR page, ICancellationTokenPtr ct) const;

    public:
        ServerInfoProvider(std::shared_ptr<const ClientConfiguration> configuration);
        ~ServerInfoProvider();

        //! Note: Temporary until WSG defect 651740 is fixed for BIMReviewSharing
        void EnableWsgServerHeader(bool enable);

        void RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);
        void UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener);

        AsyncTaskPtr<WSInfoResult> GetServerInfo(bool forceQuery, ICancellationTokenPtr ct);
        void InvalidateInfo();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
