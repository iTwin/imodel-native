/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECDb/ECDbApi.h>
#include "NavNodesCache.h"
#include "NavNodesDataSource.h"
#include "NavNodeProviders.h"
#include "ExtendedData.h"
#include "UpdateHandler.h"
#include "LoggingHelper.h"

USING_NAMESPACE_BENTLEY_LOGGING

#define NAVNODES_CACHE_DB_NAME              L"HierarchyCache.db"
#define NAVNODES_CACHE_DB_VERSION_MAJOR     14
#define NAVNODES_CACHE_DB_VERSION_MINOR     0
#define NAVNODES_CACHE_BINARY_INDEX
//#define NAVNODES_CACHE_DEBUG
#ifdef NAVNODES_CACHE_DEBUG
    // binary index is hard to read - use string index when debugging
    #undef NAVNODES_CACHE_BINARY_INDEX
#endif

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct NavigationCacheBusyRetry : BusyRetry
{
    int _OnBusy(int count) const override
        {
        return 0;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter::DataSourceFilter(RelatedInstanceInfo const& relatedInstanceInfo, Utf8CP instanceFilter)
    {
    m_relatedInstanceInfo = (relatedInstanceInfo.IsValid()) ? new RelatedInstanceInfo(relatedInstanceInfo) : nullptr;
    m_instanceFilter = instanceFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter::DataSourceFilter(DataSourceFilter const& other)
    : m_relatedInstanceInfo(nullptr), m_instanceFilter(other.m_instanceFilter)
    {
    if (nullptr != other.m_relatedInstanceInfo && other.m_relatedInstanceInfo->IsValid())
        m_relatedInstanceInfo = new RelatedInstanceInfo(*other.m_relatedInstanceInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter::DataSourceFilter(DataSourceFilter&& other)
    : m_relatedInstanceInfo(other.m_relatedInstanceInfo), m_instanceFilter(std::move(other.m_instanceFilter))
    {
    other.m_relatedInstanceInfo = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter& DataSourceFilter::operator=(DataSourceFilter const& other)
    {
    m_instanceFilter = other.m_instanceFilter;
    m_relatedInstanceInfo = (nullptr != other.m_relatedInstanceInfo && other.m_relatedInstanceInfo->IsValid()) ? new RelatedInstanceInfo(*other.m_relatedInstanceInfo) : nullptr;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceFilter& DataSourceFilter::operator=(DataSourceFilter&& other)
    {
    m_instanceFilter = std::move(other.m_instanceFilter);
    m_relatedInstanceInfo = other.m_relatedInstanceInfo;
    other.m_relatedInstanceInfo = nullptr;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DataSourceFilter::InitFromJson(RapidJsonValueCR json)
    {
    DELETE_AND_CLEAR(m_relatedInstanceInfo);
    m_instanceFilter = nullptr;

    if (json.IsNull())
        return;

    if (json.HasMember("RelatedInstanceInfo"))
        {
        RapidJsonValueCR relatedInstanceInfoJson = json["RelatedInstanceInfo"];
        RequiredRelationDirection direction = (RequiredRelationDirection)relatedInstanceInfoJson["Direction"].GetInt();
        ECInstanceId instanceId(relatedInstanceInfoJson["ECInstanceId"].GetUint64());
        RapidJsonValueCR relationshipIdsJson = relatedInstanceInfoJson["ECRelationshipClassIds"];
        bset<ECClassId> relationshipIds;
        if (relationshipIdsJson.IsArray())
            {
            for (rapidjson::SizeType i = 0; i < relationshipIdsJson.Size(); ++i)
                relationshipIds.insert(ECClassId(relationshipIdsJson[i].GetUint64()));
            }
        m_relatedInstanceInfo = new RelatedInstanceInfo(relationshipIds, direction, instanceId);
        }
    if (json.HasMember("Filter"))
        m_instanceFilter = json["Filter"].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DataSourceFilter::AsJson() const
    {
    rapidjson::Document json;
    if (nullptr != m_relatedInstanceInfo && m_relatedInstanceInfo->IsValid())
        {
        json.SetObject();
        rapidjson::Value relatedInstanceInfoJson;
        relatedInstanceInfoJson.SetObject();
        relatedInstanceInfoJson.AddMember("Direction", (int)m_relatedInstanceInfo->m_direction, json.GetAllocator());
        relatedInstanceInfoJson.AddMember("ECInstanceId", m_relatedInstanceInfo->m_instanceId.GetValueUnchecked(), json.GetAllocator());
        rapidjson::Value relationshipClassIdsJson;
        relationshipClassIdsJson.SetArray();
        for (ECClassId classId : m_relatedInstanceInfo->m_relationshipClassIds)
            relationshipClassIdsJson.PushBack(classId.GetValue(), json.GetAllocator());
        relatedInstanceInfoJson.AddMember("ECRelationshipClassIds", relationshipClassIdsJson, json.GetAllocator());
        json.AddMember("RelatedInstanceInfo", relatedInstanceInfoJson, json.GetAllocator());
        }
    if (!m_instanceFilter.empty())
        {
        json.SetObject();
        json.AddMember("Filter", rapidjson::Value(m_instanceFilter.c_str(), json.GetAllocator()), json.GetAllocator());
        }
    return json;
    }

static const unsigned DS_INDEX_PADDING = 20;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetPaddedNumber(uint64_t number)
    {
    Utf8String padded;
    padded.reserve(DS_INDEX_PADDING);
    auto numberStr = std::to_string(number);
    for (int i = numberStr.length(); i < DS_INDEX_PADDING; i++)
        padded.append("0");
    padded.append(numberStr.c_str());
    return padded;
    }

static const Utf8String DS_INDEX_SEPARATOR = "-";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String IndexToString(bvector<uint64_t> const& index, bool pad)
    {
    Utf8String str;
    if (pad)
        str.reserve(index.size() * (DS_INDEX_PADDING + 1));
    for (uint64_t i : index)
        {
        if (!str.empty())
            str.append(DS_INDEX_SEPARATOR);
        if (pad)
            str.append(GetPaddedNumber(i).c_str());
        else
            str.append(std::to_string(i).c_str());
        }
    return str;
    }

#ifdef NAVNODES_CACHE_BINARY_INDEX
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<uint64_t> IndexFromBlob(void const* blob, int size)
    {
    uint64_t const* arr = static_cast<uint64_t const*>(blob);
    int count = size / sizeof(uint64_t);
    bvector<uint64_t> index;
    index.reserve(count);
    for (int i = 0; i < count; ++i)
        index.push_back(arr[i]);
    return index;
    }
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2019
+===============+===============+===============+===============+===============+======*/
struct ConcatBinaryIndexScalar : BeSQLite::ScalarFunction
    {
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        uint64_t const* lhsArr = static_cast<uint64_t const*>(args[0].GetValueBlob());
        int lhsCount = args[0].GetValueBytes() / sizeof(uint64_t);
        uint64_t const* rhsArr = static_cast<uint64_t const*>(args[1].GetValueBlob());
        int rhsCount = args[1].GetValueBytes() / sizeof(uint64_t);
        bvector<uint64_t> result;
        result.reserve(lhsCount + rhsCount);
        for (int i = 0; i < lhsCount; ++i)
            result.push_back(lhsArr[i]);
        for (int i = 0; i < rhsCount; ++i)
            result.push_back(rhsArr[i]);
        ctx.SetResultBlob(reinterpret_cast<void const*>(&result.front()), result.size() * sizeof(uint64_t), DbFunction::Context::CopyData::Yes);
        }
    ConcatBinaryIndexScalar()
        : ScalarFunction(NODESCACHE_FUNCNAME_ConcatBinaryIndex, 2, DbValueType::BlobVal)
        {}
    };
#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<uint64_t> IndexFromString(Utf8StringCR str)
    {
    bvector<uint64_t> index;
    size_t offset = 0;
    Utf8String token;
    while (Utf8String::npos != (offset = str.GetNextToken(token, DS_INDEX_SEPARATOR.c_str(), offset)))
        index.push_back(BeStringUtilities::ParseUInt64(token.c_str()));
    return index;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static JsonNavNodePtr CreateNodeFromString(JsonNavNodesFactoryCR nodesFactory, IConnectionR connection, Utf8CP serializedNode)
    {
    rapidjson::Document json;
    json.Parse(serializedNode);
    return nodesFactory.CreateFromJson(connection, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static JsonNavNodePtr CreateNodeFromStatement(Statement& stmt, JsonNavNodesFactoryCR nodesFactory, IConnectionR connection)
    {
    if (stmt.IsColumnNull(3))
        return nullptr;

    Utf8CP serializedNode = stmt.GetValueText(3);
    if (nullptr == serializedNode || 0 == *serializedNode)
        {
        BeAssert(false);
        return nullptr;
        }

    JsonNavNodePtr node = CreateNodeFromString(nodesFactory, connection, serializedNode);
    if (node.IsNull())
        return nullptr;

    node->SetNodeId(stmt.GetValueUInt64(4));
    node->SetIsExpanded(!stmt.IsColumnNull(5));
    node->SetParentNodeId(stmt.IsColumnNull(1) ? 0 : stmt.GetValueUInt64(1));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetVirtualParentId(stmt.IsColumnNull(2) ? 0 : stmt.GetValueUInt64(2));

    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, *node, stmt.GetValueText(6)));

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static IConnectionP GetConnectionFromStatement(Statement& stmt, IConnectionCacheCR connections)
    {
    Utf8CP connectionId = stmt.GetValueText(0);
    return connections.GetConnection(connectionId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static JsonNavNodePtr CreateNodeFromStatement(Statement& stmt, JsonNavNodesFactoryCR nodesFactory, IConnectionCacheCR connections)
    {
    IConnection* connection = GetConnectionFromStatement(stmt, connections);
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }
    return CreateNodeFromStatement(stmt, nodesFactory, *connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetSerializedJson(RapidJsonValueCR json)
    {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    json.Accept(writer);
    return buf.GetString();
    }

static PropertySpec s_versionPropertySpec("Version", "HierarchyCache");
#ifndef NAVNODES_CACHE_DEBUG
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BeVersion GetCacheVersion(BeSQLite::Db const& db)
    {
    Utf8String versionStr;
    if (BE_SQLITE_ROW != db.QueryProperty(versionStr, s_versionPropertySpec))
        return BeVersion();

    return BeVersion(versionStr.c_str());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult CreateCacheDb(BeSQLite::Db& db, BeFileNameCR path, DefaultTxn txnLockType)
    {
    DbResult result = db.CreateNewDb(path, BeGuid(), Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8,
        true, txnLockType, new NavigationCacheBusyRetry()));

    if (BE_SQLITE_OK == result)
        {
        // save the cache version
        static BeVersion s_cacheVersion(NAVNODES_CACHE_DB_VERSION_MAJOR, NAVNODES_CACHE_DB_VERSION_MINOR);
        db.SaveProperty(s_versionPropertySpec, s_cacheVersion.ToString(), nullptr, 0);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::Initialize(BeFileNameCR tempDirectory)
    {
    BeSQLiteLib::Initialize(tempDirectory);

    DbResult result = BE_SQLITE_ERROR;
    if (NodesCacheType::Memory == m_type)
        {
        result = m_db.CreateNewDb(":memory:", BeGuid(), Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8,
            true, DefaultTxn::Yes, new NavigationCacheBusyRetry()));
        }

    if (NodesCacheType::Disk == m_type)
        {
        BeFileName path(tempDirectory);
        path.AppendToPath(NAVNODES_CACHE_DB_NAME);

#ifdef NAVNODES_CACHE_DEBUG
        // don't use Exclusive lock so we can have a look at the DB while debugging
        DefaultTxn txnLockType = DefaultTxn::Yes;
#else
        DefaultTxn txnLockType = DefaultTxn::Exclusive;
#endif

#ifdef NAVNODES_CACHE_DEBUG
        // always create a new cache if debugging
        path.BeDeleteFile();
#else
        if (path.DoesPathExist())
            {
            result = m_db.OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::ReadWrite, txnLockType, new NavigationCacheBusyRetry()));
            if (BE_SQLITE_OK == result && GetCacheVersion(m_db).GetMajor() != NAVNODES_CACHE_DB_VERSION_MAJOR)
                {
                // if the existing cache version is too old, simply delete the old cache and create a new one
                m_db.CloseDb();
                path.BeDeleteFile();
                result = BE_SQLITE_ERROR_ProfileTooOld;
                }
            }
#endif

        if (BE_SQLITE_OK != result && BE_SQLITE_BUSY != result)
            result = CreateCacheDb(m_db, path, txnLockType);

        if (BE_SQLITE_BUSY == result)
            {
            path.SetName(tempDirectory).AppendSeparator();
            Utf8PrintfString fileName("HierarchyCache.%s.db", BeGuid(true).ToString().c_str());
            path.AppendUtf8(fileName.c_str());

            result = CreateCacheDb(m_db, path, txnLockType);
            m_tempCache = true;
            }
        }

    if (BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return;
        }

    // create the tables
    if (!m_db.TableExists(NODESCACHE_TABLENAME_HierarchyLevels))
        {
        Utf8CP ddl = "[Id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE, "
                     "[PhysicalParentNodeId] INTEGER REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[VirtualParentNodeId] INTEGER REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[ConnectionId] TEXT NOT NULL REFERENCES " NODESCACHE_TABLENAME_Connections "([ConnectionId]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[RulesetId] TEXT NOT NULL REFERENCES " NODESCACHE_TABLENAME_Rulesets "([RulesetId]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[Locale] TEXT, "
                     "[RemovalId] GUID ";
        m_db.CreateTable(NODESCACHE_TABLENAME_HierarchyLevels, ddl);
        m_db.ExecuteSql("CREATE UNIQUE INDEX [UX_HierarchyLevels_Virtual] ON [" NODESCACHE_TABLENAME_HierarchyLevels "](COALESCE([VirtualParentNodeId], 0),[ConnectionId],[RulesetId],[Locale],COALESCE([RemovalId], 0))");
        m_db.ExecuteSql("CREATE INDEX [IX_HierarchyLevels_Physical] ON [" NODESCACHE_TABLENAME_HierarchyLevels "]([PhysicalParentNodeId],[ConnectionId],[RulesetId],[Locale])");
        m_db.ExecuteSql("CREATE INDEX [IX_HierarchyLevels_Virtual] ON [" NODESCACHE_TABLENAME_HierarchyLevels "]([VirtualParentNodeId],[ConnectionId],[RulesetId],[Locale])");
        m_db.ExecuteSql("CREATE INDEX [IX_HierarchyLevels_RemovalId] ON [" NODESCACHE_TABLENAME_HierarchyLevels "]([RemovalId])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_DataSources))
        {
        Utf8CP ddl = "[Id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE, "
                     "[HierarchyLevelId] INTEGER REFERENCES " NODESCACHE_TABLENAME_HierarchyLevels "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
#ifdef NAVNODES_CACHE_BINARY_INDEX
                     "[FullIndex] BINARY, "
#else
                     "[FullIndex] TEXT NOT NULL, "
#endif
                     "[IsInitialized] BOOLEAN NOT NULL DEFAULT FALSE, "
                     "[Filter] TEXT, "
                     "[IsUpdatesDisabled] BOOLEAN NOT NULL DEFAULT FALSE ";
        m_db.CreateTable(NODESCACHE_TABLENAME_DataSources, ddl);
        m_db.ExecuteSql("CREATE INDEX [IX_DataSources_HierarchyLevelId] ON [" NODESCACHE_TABLENAME_DataSources "]([HierarchyLevelId])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_DataSourceClasses))
        {
        Utf8CP ddl = "[DataSourceId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[ECClassId] INTEGER NOT NULL, "
                     "[Polymorphic] BOOLEAN NOT NULL DEFAULT FALSE, "
                     "PRIMARY KEY([DataSourceId], [ECClassId])";
        m_db.CreateTable(NODESCACHE_TABLENAME_DataSourceClasses, ddl);
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_DataSourceSettings))
        {
        Utf8CP ddl = "[DataSourceId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[SettingId] TEXT NOT NULL, "
                     "[SettingValue] TEXT NOT NULL, "
                     "PRIMARY KEY([SettingId],[DataSourceId])";
        m_db.CreateTable(NODESCACHE_TABLENAME_DataSourceSettings, ddl);
        m_db.ExecuteSql("CREATE INDEX [IX_DataSourceSettings] ON [" NODESCACHE_TABLENAME_DataSourceSettings "]([DataSourceId])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_Nodes))
        {
        Utf8CP ddl = "[DataSourceId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[Id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE, "
#ifdef NAVNODES_CACHE_BINARY_INDEX
                     "[Index] BINARY, "
#else
                     "[Index] TEXT NOT NULL, "
#endif
                     "[IsVirtual] BOOLEAN NOT NULL DEFAULT FALSE, "
                     "[Data] TEXT NOT NULL, "
                     "[Label] TEXT, "
                     "[UniqueHash] TEXT NOT NULL";
        m_db.CreateTable(NODESCACHE_TABLENAME_Nodes, ddl);
        m_db.ExecuteSql("CREATE INDEX [IX_Nodes_DataSourceId] ON [" NODESCACHE_TABLENAME_Nodes "]([DataSourceId])");
        m_db.ExecuteSql("CREATE INDEX [IX_Nodes_Hash] ON [" NODESCACHE_TABLENAME_Nodes "]([UniqueHash])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_NodeKeys))
        {
        Utf8CP ddl = "[NodeId] INTEGER PRIMARY KEY NOT NULL UNIQUE REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[Type] TEXT NOT NULL,"
                     "[ECClassId] INTEGER, "
                     "[ECInstanceId] INTEGER,"
                     "[PathFromRoot] TEXT NOT NULL";
        m_db.CreateTable(NODESCACHE_TABLENAME_NodeKeys, ddl);
        m_db.ExecuteSql("CREATE INDEX [IX_NodeKeys_ECInstanceNodeKeys] ON [" NODESCACHE_TABLENAME_NodeKeys "]([Type],[ECClassId],[ECInstanceId])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_AffectingInstances))
        {
        Utf8CP ddl = "[NodeId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[ECClassId] INTEGER NOT NULL, "
                     "[ECInstanceId] INTEGER NOT NULL";
        m_db.CreateTable(NODESCACHE_TABLENAME_AffectingInstances, ddl);
        m_db.ExecuteSql("CREATE UNIQUE INDEX [UX_GroupedECInstances] ON [" NODESCACHE_TABLENAME_AffectingInstances "]([NodeId],[ECClassId],[ECInstanceId])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_Connections))
        {
        Utf8CP ddl = "[ConnectionId] TEXT PRIMARY KEY NOT NULL, "
                     "[DbGuid] GUID NOT NULL, "
                     "[DbPath] TEXT NOT NULL, "
                     "[LastModTime] INTEGER NOT NULL, "
                     "[LastUsedTime] INTEGER NOT NULL";
        m_db.CreateTable(NODESCACHE_TABLENAME_Connections, ddl);
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_Rulesets))
        {
        Utf8CP ddl = "[RulesetId] TEXT PRIMARY KEY NOT NULL, "
                     "[RulesetHash] TEXT NOT NULL";
        m_db.CreateTable(NODESCACHE_TABLENAME_Rulesets, ddl);
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_ExpandedNodes))
        {
        Utf8CP createTempTable = "CREATE TEMP TABLE [" NODESCACHE_TABLENAME_ExpandedNodes "] ("
            "[NodeId] INTEGER NOT NULL)";
        DbResult result = m_db.ExecuteSql(createTempTable);
        BeAssert(result == BE_SQLITE_OK);

        Utf8CP createUniqueIndex = "CREATE UNIQUE INDEX [UX_NodeIndex] ON [" NODESCACHE_TABLENAME_ExpandedNodes "]([NodeId])";
        result = m_db.ExecuteSql(createUniqueIndex);
        BeAssert(result == BE_SQLITE_OK);
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_NodesOrder))
        {
        Utf8CP ddl = "[HierarchyLevelId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_HierarchyLevels "([Id]) ON DELETE CASCADE, "
                     "[DataSourceId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE, "
                     "[NodeId] INTEGER PRIMARY KEY NOT NULL UNIQUE REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE, "
#ifdef NAVNODES_CACHE_BINARY_INDEX
                     "[OrderValue] BINARY";
#else
                     "[OrderValue] TEXT NOT NULL";
#endif
        m_db.CreateTable(NODESCACHE_TABLENAME_NodesOrder, ddl);
        m_db.ExecuteSql("CREATE UNIQUE INDEX [UX_Order] ON [" NODESCACHE_TABLENAME_NodesOrder "]([HierarchyLevelId],[OrderValue])");
        m_db.ExecuteSql("CREATE TRIGGER IF NOT EXISTS [TRIGG_" NODESCACHE_TABLENAME_NodesOrder "] AFTER INSERT ON [" NODESCACHE_TABLENAME_Nodes "] "
                        "BEGIN"
                        "    INSERT INTO [" NODESCACHE_TABLENAME_NodesOrder "] "
                        "    SELECT [d].[HierarchyLevelId], [d].[Id], NEW.[Id], "
#ifdef NAVNODES_CACHE_BINARY_INDEX
                        "    " NODESCACHE_FUNCNAME_ConcatBinaryIndex "([d].[FullIndex], NEW.[Index]) "
#else
                        "    [d].[FullIndex] || '-' || NEW.[Index] "
#endif
                        "    FROM [" NODESCACHE_TABLENAME_DataSources "] d "
                        "    WHERE [d].[Id] = NEW.[DataSourceId]; "
                        "END");
        }
    m_db.SaveChanges();

#ifdef NAVNODES_CACHE_BINARY_INDEX
    m_customFunctions.push_back(new ConcatBinaryIndexScalar());
    m_db.AddFunction(*m_customFunctions.back());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache::NodesCache(BeFileNameCR tempDirectory, JsonNavNodesFactoryCR nodesFactory, INodesProviderContextFactoryCR contextFactory, IConnectionManagerCR connections,
    IUserSettingsManager const& userSettings, IECSqlStatementCacheProvider& ecsqlStatements, NodesCacheType type)
    : m_nodesFactory(nodesFactory), m_contextFactory(contextFactory), m_connections(connections), m_userSettings(userSettings),
    m_statements(50), m_type(type), m_tempCache(false), m_ecsqlStamementCache(ecsqlStatements), m_sizeLimit(0)
    {
#ifdef NAVNODES_CACHE_DEBUG
    // always use Disk cache when debugging
    m_type = NodesCacheType::Disk;
#endif
    Initialize(tempDirectory);
    m_connections.AddListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache::~NodesCache()
    {
    m_connections.DropListener(*this);
    if (!m_tempCache && !Utf8String::IsNullOrEmpty(m_db.GetDbFileName()))
        LimitCacheSize();

    if (!m_db.IsDbOpen() || !m_tempCache)
        return;

    BeFileName dbFile(m_db.GetDbFileName());
    m_statements.Empty();
    m_db.CloseDb();
    dbFile.BeDeleteFile();

    for (BeSQLite::ScalarFunction* func : m_customFunctions)
        delete func;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::OnFirstConnection(IConnectionCR connection)
    {
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::NavigationCache, Utf8PrintfString("First connection: Id=%s, Guid=%s, Path=%s",
        connection.GetId().c_str(), connection.GetDb().GetDbGuid().ToString().c_str(), connection.GetDb().GetDbFileName()).c_str());

    // get the last modified time of the db file
    time_t fileModTime;
    BeFileName(connection.GetDb().GetDbFileName()).GetFileTime(nullptr, nullptr, &fileModTime);

    // delete connections whose modification times don't match file modification time (hierarchies may be out of sync)
    Utf8CP deleteQuery = "DELETE FROM [" NODESCACHE_TABLENAME_Connections "] WHERE [DbGuid] = ? AND [DbPath] = ? AND [LastModTime] <> ?";
    CachedStatementPtr deleteStmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(deleteStmt, *m_db.GetDbFile(), deleteQuery))
        {
        BeAssert(false);
        return;
        }
    deleteStmt->BindGuid(1, connection.GetDb().GetDbGuid());
    deleteStmt->BindText(2, connection.GetDb().GetDbFileName(), Statement::MakeCopy::No);
    deleteStmt->BindInt64(3, (int64_t)fileModTime);
    DbResult deleteResult = deleteStmt->Step();
    BeAssert(BE_SQLITE_DONE == deleteResult);
    LoggingHelper::LogMessage(Log::NavigationCache, Utf8PrintfString("Deleted cached connections with LastModTime <> %" PRIu64, (uint64_t)fileModTime).c_str());

    // either update existing ECDb's connection id or insert a new one
    Utf8CP selectQuery = "SELECT * FROM [" NODESCACHE_TABLENAME_Connections "] WHERE [DbGuid] = ? AND [DbPath] = ?";
    CachedStatementPtr selectStmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(selectStmt, *m_db.GetDbFile(), selectQuery))
        {
        BeAssert(false);
        return;
        }
    selectStmt->BindGuid(1, connection.GetDb().GetDbGuid());
    selectStmt->BindText(2, connection.GetDb().GetDbFileName(), Statement::MakeCopy::No);
    DbResult selectResult = selectStmt->Step();
    if (BE_SQLITE_ROW == selectResult)
        {
        LoggingHelper::LogMessage(Log::NavigationCache, "Found matching connection, update");
        BeAssert(BE_SQLITE_DONE == selectStmt->Step() && "Don't expect more than one row");

        // update existing connection id
        Utf8CP updateQuery = "UPDATE [" NODESCACHE_TABLENAME_Connections "] SET [ConnectionId] = ?, [LastUsedTime] = ? WHERE [DbGuid] = ? AND [DbPath] = ?";
        CachedStatementPtr updateStmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(updateStmt, *m_db.GetDbFile(), updateQuery))
            {
            BeAssert(false);
            return;
            }
        updateStmt->BindText(1, connection.GetId().c_str(), Statement::MakeCopy::No);
        updateStmt->BindUInt64(2, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        updateStmt->BindGuid(3, connection.GetDb().GetDbGuid());
        updateStmt->BindText(4, connection.GetDb().GetDbFileName(), Statement::MakeCopy::No);
        DbResult updateResult = updateStmt->Step();
        BeAssert(BE_SQLITE_DONE == updateResult);
        }
    else
        {
        LoggingHelper::LogMessage(Log::NavigationCache, "Matching connection not found, insert");
        // insert a new connection
        Utf8CP insertQuery = "INSERT INTO [" NODESCACHE_TABLENAME_Connections "] ([ConnectionId], [DbGuid], [DbPath], [LastModTime], [LastUsedTime]) VALUES (?, ?, ?, ?, ?)";
        CachedStatementPtr insertStmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(insertStmt, *m_db.GetDbFile(), insertQuery))
            {
            BeAssert(false);
            return;
            }
        insertStmt->BindText(1, connection.GetId().c_str(), Statement::MakeCopy::No);
        insertStmt->BindGuid(2, connection.GetDb().GetDbGuid());
        insertStmt->BindText(3, connection.GetDb().GetDbFileName(), Statement::MakeCopy::No);
        insertStmt->BindInt64(4, (int64_t)fileModTime);
        insertStmt->BindUInt64(5, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        DbResult insertResult = insertStmt->Step();
        BeAssert(BE_SQLITE_DONE == insertResult);
        }

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::OnConnectionClosed(IConnectionCR connection)
    {
    if (!connection.GetDb().IsDbOpen())
        {
        BeAssert(false);
        return;
        }

    // get the last modified time of the db file
    time_t lastMod;
    BeFileName(connection.GetDb().GetDbFileName()).GetFileTime(nullptr, nullptr, &lastMod);

    Utf8CP query = "UPDATE [" NODESCACHE_TABLENAME_Connections "] SET [LastModTime] = ?, [LastUsedTime] = ? WHERE [ConnectionId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return;
        }
    stmt->BindInt64(1, (int64_t)lastMod);
    stmt->BindUInt64(2, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    stmt->BindText(3, connection.GetId().c_str(), Statement::MakeCopy::No);
    stmt->Step();

    m_quickDataSourceCache.clear();
    m_quickNodesCache.clear();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_OnConnectionEvent(ConnectionEvent const& evt)
    {
    if (ConnectionEventType::Opened == evt.GetEventType())
        const_cast<NodesCache*>(this)->OnFirstConnection(evt.GetConnection());
    else if (ConnectionEventType::Closed == evt.GetEventType())
        const_cast<NodesCache*>(this)->OnConnectionClosed(evt.GetConnection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::OnRulesetCreated(PresentationRuleSetCR ruleset)
    {
    Utf8CP hashQuery = "SELECT [RulesetHash] FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [RulesetId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), hashQuery))
        {
        BeAssert(false);
        return;
        }
    stmt->BindText(1, ruleset.GetRuleSetId().c_str(), Statement::MakeCopy::No);

    Utf8StringCR rulesetHash = ruleset.GetHash();
    if (BE_SQLITE_ROW == stmt->Step() && !rulesetHash.Equals(stmt->GetValueText(0)))
        {
        Utf8CP deleteQuery = "DELETE FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [RulesetId] = ?";
        CachedStatementPtr deleteStmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(deleteStmt, *m_db.GetDbFile(), deleteQuery))
            {
            BeAssert(false);
            return;
            }
        deleteStmt->BindText(1, ruleset.GetRuleSetId().c_str(), Statement::MakeCopy::No);
        deleteStmt->Step();
        }

    Utf8CP insertQuery = "INSERT INTO [" NODESCACHE_TABLENAME_Rulesets "] ([RulesetId], [RulesetHash]) VALUES (?, ?)";
    CachedStatementPtr insertStmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(insertStmt, *m_db.GetDbFile(), insertQuery))
        {
        BeAssert(false);
        return;
        }
    insertStmt->BindText(1, ruleset.GetRuleSetId().c_str(), Statement::MakeCopy::No);
    insertStmt->BindText(2, rulesetHash.c_str(), Statement::MakeCopy::No);
    insertStmt->Step();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct NodesCache::Savepoint : IHierarchyCache::Savepoint
    {
    BeSQLite::Savepoint m_sqliteSavepoint;
    NodesCache const& m_cache;
    Savepoint(NodesCache const& cache)
        : m_sqliteSavepoint(cache.m_db, BeGuid(true).ToString().c_str()), m_cache(cache)
        {
        BeAssert(m_sqliteSavepoint.IsActive());
        }
    void _Cancel() override
        {
        m_sqliteSavepoint.Cancel();
        m_cache.m_quickDataSourceCache.clear();
        m_cache.m_quickNodesCache.clear();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IHierarchyCache::SavepointPtr NodesCache::_CreateSavepoint() {return new Savepoint(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::IsNodeCached(uint64_t nodeId) const
    {
    Utf8String query = "SELECT 1 FROM [" NODESCACHE_TABLENAME_Nodes "] WHERE [Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindInt64(1, nodeId);

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::IsHierarchyLevelCached(Utf8StringCR connectionId, Utf8CP rulesetId, Utf8CP locale) const
    {
    Utf8String query = "SELECT 1 "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] "
                       " WHERE [ConnectionId] = ? AND [RulesetId] = ? AND [Locale] = ? AND [VirtualParentNodeId] IS NULL";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, connectionId.c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, rulesetId, Statement::MakeCopy::No);
    stmt->BindText(3, locale, Statement::MakeCopy::No);

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::IsHierarchyLevelCached(uint64_t virtualParentNodeId) const
    {
    Utf8String query = "SELECT 1 FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] WHERE [VirtualParentNodeId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindInt64(1, virtualParentNodeId);

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreatePathFromRootString(bvector<Utf8String> const& path)
    {
    rapidjson::Document json(rapidjson::kArrayType);
    for (Utf8StringCR pathElement : path)
        json.PushBack(rapidjson::StringRef(pathElement.c_str()), json.GetAllocator());
    return BeRapidJsonUtilities::ToString(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheNodeKey(NavNodeCR node)
    {
    // delete the old key, if exists
    Utf8String query = "DELETE FROM [" NODESCACHE_TABLENAME_NodeKeys "] WHERE [NodeId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }
    stmt->BindUInt64(1, node.GetNodeId());
    stmt->Step();

    // insert a new key
    query = "INSERT INTO [" NODESCACHE_TABLENAME_NodeKeys "] ([NodeId],[Type],[PathFromRoot],[ECClassId],[ECInstanceId])"
                 "VALUES (?, ?, ?, ?, ?)";

    NavNodeKeyCR key = *node.GetKey();

    stmt = nullptr;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    stmt->BindInt64(1, node.GetNodeId());
    stmt->BindText(2, node.GetType().c_str(), Statement::MakeCopy::Yes);
    Utf8String pathFromRootString = CreatePathFromRootString(key.GetPathFromRoot());
    stmt->BindText(3, pathFromRootString, Statement::MakeCopy::No);

    NavNodeExtendedData extendedData(node);
    if (extendedData.HasECClassId())
        stmt->BindId(4, extendedData.GetECClassId());
    else
        stmt->BindNull(4);

    if (nullptr != key.AsECInstanceNodeKey())
        stmt->BindId(5, key.AsECInstanceNodeKey()->GetInstanceId());
    else
        stmt->BindNull(5);

    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void InsertNodeInstanceKeys(Statement& stmt, NavNodeCR node, bvector<ECInstanceKey> const& keys)
    {
    for (ECInstanceKeyCR key : keys)
        {
        stmt.BindInt64(1, node.GetNodeId());
        stmt.BindInt64(2, key.GetClassId().GetValueUnchecked());
        stmt.BindInt64(3, key.GetInstanceId().GetValueUnchecked());
        stmt.Step();
        stmt.Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheNodeInstanceKeys(NavNodeCR node)
    {
    // delete old keys, if any
    Utf8String query = "DELETE FROM [" NODESCACHE_TABLENAME_AffectingInstances "] WHERE [NodeId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }
    stmt->BindUInt64(1, node.GetNodeId());
    stmt->Step();

    // insert new keys
    query = "INSERT INTO [" NODESCACHE_TABLENAME_AffectingInstances "] ([NodeId], [ECClassId], [ECInstanceId])"
                 "VALUES (?, ?, ?)";

    stmt = nullptr;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    NavNodeExtendedData extendedData(node);
    InsertNodeInstanceKeys(*stmt, node, extendedData.GetGroupedInstanceKeys());
    InsertNodeInstanceKeys(*stmt, node, extendedData.GetSkippedInstanceKeys());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetNodeDebugString(Utf8CP action, NavNodeCR node, int visibility)
    {
    Utf8String str = action;
    switch (visibility)
        {
        case (int)NodeVisibility::Physical:
            str.append(" physical node {");
            break;
        case (int)NodeVisibility::Virtual:
            str.append(" virtual node {");
            break;
        default:
            str.append(" node {");
        }
    str.append("Id: ").append(std::to_string(node.GetNodeId()).c_str());
    str.append(", Type: '").append(node.GetType()).append("'");
    str.append(", Label: '").append(node.GetLabel()).append("'");
    str.append("}");
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetNodeDebugString(Utf8CP action, uint64_t nodeId, int visibility)
    {
    Utf8String str = action;
    switch (visibility)
        {
        case (int)NodeVisibility::Physical:
            str.append(" physical node {");
            break;
        case (int)NodeVisibility::Virtual:
            str.append(" virtual node {");
            break;
        default:
            str.append(" node {");
        }
    str.append("Id: ").append(std::to_string(nodeId).c_str()).append("}");
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetDataSourceDebugString(Utf8CP action, DataSourceInfo const& info)
    {
    Utf8String str = action;
    str.append(" data source {");
    str.append("Id: ").append(std::to_string(info.GetId()).c_str()).append(", ");
    str.append("HierarchyLevelId: ").append(std::to_string(info.GetHierarchyLevelId()).c_str()).append(", ");
    str.append("Index: ").append(IndexToString(info.GetIndex(), false).c_str());
    str.append("}");
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetHierarchyLevelDebugString(Utf8CP action, HierarchyLevelInfo const& info)
    {
    Utf8String str = action;
    if (!info.GetConnectionId().empty() && !info.GetRulesetId().empty())
        {
        str.append(" hierarchy level {");
        str.append("Id: ").append(std::to_string(info.GetId()).c_str()).append(", ");
        str.append("ConnectionId: '").append(info.GetConnectionId()).append("', ");
        str.append("RulesetId: '").append(info.GetRulesetId()).append("', ");
        str.append("PhysicalParentNodeId: ").append((nullptr != info.GetPhysicalParentNodeId()) ? std::to_string(*info.GetPhysicalParentNodeId()).c_str() : "null").append(", ");
        str.append("VirtualParentNodeId: ").append((nullptr != info.GetVirtualParentNodeId()) ? std::to_string(*info.GetVirtualParentNodeId()).c_str() : "null").append(", ");
        str.append("}");
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheNode(DataSourceInfo const& datasourceInfo, NavNodeR node, uint64_t index, bool isVirtual)
    {
    BeAssert(node.GetNodeId() == 0);

    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_Nodes "] ("
                       "[DataSourceId], [Index], [IsVirtual], [Data], [UniqueHash], [Label]"
                       ") VALUES (?, ?, ?, ?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    int bindingIndex = 1;

    stmt->BindUInt64(bindingIndex++, datasourceInfo.GetId());
#ifdef NAVNODES_CACHE_BINARY_INDEX
    stmt->BindBlob(bindingIndex++, reinterpret_cast<void*>(&index), sizeof(uint64_t), Statement::MakeCopy::No);
#else
    Utf8String indexStr = GetPaddedNumber(index);
    stmt->BindText(bindingIndex++, indexStr.c_str(), Statement::MakeCopy::No);
#endif
    stmt->BindBoolean(bindingIndex++, isVirtual);

    BeAssert(nullptr != dynamic_cast<JsonNavNodeP>(&node));
    JsonNavNodeR jsonNode = static_cast<JsonNavNodeR>(node);

    IConnectionPtr nodeConnection = m_connections.GetConnection(NavNodeExtendedData(jsonNode).GetConnectionId());
    BeAssert(nodeConnection.IsValid());
    Utf8StringCR hash = node.GetKey()->GetHash();

    Utf8String nodeStr = GetSerializedJson(jsonNode.GetJson());
    stmt->BindText(bindingIndex++, nodeStr.c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, hash.c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, node.GetLabel(), Statement::MakeCopy::Yes);

    DbResult result = stmt->Step();
    BeAssert(BE_SQLITE_DONE == result);

    jsonNode.SetNodeId((uint64_t)m_db.GetLastInsertRowId());

    CacheNodeKey(node);
    CacheNodeInstanceKeys(node);

    if (node.IsExpanded())
        SetIsExpanded(node.GetNodeId(), node.IsExpanded());

    LoggingHelper::LogMessage(Log::NavigationCache, GetNodeDebugString("Cached", node, (int)(isVirtual ? NodeVisibility::Virtual : NodeVisibility::Physical)).c_str());

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void BindNullableId(StatementR stmt, int index, uint64_t const* id)
    {
    DbResult result = BE_SQLITE_ERROR;
    if (nullptr != id)
        result = stmt.BindUInt64(index, *id);
    else
        result = stmt.BindNull(index);
    BeAssert(BE_SQLITE_OK == result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheEmptyHierarchyLevel(HierarchyLevelInfo& info)
    {
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_HierarchyLevels "] ("
                       "[ConnectionId], [RulesetId], [Locale], [PhysicalParentNodeId], [VirtualParentNodeId]"
                       ") VALUES (?, ?, ?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    stmt->BindText(1, info.GetConnectionId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, info.GetLocale().c_str(), Statement::MakeCopy::No);
    BindNullableId(*stmt, 4, info.GetPhysicalParentNodeId());
    BindNullableId(*stmt, 5, info.GetVirtualParentNodeId());

    DbResult result = stmt->Step();
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return;
        }

    info.SetId((uint64_t)m_db.GetLastInsertRowId());
    LoggingHelper::LogMessage(Log::NavigationCache, GetHierarchyLevelDebugString("Cached empty", info).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheEmptyDataSource(DataSourceInfo& info, DataSourceFilter const& filter, bool disableUpdates)
    {
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_DataSources "] ("
                       "[HierarchyLevelId], [FullIndex], [Filter], [IsUpdatesDisabled]"
                       ") VALUES (?, ?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    int bindingIndex = 1;
    Utf8String filterStr = GetSerializedJson(filter.AsJson());

    stmt->BindUInt64(bindingIndex++, info.GetHierarchyLevelId());
#ifdef NAVNODES_CACHE_BINARY_INDEX
    stmt->BindBlob(bindingIndex++, reinterpret_cast<void const*>(&info.GetIndex().front()), info.GetIndex().size() * sizeof(uint64_t), Statement::MakeCopy::No);
#else
    Utf8String dsIndex = IndexToString(info.GetIndex(), true);
    stmt->BindText(bindingIndex++, dsIndex.c_str(), Statement::MakeCopy::No);
#endif
    stmt->BindText(bindingIndex++, filterStr.c_str(), Statement::MakeCopy::No);
    stmt->BindBoolean(bindingIndex++, disableUpdates);

    DbResult result = stmt->Step();
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return;
        }

    info.SetId((uint64_t)m_db.GetLastInsertRowId());
    LoggingHelper::LogMessage(Log::NavigationCache, GetDataSourceDebugString("Cached empty", info).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheRelatedClassIds(uint64_t datasourceId, bmap<ECClassId, bool> const& classIds)
    {
    Utf8CP deleteQuery = "DELETE FROM [" NODESCACHE_TABLENAME_DataSourceClasses "] WHERE [DataSourceId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), deleteQuery))
        {
        BeAssert(false);
        return;
        }
    stmt->BindUInt64(1, datasourceId);
    DbResult deleteResult = stmt->Step();
    BeAssert(BE_SQLITE_DONE == deleteResult);

    Utf8CP insertQuery = "INSERT INTO [" NODESCACHE_TABLENAME_DataSourceClasses "] ([DataSourceId], [ECClassId], [Polymorphic]) VALUES (?, ?, ?)";
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
        {
        BeAssert(false);
        return;
        }
    for (auto const& pair : classIds)
        {
        stmt->Reset();
        stmt->BindUInt64(1, datasourceId);
        stmt->BindUInt64(2, pair.first.GetValue());
        stmt->BindBoolean(3, pair.second);
        DbResult insertResult = stmt->Step();
        BeAssert(BE_SQLITE_DONE == insertResult);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheRelatedSettings(uint64_t datasourceId, bvector<UserSettingEntry> const& settings)
    {
    Utf8CP deleteQuery = "DELETE FROM [" NODESCACHE_TABLENAME_DataSourceSettings "] WHERE [DataSourceId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), deleteQuery))
        {
        BeAssert(false);
        return;
        }
    stmt->BindUInt64(1, datasourceId);
    DbResult deleteResult = stmt->Step();
    BeAssert(BE_SQLITE_DONE == deleteResult);

    Utf8CP insertQuery = "INSERT INTO [" NODESCACHE_TABLENAME_DataSourceSettings "] ([DataSourceId], [SettingId], [SettingValue]) VALUES (?, ?, ?)";
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), insertQuery))
        {
        BeAssert(false);
        return;
        }
    for (UserSettingEntry const& setting : settings)
        {
        stmt->Reset();
        stmt->BindUInt64(1, datasourceId);
        stmt->BindText(2, setting.GetId().c_str(), Statement::MakeCopy::No);
        Utf8String valueAsString = setting.GetValue().ToString();
        stmt->BindText(3, valueAsString, Statement::MakeCopy::No);
        DbResult insertResult = stmt->Step();
        BeAssert(BE_SQLITE_DONE == insertResult);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyLevelInfo NodesCache::_FindHierarchyLevel(Utf8CP connectionId, Utf8CP rulesetId, Utf8CP locale, uint64_t const* virtualParentNodeId) const
    {
    Utf8String query = "SELECT [hl].[Id], [hl].[PhysicalParentNodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       " WHERE [hl].[RemovalId] IS NULL AND [hl].[VirtualParentNodeId] ";
    if (nullptr == virtualParentNodeId || 0 == *virtualParentNodeId)
        query.append(" IS NULL");
    else
        query.append(" = ?");
    if (nullptr != connectionId)
        query.append(" AND [hl].[ConnectionId] = ?");
    if (nullptr != rulesetId)
        query.append(" AND [hl].[RulesetId] = ?");
    if (nullptr != locale)
        query.append(" AND [hl].[Locale] = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return HierarchyLevelInfo();
        }

    int bindingIndex = 1;

    if (nullptr != virtualParentNodeId && 0 != *virtualParentNodeId)
        stmt->BindInt64(bindingIndex++, *virtualParentNodeId);

    if (nullptr != connectionId)
        stmt->BindText(bindingIndex++, connectionId, Statement::MakeCopy::No);

    if (nullptr != rulesetId)
        stmt->BindText(bindingIndex++, rulesetId, Statement::MakeCopy::No);

    if (nullptr != locale)
        stmt->BindText(bindingIndex++, locale, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return HierarchyLevelInfo();

    return HierarchyLevelInfo(stmt->GetValueUInt64(0), connectionId, rulesetId, locale,
        stmt->GetValueUInt64(1), virtualParentNodeId ? *virtualParentNodeId : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyLevelInfo NodesCache::FindHierarchyLevel(uint64_t id) const
    {
    Utf8String query = "SELECT [hl].[Id], [hl].[ConnectionId], [hl].[RulesetId], [hl].[Locale], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       " WHERE [hl].[RemovalId] IS NULL AND [hl].[Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return HierarchyLevelInfo();
        }

    stmt->BindUInt64(1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        return HierarchyLevelInfo();

    uint64_t physicalParentNodeId = stmt->GetValueUInt64(4);
    uint64_t virtualParentNodeId = stmt->GetValueUInt64(5);
    return HierarchyLevelInfo(stmt->GetValueUInt64(0), stmt->GetValueText(1), stmt->GetValueText(2),
        stmt->GetValueText(3), physicalParentNodeId, virtualParentNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCache::_FindDataSource(uint64_t hierarchyLevelId, bvector<uint64_t> const& index) const
    {
    Utf8CP query = "SELECT [ds].[Id] "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
                   " WHERE [hl].[RemovalId] IS NULL AND [hl].[Id] = ? AND [ds].[FullIndex] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return DataSourceInfo();
        }

    stmt->BindUInt64(1, hierarchyLevelId);
#ifdef NAVNODES_CACHE_BINARY_INDEX
    stmt->BindBlob(2, reinterpret_cast<void const*>(&index.front()), index.size() * sizeof(uint64_t), Statement::MakeCopy::No);
#else
    Utf8String dsIndex = IndexToString(index, true);
    stmt->BindText(2, dsIndex.c_str(), Statement::MakeCopy::No);
#endif

    if (BE_SQLITE_ROW != stmt->Step())
        return DataSourceInfo();

    return DataSourceInfo(stmt->GetValueUInt64(0), hierarchyLevelId, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCache::_FindDataSource(uint64_t nodeId) const
    {
    Utf8CP query = "SELECT [ds].[Id], [ds].[HierarchyLevelId], [ds].[FullIndex] "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id] "
                   " WHERE [hl].[RemovalId] IS NULL AND [n].[Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return DataSourceInfo();
        }

    stmt->BindUInt64(1, nodeId);

    if (BE_SQLITE_ROW != stmt->Step())
        return DataSourceInfo();

    return DataSourceInfo(stmt->GetValueUInt64(0), stmt->GetValueUInt64(1),
#ifdef NAVNODES_CACHE_BINARY_INDEX
        IndexFromBlob(stmt->GetValueBlob(2), stmt->GetColumnBytes(2)));
#else
        IndexFromString(stmt->GetValueText(2)));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr NodesCache::_GetNode(uint64_t id) const
    {
    JsonNavNodePtr node = GetQuick(id);
    if (node.IsValid())
        return node;

    Utf8String query = "SELECT [hl].[ConnectionId], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Data], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                       "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                       " WHERE [n].[Id] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindUInt64(1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    node = CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections);
    AddQuick(*node);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NodeVisibility NodesCache::_GetNodeVisibility(uint64_t nodeId) const
    {
    Utf8CP query = "SELECT [n].[IsVirtual] "
                   "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                   " WHERE [n].[Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return NodeVisibility::Virtual;
        }

    stmt->BindUInt64(1, nodeId);

    if (BE_SQLITE_ROW != stmt->Step())
        {
        BeAssert(false);
        return NodeVisibility::Virtual;
        }

    return stmt->GetValueBoolean(0) ? NodeVisibility::Virtual : NodeVisibility::Physical;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::IsUpdatesDisabled(CombinedHierarchyLevelInfo const& info) const
    {
    Utf8String query = "SELECT 1 "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels"] hl "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       " WHERE     [ds].[IsUpdatesDisabled] AND [hl].[RemovalId] IS NULL "
                       "       AND [hl].[ConnectionId] = ? AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? "
                       "       AND [hl].[PhysicalParentNodeId] ";
    if (nullptr != info.GetPhysicalParentNodeId())
        query.append("= ?");
    else
        query.append("IS NULL");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, info.GetConnectionId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, info.GetLocale().c_str(), Statement::MakeCopy::No);
    if (nullptr != info.GetPhysicalParentNodeId())
        stmt->BindInt64(4, *info.GetPhysicalParentNodeId());

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::HasRelatedSettingsChanged(uint64_t datasourceId, Utf8StringCR rulesetId) const
    {
    Utf8CP query = "SELECT [dss].[SettingId], [dss].[SettingValue] "
                    "  FROM [" NODESCACHE_TABLENAME_DataSourceSettings "] dss "
                    " WHERE [dss].[DataSourceId] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return true;
        }

    stmt->BindUInt64(1, datasourceId);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8CP settingId = stmt->GetValueText(0);
        Utf8String cachedValue = stmt->GetValueText(1);
        if (cachedValue != m_userSettings.GetSettings(rulesetId).GetSettingValueAsJson(settingId).ToString())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DataSourceInfo> NodesCache::GetDataSourcesWithChangedUserSettings(CombinedHierarchyLevelInfo const& info) const
    {
    Utf8String query = "SELECT [ds].[Id], [ds].[HierarchyLevelId], [ds].[FullIndex], [dss].[SettingId], [dss].[SettingValue] "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSourceSettings "] dss ON [dss].[DataSourceId] = [ds].[Id] "
                       " WHERE     [hl].[ConnectionId] = ? AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? "
                       "       AND [hl].[PhysicalParentNodeId] ";
    query.append((nullptr != info.GetPhysicalParentNodeId()) ? "= ?" : "IS NULL");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return bvector<DataSourceInfo>();
        }

    stmt->BindText(1, info.GetConnectionId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, info.GetLocale().c_str(), Statement::MakeCopy::No);
    if (nullptr != info.GetPhysicalParentNodeId())
        stmt->BindUInt64(4, *info.GetPhysicalParentNodeId());

    bvector<DataSourceInfo> infos;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8CP settingId = stmt->GetValueText(3);
        Utf8String cachedValue = stmt->GetValueText(4);
        if (cachedValue != m_userSettings.GetSettings(info.GetRulesetId()).GetSettingValueAsJson(settingId).ToString())
            {
            infos.push_back(DataSourceInfo(stmt->GetValueUInt64(0), stmt->GetValueUInt64(1),
#ifdef NAVNODES_CACHE_BINARY_INDEX
                IndexFromBlob(stmt->GetValueBlob(2), stmt->GetColumnBytes(2))));
#else
                IndexFromString(stmt->GetValueText(2))));
#endif
            }
        }

    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DataSourceInfo> NodesCache::GetDataSourcesWithChangedUserSettings(HierarchyLevelInfo const& info) const
    {
    Utf8CP query = "SELECT [ds].[Id], [ds].[HierarchyLevelId], [ds].[FullIndex], [dss].[SettingId], [dss].[SettingValue] "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSourceSettings "] dss ON [dss].[DataSourceId] = [ds].[Id] "
                   " WHERE [ds].[HierarchyLevelId] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return bvector<DataSourceInfo>();
        }

    stmt->BindUInt64(1, info.GetId());

    bvector<DataSourceInfo> infos;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8CP settingId = stmt->GetValueText(3);
        Utf8String cachedValue = stmt->GetValueText(4);
        if (cachedValue != m_userSettings.GetSettings(info.GetRulesetId()).GetSettingValueAsJson(settingId).ToString())
            {
            infos.push_back(DataSourceInfo(stmt->GetValueUInt64(0), stmt->GetValueUInt64(1),
#ifdef NAVNODES_CACHE_BINARY_INDEX
                IndexFromBlob(stmt->GetValueBlob(2), stmt->GetColumnBytes(2))));
#else
                IndexFromString(stmt->GetValueText(2))));
#endif
            }
        }

    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo const& info, bool removeIfInvalid, bool onlyInitialized) const
    {
    NavNodesProviderPtr source = GetQuick(info);
    if (source.IsValid())
        return source;

    if (removeIfInvalid)
        {
        bvector<DataSourceInfo> changedDataSourceInfos = GetDataSourcesWithChangedUserSettings(info);
        for (DataSourceInfo const& changedDataSourceInfo : changedDataSourceInfos)
            const_cast<NodesCache*>(this)->ResetDataSource(changedDataSourceInfo);
        }

    // make sure it's fully initialized
    if (onlyInitialized && !IsInitialized(info))
        return nullptr;

    IConnectionCP connection = m_connections.GetConnection(info.GetConnectionId().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }

    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, info.GetRulesetId().c_str(), info.GetLocale().c_str(),
        info.GetPhysicalParentNodeId(), nullptr, IsUpdatesDisabled(info));
    return context.IsValid() ? CachedCombinedHierarchyLevelProvider::Create(*context, m_db, m_statements, info) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetHierarchyLevel(HierarchyLevelInfo const& info, bool removeIfInvalid, bool onlyInitialized) const
    {
    if (removeIfInvalid)
        {
        bvector<DataSourceInfo> changedDataSourceInfos = GetDataSourcesWithChangedUserSettings(info);
        for (DataSourceInfo const& changedDataSourceInfo : changedDataSourceInfos)
            const_cast<NodesCache*>(this)->ResetDataSource(changedDataSourceInfo);
        }

    // make sure it's fully initialized
    if (onlyInitialized && !IsInitialized(info))
        return nullptr;

    IConnectionCP connection = m_connections.GetConnection(info.GetConnectionId().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }

    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, info.GetRulesetId().c_str(), info.GetLocale().c_str(),
        info.GetPhysicalParentNodeId(), nullptr, IsUpdatesDisabled(info));
    if (info.GetVirtualParentNodeId())
        context->SetVirtualParentNodeId(*info.GetVirtualParentNodeId());
    return context.IsValid() ? CachedHierarchyLevelProvider::Create(*context, m_db, m_statements, info.GetId()) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetDataSource(DataSourceInfo const& dsInfo, bool removeIfInvalid, bool onlyInitialized) const
    {
    // make sure data source is initialized
    if (onlyInitialized && !IsInitialized(dsInfo))
        return nullptr;

    HierarchyLevelInfo hlInfo = FindHierarchyLevel(dsInfo.GetHierarchyLevelId());
    if (!hlInfo.IsValid())
        return nullptr;

    if (removeIfInvalid && HasRelatedSettingsChanged(dsInfo.GetId(), hlInfo.GetRulesetId()))
        {
        const_cast<NodesCache*>(this)->ResetDataSource(dsInfo);
        return nullptr;
        }

    IConnectionCP connection = m_connections.GetConnection(hlInfo.GetConnectionId().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }

    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, hlInfo.GetRulesetId().c_str(), hlInfo.GetLocale().c_str(),
        hlInfo.GetVirtualParentNodeId(), nullptr, IsUpdatesDisabled(hlInfo));
    if (hlInfo.GetVirtualParentNodeId())
        context->SetVirtualParentNodeId(*hlInfo.GetVirtualParentNodeId());
    return context.IsValid() ? CachedPartialDataSourceProvider::Create(*context, m_db, m_statements, dsInfo.GetId()) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetDataSource(uint64_t nodeId, bool removeIfInvalid, bool onlyInitialized) const
    {
    DataSourceInfo dsInfo = FindDataSource(nodeId);
    if (!dsInfo.IsValid() || onlyInitialized && !IsInitialized(dsInfo))
        return nullptr;

    return GetDataSource(dsInfo, removeIfInvalid, onlyInitialized);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Cache(JsonNavNodeR node, DataSourceInfo const& dsInfo, uint64_t index, bool isVirtual)
    {
    CacheNode(dsInfo, node, index, isVirtual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Cache(HierarchyLevelInfo& info)
    {
    CacheEmptyHierarchyLevel(info);

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Cache(DataSourceInfo& info, DataSourceFilter const& filter, bmap<ECClassId, bool> const& relatedClassIds, bvector<UserSettingEntry> const& relatedSettings,
    bool disableUpdates)
    {
    CacheEmptyDataSource(info, filter, disableUpdates);

    if (info.IsValid())
        {
        CacheRelatedClassIds(info.GetId(), relatedClassIds);
        CacheRelatedSettings(info.GetId(), relatedSettings);
        }

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheHierarchyLevel(CombinedHierarchyLevelInfo const& info, NavNodesProviderR provider)
    {
    if (provider.GetNodesCount() > NODESCACHE_QUICK_Boundary)
        AddQuick(info, provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Update(uint64_t nodeId, JsonNavNodeCR node)
    {
    Utf8String query = "UPDATE [" NODESCACHE_TABLENAME_Nodes "] "
                       "SET [Id] = ?, [Data] = ?, [Label] = ? "
                       "WHERE [Id] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    stmt->BindUInt64(1, node.GetNodeId());

    BeAssert(nullptr != dynamic_cast<JsonNavNodeCP>(&node));
    Utf8String nodeStr = GetSerializedJson(static_cast<JsonNavNodeCR>(node).GetJson());
    stmt->BindText(2, nodeStr.c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, node.GetLabel(), Statement::MakeCopy::Yes);

    stmt->BindUInt64(4, nodeId);

    if (BE_SQLITE_DONE == stmt->Step() && 1 == m_db.GetModifiedRowCount())
        {
        CacheNodeKey(node);
        CacheNodeInstanceKeys(node);
        SetIsExpanded(node.GetNodeId(), node.IsExpanded());
        }

    RemoveQuick(nodeId);

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Update(DataSourceInfo const& info, DataSourceFilter const* filter, bmap<ECClassId, bool> const* relatedClassIds, bvector<UserSettingEntry> const* relatedSettings)
    {
    if (nullptr != filter)
        {
        Utf8String query = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] SET [Filter] = ? "
                           " WHERE [Id] = ?";

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
            {
            BeAssert(false);
            return;
            }

        Utf8String filterStr = GetSerializedJson(filter->AsJson());
        stmt->BindText(1, filterStr.c_str(), Statement::MakeCopy::No);
        stmt->BindInt64(2, info.GetId());
        stmt->Step();
        }

    if (nullptr != relatedClassIds)
        CacheRelatedClassIds(info.GetId(), *relatedClassIds);

    if (nullptr != relatedSettings)
        CacheRelatedSettings(info.GetId(), *relatedSettings);

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemapNodeIds(bmap<uint64_t, uint64_t> const& remapInfo)
    {
    // statement to remap physical hierarchy levels
    static Utf8CP s_pdsQuery = "UPDATE [" NODESCACHE_TABLENAME_HierarchyLevels "] "
                               "   SET [PhysicalParentNodeId] = ? "
                               " WHERE [PhysicalParentNodeId] = ?";
    CachedStatementPtr pdsStatement;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(pdsStatement, *m_db.GetDbFile(), s_pdsQuery))
        {
        BeAssert(false);
        return;
        }

    // statement to remap virtual hierarchy levels
    static Utf8CP s_vdsQuery = "UPDATE [" NODESCACHE_TABLENAME_HierarchyLevels "] "
                               "   SET [VirtualParentNodeId] = ? "
                               " WHERE [VirtualParentNodeId] = ?";
    CachedStatementPtr vdsStatement;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(vdsStatement, *m_db.GetDbFile(), s_vdsQuery))
        {
        BeAssert(false);
        return;
        }

    // do remap
    for (auto pair : remapInfo)
        {
        pdsStatement->BindUInt64(1, pair.second);
        pdsStatement->BindUInt64(2, pair.first);
        pdsStatement->Step();
        pdsStatement->Reset();

        vdsStatement->BindUInt64(1, pair.second);
        vdsStatement->BindUInt64(2, pair.first);
        vdsStatement->Step();
        vdsStatement->Reset();
        }

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCache::CreateRemovalId(CombinedHierarchyLevelInfo const& info)
    {
    BeGuid removalId(true);
    Utf8String query = "UPDATE [" NODESCACHE_TABLENAME_HierarchyLevels "] "
                       "   SET [RemovalId] = ? "
                       " WHERE [ConnectionId] = ? AND [RulesetId] = ? AND [Locale] = ? AND [PhysicalParentNodeId] ";
    query.append(info.GetPhysicalParentNodeId() ? "= ?" : "IS NULL");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return BeGuid();
        }
    stmt->BindGuid(1, removalId);
    stmt->BindText(2, info.GetConnectionId(), Statement::MakeCopy::No);
    stmt->BindText(3, info.GetRulesetId(), Statement::MakeCopy::No);
    stmt->BindText(4, info.GetLocale(), Statement::MakeCopy::No);
    if (info.GetPhysicalParentNodeId())
        stmt->BindUInt64(5, *info.GetPhysicalParentNodeId());
    stmt->Step();

    RemoveQuick(info);
    m_quickNodesCache.clear();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif

    return removalId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveHierarchyLevel(BeGuidCR removalId)
    {
    Utf8String query = "DELETE FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] "
                       " WHERE [RemovalId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }
    stmt->BindGuid(1, removalId);
    stmt->Step();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::ResetDataSource(DataSourceInfo const& info)
    {
    Utf8CP query = "DELETE FROM [" NODESCACHE_TABLENAME_DataSources "] "
                   " WHERE [Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return;
        }
    stmt->BindUInt64(1, info.GetId());
    stmt->Step();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyLevelInfo NodesCache::GetParentHierarchyLevelInfo(uint64_t nodeId) const
    {
    Utf8String query = "SELECT [hl].[Id], [hl].[ConnectionId], [hl].[RulesetId], [hl].[Locale], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON ds.HierarchyLevelId = hl.Id "
                       "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON n.DataSourceId = ds.Id "
                       " WHERE n.Id = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return HierarchyLevelInfo();
        }

    stmt->BindUInt64(1, nodeId);

    if (BE_SQLITE_ROW != stmt->Step())
        return HierarchyLevelInfo();

    uint64_t physicalParentNodeId = stmt->GetValueUInt64(4);
    uint64_t virtualParentNodeId = stmt->GetValueUInt64(5);
    return HierarchyLevelInfo(stmt->GetValueUInt64(0), stmt->GetValueText(1), stmt->GetValueText(2),
        stmt->GetValueText(3), physicalParentNodeId, virtualParentNodeId);
    }

#define FLAT_HIERARCHY_TABLE_NAME "flat_hierarchy"
#define FLAT_HIERARCHY_COLUMN_NAME_NodeId "NodeId"
#define WITH_FLAT_HIERARCHY \
    "WITH RECURSIVE" \
    "    " FLAT_HIERARCHY_TABLE_NAME "(" FLAT_HIERARCHY_COLUMN_NAME_NodeId ") AS ( " \
    "        VALUES(?) " \
    "        UNION ALL " \
    "        SELECT n.Id " \
    "        FROM [" NODESCACHE_TABLENAME_Nodes "] as n " \
    "        JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] " \
    "        JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] as hl ON [hl].[Id] = [ds].[HierarchyLevelId] " \
    "        JOIN " FLAT_HIERARCHY_TABLE_NAME " as parent ON [hl].[VirtualParentNodeId] = [parent].[" FLAT_HIERARCHY_COLUMN_NAME_NodeId "] OR [parent].[" FLAT_HIERARCHY_COLUMN_NAME_NodeId "] IS NULL AND [hl].[VirtualParentNodeId] IS NULL " \
    "    )"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::ChangeVisibility(uint64_t nodeId, bool isVirtual, bool updateChildHierarchyLevels)
    {
    // first, make sure the node is not virtual
    Utf8String query = "UPDATE [" NODESCACHE_TABLENAME_Nodes "] "
                       "   SET [IsVirtual] = ? "
                       " WHERE [Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }
    stmt->BindBoolean(1, isVirtual);
    stmt->BindUInt64(2, nodeId);
    stmt->Step();

    Utf8String logMsg = "Converted to ";
    logMsg.append(isVirtual ? "virtual" : "physical");
    LoggingHelper::LogMessage(Log::NavigationCache, GetNodeDebugString(logMsg.c_str(), nodeId, -1).c_str());

    std::function<bool(JsonNavNodeCR)> removeQuickHandler;

    // then, remap its child hierarchy levels
    if (updateChildHierarchyLevels)
        {
        if (isVirtual)
            {
            query = "UPDATE [" NODESCACHE_TABLENAME_HierarchyLevels "] "
                    "SET [PhysicalParentNodeId] = ("
                    "    SELECT [parentHl].[PhysicalParentNodeId] "
                    "      FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] parentHl "
                    "      JOIN [" NODESCACHE_TABLENAME_DataSources "] parentDs ON [parentDs].[HierarchyLevelId] = [parentHl].[Id] "
                    "      JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [parentDs].[Id] "
                    "     WHERE [n].[Id] = [" NODESCACHE_TABLENAME_HierarchyLevels "].[PhysicalParentNodeId] "
                    ") "
                    "WHERE [PhysicalParentNodeId] = ?";
            if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
                {
                BeAssert(false);
                return;
                }
            stmt->BindUInt64(1, nodeId);
            stmt->Step();
            removeQuickHandler = [&](JsonNavNodeCR n)
                {
                return n.GetNodeId() == nodeId
                    || n.GetParentNodeId() == nodeId;
                };
            }
        else
            {
            HierarchyLevelInfo parentInfo = GetParentHierarchyLevelInfo(nodeId);
            query = WITH_FLAT_HIERARCHY
                    "UPDATE [" NODESCACHE_TABLENAME_HierarchyLevels "] "
                    "SET [PhysicalParentNodeId] = ? "
                    "WHERE [VirtualParentNodeId] IN ( "
                    "    SELECT " FLAT_HIERARCHY_COLUMN_NAME_NodeId " FROM " FLAT_HIERARCHY_TABLE_NAME
                    ") AND [PhysicalParentNodeId] ";
            query.append(parentInfo.GetPhysicalParentNodeId() ? "= ?" : "IS NULL");
            if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
                {
                BeAssert(false);
                return;
                }
            stmt->BindUInt64(1, nodeId); // for WITH_FLAT_HIERARCHY
            stmt->BindUInt64(2, nodeId);
            if (parentInfo.GetPhysicalParentNodeId())
                stmt->BindUInt64(3, *parentInfo.GetPhysicalParentNodeId());
            stmt->Step();
            removeQuickHandler = [&](JsonNavNodeCR n)
                {
                return n.GetNodeId() == nodeId
                    || n.GetParentNodeId() == (parentInfo.GetPhysicalParentNodeId() ? *parentInfo.GetPhysicalParentNodeId() : 0);
                };
            }
        LoggingHelper::LogMessage(Log::NavigationCache, Utf8PrintfString("    Affected child data sources: %d", m_db.GetModifiedRowCount()).c_str());
        }

    RemoveQuick(removeQuickHandler);

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_MakePhysical(JsonNavNodeCR node) {ChangeVisibility(node.GetNodeId(), false, true);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_MakeVirtual(JsonNavNodeCR node) {ChangeVisibility(node.GetNodeId(), true, true);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::_IsInitialized(CombinedHierarchyLevelInfo const& info) const
    {
    Utf8String query = "SELECT [ds].[IsInitialized] "
                       " FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       " LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       " WHERE     [hl].[ConnectionId] = ? AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? "
                       "       AND [hl].[PhysicalParentNodeId] ";
    query.append((nullptr != info.GetPhysicalParentNodeId()) ? "= ?" : "IS NULL");
    query.append(" GROUP BY [ds].[IsInitialized]");


    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, info.GetConnectionId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, info.GetLocale().c_str(), Statement::MakeCopy::No);
    if (nullptr != info.GetPhysicalParentNodeId())
        stmt->BindUInt64(4, *info.GetPhysicalParentNodeId());

    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    bool value = stmt->GetValueBoolean(0);
    return value && (BE_SQLITE_ROW != stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::_IsInitialized(HierarchyLevelInfo const& info) const
    {
    if (!info.IsValid())
        {
        BeAssert(false);
        return false;
        }

    Utf8CP query = "SELECT [ds].[IsInitialized] "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                   " WHERE [hl].[Id] = ?"
                   " GROUP BY [ds].[IsInitialized]";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindInt64(1, info.GetId());

    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    bool value = stmt->GetValueBoolean(0);
    return value && (BE_SQLITE_ROW != stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::_IsInitialized(DataSourceInfo const& info) const
    {
    if (!info.IsValid())
        {
        BeAssert(false);
        return false;
        }

    Utf8CP query = "SELECT 1 "
                   " FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   " WHERE [ds].[IsInitialized] AND [ds].[Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindInt64(1, info.GetId());

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_FinalizeInitialization(DataSourceInfo const& info)
    {
    if (0 == info.GetId())
        {
        BeAssert(false);
        return;
        }

    Utf8CP query = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] "
                   "   SET [IsInitialized] = true "
                   " WHERE [Id] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return;
        }

    stmt->BindInt64(1, info.GetId());
    DbResult result = stmt->Step();
    BeAssert(BE_SQLITE_DONE == result);

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::Clear(IConnectionCP connection, Utf8CP rulesetId)
    {
    Utf8String query = "DELETE FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] ";
    if (nullptr != connection)
        query.append("WHERE [ConnectionId] = ? ");
    if (nullptr != rulesetId)
        {
        query.append((nullptr == connection) ? "WHERE " : "AND ");
        query.append("[RulesetId] = ?");
        }

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    int boundVariablesCount = 0;
    if (nullptr != connection)
        stmt->BindText(++boundVariablesCount, connection->GetId().c_str(), Statement::MakeCopy::No);
    if (nullptr != rulesetId)
        stmt->BindText(++boundVariablesCount, rulesetId, Statement::MakeCopy::No);

    stmt->Step();
    m_quickDataSourceCache.clear();
    m_quickNodesCache.clear();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct ECInstanceKeySet : VirtualSet
{
private:
    bset<ECInstanceKey> const& m_keys;
public:
    ECInstanceKeySet(bset<ECInstanceKey> const& keys) : m_keys(keys) {}
    bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        BeAssert(2 == nVals);
        ECClassId classId = vals[0].GetValueId<ECClassId>();
        ECInstanceId instanceId = vals[1].GetValueId<ECInstanceId>();
        return (m_keys.end() != m_keys.find(ECInstanceKey(classId, instanceId)));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
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
    bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        ECInstanceId instanceId = vals[0].GetValueId<ECInstanceId>();
        return (m_ids.end() != m_ids.find(instanceId));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
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
        BeAssert(1 == nVals);
        ECClassId classId = vals[0].GetValueId<ECClassId>();
        return (m_ids.end() != m_ids.find(classId));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AreRelated(IConnectionCR connection, DataSourceFilter::RelatedInstanceInfo const& relationshipInfo, bset<ECInstanceKey> const& keys, ECSqlStatementCache& statements)
    {
    for (ECInstanceKeyCR key : keys)
        {
        if (key.GetInstanceId() == relationshipInfo.m_instanceId)
            return true;
        }

    Utf8String query;
    for (ECClassId relationshipClassId : relationshipInfo.m_relationshipClassIds)
        {
        ECRelationshipClassCP relationshipClass = connection.GetECDb().Schemas().GetClass(relationshipClassId)->GetRelationshipClassCP();
        if (nullptr == relationshipClass)
            {
            BeAssert(false);
            return false;
            }

        if (!query.empty())
            query.append(" UNION ALL ");

        query.append("SELECT 1 FROM ");
        query.append(relationshipClass->GetECSqlName());
        query.append(" WHERE ");

        switch (relationshipInfo.m_direction)
            {
            case RequiredRelationDirection_Forward:
                query.append("SourceECInstanceId = ? AND InVirtualSet(?, TargetECInstanceId)");
                break;
            case RequiredRelationDirection_Backward:
                query.append("TargetECInstanceId = ? AND InVirtualSet(?, SourceECInstanceId)");
                break;
            case RequiredRelationDirection_Both:
                query.append("SourceECInstanceId = ? AND InVirtualSet(?, TargetECInstanceId)");
                query.append(" OR ");
                query.append("TargetECInstanceId = ? AND InVirtualSet(?, SourceECInstanceId)");
                break;
            }
        }

    Savepoint txn(connection.GetDb(), "NodesCache/AreRelated");
    BeAssert(txn.IsActive());

    CachedECSqlStatementPtr stmt = statements.GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), query.c_str());
    if (!stmt.IsValid())
        {
        BeAssert(false);
        return false;
        }

    VirtualECInstanceIdSet idsSet(keys);
    int bindingIndex = 1;

    for (auto i = 0; i < relationshipInfo.m_relationshipClassIds.size(); ++i)
        {
        stmt->BindId(bindingIndex++, relationshipInfo.m_instanceId);
        stmt->BindVirtualSet(bindingIndex++, idsSet);

        if (RequiredRelationDirection_Both == relationshipInfo.m_direction)
            {
            stmt->BindId(bindingIndex++, relationshipInfo.m_instanceId);
            stmt->BindVirtualSet(bindingIndex++, idsSet);
            }
        }

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AnyKeyMatchesFilter(IConnectionCR connection, Utf8CP serializedFilter, bset<ECInstanceKey> const& keys, ECSqlStatementCache& statements)
    {
    rapidjson::Document json;
    json.Parse(serializedFilter);
    if (json.IsNull())
        return true; // no filter means a match

    DataSourceFilter filter(json);
    if (nullptr != filter.GetRelatedInstanceInfo())
        return AreRelated(connection, *filter.GetRelatedInstanceInfo(), keys, statements);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
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
* @bsimethod                                    Saulius.Skliutas                10/2017
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
* @bsimethod                                    Saulius.Skliutas                10/2017
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
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HierarchyLevelInfo> NodesCache::GetRelatedHierarchyLevels(Utf8StringCR connectionId, bset<ECInstanceKey> const& keys) const
    {
    IConnectionCP connection = m_connections.GetConnection(connectionId.c_str());
    if (nullptr == connection || !connection->IsOpen())
        {
        LoggingHelper::LogMessage(Log::NavigationCache, Utf8PrintfString("Requested related hierarchy levels from "
            "connection that is not tracked or open (%s). Returning empty list.", connectionId.c_str()).c_str(), NativeLogging::LOG_WARNING);
        return bvector<HierarchyLevelInfo>();
        }

    bvector<HierarchyLevelInfo> infos;

    Utf8CP query = "SELECT [hl].[Id], [ConnectionId], [RulesetId], [Locale], [PhysicalParentNodeId], [VirtualParentNodeId], [Filter], 1 AS Priority "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSourceClasses "] dsc ON [dsc].[DataSourceId] = [ds].[Id] "
                   " WHERE [hl].[ConnectionId] = ? "
                   "       AND NOT [ds].[IsUpdatesDisabled] "
                   "       AND ([Polymorphic] AND InVirtualSet(?, [dsc].[ECClassId])"
                   "            OR NOT [Polymorphic] AND InVirtualSet(?, [dsc].[ECClassId])) "
                   "UNION ALL "
                   "SELECT [hl].[Id], [ConnectionId], [RulesetId], [Locale], [PhysicalParentNodeId], [VirtualParentNodeId], [Filter], 2 AS Priority "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id] "
                   "  JOIN [" NODESCACHE_TABLENAME_AffectingInstances "] ai ON [ai].[NodeId] = [n].[Id] "
                   " WHERE [hl].[ConnectionId] = ? "
                   "       AND NOT [ds].[IsUpdatesDisabled] "
                   "       AND InVirtualSet(?, [ai].[ECClassId], [ai].[ECInstanceId]) ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return infos;
        }

    stmt->BindText(1, connectionId.c_str(), Statement::MakeCopy::No);

    // bind classes for polymorphic search
    bset<ECClassId> ids;
    AddBaseAndDerivedClasses(ids, connection->GetECDb(), keys);
    ECClassIdSet polymorphicIds(ids);
    stmt->BindVirtualSet(2, polymorphicIds);

    // bind classes for nonpolymorphic search
    ECClassIdSet nonPolymorphicIds(keys);
    stmt->BindVirtualSet(3, nonPolymorphicIds);

    stmt->BindText(4, connectionId.c_str(), Statement::MakeCopy::No);

    ECInstanceKeySet instanceKeysVirtualSet(keys);
    stmt->BindVirtualSet(5, instanceKeysVirtualSet);

    bset<uint64_t> uniqueHierarchyLevelIds;
    ECSqlStatementCache& ecsqlStatements = m_ecsqlStamementCache.GetECSqlStatementCache(*connection);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8CP filter = stmt->GetValueText(6);
        int priority = stmt->GetValueInt(7);
        if (priority < 2 && nullptr != filter && 0 != *filter && !AnyKeyMatchesFilter(*connection, filter, keys, ecsqlStatements))
            continue;

        uint64_t hierarchyLevelId = stmt->GetValueUInt64(0);
        if (uniqueHierarchyLevelIds.find(hierarchyLevelId) != uniqueHierarchyLevelIds.end())
            continue;

        Utf8CP connectionId = stmt->GetValueText(1);
        Utf8CP rulesetId = stmt->GetValueText(2);
        Utf8CP locale = stmt->GetValueText(3);
        uint64_t physicalParentNodeId = stmt->GetValueUInt64(4);
        uint64_t virtualParentNodeId = stmt->GetValueUInt64(5);
        infos.push_back(HierarchyLevelInfo(hierarchyLevelId, connectionId, rulesetId, locale, physicalParentNodeId, virtualParentNodeId));
        uniqueHierarchyLevelIds.insert(hierarchyLevelId);
        }

    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HierarchyLevelInfo> NodesCache::GetRelatedHierarchyLevels(Utf8CP rulesetId, Utf8CP settingId) const
    {
    bvector<HierarchyLevelInfo> infos;
    Utf8CP query = "SELECT DISTINCT [hl].[Id], [ConnectionId], [Locale], [PhysicalParentNodeId], [VirtualParentNodeId] "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSourceSettings "] dss ON [dss].[DataSourceId] = [ds].[Id] "
                   " WHERE [hl].[RulesetId] = ? AND [dss].[SettingId] = ? AND NOT [ds].[IsUpdatesDisabled] ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return infos;
        }

    stmt->BindText(1, rulesetId, Statement::MakeCopy::No);
    stmt->BindText(2, settingId, Statement::MakeCopy::No);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        infos.push_back(HierarchyLevelInfo(stmt->GetValueUInt64(0), stmt->GetValueText(1), rulesetId,
            stmt->GetValueText(2), stmt->GetValueUInt64(3), stmt->GetValueUInt64(4)));
        }

    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::HasParentNode(uint64_t nodeId, bset<uint64_t> const& parentNodeIds) const
    {
    if (parentNodeIds.empty())
        return false;

    Utf8CP query = "SELECT [hl].[VirtualParentNodeId] "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id] "
                   " WHERE [n].[Id] = ?";

    DbResult result = BE_SQLITE_ROW;
    while (BE_SQLITE_ROW == result)
        {
        if (parentNodeIds.end() != parentNodeIds.find(nodeId))
            return true;

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
            {
            BeAssert(false);
            return false;
            }
        stmt->BindUInt64(1, nodeId);
        if (BE_SQLITE_ROW == (result = stmt->Step()))
            nodeId = stmt->GetValueUInt64(0);
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodeCPtr NodesCache::_LocateNode(IConnectionCR connection, Utf8StringCR locale, NavNodeKeyCR nodeKey) const
    {
    Utf8String query = "SELECT [hl].[ConnectionId], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Data], [n].[Id], [ex].[NodeId], [k].[PathFromRoot] "
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id]"
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
        " WHERE [n].[UniqueHash] = ? AND [hl].[ConnectionId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, nodeKey.GetHash(), Statement::MakeCopy::No);
    stmt->BindText(2, connection.GetId(), Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    JsonNavNodeCPtr node = CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections);
    BeAssert(BE_SQLITE_DONE == stmt->Step());
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::AddQuick(CombinedHierarchyLevelInfo info, NavNodesProviderR provider)
    {
    RemoveQuick(info);
    m_quickDataSourceCache.push_back(bpair<CombinedHierarchyLevelInfo, NavNodesProviderPtr>(info, &provider));
    if (m_quickDataSourceCache.size() > NODESCACHE_QUICK_Size)
        m_quickDataSourceCache.erase(m_quickDataSourceCache.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveQuick(CombinedHierarchyLevelInfo const& info)
    {
    for (size_t i = 0; i < m_quickDataSourceCache.size(); ++i)
        {
        CombinedHierarchyLevelInfo const& lhs = m_quickDataSourceCache[i].first;
        if (lhs == info)
            {
            m_quickDataSourceCache.erase(m_quickDataSourceCache.begin() + i);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::GetQuick(CombinedHierarchyLevelInfo const& info) const
    {
    for (size_t i = 0; i < m_quickDataSourceCache.size(); ++i)
        {
        CombinedHierarchyLevelInfo const& lhs = m_quickDataSourceCache[i].first;
        if (lhs == info)
            {
            NavNodesProviderPtr ds = m_quickDataSourceCache[i].second;
            m_quickDataSourceCache.erase(m_quickDataSourceCache.begin() + i);
            m_quickDataSourceCache.push_back(bpair<CombinedHierarchyLevelInfo, NavNodesProviderPtr>(info, ds));
            return ds;
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::AddQuick(JsonNavNodeR node) const
    {
    RemoveQuick(node.GetNodeId());
    m_quickNodesCache.push_back(bpair<uint64_t, JsonNavNodePtr>(node.GetNodeId(), &node));
    if (m_quickNodesCache.size() > NODESCACHE_QUICK_Size)
        m_quickNodesCache.erase(m_quickNodesCache.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveQuick(uint64_t id) const
    {
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
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveQuick(std::function<bool(JsonNavNodeCR)> const& pred) const
    {
    bvector<bpair<uint64_t, JsonNavNodePtr>> processed;
    for (auto iter = m_quickNodesCache.begin(); iter != m_quickNodesCache.end(); ++iter)
        {
        if (!pred(*iter->second))
            processed.push_back(*iter);
        }
    m_quickNodesCache.swap(processed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr NodesCache::GetQuick(uint64_t id) const
    {
    for (size_t i = 0; i < m_quickNodesCache.size(); ++i)
        {
        uint64_t cachedId = m_quickNodesCache[i].first;
        if (id == cachedId)
            {
            JsonNavNodePtr node = m_quickNodesCache[i].second;
            m_quickNodesCache.erase(m_quickNodesCache.begin() + i);
            m_quickNodesCache.push_back(bpair<uint64_t, JsonNavNodePtr>(id, node));
            return node;
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::Persist() {m_db.SaveChanges();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::ResetExpandedNodes(Utf8CP connectionId, Utf8CP rulesetId)
    {
    Utf8String query = "  WITH DataSet AS "
                                 "(SELECT [n].[Id] "
                                 "   FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                                 "   JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                                 "   JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
                                 "  WHERE [hl].[ConnectionId] = ? AND [hl].[RulesetId] = ?)"
                       "DELETE FROM [" NODESCACHE_TABLENAME_ExpandedNodes "]"
                       " WHERE [NodeId] IN [DataSet] ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    stmt->BindText(1, connectionId, Statement::MakeCopy::No);
    stmt->BindText(2, rulesetId, Statement::MakeCopy::No);
    stmt->Step();

    m_quickNodesCache.clear();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::SetIsExpanded(uint64_t nodeId, bool isExpanded) const
    {
    Utf8CP query;
    if (isExpanded)
        query = "INSERT INTO [" NODESCACHE_TABLENAME_ExpandedNodes "] ([NodeId]) VALUES (?) ";
    else
        query = "DELETE FROM [" NODESCACHE_TABLENAME_ExpandedNodes "] WHERE [NodeId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return;
        }

    stmt->BindUInt64(1, nodeId);
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::GetUndeterminedNodesProvider(IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale, bool isUpdatesDisabled) const
    {
    NavNodesProviderContextPtr context = m_contextFactory.Create(connection, rulesetId, locale, nullptr, nullptr, isUpdatesDisabled);
    return context.IsValid() ? NodesWithUndeterminedChildrenProvider::Create(*context, m_db, m_statements) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::GetFilteredNodesProvider(Utf8CP filter, IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale) const
    {
    NavNodesProviderContextPtr context = m_contextFactory.Create(connection, rulesetId, locale, nullptr, nullptr);
    return context.IsValid() ? FilteredNodesProvider::Create(*context, m_db, m_statements, filter) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t GetCacheFileSize(Utf8CP fileName)
    {
    BeFileName dbFile(fileName);
    uint64_t size = 0;
    if (BeFileNameStatus::Success != dbFile.GetFileSize(size))
        {
        BeAssert(false);
        return 0;
        }
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult QueryOldestConnection(BeSQLite::Db& db, Utf8StringR connectionId)
    {
    Utf8CP query = "SELECT [ConnectionId] FROM [" NODESCACHE_TABLENAME_Connections "] ORDER BY [LastUsedTime] ASC";
    Statement stmt(db, query);
    BeAssert(stmt.IsPrepared());
    DbResult result = stmt.Step();
    if (BE_SQLITE_ROW == result)
        connectionId = stmt.GetValueText(0);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::LimitCacheSize()
    {
    m_db.SaveChanges();
    if (0 == m_sizeLimit || GetCacheFileSize(m_db.GetDbFileName()) <= m_sizeLimit)
        return;

    m_db.GetDefaultTransaction()->Commit();
    m_db.TryExecuteSql("PRAGMA synchronous=0");
    m_db.GetDefaultTransaction()->Begin();

    Utf8String connectionId;
    while (GetCacheFileSize(m_db.GetDbFileName()) > m_sizeLimit && BE_SQLITE_ROW == QueryOldestConnection(m_db, connectionId))
        {
        Utf8CP deleteQuery = "DELETE FROM [" NODESCACHE_TABLENAME_Connections "] WHERE [ConnectionId] = ?";
        Statement deleteStmt(m_db, deleteQuery);
        BeAssert(deleteStmt.IsPrepared());
        deleteStmt.BindText(1, connectionId, Statement::MakeCopy::No);
        deleteStmt.Step();
        deleteStmt.Finalize();
        m_db.GetDefaultTransaction()->Commit();
        DbResult vacuumResult = m_db.TryExecuteSql("VACUUM");
        BeAssert(BE_SQLITE_OK == vacuumResult);
        m_db.GetDefaultTransaction()->Begin();
        }
    }
