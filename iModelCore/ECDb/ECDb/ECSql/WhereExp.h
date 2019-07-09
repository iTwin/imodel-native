/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ComputedExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
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