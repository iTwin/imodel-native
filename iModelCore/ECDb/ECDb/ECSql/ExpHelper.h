/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "Exp.h"
#include "SelectStatementExp.h"
#include "JoinExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct ExpHelper final
    {
private:
    ExpHelper ();
    ~ExpHelper ();

public:
    static BentleyStatus ToPrimitiveType (ECN::PrimitiveType&, Utf8StringCR colType);
    static Utf8CP ToString(ECN::PrimitiveType);
    static Utf8CP ToSql(SqlCompareListType);
    static Utf8CP ToECSql(JoinDirection);
    static Utf8CP ToSql(ECSqlJoinType);
    static Utf8CP ToSql(SqlSetQuantifier);
    static Utf8CP ToSql(SubqueryTestOperator);
    static Utf8CP ToSql(BinarySqlOperator);
    static Utf8CP ToSql(BooleanSqlOperator);
    static Utf8CP ToSql(UnaryValueExp::Operator);
    static Utf8CP ToSql(ECSqlType);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE