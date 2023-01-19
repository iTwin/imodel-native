/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECDb/ECDbApi.h>
#include <BeSQLite/Profiler.h>
#include "../Shared/ExtendedData.h"
#include "../UpdateHandler.h"
#include "NavNodesHelper.h"
#include "NavNodesCache.h"
#include "NavNodesDataSource.h"
#include "NavNodeProviders.h"

USING_NAMESPACE_BENTLEY_LOGGING

#define NAVNODES_CACHE_DB_SUFFIX            L"-hierarchies"
#define NAVNODES_CACHE_DB_VERSION_MAJOR     35
#define NAVNODES_CACHE_DB_VERSION_MINOR     0

#define NAVNODES_CACHE_LockWaitTime 200
#define NAVNODES_CACHE_LockTimeout 90000

#ifdef NAVNODES_CACHE_BINARY_GUID
#define NAVNODES_CACHE_ID_TYPE "GUID"
#else
#define NAVNODES_CACHE_ID_TYPE "TEXT"
#endif

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InfiniteBusyRetry : BusyRetry
    {
    int _OnBusy(int count) const override
        {
        BeThreadUtilities::BeSleep(1);
        return 1;
        }
    static RefCountedPtr<InfiniteBusyRetry> Create() {return new InfiniteBusyRetry();}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationCacheBusyRetry : BusyRetry
    {
    int _OnBusy(int count) const override
        {
        return 0;
        }
    static RefCountedPtr<NavigationCacheBusyRetry> Create() {return new NavigationCacheBusyRetry();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter::DataSourceFilter(std::unique_ptr<RelatedInstanceInfo> relatedInstanceInfo, Utf8String instanceFilter)
    : m_relatedInstanceInfo(std::move(relatedInstanceInfo)), m_instanceFilter(instanceFilter)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter::DataSourceFilter(DataSourceFilter const& other)
    : m_instanceFilter(other.m_instanceFilter)
    {
    if (nullptr != other.m_relatedInstanceInfo)
        m_relatedInstanceInfo = std::make_unique<RelatedInstanceInfo>(*other.m_relatedInstanceInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter::DataSourceFilter(DataSourceFilter&& other)
    : m_relatedInstanceInfo(std::move(other.m_relatedInstanceInfo)), m_instanceFilter(std::move(other.m_instanceFilter))
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter& DataSourceFilter::operator=(DataSourceFilter const& other)
    {
    m_instanceFilter = other.m_instanceFilter;
    if (nullptr != other.m_relatedInstanceInfo)
        m_relatedInstanceInfo = std::make_unique<RelatedInstanceInfo>(*other.m_relatedInstanceInfo);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter& DataSourceFilter::operator=(DataSourceFilter&& other)
    {
    m_instanceFilter = std::move(other.m_instanceFilter);
    m_relatedInstanceInfo = std::move(other.m_relatedInstanceInfo);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DataSourceFilter::InitFromJson(RapidJsonValueCR json)
    {
    m_relatedInstanceInfo = nullptr;
    m_instanceFilter.clear();

    if (json.IsNull())
        return;

    if (json.HasMember("RelatedInstanceInfo"))
        {
        RapidJsonValueCR relatedInstanceInfoJson = json["RelatedInstanceInfo"];
        RequiredRelationDirection direction = (RequiredRelationDirection)relatedInstanceInfoJson["Direction"].GetInt();
        RapidJsonValueCR relationshipIdsJson = relatedInstanceInfoJson["ECRelationshipClassIds"];
        bvector<ECClassId> relationshipIds;
        if (relationshipIdsJson.IsArray())
            {
            for (rapidjson::SizeType i = 0; i < relationshipIdsJson.Size(); ++i)
                relationshipIds.push_back(ECClassId(relationshipIdsJson[i].GetUint64()));
            }
        RapidJsonValueCR instanceKeysJson = relatedInstanceInfoJson["ECInstanceKeys"];
        bvector<ECInstanceKey> instanceKeys;
        if (instanceKeysJson.IsArray())
            {
            for (rapidjson::SizeType i = 0; i < instanceKeysJson.Size(); ++i)
                {
                RapidJsonValueCR instanceKeyJson = instanceKeysJson[i];
                instanceKeys.push_back(ECInstanceKey(ECClassId(instanceKeyJson["ECClassId"].GetUint64()), ECInstanceId(instanceKeyJson["ECInstanceId"].GetUint64())));
                }
            }
        m_relatedInstanceInfo = std::make_unique<RelatedInstanceInfo>(relationshipIds, direction, instanceKeys);
        }
    if (json.HasMember("Filter"))
        m_instanceFilter = json["Filter"].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DataSourceFilter::AsJson() const
    {
    rapidjson::Document json;
    if (nullptr != m_relatedInstanceInfo)
        {
        json.SetObject();
        rapidjson::Value relatedInstanceInfoJson;
        relatedInstanceInfoJson.SetObject();
        relatedInstanceInfoJson.AddMember("Direction", (int)m_relatedInstanceInfo->m_direction, json.GetAllocator());
        rapidjson::Value relationshipClassIdsJson(rapidjson::kArrayType);
        for (ECClassId classId : m_relatedInstanceInfo->m_relationshipClassIds)
            relationshipClassIdsJson.PushBack(classId.GetValue(), json.GetAllocator());
        relatedInstanceInfoJson.AddMember("ECRelationshipClassIds", relationshipClassIdsJson, json.GetAllocator());
        rapidjson::Value instanceKeysJson(rapidjson::kArrayType);
        for (ECInstanceKeyCR instanceKey : m_relatedInstanceInfo->m_instanceKeys)
            {
            rapidjson::Value instanceKeyJson(rapidjson::kObjectType);
            instanceKeyJson.AddMember("ECClassId", instanceKey.GetClassId().GetValueUnchecked(), json.GetAllocator());
            instanceKeyJson.AddMember("ECInstanceId", instanceKey.GetInstanceId().GetValueUnchecked(), json.GetAllocator());
            instanceKeysJson.PushBack(instanceKeyJson, json.GetAllocator());
            }
        relatedInstanceInfoJson.AddMember("ECInstanceKeys", instanceKeysJson, json.GetAllocator());
        json.AddMember("RelatedInstanceInfo", relatedInstanceInfoJson, json.GetAllocator());
        }
    if (!m_instanceFilter.empty())
        {
        json.SetObject();
        json.AddMember("Filter", rapidjson::Value(m_instanceFilter.c_str(), json.GetAllocator()), json.GetAllocator());
        }
    return json;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ConcatBinaryIndexScalar : BeSQLite::ScalarFunction
    {
#ifdef NAVNODES_CACHE_BINARY_INDEX
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        int length = 0;
        for (int i = 0; i < nArgs; ++i)
            length += args[i].GetValueBytes() / sizeof(uint64_t);

        bvector<uint64_t> result;
        result.reserve(length);

        for (int i = 0; i < nArgs; ++i)
            {
            NodesCacheHelpers::PushToIndex(
                result,
                static_cast<uint64_t const*>(args[i].GetValueBlob()),
                args[i].GetValueBytes() / sizeof(uint64_t));
            }
        ctx.SetResultBlob(reinterpret_cast<void const*>(&result.front()), result.size() * sizeof(uint64_t), DbFunction::Context::CopyData::Yes);
        }
#else
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        bvector<Utf8CP> pieces;
        for (int i = 0; i < nArgs; ++i)
            {
            if (!args[i].IsNull() && 0 != *args[i].GetValueText())
                pieces.push_back(args[i].GetValueText());
            }
        auto result = BeStringUtilities::Join(pieces, "-");
        ctx.SetResultText(result.c_str(), result.size(), DbFunction::Context::CopyData::Yes);
        }
#endif
    ConcatBinaryIndexScalar()
        : ScalarFunction(NODESCACHE_FUNCNAME_ConcatBinaryIndex, -1, DbValueType::BlobVal)
        {}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct VariablesMatchScalar : BeSQLite::ScalarFunction
    {
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        RulesetVariables lhs = RulesetVariables::FromSerializedInternalJsonObjectString(args[0].GetValueText());
        RulesetVariables rhs = RulesetVariables::FromSerializedInternalJsonObjectString(args[1].GetValueText());
        bool fullMatch = (3 == nArgs && 0 != args[2].GetValueInt());

        bool variablesMatches = fullMatch ? lhs == rhs : rhs.Contains(lhs);
        ctx.SetResultInt(variablesMatches ? 1 : 0);
        }
    VariablesMatchScalar()
        : ScalarFunction(NODESCACHE_FUNCNAME_VariablesMatch, -1, DbValueType::IntegerVal)
        {}
    };

/*=================================================================================**//**
* Parameters:
* - GUID
* Returns: string with concatenated GUIDs.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GuidConcatAggregate : BeSQLite::AggregateFunction
{
private:
    Utf8StringR GetSerializedAggregateGuid(Context& ctx)
        {
        Utf8String** guidsPP = (Utf8String**)ctx.GetAggregateContext(sizeof(Utf8String*));
        Utf8String*& guidP = *guidsPP;
        if (nullptr == guidP)
            guidP = new Utf8String("");
        return *guidP;
        }
protected:
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].IsNull())
            return;

        Utf8StringR agg = GetSerializedAggregateGuid(ctx);
        if (!agg.empty())
            agg.append(",");
#ifdef NAVNODES_CACHE_BINARY_GUID
        agg.append(ValueHelpers::GuidToString(args[0].GetValueGuid()));
#else
        agg.append(args[0].GetValueText());
#endif
        }
    void _FinishAggregate(Context& ctx) override
        {
        Utf8StringCR agg = GetSerializedAggregateGuid(ctx);
        if (agg.empty())
            ctx.SetResultNull();
        else
            ctx.SetResultText(agg.c_str(), (int)agg.size(), DbFunction::Context::CopyData::Yes);
        delete& agg;
        }
public:
    GuidConcatAggregate()
        : AggregateFunction(NODESCACHE_FUNCNAME_GuidConcat, 1, DbValueType::TextVal)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetSerializedJson(RapidJsonValueCR json)
    {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    json.Accept(writer);
    return buf.GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int64_t GetConnectionLastModTime(IConnectionCR connection)
    {
    time_t fileModTime;
    BeFileName(connection.GetDb().GetDbFileName()).GetFileTime(nullptr, nullptr, &fileModTime);
    return fileModTime;
    }

static PropertySpec s_versionPropertySpec("Version", "HierarchyCache");
static PropertySpec s_cachesUpdateDataFlagPropertySpec("CachesUpdateData", "HierarchyCache");
static PropertySpec s_connectionLastModTimePropertySpec("ConnectionLastModTime", "HierarchyCache");
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BeVersion GetCacheVersion(BeSQLite::Db const& db)
    {
    Utf8String versionStr;
    if (BE_SQLITE_ROW != db.QueryProperty(versionStr, s_versionPropertySpec))
        return BeVersion();
    return BeVersion(versionStr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsConnectionAndCacheOutOfSync(BeSQLite::Db const& db, IConnectionCR connection)
    {
    Utf8String lastModValueStr;
    if (BE_SQLITE_ROW != db.QueryProperty(lastModValueStr, s_connectionLastModTimePropertySpec))
        return false;

    int64_t lastModValue = atoll(lastModValueStr.c_str());
    return GetConnectionLastModTime(connection) != lastModValue;
    }

#ifndef NAVNODES_CACHE_DEBUG_REMOVE_DB
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DeleteSQLiteDbFile(BeFileNameCR path)
    {
#if !defined (BENTLEYCONFIG_OS_WINDOWS)
    // on platforms other than windows BeDeleteFile deletes file even if it is opened. In that case attempt to open SQLite db in exclusive mode to make sure
    // that it is not opened by other processes.
    Db db;
    auto busyRetry = NavigationCacheBusyRetry::Create();
    DbResult openResult = db.OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive, busyRetry.get()));
    if (BE_SQLITE_OK != openResult)
        return false;
#endif
    return BeFileNameStatus::Success == path.BeDeleteFile();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName GetCacheDbPath(BeFileNameCR directory, IConnectionCR connection)
    {
    BeFileName path;
    if (directory.IsEmpty())
        {
        path = BeFileName(connection.GetECDb().GetTempFileBaseName().c_str());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, Utf8PrintfString("Cache directory not set, using base path: '%s'", path.GetNameUtf8().c_str()));
        }
    else
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, directory.DoesPathExist(), Utf8PrintfString("Provided cache directory does not exist: '%s'", directory.GetNameUtf8().c_str()));
        path = directory;
        path.AppendToPath(BeFileName(connection.GetECDb().GetDbFileName()).GetFileNameAndExtension().c_str());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, Utf8PrintfString("Cache directory set, using base path: '%s'", path.GetNameUtf8().c_str()));
        }
    path.AppendString(NAVNODES_CACHE_DB_SUFFIX);
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsMemoryCache(NodesCacheType cacheType)
    {
    return NodesCacheType::Memory == cacheType || NodesCacheType::HybridMemory == cacheType;
    }

#define EVALUATE_SQLITE_RESULT(db, result) \
    if (BE_SQLITE_OK != (result)) \
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, db.GetLastError());

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NodesCache::DbFactory::CheckCacheCompatibility(BeSQLite::Db& db, IConnectionCR connection)
    {
    DbResult result = BE_SQLITE_OK;
    if (GetCacheVersion(db).GetMajor() != NAVNODES_CACHE_DB_VERSION_MAJOR)
        {
        // if the existing cache version is too old, simply delete the old cache and create a new one
        db.CloseDb();
        result = BE_SQLITE_ERROR_ProfileTooOld;
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, "Profile too old, deleted DB file");
        }
    else if (IsConnectionAndCacheOutOfSync(db, connection))
        {
        // if connection modification date does not match cached date delete cache (hierarchies may be out of sync)
        db.CloseDb();
        result = BE_SQLITE_ERROR_ProfileTooOld;
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, "Cache is out-of-sync, deleted DB file");
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NodesCache::DbFactory::CreateCacheDb(IConnectionCR connection, BeSQLite::Db& db, BeFileNameCR path, DefaultTxn txnLockType, RefCountedPtr<BusyRetry> busyHandler)
    {
    DbResult result = db.CreateNewDb(path,Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8,
        true, txnLockType, busyHandler.get()));

    if (BE_SQLITE_OK != result)
        return result;

    // check if new cache was created
    if (GetCacheVersion(db).IsEmpty())
        {
        // save the cache version
        static BeVersion s_cacheVersion(NAVNODES_CACHE_DB_VERSION_MAJOR, NAVNODES_CACHE_DB_VERSION_MINOR);
        db.SaveProperty(s_versionPropertySpec, s_cacheVersion.ToString(), nullptr, 0);
        db.SaveChanges();
        return BE_SQLITE_OK;
        }

    // in case other process created hierarchy cache check if it's compatible
    return CheckCacheCompatibility(db, connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NodesCache::DbFactory::OpenCacheDb(IConnectionCR connection, BeSQLite::Db& db, BeFileNameCR path, DefaultTxn txnLockType, RefCountedPtr<BusyRetry> busyHandler)
    {
    DbResult result = db.OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::ReadWrite, txnLockType, busyHandler.get()));
    if (BE_SQLITE_OK != result)
        return result;

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, "DB opened for read-write successfully");
    return CheckCacheCompatibility(db, connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::DbFactory::SetupDbConnection(Db& db, NodesCacheType type, Nullable<uint64_t> const& memoryCacheLimit)
    {
    bool wasTransactionActive = db.GetDefaultTransaction()->IsActive();
    if (wasTransactionActive)
        db.GetDefaultTransaction()->Commit();

    db.TryExecuteSql("PRAGMA main.journal_mode=WAL");
    if (IsMemoryCache(type))
        {
        db.TryExecuteSql("PRAGMA main.synchronous=0");
        }
    else
        {
        db.TryExecuteSql("PRAGMA main.synchronous=1");
        db.TryExecuteSql("PRAGMA main.journal_size_limit=0");
        }

    if (memoryCacheLimit.IsValid())
        {
        BeSQLite::Savepoint savepoint(db, "Set memory cache size");
        uint64_t cacheSizeInKb = memoryCacheLimit.Value() / 1024;
        db.TryExecuteSql(Utf8PrintfString("PRAGMA cache_size=-%" PRIu64, cacheSizeInKb).c_str());
        }

    if (wasTransactionActive)
        db.GetDefaultTransaction()->Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NodesCache::DbFactory::InitializeCacheTables(Db& db)
    {
    BeSQLite::Savepoint savepoint(db, "CreateTables", true, BeSQLiteTxnMode::Immediate);
    if (!db.TableExists(NODESCACHE_TABLENAME_HierarchyLevels))
        {
        Utf8CP ddl =
            "[Id] " NAVNODES_CACHE_ID_TYPE " PRIMARY KEY NOT NULL, "
            "[ParentNodeId] " NAVNODES_CACHE_ID_TYPE " REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[RulesetId] " NAVNODES_CACHE_ID_TYPE " NOT NULL REFERENCES " NODESCACHE_TABLENAME_Rulesets "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[RemovalId] " NAVNODES_CACHE_ID_TYPE ", "
            "[LockTimestamp] INTEGER ";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_HierarchyLevels, ddl));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE UNIQUE INDEX [UX_" NODESCACHE_TABLENAME_HierarchyLevels "_Parent] ON [" NODESCACHE_TABLENAME_HierarchyLevels "]([ParentNodeId], [RemovalId])"));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_" NODESCACHE_TABLENAME_HierarchyLevels "_RemovalId] ON [" NODESCACHE_TABLENAME_HierarchyLevels "]([RemovalId])"));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_" NODESCACHE_TABLENAME_HierarchyLevels "_RulesetId] ON [" NODESCACHE_TABLENAME_HierarchyLevels "]([RulesetId])"));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_DataSources))
        {
        Utf8CP ddl =
            "[Id] " NAVNODES_CACHE_ID_TYPE " PRIMARY KEY NOT NULL, "
            "[HierarchyLevelId] " NAVNODES_CACHE_ID_TYPE " REFERENCES " NODESCACHE_TABLENAME_HierarchyLevels "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[VariablesId] " NAVNODES_CACHE_ID_TYPE " REFERENCES " NODESCACHE_TABLENAME_Variables "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
#ifdef NAVNODES_CACHE_BINARY_INDEX
            "[Index] BINARY, "
#else
            "[Index] TEXT NOT NULL, "
#endif
            "[InstanceFilter] TEXT, "
            "[DirectNodesCount] INTEGER, "
            "[IsInitialized] BOOLEAN NOT NULL DEFAULT FALSE, "
            "[HasNodes] BOOLEAN, "
            "[TotalNodesCount] INTEGER, "
            "[Filter] TEXT, "
            "[SpecificationHash] TEXT, "
            "[NodeTypes] TEXT, "
            "[ParentId] " NAVNODES_CACHE_ID_TYPE " DEFAULT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[CustomJson] TEXT DEFAULT ''";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_DataSources, ddl));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_DataSources_HierarchyLevelId] ON [" NODESCACHE_TABLENAME_DataSources "]([HierarchyLevelId],[InstanceFilter],[VariablesId],[Index])"));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_DataSources_ParentDataSourceId] ON [" NODESCACHE_TABLENAME_DataSources "]([ParentId])"));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_DataSourceClasses))
        {
        Utf8CP ddl =
            "[DataSourceId] " NAVNODES_CACHE_ID_TYPE " NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[ECClassId] INTEGER NOT NULL, "
            "[Polymorphic] BOOLEAN NOT NULL DEFAULT FALSE, "
            "PRIMARY KEY([DataSourceId], [ECClassId])";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_DataSourceClasses, ddl));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_Variables))
        {
        Utf8CP ddl =
            "[Id] " NAVNODES_CACHE_ID_TYPE " PRIMARY KEY NOT NULL, "
            "[RulesetId] " NAVNODES_CACHE_ID_TYPE " NOT NULL REFERENCES " NODESCACHE_TABLENAME_Rulesets "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[Variables] TEXT NOT NULL ";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_Variables, ddl));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_Variables_RulesetId] ON [" NODESCACHE_TABLENAME_Variables "]([RulesetId])"));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_Nodes))
        {
        Utf8CP ddl =
            "[Id] " NAVNODES_CACHE_ID_TYPE " PRIMARY KEY NOT NULL, "
            "[HierarchyLevelId] " NAVNODES_CACHE_ID_TYPE " NOT NULL, "
            "[Data] TEXT NOT NULL, "
            "[Label] TEXT, "
            "[InstanceKeysSelectQuery] TEXT";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_Nodes, ddl));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_" NODESCACHE_TABLENAME_Nodes "_HierarchyLevelId] ON [" NODESCACHE_TABLENAME_Nodes "]([HierarchyLevelId])"));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_NodeKeys))
        {
        Utf8CP ddl =
            "[NodeId] " NAVNODES_CACHE_ID_TYPE " PRIMARY KEY NOT NULL UNIQUE REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[Type] TEXT NOT NULL,"
            "[SpecificationIdentifier] TEXT NOT NULL,"
            "[ClassId] INT,"
            "[IsClassPolymorphic] BOOLEAN,"
            "[PropertyName] INT,"
            "[SerializedPropertyValues] INT,"
            "[GroupedInstanceKeysCount] INT,"
            "[GroupedInstanceKeys] TEXT,"
            "[PathFromRoot] TEXT NOT NULL";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_NodeKeys, ddl));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_NodeKeys_Lookup] ON [" NODESCACHE_TABLENAME_NodeKeys "]([Type],[PathFromRoot])"));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_MergedNodes))
        {
        Utf8CP ddl =
            "[MergingNodeId] " NAVNODES_CACHE_ID_TYPE " NOT NULL REFERENCES " NODESCACHE_TABLENAME_Nodes"([Id]) ON DELETE CASCADE ON UPDATE CASCADE,"
            "[MergedNodeId] " NAVNODES_CACHE_ID_TYPE " NOT NULL REFERENCES " NODESCACHE_TABLENAME_Nodes"([Id]) ON DELETE CASCADE ON UPDATE CASCADE,"
            "PRIMARY KEY([MergingNodeId], [MergedNodeId])";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_MergedNodes, ddl));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_NodeInstances))
        {
        Utf8CP ddl =
            "[NodeId] " NAVNODES_CACHE_ID_TYPE " NOT NULL REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[ECClassId] INTEGER NOT NULL, "
            "[ECInstanceId] INTEGER NOT NULL, "
            "PRIMARY KEY([NodeId],[ECClassId],[ECInstanceId])";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_NodeInstances, ddl));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_NodeInstances_InstanceKeys] ON [" NODESCACHE_TABLENAME_NodeInstances "]([ECClassId],[ECInstanceId])"));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_DataSourceNodes))
        {
        Utf8CP ddl =
            "[DataSourceId] " NAVNODES_CACHE_ID_TYPE " NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
            "[NodeId] " NAVNODES_CACHE_ID_TYPE " NOT NULL REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
#ifdef NAVNODES_CACHE_BINARY_INDEX
            "[NodeIndex] BINARY, "
#else
            "[NodeIndex] TEXT NOT NULL, "
#endif
            "[Visibility] INTEGER NOT NULL DEFAULT 0, "
            "PRIMARY KEY([DataSourceId], [NodeId])";
            ;
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_DataSourceNodes, ddl));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_" NODESCACHE_TABLENAME_DataSourceNodes "_NodeId] ON [" NODESCACHE_TABLENAME_DataSourceNodes "]([NodeId])"));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_" NODESCACHE_TABLENAME_DataSourceNodes "_Order] ON [" NODESCACHE_TABLENAME_DataSourceNodes "]([DataSourceId], [NodeIndex])"));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE INDEX [IX_" NODESCACHE_TABLENAME_DataSourceNodes "_Visibility] ON [" NODESCACHE_TABLENAME_DataSourceNodes "]([DataSourceId], [Visibility])"));

        Utf8CP triggerDdl = "CREATE TRIGGER IF NOT EXISTS [TRIGG_" NODESCACHE_TABLENAME_DataSourceNodes "_OnDataSourceDelete] AFTER DELETE ON [" NODESCACHE_TABLENAME_DataSourceNodes "] "
            "BEGIN"
            "   DELETE FROM [" NODESCACHE_TABLENAME_Nodes "] "
            "   WHERE [Id] NOT IN ("
            "       SELECT [NodeId] "
            "       FROM " NODESCACHE_TABLENAME_DataSourceNodes ""
            "   );"
            "END";
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql(triggerDdl));
        }
    if (!db.TableExists(NODESCACHE_TABLENAME_Rulesets))
        {
        Utf8CP ddl =
            "[Id] " NAVNODES_CACHE_ID_TYPE " PRIMARY KEY NOT NULL, "
            "[Identifier] TEXT NOT NULL, "
            "[Hash] TEXT NOT NULL, "
            "[LastUsedTime] INTEGER NOT NULL";
        EVALUATE_SQLITE_RESULT(db, db.CreateTable(NODESCACHE_TABLENAME_Rulesets, ddl));
        EVALUATE_SQLITE_RESULT(db, db.ExecuteSql("CREATE UNIQUE INDEX [UX_Rulesets_Identifier] ON [" NODESCACHE_TABLENAME_Rulesets "]([Identifier])"));
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NodesCache::DbFactory::CreateTempDiskDb(Db& db, BeFileNameCR baseFileName, IConnectionCR connection, NodesCacheType cacheType)
    {
    BeFileName path(baseFileName);
    path.AppendUtf8(Utf8PrintfString("-%s", BeGuid(true).ToString().c_str()).c_str());
    return CreateCacheDb(connection, db, path, DefaultTxn::Yes, InfiniteBusyRetry::Create());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NodesCache::DbFactory::InitializeDiskDb(Db& db, BeFileNameCR directory, IConnectionCR connection, NodesCacheType cacheType, bool& tempCache)
    {
    BeFileName path = GetCacheDbPath(directory, connection);

    if (tempCache)
        return CreateTempDiskDb(db, path, connection, cacheType);

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, Utf8PrintfString("Using path '%s'", path.GetNameUtf8().c_str()));

    bool createNewCache = true;
#ifdef NAVNODES_CACHE_DEBUG_REMOVE_DB
    // always create a new cache if debugging
    path.BeDeleteFile();
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, "Deleted the cache for debugging");
#else
    if (path.DoesPathExist())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, Utf8PrintfString("File exists: '%s'", path.GetNameUtf8().c_str()));
        DbResult result = OpenCacheDb(connection, db, path, DefaultTxn::No, InfiniteBusyRetry::Create());
        if (BE_SQLITE_OK == result)
            return BE_SQLITE_OK;

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, Utf8PrintfString("Failed to open the cache with result '%d'. Creating a new one.", (int)result));
        // attempt to delete old cache file. Sets 'createNewCache' to false if old file could not be deleted
        createNewCache = DeleteSQLiteDbFile(path);
    }
#endif

    // attempt to create new cache db
    if (createNewCache)
        {
        DbResult result = CreateCacheDb(connection, db, path, DefaultTxn::No, InfiniteBusyRetry::Create());
        if (BE_SQLITE_OK == result)
            return BE_SQLITE_OK;
        }

    tempCache = true;
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE, "Could not open or create main cache DB, switching to a temporary cache");
    return CreateTempDiskDb(db, path, connection, cacheType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NodesCache::DbFactory::OpenMemoryDb(Db& db) const
    {
    auto busyRetry = InfiniteBusyRetry::Create();
    DbResult result = db.CreateNewDb(nullptr, Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8,
        true, DefaultTxn::Yes, busyRetry.get()));
    if (result != BE_SQLITE_OK)
        return result;

    SetupDbConnection(db, NodesCacheType::Memory, nullptr);

    // create the tables
    if (InitializeCacheTables(db) != SUCCESS)
        return DbResult::BE_SQLITE_ERROR;

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NodesCache::DbFactory::OpenDiskDb(Db& db) const
    {
    auto busyRetry = InfiniteBusyRetry::Create();
    DbResult result = db.OpenBeSQLiteDb(m_cachePath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No, busyRetry.get()));
    if (result != BE_SQLITE_OK)
        return result;

    SetupDbConnection(db, NodesCacheType::Disk, m_memoryCacheLimit);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<NodesCache::DbFactory> NodesCache::DbFactory::Create(IConnectionCR connection, BeFileNameCR directory, NodesCacheType cacheType,
    uint64_t sizeLimit, Nullable<uint64_t> const& memoryCacheLimit)
    {
    auto scope = Diagnostics::Scope::Create("Initialize hierarchy cache");
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_INFO, Utf8PrintfString("Directory: '%s', Connection: '%s', CacheType: '%s'",
        directory.GetNameUtf8().c_str(), connection.GetId().c_str(), cacheType == NodesCacheType::Memory ? "Memory" : "Disk"));

    bool tempCache = false;
#ifdef NAVNODES_CACHE_DEBUG
    // always use Disk cache when debugging
    switch (cacheType)
        {
        case NodesCacheType::Memory: cacheType = NodesCacheType::Disk; break;
        case NodesCacheType::HybridMemory: cacheType = NodesCacheType::HybridDisk; tempCache = true; break;
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_INFO, "Overriding CacheType to 'Disk' for hierarchy cache debugging");
#endif

    if (IsMemoryCache(cacheType))
        return std::make_shared<DbFactory>(connection, nullptr, sizeLimit, memoryCacheLimit);

    Db db;
    DbResult result = InitializeDiskDb(db, directory, connection, cacheType, tempCache);
    if (result != DbResult::BE_SQLITE_OK)
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Failed to initialize nodes cache. Directory: '%s', "
            "Type: %d, IsTemp: %s", directory.GetNameUtf8().c_str(), (int)cacheType, tempCache ? "TRUE" : "FALSE"));
        }
    SetupDbConnection(db, cacheType, nullptr);

    // create the tables
    if (InitializeCacheTables(db) != SUCCESS)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Failed to initialize nodes cache tables."));

    db.SaveChanges();
    return std::make_shared<DbFactory>(connection, db.GetDbFileName(), sizeLimit, memoryCacheLimit, tempCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache::DbFactory::~DbFactory()
    {
    if (m_cachePath.empty())
        return;

    BeFileName dbFile(m_cachePath);
    if (m_deleteDb)
        {
        dbFile.BeDeleteFile();
        return;
        }

    // try to open db in exclusive mode to check if there is no more connections to this db
    // if this is the last connection to db update 'connectionLastModTime' and limit cache size
    Db db;
    auto busyRetry = NavigationCacheBusyRetry::Create();
    DbResult openResult = db.OpenBeSQLiteDb(dbFile, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive, busyRetry.get()));
    if (BE_SQLITE_OK != openResult)
        return;

    db.SaveProperty(s_connectionLastModTimePropertySpec, std::to_string(GetConnectionLastModTime(m_connection)), nullptr, 0);
    db.GetDefaultTransaction()->Commit();
    NodesCacheHelpers::LimitCacheSize(db, m_sizeLimit, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NodesCache::DbFactory::CreateDbConnection(Db& db) const
    {
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_INFO, Utf8PrintfString("Cache file: '%s'", m_cachePath.c_str()));
    if (m_cachePath.empty())
        return OpenMemoryDb(db);

    return OpenDiskDb(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<NodesCache> NodesCache::Create(std::shared_ptr<DbFactory> initializedCache, NavNodesFactoryCR nodesFactory, INodesProviderContextFactoryCR contextFactory,
    INodesProviderFactoryCR providersFactory, bool ensureThreadSafety)
    {
    if (nullptr == initializedCache)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Cache is not initialized.");

    auto cache = std::shared_ptr<NodesCache>(new NodesCache(initializedCache, nodesFactory, contextFactory, providersFactory, ensureThreadSafety));
    if (SUCCESS != cache->OpenCache())
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Failed to open nodes cache. Directory: '%s'.",
            initializedCache->GetCachePath().c_str()));
        }
    return cache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<NodesCache> NodesCache::Create(IConnectionCR connection, BeFileNameCR directory, NavNodesFactoryCR nodesFactory, INodesProviderContextFactoryCR contextFactory,
    INodesProviderFactoryCR providersFactory, NodesCacheType type, bool ensureThreadSafety)
    {
    auto dbFactory = DbFactory::Create(connection, directory, type, 0, nullptr);
    return Create(dbFactory, nodesFactory, contextFactory, providersFactory, ensureThreadSafety);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus NodesCache::OpenCache()
    {
    auto scope = Diagnostics::Scope::Create("Open hierarchy cache");
    DbResult result = m_dbFactory->CreateDbConnection(m_db);
    EVALUATE_SQLITE_RESULT(m_db, result);

    m_customFunctions.push_back(std::make_unique<VariablesMatchScalar>());
    m_db.AddFunction(*m_customFunctions.back());
    m_customFunctions.push_back(std::make_unique<GuidConcatAggregate>());
    m_db.AddFunction(*m_customFunctions.back());
    m_customFunctions.push_back(std::make_unique<ConcatBinaryIndexScalar>());
    m_db.AddFunction(*m_customFunctions.back());

#ifdef NAVNODES_CACHE_PROFILE_DB
    Profiler::InitScope(m_db, "hierarchies-cache", Utf8PrintfString("%" PRIu64, (uint64_t)&m_db).c_str(), Profiler::Params(false, true));
    Profiler::GetScope(m_db)->Start();
#endif

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_INFO, "Opened successfully");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache::NodesCache(std::shared_ptr<DbFactory> dbFactory, NavNodesFactoryCR nodesFactory, INodesProviderContextFactoryCR contextFactory,
    INodesProviderFactoryCR providersFactory, bool ensureThreadSafety)
    : m_dbFactory(dbFactory), m_nodesFactory(nodesFactory), m_contextFactory(contextFactory), m_providersFactory(providersFactory),
    m_statements(50), m_ensureThreadSafety(ensureThreadSafety)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache::~NodesCache()
    {
    m_statements.Empty();
    if (!m_db.IsDbOpen() || Utf8String::IsNullOrEmpty(m_db.GetDbFileName()))
        return;

    m_db.SaveChanges();
    m_db.CloseDb();
    }

#define LOCK_MUTEX_ON_CONDITION(mutex, condition) \
    BeMutexHolder lock(mutex, BeMutexHolder::Lock::No); \
    if (condition) \
        lock.lock();

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SavePointWithMutexHolder
    {
    private:
        BeMutexHolder m_mutexHolder;
        WalSavepoint m_savepoint;

    public:
        SavePointWithMutexHolder(BeMutex& mutex, bool lockMutex, Db& db, Utf8CP name, BeSQLiteTxnMode txnMode)
            : m_mutexHolder(mutex, BeMutexHolder::Lock::No), m_savepoint(db, name, txnMode, false)
            {
            if (lockMutex)
                m_mutexHolder.lock();

            m_savepoint.Begin();
            }
        ~SavePointWithMutexHolder() {m_savepoint.Commit();}
        void Cancel() {m_savepoint.Cancel();}
        bool IsActive() const {return m_savepoint.IsActive();}
        DbResult Commit() {return m_savepoint.Commit();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::Optimize()
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);
    m_db.TryExecuteSql("PRAGMA analysis_limit=500");
    m_db.TryExecuteSql("PRAGMA optimize");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_OnRulesetUsed(PresentationRuleSetCR ruleset)
    {
    SavePointWithMutexHolder savepoint(m_mutex, m_ensureThreadSafety, m_db, "OnRulesetUsed", BeSQLiteTxnMode::Immediate);

    static Utf8CP query = "SELECT [Id], [Hash] FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [Identifier] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare ruleset hash query");

    stmt->BindText(1, ruleset.GetRuleSetId(), Statement::MakeCopy::No);

    BeGuid id = BE_SQLITE_ROW == stmt->Step() ? NodesCacheHelpers::GetGuid(*stmt, 0) : BeGuid();
    if (id.IsValid() && !ruleset.GetHash().Equals(stmt->GetValueText(1)))
        {
        static Utf8CP deleteQuery = "DELETE FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [Id] = ?";
        CachedStatementPtr deleteStmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(deleteStmt, *m_db.GetDbFile(), deleteQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare ruleset deletion query");

        NodesCacheHelpers::BindGuid(*deleteStmt, 1, id);
        deleteStmt->Step();
        id.Invalidate();
        }

    if (!id.IsValid())
        {
        static Utf8CP insertQuery = "INSERT INTO [" NODESCACHE_TABLENAME_Rulesets "] ([Id], [Identifier], [Hash], [LastUsedTime]) VALUES (?, ?, ?, ?)";
        CachedStatementPtr insertStmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(insertStmt, *m_db.GetDbFile(), insertQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare ruleset insertion query");

        NodesCacheHelpers::BindGuid(*insertStmt, 1, BeGuid(true));
        insertStmt->BindText(2, ruleset.GetRuleSetId(), Statement::MakeCopy::No);
        insertStmt->BindText(3, ruleset.GetHash(), Statement::MakeCopy::No);
        insertStmt->BindUInt64(4, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        insertStmt->Step();
        }
    else
        {
        static Utf8CP updateQuery =
            "UPDATE [" NODESCACHE_TABLENAME_Rulesets "] "
            "SET [LastUsedTime] = ? WHERE [Id] = ?";
        CachedStatementPtr updateStmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(updateStmt, *m_db.GetDbFile(), updateQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare ruleset update query");

        updateStmt->BindUInt64(1, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        NodesCacheHelpers::BindGuid(*updateStmt, 2, id);
        updateStmt->Step();
        }
    stmt = nullptr;
    savepoint.Commit();

    NodesCacheHelpers::LimitCacheSize(m_db, m_dbFactory->GetSizeLimit());

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCache::Savepoint : IHierarchyCache::Savepoint
    {
    private:
        SavePointWithMutexHolder m_sqliteSavepoint;
        NodesCache& m_cache;
        bool m_isCancelled;
        bool m_optimize;
    private:
        size_t GetNodesCount() const
            {
            static Utf8CP query = "SELECT COUNT(*) FROM [" NODESCACHE_TABLENAME_Nodes "]";
            CachedStatementPtr stmt;
            if (BE_SQLITE_OK != m_cache.m_statements.GetPreparedStatement(stmt, *m_cache.m_db.GetDbFile(), query))
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare nodes count query");

            stmt->Step();
            return (size_t)stmt->GetValueUInt64(0);
            }
    public:
        Savepoint(NodesCache& cache, bool lockMutex, BeSQLiteTxnMode txnMode, bool optimize)
            : m_sqliteSavepoint(cache.GetMutex(), lockMutex, cache.m_db, BeGuid(true).ToString().c_str(), txnMode), m_cache(cache), m_isCancelled(false), m_optimize(optimize)
            {
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, m_sqliteSavepoint.IsActive(), "Failed to start transaction");
            }
        ~Savepoint()
            {
            if (m_isCancelled || !m_optimize)
                return;
            size_t nodesCountAfter = GetNodesCount();
            m_sqliteSavepoint.Commit();
            if (nodesCountAfter > 1000)
                {
                // call optimize only when there are more than 1000 nodes in cache - we've seen that
                // causing more damage than good when there's little data to optimize
                m_cache.Optimize();
                }
            }
        void _Cancel() override
            {
            m_isCancelled = true;
            m_sqliteSavepoint.Cancel();
            BeMutexHolder lockQuick(m_cache.m_quickCacheMutex);
            m_cache.m_quickNodesCache.clear();
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IHierarchyCache::SavepointPtr NodesCache::CreateSavepointInternal(BeSQLiteTxnMode txnMode, bool optimize)
    {
    return new Savepoint(*this, m_ensureThreadSafety, txnMode, optimize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IHierarchyCache::SavepointPtr NodesCache::_CreateSavepoint(bool bulkTransaction) {return CreateSavepointInternal(BeSQLiteTxnMode::Immediate, bulkTransaction);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<IHierarchyLevelLocker> NodesCache::_CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const& identifier)
    {
    return std::make_shared<NodesCacheHierarchyLevelLocker>(*this, identifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveHierarchyLevelLock(BeGuidCR hierarchyLevelId)
    {
    SavePointWithMutexHolder savepoint(m_mutex, m_ensureThreadSafety, m_db, "RemoveHierarchyLevelLock", BeSQLiteTxnMode::Immediate);

    static Utf8CP query =
        " UPDATE [" NODESCACHE_TABLENAME_HierarchyLevels "] "
        " SET [LockTimestamp] = NULL "
        " WHERE [Id] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare RemoveHierarchyLevelLock query");

    NodesCacheHelpers::BindGuid(*stmt, 1, hierarchyLevelId);
    DbResult res = stmt->Step();

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, BE_SQLITE_DONE == res,
        Utf8PrintfString("Failed to unlock hierarchy level - %s with status - %d", hierarchyLevelId.ToString().c_str(), (int)res).c_str());
    }

#define FLAT_HIERARCHYLEVELS_COLUMN_NAME_LevelId "LevelId"
#define FLAT_HIERARCHYLEVELS_COLUMN_NAME_ParentId "ParentId"
#define FLAT_HIERARCHYLEVELS_COLUMN_NAME_RemovalId "RemovalId"
#define FLAT_HIERARCHYLEVELS_COLUMN_NAME_LockTimestamp "LockTimestamp"
#define FLAT_HIERARCHYLEVELS_TABLE_NAME "flat_hierarchy_levels"
#define WITH_FLAT_HIERARCHYLEVELS_BASE \
    "WITH RECURSIVE " \
    "    " FLAT_HIERARCHYLEVELS_TABLE_NAME "(" \
                FLAT_HIERARCHYLEVELS_COLUMN_NAME_LevelId ", " \
                FLAT_HIERARCHYLEVELS_COLUMN_NAME_ParentId ", " \
                FLAT_HIERARCHYLEVELS_COLUMN_NAME_LockTimestamp ", " \
                FLAT_HIERARCHYLEVELS_COLUMN_NAME_RemovalId \
    "    ) AS (" \
    "        SELECT [hl].[Id], [hl].[ParentNodeId], [hl].[LockTimestamp], [hl].[RemovalId] " \
    "        FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl " \
    "        JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hl].[RulesetId] " \
    "        WHERE [hl].[ParentNodeId] IS ? AND [r].[Identifier] = ? AND [hl].[RemovalId] IS ? " \
    "        UNION ALL "

#define WITH_FLAT_ANCESTOR_HIERARCHYLEVELS \
    WITH_FLAT_HIERARCHYLEVELS_BASE \
    "        SELECT [hl].[Id], [hl].[ParentNodeId], [hl].[LockTimestamp], [hl].[RemovalId] " \
    "        FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl " \
    "        JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[HierarchyLevelId] = [hl].[Id] " \
    "        JOIN [" FLAT_HIERARCHYLEVELS_TABLE_NAME "] child_level ON [child_level].[" FLAT_HIERARCHYLEVELS_COLUMN_NAME_ParentId "] = [n].[Id] " \
    "        WHERE [child_level].[" FLAT_HIERARCHYLEVELS_COLUMN_NAME_RemovalId "] IS [hl].[RemovalId]" \
    "    )"

#define WITH_FLAT_CHILD_HIERARCHYLEVELS \
    WITH_FLAT_HIERARCHYLEVELS_BASE \
    "        SELECT [hl].[Id], [hl].[ParentNodeId], [hl].[LockTimestamp], [hl].[RemovalId] " \
    "        FROM [" FLAT_HIERARCHYLEVELS_TABLE_NAME "] parent_level " \
    "        JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[HierarchyLevelId] = [parent_level].[" FLAT_HIERARCHYLEVELS_COLUMN_NAME_LevelId "] " \
    "        JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[ParentNodeId] = [n].[Id] " \
    "        WHERE [parent_level].[" FLAT_HIERARCHYLEVELS_COLUMN_NAME_RemovalId "] IS [hl].[RemovalId]" \
    "    )"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::_IsHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const& identifier) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);
    static Utf8CP query =
        WITH_FLAT_ANCESTOR_HIERARCHYLEVELS
        " SELECT [" FLAT_HIERARCHYLEVELS_COLUMN_NAME_LevelId "], [" FLAT_HIERARCHYLEVELS_COLUMN_NAME_LockTimestamp "] "
        " FROM " FLAT_HIERARCHYLEVELS_TABLE_NAME
        " WHERE [LockTimestamp] IS NOT NULL AND ((strftime('%s','now') || substr(strftime('%f','now'), 4)) - [LockTimestamp]) < ?  ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare IsHierarchyLevelLocked query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, identifier.GetPhysicalParentNodeId()); // for WITH_FLAT_ANCESTOR_HIERARCHYLEVELS
    stmt->BindText(bindingIndex++, identifier.GetRulesetId(), Statement::MakeCopy::No); // for WITH_FLAT_ANCESTOR_HIERARCHYLEVELS
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, identifier.GetRemovalId()); // for WITH_FLAT_ANCESTOR_HIERARCHYLEVELS
    stmt->BindUInt64(bindingIndex++, NAVNODES_CACHE_LockTimeout);

    DbResult stepResult = stmt->Step();
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, (stepResult == BE_SQLITE_DONE || stepResult == BE_SQLITE_ROW), Utf8PrintfString("Checking ancestor hierarchy levels locks ended with unexpected result - %d", (int)stepResult).c_str());
    return BE_SQLITE_DONE != stepResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::IsAnyChildHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const& identifier) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);
    static Utf8CP query =
        WITH_FLAT_CHILD_HIERARCHYLEVELS
        " SELECT [" FLAT_HIERARCHYLEVELS_COLUMN_NAME_LevelId "], [" FLAT_HIERARCHYLEVELS_COLUMN_NAME_LockTimestamp "] "
        " FROM " FLAT_HIERARCHYLEVELS_TABLE_NAME
        " WHERE [LockTimestamp] IS NOT NULL AND ((strftime('%s','now') || substr(strftime('%f','now'), 4)) - [LockTimestamp]) < ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare IsHierarchyLevelLocked query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, identifier.GetPhysicalParentNodeId()); // for WITH_FLAT_CHILD_HIERARCHYLEVELS
    stmt->BindText(bindingIndex++, identifier.GetRulesetId(), Statement::MakeCopy::No); // for WITH_FLAT_CHILD_HIERARCHYLEVELS
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, identifier.GetRemovalId()); // for WITH_FLAT_CHILD_HIERARCHYLEVELS
    stmt->BindUInt64(bindingIndex++, NAVNODES_CACHE_LockTimeout);

    DbResult stepResult = stmt->Step();
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, (stepResult == BE_SQLITE_DONE || stepResult == BE_SQLITE_ROW), Utf8PrintfString("Checking child hierarchy levels locks ended with unexpected result - %d", (int)stepResult).c_str());
    return BE_SQLITE_DONE != stepResult;
    }

/*---------------------------------------------------------------------------------**//**
* note: must be called within a mutex
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t NodesCache::UpdateHierarchyLevelLock(BeGuidCR levelId)
    {
    static Utf8CP query =
        " UPDATE [" NODESCACHE_TABLENAME_HierarchyLevels "] "
        " SET [LockTimestamp] = ? "
        " WHERE [Id] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare UpdatePhysicalHierarchyLevelLock query");

    uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    stmt->BindUInt64(1, timestamp);
    NodesCacheHelpers::BindGuid(*stmt, 2, levelId);
    DbResult res = stmt->Step();

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE,
        Utf8PrintfString("Updated hierachy lock for level - %s on thread - %d with status - %d", levelId.ToString().c_str(), (int)BeThreadUtilities::GetCurrentThreadId(), (int)res));

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    return timestamp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void BindOptionalInstanceKeys(Statement& stmt, int bindingIndex, bvector<ECInstanceKey> const* keys)
    {
    if (keys)
        stmt.BindText(bindingIndex, ValueHelpers::GetECInstanceKeysAsJsonString(*keys), Statement::MakeCopy::Yes);
    else
        stmt.BindNull(bindingIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheNodeKey(NavNodeCR node)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    // delete the old key, if exists
    static Utf8CP deleteQuery =
        "DELETE FROM [" NODESCACHE_TABLENAME_NodeKeys "] "
        "WHERE [NodeId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), deleteQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node key deletion query");

    NodesCacheHelpers::BindGuid(*stmt, 1, node.GetNodeId());
    stmt->Step();
    stmt = nullptr;

    // insert a new key
    NavNodeKeyCR key = *node.GetKey();
    if (key.AsECInstanceNodeKey())
        {
        static Utf8CP insertQuery =
            "INSERT INTO [" NODESCACHE_TABLENAME_NodeKeys "] "
            "([NodeId],[Type],[SpecificationIdentifier],[PathFromRoot])"
            "VALUES (?, ?, ?, ?)";

        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare instance node key insertion query");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, node.GetNodeId());
        stmt->BindText(bindingIndex++, key.GetType(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, key.GetSpecificationIdentifier(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, NavNodesHelper::NodeKeyHashPathToString(key), Statement::MakeCopy::Yes);
        stmt->Step();

        CacheNodeInstanceKeys(node.GetNodeId(), key.AsECInstanceNodeKey()->GetInstanceKeys());
        }
    else if (key.AsECClassGroupingNodeKey())
        {
        static Utf8CP insertQuery =
            "INSERT INTO [" NODESCACHE_TABLENAME_NodeKeys "] "
            "([NodeId],[Type],[SpecificationIdentifier],[ClassId],[IsClassPolymorphic],[GroupedInstanceKeysCount],[GroupedInstanceKeys],[PathFromRoot])"
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare class grouping node key insertion query");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, node.GetNodeId());
        stmt->BindText(bindingIndex++, key.GetType(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, key.GetSpecificationIdentifier(), Statement::MakeCopy::No);
        stmt->BindId(bindingIndex++, key.AsECClassGroupingNodeKey()->GetECClassId());
        stmt->BindBoolean(bindingIndex++, key.AsECClassGroupingNodeKey()->IsPolymorphic());
        stmt->BindUInt64(bindingIndex++, key.AsECClassGroupingNodeKey()->GetGroupedInstancesCount());
        BindOptionalInstanceKeys(*stmt, bindingIndex++, key.AsECClassGroupingNodeKey()->GetGroupedInstanceKeys());
        stmt->BindText(bindingIndex++, NavNodesHelper::NodeKeyHashPathToString(key), Statement::MakeCopy::Yes);
        stmt->Step();
        }
    else if (key.AsECPropertyGroupingNodeKey())
        {
        static Utf8CP insertQuery =
            "INSERT INTO [" NODESCACHE_TABLENAME_NodeKeys "] "
            "([NodeId],[Type],[SpecificationIdentifier],[ClassId],[PropertyName],[SerializedPropertyValues],[GroupedInstanceKeysCount],[GroupedInstanceKeys],[PathFromRoot])"
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare property grouping node key insertion query");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, node.GetNodeId());
        stmt->BindText(bindingIndex++, key.GetType(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, key.GetSpecificationIdentifier(), Statement::MakeCopy::No);
        stmt->BindId(bindingIndex++, key.AsECPropertyGroupingNodeKey()->GetECClassId());
        stmt->BindText(bindingIndex++, key.AsECPropertyGroupingNodeKey()->GetPropertyName(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, BeRapidJsonUtilities::ToString(key.AsECPropertyGroupingNodeKey()->GetGroupingValuesArray()), Statement::MakeCopy::Yes);
        stmt->BindUInt64(bindingIndex++, key.AsECPropertyGroupingNodeKey()->GetGroupedInstancesCount());
        BindOptionalInstanceKeys(*stmt, bindingIndex++, key.AsECPropertyGroupingNodeKey()->GetGroupedInstanceKeys());
        stmt->BindText(bindingIndex++, NavNodesHelper::NodeKeyHashPathToString(key), Statement::MakeCopy::Yes);
        stmt->Step();
        }
    else if (key.AsLabelGroupingNodeKey())
        {
        static Utf8CP insertQuery =
            "INSERT INTO [" NODESCACHE_TABLENAME_NodeKeys "] "
            "([NodeId],[Type],[SpecificationIdentifier],[GroupedInstanceKeysCount],[GroupedInstanceKeys],[PathFromRoot])"
            "VALUES (?, ?, ?, ?, ?, ?)";

        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node key insertion query");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, node.GetNodeId());
        stmt->BindText(bindingIndex++, key.GetType(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, key.GetSpecificationIdentifier(), Statement::MakeCopy::No);
        stmt->BindUInt64(bindingIndex++, key.AsLabelGroupingNodeKey()->GetGroupedInstancesCount());
        BindOptionalInstanceKeys(*stmt, bindingIndex++, key.AsLabelGroupingNodeKey()->GetGroupedInstanceKeys());
        stmt->BindText(bindingIndex++, NavNodesHelper::NodeKeyHashPathToString(key), Statement::MakeCopy::Yes);
        stmt->Step();
        }
    else
        {
        static Utf8CP insertQuery =
            "INSERT INTO [" NODESCACHE_TABLENAME_NodeKeys "] "
            "([NodeId],[Type],[SpecificationIdentifier],[PathFromRoot])"
            "VALUES (?, ?, ?, ?)";

        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node key insertion query");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, node.GetNodeId());
        stmt->BindText(bindingIndex++, key.GetType(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, key.GetSpecificationIdentifier(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, NavNodesHelper::NodeKeyHashPathToString(key), Statement::MakeCopy::Yes);
        stmt->Step();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheNodeInstanceKeys(BeGuidCR nodeId, bvector<ECClassInstanceKey> const& keys)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    // delete old keys, if any
    static Utf8CP deleteQuery =
        "DELETE FROM [" NODESCACHE_TABLENAME_NodeInstances "] "
        "WHERE [NodeId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), deleteQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node instance keys deletion query");

    NodesCacheHelpers::BindGuid(*stmt, 1, nodeId);
    stmt->Step();

    if (keys.empty())
        return;

    // insert new keys
    static Utf8CP insertQuery =
        "INSERT INTO [" NODESCACHE_TABLENAME_NodeInstances "] "
        "([NodeId], [ECClassId], [ECInstanceId]) "
        "VALUES (?, ?, ?)";

    stmt = nullptr;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node instance keys insertion query");

    for (auto const& key : keys)
        {
        NodesCacheHelpers::BindGuid(*stmt, 1, nodeId);
        stmt->BindId(2, key.GetClass()->GetId());
        stmt->BindId(3, key.GetId());
        stmt->Step();
        stmt->Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheMergedNodeIds(NavNodeCR node)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    // delete old ids, if any
    static Utf8CP deleteQuery =
        "DELETE FROM [" NODESCACHE_TABLENAME_MergedNodes"] "
        "WHERE [MergingNodeId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), deleteQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare merged node IDs deletion query");

    NodesCacheHelpers::BindGuid(*stmt, 1, node.GetNodeId());
    stmt->Step();
    stmt = nullptr;

    auto mergedNodeIds = NavNodeExtendedData(node).GetMergedNodeIds();
    if (mergedNodeIds.empty())
        return;

    // insert new keys
    static Utf8CP insertQuery =
        "INSERT INTO [" NODESCACHE_TABLENAME_MergedNodes "]([MergingNodeId], [MergedNodeId]) VALUES (?, ?)";
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare merged node IDs insertion query");

    for (BeGuidCR mergedNodeId : mergedNodeIds)
        {
        NodesCacheHelpers::BindGuid(*stmt, 1, node.GetNodeId());
        NodesCacheHelpers::BindGuid(*stmt, 2, mergedNodeId);
        stmt->Step();
        stmt->Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheDataSourceNodeAssociation(BeGuidCR datasourceId, BeGuidCR nodeId, bvector<uint64_t> const& nodeIndex, NodeVisibility visibility)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "INSERT OR IGNORE INTO [" NODESCACHE_TABLENAME_DataSourceNodes "] ("
        "  [DataSourceId], [NodeId], [NodeIndex], [Visibility]"
        ") VALUES (?, ?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node insertion query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, datasourceId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, nodeId);
    NodesCacheHelpers::BindVectorIndex(*stmt, bindingIndex++, nodeIndex, true);
    stmt->BindInt(bindingIndex++, (int)visibility);

    DbResult result = stmt->Step();
    if (BE_SQLITE_DONE != result)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Unexpected step result: %d", (int)result));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheNode(BeGuidCR hierarchyLevelId, NavNodeR node)
    {
    static Utf8CP query =
        "INSERT INTO [" NODESCACHE_TABLENAME_Nodes "] ("
        "  [Id], [HierarchyLevelId], [Data], [Label], [InstanceKeysSelectQuery]"
        ") VALUES (?, ?, ?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node insertion query");

    int bindingIndex = 1;
    BeGuid nodeId(true);

    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, nodeId);

    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, hierarchyLevelId);

    Utf8String nodeStr = GetSerializedJson(NavNodesHelper::SerializeNodeToJson(node));
    stmt->BindText(bindingIndex++, nodeStr.c_str(), Statement::MakeCopy::No);

    stmt->BindText(bindingIndex++, node.GetLabelDefinition().GetDisplayValue(), Statement::MakeCopy::Yes);

    if (node.GetInstanceKeysSelectQuery() != nullptr)
        stmt->BindText(bindingIndex++, BeRapidJsonUtilities::ToString(node.GetInstanceKeysSelectQuery()->ToJson()), Statement::MakeCopy::Yes);
    else
        stmt->BindNull(bindingIndex++);

    DbResult result = stmt->Step();
    if (BE_SQLITE_DONE != result)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Unexpected step result: %d", (int)result));

    node.SetNodeId(nodeId);

    CacheNodeKey(node);
    CacheMergedNodeIds(node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::CacheHierarchyLevel(BeGuidCR parentNodeId, Utf8StringCR rulesetId, BeGuidCR removalId)
    {
    static Utf8CP query =
        "INSERT INTO [" NODESCACHE_TABLENAME_HierarchyLevels "] "
        "([Id], [ParentNodeId], [RemovalId], [RulesetId]) "
        "VALUES (?, ?, ?, "
        "  (SELECT [Id] FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [Identifier] = ?)"
        ") ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare hierarchy level insertion query");

    BeGuid levelId(true);
    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, levelId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, parentNodeId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, removalId);
    stmt->BindText(bindingIndex++, rulesetId, Statement::MakeCopy::No);

    DbResult insertResult = stmt->Step();
    if (BE_SQLITE_DONE != insertResult)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Unexpected step result: %d", (int)insertResult));

    return levelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::CacheOrGetPhysicalHierarchyLevel(CombinedHierarchyLevelIdentifier const& identifier)
    {
    SavePointWithMutexHolder savepoint(m_mutex, m_ensureThreadSafety, m_db, "CacheOrGetPhysicalHierarchyLevel", BeSQLiteTxnMode::Immediate);

    BeGuid levelId = FindPhysicalHierarchyLevelId(identifier);
    if (levelId.IsValid())
        return levelId;

    return CacheHierarchyLevel(identifier.GetPhysicalParentNodeId(), identifier.GetRulesetId(), identifier.GetRemovalId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheEmptyDataSource(DataSourceIdentifier& info, DataSourceFilter const& filter, BeGuidCR variablesId, Utf8StringCR specificationHash, Utf8StringCR nodeTypes,
    BeGuidCR parentId, Nullable<size_t> const& directNodesCount, bool isFinalized, Nullable<bool> const& hasNodes, Nullable<size_t> const& nodesCount)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "INSERT INTO [" NODESCACHE_TABLENAME_DataSources "] ("
        "[Id], [HierarchyLevelId], [Index], [InstanceFilter], [Filter], [VariablesId], [SpecificationHash], [NodeTypes], [ParentId], [HasNodes], [DirectNodesCount], [TotalNodesCount], [IsInitialized]"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare data source insertion query");

    int bindingIndex = 1;
    Utf8String filterStr = GetSerializedJson(filter.AsJson());
    BeGuid dataSourceId(true);

    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, dataSourceId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, info.GetHierarchyLevelId());
    NodesCacheHelpers::BindVectorIndex(*stmt, bindingIndex++, info.GetIndex(), false);
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, info.GetInstanceFilter().get());
    stmt->BindText(bindingIndex++, filterStr.c_str(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, variablesId);
    stmt->BindText(bindingIndex++, specificationHash, Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, nodeTypes, Statement::MakeCopy::No);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, parentId);
    if (hasNodes.IsNull())
        stmt->BindNull(bindingIndex++);
    else
        stmt->BindBoolean(bindingIndex++, hasNodes.Value());
    if (directNodesCount.IsNull())
        stmt->BindNull(bindingIndex++);
    else
        stmt->BindUInt64(bindingIndex++, (uint64_t)directNodesCount.Value());
    if (nodesCount.IsNull())
        stmt->BindNull(bindingIndex++);
    else
        stmt->BindInt64(bindingIndex++, (int64_t)nodesCount.Value());
    stmt->BindBoolean(bindingIndex++, isFinalized);

    DbResult result = stmt->Step();
    if (BE_SQLITE_DONE != result)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Unexpected step result: %d", (int)result));

    info.SetId(dataSourceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheRelatedClassIds(BeGuidCR datasourceId, bmap<ECClassId, bool> const& classIds)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP deleteQuery =
        "DELETE FROM [" NODESCACHE_TABLENAME_DataSourceClasses "] "
        "WHERE [DataSourceId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), deleteQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare related class ids deletion query");

    NodesCacheHelpers::BindGuid(*stmt, 1, datasourceId);
    DbResult deleteResult = stmt->Step();
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, (BE_SQLITE_DONE == deleteResult), Utf8PrintfString("Unexpected step result: %d", (int)deleteResult));

    static Utf8CP insertQuery =
        "INSERT INTO [" NODESCACHE_TABLENAME_DataSourceClasses "] "
        "([DataSourceId], [ECClassId], [Polymorphic]) "
        "VALUES (?, ?, ?)";
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare related class ids insertion query");

    for (auto const& pair : classIds)
        {
        stmt->Reset();
        NodesCacheHelpers::BindGuid(*stmt, 1, datasourceId);
        stmt->BindUInt64(2, pair.first.GetValue());
        stmt->BindBoolean(3, pair.second);
        DbResult insertResult = stmt->Step();
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, (BE_SQLITE_DONE == insertResult), Utf8PrintfString("Unexpected step result: %d", (int)insertResult));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::CacheRelatedVariables(BeGuidCR rulesetId, RulesetVariables const& relatedVariables)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP selectQuery =
        "SELECT [v].[Id]"
        "  FROM " NODESCACHE_TABLENAME_Variables " v "
        " WHERE [v].[RulesetId] = ? "
        "    AND " NODESCACHE_FUNCNAME_VariablesMatch "([v].[Variables], ? , TRUE)";

    CachedStatementPtr selectStmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(selectStmt, *m_db.GetDbFile(), selectQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare related variables selection query");

    NodesCacheHelpers::BindGuid(*selectStmt, 1, rulesetId);
    selectStmt->BindText(2, relatedVariables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    DbResult selectResult = selectStmt->Step();
    if (BE_SQLITE_ROW == selectResult)
        return NodesCacheHelpers::GetGuid(*selectStmt, 0);

    static Utf8CP insertQuery =
        "INSERT INTO [" NODESCACHE_TABLENAME_Variables "] ("
            "[Id], "
            "[Variables], "
            "[RulesetId] "
        ")"
        "VALUES ("
            "?, "
            "? , "
            "?"
        ") ";

    CachedStatementPtr insertStmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(insertStmt, *m_db.GetDbFile(), insertQuery))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare related variables insertion query");

    BeGuid variablesId(true);
    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*insertStmt, bindingIndex++, variablesId);
    insertStmt->BindText(bindingIndex++, relatedVariables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindGuid(*insertStmt, bindingIndex++, rulesetId);

    DbResult insertResult = insertStmt->Step();
    if (BE_SQLITE_DONE != insertResult)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Unexpected step result: %d", (int)insertResult));

    return variablesId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::_FindPhysicalHierarchyLevelId(CombinedHierarchyLevelIdentifier const& identifier) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);
    return NodesCacheHelpers::GetHierarchyLevelId(m_db, identifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::_FindHierarchyLevelId(Utf8CP, Utf8CP rulesetId, BeGuidCR virtualParentNodeId, BeGuidCR removalId) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    Utf8String query =
        "SELECT [hl].[Id] "
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ";

    if (nullptr != rulesetId)
        query.append("  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hl].[RulesetId] ");

    query.append(" WHERE [hl].[RemovalId] IS ? AND [hl].[ParentNodeId] IS ? ");

    if (nullptr != rulesetId)
        query.append(" AND [r].[Identifier] = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare hierarchy level selection query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, removalId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, virtualParentNodeId);

    if (nullptr != rulesetId)
        stmt->BindText(bindingIndex++, rulesetId, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return BeGuid();

    return NodesCacheHelpers::GetGuid(*stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::GetHierarchyLevelRulesetId(BeGuidCR hierarchyLevelId) const
    {
    static Utf8CP query =
        "SELECT [hl].[RulesetId]"
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        " WHERE [hl].[Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare hierarchy level ruleset ID selection query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, hierarchyLevelId);

    if (BE_SQLITE_ROW != stmt->Step())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Didn't find the requested hierarchy level");

    return NodesCacheHelpers::GetGuid(*stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCache::CreateDataSourceInfo(DataSourceIdentifier identifier, int partsToGet) const
    {
    if (!identifier.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Attempting to query data source info for invalid data source");

    RulesetVariables vars;
    DataSourceFilter filter;
    Utf8String specificationHash, nodeTypes;
    BeGuid parentId;
    bool isFinalized = false;
    Nullable<size_t> directNodesCount;
    Nullable<size_t> totalNodesCount;
    Nullable<bool> hasNodes;
    Nullable<bool> hasPartialProviders;
    Json::Value customJson;
    if (0 != ((DataSourceInfo::PARTS_All & ~DataSourceInfo::PART_RelatedClasses) & partsToGet))
        {
        LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

        static Utf8CP query =
            "SELECT [Filter], [SpecificationHash], [NodeTypes], [Variables], [ParentId], [DirectNodesCount], [IsInitialized], [TotalNodesCount], [HasNodes], [CustomJson], "
            "       (SELECT 1 FROM [" NODESCACHE_TABLENAME_DataSources "] partial_ds WHERE [partial_ds].[ParentId] = [ds].[Id] LIMIT 1) [HasPartialProviders]"
            "FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
            "JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
            "WHERE [ds].[Id] = ?";
        CachedStatementPtr stmt;
        if (BE_SQLITE_OK == m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
            {
            NodesCacheHelpers::BindGuid(*stmt, 1, identifier.GetId());
            if (BE_SQLITE_ROW == stmt->Step())
                {
                if (0 != (DataSourceInfo::PART_Filter & partsToGet))
                    {
                    rapidjson::Document json;
                    json.Parse(stmt->GetValueText(0));
                    if (!json.IsNull())
                        filter = DataSourceFilter(json);
                    }

                if (0 != (DataSourceInfo::PART_SpecificationHash & partsToGet))
                    specificationHash = stmt->GetValueText(1);

                if (0 != (DataSourceInfo::PART_NodeTypes & partsToGet))
                    nodeTypes = stmt->GetValueText(2);

                if (0 != (DataSourceInfo::PART_Vars & partsToGet))
                    vars = RulesetVariables::FromSerializedInternalJsonObjectString(stmt->GetValueText(3));

                if (0 != (DataSourceInfo::PART_ParentId & partsToGet))
                    parentId = NodesCacheHelpers::GetGuid(*stmt, 4);

                if (0 != (DataSourceInfo::PART_DirectNodesCount & partsToGet) && !stmt->IsColumnNull(5))
                    directNodesCount = (size_t)stmt->GetValueUInt64(5);

                if (0 != (DataSourceInfo::PART_IsFinalized & partsToGet) && !stmt->IsColumnNull(6))
                    isFinalized = stmt->GetValueBoolean(6);

                if (0 != (DataSourceInfo::PART_TotalNodesCount & partsToGet) && !stmt->IsColumnNull(7))
                    totalNodesCount = (size_t)stmt->GetValueUInt64(7);

                if (0 != (DataSourceInfo::PART_HasNodes & partsToGet) && !stmt->IsColumnNull(8))
                    hasNodes = stmt->GetValueBoolean(8);

                if (0 != (DataSourceInfo::PART_CustomJson & partsToGet) && !stmt->IsColumnNull(9))
                    customJson = Json::Value::From(stmt->GetValueText(9));

                if (0 != (DataSourceInfo::PART_HasPartialProviders & partsToGet))
                    hasPartialProviders = stmt->GetValueBoolean(10);
                }
            }
        }

    bmap<ECClassId, bool> relatedClassIds;
    if (0 != (DataSourceInfo::PART_RelatedClasses & partsToGet))
        {
        LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

        static Utf8CP query =
            "SELECT [ECClassId], [Polymorphic] "
            "FROM [" NODESCACHE_TABLENAME_DataSourceClasses "] dsc "
            "WHERE [dsc].[DataSourceId] = ?";
        CachedStatementPtr stmt;
        if (BE_SQLITE_OK == m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
            {
            NodesCacheHelpers::BindGuid(*stmt, 1, identifier.GetId());
            while (BE_SQLITE_ROW == stmt->Step())
                relatedClassIds.Insert(stmt->GetValueId<ECClassId>(0), stmt->GetValueBoolean(1));
            }
        }

    DataSourceInfo info(identifier, vars, filter, relatedClassIds, specificationHash, nodeTypes);
    info.SetParentId(parentId);
    info.SetHasNodes(hasNodes);
    info.SetHasPartialProviders(hasPartialProviders);
    info.SetDirectNodesCount(directNodesCount);
    info.SetTotalNodesCount(totalNodesCount);
    info.SetIsInitialized(isFinalized);
    info.GetCustomJson().swap(customJson);
    return info;
    }

#ifdef wip_enable_display_label_postprocessor
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DataSourceInfo> NodesCache::_FindDataSources(CombinedHierarchyLevelIdentifier const& hlIdentifier, RulesetVariables const& variables, int partsToGet) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "SELECT [ds].[Id], [hl].[Id], [ds].[Index], [ds].[IsInitialized]  "
        "FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
        "JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        "JOIN [" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] phl ON [phl].[Id] = [hl].[PhysicalHierarchyLevelId] "
        "JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[RulesetId] "
        "JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
        "WHERE " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
        "      AND [r].[Identifier] = ? "
        "      AND [phl].[PhysicalParentNodeId] IS ? "
        "      AND [phl].[RemovalId] IS ? "
        "ORDER BY [ds].[Index] ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare data source selection query");

    int bindingIndex = 0;
    stmt->BindText(++bindingIndex, variables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    stmt->BindText(++bindingIndex, hlIdentifier.GetRulesetId().c_str(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindGuid(*stmt, ++bindingIndex, hlIdentifier.GetPhysicalParentNodeId());
    NodesCacheHelpers::BindGuid(*stmt, ++bindingIndex, hlIdentifier.GetRemovalId());

    bvector<DataSourceInfo> infos;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        DataSourceIdentifier identifier(
            NodesCacheHelpers::GetGuid(*stmt, 0),
            NodesCacheHelpers::GetGuid(*stmt, 1),
            NodesCacheHelpers::GetVectorIndex(*stmt, 2, false),
            stmt->GetValueBoolean(3)
        );
        infos.push_back(CreateDataSourceInfo(identifier, partsToGet));
        }
    return infos;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCache::_FindDataSource(DataSourceIdentifier const& identifier, RulesetVariables const& variables, int partsToGet) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "SELECT [ds].[Id] "
        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
        " WHERE [hl].[Id] = ? "
        "       AND [ds].[Index] IS ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
        "       AND [ds].[InstanceFilter] IS ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare data source selection query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, identifier.GetHierarchyLevelId());
    NodesCacheHelpers::BindVectorIndex(*stmt, bindingIndex++, identifier.GetIndex(), false);
    stmt->BindText(bindingIndex++, variables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, identifier.GetInstanceFilter().get());

    if (BE_SQLITE_ROW != stmt->Step())
        return DataSourceInfo();

    DataSourceIdentifier persistentIdentifier(identifier);
    persistentIdentifier.SetId(NodesCacheHelpers::GetGuid(*stmt, 0));

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, stmt->Step() == BE_SQLITE_DONE, "Query returned more than 1 row for data source request");

    return CreateDataSourceInfo(persistentIdentifier, partsToGet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesCache::_GetPhysicalParentNode(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const
    {
    static Utf8CP query =
        "SELECT [hl].[ParentNodeId], [parent_dsn].[Visibility] "
        // find id of the parent node
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [n].[HierarchyLevelId] "
        // also need visibility of the parent
        "  LEFT JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] parent_dsn ON [parent_dsn].[NodeId] = [hl].[ParentNodeId] "
        "  LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] parent_ds ON [parent_ds].[Id] = [parent_dsn].[DataSourceId] AND [parent_ds].[InstanceFilter] IS ? "
        "  LEFT JOIN [" NODESCACHE_TABLENAME_Variables "] parent_dsv ON [parent_dsv].[Id] = [parent_ds].[VariablesId] AND " NODESCACHE_FUNCNAME_VariablesMatch "([parent_dsv].[Variables], ?) "
        " WHERE [n].[Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare physical parent node selection query");

    BeGuid currId = nodeId;
    while (true)
        {
        stmt->Reset();

        int bindingIndex = 1;
        NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, instanceFilter);
        stmt->BindText(bindingIndex++, contextVariables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, currId);

        if (BE_SQLITE_ROW != stmt->Step())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to find a physical parent node");

        currId = NodesCacheHelpers::GetGuid(*stmt, 0);

        if (!currId.IsValid() || NodeVisibility::Virtual != (NodeVisibility)stmt->GetValueInt(1))
            break;
        }

    return GetNode(currId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::_GetVirtualParentNodeId(BeGuidCR nodeId) const
    {
    static Utf8CP query =
        "SELECT [hl].[ParentNodeId] "
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [n].[HierarchyLevelId] "
        " WHERE [n].[Id] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare virtual parent node selection query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, nodeId);

    if (BE_SQLITE_ROW != stmt->Step())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to find a virtual parent node");

    return NodesCacheHelpers::GetGuid(*stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesCache::_GetNode(BeGuidCR id) const
    {
    if (!id.IsValid())
        return nullptr;

    NavNodePtr node = GetQuick(id);
    if (node.IsValid())
        return node;

    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "SELECT " NODE_SELECT_STMT("hl", "n", "nk")
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [n].[HierarchyLevelId] "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
        " WHERE [n].[Id] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node selection query");

    NodesCacheHelpers::BindGuid(*stmt, 1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    node = NodesCacheHelpers::CreateNodeFromStatement(*stmt, m_nodesFactory, m_dbFactory->GetConnection());

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, stmt->Step() == BE_SQLITE_DONE, "Query returned more than 1 row for node request");

    AddQuick(*node);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodeVisibility NodesCache::_GetNodeVisibility(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "SELECT [dsn].[Visibility]"
        "  FROM [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [dsn].[DataSourceId] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
        " WHERE [dsn].[NodeId] = ?"
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
        "       AND [ds].[InstanceFilter] IS ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node visibility selection query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, nodeId);
    stmt->BindText(bindingIndex++, contextVariables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, instanceFilter);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Unexpected step result: %d", (int)result));

    auto visibility = (NodeVisibility)stmt->GetValueInt(0);
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, stmt->Step() == BE_SQLITE_DONE, "Query returned more than 1 row for node visibility request");
    return visibility;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint64_t> NodesCache::_GetNodeIndex(BeGuidCR hierarchyLevelId, BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        WITH_PHYSICAL_HIERARCHY_LEVELS(
            PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[Id] = ? "
            "AND " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[RemovalId] IS NULL "
        )
        "SELECT " NODESCACHE_FUNCNAME_ConcatBinaryIndex "([phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceIndex "], [dsn].[NodeIndex]) "
        "  FROM [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_VariablesId "]"
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [dsn].[DataSourceId] "
        " WHERE [dsn].[NodeId] = ? "
        "       AND [ds].[InstanceFilter] IS ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node index selection query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, hierarchyLevelId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, nodeId);
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, instanceFilter);
    stmt->BindText(bindingIndex++, contextVariables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Unexpected step result: %d", (int)result));

    auto index = NodesCacheHelpers::GetVectorIndex(*stmt, 0, true);
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, stmt->Step() == BE_SQLITE_DONE, "Query returned more than 1 row for node index request");
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const& info, bool onlyInitialized) const
    {
    // make sure it's fully initialized
    if (onlyInitialized && !IsCombinedHierarchyLevelInitialized(info, context.GetRulesetVariables(), context.GetInstanceFilter()))
        return nullptr;

    BeGuid physicalHierarchyLevelId = FindPhysicalHierarchyLevelId(info);
    if (onlyInitialized && !physicalHierarchyLevelId.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Failed to find physical hierarchy level id for parent node: '%s'", info.GetPhysicalParentNodeId().ToString().c_str()));

    return CachedCombinedHierarchyLevelProvider::Create(context, m_db, m_statements, m_mutex, physicalHierarchyLevelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetHierarchyLevel(NavNodesProviderContextR context, BeGuidCR id, bool onlyInitialized) const
    {
    // make sure it's fully initialized
    if (onlyInitialized && !IsHierarchyLevelInitialized(id, context.GetRulesetVariables(), context.GetInstanceFilter()))
        return nullptr;

    return CachedHierarchyLevelProvider::Create(context, m_db, m_statements, m_mutex, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<DirectNodesIterator> NodesCache::_GetCachedDirectNodesIterator(NavNodesProviderContextCR context, DataSourceIdentifier const& id) const
    {
    return SqliteCacheDirectNodeIteratorBase::CreateForDataSource(context, m_db, m_statements, m_mutex, id.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BeGuid QueryExistingNode(Db& db, StatementCache& statements, BeGuidCR hierarchyLevelId, NavNodeKeyCR nodeKey)
    {
    CachedStatementPtr stmt;

    auto instanceNodeKey = nodeKey.AsECInstanceNodeKey();
    if (instanceNodeKey && instanceNodeKey->GetInstanceKeys().size() == 1)
        {
        // special handling for the most common case of instance node associated with one instance
        static Utf8CP instanceNodeLookupQuery =
            "SELECT [n].[Id] "
            "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
            "  JOIN [" NODESCACHE_TABLENAME_NodeInstances "] ni ON [ni].[NodeId] = [n].[Id] "
            "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
            " WHERE     [ni].[ECClassId] = ? "
            "       AND [ni].[ECInstanceId] = ? "
            "       AND [n].[HierarchyLevelId] = ? "
            "       AND [nk].[SpecificationIdentifier] = ? "
            "       AND 1 == (SELECT COUNT(1) FROM [" NODESCACHE_TABLENAME_NodeInstances "] nic WHERE [nic].[NodeId] = [n].[Id])";

        if (BE_SQLITE_OK != statements.GetPreparedStatement(stmt, *db.GetDbFile(), instanceNodeLookupQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare existing node query (instance)");

        int bindingIndex = 1;
        stmt->BindId(bindingIndex++, instanceNodeKey->GetInstanceKeys().back().GetClass()->GetId());
        stmt->BindId(bindingIndex++, instanceNodeKey->GetInstanceKeys().back().GetId());
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, hierarchyLevelId);
        stmt->BindText(bindingIndex++, instanceNodeKey->GetSpecificationIdentifier(), Statement::MakeCopy::No);
        }
    else
        {
        static Utf8CP genericLookupQuery =
            "SELECT [n].[Id] "
            "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
            "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
            " WHERE     [nk].[Type] = ? "
            "       AND [nk].[PathFromRoot] = ? "
            "       AND [n].[HierarchyLevelId] = ? ";

        if (BE_SQLITE_OK != statements.GetPreparedStatement(stmt, *db.GetDbFile(), genericLookupQuery))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare existing node query (generic)");

        int bindingIndex = 1;
        stmt->BindText(bindingIndex++, nodeKey.GetType(), Statement::MakeCopy::No);
        stmt->BindText(bindingIndex++, NavNodesHelper::NodeKeyHashPathToString(nodeKey), Statement::MakeCopy::Yes);
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, hierarchyLevelId);
        }

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW == result)
        {
        BeGuid result = NodesCacheHelpers::GetGuid(*stmt, 0);
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, stmt->Step() == BE_SQLITE_DONE, "Found more than one matching node in cached hierarchy level. Returning the first one.");
        return result;
        }

    return BeGuid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Cache(NavNodeR node, DataSourceIdentifier const& datasourceIdentifier, bvector<uint64_t> const& nodeIndex, NodeVisibility visibility)
    {
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, !node.GetNodeId().IsValid(), "Caching node that already has an ID");

    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    if (auto existingNodeId = QueryExistingNode(m_db, m_statements, datasourceIdentifier.GetHierarchyLevelId(), *node.GetKey()); existingNodeId.IsValid())
        {
        node.SetNodeId(existingNodeId);
        }
    else
        {
        CacheNode(datasourceIdentifier.GetHierarchyLevelId(), node);
        }

    CacheDataSourceNodeAssociation(datasourceIdentifier.GetId(), node.GetNodeId(), nodeIndex, visibility);

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Cache(HierarchyLevelIdentifier& identifier)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    identifier.SetId(CacheHierarchyLevel(identifier.GetVirtualParentNodeId(), identifier.GetRulesetId(), identifier.GetRemovalId()));

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Cache(DataSourceInfo& info)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    BeGuid rulesetId = GetHierarchyLevelRulesetId(info.GetIdentifier().GetHierarchyLevelId());
    if (!rulesetId.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to find hierarchy level ruleset id");

    BeGuid variablesId = CacheRelatedVariables(rulesetId, info.GetRelatedVariables());
    CacheEmptyDataSource(info.GetIdentifier(), info.GetFilter(), variablesId, info.GetSpecificationHash(), info.GetNodeTypes(),
        info.GetParentId(), info.GetDirectNodesCount(), info.IsInitialized(), info.HasNodes(), info.GetTotalNodesCount());

    if (info.GetIdentifier().IsValid())
        CacheRelatedClassIds(info.GetIdentifier().GetId(), info.GetRelatedClasses());

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Update(DataSourceInfo const& info, int partsToUpdate)
    {
    if (0 == partsToUpdate)
        {
        // early return if nothing to update
        return;
        }

    SavePointWithMutexHolder savepoint(m_mutex, m_ensureThreadSafety, m_db, "UpdateDataSource", BeSQLiteTxnMode::Immediate);

    if (0 != ((DataSourceInfo::PARTS_All & ~DataSourceInfo::PART_RelatedClasses) & partsToUpdate))
        {
        bvector<Utf8String> assignments;
        if (0 != (DataSourceInfo::PART_ParentId & partsToUpdate))
            assignments.push_back("[ParentId] = ?");
        if (0 != (DataSourceInfo::PART_Filter & partsToUpdate))
            assignments.push_back("[Filter] = ?");
        if (0 != (DataSourceInfo::PART_Vars & partsToUpdate))
            assignments.push_back("[VariablesId] = ?");
        if (0 != (DataSourceInfo::PART_SpecificationHash & partsToUpdate))
            assignments.push_back("[SpecificationHash] = ?");
        if (0 != (DataSourceInfo::PART_NodeTypes & partsToUpdate))
            assignments.push_back("[NodeTypes] = ?");
        if (0 != (DataSourceInfo::PART_TotalNodesCount & partsToUpdate))
            assignments.push_back("[TotalNodesCount] = ?");
        if (0 != (DataSourceInfo::PART_HasNodes & partsToUpdate))
            assignments.push_back("[HasNodes] = ?");
        if (0 != (DataSourceInfo::PART_DirectNodesCount & partsToUpdate))
            assignments.push_back("[DirectNodesCount] = ?");
        if (0 != (DataSourceInfo::PART_IsFinalized & partsToUpdate))
            assignments.push_back("[IsInitialized] = ?");
        if (0 != (DataSourceInfo::PART_CustomJson & partsToUpdate))
            assignments.push_back("[CustomJson] = ?");

        Utf8String query("UPDATE [" NODESCACHE_TABLENAME_DataSources "] ");
        query.append("SET ").append(BeStringUtilities::Join(assignments, ", ")).append(" ");
        query.append("WHERE [Id] = ?");

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare data source update query");

        int bindingIndex = 0;
        if (0 != (DataSourceInfo::PART_ParentId & partsToUpdate))
            NodesCacheHelpers::BindGuid(*stmt, ++bindingIndex, info.GetParentId());
        if (0 != (DataSourceInfo::PART_Filter & partsToUpdate))
            stmt->BindText(++bindingIndex, GetSerializedJson(info.GetFilter().AsJson()), Statement::MakeCopy::Yes);
        if (0 != (DataSourceInfo::PART_Vars & partsToUpdate))
            {
            BeGuid rulesetId = GetHierarchyLevelRulesetId(info.GetIdentifier().GetHierarchyLevelId());
            BeGuid variablesId = CacheRelatedVariables(rulesetId, info.GetRelatedVariables());
            NodesCacheHelpers::BindGuid(*stmt, ++bindingIndex, variablesId);
            }
        if (0 != (DataSourceInfo::PART_SpecificationHash & partsToUpdate))
            stmt->BindText(++bindingIndex, info.GetSpecificationHash(), Statement::MakeCopy::No);
        if (0 != (DataSourceInfo::PART_NodeTypes & partsToUpdate))
            stmt->BindText(++bindingIndex, info.GetNodeTypes(), Statement::MakeCopy::No);
        if (0 != (DataSourceInfo::PART_TotalNodesCount & partsToUpdate))
            {
            if (info.GetTotalNodesCount().IsValid())
                stmt->BindUInt64(++bindingIndex, info.GetTotalNodesCount().Value());
            else
                stmt->BindNull(++bindingIndex);
            }
        if (0 != (DataSourceInfo::PART_HasNodes & partsToUpdate))
            {
            if (info.HasNodes().IsValid())
                stmt->BindBoolean(++bindingIndex, info.HasNodes().Value());
            else
                stmt->BindNull(++bindingIndex);
            }
        if (0 != (DataSourceInfo::PART_DirectNodesCount & partsToUpdate))
            {
            if (info.GetDirectNodesCount().IsValid())
                stmt->BindUInt64(++bindingIndex, info.GetDirectNodesCount().Value());
            else
                stmt->BindNull(++bindingIndex);
            }
        if (0 != (DataSourceInfo::PART_IsFinalized & partsToUpdate))
            stmt->BindBoolean(++bindingIndex, info.IsInitialized());
        if (0 != (DataSourceInfo::PART_CustomJson & partsToUpdate))
            stmt->BindText(++bindingIndex, info.GetCustomJson().ToString(), Statement::MakeCopy::Yes);
        NodesCacheHelpers::BindGuid(*stmt, ++bindingIndex, info.GetIdentifier().GetId());

        stmt->Step();
        }

    if (0 != (DataSourceInfo::PART_RelatedClasses & partsToUpdate))
        CacheRelatedClassIds(info.GetIdentifier().GetId(), info.GetRelatedClasses());

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::SetRemovalId(CombinedHierarchyLevelIdentifier const& info)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        WITH_FLAT_CHILD_HIERARCHYLEVELS
        "UPDATE [" NODESCACHE_TABLENAME_HierarchyLevels "] "
        "   SET [RemovalId] = ? "
        " WHERE [Id] IN ("
        "           SELECT [chl].[" FLAT_HIERARCHYLEVELS_COLUMN_NAME_LevelId "]"
        "             FROM [" FLAT_HIERARCHYLEVELS_TABLE_NAME "] chl"
        "       ) ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare hierarchy level update query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, info.GetPhysicalParentNodeId()); // for WITH_FLAT_CHILD_HIERARCHYLEVELS
    stmt->BindText(bindingIndex++, info.GetRulesetId(), Statement::MakeCopy::No); // for WITH_FLAT_CHILD_HIERARCHYLEVELS
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, BeGuid()); // for WITH_FLAT_CHILD_HIERARCHYLEVELS
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, info.GetRemovalId());

    DbResult result = stmt->Step();
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, BE_SQLITE_DONE == result, Utf8PrintfString("Unexpected update step result: %d", (int)result));

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif

    BeMutexHolder lockQuick(m_quickCacheMutex);
    m_quickNodesCache.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveHierarchyLevels(BeGuidCR removalId)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "DELETE "
        "FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] "
        "WHERE [RemovalId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare hierarchy level deletion query");

    NodesCacheHelpers::BindGuid(*stmt, 1, removalId);
    stmt->Step();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::ResetDataSource(DataSourceIdentifier const& info)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "DELETE FROM [" NODESCACHE_TABLENAME_DataSources "] "
        "WHERE [Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare data source deletion query");

    NodesCacheHelpers::BindGuid(*stmt, 1, info.GetId());
    stmt->Step();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::GetNodeDataSourceId(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const
    {
    static Utf8CP query =
        "SELECT [ds].[Id] "
        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        " WHERE [dsn].[NodeId] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
        "       AND [ds].[InstanceFilter] IS ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node's data source selection query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, nodeId);
    stmt->BindText(bindingIndex++, contextVariables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, instanceFilter);

    if (BE_SQLITE_ROW != stmt->Step())
        return BeGuid();

    auto id = NodesCacheHelpers::GetGuid(*stmt, 0);
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, stmt->Step() == BE_SQLITE_DONE, "Query returned more than 1 row for node's data source request");
    return id;
    }

/*---------------------------------------------------------------------------------**//**
* note: must be called within a mutex
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::ChangeVisibility(BeGuidCR nodeId, BeGuidCR dataSourceId, NodeVisibility visibility)
    {
    // first, make sure the node is not virtual
    static Utf8CP query =
        "UPDATE [" NODESCACHE_TABLENAME_DataSourceNodes "] "
        "SET [Visibility] = ? "
        "WHERE [NodeId] = ? AND [DataSourceId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node visibility change query");

    int bindingIndex = 1;
    stmt->BindInt(bindingIndex++, (int)visibility);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, nodeId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, dataSourceId);

    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_MakeVirtual(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    BeGuid dataSourceId = GetNodeDataSourceId(nodeId, contextVariables, instanceFilter);
    ChangeVisibility(nodeId, dataSourceId, NodeVisibility::Virtual);

    RemoveQuick([&](NavNodeCR n)
        {
        return n.GetNodeId() == nodeId;
        });

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_MakeHidden(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);
    BeGuid dataSourceId = GetNodeDataSourceId(nodeId, contextVariables, instanceFilter);
    ChangeVisibility(nodeId, dataSourceId, NodeVisibility::Hidden);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::_IsCombinedHierarchyLevelInitialized(CombinedHierarchyLevelIdentifier const& info, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        WITH_PHYSICAL_HIERARCHY_LEVELS(
            PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[ParentNodeId] IS ? "
            "AND " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[RemovalId] IS ? "
        )
        " SELECT MIN([ds].[IsInitialized])"
        " FROM [" NODESCACHE_TABLENAME_Rulesets "] r "
        " CROSS JOIN [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl ON [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_RulesetId "] = [r].[Id] "
        " LEFT JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[RulesetId] = [r].[Id] AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
        " LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] AND [ds].[VariablesId] = [dsv].[Id] AND [ds].[InstanceFilter] IS ? "
        " WHERE     [r].[Identifier] = ? "
        " GROUP BY [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_HierarchyLevelId "], [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceIndex "]";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare IsNodeInitialized query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, info.GetPhysicalParentNodeId());
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, info.GetRemovalId());
    stmt->BindText(bindingIndex++, contextVariables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, instanceFilter);
    stmt->BindText(bindingIndex++, info.GetRulesetId().c_str(), Statement::MakeCopy::No);

    DbResult res = stmt->Step();
    if (BE_SQLITE_ROW != res)
        {
        // no matching data sources means the hierarchy level is not ready
        return false;
        }

    while (BE_SQLITE_ROW == res)
        {
        if (stmt->IsColumnNull(0) || !stmt->GetValueBoolean(0))
            return false;

        res = stmt->Step();
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::_IsHierarchyLevelInitialized(BeGuidCR id, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const
    {
    if (!id.IsValid())
        return false;

    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "SELECT [ds].[IsInitialized] "
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
        " WHERE [hl].[Id] = ? "
        "       AND [ds].[InstanceFilter] IS ? "
        "GROUP BY [ds].[IsInitialized]";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare IsHierarchyLevelInitialized query");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, contextVariables.GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, id);
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, instanceFilter);

    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    bool value = stmt->GetValueBoolean(0);
    return value && (BE_SQLITE_ROW != stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::_IsDataSourceInitialized(BeGuidCR id) const
    {
    if (!id.IsValid())
        return false;

    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "SELECT 1 "
        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
        " WHERE     [ds].[IsInitialized] "
        "       AND [ds].[Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare IsDataSourceInitialized query");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, id);

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::Clear(Utf8CP rulesetId)
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    Utf8String query = "DELETE FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] ";
    if (nullptr != rulesetId)
        query.append("WHERE [RulesetId] IN (SELECT [Id] FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [Identifier] = ?) ");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare hierarchy levels deletion query");

    if (nullptr != rulesetId)
        stmt->BindText(1, rulesetId, Statement::MakeCopy::No);

    stmt->Step();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif

    BeMutexHolder lockQuick(m_quickCacheMutex);
    m_quickNodesCache.clear();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstanceKeySet : VirtualSet
    {
    private:
        bset<ECInstanceKey> const& m_keys;
    public:
        ECInstanceKeySet(bset<ECInstanceKey> const& keys) : m_keys(keys) {}
        bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            if (nVals < 2)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Invalid number of arguments. Expected 2, got: %d", nVals));
            if (nVals > 2)
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_WARNING, Utf8PrintfString("Invalid number of arguments. Expected 2, got: %d", nVals));
            ECClassId classId = vals[0].GetValueId<ECClassId>();
            ECInstanceId instanceId = vals[1].GetValueId<ECInstanceId>();
            return (m_keys.end() != m_keys.find(ECInstanceKey(classId, instanceId)));
            }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct VirtualECInstanceIdSet : VirtualSet
    {
    private:
        bset<ECInstanceId> m_ids;
    public:
        VirtualECInstanceIdSet(bset<ECInstanceKey> const& keys)
            {
            for (ECInstanceKeyCR key : keys)
                m_ids.insert(key.GetInstanceId());
            }
        VirtualECInstanceIdSet(bvector<ECInstanceKey> const& keys)
            {
            for (ECInstanceKeyCR key : keys)
                m_ids.insert(key.GetInstanceId());
            }
        bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            if (nVals < 1)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Invalid number of arguments. Expected 1, got: %d", nVals));
            if (nVals > 1)
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_WARNING, Utf8PrintfString("Invalid number of arguments. Expected 1, got: %d", nVals));
            ECInstanceId instanceId = vals[0].GetValueId<ECInstanceId>();
            return (m_ids.end() != m_ids.find(instanceId));
            }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassIdSet : VirtualSet
    {
    private:
        bset<ECClassId> m_ids;
    public:
        ECClassIdSet(bset<ECInstanceKey> const& keys)
            {
            for (ECInstanceKeyCR key : keys)
                m_ids.insert(key.GetClassId());
            }
        ECClassIdSet(bset<ECClassId> ids) : m_ids(ids) {}
        bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            if (nVals < 1)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Invalid number of arguments. Expected 1, got: %d", nVals));
            if (nVals > 1)
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_WARNING, Utf8PrintfString("Invalid number of arguments. Expected 1, got: %d", nVals));
            ECClassId classId = vals[0].GetValueId<ECClassId>();
            return (m_ids.end() != m_ids.find(classId));
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AreRelated(IConnectionCR connection, DataSourceFilter::RelatedInstanceInfo const& relationshipInfo, bset<ECInstanceKey> const& keys)
    {
    for (ECInstanceKeyCR key : keys)
        {
        if (relationshipInfo.m_instanceKeys.end() != std::find(relationshipInfo.m_instanceKeys.begin(), relationshipInfo.m_instanceKeys.end(), key))
            return true;
        }

    Utf8String query;
    for (ECClassId relationshipClassId : relationshipInfo.m_relationshipClassIds)
        {
        ECRelationshipClassCP relationshipClass = connection.GetECDb().Schemas().GetClass(relationshipClassId)->GetRelationshipClassCP();
        if (nullptr == relationshipClass)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Invalid class ID or the class is not a relationship class: %" PRIu64, relationshipClassId.GetValue()));

        if (!query.empty())
            query.append(" UNION ALL ");

        query.append("SELECT 1 FROM ");
        query.append(relationshipClass->GetECSqlName());
        query.append(" WHERE ");

        switch (relationshipInfo.m_direction)
            {
            case RequiredRelationDirection_Forward:
                query.append("InVirtualSet(?, SourceECInstanceId) AND InVirtualSet(?, TargetECInstanceId)");
                break;
            case RequiredRelationDirection_Backward:
                query.append("InVirtualSet(?, TargetECInstanceId) AND InVirtualSet(?, SourceECInstanceId)");
                break;
            case RequiredRelationDirection_Both:
                query.append("InVirtualSet(?, SourceECInstanceId) AND InVirtualSet(?, TargetECInstanceId)");
                query.append(" OR ");
                query.append("InVirtualSet(?, TargetECInstanceId) AND InVirtualSet(?, SourceECInstanceId)");
                break;
            }
        }

    Savepoint txn(connection.GetDb(), "NodesCache/AreRelated");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, txn.IsActive(), "Failed to start a transaction");

    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), query.c_str());
    if (!stmt.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare 'are keys related to given RelatedInstanceInfo' query");

    std::shared_ptr<VirtualECInstanceIdSet> relationshipInfoIdsSet = std::make_shared<VirtualECInstanceIdSet>(relationshipInfo.m_instanceKeys);
    std::shared_ptr<VirtualECInstanceIdSet> inputIdsSet = std::make_shared<VirtualECInstanceIdSet>(keys);
    int bindingIndex = 1;

    for (auto i = 0; i < relationshipInfo.m_relationshipClassIds.size(); ++i)
        {
        stmt->BindVirtualSet(bindingIndex++, relationshipInfoIdsSet);
        stmt->BindVirtualSet(bindingIndex++, inputIdsSet);

        if (RequiredRelationDirection_Both == relationshipInfo.m_direction)
            {
            stmt->BindVirtualSet(bindingIndex++, relationshipInfoIdsSet);
            stmt->BindVirtualSet(bindingIndex++, inputIdsSet);
            }
        }

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AnyKeyMatchesFilter(IConnectionCR connection, Utf8CP serializedFilter, bset<ECInstanceKey> const& keys)
    {
    rapidjson::Document json;
    json.Parse(serializedFilter);
    if (json.IsNull())
        return true; // no filter means a match

    DataSourceFilter filter(json);
    if (nullptr != filter.GetRelatedInstanceInfo())
        return AreRelated(connection, *filter.GetRelatedInstanceInfo(), keys);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddBaseClasses(bset<ECClassId>& ids, ECClassCR ecClass)
    {
    for (ECClassCP base : ecClass.GetBaseClasses())
        {
        ids.insert(base->GetId());
        AddBaseClasses(ids, *base);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddDerivedClasses(bset<ECClassId>& ids, ECClassCR ecClass)
    {
    for (ECClassCP derived : ecClass.GetDerivedClasses())
        {
        ids.insert(derived->GetId());
        AddDerivedClasses(ids, *derived);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddBaseAndDerivedClasses(bset<ECClassId>& ids, ECDbCR db, bset<ECInstanceKey> const& keys)
    {
    for (ECInstanceKey const& key : keys)
        {
        ECClassCP ecClass = db.Schemas().GetClass(key.GetClassId());
        if (nullptr == ecClass)
            continue;
        ids.insert(ecClass->GetId());
        AddBaseClasses(ids, *ecClass);
        AddDerivedClasses(ids, *ecClass);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bset<CombinedHierarchyLevelIdentifier> NodesCache::GetRelatedHierarchyLevels(IConnectionCR connection, bset<ECInstanceKey> const& keys) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);
    bset<CombinedHierarchyLevelIdentifier> infos;

    static Utf8CP query =
        WITH_PHYSICAL_HIERARCHY_LEVELS(
            PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[ParentNodeId] IS NULL "
            "OR " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[ParentNodeId] IN ("
            "    SELECT [dsn].[NodeId]"
            "      FROM [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn"
            "     WHERE [dsn].[Visibility] = 0"
            ")"
        )
        "SELECT [r].[Identifier], [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_PhysicalParentNodeId "], [ds].[Filter], 1 AS Priority "
        "  FROM [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_RulesetId "] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] "
        " WHERE [ds].[Id] IN ("
        "           SELECT [dsc].[DataSourceId]"
        "             FROM [" NODESCACHE_TABLENAME_DataSourceClasses "] dsc"
        "            WHERE [dsc].[Polymorphic] AND InVirtualSet(?, [dsc].[ECClassId])"
        "                  OR NOT [dsc].[Polymorphic] AND InVirtualSet(?, [dsc].[ECClassId])"
        "       )"
        "UNION ALL "
        "SELECT [r].[Identifier], [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_PhysicalParentNodeId "], [ds].[Filter], 2 AS Priority "
        "  FROM [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_RulesetId "] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] "
        " WHERE [ds].[Id] IN ("
        "           SELECT [dsn].[DataSourceId] "
        "             FROM [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn "
        "             JOIN [" NODESCACHE_TABLENAME_NodeInstances "] ni ON [ni].[NodeId] = [dsn].[NodeId] "
        "            WHERE InVirtualSet(?, [ni].[ECClassId], [ni].[ECInstanceId]) "
        "       )";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare related hierarchy levels query");

    // bind classes for polymorphic search
    bset<ECClassId> ids;
    AddBaseAndDerivedClasses(ids, connection.GetECDb(), keys);
    ECClassIdSet polymorphicIds(ids);
    stmt->BindVirtualSet(1, polymorphicIds);

    // bind classes for nonpolymorphic search
    ECClassIdSet nonPolymorphicIds(keys);
    stmt->BindVirtualSet(2, nonPolymorphicIds);

    ECInstanceKeySet instanceKeysVirtualSet(keys);
    stmt->BindVirtualSet(3, instanceKeysVirtualSet);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8CP filter = stmt->GetValueText(2);
        int priority = stmt->GetValueInt(3);
        if (priority < 2 && nullptr != filter && 0 != *filter && !AnyKeyMatchesFilter(connection, filter, keys))
            continue;

        BeGuid physicalParentNodeId = NodesCacheHelpers::GetGuid(*stmt, 1);
        Utf8CP rulesetId = stmt->GetValueText(0);
        infos.insert(CombinedHierarchyLevelIdentifier(m_dbFactory->GetConnection().GetId().c_str(), rulesetId, physicalParentNodeId));
        }

    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr NodesCache::_LocateNode(IConnectionCR connection, Utf8StringCR rulesetId, NavNodeKeyCR nodeKey) const
    {
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);

    static Utf8CP query =
        "SELECT " NODE_SELECT_STMT("hl", "n", "k")
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id]"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [n].[HierarchyLevelId] "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hl].[RulesetId] "
        " WHERE     [k].[PathFromRoot] = ?"
        "       AND [k].[Type] = ?"
        "       AND [r].[Identifier] = ?"
        "       AND [hl].[RemovalId] IS NULL";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare node locate query");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, NavNodesHelper::NodeKeyHashPathToString(nodeKey), Statement::MakeCopy::Yes);
    stmt->BindText(bindingIndex++, nodeKey.GetType(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, rulesetId, Statement::MakeCopy::No);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return nullptr;

    NavNodeCPtr node = NodesCacheHelpers::CreateNodeFromStatement(*stmt, m_nodesFactory, m_dbFactory->GetConnection());

    result = stmt->Step();
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, (BE_SQLITE_DONE == result), Utf8PrintfString("Located more than one node? Result: %d", (int)result));

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::AddQuick(NavNodeR node) const
    {
    BeMutexHolder lockQuick(m_quickCacheMutex);
    RemoveQuick(node.GetNodeId());
    m_quickNodesCache.push_back(bpair<BeGuid, NavNodePtr>(node.GetNodeId(), &node));
    if (m_quickNodesCache.size() > NODESCACHE_QUICK_Size)
        m_quickNodesCache.erase(m_quickNodesCache.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveQuick(BeGuidCR id) const
    {
    BeMutexHolder lockQuick(m_quickCacheMutex);
    for (auto iter = m_quickNodesCache.begin(); iter != m_quickNodesCache.end(); ++iter)
        {
        if (id == iter->first)
            {
            m_quickNodesCache.erase(iter);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveQuick(std::function<bool(NavNodeCR)> const& pred) const
    {
    BeMutexHolder lockQuick(m_quickCacheMutex);
    bvector<bpair<BeGuid, NavNodePtr>> processed;
    for (auto iter = m_quickNodesCache.begin(); iter != m_quickNodesCache.end(); ++iter)
        {
        if (!pred(*iter->second))
            processed.push_back(*iter);
        }
    m_quickNodesCache.swap(processed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesCache::GetQuick(BeGuidCR id) const
    {
    BeMutexHolder lockQuick(m_quickCacheMutex);
    for (size_t i = 0; i < m_quickNodesCache.size(); ++i)
        {
        BeGuidCR cachedId = m_quickNodesCache[i].first;
        if (id == cachedId)
            {
            NavNodePtr node = m_quickNodesCache[i].second;
            m_quickNodesCache.erase(m_quickNodesCache.begin() + i);
            m_quickNodesCache.push_back(bpair<BeGuid, NavNodePtr>(id, node));
            return node;
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::Persist()
    {
    if (!m_db.GetDefaultTransaction()->IsActive())
        return;
    LOCK_MUTEX_ON_CONDITION(m_mutex, m_ensureThreadSafety);
    m_db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::GetUndeterminedNodesProvider(NavNodesProviderContextR context) const
    {
    auto provider = NodesWithUndeterminedChildrenProvider::Create(context, m_db, m_statements, m_mutex);
    DisabledPostProcessingContext doNotPostProcess(*provider);
    return provider->PostProcess(m_providersFactory.GetPostProcessors());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::GetFilteredNodesProvider(NavNodesProviderContextR context, Utf8CP filter) const
    {
    auto provider = FilteredNodesProvider::Create(context, m_db, m_statements, m_mutex, filter);
    DisabledPostProcessingContext doNotPostProcess(*provider);
    return provider->PostProcess(m_providersFactory.GetPostProcessors());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<IHierarchyLevelLocker> INavNodesCache::CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const& identifier, bool lock)
    {
    std::shared_ptr<IHierarchyLevelLocker> locker = _CreateHierarchyLevelLocker(identifier);
    if (lock || !FindPhysicalHierarchyLevelId(identifier).IsValid())
        {
        // if physical hierarchy level does not exists when locker is created, lock it immediatly
        locker->Lock();
        }
    else
        {
        // if physical hierarchy level already exists check if it's not locked to avoid doing duplicate work
        locker->WaitForUnlock();
        }

    return locker;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCacheHierarchyLevelLocker::LockResult NodesCacheHierarchyLevelLocker::TryLockHierarchyLevel(int options)
    {
    // start transaction in order to prevent other connections writing to cache while trying to aquire hierarchy lock
    IHierarchyCache::SavepointPtr savepoint = m_cache.CreateSavepoint();

    if (m_cache.IsHierarchyLevelLocked(m_hierarchyLevelIdentifier))
        return LockResult::LockNotAquired;

    if (0 != (LockOptions::CheckLockedChildLevels & options) && m_cache.IsAnyChildHierarchyLevelLocked(m_hierarchyLevelIdentifier))
        return LockResult::LockNotAquired;

    m_hierarchyLevelId = m_cache.CacheOrGetPhysicalHierarchyLevel(m_hierarchyLevelIdentifier);
    if (!m_hierarchyLevelId.IsValid())
        return LockResult::LockFailed;

    m_lockTimestamp = m_cache.UpdateHierarchyLevelLock(m_hierarchyLevelId);
    if (m_lockTimestamp.Value() == 0)
        return LockResult::LockFailed;

    return LockResult::LockAquired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheHierarchyLevelLocker::_Lock(int options)
    {
    if (m_lockTimestamp.IsValid())
        {
        if (BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_lockTimestamp.Value() < NAVNODES_CACHE_LockTimeout / 2)
            return true;

        IHierarchyCache::SavepointPtr savepoint = m_cache.CreateSavepoint();
        m_lockTimestamp = m_cache.UpdateHierarchyLevelLock(m_hierarchyLevelId);
        return true;
        }

    LockResult result = LockResult::LockNotAquired;
    while ((result = TryLockHierarchyLevel(options)) == LockResult::LockNotAquired)
        {
        if (result == LockResult::LockFailed)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to lock hierarchy level.");

        if (0 != (LockOptions::DisableLockWait & options))
            return false;

        BeThreadUtilities::BeSleep(NAVNODES_CACHE_LockWaitTime);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheHierarchyLevelLocker::_Unlock()
    {
    if (m_lockTimestamp.IsNull())
        return;

    m_cache.RemoveHierarchyLevelLock(m_hierarchyLevelId);
    m_lockTimestamp = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheHierarchyLevelLocker::_WaitForUnlock()
    {
    if (m_lockTimestamp.IsValid())
        return;

    while (true)
        {
        if (!m_cache.IsHierarchyLevelLocked(m_hierarchyLevelIdentifier))
            return;

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesCache, LOG_TRACE,
            Utf8PrintfString("Waiting for hierarchy to be unlocked on thread - %d", (int)BeThreadUtilities::GetCurrentThreadId()));
        BeThreadUtilities::BeSleep(NAVNODES_CACHE_LockWaitTime);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t SqliteCacheDirectNodeIteratorBase::GetTotalNodesCount() const
    {
    if (m_nodesCount.IsNull())
        m_nodesCount = _QueryTotalNodesCount();
    return m_nodesCount.Value();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr SqliteCacheDirectNodeIteratorBase::GetNodesPageStatement() const
    {
    Utf8String query = _GetNodesQuery();
    if (GetPageSize() != 0 || GetOffset() != 0)
        query.append("LIMIT ? OFFSET ? ");

#ifdef CHECK_ORDERED_QUERY_PLANS
    CheckQueryPlan(GetDb(), query.c_str(), GetStatements());
#endif

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetDb().GetDbFile(), query.c_str()))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to create next nodes page statement");

    int bindingIndex = 1;
    _BindNodesStatement(*stmt, bindingIndex);
    if (GetPageSize() != 0 || GetOffset() != 0)
        {
        stmt->BindInt(bindingIndex++, GetPageSize() == 0 ? -1 : GetPageSize());
        stmt->BindInt(bindingIndex++, GetOffset());
        }
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SqliteCacheDirectNodeIteratorBase::LoadNodesPage()
    {
    BeMutexHolder lock(m_cacheMutex);
    CachedStatementPtr stmt = GetNodesPageStatement();
    if (stmt.IsNull())
        return;

    m_loadedNodes = std::make_unique<bvector<NavNodePtr>>();
    DbResult stepResult;
    while (BE_SQLITE_ROW == (stepResult = stmt->Step()))
        {
        NavNodePtr node = NodesCacheHelpers::CreateNodeFromStatement(*stmt, m_context->GetNodesFactory(), m_context->GetConnection());
        if (node.IsNull())
            continue;

        m_loadedNodes->push_back(node);
        }
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, BE_SQLITE_DONE == stepResult, Utf8PrintfString("Nodes page load ended with unexpected db result code: %d", (int)stepResult));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SqliteCacheDirectNodeIteratorBase::_SkippedNodesToPageStart() const
    {
    if (HasVirtualNodes())
        return false;

    return m_context->HasPageOptions() && m_context->GetPageOptions()->GetStart() != 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t SqliteCacheDirectNodeIteratorBase::_NodesCount() const
    {
    if (HasVirtualNodes() || !m_context->HasPageOptions())
        return GetTotalNodesCount();

    return m_context->GetPageOptions()->GetAdjustedPageSize(GetTotalNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr SqliteCacheDirectNodeIteratorBase::_NextNode()
    {
    /*
    * if we're out of loaded nodes, we need to load some more if there's a defined page size and our offset
    * is less than total nodes count
    */
    if (!m_loadedNodes || m_currNodeIndex >= m_loadedNodes->size() && GetPageSize() && GetOffset() < GetTotalNodesCount())
        {
        LoadNodesPage();
        m_currNodeIndex = 0;
        m_offset.ValueR() += GetPageSize();
        }
    return (m_loadedNodes && m_currNodeIndex < m_loadedNodes->size()) ? m_loadedNodes->at(m_currNodeIndex++) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int SqliteCacheDirectNodeIteratorBase::GetOffset() const
    {
    if (m_offset.IsNull())
        {
        BeMutexHolder lock(m_cacheMutex);
        m_offset = !HasVirtualNodes() && m_context->HasPageOptions() ? m_context->GetPageOptions()->GetStart() : 0;
        }
    return m_offset.Value();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SqliteCacheDirectNodeIteratorBase::HasVirtualNodes() const
    {
    if (m_hasVirtualNodes.IsNull())
        {
        BeMutexHolder lock(m_cacheMutex);
        m_hasVirtualNodes = _QueryHasVirtualNodes();
        }
    return m_hasVirtualNodes.Value();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CachedHierarchyLevelDirectNodeIterator : SqliteCacheDirectNodeIteratorBase
{
private:
    BeGuid m_hierarchyLevelId;

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _QueryHasVirtualNodes() const override
        {
        static Utf8CP query =
            "SELECT 1 "
            "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
            "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
            "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
            "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
            "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId]"
            " WHERE [hl].[Id] = ? "
            "       AND [dsn].[Visibility] = ? "
            "       AND [ds].[InstanceFilter] IS ? "
            "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetDb().GetDbFile(), query))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare virtual nodes statement");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_hierarchyLevelId);
        stmt->BindInt(bindingIndex++, (int)NodeVisibility::Virtual);
        NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, GetContext().GetInstanceFilter());
        stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
        return BE_SQLITE_ROW == stmt->Step();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t _QueryTotalNodesCount() const override
        {
        static Utf8CP query =
            "SELECT COUNT(1) "
            "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
            "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
            "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
            "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
            "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId]"
            " WHERE [hl].[Id] = ? "
            "       AND [dsn].[Visibility] != ? "
            "       AND [ds].[InstanceFilter] IS ? "
            "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetDb().GetDbFile(), query))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare nodes count statement");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_hierarchyLevelId);
        stmt->BindInt(bindingIndex++, (int)NodeVisibility::Hidden);
        NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, GetContext().GetInstanceFilter());
        stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

        DbResult stepResult = stmt->Step();
        if (BE_SQLITE_ROW != stepResult)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Unexpected nodes count statement result: %d", (int)stepResult));

        return (size_t)stmt->GetValueUInt64(0);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8CP _GetNodesQuery() const override
        {
        static Utf8CP query =
            "SELECT " NODE_SELECT_STMT("hl", "n", "nk")
            "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
            "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
            "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[NodeId] = [n].[Id] "
            "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [dsn].[DataSourceId] "
            "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
            "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
            " WHERE [hl].[Id] = ? "
            "       AND [dsn].[Visibility] != ? "
            "       AND [ds].[InstanceFilter] IS ? "
            "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
            " ORDER BY [ds].[Index], [dsn].[NodeIndex]";
        return query;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _BindNodesStatement(StatementR stmt, int& bindingIndex) const override
        {
        NodesCacheHelpers::BindGuid(stmt, bindingIndex++, m_hierarchyLevelId);
        stmt.BindInt(bindingIndex++, (int)NodeVisibility::Hidden);
        NodesCacheHelpers::BindInstanceFilter(stmt, bindingIndex++, GetContext().GetInstanceFilter());
        stmt.BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
        }

public:
    CachedHierarchyLevelDirectNodeIterator(NavNodesProviderContextCR context, Db& db, StatementCache& statements, BeMutex& mutex, BeGuid hierarchyLevelId)
        : SqliteCacheDirectNodeIteratorBase(context, db, statements, mutex), m_hierarchyLevelId(hierarchyLevelId)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<DirectNodesIterator> SqliteCacheDirectNodeIteratorBase::CreateForHierarchyLevel(NavNodesProviderContextCR context, Db& db, StatementCache& statements, BeMutex& mutex, BeGuid hierarchyLevelId)
    {
    return std::make_unique<CachedHierarchyLevelDirectNodeIterator>(context, db, statements, mutex, hierarchyLevelId);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CachedPartialDataSourceDirectNodeIterator : SqliteCacheDirectNodeIteratorBase
{
private:
    BeGuid m_dataSourceId;

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _QueryHasVirtualNodes() const override
        {
        static Utf8CP query =
            "SELECT 1 "
            "  FROM [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn "
            "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId]"
            " WHERE [dsn].[DataSourceId] = ? "
            "       AND [dsn].[Visibility] = ? ";

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Virtual nodes query: `%s`", query));

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetDb().GetDbFile(), query))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare virtual nodes statement");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_dataSourceId);
        stmt->BindInt(bindingIndex++, (int)NodeVisibility::Virtual);

        return BE_SQLITE_ROW == stmt->Step();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t _QueryTotalNodesCount() const override
        {
        static Utf8CP query =
            "SELECT COUNT(1) "
            "  FROM [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn "
            "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId]"
            " WHERE [dsn].[DataSourceId] = ? "
            "       AND [dsn].[Visibility] != ? ";

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetDb().GetDbFile(), query))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare nodes count statement");

        int bindingIndex = 1;
        NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_dataSourceId);
        stmt->BindInt(bindingIndex++, (int)NodeVisibility::Hidden);

        DbResult stepResult = stmt->Step();
        if (BE_SQLITE_ROW != stepResult)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Unexpected nodes count statement result: %d", (int)stepResult));

        return (size_t)stmt->GetValueUInt64(0);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8CP _GetNodesQuery() const override
        {
        static Utf8CP query =
            "SELECT " NODE_SELECT_STMT("hl", "n", "nk")
            "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
            "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
            "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[NodeId] = [n].[Id] "
            "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [dsn].[DataSourceId] "
            "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
            " WHERE     [ds].[Id] = ? "
            "       AND [dsn].[Visibility] != ? "
            " ORDER BY [dsn].[NodeIndex] ";
        return query;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _BindNodesStatement(StatementR stmt, int& bindingIndex) const override
        {
        NodesCacheHelpers::BindGuid(stmt, bindingIndex++, m_dataSourceId);
        stmt.BindInt(bindingIndex++, (int)NodeVisibility::Hidden);
        }

public:
    CachedPartialDataSourceDirectNodeIterator(NavNodesProviderContextCR context, Db& db, StatementCache& statements, BeMutex& mutex, BeGuid dataSourceId)
        : SqliteCacheDirectNodeIteratorBase(context, db, statements, mutex), m_dataSourceId(dataSourceId)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<DirectNodesIterator> SqliteCacheDirectNodeIteratorBase::CreateForDataSource(NavNodesProviderContextCR context, Db& db, StatementCache& statements, BeMutex& mutex, BeGuid dataSourceId)
    {
    return std::make_unique<CachedPartialDataSourceDirectNodeIterator>(context, db, statements, mutex, dataSourceId);
    }
