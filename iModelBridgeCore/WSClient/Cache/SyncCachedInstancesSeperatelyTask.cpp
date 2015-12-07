/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/SyncCachedInstancesSeperatelyTask.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
ICancellationTokenPtr ct
) :
CachingTaskBase(ds, ct),
m_objectsLeftToCache(objects.begin(), objects.end())
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
        {
        return;
        }

    ObjectId objectId = m_objectsLeftToCache.front();
    m_objectsLeftToCache.pop_front();

    Utf8String cacheTag = txn.GetCache().ReadInstanceCacheTag(objectId);

    m_ds->GetClient()->SendGetObjectRequest(objectId, cacheTag, GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (WSObjectsResult& result)
        {
        if (IsTaskCanceled()) return;

        auto txn = m_ds->StartCacheTransaction();

        if (result.IsSuccess())
            {
            if (SUCCESS != txn.GetCache().UpdateInstance(objectId, result.GetValue()))
                {
                SetError();
                return;
                }
            }
        else if (result.GetError().GetId() == WSError::Id::InstanceNotFound ||
                 result.GetError().GetId() == WSError::Id::NotEnoughRights)
            {
            AddFailedObject(txn, objectId, result.GetError());
            if (CacheStatus::OK != txn.GetCache().RemoveInstance(objectId))
                {
                SetError();
                }
            }
        else
            {
            SetError(result.GetError());
            return;
            }

        CacheNextObjects(txn);
        txn.Commit();
        });
    }
