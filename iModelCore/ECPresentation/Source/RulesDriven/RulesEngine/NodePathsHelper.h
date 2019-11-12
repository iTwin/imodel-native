/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
+===============+===============+===============+===============+===============+======*/
struct NodePathsHelper
{
private:
    NodePathsHelper() {}
public:
    static folly::Future<bvector<NodesPathElement>> CreateHierarchy(IECPresentationManagerR manager, ECDbCR db, JsonValueCR jsonOptions, bvector<NavNodeCPtr> const& nodes, Utf8CP matchText);
    static bvector<NodesPathElement> MergePaths(std::vector<NodesPathElement>& paths, int64_t const* markedIndex);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
