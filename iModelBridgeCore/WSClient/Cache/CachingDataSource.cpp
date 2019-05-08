/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/CachingDataSource.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/BeDebugLog.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/ServerQueryHelper.h>
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>
#include <WebServices/Cache/Util/FileUtil.h>
#include <WebServices/Cache/SyncNotifier.h>
#include <BeHttp/HttpStatusHelper.h>
#include <Bentley/BeTimeUtilities.h>

#include "Compatibility/SchemaChangeWSObjectsResponse.h"
#include "Compatibility/Schemas.h"
#include "CacheNavigationTask.h"
#include "Logging.h"
#include "DownloadFilesTask.h"
#include "Persistence/Core/SchemaContext.h"
#include "Persistence/RepositoryInfoStore.h"
#include "SessionInfo.h"
#include "SyncCachedDataTask.h"
#include "SyncCachedInstancesTask.h"
#include "SyncLocalChangesTask.h"
#include "Util/StringHelper.h"
#include "FileDownloadManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define CachedResultsName_Navigation    "CachingDataSource.Navigation"
#define CachedResultsName_Schemas       "CachingDataSource.Schemas"

static BeFileName s_assetDirectory;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachingDataSource::CachingDataSource
(
IWSRepositoryClientPtr client,
std::shared_ptr<ICacheTransactionManager> cacheTransactionManager,
std::shared_ptr<IRepositoryInfoStore> infoStore,
WorkerThreadPtr cacheAccessThread,
BeFileNameCR temporaryDir
) :
m_client(client),
m_cacheTransactionManager(cacheTransactionManager),
m_infoStore(infoStore),
m_sessionInfo(new SessionInfo()),
m_cacheAccessThread(cacheAccessThread),
m_cancellationToken(SimpleCancellationToken::Create()),
m_temporaryDir(temporaryDir),
m_fileDownloadManager(new FileDownloadManager(*this))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Arturas.Januska    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::Initialize (BeFileNameCR hostAssetsDirectory)
    {
    s_assetDirectory = hostAssetsDirectory;
    s_assetDirectory.AppendSeparator();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachingDataSource::~CachingDataSource()
    {
    Close();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::Close()
    {
    // Prevent from hanging when destroyed after failed open/create
    if (!m_isOpen)
        return;

    m_isOpen = false;

    CancelAllTasks()->Wait();

    m_cacheAccessThread->ExecuteAsync([=]
        {
        auto txn = StartCacheTransaction();
        IDataSourceCache& cache = txn.GetCache();
        txn.Rollback();
        cache.Close();
        })->Wait();
    }
    
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Arturas.Januska    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR CachingDataSource::GetHostAssetsDirectory ()
    {
    BeAssert(!s_assetDirectory.empty() && "Call CachingDataSource::Initialize() first.");
    return s_assetDirectory;
    }
    
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> CachingDataSource::CancelAllTasks()
    {
    m_cancellationToken->SetCanceled();
    return m_cacheAccessThread->OnEmpty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::EnableSkipTokens(bool enable)
    {
    m_enableSkipTokens = enable;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::SetMaxParalelFileDownloadLimit(size_t maxParalelDownloads)
    {
    m_maxParalelDownloads = maxParalelDownloads;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::SetMinTimeBetweenProgressCalls(uint64_t minTimeBetweenProgressCallsMs)
    {
    m_minTimeBetweenProgressCallsMs = minTimeBetweenProgressCallsMs;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CachingDataSource::GetInitialSkipToken() const
    {
    if (!m_enableSkipTokens)
        return "";
    return WSRepositoryClient::InitialSkipToken;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ICancellationTokenPtr CachingDataSource::CreateCancellationToken(ICancellationTokenPtr ct)
    {
    return MergeCancellationToken::Create(m_cancellationToken, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IWSRepositoryClientPtr CachingDataSource::GetClient() const
    {
    return m_client;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::SetClient(IWSRepositoryClientPtr client)
    {
    m_client = client;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransaction CachingDataSource::StartCacheTransaction()
    {
    return m_cacheTransactionManager->StartCacheTransaction();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WorkerThreadPtr CachingDataSource::GetCacheAccessThread()
    {
    return m_cacheAccessThread;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::SetClassesToAlwaysCacheChildren(const bset<Utf8String>& classesToAlwaysCacheChildren)
    {
    m_cachingOptions.SetClassesToAlwaysCacheChildren(classesToAlwaysCacheChildren);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::OpenResult> CachingDataSource::OpenOrCreate
(
IWSRepositoryClientPtr client,
BeFileNameCR cacheFilePath,
CacheEnvironmentCR cacheEnvironment,
WorkerThreadPtr cacheAccessThread,
ICancellationTokenPtr ct
)
    {
    if (nullptr == ct)
        ct = SimpleCancellationToken::Create();

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    if (cacheAccessThread == nullptr)
        {
        Utf8PrintfString threadName("Cache '%s'", client->GetRepositoryId().c_str());
        cacheAccessThread = WorkerThread::Create(threadName.c_str());
        }

    auto openResult = std::make_shared<OpenResult>();
    auto ds = std::shared_ptr<CachingDataSource>();

    return cacheAccessThread->ExecuteAsync([=] () mutable
        {
        if (ct->IsCanceled())
            {
            openResult->SetError(Status::Canceled);
            return;
            }

        LOG.infov("CachingDataSource::OpenOrCreate() URL:'%s' ID:'%s' using environment:\n%s\n%s",
            client->GetWSClient()->GetServerUrl().c_str(),
            client->GetRepositoryId().c_str(),
            cacheEnvironment.persistentFileCacheDir.GetNameUtf8().c_str(),
            cacheEnvironment.temporaryFileCacheDir.GetNameUtf8().c_str());

        const DefaultTxn defaultTxnMode = DefaultTxn::No;// Allow concurrent multiple connection access

        std::unique_ptr<DataSourceCache> cache(new DataSourceCache());
        if (cacheFilePath.DoesPathExist())
            {
            LOG.infov(L"Found existing cache at %ls", cacheFilePath.c_str());
            ECDb::OpenParams params(Db::OpenMode::ReadWrite);
            params.SetStartDefaultTxn(defaultTxnMode);
            if (SUCCESS != cache->Open(cacheFilePath, cacheEnvironment, params))
                {
                openResult->SetError(Status::InternalCacheError);
                return;
                }
            }
        else
            {
            LOG.infov(L"Creating new cache at %ls", cacheFilePath.c_str());
            ECDb::CreateParams params;
            params.SetStartDefaultTxn(defaultTxnMode); 
            if (SUCCESS != cache->Create(cacheFilePath, cacheEnvironment, params))
                {
                openResult->SetError(Status::InternalCacheError);
                return;
                }
            }

        if (ct->IsCanceled())
            {
            openResult->SetError(Status::Canceled);
            return;
            }

        auto cacheTransactionManager = std::make_shared<CacheTransactionManager>(std::move(cache), cacheAccessThread);
        auto infoStore = std::make_shared<RepositoryInfoStore>(cacheTransactionManager.get(), client, cacheAccessThread);

        ds = std::shared_ptr<CachingDataSource>(new CachingDataSource(
            client,
            cacheTransactionManager,
            infoStore,
            cacheAccessThread,
            cacheEnvironment.temporaryFileCacheDir
            ));

        auto txn = ds->StartCacheTransaction();
        if (ds->m_infoStore->IsCacheInitialized(txn.GetCache()))
            {
            if (SUCCESS != ds->FinalizeOpen(txn))
                {
                openResult->SetError(Status::InternalCacheError);
                return;
                }
            txn.Commit();
            openResult->SetSuccess(ds);
            return;
            }

        ds->UpdateSchemas(ct)
            ->Then(cacheAccessThread, [=] (Result result)
            {
            if (!result.IsSuccess())
                {
                openResult->SetError(result.GetError());
                return;
                }

            auto txn = ds->StartCacheTransaction();
            if (SUCCESS != ds->m_infoStore->SetCacheInitialized(txn.GetCache()) ||
                SUCCESS != ds->FinalizeOpen(txn))
                {
                openResult->SetError(Status::InternalCacheError);
                return;
                }
            txn.Commit();
            openResult->SetSuccess(ds);
            });
        })
            ->Then<OpenResult>(cacheAccessThread, [=]
            {
            if (openResult->IsSuccess())
                {
                if (openResult->GetValue() == nullptr || !openResult->GetValue()->m_isOpen)
                    {
                    BeAssert(false);
                    openResult->SetError({});
                    }
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LOG.infov("CachingDataSource::OpenOrCreate() URL:'%s' ID:'%s' %s and took: %.2f ms",
                client->GetWSClient()->GetServerUrl().c_str(),
                client->GetRepositoryId().c_str(),
                openResult->IsSuccess() ? "succeeded" : "failed",
                end - start);

            if (!openResult->IsSuccess())
                {
                LOG.errorv("CachingDataSource::OpenOrCreate() URL:'%s' ID:'%s' failed:\nFile: '%s'\nError: %s %s",
                    client->GetWSClient()->GetServerUrl().c_str(),
                    client->GetRepositoryId().c_str(),
                    cacheFilePath.GetNameUtf8().c_str(),
                    openResult->GetError().GetStatus() == Status::Canceled ? "Canceled" : openResult->GetError().GetMessage().c_str(),
                    openResult->GetError().GetDescription().c_str());

                // Force close cache
                if (nullptr != ds)
                    {
                    auto txn = ds->StartCacheTransaction();
                    IDataSourceCache& cache = txn.GetCache();
                    txn.Commit();
                    if (cache.GetECDb().IsDbOpen())
                        cache.Close();
                    }
                }

            return *openResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachingDataSource::FinalizeOpen(CacheTransactionCR txn)
    {
    if (SUCCESS != m_infoStore->PrepareInfo(txn.GetCache()))
        return ERROR;
    m_isOpen = true;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CachingDataSourcePtr CachingDataSource::Create
(
IWSRepositoryClientPtr client,
std::shared_ptr<ICacheTransactionManager> cacheTransactionManager,
std::shared_ptr<IRepositoryInfoStore> infoStore,
WorkerThreadPtr cacheAccessThread,
BeFileNameCR temporaryDir
)
    {
    return std::shared_ptr<CachingDataSource>(new CachingDataSource(client, cacheTransactionManager, infoStore, cacheAccessThread, temporaryDir));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::Result> CachingDataSource::UpdateSchemas(ICancellationTokenPtr ct)
    {
    ct = CreateCancellationToken(ct);

    auto schemaDownloadResults = std::make_shared<bmap<ObjectId, WSFileResult>>();
    auto temporaryFiles = std::make_shared<bvector<TempFilePtr>>();
    auto result = std::make_shared<Result>(Result::Success());

    auto getServerInfoTask = m_client->GetWSClient()->GetServerInfo(ct)
        ->Then(m_cacheAccessThread, [=] (WSInfoResult infoResult)
        {
        if (!infoResult.IsSuccess())
            {
            result->SetError(infoResult.GetError());
            return;
            }

        auto txn = StartCacheTransaction();
        m_infoStore->CacheServerInfo(txn.GetCache(), infoResult.GetValue());
        txn.Commit();
        });

    auto getRepositoryInfoTask = m_client->GetInfo(ct)
        ->Then(m_cacheAccessThread, [=] (WSRepositoryResult infoResult)
        {
        if (!infoResult.IsSuccess())
            {
            result->SetError(infoResult.GetError());
            return;
            }

        auto txn = StartCacheTransaction();
        m_infoStore->CacheRepositoryInfo(txn.GetCache(), infoResult.GetValue());
        txn.Commit();
        });

    bvector<AsyncTaskPtr<void>> tasks;
    tasks.push_back(getServerInfoTask);
    tasks.push_back(getRepositoryInfoTask);

    return AsyncTask::WhenAll(tasks)
            ->Then(m_cacheAccessThread, [=]
            {
            if (ct->IsCanceled())
                {
                result->SetError(Status::Canceled);
                return;
                }
            if (!result->IsSuccess())
                {
                return;
                }
            
            // TODO: cache all information about schemas in same transaction as importing schemas to avoid missing data.
            // TODO: Cache schema response and files AFTER schemas were upgraded in ECDb.

            // Ensure MetaSchema is available
            auto txn = StartCacheTransaction();
            if (!txn.GetCache().GetAdapter().HasECSchema(SCHEMA_MetaSchema))
                {
                auto path = SchemaContext::GetCacheSchemasDir().AppendToPath(BeFileName(SCHEMA_MetaSchema ".02.00.ecschema.xml"));
                if (SUCCESS != txn.GetCache().UpdateSchemas({path}))
                    {
                    result->SetError(Status::InternalCacheError);
                    return;
                    }
                }

            // Update schema list
            CachedResponseKey responseKey = CreateSchemaListResponseKey(txn);
            Utf8String eTag = txn.GetCache().ReadResponseCacheTag(responseKey);
            txn.Commit();

            m_client->SendGetSchemasRequest(eTag, ct)
                ->Then(m_cacheAccessThread, [=] (WSObjectsResult& objectsResult)
                {
                if (!objectsResult.IsSuccess())
                    {
                    result->SetError(objectsResult.GetError());
                    return;
                    }

                auto txn = StartCacheTransaction();
                if (CacheStatus::OK != txn.GetCache ().CacheResponse (responseKey, objectsResult.GetValue ()))
                    {
                    result->SetError(Status::InternalCacheError);
                    return;
                    }
                txn.Commit();
                });
            })
                ->Then(m_cacheAccessThread, [=]
                {
                if (ct->IsCanceled())
                    {
                    result->SetError(Status::Canceled);
                    return;
                    }

                if (!result->IsSuccess())
                    {
                    return;
                    }

                // Download changed schemas
                auto txn = StartCacheTransaction();
                bset<ObjectId> schemaIds;
                if (CacheStatus::OK != txn.GetCache().ReadResponseObjectIds(CreateSchemaListResponseKey(txn), schemaIds))
                    {
                    result->SetError(Status::InternalCacheError);
                    return;
                    }

                for (ObjectId schemaId : schemaIds)
                    {
                    Json::Value schemaDef;
                    if (CacheStatus::OK != txn.GetCache().ReadInstance(schemaId, schemaDef))
                        {
                        result->SetError(Status::InternalCacheError);
                        return;
                        }

                    if (!IsServerSchemaSupported(txn, schemaDef))
                        continue;

                    Utf8String eTag = txn.GetCache().ReadFileCacheTag(schemaId);
                    TempFilePtr schemaFile = GetTempFileForSchema(ExtractSchemaKey(schemaDef));

                    temporaryFiles->push_back(schemaFile);

                    m_client->SendGetFileRequest (schemaId, schemaFile->GetPath (), eTag, nullptr, ct)
                        ->Then(m_cacheAccessThread, [=] (WSFileResult& schemaFileResult)
                        {
                        schemaDownloadResults->insert({schemaId, schemaFileResult});
                        });
                    }
                })
                    ->Then(m_cacheAccessThread, [=]
                    {
                    if (ct->IsCanceled())
                        {
                        result->SetError(Status::Canceled);
                        return;
                        }

                    if (!result->IsSuccess())
                        {
                        return;
                        }

                    // Get downloaded schema paths
                    std::vector<BeFileName> changedSchemaPaths;
                    for (auto& pair : *schemaDownloadResults)
                        {
                        const WSFileResult& downloadResult = pair.second;
                        if (!downloadResult.IsSuccess())
                            {
                            result->SetError(downloadResult.GetError());
                            return;
                            }
                        if (downloadResult.GetValue().IsModified())
                            {
                            changedSchemaPaths.push_back(downloadResult.GetValue().GetFilePath());
                            }
                        }

                    // Update schemas
                    auto txn = StartCacheTransaction();
                    if (SUCCESS != txn.GetCache().UpdateSchemas(changedSchemaPaths))
                        {
                        LOG.errorv("UpdateSchemas(): Failed to import/update server schemas into ECDb.");
                        result->SetError(Status::SchemaError);
                        return;
                        }

                    // Cache schema files after schemas been updated
                    for (auto& pair : *schemaDownloadResults)
                        {
                        if (SUCCESS != txn.GetCache().CacheFile(pair.first, pair.second.GetValue(), FileCache::Persistent))
                            {
                            result->SetError(Status::InternalCacheError);
                            return;
                            }
                        }
                    txn.Commit();
                    })
                        ->Then<Result>([=]
                        {
                        temporaryFiles->clear();
                        return *result;
                        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECN::ECSchemaCP> CachingDataSource::GetRepositorySchemas(CacheTransactionCR txn)
    {
    bvector<ECN::ECSchemaCP> schemas;
    for (SchemaKeyCR schemaKey : GetRepositorySchemaKeys(txn))
        {
        ECSchemaCP schema = txn.GetCache().GetAdapter().GetECSchema(schemaKey.m_schemaName.c_str());
        if (nullptr == schema)
            {
            LOG.errorv("Server schema '%s' not found in cache", schemaKey.m_schemaName.c_str());
            BeAssert(false && "Schema not found in cache");
            continue;
            }
        schemas.push_back(schema);
        }
    return schemas;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SchemaKey> CachingDataSource::GetRepositorySchemaKeys(CacheTransactionCR txn)
    {
    bvector<SchemaKey> keys;

    Json::Value schemaDefs;
    if (CacheStatus::OK != txn.GetCache().ReadResponse(CreateSchemaListResponseKey(txn), schemaDefs))
        {
        BeAssert(false);
        return keys;
        }

    for (JsonValueCR schemaDef : schemaDefs)
        {
        if (IsServerSchemaDeprecated(schemaDef))
            continue;
        keys.push_back(ExtractSchemaKey(schemaDef));
        }

    return keys;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo CachingDataSource::GetServerInfo(CacheTransactionCR txn)
    {
    return GetServerInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo CachingDataSource::GetServerInfo()
    {
    return m_infoStore->GetServerInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepository CachingDataSource::GetRepositoryInfo()
    {
    return m_infoStore->GetRepositoryInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TempFilePtr CachingDataSource::GetTempFileForSchema(SchemaKeyCR schemaKey)
    {
    if (schemaKey.m_schemaName.empty())
        return GetTempFile(BeGuid(true).ToString(), ObjectId());

    Utf8String schemaFileName = schemaKey.m_schemaName + "." + schemaKey.GetLegacyVersionString() + ".ecschema.xml";
    return GetTempFile(schemaFileName, ObjectId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachingDataSource::IsServerSchemaSupported(CacheTransactionCR txn, JsonValueCR schemaDef)
    {
    Utf8CP name = schemaDef[CLASS_ECSchemaDef_PROPERTY_Name].asCString();

    if (ECSchema::IsStandardSchema(name))
        {
        if (txn.GetCache().GetAdapter().HasECSchema(name))
             return false; // Avoid downgrading standard schemas. // TODO: import if standard schema version is higher than local, but not lower
        return true;
        }

    if (IsServerSchemaDeprecated(schemaDef))
        return false;

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachingDataSource::IsServerSchemaDeprecated(JsonValueCR schemaDef)
    {
    Utf8CP name = schemaDef[CLASS_ECSchemaDef_PROPERTY_Name].asCString();
    Utf8CP prefix = schemaDef[CLASS_ECSchemaDef_PROPERTY_NameSpacePrefix].asCString();

    if (0 == strcmp(name, "Contents") && 0 == strcmp(prefix, "rest_cnt"))
        return true; // Deprecated, incompatible schema

    if (0 == strcmp(name, "Views") && 0 == strcmp(prefix, "rest_view"))
        return true; // Deprecated, incompatible schema

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaKey CachingDataSource::ExtractSchemaKey(JsonValueCR schemaDef)
    {
    return SchemaKey(
        schemaDef[CLASS_ECSchemaDef_PROPERTY_Name].asCString(),
        schemaDef[CLASS_ECSchemaDef_PROPERTY_VersionMajor].asInt(),
        schemaDef[CLASS_ECSchemaDef_PROPERTY_VersionWrite].asInt(),
        schemaDef[CLASS_ECSchemaDef_PROPERTY_VersionMinor].asInt()
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TempFilePtr CachingDataSource::GetTempFile(Utf8StringCR fileName, ObjectIdCR objectId)
    {
    if (fileName.empty())
        {
        return std::make_shared<TempFile>(m_temporaryDir, objectId.className + "_" + objectId.remoteId);
        }

    return std::make_shared<TempFile>(m_temporaryDir, fileName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CachingDataSource::GetObjectLabel(CacheTransactionCR txn, ObjectIdCR objectId)
    {
    Utf8String label = txn.GetCache().ReadInstanceLabel(objectId);
    if (label.empty() && !objectId.IsEmpty())
        {
        ECClassCP objectClass = txn.GetCache().GetAdapter().GetECClass(objectId.schemaName, objectId.className);
        if (nullptr == objectClass)
            {
            BeAssert(false);
            return label;
            }
        label.Sprintf("%s:%s", Utf8String(objectClass->GetDisplayLabel()).c_str(), objectId.remoteId.c_str());
        }
    return label;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CachingDataSource::GetObjectLabel(CacheTransactionCR txn, ECInstanceKeyCR instanceKey)
    {
    return GetObjectLabel(txn, txn.GetCache().FindInstance(instanceKey));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::ObjectsResult> CachingDataSource::GetObjectInternal
(
ObjectIdCR objectId,
DataOrigin origin,
ICancellationTokenPtr ct
)
    {
    auto result = std::make_shared <ObjectsResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (ct->IsCanceled())
            {
            result->SetError(Status::Canceled);
            return;
            }

        // check cache for object
        auto txn = StartCacheTransaction();
        if (DataOrigin::CachedData == origin || DataOrigin::CachedOrRemoteData == origin)
            {
            auto cachedData = std::make_shared<Json::Value>();

            CachedObjectInfo objectInfo = txn.GetCache().GetCachedObjectInfo(objectId);
            if (objectInfo.IsFullyCached() || IChangeManager::ChangeStatus::Created == objectInfo.GetChangeStatus())
                {
                txn.GetCache().ReadInstance(objectId, *cachedData);
                }

            if (!cachedData->isNull())
                {
                result->SetSuccess(ObjectsData(cachedData, DataOrigin::CachedData));
                return;
                }
            else if (DataOrigin::CachedData == origin)
                {
                result->SetError(Error(Status::DataNotCached));
                return;
                }
            }

        m_client->SendGetObjectRequest(objectId, txn.GetCache().ReadInstanceCacheTag(objectId), ct)
            ->Then<bool>(m_cacheAccessThread, [=] (WSObjectsResult& objectsResult)
            {
            auto txn = StartCacheTransaction();
            DataOrigin returningDataOrigin = DataOrigin::RemoteData;
            bool isObjectNotFound = false;
            if (objectsResult.IsSuccess())
                {
                auto status = txn.GetCache().UpdateInstance(objectId, objectsResult.GetValue());
                if (CacheStatus::OK != status)
                    {
                    result->SetError(status);
                    return isObjectNotFound;
                    }

                if (!objectsResult.GetValue().IsModified())
                    {
                    returningDataOrigin = DataOrigin::CachedData;
                    }
                }
            else
                {
                if (objectsResult.GetError().IsInstanceNotAvailableError())
                    isObjectNotFound = true;

                if (CachingDataSource::DataOrigin::RemoteOrCachedData == origin &&
                    WSError::Status::ConnectionError == objectsResult.GetError().GetStatus() &&
                    txn.GetCache().GetCachedObjectInfo(objectId).IsFullyCached())
                    {
                    returningDataOrigin = DataOrigin::CachedData;
                    }
                else
                    {
                    result->SetError(objectsResult.GetError());
                    return isObjectNotFound;
                    }
                }

            auto cachedData = std::make_shared<Json::Value>();
            txn.GetCache().ReadInstance(objectId, *cachedData);

            if (cachedData->isNull())
                {
                result->SetError(Error(Status::InternalCacheError));
                return isObjectNotFound;
                }

            txn.Commit();
            result->SetSuccess({cachedData, returningDataOrigin});
            return isObjectNotFound;
            })->Then(m_cacheAccessThread, [=] (bool isObjectNotFound)
                {
                if (isObjectNotFound)
                    {
                    auto txn = StartCacheTransaction();
                    txn.GetCache().RemoveInstance(objectId);
                    txn.Commit();
                    }
                });
        })
            ->Then<ObjectsResult>([=]
            {
            return *result;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                          Viktorija Adomauskaite    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::ObjectsResult> CachingDataSource::GetObject
(
ObjectIdCR objectId,
RetrieveOptions retrieveOptions,
ICancellationTokenPtr ct
)
    {
    ct = CreateCancellationToken(ct);

    return GetObjectInternal(objectId, retrieveOptions.GetOrigin(), ct)->Then<CachingDataSource::ObjectsResult>([=] (CachingDataSource::ObjectsResult& result)
        {
        if (!result.IsSuccess())
            return result;

        GetObjectInBackgroundIfNeeded(objectId, retrieveOptions, result, ct);

        return result;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                          Viktorija Adomauskaite    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::GetObjectInBackgroundIfNeeded
(
ObjectIdCR objectId,
RetrieveOptions retrieveOptions,
CachingDataSource::ObjectsResult result,
ICancellationTokenPtr ct
)
    {
    if (!retrieveOptions.GetSyncNotifier())
        return;

    if (retrieveOptions.GetOrigin() != DataOrigin::CachedData && retrieveOptions.GetOrigin() != DataOrigin::CachedOrRemoteData)
        return;

    if (result.GetValue().GetOrigin() == DataOrigin::RemoteData)
        return;

    auto finalBackgroundSyncResult = std::make_shared <SyncResult>();
    auto backgroundSyncTask = m_cacheAccessThread->ExecuteAsyncWithoutAttachingToCurrentTask([=]
        {
        if (ct->IsCanceled())
            {
            finalBackgroundSyncResult->SetError(Status::Canceled);
            return;
            }

        GetObjectInternal(objectId, DataOrigin::RemoteData, ct)
            ->Then(m_cacheAccessThread, [=] (CachingDataSource::ObjectsResult& result)
            {
            if (!result.IsSuccess() && result.GetError().GetWSError().IsInstanceNotAvailableError())
                finalBackgroundSyncResult->SetSuccess(SyncStatus::Synced);
            else if (!result.IsSuccess())
                finalBackgroundSyncResult->SetError(result.GetError());
            else if (result.GetValue().GetOrigin() == DataOrigin::RemoteData)
                finalBackgroundSyncResult->SetSuccess(SyncStatus::Synced);
            else
                finalBackgroundSyncResult->SetSuccess(SyncStatus::NotModified);
            });
        })
            ->Then<SyncResult>([=]
            {
            return *finalBackgroundSyncResult;
            });

        retrieveOptions.GetSyncNotifier()->AddTask(backgroundSyncTask);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::DataOriginResult> CachingDataSource::CacheObjects
(
CachedResponseKeyCR responseKey,
WSQueryCR query,
DataOrigin origin,
Utf8StringCR skipToken,
uint64_t page,
ICancellationTokenPtr ct
)
    {
    auto result = std::make_shared <DataOriginResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (ct->IsCanceled())
            {
            result->SetError(Status::Canceled);
            return;
            }

        // check cache
        auto txn = StartCacheTransaction();
        if (DataOrigin::CachedData == origin || DataOrigin::CachedOrRemoteData == origin)
            {
            if (txn.GetCache().IsResponseCached(responseKey))
                {
                result->SetSuccess({DataOrigin::CachedData, SyncStatus::NotSynced});
                return;
                }
            else if (DataOrigin::CachedData == origin)
                {
                result->SetError(Status::DataNotCached);
                return;
                }
            }

        // connect to server for data
        Utf8String cacheTag = txn.GetCache().ReadResponseCacheTag(responseKey, page);
        m_client->SendQueryRequest(query, cacheTag, skipToken, ct)
            ->Then(m_cacheAccessThread, [=] (WSObjectsResult& objectsResult)
            {
            auto txn = StartCacheTransaction();
            DataOrigin returningDataOrigin = DataOrigin::RemoteData;
            SyncStatus returningSyncStatus = SyncStatus::Synced;
            if (objectsResult.IsSuccess())
                {
                WSObjectsResponseCR response = objectsResult.GetValue();

                bset<ObjectId> rejected;
                auto status = txn.GetCache().CacheResponse(responseKey, response, &rejected, &query, page, ct);
                if (CacheStatus::OK != status)
                    {
                    result->SetError({status, ct});
                    return;
                    }

                if (!response.IsModified())
                    {
                    // TODO: This may be misleading and loosing info:
                    // Technically server check was done, but it did not indicated that data didi not changed.
                    // UI may want to know this and do not do any more checks.
                    // On the other hand, such check might be done on DataOrigin::CachedData to refresh data.
                    returningDataOrigin = DataOrigin::CachedData;
                    returningSyncStatus = SyncStatus::NotModified;
                    }

                if (!response.IsFinal())
                    {
                    CacheObjects(responseKey, query, origin, response.GetSkipToken(), page + 1, ct)
                        ->Then([=] (DataOriginResult nextResult)
                        {
                        *result = nextResult;
                        });
                    }

                if (!rejected.empty())
                    {
                    SyncCachedInstancesTask::Run(this->shared_from_this(), rejected, nullptr, ct)
                        ->Then(m_cacheAccessThread, [=] (BatchResult instancesResult)
                        {
                        if (instancesResult.IsSuccess())
                            {
                            result->SetSuccess({returningDataOrigin, returningSyncStatus});
                            }
                        else
                            {
                            result->SetError(instancesResult.GetError());
                            }
                        });
                    }
                }
            else
                {
                if (CachingDataSource::DataOrigin::RemoteOrCachedData == origin &&
                    WSError::Status::ConnectionError == objectsResult.GetError().GetStatus() &&
                    txn.GetCache().IsResponseCached(responseKey))
                    {
                    returningDataOrigin = DataOrigin::CachedData;
                    returningSyncStatus = SyncStatus::SyncError;
                    }
                else
                    {
                    result->SetError(objectsResult.GetError());
                    return;
                    }
                }

            txn.Commit();
            result->SetSuccess({returningDataOrigin, returningSyncStatus});
            });
        })
            ->Then<DataOriginResult>([=]
            {
            return *result;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::ObjectsResult> CachingDataSource::GetObjects
(
CachedResponseKeyCR responseKey,
WSQueryCR query,
DataOrigin origin,
std::shared_ptr<const ISelectProvider> cachedSelectProvider,
ICancellationTokenPtr ct
)
    {
    if (nullptr == cachedSelectProvider)
        cachedSelectProvider = std::make_shared<ISelectProvider>();

    ct = CreateCancellationToken(ct);

    return CacheObjects(responseKey, query, origin, GetInitialSkipToken(), 0, ct)
        ->Then<ObjectsResult>(m_cacheAccessThread, [=] (DataOriginResult& result)
        {
        if (!result.IsSuccess())
            return ObjectsResult::Error(result.GetError());

        auto txn = StartCacheTransaction();
        auto cachedInstances = std::make_shared<Json::Value>();
        CacheStatus status = txn.GetCache().ReadResponse(responseKey, *cachedInstances, *cachedSelectProvider);
        if (CacheStatus::OK != status)
            return ObjectsResult::Error(status);

        return ObjectsResult::Success({cachedInstances, result.GetValue().origin});
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::CacheObjectsInBackgroundIfNeeded
(
CachedResponseKeyCR responseKey,
WSQueryCR query,
RetrieveOptions retrieveOptions,
DataOriginResult result,
ICancellationTokenPtr ct
)
    {
    if (!retrieveOptions.GetSyncNotifier())
        return;

    if (retrieveOptions.GetOrigin() != DataOrigin::CachedData && retrieveOptions.GetOrigin() != DataOrigin::CachedOrRemoteData)
        return;

    if (result.GetValue().syncStatus != SyncStatus::NotSynced)
        return;

    auto finalBackgroundSyncResult = std::make_shared <SyncResult>();
    auto backgroundSyncTask = m_cacheAccessThread->ExecuteAsyncWithoutAttachingToCurrentTask([=]
        {
        if (ct->IsCanceled())
            {
            finalBackgroundSyncResult->SetError(Status::Canceled);
            return;
            }

        CacheObjects(responseKey, query, DataOrigin::RemoteData, GetInitialSkipToken(), 0, ct)
            ->Then(m_cacheAccessThread, [=] (DataOriginResult& result)
            {
            if (!result.IsSuccess())
                finalBackgroundSyncResult->SetError(result.GetError());
            else
                finalBackgroundSyncResult->SetSuccess(result.GetValue().syncStatus);
            });
        })
            ->Then<SyncResult>([=]
            {
            return *finalBackgroundSyncResult;
            });
        retrieveOptions.GetSyncNotifier()->AddTask(backgroundSyncTask);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::KeysResult> CachingDataSource::GetObjectsKeys
(
CachedResponseKeyCR responseKey,
WSQueryCR query,
RetrieveOptions retrieveOptions,
ICancellationTokenPtr ct
)
    {
    ct = CreateCancellationToken(ct);

    return CacheObjects(responseKey, query, retrieveOptions.GetOrigin(), GetInitialSkipToken(), 0, ct)
        ->Then<KeysResult>(m_cacheAccessThread, [=] (DataOriginResult& result)
        {
        if (!result.IsSuccess())
            return KeysResult::Error(result.GetError());

        auto txn = StartCacheTransaction();
        auto keys = std::make_shared<ECInstanceKeyMultiMap>();

        CacheStatus status = txn.GetCache().ReadResponseInstanceKeys(responseKey, *keys);
        if (CacheStatus::OK != status)
            return KeysResult::Error(status);

        CacheObjectsInBackgroundIfNeeded(responseKey, query, retrieveOptions, result, ct);

        return KeysResult::Success({keys, result.GetValue().origin, result.GetValue().syncStatus});
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::DataOriginResult> CachingDataSource::CacheNavigationChildren
(
ObjectIdCR parentId,
DataOrigin origin,
std::shared_ptr<const ISelectProvider> selectProvider,
ICancellationTokenPtr ct
)
    {
    auto finalResult = std::make_shared <DataOriginResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (ct->IsCanceled())
            {
            finalResult->SetError(Status::Canceled);
            return;
            }

        auto txn = StartCacheTransaction();
        CachedResponseKey responseKey = GetNavigationResponseKey(txn, parentId);
        if (!responseKey.GetParent().IsValid())
            {
            BeAssert(false && "Parent not found in cache");
            finalResult->SetError(Status::DataNotCached);
            return;
            }

        WSQueryPtr query = GetNavigationQuery(txn, parentId, selectProvider);
        if (nullptr == query)
            {
            finalResult->SetError(Status::InternalCacheError);
            return;
            }

        CacheObjects(responseKey, *query, origin, GetInitialSkipToken(), 0, ct)
            ->Then([=] (DataOriginResult result)
            {
            *finalResult = result;
            });
        })
            ->Then<DataOriginResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::ObjectsResult> CachingDataSource::GetNavigationChildren
(
ObjectIdCR parentId,
DataOrigin origin,
std::shared_ptr<const SelectProvider> selectProvider,
ICancellationTokenPtr ct
)
    {
    if (nullptr == selectProvider)
        {
        selectProvider = std::make_shared<SelectProvider>();
        }
    ct = CreateCancellationToken(ct);

    return CacheNavigationChildren(parentId, origin, selectProvider->GetForRemote(), ct)
        ->Then<ObjectsResult>(m_cacheAccessThread, [=] (DataOriginResult& result)
        {
        if (!result.IsSuccess())
            {
            return ObjectsResult::Error(result.GetError());
            }

        auto txn = StartCacheTransaction();
        auto responseKey = GetNavigationResponseKey(txn, parentId);
        auto cachedInstances = std::make_shared<Json::Value>();

        if (CacheStatus::OK != txn.GetCache().ReadResponse(responseKey, *cachedInstances, *selectProvider->GetForCache()))
            {
            return ObjectsResult::Error(Status::InternalCacheError);
            }

        return ObjectsResult::Success({cachedInstances, result.GetValue().origin});
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::KeysResult> CachingDataSource::GetNavigationChildrenKeys
(
ObjectIdCR parentId,
DataOrigin origin,
std::shared_ptr<const ISelectProvider> selectProvider,
ICancellationTokenPtr ct
)
    {
    if (nullptr == selectProvider)
        {
        selectProvider = std::make_shared<ISelectProvider>();
        }
    ct = CreateCancellationToken(ct);

    return CacheNavigationChildren(parentId, origin, selectProvider, ct)
        ->Then<KeysResult>(m_cacheAccessThread, [=] (DataOriginResult& result)
        {
        if (!result.IsSuccess())
            {
            return KeysResult::Error(result.GetError());
            }

        auto txn = StartCacheTransaction();
        auto responseKey = GetNavigationResponseKey(txn, parentId);
        auto keys = std::make_shared<ECInstanceKeyMultiMap>();

        CacheStatus status = txn.GetCache().ReadResponseInstanceKeys(responseKey, *keys);
        if (CacheStatus::OK != status)
            {
            return KeysResult::Error(status);
            }

        return KeysResult::Success({keys, result.GetValue().origin, result.GetValue().syncStatus});
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedResponseKey CachingDataSource::GetNavigationResponseKey(CacheTransactionCR txn, ObjectIdCR parentId)
    {
    ECInstanceKey parent = txn.GetCache().FindInstance(parentId);
    return CachedResponseKey(parent, CachedResultsName_Navigation);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedResponseKey CachingDataSource::GetNavigationResponseKey(CacheTransactionCR txn, ECInstanceKeyCR parentKey)
    {
    return CachedResponseKey(parentKey, CachedResultsName_Navigation);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSQueryPtr CachingDataSource::GetNavigationQuery(CacheTransactionCR txn, ObjectIdCR parentId, ISelectProviderPtr selectProvider)
    {
    WSInfo serverInfo = GetServerInfo();
    if (serverInfo.GetVersion() < BeVersion(2, 0))
        {
        Utf8String schemaName = parentId.schemaName;
        if (schemaName.empty())
            {
            auto schemaKeys = GetRepositorySchemaKeys(txn);
            if (schemaKeys.size() != 1)
                {
                return nullptr;
                }
            schemaName = Utf8String(schemaKeys[0].m_schemaName);
            }

        auto query = std::make_shared<WSQuery>(schemaName, parentId.className);
        query->SetCustomParameter(WSQuery_CustomParameter_NavigationParentId, parentId.remoteId);

        if (nullptr != selectProvider &&
            !txn.GetCache().IsInstanceFullyPersisted(parentId) &&
            serverInfo.IsNavigationPropertySelectForAllClassesSupported())
            {
            // TODO: investigate if selected properties meta-data could be saved in ECDb with low performance hit - to avoid using different
            // ISelectProvider for server and cache (D-133675)
            ServerQueryHelper helper(*selectProvider);
            bset<Utf8String> properties = helper.GetAllSelectedProperties(GetRepositorySchemas(txn));
            query->SetSelect(StringHelper::Join(properties.begin(), properties.end(), ","));
            }

        return query;
        }

    auto query = std::make_shared<WSQuery>("Navigation", "NavNode");
    if (!parentId.IsEmpty())
        {
        query->SetFilter(Utf8PrintfString("NavNodeChildren-backward-NavNode.$id+eq+'%s'", parentId.remoteId.c_str()));
        }

    if (nullptr != selectProvider)
        {
        ECClassCP navNodeClass = txn.GetCache().GetAdapter().GetECClass("Navigation", "NavNode");
        if (navNodeClass == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        Utf8String select = ServerQueryHelper(*selectProvider).GetSelect(*navNodeClass);
        query->SetSelect(select);
        }

    return query;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedResponseKey CachingDataSource::CreateSchemaListResponseKey(CacheTransactionCR txn)
    {
    ECInstanceKey root = txn.GetCache().FindOrCreateRoot("");
    return CachedResponseKey(root, CachedResultsName_Schemas);
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::DownloadAndCacheFiles
(
bmap<ObjectId, ICancellationTokenPtr> filesToDownload,
FileCache fileCacheLocation,
CachingDataSource::ProgressCallback onProgress,
ICancellationTokenPtr ct
)
    {
    auto task = std::make_shared<DownloadFilesTask>
        (
        shared_from_this(),
        m_fileDownloadManager,
        std::move(filesToDownload),
        fileCacheLocation,
        m_maxParalelDownloads,
        m_minTimeBetweenProgressCallsMs,
        std::move(onProgress),
        ct
        );

    m_cacheAccessThread->Push(task);

    auto result = std::make_shared <BatchResult>();
    return task->Then<BatchResult>(m_cacheAccessThread, [=]
        {
        return task->GetResult();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::DownloadAndCacheFiles
(
bset<ObjectId> filesToDownload,
FileCache fileCacheLocation,
CachingDataSource::ProgressCallback onProgress,
ICancellationTokenPtr ct
)
    {
    bmap<ObjectId, ICancellationTokenPtr> filesAndCTs;
    for (auto fileId : filesToDownload)
        filesAndCTs.Insert(fileId, ct);
    return DownloadAndCacheFiles(filesAndCTs, fileCacheLocation, onProgress, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::FileResult> CachingDataSource::GetFile
(
ObjectIdCR objectId,
DataOrigin origin,
ProgressCallback onProgress,
ICancellationTokenPtr ct
)
    {
    ct = CreateCancellationToken(ct);
    auto finalResult = std::make_shared<FileResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (ct->IsCanceled())
            {
            finalResult->SetError(Status::Canceled);
            return;
            }

        auto txn = StartCacheTransaction();

        // check cache for object
        if (DataOrigin::CachedData == origin || DataOrigin::CachedOrRemoteData == origin)
            {
            Json::Value file;
            txn.GetCache().ReadInstance(objectId, file);
            BeFileName cachedFilePath;
            if (!file.isNull())
                {
                cachedFilePath = txn.GetCache().ReadFilePath(objectId);
                if (!cachedFilePath.empty())
                    {
                    finalResult->SetSuccess(FileData(cachedFilePath, DataOrigin::CachedData));
                    return;
                    }
                }
            if (cachedFilePath.empty() && DataOrigin::CachedData == origin)
                {
                finalResult->SetError(Error(Status::DataNotCached));
                return;
                }
            }

        bset<ObjectId> filesToDownload;
        filesToDownload.insert(objectId);

        DownloadAndCacheFiles(filesToDownload, FileCache::Auto, onProgress, ct)
            ->Then(m_cacheAccessThread, [=] (ICachingDataSource::BatchResult result)
            {
            if (!result.GetValue().empty())
                result.SetError(result.GetValue().front().GetError());

            if (result.IsSuccess())
                {
                auto txn = StartCacheTransaction();
                BeFileName cachedFilePath = txn.GetCache().ReadFilePath(objectId);
                finalResult->SetSuccess(FileData(cachedFilePath, DataOrigin::RemoteData));
                }
            else if (CachingDataSource::DataOrigin::RemoteOrCachedData == origin &&
                WSError::Status::ConnectionError == result.GetError().GetWSError().GetStatus())
                {
                auto txn = StartCacheTransaction();
                BeFileName cachedFilePath = txn.GetCache().ReadFilePath(objectId);
                if (!cachedFilePath.empty())
                    finalResult->SetSuccess(FileData(cachedFilePath, DataOrigin::CachedData));
                }

            if (!finalResult->IsSuccess())
                finalResult->SetError(result.GetError());
            });
        })
            ->Then<FileResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::CacheFiles
(
const bvector<ObjectId>& filesIds,
bool skipCachedFiles,
FileCache fileCacheLocation,
ProgressCallback onProgress,
ICancellationTokenPtr ct
)
    {
    ct = CreateCancellationToken(ct);

    auto finalResult = std::make_shared <BatchResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (ct->IsCanceled())
            {
            finalResult->SetError(Status::Canceled);
            return;
            }

        // check cache for object
        bset<ObjectId> filesToDownload;

        if (skipCachedFiles)
            {
            auto txn = StartCacheTransaction();
            for (ObjectIdCR objectId : filesIds)
                {
                BeFileName cachedFilePath = txn.GetCache().ReadFilePath(objectId);
                if (cachedFilePath.empty())
                    {
                    filesToDownload.insert(objectId);
                    }
                }
            }
        else
            {
            filesToDownload.insert(filesIds.begin(), filesIds.end());
            }

        if (filesToDownload.size() == 0)
            {
            finalResult->SetSuccess(FailedObjects());
            return;
            }

        DownloadAndCacheFiles(
            filesToDownload,
            fileCacheLocation,
            onProgress,
            ct)->Then(m_cacheAccessThread, [=] (ICachingDataSource::BatchResult& result)
            {
            *finalResult = result;
            });
        })
            ->Then<BatchResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::Result> CachingDataSource::DownloadAndCacheChildren
(
const bvector<ObjectId>& parentIds,
ICancellationTokenPtr ct
)
    {
    ct = CreateCancellationToken(ct);
    auto finalResult = std::make_shared<CachingDataSource::Result>();
    finalResult->SetSuccess();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (ct->IsCanceled())
            {
            finalResult->SetError(Status::Canceled);
            return;
            }

        for (ObjectIdCR parentId : parentIds)
            {
            CacheNavigationChildren(parentId, DataOrigin::RemoteData, nullptr, ct)
                ->Then(m_cacheAccessThread, [=] (DataOriginResult& result)
                {
                if (!result.IsSuccess())
                    {
                    finalResult->SetError(result.GetError());
                    }
                });
            }
        })
            ->Then<Result>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 William.Francis     01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::SyncLocalChanges
(
ProgressCallback onProgress,
ICancellationTokenPtr ct,
SyncOptions options
)
    {
    return SyncLocalChanges(nullptr, std::move(onProgress), ct, options);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 William.Francis     01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::SyncLocalChanges
(
const bset<ECInstanceKey>& objectsToSync,
ProgressCallback onProgress,
ICancellationTokenPtr ct,
SyncOptions options
)
    {
    auto objectsToSyncPtr = std::make_shared<bset<ECInstanceKey>>(objectsToSync);
    return SyncLocalChanges(objectsToSyncPtr, std::move(onProgress), ct, options);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 William.Francis     01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::SyncLocalChanges
(
std::shared_ptr<bset<ECInstanceKey>> objectsToSync,
ProgressCallback onProgress,
ICancellationTokenPtr ct,
SyncOptions options
)
    {
    ct = CreateCancellationToken(ct);

    auto syncTask = std::make_shared<SyncLocalChangesTask>
        (
        shared_from_this(),
        objectsToSync,
        options,
        std::move(onProgress),
        ct
        );

    // Ensure that only single SyncLocalChangesTask is running at the time
    m_cacheAccessThread->ExecuteAsync([=]
        {
        if (objectsToSync)
            {
            auto txn = StartCacheTransaction();
            syncTask->PrepareObjectsForSync(txn);
            txn.Commit();
            }
        m_syncLocalChangesQueue.push_back(syncTask);
        if (m_syncLocalChangesQueue.size() == 1)
            {
            m_cacheAccessThread->Push(m_syncLocalChangesQueue.front());
            }
        });

    return syncTask->Then<BatchResult>(m_cacheAccessThread, [=]
        {
        m_syncLocalChangesQueue.pop_front();
        if (!m_syncLocalChangesQueue.empty())
            {
            m_cacheAccessThread->Push(m_syncLocalChangesQueue.front());
            }
        return syncTask->GetResult();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::Result> CachingDataSource::CacheObject
(
ObjectIdCR objectId,
ICancellationTokenPtr ct
)
    {
    ct = CreateCancellationToken(ct);
    auto result = std::make_shared<CachingDataSource::Result>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        auto txn = StartCacheTransaction();
        Utf8String cacheTag = txn.GetCache().ReadInstanceCacheTag(objectId);

        m_client->SendGetObjectRequest(objectId, cacheTag, ct)
            ->Then(m_cacheAccessThread, [=] (WSObjectsResult& objectsResult)
            {
            if (ct->IsCanceled())
                {
                result->SetError(Status::Canceled);
                return;
                }

            auto txn = StartCacheTransaction();

            if (objectsResult.IsSuccess())
                {
                if (CacheStatus::OK != txn.GetCache().UpdateInstance(objectId, objectsResult.GetValue()))
                    {
                    result->SetError(Status::InternalCacheError);
                    return;
                    }
                result->SetSuccess();
                }
            else
                {
                result->SetError(objectsResult.GetError());

                if (objectsResult.GetError().IsInstanceNotAvailableError())
                    {
                    txn.GetCache().RemoveInstance(objectId);
                    }
                }

            txn.Commit();
            });
        })
            ->Then<Result>([=]
            {
            return *result;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::SyncCachedData
(
bvector<ECInstanceKey> initialInstances,
bvector<IQueryProvider::Query> initialQueries,
bvector<IQueryProviderPtr> queryProviders,
ProgressHandler progressHandler,
ICancellationTokenPtr ct
)
    {
    auto task = std::make_shared<SyncCachedDataTask>
        (
        shared_from_this(),
        std::move(initialInstances),
        std::move(initialQueries),
        std::move(queryProviders),
        std::move(progressHandler),
        CreateCancellationToken(ct)
        );

    m_cacheAccessThread->Push(task);

    return task->Then<BatchResult>(m_cacheAccessThread, [=]
        {
        return task->GetResult();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::CacheNavigation
(
const bvector<ObjectId>& navigationTreesToCacheFully,
const bvector<ObjectId>& navigationTreesToUpdateOnly,
std::shared_ptr<const ISelectProvider> updateSelectProvider,
ProgressCallback onProgress,
ICancellationTokenPtr ct
)
    {
    HttpClient::BeginNetworkActivity();

    auto task = std::make_shared<CacheNavigationTask>
        (
        shared_from_this(),
        bvector<ObjectId>(navigationTreesToCacheFully),
        bvector<ObjectId>(navigationTreesToUpdateOnly),
        updateSelectProvider,
        std::move(onProgress),
        CreateCancellationToken(ct)
        );

    m_cacheAccessThread->Push(task);

    return task->Then<BatchResult>(m_cacheAccessThread, [=]
        {
        HttpClient::EndNetworkActivity();
        return task->GetResult();
        });
    }
