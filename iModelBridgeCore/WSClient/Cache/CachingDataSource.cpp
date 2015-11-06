/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/CachingDataSource.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/CachingDataSource.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/BeDebugLog.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/ServerQueryHelper.h>
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>
#include <WebServices/Cache/Util/FileUtil.h>
#include <DgnClientFx/Utils/Http/HttpStatusHelper.h>

#include "CacheNavigationTask.h"
#include "Logging.h"
#include "DownloadFilesTask.h"
#include "Persistence/Core/SchemaContext.h"
#include "Persistence/RepositoryInfoStore.h"
#include "SyncCachedDataTask.h"
#include "SyncCachedInstancesTask.h"
#include "SyncLocalChangesTask.h"
#include "Util/StringHelper.h"

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define CachedResultsName_Navigation    "CachingDataSource.Navigation"
#define CachedResultsName_Schemas       "CachingDataSource.Schemas"

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
m_cacheAccessThread(cacheAccessThread),
m_cancellationToken(SimpleCancellationToken::Create()),
m_temporaryDir(temporaryDir)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachingDataSource::~CachingDataSource()
    {
    CancelAllTasksAndWait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::CancelAllTasksAndWait()
    {
    m_cancellationToken->SetCanceled();
    m_cacheAccessThread->OnEmpty()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ICancellationTokenPtr CachingDataSource::CreateCancellationToken(ICancellationTokenPtr cancellationToken)
    {
    return MergeCancellationToken::Create(m_cancellationToken, cancellationToken);
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
WorkerThreadPtr cacheAccessThread
)
    {
    if (cacheAccessThread == nullptr)
        {
        Utf8PrintfString threadName("Cache '%s'", client->GetRepositoryId().c_str());
        cacheAccessThread = WorkerThread::Create(threadName);
        }

    auto openResult = std::make_shared<OpenResult>();

    return cacheAccessThread->ExecuteAsync([=]
        {
        ECDb::CreateParams params;
        params.SetStartDefaultTxn(DefaultTxn::No); // Allow concurrent multiple connection access

        std::unique_ptr<DataSourceCache> cache(new DataSourceCache());
        if (cacheFilePath.DoesPathExist())
            {
            if (SUCCESS != cache->Open(cacheFilePath, cacheEnvironment, params))
                {
                openResult->SetError(Status::InternalCacheError);
                return;
                }
            }
        else
            {
            if (SUCCESS != cache->Create(cacheFilePath, cacheEnvironment, params))
                {
                openResult->SetError(Status::InternalCacheError);
                return;
                }
            }

        auto cacheTransactionManager = std::make_shared<CacheTransactionManager>(std::move(cache), cacheAccessThread);
        auto infoStore = std::make_shared<RepositoryInfoStore>(cacheTransactionManager.get(), client, cacheAccessThread);

        auto ds = std::shared_ptr<CachingDataSource>(new CachingDataSource(
                                                     client,
                                                     cacheTransactionManager,
                                                     infoStore,
                                                     cacheAccessThread,
                                                     cacheEnvironment.temporaryFileCacheDir
                                                     ));

        auto txn = ds->StartCacheTransaction();
        if (ds->m_infoStore->IsCacheInitialized(txn.GetCache()))
            {
            openResult->SetSuccess(ds);
            return;
            }

        ds->UpdateSchemas(nullptr)
            ->Then(cacheAccessThread, [=] (Result updateResult)
            {
            if (!updateResult.IsSuccess())
                {
                openResult->SetError(updateResult.GetError());
                return;
                }

            auto txn = ds->StartCacheTransaction();
            if (!ds->m_infoStore->IsCacheInitialized(txn.GetCache()))
                {
                if (SUCCESS != ds->m_infoStore->SetCacheInitialized(txn.GetCache()))
                    {
                    openResult->SetError(Status::InternalCacheError);
                    BeAssert(false);
                    }
                }
            txn.Commit();

            openResult->SetSuccess(ds);
            });
        })
            ->Then<OpenResult>([=]
            {
            return *openResult;
            });
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
AsyncTaskPtr<CachingDataSource::Result> CachingDataSource::UpdateSchemas(ICancellationTokenPtr cancellationToken)
    {
    cancellationToken = CreateCancellationToken(cancellationToken);

    auto schemaDownloadResults = std::make_shared<bmap<ObjectId, WSFileResult>>();
    auto temporaryFiles = std::make_shared<bvector<TempFilePtr>>();
    auto result = std::make_shared<Result>(Result::Success());

    return m_client->GetWSClient()->GetServerInfo(cancellationToken)
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
        })
            ->Then(m_cacheAccessThread, [=]
            {
            if (cancellationToken->IsCanceled())
                {
                result->SetError(Status::Canceled);
                return;
                }
            if (!result->IsSuccess())
                {
                return;
                }

            // Ensure MetaSchema is available
            auto txn = StartCacheTransaction();
            if (!txn.GetCache().GetAdapter().HasECSchema("MetaSchema"))
                {
                if (SUCCESS != txn.GetCache().UpdateSchemas({GetMetaSchemaPath()}))
                    {
                    result->SetError(Status::InternalCacheError);
                    return;
                    }
                }

            // Update schema list
            CachedResponseKey responseKey = CreateSchemaListResponseKey(txn);
            Utf8String eTag = txn.GetCache().ReadResponseCacheTag(responseKey);
            txn.Commit();

            m_client->SendGetSchemasRequest(eTag, cancellationToken)
                ->Then(m_cacheAccessThread, [=] (WSObjectsResult& objectsResult)
                {
                if (!objectsResult.IsSuccess())
                    {
                    result->SetError(objectsResult.GetError());
                    return;
                    }

                auto txn = StartCacheTransaction();
                if (SUCCESS != txn.GetCache().CacheResponse(responseKey, objectsResult.GetValue()))
                    {
                    result->SetError(Status::InternalCacheError);
                    return;
                    }
                txn.Commit();
                });
            })
                ->Then(m_cacheAccessThread, [=]
                {
                if (cancellationToken->IsCanceled())
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

                for (ObjectIdCR schemaId : schemaIds)
                    {
                    SchemaKey schemaKey = ReadSchemaKey(txn, schemaId);
                    if (ECSchema::IsStandardSchema(schemaKey.m_schemaName))
                        {
                        continue;
                        }

                    Utf8String eTag = txn.GetCache().ReadFileCacheTag(schemaId);
                    TempFilePtr schemaFile = GetTempFileForSchema(schemaKey);

                    temporaryFiles->push_back(schemaFile);

                    m_client->SendGetFileRequest(schemaId, schemaFile->GetPath(), eTag, nullptr, cancellationToken)
                        ->Then(m_cacheAccessThread, [=] (WSFileResult& schemaFileResult)
                        {
                        schemaDownloadResults->insert({schemaId, schemaFileResult});
                        });
                    }
                })
                    ->Then(m_cacheAccessThread, [=]
                    {
                    if (cancellationToken->IsCanceled())
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

                    // Load schemas
                    std::vector<ECSchemaPtr> changedSchemas;
                    if (SUCCESS != LoadSchemas(changedSchemaPaths, changedSchemas))
                        {
                        result->SetError(Status::InternalCacheError);
                        return;
                        }

                    // Update schemas
                    auto txn = StartCacheTransaction();
                    if (SUCCESS != txn.GetCache().UpdateSchemas(changedSchemas))
                        {
                        result->SetError(Status::InternalCacheError);
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

                    temporaryFiles->clear();
                    })
                        ->Then<Result>([=]
                        {
                        return *result;
                        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaList CachingDataSource::GetRepositorySchemas(CacheTransactionCR txn)
    {
    ECSchemaList schemas;
    for (SchemaKeyCR schemaKey : GetRepositorySchemaKeys(txn))
        {
        ECSchemaCP schema = txn.GetCache().GetAdapter().GetECSchema(Utf8String(schemaKey.m_schemaName));
        if (nullptr == schema)
            {
            BeAssert(false);
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
        return keys;
        }

    for (JsonValueCR schemaDef : schemaDefs)
        {
        Utf8String schemaName = schemaDef["Name"].asString();
        uint32_t major = schemaDef["VersionMajor"].asInt();
        uint32_t minor = schemaDef["VersionMinor"].asInt();

        keys.push_back(SchemaKey(schemaName.c_str(), major, minor));
        }

    return keys;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo CachingDataSource::GetServerInfo(CacheTransactionCR txn)
    {
    return m_infoStore->GetServerInfo(txn.GetCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TempFilePtr CachingDataSource::GetTempFileForSchema(SchemaKeyCR schemaKey)
    {
    if (schemaKey.m_schemaName.empty())
        {
        return GetTempFile(BeGuid().ToString(), ObjectId());
        }

    Utf8PrintfString schemaFileName
        (
        "%s.%02d.%02d.ecschema.xml",
        Utf8String(schemaKey.m_schemaName).c_str(),
        schemaKey.m_versionMajor,
        schemaKey.m_versionMinor
        );

    return GetTempFile(schemaFileName, ObjectId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaKey CachingDataSource::ReadSchemaKey(CacheTransactionCR txn, ObjectIdCR schemaid)
    {
    Json::Value schemaDef;
    if (CacheStatus::OK != txn.GetCache().ReadInstance(schemaid, schemaDef))
        {
        BeAssert(false && "SchemaDef should be cached before calling this functions");
        return SchemaKey();
        }

    return SchemaKey(
        schemaDef["Name"].asCString(),
        schemaDef["VersionMajor"].asInt(),
        schemaDef["VersionMinor"].asInt()
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachingDataSource::LoadSchemas
(
const std::vector<BeFileName>& schemaPaths,
std::vector<ECSchemaPtr>& loadedSchemasOut
)
    {
    auto txn = StartCacheTransaction();
    auto readContext = SchemaContext::CreateReadContext();

    for (BeFileNameCR schemaPath : schemaPaths)
        {
        readContext->AddSchemaPath(schemaPath.GetDirectoryName());
        }
    readContext->AddSchemaLocater(txn.GetCache().GetAdapter().GetECDb().GetSchemaLocater());

    for (BeFileNameCR schemaPath : schemaPaths)
        {
        ECSchemaPtr schema;
        SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, schemaPath.GetName(), *readContext);
        if (SchemaReadStatus::SCHEMA_READ_STATUS_Success != status &&
            SchemaReadStatus::SCHEMA_READ_STATUS_DuplicateSchema != status)
            {
            return ERROR;
            }
        loadedSchemasOut.push_back(schema);
        }

    return SUCCESS;
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
AsyncTaskPtr<CachingDataSource::ObjectsResult> CachingDataSource::GetObject
(
ObjectIdCR objectId,
DataOrigin origin,
IDataSourceCache::JsonFormat format,
ICancellationTokenPtr cancellationToken
)
    {
    cancellationToken = CreateCancellationToken(cancellationToken);
    auto result = std::make_shared <ObjectsResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (cancellationToken->IsCanceled())
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
                txn.GetCache().ReadInstance(objectId, *cachedData, format);
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

        m_client->SendGetObjectRequest(objectId, txn.GetCache().ReadInstanceCacheTag(objectId), cancellationToken)
            ->Then(m_cacheAccessThread, [=] (WSObjectsResult& objectsResult)
            {
            auto txn = StartCacheTransaction();
            DataOrigin returningDataOrigin = DataOrigin::RemoteData;
            if (objectsResult.IsSuccess())
                {
                if (SUCCESS != txn.GetCache().UpdateInstance(objectId, objectsResult.GetValue()))
                    {
                    result->SetError(Status::InternalCacheError);
                    return;
                    }
                if (!objectsResult.GetValue().IsModified())
                    {
                    returningDataOrigin = DataOrigin::CachedData;
                    }
                }
            else
                {
                if (CachingDataSource::DataOrigin::RemoteOrCachedData == origin &&
                    WSError::Status::ConnectionError == objectsResult.GetError().GetStatus() &&
                    txn.GetCache().GetCachedObjectInfo(objectId).IsFullyCached())
                    {
                    returningDataOrigin = DataOrigin::CachedData;
                    }
                else
                    {
                    result->SetError(objectsResult.GetError());
                    return;
                    }
                }

            auto cachedData = std::make_shared<Json::Value>();
            txn.GetCache().ReadInstance(objectId, *cachedData, format);

            if (cachedData->isNull())
                {
                result->SetError(Error(Status::InternalCacheError));
                return;
                }

            txn.Commit();
            result->SetSuccess({cachedData, returningDataOrigin});
            });
        })
            ->Then<ObjectsResult>([=]
            {
            return *result;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::DataOriginResult> CachingDataSource::CacheObjects
(
CachedResponseKeyCR responseKey,
WSQueryCR query,
DataOrigin origin,
ICancellationTokenPtr cancellationToken
)
    {
    auto result = std::make_shared <DataOriginResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (cancellationToken->IsCanceled())
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
                result->SetSuccess(DataOrigin::CachedData);
                return;
                }
            else if (DataOrigin::CachedData == origin)
                {
                result->SetError(Status::DataNotCached);
                return;
                }
            }

        // connect to server for data
        Utf8String cacheTag = txn.GetCache().ReadResponseCacheTag(responseKey);
        m_client->SendQueryRequest(query, cacheTag, cancellationToken)
            ->Then(m_cacheAccessThread, [=] (WSObjectsResult& objectsResult)
            {
            auto txn = StartCacheTransaction();
            DataOrigin returningDataOrigin = DataOrigin::RemoteData;
            if (objectsResult.IsSuccess())
                {
                bset<ObjectId> rejected;
                if (SUCCESS != txn.GetCache().CachePartialResponse(responseKey, objectsResult.GetValue(), rejected, &query, cancellationToken))
                    {
                    result->SetError({ICachingDataSource::Status::InternalCacheError, cancellationToken});
                    return;
                    }

                if (!objectsResult.GetValue().IsModified())
                    {
                    returningDataOrigin = DataOrigin::CachedData;
                    }

                if (!rejected.empty())
                    {
                    SyncCachedInstancesTask::Run(this->shared_from_this(), rejected, cancellationToken)
                        ->Then(m_cacheAccessThread, [=] (BatchResult instancesResult)
                        {
                        if (instancesResult.IsSuccess())
                            {
                            result->SetSuccess(returningDataOrigin);
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
                    }
                else
                    {
                    result->SetError(objectsResult.GetError());
                    return;
                    }
                }

            txn.Commit();
            result->SetSuccess(returningDataOrigin);
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
ICancellationTokenPtr cancellationToken
)
    {
    if (nullptr == cachedSelectProvider)
        {
        cachedSelectProvider = std::make_shared<ISelectProvider>();
        }
    cancellationToken = CreateCancellationToken(cancellationToken);

    return CacheObjects(responseKey, query, origin, cancellationToken)
        ->Then<ObjectsResult>(m_cacheAccessThread, [=] (DataOriginResult& result)
        {
        if (!result.IsSuccess())
            {
            return ObjectsResult::Error(result.GetError());
            }

        auto txn = StartCacheTransaction();
        auto cachedInstances = std::make_shared<Json::Value>();
        if (CacheStatus::OK != txn.GetCache().ReadResponse(responseKey, *cachedInstances, *cachedSelectProvider))
            {
            return ObjectsResult::Error(Status::InternalCacheError);
            }

        return ObjectsResult::Success({cachedInstances, result.GetValue()});
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::KeysResult> CachingDataSource::GetObjectsKeys
(
CachedResponseKeyCR responseKey,
WSQueryCR query,
DataOrigin origin,
ICancellationTokenPtr cancellationToken
)
    {
    cancellationToken = CreateCancellationToken(cancellationToken);

    return CacheObjects(responseKey, query, origin, cancellationToken)
        ->Then<KeysResult>(m_cacheAccessThread, [=] (DataOriginResult& result)
        {
        if (!result.IsSuccess())
            {
            return KeysResult::Error(result.GetError());
            }

        auto txn = StartCacheTransaction();
        auto cachedInstances = std::make_shared<Json::Value>();
        auto keys = std::make_shared<ECInstanceKeyMultiMap>();

        CacheStatus status = txn.GetCache().ReadResponseInstanceKeys(responseKey, *keys);
        if (CacheStatus::OK != status)
            {
            return KeysResult::Error(status);
            }

        return KeysResult::Success({keys, result.GetValue()});
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
ICancellationTokenPtr cancellationToken
)
    {
    auto finalResult = std::make_shared <DataOriginResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (cancellationToken->IsCanceled())
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

        CacheObjects(responseKey, *query, origin, cancellationToken)
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
ICancellationTokenPtr cancellationToken
)
    {
    if (nullptr == selectProvider)
        {
        selectProvider = std::make_shared<SelectProvider>();
        }
    cancellationToken = CreateCancellationToken(cancellationToken);

    return CacheNavigationChildren(parentId, origin, selectProvider->GetForRemote(), cancellationToken)
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

        return ObjectsResult::Success({cachedInstances, result.GetValue()});
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
ICancellationTokenPtr cancellationToken
)
    {
    if (nullptr == selectProvider)
        {
        selectProvider = std::make_shared<ISelectProvider>();
        }
    cancellationToken = CreateCancellationToken(cancellationToken);

    return CacheNavigationChildren(parentId, origin, selectProvider, cancellationToken)
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

        return KeysResult::Success({keys, result.GetValue()});
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
    WSInfo serverInfo = GetServerInfo(txn);

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
+--------------------------------------------------------------------------------------*/
BeFileName CachingDataSource::GetMetaSchemaPath()
    {
    return SchemaContext::GetCacheSchemasDir().AppendToPath(L"MetaSchema.02.00.ecschema.xml");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::FileResult> CachingDataSource::GetFile
(
ObjectIdCR objectId,
DataOrigin origin,
LabeledProgressCallback onProgress,
ICancellationTokenPtr cancellationToken
)
    {
    cancellationToken = CreateCancellationToken(cancellationToken);

    // TODO: Support RemoteOrCachedData
    if (origin == DataOrigin::RemoteOrCachedData)
        {
        BeAssert(false && "DataOrigin::RemoteOrCachedData is not supported yet");
        }

    auto result = std::make_shared <FileResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (cancellationToken->IsCanceled())
            {
            result->SetError(Status::Canceled);
            return;
            }

        // check cache for object
        if (DataOrigin::CachedData == origin || DataOrigin::CachedOrRemoteData == origin)
            {
            auto txn = StartCacheTransaction();
            Json::Value file;
            txn.GetCache().ReadInstance(objectId, file);
            BeFileName cachedFilePath;
            if (!file.isNull())
                {
                cachedFilePath = txn.GetCache().ReadFilePath(objectId);
                if (!cachedFilePath.empty())
                    {
                    result->SetSuccess(FileData(cachedFilePath, DataOrigin::CachedData));
                    return;
                    }
                }
            if (cachedFilePath.empty() && DataOrigin::CachedData == origin)
                {
                result->SetError(Error(Status::DataNotCached));
                return;
                }
            }

        bset<ObjectId> filesToDownload;
        filesToDownload.insert(objectId);

        auto task = std::make_shared<DownloadFilesTask>
            (
            shared_from_this(),
            std::move(filesToDownload),
            FileCache::ExistingOrTemporary,
            std::move(onProgress),
            cancellationToken
            );

        m_cacheAccessThread->Push(task);

        task->Then(m_cacheAccessThread, [=]
            {
            if (!task->IsSuccess())
                {
                result->SetError(task->GetError());
                }
            else if (!task->GetFailedObjects().empty())
                {
                result->SetError(task->GetFailedObjects().front().GetError());
                }
            else
                {
                auto txn = StartCacheTransaction();
                BeFileName cachedFilePath = txn.GetCache().ReadFilePath(objectId);
                result->SetSuccess(FileData(cachedFilePath, DataOrigin::RemoteData));
                }
            });
        })
            ->Then<FileResult>([=]
            {
            return *result;
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
LabeledProgressCallback onProgress,
ICancellationTokenPtr cancellationToken
)
    {
    cancellationToken = CreateCancellationToken(cancellationToken);

    auto result = std::make_shared <BatchResult>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (cancellationToken->IsCanceled())
            {
            result->SetError(Status::Canceled);
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
            result->SetSuccess(FailedObjects());
            return;
            }

        auto task = std::make_shared<DownloadFilesTask>
            (
            shared_from_this(),
            std::move(filesToDownload),
            fileCacheLocation,
            std::move(onProgress),
            cancellationToken
            );

        m_cacheAccessThread->Push(task);

        task->Then(m_cacheAccessThread, [=]
            {
            *result = task->GetResult();
            });
        })
            ->Then<BatchResult>([=]
            {
            return *result;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::Result> CachingDataSource::DownloadAndCacheChildren
(
const bvector<ObjectId>& parentIds,
ICancellationTokenPtr cancellationToken
)
    {
    cancellationToken = CreateCancellationToken(cancellationToken);
    auto finalResult = std::make_shared<CachingDataSource::Result>();
    finalResult->SetSuccess();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        if (cancellationToken->IsCanceled())
            {
            finalResult->SetError(Status::Canceled);
            return;
            }

        for (ObjectIdCR parentId : parentIds)
            {
            CacheNavigationChildren(parentId, DataOrigin::RemoteData, nullptr, cancellationToken)
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
SyncProgressCallback onProgress,
ICancellationTokenPtr cancellationToken,
SyncOptions options
)
    {
    return SyncLocalChanges(nullptr, std::move(onProgress), cancellationToken, options);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 William.Francis     01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::SyncLocalChanges
(
const bset<ECInstanceKey>& objectsToSync,
SyncProgressCallback onProgress,
ICancellationTokenPtr cancellationToken,
SyncOptions options
)
    {
    auto objectsToSyncPtr = std::make_shared<bset<ECInstanceKey>>(objectsToSync);
    return SyncLocalChanges(objectsToSyncPtr, std::move(onProgress), cancellationToken, options);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 William.Francis     01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::BatchResult> CachingDataSource::SyncLocalChanges
(
std::shared_ptr<bset<ECInstanceKey>> objectsToSync,
SyncProgressCallback onProgress,
ICancellationTokenPtr cancellationToken,
SyncOptions options
)
    {
    cancellationToken = CreateCancellationToken(cancellationToken);

    auto syncTask = std::make_shared<SyncLocalChangesTask>
        (
        shared_from_this(),
        objectsToSync,
        options,
        std::move(onProgress),
        cancellationToken
        );

    m_cacheAccessThread->ExecuteAsync([=]
        {
        HttpClient::BeginNetworkActivity();
        m_syncLocalChangesQueue.push_back(syncTask);
        ExecuteNextSyncLocalChangesTask();
        });

    return syncTask->Then<BatchResult>(m_cacheAccessThread, [=]
        {
        auto txn = StartCacheTransaction();
        txn.GetCache().GetChangeManager().SetSyncActive(false);
        txn.Commit();
        HttpClient::EndNetworkActivity();
        ExecuteNextSyncLocalChangesTask();

        return syncTask->GetResult();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingDataSource::ExecuteNextSyncLocalChangesTask()
    {
    if (m_syncLocalChangesQueue.empty())
        {
        return;
        }

    auto txn = StartCacheTransaction();
    txn.GetCache().GetChangeManager().SetSyncActive(true);
    txn.Commit();

    auto syncTask = m_syncLocalChangesQueue.front();
    m_syncLocalChangesQueue.pop_back();
    m_cacheAccessThread->Push(syncTask);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<CachingDataSource::Result> CachingDataSource::CacheObject
(
ObjectIdCR objectId,
ICancellationTokenPtr cancellationToken
)
    {
    cancellationToken = CreateCancellationToken(cancellationToken);
    auto result = std::make_shared<CachingDataSource::Result>();

    return m_cacheAccessThread->ExecuteAsync([=]
        {
        auto txn = StartCacheTransaction();
        Utf8String cacheTag = txn.GetCache().ReadInstanceCacheTag(objectId);

        m_client->SendGetObjectRequest(objectId, cacheTag, cancellationToken)
            ->Then(m_cacheAccessThread, [=] (WSObjectsResult& objectsResult)
            {
            if (cancellationToken->IsCanceled())
                {
                result->SetError(Status::Canceled);
                return;
                }

            auto txn = StartCacheTransaction();

            if (objectsResult.IsSuccess())
                {
                if (SUCCESS != txn.GetCache().UpdateInstance(objectId, objectsResult.GetValue()))
                    {
                    result->SetError(Status::InternalCacheError);
                    return;
                    }
                result->SetSuccess();
                }
            else
                {
                result->SetError(objectsResult.GetError());

                WSError::Id errorId = objectsResult.GetError().GetId();
                if (WSError::Id::InstanceNotFound == errorId ||
                    WSError::Id::NotEnoughRights == errorId)
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
ProgressCallback onProgress,
ICancellationTokenPtr cancellationToken
)
    {
    auto task = std::make_shared<SyncCachedDataTask>
        (
        shared_from_this(),
        std::move(initialInstances),
        std::move(initialQueries),
        std::move(queryProviders),
        std::move(onProgress),
        CreateCancellationToken(cancellationToken)
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
LabeledProgressCallback onProgress,
ICancellationTokenPtr cancellationToken
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
        CreateCancellationToken(cancellationToken)
        );

    m_cacheAccessThread->Push(task);

    return task->Then<BatchResult>(m_cacheAccessThread, [=]
        {
        HttpClient::EndNetworkActivity();
        return task->GetResult();
        });
    }