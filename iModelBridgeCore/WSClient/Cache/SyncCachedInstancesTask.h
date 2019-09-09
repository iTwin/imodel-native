/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/CachingDataSource.h>

#include "CachingTaskBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct SyncCachedInstancesTask : public CachingTaskBase
    {
    public:
        typedef std::function<void(size_t, CacheTransactionCR, const bset<ECInstanceKey>&)> ProgressCallback;
    
    private:
        const bset<ObjectId>  m_objectsToCache;
        std::deque<ObjectId>  m_objectsLeftToCache;
        bset<ECInstanceKey>   m_cachedInstances;
        ProgressCallback      m_onProgress;

    private:
        SyncCachedInstancesTask
            (
            CachingDataSourcePtr ds,
            const bset<ObjectId>& objects,
            ProgressCallback onProgress,
            ICancellationTokenPtr userCt,
            SimpleCancellationTokenPtr abortCt
            );

        virtual void _OnExecute();
        void CacheNextObjects();
        WSQuery GetQuery();
        void ResolveNotFoundInstances();

    public:
        // Will run task depending on server version
        static AsyncTaskPtr<CachingDataSource::BatchResult> Run
            (
            CachingDataSourcePtr ds,
            const bset<ObjectId>& instanceIds,
            ProgressCallback onProgress,
            ICancellationTokenPtr userCt,
            SimpleCancellationTokenPtr abortCt
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
