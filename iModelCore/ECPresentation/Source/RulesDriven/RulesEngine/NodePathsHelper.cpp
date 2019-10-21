/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "NodePathsHelper.h"
#include "ExtendedData.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static uint16_t CountFilterTextMatches(NavNodeCR node, Utf8CP lowerFilterText)
    {
    if (Utf8String::IsNullOrEmpty(lowerFilterText))
        return 0;

    uint16_t occurances = 0;
    Utf8String lowerLabel(node.GetLabel().ToLower());
    size_t position = lowerLabel.find(lowerFilterText, 0);
    while (position != Utf8String::npos)
        {
        occurances++;
        position = lowerLabel.find(lowerFilterText, position + 1);
        }
    return occurances;
    }

struct NodesHierarchy
    {
    bvector<NavNodeCPtr> roots;
    bmap<uint64_t, bvector<NavNodeCPtr>> children;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<bmap<uint64_t, bvector<NavNodeCPtr>>::iterator> CreateHierarchy(IECPresentationManager& mgr, ECDbCR db, NavNodeCR node, std::shared_ptr<NodesHierarchy> hierarchy, JsonValueCR jsonOptions)
    {
    auto parentIterFuture = folly::makeFuture(hierarchy->children.end());
    if (0 == node.GetParentNodeId())
        {
        auto rootsIter = std::find_if(hierarchy->roots.begin(), hierarchy->roots.end(), [node = NavNodeCPtr(&node)](NavNodeCPtr rootNode) {return rootNode->GetNodeId() == node->GetNodeId(); });
        if (hierarchy->roots.end() == rootsIter) 
            {
            // this node has no parent and is not already added to hierarchy
            hierarchy->roots.push_back(&node);
            }
        }
    else
        {
        // see if parent is already in the hierarchy
        auto parentIter = hierarchy->children.find(node.GetParentNodeId());
        if (parentIter != hierarchy->children.end())
            {
            parentIterFuture = folly::makeFuture(parentIter);
            }
        else
            {
            // get the parent and put it into the hierarchy
            parentIterFuture = mgr.GetParent(db, node, jsonOptions).then([&mgr, &db, hierarchy, jsonOptions](NavNodeCPtr parent)
                {
                if (parent.IsNull())
                    return folly::makeFuture(hierarchy->children.end());
                return CreateHierarchy(mgr, db, *parent, hierarchy, jsonOptions);
                });
            }
        }
    return parentIterFuture.then([hierarchy, node = NavNodeCPtr(&node)](bmap<uint64_t, bvector<NavNodeCPtr>>::iterator parentIter)
        {
        // see if this node is already in the hierarchy
        auto iter = hierarchy->children.find(node->GetNodeId());
        if (iter == hierarchy->children.end())
            {
            if (parentIter != hierarchy->children.end())
                {
                // if this node has parent, add this as parent's child
                parentIter->second.push_back(node);
                }
            // insert this node into the hierarchy
            return hierarchy->children.Insert(node->GetNodeId(), bvector<NavNodeCPtr>()).first;
            }
        return iter;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static NodesPathElement GetPath(NavNodeCR root, bmap<uint64_t, bvector<NavNodeCPtr>> const& hierarchy, size_t index, Utf8CP lowerFilterText, uint64_t* totalTextMatches)
    {
    uint16_t currentNodeMatches = CountFilterTextMatches(root, lowerFilterText);
    uint64_t totalChildrenMatches = 0;

    NodesPathElement node(root, index);
    auto iter = hierarchy.find(root.GetNodeId());
    for (size_t i = 0; i < iter->second.size(); i++)
        {
        uint64_t branchChildrenMatches = 0;
        node.GetChildren().push_back(GetPath(*iter->second[i], hierarchy, i, lowerFilterText, &branchChildrenMatches));
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
* @bsimethod                                    Grigas.Petraitis                12/2016
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
* @bsimethod                                    Grigas.Petraitis                12/2016
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
* @bsimethod                                    Grigas.Petraitis                12/2016
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
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NodesPathElement>> NodePathsHelper::CreateHierarchy(IECPresentationManagerR manager, ECDbCR db, JsonValueCR jsonOptions, bvector<NavNodeCPtr> const& nodes, Utf8CP matchText)
    {
    auto hierarchy = std::make_shared<NodesHierarchy>();
    bvector<folly::Future<folly::Unit>> futures;
    for (NavNodeCPtr node : nodes)
        futures.push_back(::CreateHierarchy(manager, db, *node, hierarchy, jsonOptions).then());

    return folly::collect(futures.begin(), futures.end()).then([hierarchy, matchText = Utf8String(matchText)]()
        {
        size_t index = 0;
        bvector<NodesPathElement> paths;
        for (NavNodeCPtr const& root : hierarchy->roots)
            paths.push_back(GetPath(*root, hierarchy->children, index++, matchText.c_str(), nullptr));
        return paths;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodesPathElement> NodePathsHelper::MergePaths(std::vector<NodesPathElement>& paths, int64_t const* markedIndex)
    {
    bvector<NodesPathElement> mergedPaths;
    for (size_t i = 0; i < paths.size(); ++i)
        {
        NodesPathElement& path = paths[i];
        if (path.GetNode().IsValid())
            {
            if (markedIndex && *markedIndex == (int64_t)i)
                MarkLeaves(path);
            AppendPath(mergedPaths, path);
            }
        }
    return mergedPaths;
    }