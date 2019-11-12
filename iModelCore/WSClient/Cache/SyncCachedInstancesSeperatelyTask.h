/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/CachingDataSource.h>

#include "CachingTaskBase.h"
#include "SyncCachedInstancesTask.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct SyncCachedInstancesSeperatelyTask : public CachingTaskBase
    {
    private:
        std::deque<ObjectId> m_objectsLeftToCache;
        SyncCachedInstancesTask::ProgressCallback m_onProgress;
        size_t m_totalToCache;

    private:
        virtual void _OnExecute();
        void CacheNextObjects(CacheTransactionCR txn);

    public:
        SyncCachedInstancesSeperatelyTask
            (
            CachingDataSourcePtr ds,
            const bset<ObjectId>& objects,
            SyncCachedInstancesTask::ProgressCallback onProgress,
            ICancellationTokenPtr userCt,
            SimpleCancellationTokenPtr abortCt
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
