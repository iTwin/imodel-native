/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include "../PresentationManagerImpl.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodePathsHelper
{
private:
    NodePathsHelper() {}
public:
    static bvector<NodesPathElement> CreateHierarchy(ECPresentationManager::Impl&, NodePathsFromFilterTextRequestImplParams const&, bvector<NavNodeCPtr> const& nodes);
    static bvector<NodesPathElement> MergePaths(std::vector<NodesPathElement>& paths, Nullable<size_t> const& markedIndex);
    static NodesPathElement FindNode(ECPresentationManager::Impl&, NodeByInstanceKeyRequestImplParams const&);
    static NodesPathElement CreateNodePath(ECPresentationManager::Impl&, NodePathFromInstanceKeyPathRequestImplParams const&);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
