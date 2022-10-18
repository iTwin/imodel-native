/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationTypes.h>
#include "NavNodesCache.h"
#include "NavNodesCacheHelpers.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheWrapper : INavNodesCache
{
private:
    NodesCache& m_diskCache;
    std::shared_ptr<NodesCache> m_memoryCache;
    std::shared_ptr<IHierarchyLevelLocker> m_hierarchyLevelLocker;
    BeGuid m_rootParentId;
    bset<Utf8String> m_rulesetsUsed;
    BeMutex m_mutex;

    bset<BeGuid> m_hierarchyLevelsFromDisk;
    bset<BeGuid> m_updatedNodeIds;

private:
    ECPRESENTATION_EXPORT void PersistMemoryData();
    ECPRESENTATION_EXPORT void EnsureParentNodesExist(HierarchyLevelIdentifier const& identifier);
    ECPRESENTATION_EXPORT void EnsureParentNodesExist(bvector<BeGuid> const& parents);
    ECPRESENTATION_EXPORT void EnsureNodeExists(BeGuidCR nodeId);
    ECPRESENTATION_EXPORT void EnsureDataSourceExists(BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT void EnsureHierarchyLevelExists(BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT void EnsureRulesetExists(Utf8StringCR rulesetId);
    ECPRESENTATION_EXPORT bool ShouldTakeHierarchyLevelFromMemory(Utf8CP rulesetId, BeGuidCR parentId, bool isPhysical) const;
    ECPRESENTATION_EXPORT bool ShouldTakeDataSourceFromMemory(BeGuidCR dataSourceId, BeGuidCR hierarchyLevelId) const;
    ECPRESENTATION_EXPORT bool HierarchyLevelExistInMemory(Utf8CP rulesetId, BeGuidCR parentId, bool isPhysical) const;
    ECPRESENTATION_EXPORT void InitializeMemoryCache();
    bool IsMemoryCacheInitialized() const {return nullptr != m_memoryCache;}
    ECPRESENTATION_EXPORT void LoadRootHierarchyLevels(Utf8StringCR rulesetId);

protected:
    // IHierarchyCache
    ECPRESENTATION_EXPORT NavNodePtr _GetNode(BeGuidCR nodeId) const override;
    ECPRESENTATION_EXPORT NodeVisibility _GetNodeVisibility(BeGuidCR nodeId) const override;
    ECPRESENTATION_EXPORT bvector<uint64_t> _GetNodeIndex(BeGuidCR nodeId) const override;

    ECPRESENTATION_EXPORT HierarchyLevelIdentifier _FindHierarchyLevel(Utf8CP connectionId, Utf8CP rulesetId, BeGuidCR virtualParentNodeId, BeGuidCR removalId) const override;
#ifdef wip_enable_display_label_postprocessor
    ECPRESENTATION_EXPORT bvector<DataSourceInfo> _FindDataSources(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&, int partsToGet) const override;
#endif
    ECPRESENTATION_EXPORT DataSourceInfo _FindDataSource(DataSourceIdentifier const&, RulesetVariables const&, int partsToGet) const override;
    ECPRESENTATION_EXPORT DataSourceInfo _FindDataSource(BeGuidCR nodeId, int partsToGet) const override;

    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const&, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetHierarchyLevel(NavNodesProviderContextR context, HierarchyLevelIdentifier const&, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetDataSource(NavNodesProviderContextR context, DataSourceIdentifier const&, bool, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetDataSource(NavNodesProviderContextR context, BeGuidCR nodeId, bool, bool) const override;

    ECPRESENTATION_EXPORT void _Cache(HierarchyLevelIdentifier&) override;
    ECPRESENTATION_EXPORT void _Cache(DataSourceInfo&) override;
    ECPRESENTATION_EXPORT void _Cache(NavNodeR, DataSourceIdentifier const&, bvector<uint64_t> const&, NodeVisibility) override;

    ECPRESENTATION_EXPORT void _Update(DataSourceInfo const&, int partsToUpdate) override;

    ECPRESENTATION_EXPORT void _MakePhysical(NavNodeCR) override;
    ECPRESENTATION_EXPORT void _MakeVirtual(NavNodeCR) override;
    ECPRESENTATION_EXPORT void _MakeHidden(NavNodeCR) override;

    ECPRESENTATION_EXPORT bool _IsInitialized(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&) const override;
    ECPRESENTATION_EXPORT bool _IsInitialized(HierarchyLevelIdentifier const&, RulesetVariables const&) const override;
    ECPRESENTATION_EXPORT bool _IsInitialized(DataSourceIdentifier const&, RulesetVariables const&) const override;

    ECPRESENTATION_EXPORT SavepointPtr _CreateSavepoint(bool) override;
    BeMutex& _GetMutex()  override { return m_mutex; }
    ECPRESENTATION_EXPORT std::shared_ptr<IHierarchyLevelLocker> _CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const& identifier) override;

    // INavNodeLocater
    ECPRESENTATION_EXPORT NavNodeCPtr _LocateNode(IConnectionCR connection, Utf8StringCR rulesetId, NavNodeKeyCR key, RulesetVariables const& variables) const override;

    // INavNodesCache
    ECPRESENTATION_EXPORT void _OnRulesetUsed(PresentationRuleSetCR ruleset) override;
    ECPRESENTATION_EXPORT void _OnRulesetVariablesUsed(RulesetVariables const& variables, Utf8StringCR rulesetId) override;

    ECPRESENTATION_EXPORT BeGuid _FindPhysicalHierarchyLevelId(CombinedHierarchyLevelIdentifier const& identifier) const override;
    ECPRESENTATION_EXPORT bool _IsHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const&) const override;

public:
    NodesCacheWrapper(NodesCache& diskCache, BeGuid parentNodeId = BeGuid())
        : m_diskCache(diskCache), m_rootParentId(parentNodeId)
        {}
    ECPRESENTATION_EXPORT ~NodesCacheWrapper();

    NodesCache* GetMemoryCache() {return m_memoryCache.get();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
