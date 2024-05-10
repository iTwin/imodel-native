/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavValueCreationFuncExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("NAVIGATION_VALUE(").AppendToECSql(*GetColumnRefExp()).AppendToECSql(" ").AppendToECSql(*GetIdArgExp());
    if (GetRelECClassIdExp() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetRelECClassIdExp());
    ctx.AppendToECSql(")");
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
