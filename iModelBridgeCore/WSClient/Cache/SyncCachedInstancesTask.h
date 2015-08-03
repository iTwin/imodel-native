/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/SyncCachedInstancesTask.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    private:
        const bset<ObjectId>  m_objectsToCache;
        std::deque<ObjectId>  m_objectsLeftToCache;
        bset<ECInstanceKey>   m_cachedInstances;

    private:
        SyncCachedInstancesTask
            (
            CachingDataSourcePtr ds,
            const bset<ObjectId>& objects,
            ICancellationTokenPtr ct
            );

        virtual void _OnExecute ();
        void CacheNextObjects ();
        WSQuery GetQuery ();
        void ResolveNotFoundInstances ();

    public:
        // Will run task depending on server version
        static AsyncTaskPtr<CachingDataSource::BatchResult> Run
            (
            CachingDataSourcePtr ds,
            const bset<ObjectId>& instanceIds,
            ICancellationTokenPtr ct
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
