/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "SyncCachedInstancesSeperatelyTask.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SyncCachedInstancesSeperatelyTask::SyncCachedInstancesSeperatelyTask
(
CachingDataSourcePtr ds,
const bset<ObjectId>& objects,
SyncCachedInstancesTask::ProgressCallback onProgress,
ICancellationTokenPtr ct
) :
CachingTaskBase(ds, ct),
m_objectsLeftToCache(objects.begin(), objects.end()),
m_onProgress(onProgress ? onProgress : [] (size_t, CacheTransactionCR, const bset<ECInstanceKey>&) {}),
m_totalToCache(m_objectsLeftToCache.size())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedInstancesSeperatelyTask::_OnExecute()
    {
    m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled() || m_objectsLeftToCache.empty())
            {
            return;
            }
        auto txn = m_ds->StartCacheTransaction();
        m_onProgress(0, txn, bset<ECInstanceKey>());
        CacheNextObjects(txn);
        txn.Commit();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedInstancesSeperatelyTask::CacheNextObjects(CacheTransactionCR txn)
    {
    if (IsTaskCanceled() || m_objectsLeftToCache.empty())
        return;

    ObjectId objectId = m_objectsLeftToCache.front();
    m_objectsLeftToCache.pop_front();

    Utf8String cacheTag = txn.GetCache().ReadInstanceCacheTag(objectId);

    m_ds->GetClient()->SendGetObjectRequest(objectId, cacheTag, GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (WSObjectsResult& result)
        {
        if (IsTaskCanceled()) return;

        auto txn = m_ds->StartCacheTransaction();

        bset<ECInstanceKey> cachedInstances;

        if (result.IsSuccess())
            {
            auto status = txn.GetCache().UpdateInstance(objectId, result.GetValue());
            if (CacheStatus::OK != status &&
                CacheStatus::DataNotCached != status)
                {
                SetError();
                return;
                }

            if (CacheStatus::DataNotCached == status)
                AddFailedObject(txn.GetCache(), objectId, status);
            else
                cachedInstances.insert(txn.GetCache().FindInstance(objectId));
            }

        else if (result.GetError().IsInstanceNotAvailableError())
            {
            AddFailedObject(txn.GetCache(), objectId, result.GetError());
            auto status = txn.GetCache().RemoveInstance(objectId);
            if (CacheStatus::OK != status &&
                CacheStatus::DataNotCached != status)
                {
                SetError();
                }
            }
        else
            {
            SetError(result.GetError());
            return;
            }

        m_onProgress(m_totalToCache - m_objectsLeftToCache.size(), txn, cachedInstances);

        CacheNextObjects(txn);
        txn.Commit();
        });
    }
