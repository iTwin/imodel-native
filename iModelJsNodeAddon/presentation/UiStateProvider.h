/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <ECPresentation/IUiStateProvider.h>
#include "ECPresentationUtils.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationUiStateProvider : IUiStateProvider
{
private:
    bmap<Utf8String, NavNodeKeySet> m_expandedNodesPerHierarchy;
    mutable BeMutex m_mutex;

private:
    ECPresentationResult AddNodeKeys(Utf8StringCR rulesetId, NavNodeKeyListCR keys);
    ECPresentationResult RemoveNodeKeys(Utf8StringCR rulesetId, NavNodeKeyListCR keys);

protected:
    INavNodeKeysContainerCPtr _GetExpandedNodes(Utf8StringCR rulesetId) const override;

public:
    ECPresentationResult UpdateHierarchyState(ECPresentationManager& manager, ECDbCR, Utf8StringCR rulesetId, Utf8StringCR change, Utf8StringCR serializedKeys);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
