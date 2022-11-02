/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ComputedExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WhereExp final : Exp
    {
private:
    void _ToECSql(ECSqlRenderContext& ctx) const override { ctx.AppendToECSql("WHERE ").AppendToECSql(*GetSearchConditionExp()); }
    Utf8String _ToString() const override { return "Where"; }

public:
    explicit WhereExp(std::unique_ptr<BooleanExp> expression);

    BooleanExp const* GetSearchConditionExp() const { return GetChild<BooleanExp>(0); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE