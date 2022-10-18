/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "../../../Source/Hierarchies/NavNodeProviders.h"
#include "../../../Source/Hierarchies/NavNodesCache.h"
#include "TestNavNode.h"
#include "TestNodesProvider.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNodesCache : INavNodesCache
{
    typedef std::function<NavNodePtr(BeGuid)> GetNodeHandler;
    typedef std::function<NavNodesProviderPtr(HierarchyLevelIdentifier const&)> GetHierarchyDataSourceHandler;
    typedef std::function<NavNodesProviderPtr(DataSourceIdentifier const&)> GetVirtualDataSourceHandler;
    typedef std::function<NavNodesProviderPtr(BeGuid)> GetParentNodeDataSourceHandler;
    typedef std::function<void(HierarchyLevelIdentifier&)> CacheHierarchyLevelHandler;
    typedef std::function<void(DataSourceInfo&)> CacheDataSourceHandler;
    typedef std::function<void(NavNodeR, NodeVisibility)> CacheNodeHandler;
    typedef std::function<void(NavNodeCR)> NodeHandler;
    typedef std::function<void(DataSourceInfo const&, int)> UpdateDataSourceHandler;
    typedef std::function<NavNodeCPtr(IConnectionCR, Utf8StringCR, NavNodeKeyCR)> LocateNodeHandler;

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct Savepoint : IHierarchyCache::Savepoint
        {
        TestNodesCache& m_cache;
        bmap<BeGuid, NavNodePtr> m_nodes;
        bmap<BeGuid, bvector<uint64_t>> m_nodeIndexes;
        bmap<CombinedHierarchyLevelIdentifier, bvector<DataSourceIdentifier>> m_physicalHierarchy;
        bmap<HierarchyLevelIdentifier, bvector<DataSourceIdentifier>> m_virtualHierarchy;
        bmap<DataSourceIdentifier, bpair<DataSourceInfo, bvector<NavNodePtr>>> m_partialHierarchies;
        bset<BeGuid> m_finalizedDataSources;
        Savepoint(TestNodesCache& cache)
            : m_cache(cache)
            {
            BeMutexHolder lock(m_cache.GetMutex());
            m_nodes = cache.m_nodes;
            m_nodeIndexes = cache.m_nodeIndexes;
            m_partialHierarchies = cache.m_partialHierarchies;
            m_physicalHierarchy = cache.m_physicalHierarchy;
            m_virtualHierarchy = cache.m_virtualHierarchy;
            m_finalizedDataSources = cache.m_finalizedDataSources;
            }
        void _Cancel() override
            {
            BeMutexHolder lock(m_cache.GetMutex());
            m_cache.m_nodes = m_nodes;
            m_cache.m_nodeIndexes = m_nodeIndexes;
            m_cache.m_partialHierarchies = m_partialHierarchies;
            m_cache.m_physicalHierarchy = m_physicalHierarchy;
            m_cache.m_virtualHierarchy = m_virtualHierarchy;
            m_cache.m_finalizedDataSources = m_finalizedDataSources;
            }
        };

private:
    mutable BeMutex m_mutex;

    bmap<BeGuid, NavNodePtr> m_nodes;
    bmap<BeGuid, bvector<uint64_t>> m_nodeIndexes;
    bmap<CombinedHierarchyLevelIdentifier, bvector<DataSourceIdentifier>> m_physicalHierarchy;
    bmap<HierarchyLevelIdentifier, bvector<DataSourceIdentifier>> m_virtualHierarchy;
    bmap<DataSourceIdentifier, bpair<DataSourceInfo, bvector<NavNodePtr>>> m_partialHierarchies;
    bset<BeGuid> m_finalizedDataSources;
    bmap<BeGuid, NodeVisibility> m_nodeVisibilities;

    GetNodeHandler m_getNodeHandler;
    GetHierarchyDataSourceHandler m_getHierarchyDataSourceHandler;
    GetVirtualDataSourceHandler m_getVirtualDataSourceHandler;
    GetParentNodeDataSourceHandler m_getParentNodeDataSourceHandler;
    CacheHierarchyLevelHandler m_cacheHierarchyLevelHandler;
    CacheDataSourceHandler m_cacheDataSourceHandler;
    CacheNodeHandler m_cacheNodeHandler;
    NodeHandler m_makePhysicalHandler;
    NodeHandler m_makeVirtualHandler;
    NodeHandler m_makeHiddenHandler;
    UpdateDataSourceHandler m_updateDataSourceHandler;
    LocateNodeHandler m_locateNodeHandler;

private:
    HierarchyLevelIdentifier GetHierarchyLevelIdentifier(NavNodeCR node) const
        {
        NavNodeExtendedData ex(node);
        return HierarchyLevelIdentifier(ex.GetConnectionId(), ex.GetRulesetId(), node.GetParentNodeId(), node.GetParentNodeId());
        }
    HierarchyLevelIdentifier GetHierarchyLevelIdentifier(DataSourceIdentifier const& dsInfo) const
        {
        for (auto const& entry : m_virtualHierarchy)
            {
            if (entry.first.GetId() == dsInfo.GetHierarchyLevelId())
                return entry.first;
            }
        ADD_FAILURE();
        return HierarchyLevelIdentifier();
        }
    bvector<NavNodePtr> GetFullHierarchyLevel(bvector<DataSourceIdentifier> const& partialInfos) const
        {
        bvector<NavNodePtr> vec;
        for (auto const& partialInfo : partialInfos)
            {
            auto iter = m_partialHierarchies.find(partialInfo);
            if (iter == m_partialHierarchies.end())
                continue;

            ContainerHelpers::Push(vec, iter->second.second);
            }
        return vec;
        }

protected:
    BeMutex& _GetMutex() override { return m_mutex; }
    NavNodePtr _GetNode(BeGuidCR nodeId) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getNodeHandler)
            return m_getNodeHandler(nodeId);

        auto iter = m_nodes.find(nodeId);
        return (m_nodes.end() != iter) ? iter->second : nullptr;
        }
    NodeVisibility _GetNodeVisibility(BeGuidCR nodeId) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_nodeVisibilities.find(nodeId);
        return (m_nodeVisibilities.end() != iter) ? iter->second : NodeVisibility::Hidden;
        }
    bvector<uint64_t> _GetNodeIndex(BeGuidCR nodeId) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_nodeIndexes.find(nodeId);
        return (m_nodeIndexes.end() != iter) ? iter->second : bvector<uint64_t>();
        }

    HierarchyLevelIdentifier _FindHierarchyLevel(Utf8CP connectionId, Utf8CP rulesetId, BeGuidCR virtualParentNodeId, BeGuidCR) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto const& entry : m_virtualHierarchy)
            {
            if ((!connectionId || entry.first.GetConnectionId().Equals(connectionId))
                && (!rulesetId || entry.first.GetRulesetId().Equals(rulesetId))
                && entry.first.GetVirtualParentNodeId() == virtualParentNodeId)
                return entry.first;
            }
        return HierarchyLevelIdentifier();
        }
#ifdef wip_enable_display_label_postprocessor
    bvector<DataSourceInfo> _FindDataSources(CombinedHierarchyLevelIdentifier const& hlIdentifier, RulesetVariables const&, int) const override
        {
        BeMutexHolder lock(m_mutex);
        auto dsIdentifiers = m_physicalHierarchy.find(hlIdentifier);
        if (dsIdentifiers != m_physicalHierarchy.end())
            {
            bvector<DataSourceInfo> result;
            for (DataSourceIdentifier const& id : dsIdentifiers->second)
                {
                auto dsInfo = m_partialHierarchies.find(id);
                if (dsInfo != m_partialHierarchies.end())
                    result.push_back(dsInfo->second.first);
                }
            }
        return bvector<DataSourceInfo>();
        }
#endif
    DataSourceInfo _FindDataSource(DataSourceIdentifier const& identifier, RulesetVariables const&, int) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto const& entry : m_partialHierarchies)
            {
            if (entry.first.GetHierarchyLevelId() == identifier.GetHierarchyLevelId() && entry.first.GetIndex() == identifier.GetIndex())
                return entry.second.first;
            }
        return DataSourceInfo();
        }
    DataSourceInfo _FindDataSource(BeGuidCR nodeId, int) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto const& entry : m_partialHierarchies)
            {
            for (NavNodeCPtr node : entry.second.second)
                {
                if (node->GetNodeId() == nodeId)
                    return entry.second.first;
                }
            }
        return DataSourceInfo();
        }
    NavNodesProviderPtr _GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const& info, bool) const override
        {
        BeMutexHolder lock(m_mutex);

        if (!IsInitialized(info, context.GetRulesetVariables()))
            return nullptr;

        auto iter = m_physicalHierarchy.find(info);
        return (m_physicalHierarchy.end() != iter) ? BVectorNodesProvider::Create(context, GetFullHierarchyLevel(iter->second)) : nullptr;
        }

    BeGuid _FindPhysicalHierarchyLevelId(CombinedHierarchyLevelIdentifier const&) const override {return BeGuid();}

    NavNodesProviderPtr _GetHierarchyLevel(NavNodesProviderContextR context, HierarchyLevelIdentifier const& info, bool) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getHierarchyDataSourceHandler)
            return m_getHierarchyDataSourceHandler(info);

        if (!IsInitialized(info, context.GetRulesetVariables()))
            return nullptr;

        auto iter = m_virtualHierarchy.find(info);
        return (m_virtualHierarchy.end() != iter) ? BVectorNodesProvider::Create(context, GetFullHierarchyLevel(iter->second)) : nullptr;
        }
    NavNodesProviderPtr _GetDataSource(NavNodesProviderContextR context, DataSourceIdentifier const& dsInfo, bool, bool) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getVirtualDataSourceHandler)
            return m_getVirtualDataSourceHandler(dsInfo);

        auto iter = m_partialHierarchies.find(dsInfo);
        return (m_partialHierarchies.end() != iter) ? BVectorNodesProvider::Create(context, iter->second.second) : nullptr;
        }
    NavNodesProviderPtr _GetDataSource(NavNodesProviderContextR context, BeGuidCR nodeId, bool, bool) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getParentNodeDataSourceHandler)
            return m_getParentNodeDataSourceHandler(nodeId);

        NavNodePtr node = GetNode(nodeId);
        for (auto const& entry : m_partialHierarchies)
            {
            for (NavNodePtr const& entryNode : entry.second.second)
                {
                if (entryNode == node)
                    return GetDataSource(context, entry.first);
                }
            }
        return nullptr;
        }

    void _Cache(HierarchyLevelIdentifier& info) override
        {
        BeMutexHolder lock(m_mutex);
        info.SetId(BeGuid(true));
        m_physicalHierarchy[info.GetCombined()] = bvector<DataSourceIdentifier>();
        m_virtualHierarchy[info] = bvector<DataSourceIdentifier>();

        if (m_cacheHierarchyLevelHandler)
            m_cacheHierarchyLevelHandler(info);
        }
    void _Cache(DataSourceInfo& info) override
        {
        BeMutexHolder lock(m_mutex);
        info.GetIdentifier().SetId(BeGuid(true));
        m_partialHierarchies[info.GetIdentifier()] = bpair<DataSourceInfo, bvector<NavNodePtr>>(info, bvector<NavNodePtr>());
        bvector<DataSourceIdentifier>& physicalHierarchy = m_physicalHierarchy[GetHierarchyLevelIdentifier(info.GetIdentifier()).GetCombined()];
        physicalHierarchy.insert(physicalHierarchy.begin() + info.GetIdentifier().GetIndex().back(), info.GetIdentifier());
        bvector<DataSourceIdentifier>& virtualHierarchy = m_virtualHierarchy[GetHierarchyLevelIdentifier(info.GetIdentifier())];
        virtualHierarchy.insert(virtualHierarchy.begin() + info.GetIdentifier().GetIndex().back(), info.GetIdentifier());

        if (m_cacheDataSourceHandler)
            m_cacheDataSourceHandler(info);
        }
    void _Cache(NavNodeR node, DataSourceIdentifier const& dsInfo, bvector<uint64_t> const& index, NodeVisibility visibility) override
        {
        BeMutexHolder lock(m_mutex);
        BeGuid nodeId(true);
        node.SetNodeId(nodeId);

        HierarchyLevelIdentifier hlInfo = GetHierarchyLevelIdentifier(dsInfo);

        m_nodes[nodeId] = &node;
        m_nodeIndexes[nodeId] = index;
        m_partialHierarchies[dsInfo].second.push_back(&node);
        m_nodeVisibilities.Insert(node.GetNodeId(), visibility);

        if (m_cacheNodeHandler)
            m_cacheNodeHandler(node, visibility);
        }

    void _MakePhysical(NavNodeCR node) override
        {
        BeMutexHolder lock(m_mutex);
        m_nodeVisibilities.erase(node.GetNodeId());
        m_nodeVisibilities.Insert(node.GetNodeId(), NodeVisibility::Visible);
        if (m_makePhysicalHandler)
            m_makePhysicalHandler(node);
        }
    void _MakeVirtual(NavNodeCR node) override
        {
        BeMutexHolder lock(m_mutex);
        m_nodeVisibilities.erase(node.GetNodeId());
        m_nodeVisibilities.Insert(node.GetNodeId(), NodeVisibility::Virtual);
        if (m_makeVirtualHandler)
            return m_makeVirtualHandler(node);
        }
    void _MakeHidden(NavNodeCR node) override
        {
        BeMutexHolder lock(m_mutex);
        m_nodeVisibilities.erase(node.GetNodeId());
        m_nodeVisibilities.Insert(node.GetNodeId(), NodeVisibility::Hidden);
        if (m_makeHiddenHandler)
            return m_makeHiddenHandler(node);
        }

    bool _IsInitialized(CombinedHierarchyLevelIdentifier const& info, RulesetVariables const& variables) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_physicalHierarchy.find(info);
        if (m_physicalHierarchy.end() == iter || iter->second.empty())
            return false;
        for (auto const& dsInfo : iter->second)
            {
            if (!IsInitialized(dsInfo, variables))
                return false;
            }
        return true;
        }
    bool _IsInitialized(HierarchyLevelIdentifier const& info, RulesetVariables const& variables) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_virtualHierarchy.find(info);
        if (m_virtualHierarchy.end() == iter || iter->second.empty())
            return false;
        for (auto const& dsInfo : iter->second)
            {
            if (!IsInitialized(dsInfo, variables))
                return false;
            }
        return true;
        }
    bool _IsInitialized(DataSourceIdentifier const& info, RulesetVariables const& variables) const override
        {
        BeMutexHolder lock(m_mutex);
        return m_finalizedDataSources.end() != m_finalizedDataSources.find(info.GetId());
        }

    void _Update(DataSourceInfo const& info, int partsToUpdate) override
        {
        BeMutexHolder lock(m_mutex);
        if (info.IsInitialized() && 0 != ((int)DataSourceInfo::PART_IsFinalized & partsToUpdate))
            m_finalizedDataSources.insert(info.GetIdentifier().GetId());
        if (m_updateDataSourceHandler)
            return m_updateDataSourceHandler(info, partsToUpdate);
        }

    NavNodeCPtr _LocateNode(IConnectionCR connection, Utf8StringCR rulesetId, NavNodeKeyCR key, RulesetVariables const&) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_locateNodeHandler)
            return m_locateNodeHandler(connection, rulesetId, key);
        return nullptr;
        }
    IHierarchyCache::SavepointPtr _CreateSavepoint(bool) override {return new Savepoint(*this);}

    void _OnRulesetUsed(PresentationRuleSetCR) override {}
    void _OnRulesetVariablesUsed(RulesetVariables const&, Utf8StringCR) override {}

    std::shared_ptr<IHierarchyLevelLocker> _CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const& identifier) override {return nullptr;}

    bool _IsHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const&) const override {return false;}

public:
    TestNodesCache() {}
    void SetGetNodeHandler(GetNodeHandler handler) {m_getNodeHandler = handler;}
    void SetGetHierarchyDataSourceHandler(GetHierarchyDataSourceHandler handler) {m_getHierarchyDataSourceHandler = handler;}
    void SetGetVirtualDataSourceHandler(GetVirtualDataSourceHandler handler) {m_getVirtualDataSourceHandler = handler;}
    void SetGetParentNodeDataSourceHandler(GetParentNodeDataSourceHandler handler) {m_getParentNodeDataSourceHandler = handler;}
    void SetCacheHierarchyLevelHandler(CacheHierarchyLevelHandler handler) {m_cacheHierarchyLevelHandler = handler;}
    void SetCacheDataSourceHandler(CacheDataSourceHandler handler) {m_cacheDataSourceHandler = handler;}
    void SetCacheNodeHandler(CacheNodeHandler handler) {m_cacheNodeHandler = handler;}
    void SetMakePhysicalHandler(NodeHandler handler) {m_makePhysicalHandler = handler;}
    void SetMakeVirtualHandler(NodeHandler handler) {m_makeVirtualHandler = handler;}
    void SetMakeHiddenHandler(NodeHandler handler) {m_makeHiddenHandler = handler;}
    void SetUpdateDataSourceHandler(UpdateDataSourceHandler handler) {m_updateDataSourceHandler = handler;}
    void SetLocateNodeHandler(LocateNodeHandler handler) {m_locateNodeHandler = handler;}
    size_t GetCachedChildrenCount(BeGuidCR parentId)
        {
        BeMutexHolder lock(m_mutex);
        return std::count_if(m_nodes.begin(), m_nodes.end(),
            [&](bpair<BeGuid, NavNodePtr> nodePair)
            {
            return parentId == nodePair.second->GetParentNodeId();
            });
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE
