/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#include "SyncCachedDataTask.h"

#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Cache/ServerQueryHelper.h>
#include <algorithm>

#include "Logging.h"
#include "SyncCachedInstancesSeperatelyTask.h"
#include "SyncCachedInstancesTask.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SyncCachedDataTask::SyncCachedDataTask
(
CachingDataSourcePtr ds,
bvector<ECInstanceKey> initialInstances,
bvector<IQueryProvider::Query> initialQueries,
bvector<IQueryProviderPtr> queryProviders,
ICachingDataSource::ProgressHandler progressHandler,
ICancellationTokenPtr ct
) :
CachingTaskBase(ds, ct, SimpleCancellationToken::Create()),
m_queryProviders(queryProviders),
m_progressHandler(progressHandler)
    {
    for (auto& key : initialInstances)        
        m_initialInstances.push_back(Instance(key));

    for (auto& query : initialQueries)        
        m_queriesToCache.push_back(std::make_shared<CacheQuery>(query));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::ReportProgress(Utf8StringCPtr label)
    {
    size_t total = m_initialInstances.size() + m_totalQueries;
    size_t synced = m_syncedInitialInstances + m_syncedQueries;
    double progress = 0 == total ? 1 : (double) synced / (double) total;

    size_t syncedInstances = m_syncedInstancesToReportProgress.size();
    size_t totalInstances = m_instancesToReportProgress.size();

    if (totalInstances == 0)
        totalInstances = m_initialInstances.size();

    m_progressHandler.progressCallback({
        progress,
        {(double) syncedInstances, (double) totalInstances},
        m_downloadBytesProgress,
        label
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::_OnExecute()
    {
    m_ds->GetCacheAccessThread()->ExecuteAsync(std::bind(&SyncCachedDataTask::StartCaching, this))
        ->Then(m_ds->GetCacheAccessThread(), std::bind(&SyncCachedDataTask::CacheFiles, this));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::StartCaching()
    {
    m_totalQueries = m_queriesToCache.size();
    ReportProgress();

    if (IsTaskCanceled())
        return;

    auto txn = m_ds->StartCacheTransaction();
    
    bset<Instance> instancesToCache(m_initialInstances.begin(), m_initialInstances.end());
    CacheInitialInstances(txn, instancesToCache);
    txn.Commit();

    ContinueCachingQueries();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::CacheInitialInstances(CacheTransactionCR txn, const bset<Instance>& instanceKeys)
    {
    if (IsTaskCanceled() || instanceKeys.empty())
        return;

    bset<ObjectId> objectIds;
    for (auto& instance : instanceKeys)
        {
        ObjectId objectId = txn.GetCache().FindInstance(instance.key);
        if (!objectId.IsValid())
            {
            RegisterError(txn, instance.key, {ICachingDataSource::Status::DataNotCached});
            continue;
            }
        objectIds.insert(objectId);
        }
       
    auto onProgress = [=] (size_t synced, CacheTransactionCR txn, const bset<ECInstanceKey>& handled)
        {
        m_syncedInitialInstances = synced;

        ECInstanceKeyDeque cachedInstancesDeque;
        for (auto instance : instanceKeys)
            cachedInstancesDeque.push_back(instance.key);

        PrepareCachingQueriesAsync(cachedInstancesDeque, true);

        if (0 != synced)
            ReportProgress();
        };

    SyncCachedInstancesTask::Run(m_ds, objectIds, onProgress, GetUserCancellationToken(), GetAbortCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (ICachingDataSource::BatchResult result)
        {
        AddResult(result);

        if (IsTaskCanceled())
            return;

        ContinueCachingQueries();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::CacheFiles()
    {
    if (IsTaskCanceled() || m_filesToDownload.empty())
        return;

    auto txn = m_ds->StartCacheTransaction();

    bmap<ObjectId, ICancellationTokenPtr> filesToDownload;
    for (auto fileEntry : m_filesToDownload)
        {
        auto objectId = txn.GetCache().FindInstance(fileEntry.first);
        if (!objectId.IsValid())
            continue;
        filesToDownload.Insert(objectId, fileEntry.second);
        }

    txn.Commit();

    auto onProgress = [=] (CachingDataSource::ProgressCR progress)
        {
        m_downloadBytesProgress = progress.GetBytes();
        ReportProgress(progress.GetLabelPtr());
        };

    m_ds->DownloadAndCacheFiles(filesToDownload, FileCache::Auto, onProgress, GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (ICachingDataSource::BatchResult& result)
        {
        AddResult(result);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::ContinueCachingQueries()
    {
    if (IsTaskCanceled() || m_queriesToCache.empty())
        return;

    auto providedQuery = m_queriesToCache.front();
    m_queriesToCache.pop_front();

    if (!providedQuery->query.IsValid())
        {
        LOG.warningv("Invalid query provided");
        ContinueCachingQueries();
        return;
        }

    CachedResponseKey responseKey = providedQuery->query.key;
    WSQueryPtr query = providedQuery->query.query;
    bool syncRecursively = providedQuery->query.syncRecursively;

    auto ct = GetCancellationToken();

    m_ds->CacheObjects(responseKey, *query, CachingDataSource::DataOrigin::RemoteData, m_ds->GetInitialSkipToken(), 0, ct)
        ->Then(m_ds->GetCacheAccessThread(), [=] (CachingDataSource::DataOriginResult result)
        {
        if (IsTaskCanceled())
            return;

        ECInstanceKeyDeque cachedInstancesDeque;
        auto txn = m_ds->StartCacheTransaction();
        if (result.IsSuccess())
            {
            InvalidatePersistentInstances();

            ECInstanceKeyMultiMap cachedInstances;
            if (CacheStatus::OK != txn.GetCache().ReadResponseInstanceKeys(responseKey, cachedInstances))
                {
                SetError();
                return;
                }

            if (IsTaskCanceled())
                return;
            
            
            for (auto& pair : cachedInstances)
                cachedInstancesDeque.push_back(ECInstanceKey(pair.first, pair.second));

            }
        else
            {
            RegisterError(txn, responseKey, result.GetError());
            }

        txn.Commit();
        PrepareCachingQueriesAsync(cachedInstancesDeque, syncRecursively)
            ->Then(m_ds->GetCacheAccessThread(), [=]
            { 
            if (providedQuery->instance != nullptr)
                {
                providedQuery->instance->Remove(*providedQuery);
                if (providedQuery->instance->IsComplete())
                    {
                    auto instanceKey = providedQuery->instance->key;
                    auto txn = m_ds->StartCacheTransaction();
                    auto reportProgress = m_progressHandler.shouldReportInstanceProgress(instanceKey, txn);
                    txn.Commit();
                    if (reportProgress)
                        m_syncedInstancesToReportProgress.insert(instanceKey);
                    }
                }

            m_syncedQueries++;
            ReportProgress();

            ContinueCachingQueries();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::RegisterError(CacheTransactionCR txn, CachedResponseKeyCR responseKey, CachingDataSource::ErrorCR error)
    {
    if (WSError::Status::ReceivedError == error.GetWSError().GetStatus() ||
        ICachingDataSource::Status::DataNotCached == error.GetStatus())
        {
        AddFailedObject(txn.GetCache(), txn.GetCache().FindInstance(responseKey.GetParent()), error);
        return;
        }
    SetError(error);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::RegisterError(CacheTransactionCR txn, ECInstanceKeyCR instanceKey, CachingDataSource::ErrorCR error)
    {
    if (WSError::Status::ReceivedError == error.GetWSError().GetStatus() ||
        ICachingDataSource::Status::DataNotCached == error.GetStatus())
        {
        AddFailedObject(txn.GetCache(), txn.GetCache().FindInstance(instanceKey), error);
        return;
        }
    SetError(error);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Daumantas.Kojelis    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncCachedDataTask::PrepareCachingQueriesAsync(ECInstanceKeyDeque cachedInstancesDeque, bool syncRecursively)
    {
    if (IsTaskCanceled() || cachedInstancesDeque.empty())
        return CreateCompletedAsyncTask();

    auto instanceKey = cachedInstancesDeque.front();
    cachedInstancesDeque.pop_front();

    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        auto txn = m_ds->StartCacheTransaction();
        PrepareCachingQueries(txn, instanceKey, syncRecursively);
        txn.Commit();

        PrepareCachingQueriesAsync(cachedInstancesDeque, syncRecursively);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::PrepareCachingQueries(CacheTransactionCR txn, ECInstanceKeyCR instanceKey, bool syncRecursively)
    {
    if (m_instanceQueriesPrepared.find(instanceKey) != m_instanceQueriesPrepared.end())
        return;

    m_instanceQueriesPrepared.insert(instanceKey);

    auto reportProgress = m_progressHandler.shouldReportInstanceProgress(instanceKey, txn);
    if (reportProgress)
        m_instancesToReportProgress.insert(instanceKey);

    bool isPersistent = IsInstancePersistent(txn, instanceKey);
    auto instancePtr = std::make_shared<Instance>(instanceKey);

    for (auto queryProviderPtr : m_queryProviders)
        {
        if (syncRecursively)
            {
            auto queries = queryProviderPtr->GetQueries(txn, instanceKey, isPersistent);
            for (auto& query : queries)
                {
                auto cacheQueryPtr = std::make_shared<CacheQuery>(query);
                cacheQueryPtr->instance = instancePtr;
                instancePtr->cacheQueries.push_back(cacheQueryPtr);

                m_queriesToCache.insert(m_queriesToCache.end(), cacheQueryPtr);
                }
            }
        auto ct = queryProviderPtr->IsFileRetrievalNeeded(txn, instanceKey, isPersistent);
        if (ct)
            {
            auto it = m_filesToDownload.find(instanceKey);
            if (it == m_filesToDownload.end())
                m_filesToDownload.Insert(instanceKey, ct);
            else
                it->second = ConjunctiveCancellationToken::Create(it->second, ct);

            uint64_t fileSize = 0;
            txn.GetCache().ReadFileProperties(instanceKey, nullptr, &fileSize);
            m_downloadBytesProgress.total += fileSize;
            }
        }

    size_t instanceQueriesCount = instancePtr->cacheQueries.size();
    m_totalQueries += instanceQueriesCount;
    if (instanceQueriesCount == 0 && reportProgress)
        m_syncedInstancesToReportProgress.insert(instancePtr->key);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::InvalidatePersistentInstances()
    {
    m_persistentInstances = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncCachedDataTask::IsInstancePersistent(CacheTransactionCR txn, ECInstanceKeyCR instanceKey)
    {
    if (nullptr == m_persistentInstances)
        {
        m_persistentInstances = std::make_shared<ECInstanceKeyMultiMap>();
        if (SUCCESS != txn.GetCache().ReadFullyPersistedInstanceKeys(*m_persistentInstances))
            SetError();
        }

    return ECDbHelper::IsInstanceInMultiMap(instanceKey, *m_persistentInstances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vilius.Kazlauskas    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::Instance::Remove(CacheQuery& queryToRemove)
    {
    auto isEqual = [&] (const std::weak_ptr<CacheQuery> queryWeak)
        {
        auto queryPtr = queryWeak.lock();
        if (!queryPtr)
            return false;
        return queryToRemove.query.key == queryPtr->query.key;
        };
    cacheQueries.erase(std::remove_if(cacheQueries.begin(), cacheQueries.end(), isEqual), cacheQueries.end());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vilius.Kazlauskas    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncCachedDataTask::Instance::IsComplete()
    {
    return cacheQueries.empty();
    }
