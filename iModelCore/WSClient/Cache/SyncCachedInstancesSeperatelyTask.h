/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/SyncCachedInstancesSeperatelyTask.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/CachingDataSource.h>

#include "CachingTaskBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct SyncCachedInstancesSeperatelyTask : public CachingTaskBase
    {
    private:
        std::deque<ObjectId>  m_objectsLeftToCache;
        ProgressCallback      m_onProgress;
        size_t                m_totalToCache;

    private:
        virtual void _OnExecute();
        void CacheNextObjects(CacheTransactionCR txn);

    public:
        SyncCachedInstancesSeperatelyTask
            (
            CachingDataSourcePtr ds,
            const bset<ObjectId>& objects,
            ProgressCallback onProgress,
            ICancellationTokenPtr ct
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
