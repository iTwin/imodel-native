/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "SyncCachedInstancesTask.h"

#include "SyncCachedInstancesSeperatelyTask.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SyncCachedInstancesTask::SyncCachedInstancesTask
(
CachingDataSourcePtr ds,
const bset<ObjectId>& objects,
ProgressCallback onProgress,
ICancellationTokenPtr userCt,
SimpleCancellationTokenPtr abortCt
) :
CachingTaskBase(ds, userCt, abortCt),
m_objectsToCache(objects.begin(), objects.end()),
m_objectsLeftToCache(objects.begin(), objects.end()),
m_onProgress(onProgress ? onProgress : [] (size_t, CacheTransactionCR, const bset<ECInstanceKey>&) {})
    {
    std::sort(m_objectsLeftToCache.begin(), m_objectsLeftToCache.end());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedInstancesTask::_OnExecute()
    {
    m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        auto txn = m_ds->StartCacheTransaction();
        m_onProgress(m_cachedInstances.size(), txn, bset<ECInstanceKey>());
        txn.Commit();
        })
    ->Then(m_ds->GetCacheAccessThread(), std::bind(&SyncCachedInstancesTask::CacheNextObjects, this))
    ->Then(m_ds->GetCacheAccessThread(), std::bind(&SyncCachedInstancesTask::ResolveNotFoundInstances, this));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedInstancesTask::CacheNextObjects()
    {
    if (IsTaskCanceled() || m_objectsLeftToCache.empty())
        return;

    auto cancellationtoken = GetCancellationToken();

    m_ds->GetClient()->SendQueryRequest(GetQuery(), nullptr, nullptr, cancellationtoken)
        ->Then(m_ds->GetCacheAccessThread(), [=] (WSObjectsResult& result)
        {
        if (IsTaskCanceled()) return;

        if (!result.IsSuccess() || !result.GetValue().IsModified())
            {
            SetError(result.GetError());
            return;
            }

        auto txn = m_ds->StartCacheTransaction();
        bset<ECInstanceKey> cachedInstances;
        if (SUCCESS != txn.GetCache().UpdateInstances(result.GetValue(), nullptr, &cachedInstances, cancellationtoken))
            {
            SetError({CachingDataSource::Status::InternalCacheError, cancellationtoken});
            return;
            }

        m_cachedInstances.insert(cachedInstances.begin(), cachedInstances.end());
        m_onProgress(m_cachedInstances.size(), txn, cachedInstances);

        txn.Commit();

        CacheNextObjects();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery SyncCachedInstancesTask::GetQuery()
    {
    WSQuery query(m_objectsLeftToCache.front().schemaName, m_objectsLeftToCache.front().className);
    query.AddFilterIdsIn(m_objectsLeftToCache);

    return query;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedInstancesTask::ResolveNotFoundInstances()
    {
    if (IsTaskCanceled() || m_cachedInstances.size() == m_objectsToCache.size())
        {
        return;
        }

    auto txn = m_ds->StartCacheTransaction();

    bset<ObjectId> cachedObjectIds;
    for (ECInstanceKeyCR instanceKey : m_cachedInstances)
        {
        ObjectId objectId = txn.GetCache().FindInstance(instanceKey);
        if (objectId.IsValid())
            {
            cachedObjectIds.insert(objectId);
            }
        }

    bset<ObjectId> objectsToResolve;
    for (ObjectIdCR objectId : m_objectsToCache)
        {
        if (cachedObjectIds.find(objectId) != cachedObjectIds.end())
            {
            continue;
            }
        objectsToResolve.insert(objectId);
        }

    auto onProgress = [=] (size_t synced, CacheTransactionCR txn, const bset<ECInstanceKey>& handled)
        {
        m_onProgress(m_cachedInstances.size() + synced, txn, handled);
        };

    auto task = std::make_shared<SyncCachedInstancesSeperatelyTask>(
        m_ds, objectsToResolve, onProgress, GetUserCancellationToken(), GetAbortCancellationToken());

    m_ds->GetCacheAccessThread()->Push(task);
    task->Then([=]
        {
        AddResult(task->GetResult());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> SyncCachedInstancesTask::Run
(
CachingDataSourcePtr ds,
const bset<ObjectId>& instanceIds,
ProgressCallback onProgress,
ICancellationTokenPtr userCt,
SimpleCancellationTokenPtr abortCt
)
    {
    if (instanceIds.empty())
        {
        return CreateCompletedAsyncTask(CachingDataSource::BatchResult::Success(ICachingDataSource::FailedObjects()));
        }

    auto finalResult = std::make_shared<CachingDataSource::BatchResult>();

    return ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        std::shared_ptr<CachingTaskBase> task;
        if (ds->GetServerInfo(ds->StartCacheTransaction()).GetVersion() < BeVersion(2, 0))
            {
            task = std::shared_ptr<CachingTaskBase>(
                new SyncCachedInstancesSeperatelyTask(ds, instanceIds, onProgress, userCt, abortCt));
            }
        else
            {
            task = std::shared_ptr<CachingTaskBase>(
                new SyncCachedInstancesTask(ds, instanceIds, onProgress, userCt, abortCt));
            }

        ds->GetCacheAccessThread()->Push(task);
        task->Then([=]
            {
            *finalResult = task->GetResult();
            });
        })
            ->Then<CachingDataSource::BatchResult>([=]
            {
            return *finalResult;
            });
    }
