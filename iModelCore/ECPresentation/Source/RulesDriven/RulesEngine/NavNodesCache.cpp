/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/NavNodesCache.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

#define NAVNODES_CACHE_DB_NAME          L"HierarchyCache.db"
#define NAVNODES_CACHE_DB_VERSION_MAJOR 9
#define NAVNODES_CACHE_DB_VERSION_MINOR 0
//#define NAVNODES_CACHE_DEBUG

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

    NavNodeExtendedData extendedData(*node);
    if (!stmt.IsColumnNull(2))
        extendedData.SetVirtualParentId(stmt.GetValueUInt64(2));

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
        DefaultTxn txnLockType = DefaultTxn::Yes;
#else
        DefaultTxn txnLockType = DefaultTxn::Exclusive;
#endif

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

        if (BE_SQLITE_OK != result && BE_SQLITE_BUSY != result)
            {
            result = m_db.CreateNewDb(path, BeGuid(), Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8,
                true, txnLockType, new NavigationCacheBusyRetry()));

            // save the cache version
            static BeVersion s_cacheVersion(NAVNODES_CACHE_DB_VERSION_MAJOR, NAVNODES_CACHE_DB_VERSION_MINOR);
            m_db.SaveProperty(s_versionPropertySpec, s_cacheVersion.ToString(), nullptr, 0);
            }

        if (BE_SQLITE_BUSY == result)
            {
            path.SetName(tempDirectory);
            Utf8PrintfString fileName("HierarchyCache.%s.db", BeGuid(true).ToString().c_str());
            path.AppendUtf8(fileName.c_str());

            result = m_db.CreateNewDb(path, BeGuid(), Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8, true,
                txnLockType, new NavigationCacheBusyRetry()));
            m_tempCache = true;
            }
        }

    if (BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return;
        }
    
    // create the tables
    if (!m_db.TableExists(NODESCACHE_TABLENAME_DataSources))
        {
        Utf8CP ddl = "[Id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE, "
                     "[PhysicalParentNodeId] INTEGER REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[VirtualParentNodeId] INTEGER REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[ConnectionId] TEXT NOT NULL REFERENCES " NODESCACHE_TABLENAME_Connections "([ConnectionId]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[RulesetId] TEXT NOT NULL REFERENCES " NODESCACHE_TABLENAME_Rulesets "([RulesetId]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[Locale] TEXT, "
                     "[Filter] TEXT, "
                     "[RemovalId] GUID, "
                     "[IsUpdatesDisabled] BOOLEAN NOT NULL DEFAULT FALSE ";
        m_db.CreateTable(NODESCACHE_TABLENAME_DataSources, ddl);
        m_db.ExecuteSql("CREATE UNIQUE INDEX [UX_DataSources_Virtual] ON [" NODESCACHE_TABLENAME_DataSources "](COALESCE([VirtualParentNodeId], 0),[ConnectionId],[RulesetId],[Locale],COALESCE([RemovalId], 0))");
        m_db.ExecuteSql("CREATE INDEX [IX_DataSources_Physical] ON [" NODESCACHE_TABLENAME_DataSources "]([PhysicalParentNodeId],[ConnectionId],[RulesetId],[Locale])");
        m_db.ExecuteSql("CREATE INDEX [IX_DataSources_Virtual] ON [" NODESCACHE_TABLENAME_DataSources "]([VirtualParentNodeId],[ConnectionId],[RulesetId],[Locale])");
        m_db.ExecuteSql("CREATE INDEX [IX_DataSources_RemovalId] ON [" NODESCACHE_TABLENAME_DataSources "]([RemovalId])");
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
                     "[IsVirtual] BOOLEAN NOT NULL DEFAULT FALSE, "
                     "[Data] TEXT NOT NULL, "
                     "[Label] TEXT, "
                     "[UniqueHash] TEXT NOT NULL";
        m_db.CreateTable(NODESCACHE_TABLENAME_Nodes, ddl);
        m_db.ExecuteSql("CREATE INDEX [IX_Nodes_DataSource] ON [" NODESCACHE_TABLENAME_Nodes "]([DataSourceId])");
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
    m_db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache::NodesCache(BeFileNameCR tempDirectory, JsonNavNodesFactoryCR nodesFactory, INodesProviderContextFactoryCR contextFactory, IConnectionManagerCR connections,
    IUserSettingsManager const& userSettings, IECSqlStatementCacheProvider& ecsqlStatements, NodesCacheType type)
    : m_nodesFactory(nodesFactory), m_contextFactory(contextFactory), m_connections(connections), m_userSettings(userSettings), 
    m_statements(50), m_type(type), m_tempCache(false), m_ecsqlStamementCache(ecsqlStatements), m_sizeLimit(0)
    {
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
struct SqliteSavepoint : IHierarchyCache::Savepoint
    {
    BeSQLite::Savepoint m_sqliteSavepoint;
    SqliteSavepoint(BeSQLite::Db& db) 
        : m_sqliteSavepoint(db, BeGuid(true).ToString().c_str())
        {
        BeAssert(m_sqliteSavepoint.IsActive());
        }
    void _Cancel() override {m_sqliteSavepoint.Cancel();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IHierarchyCache::SavepointPtr NodesCache::_CreateSavepoint() {return new SqliteSavepoint(m_db);}

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
bool NodesCache::IsDataSourceCached(Utf8StringCR connectionId, Utf8CP rulesetId, Utf8CP locale) const
    {
    Utf8String query = "SELECT 1 "
                       "  FROM [" NODESCACHE_TABLENAME_DataSources "] "
                       " WHERE [ConnectionId] = ? AND [RulesetId] = ? AND [Locale] = ?";

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
bool NodesCache::IsDataSourceCached(uint64_t parentNodeId) const
    {
    Utf8String query = "SELECT 1 FROM [" NODESCACHE_TABLENAME_DataSources "] WHERE [VirtualParentNodeId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindInt64(1, parentNodeId);

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
static Utf8String GetNodeDebugString(Utf8CP action, NavNodeCR node, NodeVisibility visibility)
    {
    Utf8String str = action;
    switch (visibility)
        {
        case NodeVisibility::Physical:
            str.append(" physical node {");
            break;
        case NodeVisibility::Virtual:
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
static Utf8String GetNodeDebugString(Utf8CP action, uint64_t nodeId, NodeVisibility visibility)
    {
    Utf8String str = action;
    switch (visibility)
        {
        case NodeVisibility::Physical:
            str.append(" physical node {");
            break;
        case NodeVisibility::Virtual:
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
    if (nullptr != info.GetPhysicalParentNodeId())
        {
        str.append(" child data source {");
        str.append("Id: ").append(std::to_string(info.GetDataSourceId()).c_str()).append(", ");
        str.append("PhysicalParentNodeId: ").append(std::to_string(*info.GetPhysicalParentNodeId()).c_str()).append(", ");
        str.append("VirtualParentNodeId: ").append(std::to_string(*info.GetVirtualParentNodeId()).c_str());
        str.append("}");
        }
    else if (!info.GetConnectionId().empty() && !info.GetRulesetId().empty())
        {
        str.append(" root data source {");
        str.append("Id: ").append(std::to_string(info.GetDataSourceId()).c_str()).append(", ");
        str.append("ConnectionId: ").append(info.GetConnectionId()).append(", ");
        str.append("RulesetId: '").append(info.GetRulesetId()).append("'");
        if (nullptr != info.GetVirtualParentNodeId())
            str.append(", VirtualParentNodeId: ").append(std::to_string(*info.GetVirtualParentNodeId()).c_str());
        str.append("}");
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String ComputeNodeHash(JsonNavNodeR node, IHierarchyCacheCR cache, IConnectionCR connection)
    {
    MD5 h;
    NavNodeExtendedData extendedData(node);
    Utf8String type = node.GetType();
    Utf8String dbGuid = connection.GetDb().GetDbGuid().ToString();
    Utf8CP specHash = extendedData.GetSpecificationHash();
    h.Add(type.c_str(), type.SizeInBytes());
    h.Add(specHash, strlen(specHash));
    h.Add(dbGuid.c_str(), dbGuid.SizeInBytes());

    if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type.c_str()))
        {
        uint64_t instanceId = node.GetInstanceId();
        uint64_t classId = extendedData.GetECClassId().GetValueUnchecked();
        h.Add(&instanceId, sizeof(instanceId));
        h.Add(&classId, sizeof(instanceId));
        }
    else if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type.c_str()))
        {
        uint64_t classId = extendedData.GetECClassId().GetValueUnchecked();
        h.Add(&classId, sizeof(classId));
        }
    else if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type.c_str()))
        {
        uint64_t classId = extendedData.GetECClassId().GetValueUnchecked();
        Utf8CP propertyName = extendedData.GetPropertyName();
        int rangeIndex = extendedData.GetPropertyValueRangeIndex();
        if (extendedData.HasPropertyValue())
            {
            rapidjson::Value const* propertyValue = extendedData.GetPropertyValue();
            Utf8String valueString = BeRapidJsonUtilities::ToString(*propertyValue);
            h.Add(valueString.c_str(), valueString.SizeInBytes());
            }
        h.Add(&classId, sizeof(classId));
        h.Add(propertyName, strlen(propertyName));
        h.Add(&rangeIndex, sizeof(rangeIndex));
        }
    else
        {
        // CustomNode and DisplayLabelGroupingNode
        Utf8String nodeLabel = node.GetLabel();
        h.Add(nodeLabel.c_str(), nodeLabel.SizeInBytes());
        }

    // create path from root to this node
    NavNodeCPtr parent = cache.GetNode(extendedData.GetVirtualParentId());
    bvector<Utf8String> parentPath = parent.IsValid() ? parent->GetKey()->GetPathFromRoot() : bvector<Utf8String>();
    Utf8String nodeHash = h.GetHashString();
    parentPath.push_back(nodeHash);
    node.SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, node, parentPath));

    // compute unique hash for this node
    h.Reset();
    for (Utf8StringCR pathElement : parentPath)
        h.Add(pathElement.c_str(), pathElement.SizeInBytes());
    return h.GetHashString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheNode(DataSourceInfo const& datasourceInfo, NavNodeR node, bool isVirtual)
    {
    BeAssert(node.GetNodeId() == 0);

    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_Nodes "] ([DataSourceId],[IsVirtual],[Data],[UniqueHash],[Label])"
                       "VALUES (?, ?, ?, ?, ?)";
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }
    
    stmt->BindUInt64(1, datasourceInfo.GetDataSourceId());
    stmt->BindBoolean(2, isVirtual);

    BeAssert(nullptr != dynamic_cast<JsonNavNodeP>(&node));
    JsonNavNodeR jsonNode = static_cast<JsonNavNodeR>(node);

    IConnectionPtr nodeConnection = m_connections.GetConnection(NavNodeExtendedData(jsonNode).GetConnectionId());
    BeAssert(nodeConnection.IsValid());
    Utf8String hash = ComputeNodeHash(jsonNode, *this, *nodeConnection);

    Utf8String nodeStr = GetSerializedJson(jsonNode.GetJson());
    stmt->BindText(3, nodeStr.c_str(), Statement::MakeCopy::No);
    stmt->BindText(4, hash.c_str(), Statement::MakeCopy::No);
    stmt->BindText(5, node.GetLabel(), Statement::MakeCopy::Yes);
        
    DbResult result = stmt->Step();
    BeAssert(BE_SQLITE_DONE == result);

    jsonNode.SetNodeId((uint64_t)m_db.GetLastInsertRowId());

    CacheNodeKey(node);
    CacheNodeInstanceKeys(node);

    if (node.IsExpanded())
        SetIsExpanded(node.GetNodeId(), node.IsExpanded());

    LoggingHelper::LogMessage(Log::NavigationCache, GetNodeDebugString("Cached", node, isVirtual ? NodeVisibility::Virtual : NodeVisibility::Physical).c_str());

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
void NodesCache::CacheEmptyDataSource(DataSourceInfo& info, DataSourceFilter const& filter, bool disableUpdates)
    {
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_DataSources "] ([ConnectionId], [RulesetId], [Locale], [PhysicalParentNodeId], [VirtualParentNodeId], [Filter], [IsUpdatesDisabled]) "
                       "VALUES (?, ?, ?, ?, ?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    Utf8String filterStr = GetSerializedJson(filter.AsJson());

    stmt->BindText(1, info.GetConnectionId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, info.GetLocale().c_str(), Statement::MakeCopy::No);
    BindNullableId(*stmt, 4, info.GetPhysicalParentNodeId());
    BindNullableId(*stmt, 5, info.GetVirtualParentNodeId());
    stmt->BindText(6, filterStr.c_str(), Statement::MakeCopy::No);
    stmt->BindBoolean(7, disableUpdates);

    DbResult result = stmt->Step();
    if (BE_SQLITE_DONE == result)
        {
        info.SetDataSourceId((uint64_t)m_db.GetLastInsertRowId());
        LoggingHelper::LogMessage(Log::NavigationCache, GetDataSourceDebugString("Cached empty", info).c_str());
        return;
        }

    LoggingHelper::LogMessage(Log::NavigationCache, GetDataSourceDebugString("Already cached", info).c_str());
    DataSourceInfo existing = GetDataSourceInfo(&info.GetConnectionId(), info.GetRulesetId().c_str(), 
        info.GetLocale().c_str(), info.GetVirtualParentNodeId(), true);
    BeAssert(existing.IsValid());
    info.SetDataSourceId(existing.GetDataSourceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheRelatedClassIds(uint64_t datasourceId, bmap<ECClassId, bool> const& classIds)
    {
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_DataSourceClasses "] ([DataSourceId], [ECClassId], [Polymorphic]) VALUES (?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
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
        stmt->Step();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheRelatedSettings(uint64_t datasourceId, bvector<UserSettingEntry> const& settings)
    {
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_DataSourceSettings "] ([DataSourceId], [SettingId], [SettingValue]) VALUES (?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
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
        stmt->Step();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCache::GetDataSourceInfo(Utf8StringCP connectionId, Utf8CP rulesetId, Utf8CP locale, uint64_t const* parentNodeId, bool isVirtual) const
    {
    Utf8String query = "SELECT [ds].[Id], [ds].[ConnectionId], [ds].[RulesetId], [ds].[Locale], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                       " WHERE [ds].[RemovalId] IS NULL AND ";
    query.append(isVirtual ? "[ds].[VirtualParentNodeId]" : "[ds].[PhysicalParentNodeId]");
    if (nullptr == parentNodeId || 0 == *parentNodeId)
        query.append(" IS NULL");
    else
        query.append(" = ?");
    if (nullptr != connectionId)
        query.append(" AND [ds].[ConnectionId] = ?");
    if (nullptr != rulesetId)
        query.append(" AND [ds].[RulesetId] = ?");
    if (nullptr != locale)
        query.append(" AND [ds].[Locale] = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return DataSourceInfo();
        }

    int bindingIndex = 1;

    if (nullptr != parentNodeId && 0 != *parentNodeId)
        stmt->BindInt64(bindingIndex++, *parentNodeId);

    if (nullptr != connectionId)
        stmt->BindText(bindingIndex++, connectionId->c_str(), Statement::MakeCopy::No);
        
    if (nullptr != rulesetId)
        stmt->BindText(bindingIndex++, rulesetId, Statement::MakeCopy::No);
    
    if (nullptr != locale)
        stmt->BindText(bindingIndex++, locale, Statement::MakeCopy::No);
            
    if (BE_SQLITE_ROW != stmt->Step())
        return DataSourceInfo();

    uint64_t physicalParentNodeId = stmt->GetValueUInt64(3);
    uint64_t virtualParentNodeId = stmt->GetValueUInt64(4);
    return DataSourceInfo(stmt->GetValueUInt64(0), stmt->GetValueText(1), stmt->GetValueText(2), stmt->GetValueText(3), 
        stmt->IsColumnNull(4) ? nullptr : &physicalParentNodeId, 
        stmt->IsColumnNull(5) ? nullptr : &virtualParentNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCache::GetDataSourceInfo(uint64_t nodeId) const
    {
    Utf8CP query = "SELECT [ds].[Id], [ds].[ConnectionId], [ds].[RulesetId], [ds].[Locale], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId] "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id] "
                   " WHERE [ds].[RemovalId] IS NULL AND [n].[Id] = ?";
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return DataSourceInfo();
        }

    stmt->BindUInt64(1, nodeId);

    if (BE_SQLITE_ROW != stmt->Step())
        return DataSourceInfo();

    uint64_t physicalParentNodeId = stmt->GetValueUInt64(3);
    uint64_t virtualParentNodeId = stmt->GetValueUInt64(4);
    return DataSourceInfo(stmt->GetValueUInt64(0), stmt->GetValueText(1), stmt->GetValueText(2), stmt->GetValueText(3), 
        stmt->IsColumnNull(4) ? nullptr : &physicalParentNodeId, 
        stmt->IsColumnNull(5) ? nullptr : &virtualParentNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr NodesCache::_GetNode(uint64_t id, NodeVisibility visibility) const
    {
    JsonNavNodePtr node = GetQuick(id);
    if (node.IsValid() && NodeVisibility::Any == visibility)
        return node;

    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                       " WHERE [n].[Id] = ? ";
    if (NodeVisibility::Virtual == visibility)
        query.append("AND [n].[IsVirtual]");
    else if (NodeVisibility::Physical == visibility)
        query.append("AND NOT [n].[IsVirtual]");

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
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::IsUpdatesDisabled(HierarchyLevelInfo const& info) const
    {
    Utf8String query = "SELECT [ds].[IsUpdatesDisabled] "
                       "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                       " WHERE [ds].[RemovalId] IS NULL AND [ds].[ConnectionId] = ? AND [ds].[RulesetId] = ? AND [Locale] = ? AND [ds].[PhysicalParentNodeId] "; 
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
            
    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    return stmt->GetValueBoolean(0);
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
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetDataSource(HierarchyLevelInfo const& info, bool removeIfInvalid) const
    {
    NavNodesProviderPtr source = GetQuick(info);
    if (source.IsValid())
        return source;
    
    // Check if datasource exists
    DataSourceInfo dsInfo = GetDataSourceInfo(&info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str(), info.GetPhysicalParentNodeId(), true);
    if (!dsInfo.IsValid())
        return nullptr;

    if (removeIfInvalid && HasRelatedSettingsChanged(dsInfo.GetDataSourceId(), dsInfo.GetRulesetId()))
        {
        const_cast<NodesCache*>(this)->RemoveDataSource(dsInfo.GetDataSourceId());
        return nullptr;
        }

    IConnectionCP connection = m_connections.GetConnection(info.GetConnectionId().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }

    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, info.GetRulesetId().c_str(), info.GetLocale().c_str(),
        info.GetPhysicalParentNodeId(), nullptr, IsUpdatesDisabled(info));
    return context.IsValid() ? CachedHierarchyLevelProvider::Create(*context, m_db, m_statements, info.GetPhysicalParentNodeId()) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetDataSource(DataSourceInfo const& info, bool removeIfInvalid) const
    {
    // Check if datasource exists
    DataSourceInfo dsInfo = GetDataSourceInfo(&info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str(), info.GetVirtualParentNodeId(), true);
    if (!dsInfo.IsValid())
        return nullptr;

    if (removeIfInvalid && HasRelatedSettingsChanged(dsInfo.GetDataSourceId(), dsInfo.GetRulesetId()))
        {
        const_cast<NodesCache*>(this)->RemoveDataSource(dsInfo.GetDataSourceId());
        return nullptr;
        }

    IConnectionCP connection = m_connections.GetConnection(info.GetConnectionId().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }

    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, info.GetRulesetId().c_str(), info.GetLocale().c_str(),
        info.GetVirtualParentNodeId(), nullptr, IsUpdatesDisabled(info));
    return context.IsValid() ? CachedVirtualNodeChildrenProvider::Create(*context, m_db, m_statements) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetDataSource(uint64_t nodeId, bool removeIfInvalid) const
    {
    DataSourceInfo info = GetDataSourceInfo(nodeId);
    if (!info.IsValid())
        return nullptr;

    if (removeIfInvalid && HasRelatedSettingsChanged(info.GetDataSourceId(), info.GetRulesetId()))
        {
        const_cast<NodesCache*>(this)->RemoveDataSource(info.GetDataSourceId());
        return nullptr;
        }
    
    IConnectionCP connection = m_connections.GetConnection(info.GetConnectionId().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }
    
    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, info.GetRulesetId().c_str(), info.GetLocale().c_str(),
        info.GetPhysicalParentNodeId(), nullptr, IsUpdatesDisabled(info));
    return context.IsValid() ? CachedHierarchyLevelProvider::Create(*context, m_db, m_statements, info.GetPhysicalParentNodeId()) : nullptr;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Cache(JsonNavNodeR node, bool isVirtual)
    {
    NavNodeExtendedData extendedData(node);
    BeAssert(extendedData.HasConnectionId() && extendedData.HasRulesetId() && extendedData.HasLocale());
    Utf8String connectionId = extendedData.GetConnectionId();
    uint64_t virtualParentId = extendedData.GetVirtualParentId();
    uint64_t const* virtualParentIdP = (extendedData.HasVirtualParentId() && 0 != virtualParentId) ? &virtualParentId : nullptr;
    
    DataSourceInfo info = GetDataSourceInfo(&connectionId, extendedData.GetRulesetId(), extendedData.GetLocale(), virtualParentIdP, true);
    if (!info.IsValid())
        {
        BeAssert(false);
        return;
        }

    CacheNode(info, node, isVirtual);
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
        CacheRelatedClassIds(info.GetDataSourceId(), relatedClassIds);
        CacheRelatedSettings(info.GetDataSourceId(), relatedSettings);
        }

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheHierarchyLevel(HierarchyLevelInfo const& info, NavNodesProviderR provider)
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
        stmt->BindInt64(2, info.GetDataSourceId());
        stmt->Step();
        }

    if (nullptr != relatedClassIds)
        CacheRelatedClassIds(info.GetDataSourceId(), *relatedClassIds);

    if (nullptr != relatedSettings)
        CacheRelatedSettings(info.GetDataSourceId(), *relatedSettings);

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemapNodeIds(bmap<uint64_t, uint64_t> const& remapInfo)
    {
    // statement to remap physical data sources
    static Utf8CP s_pdsQuery = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] "
                               "   SET [PhysicalParentNodeId] = ? "
                               " WHERE [PhysicalParentNodeId] = ?";
    CachedStatementPtr pdsStatement;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(pdsStatement, *m_db.GetDbFile(), s_pdsQuery))
        {
        BeAssert(false);
        return;
        }

    // statement to remap virtual data sources
    static Utf8CP s_vdsQuery = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] "
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
BeGuid NodesCache::CreateRemovalId(HierarchyLevelInfo const& info)
    {
    BeGuid removalId(true);
    Utf8String query = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] "
                       "   SET [RemovalId] = ? "
                       " WHERE [ConnectionId] = ? AND [RulesetId] = ? AND [Locale] = ? AND [PhysicalParentNodeId] ";
    if (nullptr != info.GetPhysicalParentNodeId())
        query.append("= ?");
    else
        query.append("IS NULL");
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return BeGuid();
        }
    stmt->BindGuid(1, removalId);
    stmt->BindText(2, info.GetConnectionId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(4, info.GetLocale().c_str(), Statement::MakeCopy::No);
    if (nullptr != info.GetPhysicalParentNodeId())
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
void NodesCache::RemoveDataSource(BeGuidCR removalId)
    {
    Utf8String query = "DELETE FROM [" NODESCACHE_TABLENAME_DataSources "] "
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
void NodesCache::RemoveDataSource(uint64_t datasourceId)
    {
    Utf8CP query = "DELETE FROM [" NODESCACHE_TABLENAME_DataSources "] "
                   " WHERE [Id] = ?";
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return;
        }
    stmt->BindUInt64(1, datasourceId);
    stmt->Step();

#ifdef NAVNODES_CACHE_DEBUG
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::ChangeVisibility(uint64_t nodeId, bool isVirtual, bool updateChildDatasources)
    {
    // first, make sure the node is not virtual
    Utf8String query = "UPDATE [" NODESCACHE_TABLENAME_Nodes "] "
                       "SET [IsVirtual] = ? ";
    query.append("WHERE [Id] = ?");

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
    LoggingHelper::LogMessage(Log::NavigationCache, GetNodeDebugString(logMsg.c_str(), nodeId, NodeVisibility::Any).c_str());

    // then, remap its child datasources
    if (updateChildDatasources)
        {    
        if (isVirtual)
            {
            query = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] "
                    "SET [PhysicalParentNodeId] = ("
                    "    SELECT [p].[PhysicalParentNodeId] "
                    "      FROM [" NODESCACHE_TABLENAME_DataSources "] p "
                    "      JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [p].[Id] "
                    "     WHERE [n].[Id] = [" NODESCACHE_TABLENAME_DataSources "].[VirtualParentNodeId] "
                    ") "
                    "WHERE [VirtualParentNodeId] = ?";
            }
        else
            {
            query = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] "
                    "SET [PhysicalParentNodeId] = [VirtualParentNodeId] "
                    "WHERE [VirtualParentNodeId] = ?";
            }
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
            {
            BeAssert(false);
            return;
            }
        stmt->BindUInt64(1, nodeId);
        stmt->Step();
        LoggingHelper::LogMessage(Log::NavigationCache, Utf8PrintfString("    Affected child data sources: %d", m_db.GetModifiedRowCount()).c_str());
        }

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
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::Clear(IConnectionCP connection, Utf8CP rulesetId)
    {
    Utf8String query = "DELETE FROM [" NODESCACHE_TABLENAME_DataSources "] ";
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

    Utf8CP query = "SELECT [ConnectionId], [RulesetId], [Locale], [PhysicalParentNodeId], [Filter], 1 AS Priority "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSourceClasses "] dsc ON [dsc].[DataSourceId] = [ds].[Id] "
                   " WHERE [ds].[ConnectionId] = ? "
                   "       AND NOT [ds].[IsUpdatesDisabled] "
                   "       AND (InVirtualSet(?, [dsc].[ECClassId]) AND [Polymorphic] "
                   "            OR InVirtualSet(?, [dsc].[ECClassId]) AND NOT [Polymorphic]) "
                   "UNION ALL "
                   "SELECT [ConnectionId], [RulesetId], [Locale], [PhysicalParentNodeId], [Filter], 2 AS Priority "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id] "
                   "  JOIN [" NODESCACHE_TABLENAME_AffectingInstances "] ai ON [ai].[NodeId] = [n].[Id] "
                   " WHERE [ds].[ConnectionId] = ? "
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

    ECSqlStatementCache& ecsqlStatements = m_ecsqlStamementCache.GetECSqlStatementCache(*connection);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        int priority = stmt->GetValueInt(5);
        Utf8CP filter = stmt->GetValueText(4);
        if (priority < 2 && nullptr != filter && 0 != *filter && !AnyKeyMatchesFilter(*connection, filter, keys, ecsqlStatements))
            continue;

        Utf8CP connectionId = stmt->GetValueText(0);
        Utf8CP rulesetId = stmt->GetValueText(1);
        Utf8CP locale = stmt->GetValueText(2);
        uint64_t physicalParentNodeId = stmt->GetValueUInt64(3);
        infos.push_back(HierarchyLevelInfo(connectionId, rulesetId, locale, stmt->IsColumnNull(3) ? nullptr : &physicalParentNodeId));
        }

    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HierarchyLevelInfo> NodesCache::GetRelatedHierarchyLevels(Utf8CP rulesetId, Utf8CP settingId) const
    {
    bvector<HierarchyLevelInfo> infos;
    Utf8CP query = "SELECT DISTINCT [ConnectionId], [Locale], [PhysicalParentNodeId] "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSourceSettings "] dss ON [dss].[DataSourceId] = [ds].[Id] "
                   " WHERE [ds].[RulesetId] = ? AND [dss].[SettingId] = ? AND NOT [ds].[IsUpdatesDisabled] ";

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
        Utf8CP connectionId = stmt->GetValueText(0);
        Utf8CP locale = stmt->GetValueText(1);
        uint64_t physicalParentNodeId = stmt->GetValueUInt64(2);
        infos.push_back(HierarchyLevelInfo(connectionId, rulesetId, locale, stmt->IsColumnNull(1) ? nullptr : &physicalParentNodeId));
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

    Utf8CP query = "SELECT [ds].[VirtualParentNodeId] "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
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
    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [n].[Id], [ex].[NodeId], [k].[PathFromRoot] "
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id]"
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
        "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
        " WHERE [n].[UniqueHash] = ? AND [ds].[ConnectionId] = ?";// AND[ds].[Locale] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, nodeKey.GetNodeHash(), Statement::MakeCopy::Yes);
    stmt->BindText(2, connection.GetId().c_str(), Statement::MakeCopy::No);
    // stmt->BindText(3, locale.c_str(), Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    JsonNavNodeCPtr node = CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections);
    BeAssert(BE_SQLITE_DONE == stmt->Step());
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::AddQuick(HierarchyLevelInfo info, NavNodesProviderR provider)
    {
    RemoveQuick(info);
    m_quickDataSourceCache.push_back(bpair<HierarchyLevelInfo, NavNodesProviderPtr>(info, &provider));
    if (m_quickDataSourceCache.size() > NODESCACHE_QUICK_Size)
        m_quickDataSourceCache.erase(m_quickDataSourceCache.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemoveQuick(HierarchyLevelInfo const& info)
    {
    for (size_t i = 0; i < m_quickDataSourceCache.size(); ++i)
        {
        HierarchyLevelInfo const& lhs = m_quickDataSourceCache[i].first;
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
NavNodesProviderPtr NodesCache::GetQuick(HierarchyLevelInfo const& info) const
    {
    for (size_t i = 0; i < m_quickDataSourceCache.size(); ++i)
        {
        HierarchyLevelInfo const& lhs = m_quickDataSourceCache[i].first;
        if (lhs == info)
            {
            NavNodesProviderPtr ds = m_quickDataSourceCache[i].second;
            m_quickDataSourceCache.erase(m_quickDataSourceCache.begin() + i);
            m_quickDataSourceCache.push_back(bpair<HierarchyLevelInfo, NavNodesProviderPtr>(info, ds));
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
bvector<NavNodeCPtr> NodesCache::GetFilteredNodes(IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale, Utf8CP filtertext) const
    {
    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [n].[Id], [ex].[NodeId], [k].[PathFromRoot] "
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id] "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId] "
                       " WHERE [ds].[ConnectionId] = ? AND [ds].[RulesetId] = ? AND [ds].[Locale] = ? AND [n].[Label] LIKE ? "
                       " ORDER BY [n].[Id]";

    bvector<NavNodeCPtr> nodeList;
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nodeList;
        }

    Utf8String filter("%");
    filter.append(filtertext);
    filter.append("%");
    stmt->BindText(1, connection.GetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, rulesetId, Statement::MakeCopy::No);
    stmt->BindText(3, locale, Statement::MakeCopy::No);
    stmt->BindText(4, filter.c_str(), Statement::MakeCopy::No);

    while (BE_SQLITE_ROW == stmt->Step())
        nodeList.push_back(CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections));

    return nodeList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::ResetExpandedNodes(Utf8CP connectionId, Utf8CP rulesetId)
    { 
    Utf8String query = "  WITH DataSet AS "
                                 "(SELECT [n].[Id] "
                                 "   FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                                 "   JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "    
                                 " WHERE [ds].[ConnectionId] = ? AND [ds].[RulesetId] = ? )"
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
