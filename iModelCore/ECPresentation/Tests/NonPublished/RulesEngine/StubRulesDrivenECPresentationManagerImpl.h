/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include "RulesDrivenECPresentationManagerImplBase.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
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

private:
    INavNodesDataSourcePtr GetNodes(NavNodeCP parent)
        {
        bpair<NavNodePtr, bvector<NavNodePtr>> pair;
        if (!FindPair(parent, pair))
            return EmptyNavNodesDataSource::Create();

        bvector<NavNodePtr> nodes;
        for (NavNodePtr node : pair.second)
            nodes.push_back(node.get()->Clone());

        return NodesVectorDataSource::Create(nodes);
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
    INavNodesDataSourcePtr _GetRootNodes(IConnectionCR connection, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        return GetNodes(nullptr);
        }
    size_t _GetRootNodesCount(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        return GetNodes(nullptr)->GetSize();
        }
    INavNodesDataSourcePtr _GetChildren(IConnectionCR connection, NavNodeCR parent, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        return GetNodes(&parent);
        }
    size_t _GetChildrenCount(IConnectionCR connection, NavNodeCR parent, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        return GetNodes(&parent)->GetSize();
        }
    NavNodeCPtr _GetParent(IConnectionCR connection, NavNodeCR child, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        auto iter = m_parentship.find(&child);
        return (m_parentship.end() != iter) ? iter->second : nullptr;
        }
    NavNodeCPtr _GetNode(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        for (auto pair : m_hierarchy)
            {
            NavNodeCP node = pair.first.get();
            if (nullptr != node && node->GetKey()->GetHash().Equals(nodeKey.GetHash()))
                return node;
            }
        return nullptr;
        }
    bvector<NavNodeCPtr> _GetFilteredNodes(IConnectionCR connection, Utf8CP filterText, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        bvector<NavNodeCPtr> result;
        for (auto pair : m_parentship)
            {
            NavNodeCP node = pair.first;
            if (nullptr != node && node->GetLabelDefinition().GetDisplayValue().ContainsI(filterText))
                result.push_back(node);
            }
        return result;
        }

    bvector<SelectClassInfo> _GetContentClasses(IConnectionCR connection, Utf8CP displayType, int contentFlags, bvector<ECClassCP> const& inputClasses, ContentOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        return bvector<SelectClassInfo>();
        }
    ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR connection, Utf8CP displayType, int contentFlags, KeySetCR inputKeys, SelectionInfo const* selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        return nullptr;
        }
    ContentCPtr _GetContent(IConnectionCR connection, ContentDescriptorCR descriptor, PageOptionsCR pageOptions, ICancelationTokenCR cancelationToken) override
        {
        return nullptr;
        }
    size_t _GetContentSetSize(IConnectionCR connection, ContentDescriptorCR descriptor, ICancelationTokenCR cancelationToken) override
        {
        return 0;
        }
    LabelDefinitionCPtr _GetDisplayLabel(IConnectionCR, KeySetCR, ICancelationTokenCR) override
        {
        return LabelDefinition::Create();
        }
    void _CompareHierarchies(IUpdateRecordsHandler&, IConnectionCR, Utf8StringCR, Utf8StringCR, CommonOptions const&, ICancelationTokenCR) override {}

public:
    StubRulesDrivenECPresentationManagerImpl(RulesDrivenECPresentationManager::Params const& params)
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
};

END_ECPRESENTATIONTESTS_NAMESPACE
