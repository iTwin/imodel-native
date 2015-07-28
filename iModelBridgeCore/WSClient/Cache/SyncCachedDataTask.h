/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncCachedDataTask.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

        ICachingDataSource::ProgressCallback    m_onProgress;

    protected:
        virtual void _OnExecute ();

        void StartCaching ();
        void CacheInstances (CacheTransactionCR txn, const bset<ECInstanceKey>& instanceKeys);

        void ContinueCachingQueries (CacheTransactionCR txn);
        void PrepareCachingQueries (CacheTransactionCR txn, ECInstanceKeyCR instanceKey, bool syncRecursively);

        void InvalidatePersistentInstances ();
        bool IsInstancePersistent (CacheTransactionCR txn, ECInstanceKeyCR instanceKey);

        void CacheRejectedInstances ();
        void CacheFiles ();

        void RegisterError (CacheTransactionCR txn, CachedResponseKeyCR responseKey, WSErrorCR error);

    public:
        SyncCachedDataTask
            (
            CachingDataSourcePtr ds,
            bvector<ECInstanceKey> initialInstances,
            bvector<IQueryProvider::Query> initialQueries,
            bvector<IQueryProviderPtr> queryProviders,
            ICachingDataSource::ProgressCallback onProgress,
            ICancellationTokenPtr cancellationToken
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
