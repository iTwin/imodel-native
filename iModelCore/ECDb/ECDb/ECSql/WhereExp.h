/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/WhereExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ComputedExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct WhereExp : Exp
    {
DEFINE_EXPR_TYPE(Where) 

private:
    virtual Utf8String _ToString () const override;

public:
    explicit WhereExp(std::unique_ptr<BooleanExp> expression);

    BooleanExp const* GetSearchConditionExp() const;
    virtual Utf8String ToECSql() const override;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE