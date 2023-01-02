/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! An interface for @ref NavNode objects locater which can find nodes by their keys.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct  INavNodeLocater
{
protected:
    virtual NavNodeCPtr _LocateNode(IConnectionCR, Utf8StringCR, NavNodeKeyCR) const = 0;
public:
    virtual ~INavNodeLocater() {}
    NavNodeCPtr LocateNode(IConnectionCR connection, Utf8StringCR rulesetId, NavNodeKeyCR key) const {return _LocateNode(connection, rulesetId, key);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
