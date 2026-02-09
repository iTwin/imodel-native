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
void NavValueCreationFuncExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql("NAVIGATION_VALUE(").AppendToECSql(*GetColumnRefExp()).AppendToECSql(" ").AppendToECSql(*GetIdArgExp());
    if (GetRelECClassIdExp() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetRelECClassIdExp());
    ctx.AppendToECSql(")");
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NavValueCreationFuncExp::_ToJson(BeJsValue val, JsonFormat const& fmt) const {
    val.SetEmptyObject();
    val["id"] = "NavValueCreationFuncExp";
    GetColumnRefExp()->ToJson(val["ColumnRefExp"], fmt);
    GetIdArgExp()->ToJson(val["IdArgExp"], fmt);
    if (GetRelECClassIdExp() != nullptr)
        GetRelECClassIdExp()->ToJson(val["RelECClassIdExp"], fmt);
    GetClassNameExp()->ToJson(val["ClassNameExp"], fmt);
}

END_BENTLEY_SQLITE_EC_NAMESPACE
