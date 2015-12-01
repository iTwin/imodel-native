/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/CacheNavigationTask.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "CacheNavigationTask.h"

#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Cache/ServerQueryHelper.h>

#include "Logging.h"
#include "DownloadFilesTask.h"
#include "SyncCachedDataTask.h"
#include "SyncCachedInstancesTask.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheNavigationTask::CacheNavigationTask
(
CachingDataSourcePtr cachingDataSource,
bvector<ObjectId>&& navigationTreesToCacheFully,
bvector<ObjectId>&& navigationTreesToUpdateOnly,
std::shared_ptr<const ISelectProvider> updateSelectProvider,
CachingDataSource::LabeledProgressCallback&& onProgress,
ICancellationTokenPtr cancellationToken
)
:
CachingTaskBase(cachingDataSource, cancellationToken),
m_navigationTreesToCacheFully(navigationTreesToCacheFully),
m_navigationTreesToUpdateOnly(navigationTreesToUpdateOnly),
m_updateSelectProvider(nullptr == updateSelectProvider ? std::make_shared<ISelectProvider>() : updateSelectProvider),
m_onProgressCallback(onProgress),
m_recursiveTasksRunning(0),
m_lastTimeReported(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::OnSchemaChanged()
    {
    if (nullptr != m_statementCache)
        {
        m_statementCache->Clear();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::_OnExecute()
    {
    ReportProgress(0, 0, nullptr);

    m_ds->GetClient()->GetWSClient()->GetServerInfo(GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (WSInfoResult& infoResult)
        {
        if (!infoResult.IsSuccess())
            {
            SetError(infoResult.GetError());
            return;
            }

        auto txn = m_ds->StartCacheTransaction();
        if (infoResult.GetValue().IsNavigationPropertySelectForAllClassesSupported())
            {
            ServerQueryHelper helper(*m_updateSelectProvider);
            m_updateProperties = helper.GetAllSelectedProperties(m_ds->GetRepositorySchemas(txn));
            }

        txn.GetCache().RegisterSchemaChangeListener(this);
        })

            ->Then(m_ds->GetCacheAccessThread(),
            std::bind(&CacheNavigationTask::CacheNavigationTrees, this, std::cref(m_navigationTreesToCacheFully), true))

            ->Then(m_ds->GetCacheAccessThread(),
            std::bind(&CacheNavigationTask::CacheNavigationTrees, this, std::cref(m_navigationTreesToUpdateOnly), false))

            ->Then(m_ds->GetCacheAccessThread(),
            std::bind(&CacheNavigationTask::CacheRejectedObjects, this))

            ->Then(m_ds->GetCacheAccessThread(),
            std::bind(&CacheNavigationTask::MarkNotModifiedChildrenAsPartial, this))

            ->Then(m_ds->GetCacheAccessThread(),
            std::bind(&CacheNavigationTask::CacheFiles, this))

            ->Then(m_ds->GetCacheAccessThread(), [=]
            {
            auto txn = m_ds->StartCacheTransaction();
            txn.GetCache().UnRegisterSchemaChangeListener(this);
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::CacheRejectedObjects()
    {
    if (IsTaskCanceled() || m_objectsToRedownload.empty())
        {
        return;
        }

    SyncCachedInstancesTask::Run(m_ds, m_objectsToRedownload, GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (ICachingDataSource::BatchResult result)
        {
        AddResult(result);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::MarkNotModifiedChildrenAsPartial()
    {
    if (IsTaskCanceled())
        {
        return;
        }
    auto txn = m_ds->StartCacheTransaction();
    if (SUCCESS != txn.GetCache().MarkTemporaryInstancesAsPartial(m_partialDataNotModifiedParents))
        {
        SetError();
        return;
        }
    txn.Commit();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::CacheFiles()
    {
    if (IsTaskCanceled() ||
        m_filesToDownload.empty())
        {
        return;
        }

    auto task = std::make_shared<DownloadFilesTask>
        (
        m_ds,
        m_filesToDownload,
        FileCache::Persistent,
        m_onProgressCallback,
        GetCancellationToken()
        );

    m_ds->GetCacheAccessThread()->Push(task);

    task->Then(m_ds->GetCacheAccessThread(), [=]
        {
        AddResult(task->GetResult());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::CacheNavigationTrees(const bvector<ObjectId>& navigationTrees, bool forceFullRecursiveCaching)
    {
    if (IsTaskCanceled()) return;

    for (ObjectIdCR objectId : navigationTrees)
        {
        // Empty ObjectId represents navigation root.
        if (objectId.IsEmpty())
            {
            m_objectsToCache.push(objectId);
            ContinueCachingChildrenRecursively(forceFullRecursiveCaching);
            }
        // If it is not navigation root - update object.
        else
            {
            auto txn = m_ds->StartCacheTransaction();
            Utf8String objectLabel = m_ds->GetObjectLabel(txn, objectId);

            m_ds->CacheObject(objectId, GetCancellationToken())
                ->Then(m_ds->GetCacheAccessThread(), [=] (CachingDataSource::Result result)
                {
                if (IsTaskCanceled()) return;

                if (result.IsSuccess())
                    {
                    m_objectsToCache.push(objectId);
                    ContinueCachingChildrenRecursively(forceFullRecursiveCaching);
                    }
                else
                    {
                    WSError::Id errorId = result.GetError().GetWSError().GetId();

                    if (WSError::Id::InstanceNotFound == errorId ||
                        WSError::Id::NotEnoughRights == errorId)
                        {
                        if (forceFullRecursiveCaching)
                            {
                            GetFailedObjects().push_back(CachingDataSource::FailedObject(objectId, objectLabel, result.GetError()));
                            }
                        }
                    else
                        {
                        SetError(result.GetError());
                        }
                    }
                });

            ReportProgress(0, 0, objectLabel);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::ContinueCachingChildrenRecursively(bool forceFullRecursiveCaching)
    {
    int maxDownloadsRunning = 10;

    LOG.tracev("CacheNavigationTask: running recursive tasks: %d", m_recursiveTasksRunning);

    while (m_recursiveTasksRunning < maxDownloadsRunning && !m_objectsToCache.empty())
        {
        if (IsTaskCanceled()) break;

        ObjectId objectId = m_objectsToCache.front();
        m_objectsToCache.pop();

        auto launchedTask = CacheChildrenRecursively(objectId, forceFullRecursiveCaching);
        if (launchedTask)
            {
            m_recursiveTasksRunning++;
            launchedTask->Then(m_ds->GetCacheAccessThread(), [=]
                {
                m_recursiveTasksRunning--;
                ContinueCachingChildrenRecursively(forceFullRecursiveCaching);
                });
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::ReportProgress(ObjectIdCR objectId)
    {
    if (!m_onProgressCallback)
        {
        return;
        }
    uint64_t currentTimeMillis = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    if (currentTimeMillis - m_lastTimeReported >= 500)
        {
        m_lastTimeReported = currentTimeMillis;

        auto txn = m_ds->StartCacheTransaction();
        auto label = m_ds->GetObjectLabel(txn, objectId);
        txn.Commit();

        m_onProgressCallback(0, 0, label);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CacheNavigationTask::ReportProgress(double bytesTransfered, double bytesTotal, Utf8StringCR taskLabel)
    {
    if (!m_onProgressCallback)
        {
        return;
        }
    m_onProgressCallback(bytesTransfered, bytesTotal, taskLabel);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    10/2013
*
* Returns true if async operation was launched
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> CacheNavigationTask::CacheChildrenRecursively(ObjectIdCR objectId, bool forceFullRecursiveCaching)
    {
    if (m_cachedObjects.find(objectId) != m_cachedObjects.end())
        {
        return nullptr;
        }
    m_cachedObjects.insert(objectId);

    auto txn = m_ds->StartCacheTransaction();

    if (objectId.IsValid())
        {
        CachedObjectInfo objectInfo = txn.GetCache().GetCachedObjectInfo(objectId);

        if (objectInfo.GetChangeStatus() == IChangeManager::ChangeStatus::Created)
            {
            return nullptr;
            }

        if (forceFullRecursiveCaching && IsObjectFileBacked(txn, objectInfo.GetCachedInstanceKey()))
            {
            m_filesToDownload.insert(objectId);
            }

        if (!m_ds->m_cachingOptions.CanObjectHaveChildren(objectId))
            {
            return nullptr;
            }
        }

    bool forceUpdateChildren = m_ds->m_cachingOptions.ShouldAlwaysCacheChildrenForClass(objectId.className);

    CachedResponseKey responseKey = m_ds->GetNavigationResponseKey(txn, objectId);
    std::shared_ptr<bset<ObjectId>> oldCachedChildren = std::make_shared<bset<ObjectId>>();

    // Get current children
    CacheStatus status = txn.GetCache().ReadResponseObjectIds(responseKey, *oldCachedChildren.get());
    if (CacheStatus::Error == status)
        {
        SetError();
        return nullptr;
        }

    if (CacheStatus::DataNotCached == status &&
        !forceFullRecursiveCaching &&
        !forceUpdateChildren)
        {
        return nullptr;
        }

    Utf8String childrenCacheTag = txn.GetCache().ReadResponseCacheTag(responseKey);

    bset<Utf8String> properties;
    if (!forceFullRecursiveCaching)
        {
        properties = m_updateProperties;
        }
    bool retrievingFullData = properties.empty();

    txn.Commit();

    auto task = m_ds->m_client->SendGetChildrenRequest(objectId, properties, childrenCacheTag, GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=] (WSObjectsResult& result)
        {
        ResolveGetChildrenResponse(result, objectId, oldCachedChildren, forceFullRecursiveCaching, forceUpdateChildren, retrievingFullData);
        });

    ReportProgress(objectId);

    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheNavigationTask::ResolveGetChildrenResponse
(
WSObjectsResult& result,
ObjectIdCR parentId,
std::shared_ptr<bset<ObjectId>> oldCachedChildren,
bool forceFullRecursiveCaching,
bool forceUpdateChildren,
bool retrievingFullData
)
    {
    if (IsTaskCanceled()) return;
    auto txn = m_ds->StartCacheTransaction();

    if (!result.IsSuccess())
        {
        if (WSError::Id::InstanceNotFound == result.GetError().GetId())
            {
            if (CacheStatus::Error == txn.GetCache().RemoveInstance(parentId))
                {
                SetError();
                return;
                }
            }
        else
            {
            SetError(result.GetError());
            return;
            }
        }

    CachedResponseKey responseKey = m_ds->GetNavigationResponseKey(txn, parentId);

    if (retrievingFullData)
        {
        if (SUCCESS != txn.GetCache().CacheResponse(responseKey, result.GetValue(), nullptr, nullptr, GetCancellationToken()))
            {
            SetError({CachingDataSource::Status::InternalCacheError, GetCancellationToken()});
            return;
            }
        }
    else
        {
        bset<ObjectId> rejected;

        // Force partial caching
        WSQuery query("", "");
        query.SetSelect("$id");

        if (SUCCESS != txn.GetCache().CacheResponse(responseKey, result.GetValue(), &rejected, &query, GetCancellationToken()))
            {
            SetError({CachingDataSource::Status::InternalCacheError, GetCancellationToken()});
            return;
            }
        for (ObjectIdCR rejectedObjectId : rejected)
            {
            if (m_cachedObjects.end() == m_cachedObjects.find(rejectedObjectId))
                {
                m_objectsToRedownload.insert(rejectedObjectId);
                }
            }
        }

    if (result.GetValue().IsModified())
        {
        bset<ObjectId> newCachedChildren;
        if (CacheStatus::Error == txn.GetCache().ReadResponseObjectIds(responseKey, newCachedChildren))
            {
            SetError();
            }

        for (ObjectId newChildId : newCachedChildren)
            {
            if (!forceFullRecursiveCaching && !forceUpdateChildren)
                {
                if ((*oldCachedChildren).find(newChildId) == (*oldCachedChildren).end())
                    {
                    continue;
                    }
                }
            m_objectsToCache.push(newChildId);
            }
        }
    else
        {
        if (!retrievingFullData)
            {
            m_partialDataNotModifiedParents.push_back(responseKey);
            }

        for (ObjectId oldChildId : *oldCachedChildren)
            {
            m_objectsToCache.push(oldChildId);
            }
        }
    txn.Commit();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CacheNavigationTask::IsObjectFileBacked(CacheTransactionCR txn, ECInstanceKeyCR instance)
    {
    if (nullptr == m_statementCache)
        {
        m_statementCache = std::make_shared <ECSqlStatementCache>(txn.GetCache().GetAdapter().GetECDb());
        }

    ECClassCP objectClass = txn.GetCache().GetAdapter().GetECClass(instance.GetECClassId());

    if (!instance.IsValid() || nullptr == objectClass)
        {
        BeAssert(false);
        return false;
        }

    Utf8String fileNameProperty = m_ds->m_cachingOptions.GetFileNameProperty(objectClass);
    if (fileNameProperty.empty())
        {
        return false;
        }

    Utf8PrintfString key("IsObjectFileBacked:%lld", instance.GetECClassId());
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        ECSqlSelectBuilder builder;
        builder
            .Select("NULL")
            .From(*objectClass, "fileClass", false)
            .Where(Utf8PrintfString("ECInstanceId = ? AND fileClass.[%s] IS NOT NULL AND fileClass.[%s] != ''",
            fileNameProperty.c_str(), fileNameProperty.c_str()));

        return builder.ToString();
        });

    statement->BindId(1, instance.GetECInstanceId());

    return BE_SQLITE_ROW == statement->Step();
    }