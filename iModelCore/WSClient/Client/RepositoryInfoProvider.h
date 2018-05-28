/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/RepositoryInfoProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSRepositoryClient.h>
#include <MobileDgn/Utils/Threading/WorkerThread.h>
#include <MobileDgn/Utils/Threading/AsyncTask.h>
#include <MobileDgn/Utils/Threading/UniqueTaskHolder.h>
#include <set>

#include "ClientConnection.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryInfoProvider
    {
    private:
        BeCriticalSection m_mutex;
        UniqueTaskHolder<WSRepositoryResult> m_getInfoExecutor;

        std::shared_ptr<const ClientConnection> m_connection;
        std::vector<std::weak_ptr<IWSRepositoryClient::IRepositoryInfoListener>> m_listeners;

        WSRepository m_info;
        uint64_t m_infoUpdated;

    private:
        bool CanUseCachedInfo() const;
        void UpdateInfo(WSRepositoryCR info);
        void NotifyInfoUpdated(WSRepositoryCR info) const;

        AsyncTaskPtr<WSRepositoryResult> GetRepository(ICancellationTokenPtr ct) const;

    public:
        RepositoryInfoProvider(std::shared_ptr<const ClientConnection> connection);
        ~RepositoryInfoProvider();

        void RegisterInfoListener(std::weak_ptr<IWSRepositoryClient::IRepositoryInfoListener> listener);
        void UnregisterInfoListener(std::weak_ptr<IWSRepositoryClient::IRepositoryInfoListener> listener);

        AsyncTaskPtr<WSRepositoryResult> GetInfo(ICancellationTokenPtr ct);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
