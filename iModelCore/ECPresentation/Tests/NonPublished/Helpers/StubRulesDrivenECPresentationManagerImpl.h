/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "RulesDrivenECPresentationManagerImplBase.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StubNodeInstanceKeysProvider : INodeInstanceKeysProvider
    {
    private:
        std::function<void(NavNodeCR, std::function<bool(ECInstanceKey)>)> m_iterate;
        std::function<bool(NavNodeCR, ECInstanceKeyCR)> m_contains;
    protected:
        void _IterateInstanceKeys(NavNodeCR node, std::function<bool(ECInstanceKey)> cb) const override
            {
            if (m_iterate)
                m_iterate(node, cb);
            }
        bool _ContainsInstanceKey(NavNodeCR node, ECInstanceKeyCR key) const override
            {
            if (m_contains)
                return m_contains(node, key);
            return false;
            }
    public:
        void SetIterateFunc(std::function<void(NavNodeCR, std::function<bool(ECInstanceKey)>)> func) { m_iterate = func; }
        void SetContainsFunc(std::function<bool(NavNodeCR, ECInstanceKeyCR)> func) { m_contains = func; }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StubRulesDrivenECPresentationManagerImpl : RulesDrivenECPresentationManagerImplBase
{
    template<typename TNodePtr> struct NavNodeByIdComparer
        {
        bool operator()(TNodePtr const& lhs, TNodePtr const& rhs) const
            {
            return (lhs->GetNodeId() < rhs->GetNodeId());
            }
        };
    typedef bmap<NavNodePtr, bvector<NavNodePtr>> Hierarchy;

private:
    Hierarchy m_hierarchy; // parent -> children
    bmap<NavNodeCP, NavNodeCP, NavNodeByIdComparer<NavNodeCP>> m_parentship; // child -> parent
    std::function<std::unique_ptr<INodeInstanceKeysProvider>()> m_nodeInstanceKeysProviderFactory;

private:
    INavNodesDataSourcePtr GetNodes(NavNodeCP parent)
        {
        bpair<NavNodePtr, bvector<NavNodePtr>> pair;
        if (!FindPair(parent, pair))
            return EmptyDataSource<NavNodePtr>::Create();

        bvector<NavNodePtr> nodes;
        for (NavNodePtr node : pair.second)
            nodes.push_back(node.get()->Clone());

        return VectorDataSource<NavNodePtr>::Create(nodes);
        }
    bool FindPair(NavNodeCP parent, bpair<NavNodePtr, bvector<NavNodePtr>>& p)
        {
        for (auto& pair : m_hierarchy)
            {
            if (pair.first == nullptr && parent == nullptr ||
                pair.first != nullptr && parent != nullptr && pair.first.get()->GetKey()->Compare(*parent->GetKey()) == 0)
                {
                p = pair;
                return true;
                }
            }
        return false;
        }

protected:
    std::unique_ptr<INodeInstanceKeysProvider> _CreateNodeInstanceKeysProvider(NodeInstanceKeysRequestImplParams const&) const override
        {
        if (m_nodeInstanceKeysProviderFactory)
            return m_nodeInstanceKeysProviderFactory();
        return std::make_unique<StubNodeInstanceKeysProvider>();
        }
    INavNodesDataSourcePtr _GetNodes(WithPageOptions<HierarchyRequestImplParams> const& params) override
        {
        return GetNodes(params.GetParentNode());
        }
    size_t _GetNodesCount(HierarchyRequestImplParams const& params) override
        {
        return GetNodes(params.GetParentNode())->GetSize();
        }
    NavNodeCPtr _GetParent(NodeParentRequestImplParams const& params) override
        {
        auto iter = m_parentship.find(&params.GetNode());
        return (m_parentship.end() != iter) ? iter->second : nullptr;
        }
    bvector<NavNodeCPtr> _GetFilteredNodes(NodePathsFromFilterTextRequestImplParams const& params) override
        {
        bvector<NavNodeCPtr> result;
        for (auto const& pair : m_parentship)
            {
            NavNodeCP node = pair.first;
            if (nullptr != node && node->GetLabelDefinition().GetDisplayValue().ContainsI(params.GetFilterText()))
                result.push_back(node);
            }
        return result;
        }
    HierarchyComparePositionPtr _CompareHierarchies(HierarchyCompareRequestImplParams const&) override
        {
        return nullptr;
        }

    bvector<SelectClassInfo> _GetContentClasses(ContentClassesRequestImplParams const&) override
        {
        return bvector<SelectClassInfo>();
        }
    ContentDescriptorCPtr _GetContentDescriptor(ContentDescriptorRequestImplParams const&) override
        {
        return nullptr;
        }
    ContentCPtr _GetContent(WithPageOptions<ContentRequestImplParams> const&) override
        {
        return nullptr;
        }
    size_t _GetContentSetSize(ContentRequestImplParams const&) override
        {
        return 0;
        }
    LabelDefinitionCPtr _GetDisplayLabel(KeySetDisplayLabelRequestImplParams const&) override
        {
        return LabelDefinition::Create();
        }
    PagingDataSourcePtr<DisplayValueGroupCPtr> _GetDistinctValues(WithPageOptions<DistinctValuesRequestImplParams> const&) override
        {
        return PagingDataSource<DisplayValueGroupCPtr>::Create();
        }

public:
    StubRulesDrivenECPresentationManagerImpl(ECPresentationManager::Impl::Params const& params)
        : RulesDrivenECPresentationManagerImplBase(params)
        {}

    void SetHierarchy(Hierarchy hierarchy)
        {
        m_hierarchy = hierarchy;
        for (auto pair : m_hierarchy)
            {
            NavNodeCP parent = pair.first.get();
            for (NavNodeCPtr child : pair.second)
                m_parentship[child.get()] = parent;
            }
        }

    void SetNodeInstanceKeysProviderFactory(std::function<std::unique_ptr<INodeInstanceKeysProvider>()> nodeInstanceKeysProviderFactory)
        {
        m_nodeInstanceKeysProviderFactory = nodeInstanceKeysProviderFactory;
        }

    NavNodeCPtr GetNode(BeGuidCR nodeId) const
        {
        for (auto const& pair : m_hierarchy)
            {
            NavNodeCP node = pair.first.get();
            if (nullptr != node && node->GetNodeId() == nodeId)
                return node;
            }
        return nullptr;
        }

    NavNodeCPtr GetParentNode(BeGuidCR nodeId) const
        {
        for (auto const& entry : m_parentship)
            {
            if (entry.first->GetNodeId() == nodeId)
                return entry.second;
            }
        return nullptr;
        }
    };

END_ECPRESENTATIONTESTS_NAMESPACE
