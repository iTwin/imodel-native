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
    typedef std::function<NavNodePtr(BeGuidCR)> GetNodeHandler;
    typedef std::function<NavNodePtr(BeGuidCR)> GetPhysicalParentNodeHandler;
    typedef std::function<BeGuid(BeGuidCR)> GetParentNodeIdHandler;
    typedef std::function<void(HierarchyLevelIdentifier&)> CacheHierarchyLevelHandler;
    typedef std::function<void(DataSourceInfo&)> CacheDataSourceHandler;
    typedef std::function<void(NavNodeR, NodeVisibility)> CacheNodeHandler;

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
    GetPhysicalParentNodeHandler m_getPhysicalParentNodeHandler;
    CacheHierarchyLevelHandler m_cacheHierarchyLevelHandler;
    CacheDataSourceHandler m_cacheDataSourceHandler;
    CacheNodeHandler m_cacheNodeHandler;

private:
    HierarchyLevelIdentifier GetHierarchyLevelIdentifier(NavNodeCR node) const
        {
        NavNodeExtendedData ex(node);
        auto physicalParentNode = GetPhysicalParentNode(node.GetNodeId(), RulesetVariables(), "");
        return HierarchyLevelIdentifier(
            ex.GetConnectionId(),
            ex.GetRulesetId(),
            physicalParentNode.IsValid() ? physicalParentNode->GetNodeId() : BeGuid(),
            GetVirtualParentNodeId(node.GetNodeId())
            );
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
    NodeVisibility _GetNodeVisibility(BeGuidCR nodeId, RulesetVariables const&, Utf8StringCR) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_nodeVisibilities.find(nodeId);
        return (m_nodeVisibilities.end() != iter) ? iter->second : NodeVisibility::Hidden;
        }
    bvector<uint64_t> _GetNodeIndex(BeGuidCR hierarchyLevelId, BeGuidCR nodeId, RulesetVariables const&, Utf8StringCR) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_nodeIndexes.find(nodeId);
        return (m_nodeIndexes.end() != iter) ? iter->second : bvector<uint64_t>();
        }
    NavNodePtr _GetPhysicalParentNode(BeGuidCR nodeId, RulesetVariables const& v, Utf8StringCR instanceFilter) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getPhysicalParentNodeHandler)
            return m_getPhysicalParentNodeHandler(nodeId);
        DataSourceIdentifier dsId;
        for (auto const& dsEntry : m_partialHierarchies)
            {
            if (ContainerHelpers::Contains(dsEntry.second.second, [&](auto const& node){return node->GetNodeId() == nodeId;}))
                {
                dsId = dsEntry.first;
                break;
                }
            }
        if (!dsId.IsValid())
            return nullptr;
        for (auto const& vhEntry : m_virtualHierarchy)
            {
            if (vhEntry.first.GetId() != dsId.GetHierarchyLevelId())
                continue;

            BeGuidCR parentId = vhEntry.first.GetVirtualParentNodeId();
            if (!parentId.IsValid())
                return nullptr;

            if (NodeVisibility::Virtual == GetNodeVisibility(parentId, v, instanceFilter))
                return GetPhysicalParentNode(parentId, v, instanceFilter);

            return GetNode(parentId);
            }
        return nullptr;
        }
    BeGuid _GetVirtualParentNodeId(BeGuidCR nodeId) const override
        {
        BeMutexHolder lock(m_mutex);
        DataSourceIdentifier dsId;
        for (auto const& dsEntry : m_partialHierarchies)
            {
            if (ContainerHelpers::Contains(dsEntry.second.second, [&](auto const& node){return node->GetNodeId() == nodeId;}))
                {
                dsId = dsEntry.first;
                break;
                }
            }
        if (!dsId.IsValid())
            return BeGuid();
        for (auto const& vhEntry : m_virtualHierarchy)
            {
            if (vhEntry.first.GetId() == dsId.GetHierarchyLevelId())
                return vhEntry.first.GetVirtualParentNodeId();
            }
        return BeGuid();
        }

    BeGuid _FindHierarchyLevelId(Utf8CP connectionId, Utf8CP rulesetId, BeGuidCR virtualParentNodeId, BeGuidCR) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto const& entry : m_virtualHierarchy)
            {
            if ((!connectionId || entry.first.GetConnectionId().Equals(connectionId))
                && (!rulesetId || entry.first.GetRulesetId().Equals(rulesetId))
                && entry.first.GetVirtualParentNodeId() == virtualParentNodeId)
                return entry.first.GetId();
            }
        return BeGuid();
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
    NavNodesProviderPtr _GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const& info, bool) const override
        {
        BeMutexHolder lock(m_mutex);

        if (!IsCombinedHierarchyLevelInitialized(info, context.GetRulesetVariables(), context.GetInstanceFilter()))
            return nullptr;

        auto iter = m_physicalHierarchy.find(info);
        return (m_physicalHierarchy.end() != iter) ? BVectorNodesProvider::Create(context, GetFullHierarchyLevel(iter->second)) : nullptr;
        }

    BeGuid _FindPhysicalHierarchyLevelId(CombinedHierarchyLevelIdentifier const&) const override {return BeGuid();}

    NavNodesProviderPtr _GetHierarchyLevel(NavNodesProviderContextR context, BeGuidCR id, bool) const override
        {
        BeMutexHolder lock(m_mutex);

        if (!IsHierarchyLevelInitialized(id, context.GetRulesetVariables(), context.GetInstanceFilter()))
            return nullptr;

        for (auto const& entry : m_virtualHierarchy)
            {
            if (entry.first.GetId() == id)
                return BVectorNodesProvider::Create(context, GetFullHierarchyLevel(entry.second));
            }
        return nullptr;
        }

    std::unique_ptr<DirectNodesIterator> _GetCachedDirectNodesIterator(NavNodesProviderContextCR context, DataSourceIdentifier const& identifier) const override
        {
        BeMutexHolder lock(m_mutex);

        if (!IsDataSourceInitialized(identifier.GetId()))
            return nullptr;

        auto iter = m_partialHierarchies.find(identifier);
        return (m_partialHierarchies.end() != iter) ? std::make_unique<BVectorDirectNodesIterator>(iter->second.second) : nullptr;
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

    void _MakeVirtual(BeGuidCR nodeId, RulesetVariables const&, Utf8StringCR) override
        {
        BeMutexHolder lock(m_mutex);
        m_nodeVisibilities.erase(nodeId);
        m_nodeVisibilities.Insert(nodeId, NodeVisibility::Virtual);
        }
    void _MakeHidden(BeGuidCR nodeId, RulesetVariables const&, Utf8StringCR) override
        {
        BeMutexHolder lock(m_mutex);
        m_nodeVisibilities.erase(nodeId);
        m_nodeVisibilities.Insert(nodeId, NodeVisibility::Hidden);
        }

    bool _IsCombinedHierarchyLevelInitialized(CombinedHierarchyLevelIdentifier const& info, RulesetVariables const& variables, Utf8StringCR instanceFilter) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_physicalHierarchy.find(info);
        if (m_physicalHierarchy.end() == iter || iter->second.empty())
            return false;
        for (auto const& dsInfo : iter->second)
            {
            if (dsInfo.GetInstanceFilter() != instanceFilter)
                continue;
            if (!IsDataSourceInitialized(dsInfo.GetId()))
                return false;
            }
        return true;
        }
    bool _IsHierarchyLevelInitialized(BeGuidCR id, RulesetVariables const& variables, Utf8StringCR instanceFilter) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto const& entry : m_virtualHierarchy)
            {
            if (entry.first.GetId() == id && !entry.second.empty())
                {
                for (auto const& dsInfo : entry.second)
                    {
                    if (dsInfo.GetInstanceFilter() != instanceFilter)
                        continue;
                    if (!IsDataSourceInitialized(dsInfo.GetId()))
                        return false;
                    }
                return true;
                }
            }
        return false;
        }
    bool _IsDataSourceInitialized(BeGuidCR id) const override
        {
        BeMutexHolder lock(m_mutex);
        return m_finalizedDataSources.end() != m_finalizedDataSources.find(id);
        }

    void _Update(DataSourceInfo const& info, int partsToUpdate) override
        {
        BeMutexHolder lock(m_mutex);
        if (info.IsInitialized() && 0 != ((int)DataSourceInfo::PART_IsFinalized & partsToUpdate))
            m_finalizedDataSources.insert(info.GetIdentifier().GetId());
        }

    NavNodeCPtr _LocateNode(IConnectionCR connection, Utf8StringCR rulesetId, NavNodeKeyCR key) const override
        {
        return nullptr;
        }
    IHierarchyCache::SavepointPtr _CreateSavepoint(bool) override {return new Savepoint(*this);}

    void _OnRulesetUsed(PresentationRuleSetCR) override {}

    std::shared_ptr<IHierarchyLevelLocker> _CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const& identifier) override {return nullptr;}

    bool _IsHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const&) const override {return false;}

public:
    TestNodesCache() {}
    void SetGetNodeHandler(GetNodeHandler handler) {m_getNodeHandler = handler;}
    void SetGetPhysicalParentNodeHandler(GetPhysicalParentNodeHandler handler) {m_getPhysicalParentNodeHandler = handler;}
    void SetCacheHierarchyLevelHandler(CacheHierarchyLevelHandler handler) {m_cacheHierarchyLevelHandler = handler;}
    void SetCacheDataSourceHandler(CacheDataSourceHandler handler) {m_cacheDataSourceHandler = handler;}
    void SetCacheNodeHandler(CacheNodeHandler handler) {m_cacheNodeHandler = handler;}
    size_t GetCachedChildrenCount(BeGuidCR parentId)
        {
        BeMutexHolder lock(m_mutex);
        size_t count = 0;
        for (auto const& phEntry : m_physicalHierarchy)
            {
            if (phEntry.first.GetPhysicalParentNodeId() == parentId)
                {
                for (auto const& dsId : phEntry.second)
                    {
                    auto dsIter = m_partialHierarchies.find(dsId);
                    if (dsIter != m_partialHierarchies.end())
                        count += dsIter->second.second.size();
                    }
                }
            }
        return count;
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE
