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
    ECPRESENTATION_EXPORT static bvector<NodesPathElement> CreateHierarchy(bvector<NavNodeCPtr> const& nodes, std::function<NavNodeCPtr(NavNodeCR)> const& parentGetter, ICancelationTokenCP = nullptr);
    static void CountHierarchyFilterOccurences(bvector<NodesPathElement>&, Utf8StringCR lowerCaseText, ICancelationTokenCP = nullptr);
    static bvector<NodesPathElement> MergePaths(std::vector<NodesPathElement>& paths, Nullable<size_t> const& markedIndex);
    static NodesPathElement CreateNodePath(ECPresentationManager::Impl&, NodePathFromInstanceKeyPathRequestImplParams const&);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
