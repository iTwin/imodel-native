/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncCachedDataTask.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "SyncCachedDataTask.h"

#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Cache/ServerQueryHelper.h>

#include "Logging.h"
#include "DownloadFilesTask.h"
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
ICachingDataSource::ProgressCallback onProgress,
ICancellationTokenPtr ct
) :
CachingTaskBase(ds, ct),
m_queryProviders(queryProviders),
m_initialInstances(initialInstances),
m_queriesToCache(initialQueries.begin(), initialQueries.end()),
m_onProgress(onProgress ? onProgress : [] (double, double)
    {})
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::_OnExecute()
    {
    m_ds->GetCacheAccessThread()->ExecuteAsync(std::bind(&SyncCachedDataTask::StartCaching, this))
        ->Then(m_ds->GetCacheAccessThread(), std::bind(&SyncCachedDataTask::CacheRejectedInstances, this))
        ->Then(m_ds->GetCacheAccessThread(), std::bind(&SyncCachedDataTask::CacheFiles, this));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::StartCaching()
    {
    m_onProgress(0, 0);

    if (IsTaskCanceled())
        {
        return;
        }

    auto txn = m_ds->StartCacheTransaction();

    bset<ECInstanceKey> instancesToCache(m_initialInstances.begin(), m_initialInstances.end());
    CacheInstances(txn, instancesToCache);
    ContinueCachingQueries(txn);

    txn.Commit();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::CacheInstances(CacheTransactionCR txn, const bset<ECInstanceKey>& instanceKeys)
    {
    if (IsTaskCanceled() || instanceKeys.empty())
        {
        return;
        }

    bset<ObjectId> objectIds;
    for (ECInstanceKeyCR instanceKey : instanceKeys)
        {
        objectIds.insert(txn.GetCache().FindInstance(instanceKey));
        }

    SyncCachedInstancesTask::Run(m_ds, objectIds, GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (ICachingDataSource::BatchResult result)
        {
        AddResult(result);

        if (IsTaskCanceled())
            {
            return;
            }

        auto txn = m_ds->StartCacheTransaction();
        for (ECInstanceKeyCR instnaceKey : instanceKeys)
            {
            PrepareCachingQueries(txn, instnaceKey, true);
            }

        ContinueCachingQueries(txn);
        txn.Commit();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::CacheRejectedInstances()
    {
    if (IsTaskCanceled() || m_instancesToRedownload.empty())
        {
        return;
        }

    SyncCachedInstancesTask::Run(m_ds, m_instancesToRedownload, GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (ICachingDataSource::BatchResult result)
        {
        AddResult(result);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::CacheFiles()
    {
    if (IsTaskCanceled() || m_filesToDownload.empty())
        {
        return;
        }

    auto txn = m_ds->StartCacheTransaction();

    bset<ObjectId> filesToDownload;
    for (ECInstanceKeyCR instanceKey : m_filesToDownload)
        {
        filesToDownload.insert(txn.GetCache().FindInstance(instanceKey));
        }

    txn.Commit();

    auto onProgress = [=] (double bytesTransfered, double bytesTotal, Utf8StringCR fileName)
        {
        m_onProgress(bytesTransfered, bytesTotal);
        };

    auto task = std::make_shared<DownloadFilesTask>(m_ds, filesToDownload, FileCache::Persistent, onProgress, GetCancellationToken());
    m_ds->GetCacheAccessThread()->Push(task);

    task->Then(m_ds->GetCacheAccessThread(), [=]
        {
        AddResult(task->GetResult());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::ContinueCachingQueries(CacheTransactionCR txn)
    {
    if (IsTaskCanceled() || m_queriesToCache.empty())
        {
        return;
        }

    auto providerQuery = m_queriesToCache.front();
    m_queriesToCache.pop_front();

    if (!providerQuery.IsValid())
        {
        LOG.warningv("Invalid query provided");
        ContinueCachingQueries(txn);
        return;
        }

    CachedResponseKey responseKey = providerQuery.key;
    WSQueryPtr query = providerQuery.query;
    bool syncRecursively = providerQuery.syncRecursively;

    auto ct = GetCancellationToken();

    m_ds->CacheObjects(responseKey, *query, CachingDataSource::DataOrigin::RemoteData, IWSRepositoryClient::InitialSkipToken, 0, ct)
    ->Then(m_ds->GetCacheAccessThread(), [=] (CachingDataSource::DataOriginResult result)
        {
        if (IsTaskCanceled())
            {
            return;
            }

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
                {
                return;
                }

            for (auto& pair : cachedInstances)
                {
                PrepareCachingQueries(txn, ECInstanceKey(pair.first, pair.second), syncRecursively);
                }
            }
        else
            {
            RegisterError(txn, responseKey, result.GetError());
            }

        ContinueCachingQueries(txn);
        txn.Commit();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::RegisterError(CacheTransactionCR txn, CachedResponseKeyCR responseKey, CachingDataSource::ErrorCR error)
    {
    if (WSError::Status::ReceivedError == error.GetWSError().GetStatus())
        {
        AddFailedObject(txn, txn.GetCache().FindInstance(responseKey.GetParent()), error);
        }
    else
        {
        SetError(error);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncCachedDataTask::PrepareCachingQueries(CacheTransactionCR txn, ECInstanceKeyCR instanceKey, bool syncRecursively)
    {
    if (m_instancesWithQueriesProvided.find(instanceKey) != m_instancesWithQueriesProvided.end())
        {
        return;
        }

    bool isPersistent = IsInstancePersistent(txn, instanceKey);

    for (auto queryProviderPtr : m_queryProviders)
        {
        if (syncRecursively)
            {
            auto queries = queryProviderPtr->GetQueries(txn, instanceKey, isPersistent);
            m_queriesToCache.insert(m_queriesToCache.end(), queries.begin(), queries.end());
            }

        if (queryProviderPtr->DoUpdateFile(txn, instanceKey, isPersistent))
            {
            m_filesToDownload.insert(instanceKey);
            }
        }

    m_instancesWithQueriesProvided.insert(instanceKey);
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
            {
            SetError();
            }
        }

    return ECDbHelper::IsInstanceInMultiMap(instanceKey, *m_persistentInstances);
    }
