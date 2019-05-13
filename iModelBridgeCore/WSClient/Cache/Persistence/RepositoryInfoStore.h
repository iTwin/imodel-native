/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/Tasks/AsyncTask.h>
#include <WebServices/Cache/Persistence/IRepositoryInfoStore.h>
#include <WebServices/Cache/Transactions/ICacheTransactionManager.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <WebServices/Client/WSRepositoryClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryInfoStore : public IRepositoryInfoStore
    {
    private:
        struct InfoListener;
        struct RepositoryInfoListener;

    private:
        ICacheTransactionManager* m_cacheTxnManager;
        IWSRepositoryClientPtr m_client;
        std::shared_ptr<InfoListener> m_infoListener;
        std::shared_ptr<RepositoryInfoListener> m_repositoryInfoListener;
        WorkerThreadPtr m_thread;

        BeMutex m_infoMutex;
        WSInfo m_info;
        WSRepository m_repositoryInfo;

    private:
        WSInfo ReadServerInfo(IDataSourceCache& cache);
        WSRepository ReadRepositoryInfo(IDataSourceCache& cache);

    public:
        //! Create info cache for client and cache db
        RepositoryInfoStore(ICacheTransactionManager* cacheTxnManager, IWSRepositoryClientPtr client, WorkerThreadPtr thread);
        virtual ~RepositoryInfoStore();
        //! Cache server info asynchronously. ICacheTransactionManager and WorkerThread must be specified.
        void CacheServerInfo(WSInfoCR info);
        BentleyStatus CacheServerInfo(IDataSourceCache& cache, WSInfoCR info) override;
        void CacheRepositoryInfo(WSRepositoryCR info);
        BentleyStatus CacheRepositoryInfo(IDataSourceCache& cache, WSRepositoryCR info) override;

        BentleyStatus PrepareInfo(IDataSourceCache& cache) override;
        WSInfo GetServerInfo() override;
        WSRepository GetRepositoryInfo() override;

        BentleyStatus SetCacheInitialized(IDataSourceCache& cache) override;
        bool IsCacheInitialized(IDataSourceCache& cache) override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryInfoStore::InfoListener : public IWSClient::IServerInfoListener, public IWSRepositoryClient::IRepositoryInfoListener
    {
    private:
        RepositoryInfoStore* m_infoStore;

    public:
        InfoListener(RepositoryInfoStore* infoStore) : m_infoStore(infoStore) {};

        void OnServerInfoReceived(WSInfoCR info) override
            {
            m_infoStore->CacheServerInfo(info);
            };

        void OnInfoReceived(WSRepositoryCR info) override
            {
            m_infoStore->CacheRepositoryInfo(info);
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
