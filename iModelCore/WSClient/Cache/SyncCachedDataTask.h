/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncCachedDataTask.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        bvector<IQueryProviderPtr>          m_queryProviders;

        bvector<ECInstanceKey>              m_initialInstances;
        std::deque<IQueryProvider::Query>   m_queriesToCache;
        bset<ObjectId>                      m_instancesToRedownload;
        bset<ECInstanceKey>                 m_filesToDownload;

        bset<ECInstanceKey>                     m_instancesWithQueriesProvided;
        std::shared_ptr<ECInstanceKeyMultiMap>  m_persistentInstances;

        ICachingDataSource::SyncProgressCallback m_onProgress;

        size_t m_syncedInitialInstances = 0;
        size_t m_syncedRejectedInstances = 0;
        size_t m_syncedQueries = 0;
        size_t m_totalQueries = 0;
        double m_syncedBytes = 0;
        double m_totalBytes = 0;

    protected:
        virtual void _OnExecute();

        void StartCaching();
        void CacheInitialInstances(CacheTransactionCR txn, const bset<ECInstanceKey>& instanceKeys);

        void ContinueCachingQueries(CacheTransactionCR txn);
        void PrepareCachingQueries(CacheTransactionCR txn, ECInstanceKeyCR instanceKey, bool syncRecursively);

        void InvalidatePersistentInstances();
        bool IsInstancePersistent(CacheTransactionCR txn, ECInstanceKeyCR instanceKey);

        void CacheRejectedInstances();
        void CacheFiles();

        void RegisterError(CacheTransactionCR txn, CachedResponseKeyCR responseKey, CachingDataSource::ErrorCR error);
        void ReportProgress(Utf8StringCR label = nullptr);

    public:
        SyncCachedDataTask
            (
            CachingDataSourcePtr ds,
            bvector<ECInstanceKey> initialInstances,
            bvector<IQueryProvider::Query> initialQueries,
            bvector<IQueryProviderPtr> queryProviders,
            ICachingDataSource::SyncProgressCallback onProgress,
            ICancellationTokenPtr ct
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
