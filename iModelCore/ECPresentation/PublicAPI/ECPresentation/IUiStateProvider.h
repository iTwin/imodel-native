/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/NavNode.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! An interface for a class that provides information about current UI state.
// @bsiclass
//=======================================================================================
struct IUiStateProvider
{
protected:
    virtual INavNodeKeysContainerCPtr _GetExpandedNodes(Utf8StringCR) const = 0;
public:
    virtual ~IUiStateProvider() {}
    INavNodeKeysContainerCPtr GetExpandedNodes(Utf8StringCR rulesetId) const { return _GetExpandedNodes(rulesetId); }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE