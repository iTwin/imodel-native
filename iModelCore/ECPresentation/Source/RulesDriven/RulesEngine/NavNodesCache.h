/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/NavNodesCache.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "RulesEngineTypes.h"
#include "NavNodesDataSource.h"
#include "NavNodeProviders.h"
#include "ECExpressionContextsProvider.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
enum class NodeVisibility
    {
    Virtual,
    Physical,
    Any,
    };

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                09/2017
+===============+===============+===============+===============+===============+======*/
enum class NodesCacheType
    {
    Memory,
    Disk,
    };

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct IECSqlStatementCacheProvider
{
protected:
    virtual ~IECSqlStatementCacheProvider() {}
    virtual ECSqlStatementCache& _GetECSqlStatementCache(IConnectionCR) = 0;

public:
    ECSqlStatementCache& GetECSqlStatementCache(IConnectionCR connection) {return _GetECSqlStatementCache(connection);}
};

//=======================================================================================
//! An interface for @ref NavNode objects locater which can find nodes by their keys.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct  INavNodeLocater
{
protected:
    virtual JsonNavNodeCPtr _LocateNode(IConnectionCR, Utf8StringCR, NavNodeKeyCR) const = 0;
public:
    virtual ~INavNodeLocater() {}
    JsonNavNodeCPtr LocateNode(IConnectionCR connection, Utf8StringCR locale, NavNodeKeyCR key) const {return _LocateNode(connection, locale, key);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct IHierarchyCache
{
    struct Savepoint : RefCountedBase
    {
    protected:
        virtual void _Cancel() = 0;
    public:
        void Cancel() {_Cancel();}
    };
    typedef RefCountedPtr<Savepoint> SavepointPtr;

protected:
    virtual JsonNavNodePtr _GetNode(uint64_t nodeId, NodeVisibility) const = 0;
    virtual NavNodesProviderPtr _GetDataSource(HierarchyLevelInfo const&, bool) const = 0;
    virtual NavNodesProviderPtr _GetDataSource(DataSourceInfo const&, bool) const = 0;
    virtual NavNodesProviderPtr _GetDataSource(uint64_t nodeId, bool) const = 0;
    
    virtual void _Cache(DataSourceInfo&, DataSourceFilter const&, bmap<ECClassId, bool> const&, bvector<UserSettingEntry> const&, bool) = 0;
    virtual void _Cache(JsonNavNodeR, bool) = 0;

    virtual void _Update(uint64_t, JsonNavNodeCR) = 0;
    virtual void _Update(DataSourceInfo const&, DataSourceFilter const*, bmap<ECClassId, bool> const*, bvector<UserSettingEntry> const*) = 0;

    virtual void _MakePhysical(JsonNavNodeCR) = 0;
    virtual void _MakeVirtual(JsonNavNodeCR) = 0;

    virtual SavepointPtr _CreateSavepoint() = 0;

public:
    virtual ~IHierarchyCache() {}

    JsonNavNodePtr GetNode(uint64_t nodeId, NodeVisibility visibility = NodeVisibility::Any) const {return _GetNode(nodeId, visibility);}

    //! Get the data source for the whole hierarchy level.
    //  Cached datasource can be deleted if user settings used by cached datasource have changed.
    NavNodesProviderPtr GetDataSource(HierarchyLevelInfo const& info, bool removeIfInvalid = true) const {return _GetDataSource(info, removeIfInvalid);}
    //! Get the data source for a single virtual parent node (which is a subset of a hierarchy level). 
    //  Cached datasource can be deleted if user settings used by cached datasource have changed.
    NavNodesProviderPtr GetDataSource(DataSourceInfo const& info, bool removeIfInvalid = true) const {return _GetDataSource(info, removeIfInvalid);}
    //! Get the data source the node with the supplied ID belongs to.
    //  Cached datasource can be deleted if user settings used by cached datasource have changed.
    NavNodesProviderPtr GetDataSource(uint64_t nodeId, bool removeIfInvalid = true) const {return _GetDataSource(nodeId, removeIfInvalid);}
    
    void Cache(DataSourceInfo& info, DataSourceFilter const& filter, bmap<ECClassId, bool> const& relatedClassIds, bvector<UserSettingEntry> const& relatedSettings,
        bool disableUpdates = false)
        {
        _Cache(info, filter, relatedClassIds, relatedSettings, disableUpdates);
        }
    void Cache(JsonNavNodeR node, bool isVirtual) {_Cache(node, isVirtual);}

    void Update(uint64_t nodeId, JsonNavNodeCR node) {_Update(nodeId, node);}
    void Update(DataSourceInfo const& info, DataSourceFilter const* filter, bmap<ECClassId, bool> const* relatedClassIds, bvector<UserSettingEntry> const* relatedSettings)
        {
        _Update(info, filter, relatedClassIds, relatedSettings);
        }
    
    void MakePhysical(JsonNavNodeCR node) {_MakePhysical(node);}
    void MakeVirtual(JsonNavNodeCR node) {_MakeVirtual(node);}

    SavepointPtr CreateSavepoint() {return _CreateSavepoint();}
};

#define NODESCACHE_TABLENAME_DataSources        "DataSources"
#define NODESCACHE_TABLENAME_DataSourceClasses  "DataSourceClasses"
#define NODESCACHE_TABLENAME_DataSourceSettings "DataSourceSettings"
#define NODESCACHE_TABLENAME_Nodes              "Nodes"
#define NODESCACHE_TABLENAME_ExpandedNodes      "ExpandedNodes"
#define NODESCACHE_TABLENAME_NodeKeys           "NodeKeys"
#define NODESCACHE_TABLENAME_AffectingInstances "AffectingECInstances"
#define NODESCACHE_TABLENAME_Connections        "Connections"
#define NODESCACHE_TABLENAME_Rulesets           "Rulesets"

// Note: nodes cache uses a small in-memory structure of size NODESCACHE_QUICK_Size to store
// the most recently used data sources whose size is at least NODESCACHE_QUICK_Boundary nodes.
// The NODESCACHE_QUICK_Boundary should match the page size used in rules-driven controls to make
// sure we reuse the same data source for different pages.
#define NODESCACHE_QUICK_Boundary               20
#define NODESCACHE_QUICK_Size                   10

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct NodesCache : IHierarchyCache, INavNodeLocater, IConnectionsListener
{
private:
    JsonNavNodesFactory const& m_nodesFactory;
    INodesProviderContextFactoryCR m_contextFactory;
    IConnectionManagerCR m_connections;
    IUserSettingsManager const& m_userSettings;
    NodesCacheType m_type;
    bool m_tempCache;
    mutable BeSQLite::Db m_db;
    mutable BeSQLite::StatementCache m_statements;
    IECSqlStatementCacheProvider& m_ecsqlStamementCache;
    mutable bvector<bpair<HierarchyLevelInfo, NavNodesProviderPtr>> m_quickDataSourceCache;
    mutable bvector<bpair<uint64_t, JsonNavNodePtr>> m_quickNodesCache;
    uint64_t m_sizeLimit;
    
private:
    void Initialize(BeFileNameCR tempDirectory);

    void CacheNode(DataSourceInfo const&, NavNodeR, bool isVirtual);
    void CacheEmptyDataSource(DataSourceInfo&, DataSourceFilter const&, bool);
    void CacheRelatedClassIds(uint64_t datasourceId, bmap<ECClassId, bool> const&);
    void CacheRelatedSettings(uint64_t datasourceId, bvector<UserSettingEntry> const& settings);
    DataSourceInfo GetDataSourceInfo(Utf8String const* connectionId, Utf8CP rulesetId, Utf8CP locale, uint64_t const* parentNodeId, bool isVirtual) const;
    DataSourceInfo GetDataSourceInfo(uint64_t nodeId) const;
    bool HasRelatedSettingsChanged(uint64_t datasourceId, Utf8StringCR rulesetId) const;
    void CacheNodeKey(NavNodeCR);
    void CacheNodeInstanceKeys(NavNodeCR);
    void ChangeVisibility(uint64_t nodeId, bool isVirtual, bool updateChildDatasources);
    bool IsUpdatesDisabled(HierarchyLevelInfo const& info) const;
    void SetIsExpanded(uint64_t nodeId, bool isExpanded) const;
    void LimitCacheSize();
    void RemoveDataSource(uint64_t datasourceId);

    void AddQuick(HierarchyLevelInfo, NavNodesProviderR);
    void RemoveQuick(HierarchyLevelInfo const&);
    NavNodesProviderPtr GetQuick(HierarchyLevelInfo const&) const;

    void AddQuick(JsonNavNodeR) const;
    void RemoveQuick(uint64_t) const;
    JsonNavNodePtr GetQuick(uint64_t) const;

    void OnConnectionClosed(IConnectionCR);
    void OnFirstConnection(IConnectionCR);

protected:
    // IHierarchyCache
    ECPRESENTATION_EXPORT JsonNavNodePtr _GetNode(uint64_t, NodeVisibility) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetDataSource(HierarchyLevelInfo const&, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetDataSource(DataSourceInfo const&, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetDataSource(uint64_t nodeId, bool) const override;
    ECPRESENTATION_EXPORT void _Cache(DataSourceInfo&, DataSourceFilter const&, bmap<ECClassId, bool> const&, bvector<UserSettingEntry> const&, bool) override;
    ECPRESENTATION_EXPORT void _Cache(JsonNavNodeR, bool) override;
    ECPRESENTATION_EXPORT void _Update(uint64_t nodeId, JsonNavNodeCR) override;
    ECPRESENTATION_EXPORT void _Update(DataSourceInfo const&, DataSourceFilter const*, bmap<ECClassId, bool> const*, bvector<UserSettingEntry> const*) override;
    ECPRESENTATION_EXPORT void _MakePhysical(JsonNavNodeCR) override;
    ECPRESENTATION_EXPORT void _MakeVirtual(JsonNavNodeCR) override;
    ECPRESENTATION_EXPORT SavepointPtr _CreateSavepoint() override;
    
    // INavNodeLocater
    ECPRESENTATION_EXPORT JsonNavNodeCPtr _LocateNode(IConnectionCR, Utf8StringCR, NavNodeKeyCR) const override;

    // IConnectionsListener
    ECPRESENTATION_EXPORT void _OnConnectionEvent(ConnectionEvent const&) override;

public:
    ECPRESENTATION_EXPORT NodesCache(BeFileNameCR tempDirectory, JsonNavNodesFactoryCR, INodesProviderContextFactoryCR, IConnectionManagerCR, IUserSettingsManager const&, IECSqlStatementCacheProvider&, NodesCacheType);
    ECPRESENTATION_EXPORT ~NodesCache();

    ECPRESENTATION_EXPORT void CacheHierarchyLevel(HierarchyLevelInfo const&, NavNodesProviderR);
    
    ECPRESENTATION_EXPORT bool IsNodeCached(uint64_t nodeId) const;
    ECPRESENTATION_EXPORT bool IsDataSourceCached(Utf8StringCR connectionId, Utf8CP rulesetId, Utf8CP locale) const;
    ECPRESENTATION_EXPORT bool IsDataSourceCached(uint64_t parentNodeId) const;

    ECPRESENTATION_EXPORT BeSQLite::BeGuid CreateRemovalId(HierarchyLevelInfo const&);
    ECPRESENTATION_EXPORT void RemoveDataSource(BeSQLite::BeGuidCR removalId);

    ECPRESENTATION_EXPORT void RemapNodeIds(bmap<uint64_t, uint64_t> const&);
    ECPRESENTATION_EXPORT bool HasParentNode(uint64_t nodeId, bset<uint64_t> const& parentNodeIds) const;

    ECPRESENTATION_EXPORT bvector<HierarchyLevelInfo> GetRelatedHierarchyLevels(Utf8StringCR connectionId, bset<ECInstanceKey> const&) const;
    ECPRESENTATION_EXPORT bvector<HierarchyLevelInfo> GetRelatedHierarchyLevels(Utf8CP rulesetId, Utf8CP settingId) const;

    ECPRESENTATION_EXPORT void Clear(IConnectionCP connection = nullptr, Utf8CP rulesetId = nullptr);
    ECPRESENTATION_EXPORT void Persist();

    ECPRESENTATION_EXPORT void OnRulesetCreated(PresentationRuleSetCR);
    BeSQLite::Db const& GetDb() const {return m_db;}
    void SetCacheFileSizeLimit(uint64_t size) {m_sizeLimit = size;}

    ECPRESENTATION_EXPORT bvector<NavNodeCPtr> GetFilteredNodes(IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale, Utf8CP filtertext) const;
    ECPRESENTATION_EXPORT void ResetExpandedNodes(Utf8CP connectionId, Utf8CP rulesetId);
    ECPRESENTATION_EXPORT NavNodesProviderPtr GetUndeterminedNodesProvider(IConnectionCR connection, Utf8CP ruleSetId, Utf8CP locale, bool isUpdatesDisabled) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
