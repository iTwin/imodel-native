/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncCachedDataTask.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/CachingDataSource.h>
#include "CachingTaskBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct SyncCachedDataTask : public CachingTaskBase
    {
    private:
        struct CacheQuery;

        struct Instance
            {
            ECInstanceKey key;
            bvector<std::weak_ptr<CacheQuery>> cacheQueries;

            Instance() {};
            Instance(ECInstanceKey key) : key(key) {};

            void Remove(CacheQuery& node);
            bool IsComplete();

            bool operator==(const Instance& other) const { return key == other.key; }
            bool operator!=(const Instance& other) const { return key != other.key; }
            bool operator<(const Instance& other) const { return key < other.key; }
            };

        struct CacheQuery
            {
            IQueryProvider::Query query;
            std::shared_ptr<Instance> instance;

            CacheQuery() {};
            CacheQuery(IQueryProvider::Query query) : query(query) {};
            };
        
    private:
        bvector<IQueryProviderPtr> m_queryProviders;

        bvector<Instance> m_initialInstances;
        std::deque<std::shared_ptr<CacheQuery>> m_queriesToCache;

        bset<ECInstanceKey> m_instancesToReportProgress;
        bset<ECInstanceKey> m_syncedInstancesToReportProgress;

        bmap<ECInstanceKey, ICancellationTokenPtr> m_filesToDownload;

        bset<ECInstanceKey> m_instanceQueriesPrepared;
        std::shared_ptr<ECInstanceKeyMultiMap> m_persistentInstances;             

        ICachingDataSource::ProgressHandler m_progressHandler;

        size_t m_syncedInitialInstances = 0;
        size_t m_syncedQueries = 0;
        size_t m_totalQueries = 0;
        CachingDataSource::Progress::State m_downloadBytesProgress;

    protected:
        virtual void _OnExecute();

        void StartCaching();
        void CacheInitialInstances(CacheTransactionCR txn, const bset<Instance>& instanceKeys);

        void ContinueCachingQueries(CacheTransactionCR txn);
        void PrepareCachingQueries(CacheTransactionCR txn, ECInstanceKeyCR instanceKey, bool syncRecursively);

        void InvalidatePersistentInstances();
        bool IsInstancePersistent(CacheTransactionCR txn, ECInstanceKeyCR instanceKey);

        void CacheFiles();

        void RegisterError(CacheTransactionCR txn, CachedResponseKeyCR responseKey, CachingDataSource::ErrorCR error);
        void RegisterError(CacheTransactionCR txn, ECInstanceKeyCR instanceKey, CachingDataSource::ErrorCR error);
        void ReportProgress(Utf8StringCPtr label = nullptr);

    public:
        SyncCachedDataTask
            (
            CachingDataSourcePtr ds,
            bvector<ECInstanceKey> initialInstances,
            bvector<IQueryProvider::Query> initialQueries,
            bvector<IQueryProviderPtr> queryProviders,
            ICachingDataSource::ProgressHandler progressHandler,
            ICancellationTokenPtr ct
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
