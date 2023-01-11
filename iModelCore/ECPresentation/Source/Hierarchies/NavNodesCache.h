/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationManager.h>
#include "../RulesEngineTypes.h"
#include "../Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "../Shared/NavNodeLocater.h"
#include "NavNodesDataSource.h"
#include "NavNodeProviders.h"
#include "NavNodesCacheHelpers.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class NodeVisibility
    {
    Visible, //!< The node is visible
    Virtual, //!< The node is not visible, but it's children are
    Hidden,  //!< The node and all the hierarchy below it is hidden
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class NodesCacheType
    {
    Memory,
    Disk,
    HybridMemory,
    HybridDisk
    };

/*=================================================================================**//**
* @bsiclass
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
//! An interface for locking and working with locked hierarchy levels.
// @bsiclass
//=======================================================================================
struct IHierarchyLevelLocker
{
    enum LockOptions
        {
        DisableLockWait        = 1 << 0, //!< Disables waiting if lock could not be acquired
        CheckLockedChildLevels = 1 << 1  //!< Additionally checks if there are no locked child levels when acquiring lock
        };

protected:
    virtual bool _Lock(int) = 0;
    virtual void _Unlock() = 0;
    virtual void _WaitForUnlock() = 0;

public:
    virtual ~IHierarchyLevelLocker() {}
    //! Locks hierarchy level or updates already aquired lock. It should be called when there is a need to aquired hierarchy level lock before continuing.
    //! In case lock is already aquired all subsequent calls should update lock in order to avoid it timeouting.
    //! If hierarchy level is locked by other process this method blocks until it can aquire lock. This behavior can be changed by passing 'LockOptions::DisableLockWait' flag
    bool Lock(int options = 0) {return _Lock(options);}
    //! Unlocks hierarchy level or does nothing if lock was not aquired.
    void Unlock() {_Unlock();}
    //! Blocks until hierarchy level is unlocked. This method should be called when lock is not required but there is need to make sure that hierarchy level is not locked by
    //! other process before continuing. This method does not aquire any locks and should not be used to check if lock could be aquired as it does not guarantee that other process
    //! won't aquire lock between 'WaitForUnlock' and 'Lock' calls.
    void WaitForUnlock() {_WaitForUnlock();}
};

/*=================================================================================**//**
* @bsiclass
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
    virtual NavNodePtr _GetNode(BeGuidCR nodeId) const = 0;
    virtual NodeVisibility _GetNodeVisibility(BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) const = 0;
    virtual bvector<uint64_t> _GetNodeIndex(BeGuidCR hierarchyLevelId, BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) const = 0;
    virtual NavNodePtr _GetPhysicalParentNode(BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) const = 0;
    virtual BeGuid _GetVirtualParentNodeId(BeGuidCR nodeId) const = 0;

    virtual BeGuid _FindPhysicalHierarchyLevelId(CombinedHierarchyLevelIdentifier const&) const = 0;
    virtual BeGuid _FindHierarchyLevelId(Utf8CP connectionId, Utf8CP rulesetId, BeGuidCR virtualParentNodeId, BeGuidCR removalId) const = 0;
#ifdef wip_enable_display_label_postprocessor
    virtual bvector<DataSourceInfo> _FindDataSources(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&, int partsToGet) const = 0;
#endif
    virtual DataSourceInfo _FindDataSource(DataSourceIdentifier const&, RulesetVariables const&, int partsToGet) const = 0;

    virtual NavNodesProviderPtr _GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const&, bool) const = 0;
    virtual NavNodesProviderPtr _GetHierarchyLevel(NavNodesProviderContextR context, BeGuidCR, bool) const = 0;

    virtual std::unique_ptr<DirectNodesIterator> _GetCachedDirectNodesIterator(NavNodesProviderContextCR, DataSourceIdentifier const&) const = 0;

    virtual void _Cache(HierarchyLevelIdentifier&) = 0;
    virtual void _Cache(DataSourceInfo&) = 0;
    virtual void _Cache(NavNodeR, DataSourceIdentifier const&, bvector<uint64_t> const&, NodeVisibility) = 0;

    virtual void _Update(DataSourceInfo const&, int partsToUpdate) = 0;

    virtual void _MakeVirtual(BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) = 0;
    virtual void _MakeHidden(BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) = 0;

    virtual bool _IsCombinedHierarchyLevelInitialized(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&, InstanceFilterDefinitionCP) const = 0;
    virtual bool _IsHierarchyLevelInitialized(BeGuidCR, RulesetVariables const&, InstanceFilterDefinitionCP) const = 0;
    virtual bool _IsDataSourceInitialized(BeGuidCR) const = 0;

    virtual SavepointPtr _CreateSavepoint(bool) = 0;

    virtual BeMutex& _GetMutex() = 0;

public:
    virtual ~IHierarchyCache() {}

    NavNodePtr GetNode(BeGuidCR nodeId) const {return _GetNode(nodeId);}
    NodeVisibility GetNodeVisibility(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const {return _GetNodeVisibility(nodeId, contextVariables, instanceFilter);}
    bvector<uint64_t> GetNodeIndex(BeGuidCR hierarchyLevelId, BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const { return _GetNodeIndex(hierarchyLevelId, nodeId, contextVariables, instanceFilter); }
    NavNodePtr GetPhysicalParentNode(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) const {return _GetPhysicalParentNode(nodeId, contextVariables, instanceFilter);}
    BeGuid GetVirtualParentNodeId(BeGuidCR nodeId) const {return _GetVirtualParentNodeId(nodeId);}

    BeGuid FindPhysicalHierarchyLevelId(CombinedHierarchyLevelIdentifier const& identifier) const {return _FindPhysicalHierarchyLevelId(identifier);}
    BeGuid FindHierarchyLevelId(Utf8CP connectionId, Utf8CP rulesetId, BeGuidCR virtualParentNodeId, BeGuidCR removalId) const {return _FindHierarchyLevelId(connectionId, rulesetId, virtualParentNodeId, removalId);}
#ifdef wip_enable_display_label_postprocessor
    bvector<DataSourceInfo> FindDataSources(CombinedHierarchyLevelIdentifier const& hlIdentifier, RulesetVariables const& variables, int partsToGet = 0) const { return _FindDataSources(hlIdentifier, variables, partsToGet); }
#endif
    DataSourceInfo FindDataSource(DataSourceIdentifier const& identifier, RulesetVariables const& variables, int partsToGet = 0) const {return _FindDataSource(identifier, variables, partsToGet);}

    //! Get data source for the combined hierarchy level for specified physical parent node
    NavNodesProviderPtr GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const& info, bool onlyInitialized = true) const {return _GetCombinedHierarchyLevel(context, info, onlyInitialized);}
    //! Get data source for the whole hierarchy level
    NavNodesProviderPtr GetHierarchyLevel(NavNodesProviderContextR context, BeGuidCR id, bool onlyInitialized = true) const {return _GetHierarchyLevel(context, id, onlyInitialized);}

    std::unique_ptr<DirectNodesIterator> GetCachedDirectNodesIterator(NavNodesProviderContextCR context, DataSourceIdentifier const& identifier) const {return _GetCachedDirectNodesIterator(context, identifier);}

    void Cache(HierarchyLevelIdentifier& info) {_Cache(info);}
    void Cache(DataSourceInfo& info) {_Cache(info);}
    void Cache(NavNodeR node, DataSourceIdentifier const& dsInfo, bvector<uint64_t> const& index, NodeVisibility visibility) {_Cache(node, dsInfo, index, visibility);}
    void Cache(NavNodeR node, DataSourceIdentifier const& dsInfo, uint64_t index, NodeVisibility visibility) {_Cache(node, dsInfo, {index}, visibility);}

    void Update(DataSourceInfo const& info, int partsToUpdate) {_Update(info, partsToUpdate);}

    void MakeVirtual(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) {_MakeVirtual(nodeId, contextVariables, instanceFilter);}
    void MakeHidden(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP instanceFilter) {_MakeHidden(nodeId, contextVariables, instanceFilter);}

    bool IsCombinedHierarchyLevelInitialized(CombinedHierarchyLevelIdentifier const& info, RulesetVariables const& variables, InstanceFilterDefinitionCP instanceFilter) const {return _IsCombinedHierarchyLevelInitialized(info, variables, instanceFilter);}
    bool IsHierarchyLevelInitialized(BeGuidCR id, RulesetVariables const& variables, InstanceFilterDefinitionCP instanceFilter) const {return _IsHierarchyLevelInitialized(id, variables, instanceFilter);}
    bool IsDataSourceInitialized(BeGuidCR id) const {return _IsDataSourceInitialized(id);}

    SavepointPtr CreateSavepoint(bool bulkTransaction = false) {return _CreateSavepoint(bulkTransaction);}

    BeMutex& GetMutex() { return _GetMutex(); }
};

#define NODESCACHE_TABLENAME_HierarchyLevels         "HierarchyLevels"
#define NODESCACHE_TABLENAME_DataSources             "DataSources"
#define NODESCACHE_TABLENAME_DataSourceClasses       "DataSourceClasses"
#define NODESCACHE_TABLENAME_DataSourceNodes         "DataSourceNodes"
#define NODESCACHE_TABLENAME_Variables               "Variables"
#define NODESCACHE_TABLENAME_Nodes                   "Nodes"
#define NODESCACHE_TABLENAME_NodeKeys                "NodeKeys"
#define NODESCACHE_TABLENAME_NodeInstances           "NodeInstances"
#define NODESCACHE_TABLENAME_MergedNodes             "MergedNodes"
#define NODESCACHE_TABLENAME_Rulesets                "Rulesets"

#define NODESCACHE_QUICK_Size                   10

#define NAVNODES_CACHE_BINARY_INDEX
#define NAVNODES_CACHE_BINARY_GUID
//#define NAVNODES_CACHE_DEBUG
//#define NAVNODES_CACHE_DEBUG_REMOVE_DB
#ifdef NAVNODES_CACHE_DEBUG
    #undef NAVNODES_CACHE_BINARY_INDEX  // binary index is hard to read - use string index when debugging
    #undef NAVNODES_CACHE_BINARY_GUID   // use string GUID when debugging
    #define NAVNODES_CACHE_PROFILE_DB   // create a profile db
#endif

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct INavNodesCache : IHierarchyCache, INavNodeLocater
{
protected:
    virtual void _OnRulesetUsed(PresentationRuleSetCR) {}
    virtual bool _IsHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const&) const = 0;
    virtual std::shared_ptr<IHierarchyLevelLocker> _CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const&) = 0;

public:
    void OnRulesetUsed(PresentationRuleSetCR ruleset) {_OnRulesetUsed(ruleset);}
    bool IsHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const& identifier) const {return _IsHierarchyLevelLocked(identifier);}
    ECPRESENTATION_EXPORT std::shared_ptr<IHierarchyLevelLocker> CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const& identifier, bool lock = false);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCache : INavNodesCache
{
    struct Savepoint;
    friend struct NodesCacheHierarchyLevelLocker;
    friend struct NodesCacheWrapper;

    struct DbFactory
        {
        private:
            IConnectionCR m_connection;
            uint64_t m_sizeLimit;
            Nullable<uint64_t> m_memoryCacheLimit;
            Utf8String m_cachePath;
            bool m_deleteDb;
            BeMutex m_mutex;

        private:
            static DbResult CheckCacheCompatibility(BeSQLite::Db& db, IConnectionCR connection);
            static DbResult CreateCacheDb(IConnectionCR connection, BeSQLite::Db& db, BeFileNameCR path, DefaultTxn txnLockType, RefCountedPtr<BusyRetry> busyHandler);
            static DbResult OpenCacheDb(IConnectionCR connection, BeSQLite::Db& db, BeFileNameCR path, DefaultTxn txnLockType, RefCountedPtr<BusyRetry> busyHandler);
            static DbResult CreateTempDiskDb(Db& db, BeFileNameCR baseFileName, IConnectionCR connection, NodesCacheType cacheType);
            static DbResult InitializeDiskDb(Db& db, BeFileNameCR directory, IConnectionCR connection, NodesCacheType cacheType, bool& tempCache);
            static void SetupDbConnection(Db& db, NodesCacheType type, Nullable<uint64_t> const& memoryCacheLimit);
            static BentleyStatus InitializeCacheTables(Db& db);
            DbResult OpenMemoryDb(Db& db) const;
            DbResult OpenDiskDb(Db& db) const;

        public:
            DbFactory(IConnectionCR connection, Utf8CP path, uint64_t sizeLimit, Nullable<uint64_t> memoryCacheLimit, bool deleteDb = false)
                : m_connection(connection), m_cachePath(path), m_sizeLimit(sizeLimit), m_memoryCacheLimit(memoryCacheLimit), m_deleteDb(deleteDb)
                {}
            ~DbFactory();
            ECPRESENTATION_EXPORT static std::shared_ptr<DbFactory> Create(IConnectionCR, BeFileNameCR, NodesCacheType, uint64_t, Nullable<uint64_t> const& memoryCacheLimit);
            ECPRESENTATION_EXPORT DbResult CreateDbConnection(Db& db) const;

            IConnectionCR GetConnection() const {return m_connection;}
            Utf8StringCR GetCachePath() const {return m_cachePath;}
            uint64_t GetSizeLimit() const {return m_sizeLimit;}
            void SetSizeLimit(uint64_t limit) {BeMutexHolder lock(m_mutex); m_sizeLimit = limit;}
        };

private:
    std::shared_ptr<DbFactory> m_dbFactory;
    NavNodesFactory const& m_nodesFactory;
    INodesProviderContextFactoryCR m_contextFactory;
    INodesProviderFactoryCR m_providersFactory;
    bool m_ensureThreadSafety;
    mutable BeSQLite::Db m_db;
    mutable BeSQLite::StatementCache m_statements;
    mutable bvector<bpair<BeGuid, NavNodePtr>> m_quickNodesCache;
    bvector<std::unique_ptr<BeSQLite::DbFunction>> m_customFunctions;
    mutable BeMutex m_mutex;
    mutable BeMutex m_quickCacheMutex;

private:
    ECPRESENTATION_EXPORT NodesCache(std::shared_ptr<DbFactory>, NavNodesFactoryCR, INodesProviderContextFactoryCR, INodesProviderFactoryCR, bool);
    BentleyStatus OpenCache();
    void Optimize();

    BeGuid CacheOrGetPhysicalHierarchyLevel(CombinedHierarchyLevelIdentifier const& info);
    BeGuid CacheHierarchyLevel(BeGuidCR parentNodeId, Utf8StringCR rulesetId, BeGuidCR removalId);
    void CacheNode(BeGuidCR hierarchyLevelId, NavNodeR);
    void CacheDataSourceNodeAssociation(BeGuidCR datasourceId, BeGuidCR nodeId, bvector<uint64_t> const& fullIndex, NodeVisibility);
    void CacheEmptyDataSource(DataSourceIdentifier&, DataSourceFilter const&, BeGuidCR variablesId, Utf8StringCR specificationHash, Utf8StringCR nodeTypes,
        BeGuidCR parentId, Nullable<size_t> const& directNodesCount, bool isFinalized, Nullable<bool> const& hasNodes, Nullable<size_t> const& nodesCount);
    void CacheRelatedClassIds(BeGuidCR datasourceId, bmap<ECClassId, bool> const&);
    BeGuid CacheRelatedVariables(BeGuidCR rulesetId, RulesetVariables const& variables);
    void CacheNodeKey(NavNodeCR);
    void CacheNodeInstanceKeys(BeGuidCR nodeId, bvector<ECClassInstanceKey> const&);
    void CacheMergedNodeIds(NavNodeCR);
    void ChangeVisibility(BeGuidCR nodeId, BeGuidCR dataSourceId, NodeVisibility visibility);
    void ResetDataSource(DataSourceIdentifier const&);
    DataSourceInfo CreateDataSourceInfo(DataSourceIdentifier, int partsToGet) const;
    BeGuid GetNodeDataSourceId(BeGuidCR nodeId, RulesetVariables const& contextVariables, InstanceFilterDefinitionCP) const;
    BeGuid GetHierarchyLevelRulesetId(BeGuidCR hierarchyLevelId) const;

    void AddQuick(NavNodeR) const;
    void RemoveQuick(BeGuidCR) const;
    NavNodePtr GetQuick(BeGuidCR) const;

    uint64_t UpdateHierarchyLevelLock(BeGuidCR levelId);
    void RemoveHierarchyLevelLock(BeGuidCR levelId);
    bool IsAnyChildHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const& identifier) const;

    SavepointPtr CreateSavepointInternal(BeSQLiteTxnMode txnMode = BeSQLiteTxnMode::Immediate, bool optimize = false);

protected:
    // IHierarchyCache
    ECPRESENTATION_EXPORT NavNodePtr _GetNode(BeGuidCR) const override;
    ECPRESENTATION_EXPORT NodeVisibility _GetNodeVisibility(BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) const override;
    ECPRESENTATION_EXPORT bvector<uint64_t> _GetNodeIndex(BeGuidCR hierarchyLevelId, BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) const override;
    ECPRESENTATION_EXPORT NavNodePtr _GetPhysicalParentNode(BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) const override;
    ECPRESENTATION_EXPORT BeGuid _GetVirtualParentNodeId(BeGuidCR nodeId) const override;

    ECPRESENTATION_EXPORT BeGuid _FindPhysicalHierarchyLevelId(CombinedHierarchyLevelIdentifier const&) const override;
    ECPRESENTATION_EXPORT BeGuid _FindHierarchyLevelId(Utf8CP connectionId, Utf8CP rulesetId, BeGuidCR virtualParentNodeId, BeGuidCR removalId) const override;
#ifdef wip_enable_display_label_postprocessor
    ECPRESENTATION_EXPORT bvector<DataSourceInfo> _FindDataSources(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&, int) const override;
#endif
    ECPRESENTATION_EXPORT DataSourceInfo _FindDataSource(DataSourceIdentifier const&, RulesetVariables const&, int) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetCombinedHierarchyLevel(NavNodesProviderContextR, CombinedHierarchyLevelIdentifier const&, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetHierarchyLevel(NavNodesProviderContextR, BeGuidCR, bool) const override;
    ECPRESENTATION_EXPORT std::unique_ptr<DirectNodesIterator> _GetCachedDirectNodesIterator(NavNodesProviderContextCR, DataSourceIdentifier const&) const override;
    ECPRESENTATION_EXPORT void _Cache(HierarchyLevelIdentifier&) override;
    ECPRESENTATION_EXPORT void _Cache(DataSourceInfo&) override;
    ECPRESENTATION_EXPORT void _Cache(NavNodeR, DataSourceIdentifier const&, bvector<uint64_t> const&, NodeVisibility) override;
    ECPRESENTATION_EXPORT void _Update(DataSourceInfo const&, int partsToUpdate) override;
    ECPRESENTATION_EXPORT void _MakeVirtual(BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) override;
    ECPRESENTATION_EXPORT void _MakeHidden(BeGuidCR nodeId, RulesetVariables const&, InstanceFilterDefinitionCP) override;
    ECPRESENTATION_EXPORT bool _IsCombinedHierarchyLevelInitialized(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&, InstanceFilterDefinitionCP) const override;
    ECPRESENTATION_EXPORT bool _IsHierarchyLevelInitialized(BeGuidCR, RulesetVariables const&, InstanceFilterDefinitionCP) const override;
    ECPRESENTATION_EXPORT bool _IsDataSourceInitialized(BeGuidCR) const override;
    ECPRESENTATION_EXPORT SavepointPtr _CreateSavepoint(bool) override;
    ECPRESENTATION_EXPORT std::shared_ptr<IHierarchyLevelLocker> _CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const& identifier) override;
    BeMutex& _GetMutex() override { return m_mutex; }

    // INavNodeLocater
    ECPRESENTATION_EXPORT NavNodeCPtr _LocateNode(IConnectionCR, Utf8StringCR, NavNodeKeyCR) const override;

    // INavNodesCache
    ECPRESENTATION_EXPORT void _OnRulesetUsed(PresentationRuleSetCR) override;
    ECPRESENTATION_EXPORT bool _IsHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const&) const override;

public:
    ECPRESENTATION_EXPORT static std::shared_ptr<NodesCache> Create(std::shared_ptr<DbFactory>, NavNodesFactoryCR, INodesProviderContextFactoryCR,
        INodesProviderFactoryCR, bool ensureThreadSafety = true);
    ECPRESENTATION_EXPORT static std::shared_ptr<NodesCache> Create(IConnectionCR, BeFileNameCR, NavNodesFactoryCR, INodesProviderContextFactoryCR,
        INodesProviderFactoryCR, NodesCacheType type, bool ensureThreadSafety = true);
    ECPRESENTATION_EXPORT ~NodesCache();

    ECPRESENTATION_EXPORT void SetRemovalId(CombinedHierarchyLevelIdentifier const&);
    ECPRESENTATION_EXPORT void RemoveHierarchyLevels(BeSQLite::BeGuidCR removalId);

    ECPRESENTATION_EXPORT bset<CombinedHierarchyLevelIdentifier> GetRelatedHierarchyLevels(IConnectionCR connection, bset<ECInstanceKey> const&) const;

    ECPRESENTATION_EXPORT void Clear(Utf8CP rulesetId = nullptr);
    ECPRESENTATION_EXPORT void Persist();

    IConnectionCR GetConnection() const {return m_dbFactory->GetConnection();}
    NavNodesFactory const& GetNodesFactory() const {return m_nodesFactory;}
    INodesProviderContextFactoryCR GetNodesProviderContextFactory() const {return m_contextFactory;}
    INodesProviderFactoryCR GetNodesProviderFactory() const {return m_providersFactory;}
    BeSQLite::Db const& GetDb() const {return m_db;}
    BeSQLite::Db& GetDb() {return m_db;}
    void SetCacheFileSizeLimit(uint64_t size) {m_dbFactory->SetSizeLimit(size);}

    void RemoveQuick(std::function<bool(NavNodeCR)> const&) const;

    ECPRESENTATION_EXPORT NavNodesProviderPtr GetUndeterminedNodesProvider(NavNodesProviderContextR) const;
    ECPRESENTATION_EXPORT NavNodesProviderPtr GetFilteredNodesProvider(NavNodesProviderContextR, Utf8CP) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct INodesCacheManager
{
protected:
    virtual std::shared_ptr<INavNodesCache> _GetCache(Utf8StringCR connectionId, BeGuidCR rootNodeId) const = 0;
    virtual std::shared_ptr<NodesCache> _GetPersistentCache(Utf8StringCR connectionId) const = 0;
    virtual void _ClearCaches(Utf8CP rulesetId) const = 0;

public:
    virtual ~INodesCacheManager() {}
    std::shared_ptr<INavNodesCache> GetCache(Utf8StringCR connectionId, BeGuidCR rootNodeId = BeGuid()) const {return _GetCache(connectionId, rootNodeId);}
    std::shared_ptr<NodesCache> GetPersistentCache(Utf8StringCR connectionId) const {return _GetPersistentCache(connectionId);}
    void ClearCaches(Utf8CP rulesetId) const {_ClearCaches(rulesetId);}
};

/*=================================================================================**//**
* IHierarchyLevelLocker implementation that locks hierarchy levels in NodesCache
* using timestamps. Aquired lock is held until 'Unlock' is called or
* NodesCacheHierarchyLevelLocker is destroyed.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheHierarchyLevelLocker : IHierarchyLevelLocker
{
    enum class LockResult
        {
        LockAquired,
        LockNotAquired,
        LockFailed,
        };

private:
    NodesCache& m_cache;
    CombinedHierarchyLevelIdentifier m_hierarchyLevelIdentifier;
    BeGuid m_hierarchyLevelId;
    Nullable<uint64_t> m_lockTimestamp;

private:
    LockResult TryLockHierarchyLevel(int);

protected:
    ECPRESENTATION_EXPORT virtual bool _Lock(int) override;
    ECPRESENTATION_EXPORT virtual void _Unlock() override;
    ECPRESENTATION_EXPORT virtual void _WaitForUnlock() override;

protected:
    CombinedHierarchyLevelIdentifier const& GetHierarchyLevelIdentifier() const {return m_hierarchyLevelIdentifier;}

public:
    NodesCacheHierarchyLevelLocker(NodesCache& cache, CombinedHierarchyLevelIdentifier const& identifier)
        : m_cache(cache), m_hierarchyLevelIdentifier(identifier)
        {}
    ~NodesCacheHierarchyLevelLocker() {Unlock();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SqliteCacheDirectNodeIteratorBase : DirectNodesIterator
{
private:
    Db& m_db;
    BeSQLite::StatementCache& m_statements;
    BeMutex& m_cacheMutex;
    NavNodesProviderContextCPtr m_context;
    mutable Nullable<bool> m_hasVirtualNodes;
    mutable Nullable<size_t> m_nodesCount;
    mutable Nullable<int> m_offset;
    std::unique_ptr<bvector<NavNodePtr>> m_loadedNodes;
    int m_currNodeIndex;
    int m_pageSize;

private:
    size_t GetTotalNodesCount() const;
    CachedStatementPtr GetNodesPageStatement() const;
    void LoadNodesPage();

protected:
    virtual bool _QueryHasVirtualNodes() const = 0;
    virtual size_t _QueryTotalNodesCount() const = 0;
    virtual Utf8CP _GetNodesQuery() const = 0;
    virtual void _BindNodesStatement(StatementR, int&) const = 0;

protected:
    bool _SkippedNodesToPageStart() const override;
    size_t _NodesCount() const override;
    NavNodePtr _NextNode() override;

protected:
    SqliteCacheDirectNodeIteratorBase(NavNodesProviderContextCR context, Db& db, StatementCache& statements, BeMutex& cacheMutex)
        : m_context(&context), m_db(db), m_statements(statements), m_cacheMutex(cacheMutex), m_pageSize(0), m_currNodeIndex(0)
        {
        m_pageSize = m_context->GetOptimizationFlags().GetMaxNodesToLoad();
        if (m_pageSize == 0 && m_context->HasPageOptions() && m_context->GetPageOptions()->HasSize())
            m_pageSize = m_context->GetPageOptions()->GetSize();
        }
    Db& GetDb() const {return m_db;}
    StatementCache& GetStatements() const {return m_statements;}
    NavNodesProviderContextCR GetContext() const {return *m_context;}
    int GetPageSize() const {return m_pageSize;}
    int GetOffset() const;
    bool HasVirtualNodes() const;

public:
    static std::unique_ptr<DirectNodesIterator> CreateForHierarchyLevel(NavNodesProviderContextCR context, Db& db, StatementCache& statements, BeMutex& mutex, BeGuid hierarchyLevelId);
    static std::unique_ptr<DirectNodesIterator> CreateForDataSource(NavNodesProviderContextCR context, Db& db, StatementCache& statements, BeMutex& mutex, BeGuid dataSourceId);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
