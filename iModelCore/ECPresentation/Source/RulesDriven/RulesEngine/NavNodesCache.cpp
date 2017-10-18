/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/NavNodesCache.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
#define NAVNODES_CACHE_DB_VERSION_MAJOR 1
#define NAVNODES_CACHE_DB_VERSION_MINOR 0
#define NAVNODES_CACHE_MEMORY_CACHE
//#define NAVNODES_CACHE_PERSIST_ON_CHANGE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct NavigationCacheBusyRetry : BusyRetry
{
    int _OnBusy(int count) const override
        {
        if (count > 1000)
            {
            BeAssert(false);
            return 0;
            }
        
        BeThreadUtilities::BeSleep(1);
        return 1;
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
static JsonNavNodePtr CreateNodeFromString(JsonNavNodesFactoryCR nodesFactory, ECDb& connection, Utf8CP serializedNode)
    {
    rapidjson::Document json;
    json.Parse(serializedNode);
    return nodesFactory.CreateFromJson(connection, std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static JsonNavNodePtr CreateNodeFromStatement(Statement& stmt, JsonNavNodesFactoryCR nodesFactory, ECDb& connection)
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

    node->SetIsExpanded(!stmt.IsColumnNull(4));

    NavNodeExtendedData extendedData(*node);
    if (!stmt.IsColumnNull(2))
        extendedData.SetVirtualParentId(stmt.GetValueUInt64(2));

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static ECDb* GetConnectionFromStatement(Statement& stmt, IConnectionCacheCR connections)
    {
    BeGuid connectionId = stmt.GetValueGuid(0);
    return connections.GetConnection(connectionId.ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static JsonNavNodePtr CreateNodeFromStatement(Statement& stmt, JsonNavNodesFactoryCR nodesFactory, IConnectionCacheCR connections)
    {
    ECDb* connection = GetConnectionFromStatement(stmt, connections);
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

#ifndef NAVNODES_CACHE_MEMORY_CACHE
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::Initialize(BeFileNameCR tempDirectory)
    {
    BeSQLiteLib::Initialize(tempDirectory);

#ifdef NAVNODES_CACHE_MEMORY_CACHE
    DbResult result = m_db.CreateNewDb(":memory:", BeGuid(), Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8,
        true, DefaultTxn::Yes, new NavigationCacheBusyRetry()));
#else
    BeFileName path(tempDirectory);
    path.AppendToPath(NAVNODES_CACHE_DB_NAME);
    
    DbResult result = BE_SQLITE_ERROR;
    if (path.DoesPathExist())
        {
        result = m_db.OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes, new NavigationCacheBusyRetry()));
        if (BE_SQLITE_OK == result && GetCacheVersion(m_db).GetMajor() < NAVNODES_CACHE_DB_VERSION_MAJOR)
            {
            // if the existing cache version is too old, simply delete the old cache and create a new one
            m_db.CloseDb();
            path.BeDeleteFile();
            result = BE_SQLITE_ERROR_ProfileTooOld;
            }
        }
    if (BE_SQLITE_OK != result)
        {
        result = m_db.CreateNewDb(path, BeGuid(), Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8, 
            true, DefaultTxn::Yes, new NavigationCacheBusyRetry()));

        // save the cache version
        static BeVersion s_cacheVersion(NAVNODES_CACHE_DB_VERSION_MAJOR, NAVNODES_CACHE_DB_VERSION_MINOR);
        m_db.SaveProperty(s_versionPropertySpec, s_cacheVersion.ToString(), nullptr, 0);
        }
#endif
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
                     "[ConnectionId] GUID NOT NULL, "
                     "[RulesetId] TEXT NOT NULL, "
                     "[Filter] TEXT, "
                     "[RemovalId] GUID, "
                     "[IsUpdatesDisabled] BOOLEAN NOT NULL DEFAULT FALSE ";
        m_db.CreateTable(NODESCACHE_TABLENAME_DataSources, ddl);
        m_db.ExecuteSql("CREATE UNIQUE INDEX [UX_DataSources_Virtual] ON [" NODESCACHE_TABLENAME_DataSources "](COALESCE([VirtualParentNodeId], 0),[ConnectionId],[RulesetId],COALESCE([RemovalId], 0))");
        m_db.ExecuteSql("CREATE INDEX [IX_DataSources_Physical] ON [" NODESCACHE_TABLENAME_DataSources "]([PhysicalParentNodeId],[ConnectionId],[RulesetId])");
        m_db.ExecuteSql("CREATE INDEX [IX_DataSources_RemovalId] ON [" NODESCACHE_TABLENAME_DataSources "]([RemovalId])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_DataSourceClasses))
        {
        Utf8CP ddl = "[DataSourceId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[ECClassId] INTEGER NOT NULL, "
                     "PRIMARY KEY([DataSourceId], [ECClassId])";
        m_db.CreateTable(NODESCACHE_TABLENAME_DataSourceClasses, ddl);
        m_db.ExecuteSql("CREATE UNIQUE INDEX [UX_DataSourceClasses] ON [" NODESCACHE_TABLENAME_DataSourceClasses "]([DataSourceId],[ECClassId])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_DataSourceSettings))
        {
        Utf8CP ddl = "[DataSourceId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[SettingId] TEXT NOT NULL, "
                     "PRIMARY KEY([SettingId],[DataSourceId])";
        m_db.CreateTable(NODESCACHE_TABLENAME_DataSourceSettings, ddl);
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_Nodes))
        {
        Utf8CP ddl = "[DataSourceId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_DataSources "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[Id] INTEGER PRIMARY KEY NOT NULL UNIQUE, "
                     "[IsVirtual] BOOLEAN NOT NULL DEFAULT FALSE, "
                     "[Data] TEXT NOT NULL";
        m_db.CreateTable(NODESCACHE_TABLENAME_Nodes, ddl);
        m_db.ExecuteSql("CREATE INDEX [IX_Nodes_DataSource] ON [" NODESCACHE_TABLENAME_Nodes "]([DataSourceId])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_NodeKeys))
        {
        Utf8CP ddl = "[NodeId] INTEGER PRIMARY KEY NOT NULL UNIQUE REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[Type] TEXT NOT NULL,"
                     "[Label] TEXT NOT NULL,"
                     "[ECClassId] INTEGER, "
                     "[ECInstanceId] INTEGER,"
                     "[ECPropertyName] TEXT,"
                     "[ECPropertyGroupingValue] TEXT,"
                     "[ECPropertyGroupingRangeIndex] INTEGER";
        m_db.CreateTable(NODESCACHE_TABLENAME_NodeKeys, ddl);
        m_db.ExecuteSql("CREATE INDEX [IX_NodeKeys_ECInstanceNodeKeys] ON [" NODESCACHE_TABLENAME_NodeKeys "]([Type],[ECClassId],[ECInstanceId])");
        //m_db.ExecuteSql("CREATE INDEX [IX_NodeKeys_ECPropertyGroupingNodeKeys] ON [" NODESCACHE_TABLENAME_NodeKeys "]([Type],[ECClassId],[ECPropertyName],[ECPropertyGroupingValue],[ECPropertyGroupingRangeIndex])");
        //m_db.ExecuteSql("CREATE INDEX [IX_NodeKeys_LabelGroupingNodeKeys] ON [" NODESCACHE_TABLENAME_NodeKeys "]([Type],[Label])");
        }
    if (!m_db.TableExists(NODESCACHE_TABLENAME_AffectingInstances))
        {
        Utf8CP ddl = "[NodeId] INTEGER NOT NULL REFERENCES " NODESCACHE_TABLENAME_Nodes "([Id]) ON DELETE CASCADE ON UPDATE CASCADE, "
                     "[ECClassId] INTEGER NOT NULL, "
                     "[ECInstanceId] INTEGER NOT NULL";
        m_db.CreateTable(NODESCACHE_TABLENAME_AffectingInstances, ddl);
        m_db.ExecuteSql("CREATE UNIQUE INDEX [UX_GroupedECInstances] ON [" NODESCACHE_TABLENAME_AffectingInstances "]([NodeId],[ECClassId],[ECInstanceId])");
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
    if (!m_db.TableExists(NODESCACHE_TABLENAME_ExpandedNodes))
        {
        Utf8CP createTempTable = "CREATE TEMP TABLE [" NODESCACHE_TABLENAME_ExpandedNodes "] ("
                                 "[NodeId] INTEGER NOT NULL)";                               
        DbResult result = m_db.ExecuteSql(createTempTable);
        BeAssert(result == BE_SQLITE_OK);

        Utf8CP createUniqueIndex = "CREATE UNIQUE INDEX [NodeIndex] ON [" NODESCACHE_TABLENAME_ExpandedNodes "]([NodeId])";
        result = m_db.ExecuteSql(createUniqueIndex);
        BeAssert(result == BE_SQLITE_OK);
        }
    m_db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache::NodesCache(BeFileNameCR tempDirectory, JsonNavNodesFactoryCR nodesFactory, INodesProviderContextFactoryCR contextFactory, IConnectionCacheCR connections)
    : m_nodesFactory(nodesFactory), m_contextFactory(contextFactory), m_connections(connections), m_statements(10)
    {
    Initialize(tempDirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache::~NodesCache()
    {
    Clear();
    }

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
bool NodesCache::IsDataSourceCached(BeGuidCR connectionId, Utf8CP rulesetId) const
    {
    Utf8String query = "SELECT 1 "
                       "  FROM [" NODESCACHE_TABLENAME_DataSources "] "
                       " WHERE [ConnectionId] = ? AND [RulesetId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindGuid(1, connectionId);    
    stmt->BindText(2, rulesetId, Statement::MakeCopy::No);
        
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
    query = "INSERT INTO [" NODESCACHE_TABLENAME_NodeKeys "] ([NodeId],[Type],[Label],[ECClassId],[ECInstanceId],[ECPropertyName],[ECPropertyGroupingValue],[ECPropertyGroupingRangeIndex])"
                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    
    NavNodeKeyCR key = node.GetKey();

    stmt = nullptr;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    stmt->BindInt64(1, node.GetNodeId());    
    stmt->BindText(2, node.GetType().c_str(), Statement::MakeCopy::Yes);    
    stmt->BindText(3, node.GetLabel().c_str(), Statement::MakeCopy::Yes);
        
    NavNodeExtendedData extendedData(node);

    if (extendedData.HasECClassId())
        stmt->BindId(4, extendedData.GetECClassId());
    else
        stmt->BindNull(4);

    if (nullptr != key.AsECInstanceNodeKey())
        stmt->BindId(5, key.AsECInstanceNodeKey()->GetInstanceId());
    else
        stmt->BindNull(5);
            
    Utf8String groupingValueStr;
    if (nullptr != key.AsECPropertyGroupingNodeKey())
        {
        stmt->BindText(6, key.AsECPropertyGroupingNodeKey()->GetPropertyName().c_str(), Statement::MakeCopy::No);
        if (nullptr != key.AsECPropertyGroupingNodeKey()->GetGroupingValue())
            {
            groupingValueStr = GetSerializedJson(*key.AsECPropertyGroupingNodeKey()->GetGroupingValue());
            stmt->BindText(7, groupingValueStr.c_str(), Statement::MakeCopy::No);
            }
        else
            {
            stmt->BindNull(7);
            }
        stmt->BindInt(8, key.AsECPropertyGroupingNodeKey()->GetGroupingRangeIndex());
        }
    else
        {
        stmt->BindNull(6);
        stmt->BindNull(7);
        stmt->BindNull(8);
        }

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
    else if (info.GetConnectionId().IsValid() && !info.GetRulesetId().empty())
        {
        str.append(" root data source {");
        str.append("Id: ").append(std::to_string(info.GetDataSourceId()).c_str()).append(", ");
        str.append("ConnectionId: ").append(info.GetConnectionId().ToString()).append(", ");
        str.append("RulesetId: '").append(info.GetRulesetId()).append("'");
        if (nullptr != info.GetVirtualParentNodeId())
            str.append(", VirtualParentNodeId: ").append(std::to_string(*info.GetVirtualParentNodeId()).c_str());
        str.append("}");
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheNode(DataSourceInfo const& datasourceInfo, NavNodeCR node, bool isVirtual)
    {
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_Nodes "] ([DataSourceId],[Id],[IsVirtual],[Data])"
                       "VALUES (?, ?, ?, ?)";
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }
    
    stmt->BindUInt64(1, datasourceInfo.GetDataSourceId());
    stmt->BindUInt64(2, node.GetNodeId());
    stmt->BindBoolean(3, isVirtual);

    BeAssert(nullptr != dynamic_cast<JsonNavNodeCP>(&node));
    Utf8String nodeStr = GetSerializedJson(static_cast<JsonNavNodeCR>(node).GetJson());
    stmt->BindText(4, nodeStr.c_str(), Statement::MakeCopy::No);
        
    stmt->Step();

    CacheNodeKey(node);
    CacheNodeInstanceKeys(node);

    if (node.IsExpanded())
        SetIsExpanded(node.GetNodeId(), node.IsExpanded());

    LoggingHelper::LogMessage(Log::NavigationCache, GetNodeDebugString("Cached", node, isVirtual ? NodeVisibility::Virtual : NodeVisibility::Physical).c_str());

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
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
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_DataSources "] ([ConnectionId], [RulesetId], [PhysicalParentNodeId], [VirtualParentNodeId], [Filter], [IsUpdatesDisabled]) "
                       "VALUES (?, ?, ?, ?, ?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    Utf8String filterStr = GetSerializedJson(filter.AsJson());

    stmt->BindGuid(1, info.GetConnectionId());
    stmt->BindText(2, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    BindNullableId(*stmt, 3, info.GetPhysicalParentNodeId());
    BindNullableId(*stmt, 4, info.GetVirtualParentNodeId());
    stmt->BindText(5, filterStr.c_str(), Statement::MakeCopy::No);
    stmt->BindBoolean(6, disableUpdates);

    DbResult result = stmt->Step();
    if (BE_SQLITE_DONE == result)
        {
        info.SetDataSourceId((uint64_t)m_db.GetLastInsertRowId());
        LoggingHelper::LogMessage(Log::NavigationCache, GetDataSourceDebugString("Cached empty", info).c_str());
        return;
        }

    LoggingHelper::LogMessage(Log::NavigationCache, GetDataSourceDebugString("Already cached", info).c_str());
    DataSourceInfo existing = GetDataSourceInfo(&info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetVirtualParentNodeId(), true);
    BeAssert(existing.IsValid());
    info.SetDataSourceId(existing.GetDataSourceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheRelatedClassIds(uint64_t datasourceId, bvector<ECClassId> const& classIds)
    {
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_DataSourceClasses "] ([DataSourceId], [ECClassId]) VALUES (?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    for (ECClassId classId : classIds)
        {
        stmt->Reset();
        stmt->BindUInt64(1, datasourceId);
        stmt->BindUInt64(2, classId.GetValue());
        stmt->Step();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::CacheRelatedSettingIds(uint64_t datasourceId, bvector<Utf8String> const& settingIds)
    {
    Utf8String query = "INSERT INTO [" NODESCACHE_TABLENAME_DataSourceSettings "] ([DataSourceId], [SettingId]) VALUES (?, ?)";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    for (Utf8StringCR settingId : settingIds)
        {
        stmt->Reset();
        stmt->BindUInt64(1, datasourceId);
        stmt->BindText(2, settingId.c_str(), Statement::MakeCopy::No);
        stmt->Step();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCache::GetDataSourceInfo(BeGuidCP connectionId, Utf8CP rulesetId, uint64_t const* parentNodeId, bool isVirtual) const
    {
    Utf8String query = "SELECT [ds].[Id], [ds].[ConnectionId], [ds].[RulesetId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId] "
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
        stmt->BindGuid(bindingIndex++, *connectionId);
        
    if (nullptr != rulesetId)
        stmt->BindText(bindingIndex++, rulesetId, Statement::MakeCopy::No);
            
    if (BE_SQLITE_ROW != stmt->Step())
        return DataSourceInfo();

    uint64_t physicalParentNodeId = stmt->GetValueUInt64(3);
    uint64_t virtualParentNodeId = stmt->GetValueUInt64(4);
    return DataSourceInfo(stmt->GetValueUInt64(0), stmt->GetValueGuid(1), stmt->GetValueText(2), 
        stmt->IsColumnNull(3) ? nullptr : &physicalParentNodeId, 
        stmt->IsColumnNull(4) ? nullptr : &virtualParentNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::HasDataSource(HierarchyLevelInfo const& levelInfo) const
    {
    DataSourceInfo dsInfo = GetDataSourceInfo(&levelInfo.GetConnectionId(), levelInfo.GetRulesetId().c_str(), levelInfo.GetPhysicalParentNodeId(), true);
    return dsInfo.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCache::HasDataSource(DataSourceInfo const& info) const
    {
    DataSourceInfo dsInfo = GetDataSourceInfo(&info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetVirtualParentNodeId(), true);
    return dsInfo.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCache::GetDataSourceInfo(uint64_t nodeId) const
    {
    Utf8CP query = "SELECT [ds].[Id], [ds].[ConnectionId], [ds].[RulesetId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId] "
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
    return DataSourceInfo(stmt->GetValueUInt64(0), stmt->GetValueGuid(1), stmt->GetValueText(2), 
        stmt->IsColumnNull(3) ? nullptr : &physicalParentNodeId, 
        stmt->IsColumnNull(4) ? nullptr : &virtualParentNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr NodesCache::_GetNode(uint64_t id, NodeVisibility visibility) const
    {
    JsonNavNodePtr node = GetQuick(id);
    if (node.IsValid() && NodeVisibility::Any == visibility)
        return node;

    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [ex].[NodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
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
                       " WHERE [ds].[RemovalId] IS NULL AND [ds].[ConnectionId] = ? AND [ds].[RulesetId] = ? AND [ds].[PhysicalParentNodeId] "; 
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

    stmt->BindGuid(1, info.GetConnectionId());
    stmt->BindText(2, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    if (nullptr != info.GetPhysicalParentNodeId())
        stmt->BindInt64(3, *info.GetPhysicalParentNodeId());
            
    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    return stmt->GetValueBoolean(0);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetDataSource(HierarchyLevelInfo const& info) const
    {
    NavNodesProviderPtr source = GetQuick(info);
    if (source.IsValid())
        return source;
    
    if (!HasDataSource(info))
        return nullptr;

    ECDb const* connection = m_connections.GetConnection(info.GetConnectionId().ToString().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }

    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, info.GetRulesetId().c_str(), info.GetPhysicalParentNodeId(),
        IsUpdatesDisabled(info));
    return context.IsValid() ? CachedHierarchyLevelProvider::Create(*context, m_db, m_statements, info.GetPhysicalParentNodeId()) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetDataSource(DataSourceInfo const& info) const
    {
    if (!HasDataSource(info))
        return nullptr;

    ECDb const* connection = m_connections.GetConnection(info.GetConnectionId().ToString().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }

    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, info.GetRulesetId().c_str(), info.GetVirtualParentNodeId(), 
        IsUpdatesDisabled(info));
    return context.IsValid() ? CachedVirtualNodeChildrenProvider::Create(*context, m_db, m_statements) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCache::_GetDataSource(uint64_t nodeId) const
    {
    DataSourceInfo info = GetDataSourceInfo(nodeId);
    if (!info.IsValid())
        return nullptr;
    
    ECDb const* connection = m_connections.GetConnection(info.GetConnectionId().ToString().c_str());
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }
    
    NavNodesProviderContextPtr context = m_contextFactory.Create(*connection, info.GetRulesetId().c_str(), info.GetPhysicalParentNodeId(), 
        IsUpdatesDisabled(info));
    return context.IsValid() ? CachedHierarchyLevelProvider::Create(*context, m_db, m_statements, info.GetPhysicalParentNodeId()) : nullptr;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Cache(JsonNavNodeCR node, bool isVirtual)
    {
    NavNodeExtendedData extendedData(node);
    BeAssert(extendedData.HasConnectionId() && extendedData.HasRulesetId());
    BeGuid connectionId = extendedData.GetConnectionId();
    uint64_t virtualParentId = extendedData.GetVirtualParentId();
    uint64_t const* virtualParentIdP = (extendedData.HasVirtualParentId() && 0 != virtualParentId) ? &virtualParentId : nullptr;
    
    DataSourceInfo info = GetDataSourceInfo(&connectionId, extendedData.GetRulesetId(), virtualParentIdP, true);
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
void NodesCache::_Cache(DataSourceInfo& info, DataSourceFilter const& filter, bvector<ECClassId> const& relatedClassIds, bvector<Utf8String> const& relatedSettingIds,
    bool disableUpdates)
    {
    CacheEmptyDataSource(info, filter, disableUpdates);

    if (info.IsValid())
        {
        CacheRelatedClassIds(info.GetDataSourceId(), relatedClassIds);
        CacheRelatedSettingIds(info.GetDataSourceId(), relatedSettingIds);
        }

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
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
                       "SET [Id] = ?, [Data] = ? "
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

    stmt->BindUInt64(3, nodeId);

    if (BE_SQLITE_DONE == stmt->Step() && 1 == m_db.GetModifiedRowCount())
        {
        CacheNodeKey(node);
        CacheNodeInstanceKeys(node);
        SetIsExpanded(node.GetNodeId(), node.IsExpanded());
        }

    SetIsExpanded(nodeId, node.IsExpanded());
    RemoveQuick(nodeId);

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::_Update(DataSourceInfo const& info, DataSourceFilter const* filter, bvector<ECClassId> const* relatedClassIds, bvector<Utf8String> const* relatedSettingIds)
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

    if (nullptr != relatedSettingIds)
        CacheRelatedSettingIds(info.GetDataSourceId(), *relatedSettingIds);

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
    Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::RemapNodeIds(uint64_t from, uint64_t to)
    {
    // remap physical data sources
    Utf8String query = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] "
                       "   SET [PhysicalParentNodeId] = ? "
                       " WHERE [PhysicalParentNodeId] = ?";
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }
    stmt->BindUInt64(1, to);
    stmt->BindUInt64(2, from);
    stmt->Step();

    // remap virtual data sources
    query = "UPDATE [" NODESCACHE_TABLENAME_DataSources "] "
            "   SET [VirtualParentNodeId] = ? "
            " WHERE [VirtualParentNodeId] = ?";
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }
    stmt->BindUInt64(1, to);
    stmt->BindUInt64(2, from);
    stmt->Step();

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
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
                       " WHERE [ConnectionId] = ? AND [RulesetId] = ? AND [PhysicalParentNodeId] ";
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
    stmt->BindGuid(2, info.GetConnectionId());
    stmt->BindText(3, info.GetRulesetId().c_str(), Statement::MakeCopy::No);
    if (nullptr != info.GetPhysicalParentNodeId())
        stmt->BindUInt64(4, *info.GetPhysicalParentNodeId());
    stmt->Step();

    RemoveQuick(info);
    m_quickNodesCache.clear();

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
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

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
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

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
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
void NodesCache::Clear(ECDb const* connection, Utf8CP rulesetId)
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
        stmt->BindGuid(++boundVariablesCount, connection->GetDbGuid());
    if (nullptr != rulesetId)
        stmt->BindText(++boundVariablesCount, rulesetId, Statement::MakeCopy::No);

    stmt->Step();
    m_quickDataSourceCache.clear();
    m_quickNodesCache.clear();

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
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
static bool AreRelated(ECDbCR connection, DataSourceFilter::RelatedInstanceInfo const& relationshipInfo, bset<ECInstanceKey> const& keys)
    {
    for (ECInstanceKeyCR key : keys)
        {
        if (key.GetInstanceId() == relationshipInfo.m_instanceId)
            return true;
        }

    Utf8String query;
    for (ECClassId relationshipClassId : relationshipInfo.m_relationshipClassIds)
        {
        ECRelationshipClassCP relationshipClass = connection.Schemas().GetClass(relationshipClassId)->GetRelationshipClassCP();
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

    ECSqlStatement stmt;
    if (!stmt.Prepare(connection, query.c_str()).IsSuccess())
        {
        BeAssert(false);
        return false;
        }
    
    VirtualECInstanceIdSet idsSet(keys);
    int bindingIndex = 1;

    for (auto i = 0; i < relationshipInfo.m_relationshipClassIds.size(); ++i)
        {
        stmt.BindId(bindingIndex++, relationshipInfo.m_instanceId);
        stmt.BindVirtualSet(bindingIndex++, idsSet);

        if (RequiredRelationDirection_Both == relationshipInfo.m_direction)
            {
            stmt.BindId(bindingIndex++, relationshipInfo.m_instanceId);
            stmt.BindVirtualSet(bindingIndex++, idsSet);
            }
        }

    return (BE_SQLITE_ROW == stmt.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AnyKeyMatchesFilter(ECDbCR connection, Utf8CP serializedFilter, bset<ECInstanceKey> const& keys)
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
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HierarchyLevelInfo> NodesCache::GetRelatedHierarchyLevels(BeGuidCR connectionId, bset<ECInstanceKey> const& keys) const
    {
    ECDb const* connection = m_connections.GetConnection(connectionId.ToString().c_str());
    if (nullptr == connection)
        {
        LoggingHelper::LogMessage(Log::NavigationCache, Utf8PrintfString("Requested related hierarchy levels from "
            "connection that is not tracked (%s). Returning empty list.", connectionId.ToString().c_str()).c_str(), NativeLogging::LOG_WARNING);
        return bvector<HierarchyLevelInfo>();
        }

    bvector<HierarchyLevelInfo> infos;
    Utf8CP query = "SELECT [ConnectionId], [RulesetId], [PhysicalParentNodeId], [Filter], MAX([Priority]) "
                   "  FROM (SELECT [ConnectionId], [RulesetId], [PhysicalParentNodeId], [Filter], 1 AS Priority "
                   "          FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "          JOIN [" NODESCACHE_TABLENAME_DataSourceClasses "] dsc ON [dsc].[DataSourceId] = [ds].[Id] "
                   "         WHERE [ds].[ConnectionId] = ? AND NOT [ds].[IsUpdatesDisabled] AND InVirtualSet(?, [dsc].[ECClassId])"
                   "        UNION ALL"
                   "        SELECT [ConnectionId], [RulesetId], [PhysicalParentNodeId], [Filter], 2 AS Priority "
                   "          FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "          JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id] "
                   "          JOIN [" NODESCACHE_TABLENAME_AffectingInstances "] ai ON [ai].[NodeId] = [n].[Id] "
                   "         WHERE [ds].[ConnectionId] = ? AND NOT [ds].[IsUpdatesDisabled] AND InVirtualSet(?, [ai].[ECClassId], [ai].[ECInstanceId])"
                   ")"
                   "GROUP BY [ConnectionId], [RulesetId], [PhysicalParentNodeId]";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query))
        {
        BeAssert(false);
        return infos;
        }

    stmt->BindGuid(1, connectionId);

    ECClassIdSet classIdsVirtualSet(keys);
    stmt->BindVirtualSet(2, classIdsVirtualSet);

    stmt->BindGuid(3, connectionId);

    ECInstanceKeySet instanceKeysVirtualSet(keys);
    stmt->BindVirtualSet(4, instanceKeysVirtualSet);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        int priority = stmt->GetValueInt(4);
        Utf8CP filter = stmt->GetValueText(3);
        if (priority < 2 && nullptr != filter && 0 != *filter && !AnyKeyMatchesFilter(*connection, filter, keys))
            continue;

        BeGuid connectionId = stmt->GetValueGuid(0);
        Utf8CP rulesetId = stmt->GetValueText(1);
        uint64_t physicalParentNodeId = stmt->GetValueUInt64(2);
        infos.push_back(HierarchyLevelInfo(connectionId, rulesetId, stmt->IsColumnNull(2) ? nullptr : &physicalParentNodeId));
        }

    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HierarchyLevelInfo> NodesCache::GetRelatedHierarchyLevels(Utf8CP rulesetId, Utf8CP settingId) const
    {
    bvector<HierarchyLevelInfo> infos;
    Utf8CP query = "SELECT DISTINCT [ConnectionId], [PhysicalParentNodeId] "
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
        BeGuid connectionId = stmt->GetValueGuid(0);
        uint64_t physicalParentNodeId = stmt->GetValueUInt64(1);
        infos.push_back(HierarchyLevelInfo(connectionId, rulesetId, stmt->IsColumnNull(1) ? nullptr : &physicalParentNodeId));
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
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr NodesCache::LocateECInstanceNode(ECInstanceNodeKey const& key) const
    {
    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [ex].[NodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                       " WHERE [k].[Type] = ? AND [k].[ECClassId] = ? AND [k].[ECInstanceId] = ?";
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, NAVNODE_TYPE_ECInstanceNode, Statement::MakeCopy::No);
    stmt->BindId(2, key.GetECClassId());
    stmt->BindId(3, key.GetInstanceId());
    
    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    return CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr NodesCache::LocateECClassGroupingNode(ECClassGroupingNodeKey const& key) const
    {
    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [ex].[NodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId] "
                       " WHERE [k].[NodeId] = ? AND [k].[Type] = ? AND [k].[ECClassId] = ?";
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindUInt64(1, key.GetNodeId());
    stmt->BindText(2, key.GetType().c_str(), Statement::MakeCopy::No);
    stmt->BindId(3, key.GetECClassId());
        
    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    return CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr NodesCache::LocateECPropertyGroupingNode(ECPropertyGroupingNodeKey const& key) const
    {
    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [ex].[NodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId] "
                       " WHERE [k].[NodeId] = ?";
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindUInt64(1, key.GetNodeId());

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    return CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr NodesCache::LocateDisplayLabelGroupingNode(DisplayLabelGroupingNodeKey const& key) const
    {
    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [ex].[NodeId] "
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId] "
                       " WHERE [k].[NodeId] = ? AND [k].[Type] = ? AND [k].[Label] = ?";
    
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_db.GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }
    
    stmt->BindUInt64(1, key.GetNodeId());
    stmt->BindText(2, key.GetType().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, key.GetLabel().c_str(), Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    return CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr NodesCache::_LocateNode(NavNodeKeyCR nodeKey) const
    {
    if (nullptr != nodeKey.AsECInstanceNodeKey())
        return LocateECInstanceNode(*nodeKey.AsECInstanceNodeKey());
    
    if (nullptr != nodeKey.AsECPropertyGroupingNodeKey())
        return LocateECPropertyGroupingNode(*nodeKey.AsECPropertyGroupingNodeKey());

    if (nullptr != nodeKey.AsECClassGroupingNodeKey())
        return LocateECClassGroupingNode(*nodeKey.AsECClassGroupingNodeKey());
    
    if (nullptr != nodeKey.AsDisplayLabelGroupingNodeKey())
        return LocateDisplayLabelGroupingNode(*nodeKey.AsDisplayLabelGroupingNodeKey());

    BeAssert(false);
    return nullptr;
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
bvector<NavNodeCPtr> NodesCache::GetFilteredNodes(ECDbR connection, Utf8CP rulesetId, Utf8CP filtertext) const
    {
    Utf8String query = "SELECT [ds].[ConnectionId], [ds].[PhysicalParentNodeId], [ds].[VirtualParentNodeId], [n].[Data], [ex].[NodeId], [n].[Id]"
                       "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id] "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId] "
                       " WHERE [ds].[ConnectionId] = ? AND [ds].[RulesetId] = ? AND [k].[label] LIKE ? "
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
    stmt->BindGuid(1, connection.GetDbGuid());
    stmt->BindText(2, rulesetId, Statement::MakeCopy::No);
    stmt->BindText(3, filter.c_str(), Statement::MakeCopy::No);

    while (BE_SQLITE_ROW == stmt->Step())
        nodeList.push_back(CreateNodeFromStatement(*stmt, m_nodesFactory, m_connections));

    return nodeList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCache::ResetExpandedNodes(BeGuid connectionId, Utf8CP rulesetId)
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

    stmt->BindGuid(1, connectionId);
    stmt->BindText(2, rulesetId, Statement::MakeCopy::No);
    stmt->Step();

#ifdef NAVNODES_CACHE_PERSIST_ON_CHANGE
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
