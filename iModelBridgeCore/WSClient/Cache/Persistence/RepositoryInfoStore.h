/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/RepositoryInfoStore.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <MobileDgn/Utils/Threading/AsyncTask.h>
#include <WebServices/Cache/Persistence/IRepositoryInfoStore.h>
#include <WebServices/Cache/Transactions/ICacheTransactionManager.h>
#include <MobileDgn/Utils/Threading/WorkerThread.h>
#include <WebServices/Client/WSRepositoryClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryInfoStore : public IRepositoryInfoStore
    {
    private:
        struct InfoListener;

    private:
        ICacheTransactionManager* m_cacheTxnManager;
        IWSRepositoryClientPtr m_client;
        std::shared_ptr<InfoListener> m_infoListener;
        WorkerThreadPtr m_thread;
        WSInfo m_cachedInfo;

    private:
        WSInfo ReadServerInfo(IDataSourceCache& cache);

    public:
        //! Create info cache for client and cache db
        RepositoryInfoStore(ICacheTransactionManager* cacheTxnManager, IWSRepositoryClientPtr client, WorkerThreadPtr thread);
        virtual ~RepositoryInfoStore();

        //! Cache server info asynchronously. ICacheTransactionManager and WorkerThread must be specified.
        void CacheServerInfo(WSInfoCR info);

        BentleyStatus CacheServerInfo(IDataSourceCache& cache, WSInfoCR info) override;
        WSInfoCR GetServerInfo(IDataSourceCache& cache) override;

        BentleyStatus SetCacheInitialized(IDataSourceCache& cache) override;
        bool IsCacheInitialized(IDataSourceCache& cache) override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryInfoStore::InfoListener : public IWSClient::IServerInfoListener
    {
    private:
        RepositoryInfoStore* m_infoStore;

    public:
        InfoListener(RepositoryInfoStore* infoStore) : m_infoStore(infoStore)
            {};

        void OnServerInfoReceived(WSInfoCR info) override
            {
            m_infoStore->CacheServerInfo(info);
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
