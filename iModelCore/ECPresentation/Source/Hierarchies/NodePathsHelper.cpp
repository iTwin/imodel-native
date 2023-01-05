/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "NodePathsHelper.h"
#include "../Shared/ExtendedData.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static uint16_t CountFilterTextMatches(NavNodeCR node, Utf8CP lowerFilterText)
    {
    if (Utf8String::IsNullOrEmpty(lowerFilterText))
        return 0;

    uint16_t occurances = 0;
    Utf8String lowerLabel = Utf8String(node.GetLabelDefinition().GetDisplayValue()).ToLower();
    size_t position = lowerLabel.find(lowerFilterText, 0);
    while (position != Utf8String::npos)
        {
        occurances++;
        position = lowerLabel.find(lowerFilterText, position + 1);
        }
    return occurances;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesHierarchy
{
private:
    bvector<bpair<NavNodeCPtr, int>> m_roots;
    bmap<BeGuid, bvector<bpair<NavNodeCPtr, int>>> m_children;
    BeMutex m_mutex;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void InsertByIndex(bvector<bpair<NavNodeCPtr, int>>& list, NavNodeCR node, int index)
        {
        for (size_t i = 0; i < list.size(); ++i)
            {
            if (list[i].second > index)
                {
                list.insert(list.begin() + i, bpair<NavNodeCPtr, int>(&node, index));
                return;
                }
            }
        list.push_back(bpair<NavNodeCPtr, int>(&node, index));
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<bpair<NavNodeCPtr, int>> const& GetRoots() const { return m_roots; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bmap<BeGuid, bvector<bpair<NavNodeCPtr, int>>> const& GetChildren() const { return m_children; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddRootNode(NavNodeCR node, int index)
        {
        BeMutexHolder lock(m_mutex);
        // check if node is already added as a root node
        auto rootsIter = std::find_if(m_roots.begin(), m_roots.end(), [node = NavNodeCPtr(&node)](bpair<NavNodeCPtr, int> rootNode) {return rootNode.first->GetNodeId() == node->GetNodeId(); });
        if (m_roots.end() != rootsIter)
            return;

        InsertByIndex(m_roots, node, index);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddChildNode(BeGuidCR parentNodeId, NavNodeCR node, int index)
        {
        BeMutexHolder lock(m_mutex);
        // see if this node is already in the hierarchy
        auto nodeIter = m_children.find(node.GetNodeId());
        if (nodeIter != m_children.end())
            return;

        auto parentIter = m_children.find(parentNodeId);
        if (parentIter != m_children.end())
            {
            // if this node has parent, add this as parent's child
            InsertByIndex(parentIter->second, node, index);
            }
        // insert this node into the hierarchy
        m_children.Insert(node.GetNodeId(), bvector<bpair<NavNodeCPtr, int>>());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool NodeExits(BeGuidCR nodeId)
        {
        BeMutexHolder lock(m_mutex);
        return m_children.end() != m_children.find(nodeId);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BeGuid CreateHierarchy(NodesHierarchy& hierarchy, NavNodeCR node, int index, std::function<NavNodeCPtr(NavNodeCR)> const& parentGetter)
    {
    auto parent = parentGetter(node);
    if (!parent.IsValid())
        {
        hierarchy.AddRootNode(node, index);
        }
    else if (!hierarchy.NodeExits(parent->GetNodeId()))
        {
        // get the parent and put it into the hierarchy
        CreateHierarchy(hierarchy, *parent, index, parentGetter);
        }
    hierarchy.AddChildNode(parent.IsValid() ? parent->GetNodeId() : BeGuid(), node, index);
    return node.GetNodeId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NodesPathElement GetPath(NavNodeCR root, bmap<BeGuid, bvector<bpair<NavNodeCPtr, int>>> const& hierarchy, size_t index, Utf8CP lowerFilterText, uint64_t* totalTextMatches)
    {
    uint16_t currentNodeMatches = CountFilterTextMatches(root, lowerFilterText);
    uint64_t totalChildrenMatches = 0;

    NodesPathElement node(root, index);
    auto iter = hierarchy.find(root.GetNodeId());
    for (size_t i = 0; i < iter->second.size(); i++)
        {
        uint64_t branchChildrenMatches = 0;
        node.GetChildren().push_back(GetPath(*iter->second[i].first, hierarchy, i, lowerFilterText, &branchChildrenMatches));
        totalChildrenMatches += branchChildrenMatches;
        }

    node.GetFilteringData().SetOccurances(currentNodeMatches);
    node.GetFilteringData().SetChildrenOccurances(totalChildrenMatches);

    if (nullptr != totalTextMatches)
        *totalTextMatches = currentNodeMatches + totalChildrenMatches;
    return node;
    }

static bool TryAppendPath(NodesPathElement& lhs, NodesPathElement const& rhs);
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendPath(bvector<NodesPathElement>& lhsList, NodesPathElement const& rhs)
    {
    bool didAppend = false;
    for (NodesPathElement& lhs : lhsList)
        {
        if (true == (didAppend = TryAppendPath(lhs, rhs)))
            break;
        }
    if (!didAppend)
        lhsList.push_back(rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool TryAppendPath(NodesPathElement& lhs, NodesPathElement const& rhs)
    {
    if (lhs.GetNode()->GetKey()->Compare(*rhs.GetNode()->GetKey()) == 0)
        {
        if (rhs.IsMarked())
            lhs.SetIsMarked(true);
        for (NodesPathElement const& rhsChild : rhs.GetChildren())
            AppendPath(lhs.GetChildren(), rhsChild);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MarkLeaves(NodesPathElement& path)
    {
    if (path.GetChildren().empty())
        {
        path.SetIsMarked(true);
        }
    else
        {
        for (NodesPathElement& child : path.GetChildren())
            MarkLeaves(child);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodesPathElement> NodePathsHelper::CreateHierarchy(bvector<NavNodeCPtr> const& nodes, std::function<NavNodeCPtr(NavNodeCR)> const& parentGetter, Utf8StringCR matchText)
    {
    int nodeIndex = 0;
    NodesHierarchy hierarchy;
    for (NavNodeCPtr node : nodes)
        ::CreateHierarchy(hierarchy, *node, nodeIndex++, parentGetter);

    size_t index = 0;
    bvector<NodesPathElement> paths;
    for (auto const& root : hierarchy.GetRoots())
        paths.push_back(GetPath(*root.first, hierarchy.GetChildren(), index++, matchText.c_str(), nullptr));

    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodesPathElement> NodePathsHelper::MergePaths(std::vector<NodesPathElement>& paths, Nullable<size_t> const& markedIndex)
    {
    bvector<NodesPathElement> mergedPaths;
    for (size_t i = 0; i < paths.size(); ++i)
        {
        NodesPathElement& path = paths[i];
        if (path.GetNode().IsValid())
            {
            if (markedIndex.IsValid() && markedIndex.Value() == i)
                MarkLeaves(path);
            AppendPath(mergedPaths, path);
            }
        }
    return mergedPaths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NodesPathElement FindNode(ECPresentationManager::Impl& manager, NodeByInstanceKeyRequestImplParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Find node");

    INavNodesDataSourcePtr nodes = manager.GetNodes(HierarchyRequestImplParams::Create(params));
    if (!nodes.IsValid())
        return NodesPathElement();

    auto instanceKeysProvider = manager.CreateNodeInstanceKeysProvider(NodeInstanceKeysRequestImplParams::Create(NodeInstanceKeysRequestParams(params, nullptr), params));
    if (!instanceKeysProvider)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to create node instance keys provider. Returning invalid result.");

    size_t nodeIndex = 0;
    for (NavNodeCPtr node : *nodes)
        {
        if (node->GetKey()->AsECInstanceNodeKey() && node->GetKey()->AsECInstanceNodeKey()->HasInstanceKey(params.GetInstanceKey()))
            return NodesPathElement(*node, nodeIndex);

        if (nullptr != node->GetKey()->AsGroupingNodeKey())
            {
            if (instanceKeysProvider->ContainsInstanceKey(*node, params.GetInstanceKey()))
                return NodesPathElement(*node, nodeIndex);
            }

        nodeIndex++;
        }

    return NodesPathElement();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesPathElement NodePathsHelper::CreateNodePath(ECPresentationManager::Impl& manager, NodePathFromInstanceKeyPathRequestImplParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Create node path");

    if (params.GetInstanceKeyPath().empty())
        return NodesPathElement();

    auto parentEl = FindNode(manager, NodeByInstanceKeyRequestImplParams::Create(params.GetConnection(), params.GetCancellationToken(), params, params.GetInstanceKeyPath().front()));
    if (!parentEl.GetNode().IsValid())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_DEBUG, LOG_ERROR, "Requested ECInstance keys path not found in hierarchy");
        return NodesPathElement();
        }

    auto childParams(params);
    childParams.SetParentNode(parentEl.GetNode().get());
    if (parentEl.GetNode()->GetKey()->AsECInstanceNodeKey() && parentEl.GetNode()->GetKey()->AsECInstanceNodeKey()->HasInstanceKey(childParams.GetInstanceKeyPath().front()))
        childParams.GetInstanceKeyPath().erase(childParams.GetInstanceKeyPath().begin());

    auto childPath = CreateNodePath(manager, childParams);
    if (childPath.GetNode().IsValid())
        parentEl.GetChildren().push_back(childPath);
    return parentEl;
    }
