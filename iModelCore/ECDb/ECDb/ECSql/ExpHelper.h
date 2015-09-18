/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ExpHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/ECSqlBuilder.h>
#include "Exp.h"
#include "SelectStatementExp.h"
#include "JoinExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct ExpHelper
    {
private:
    ExpHelper ();
    ~ExpHelper ();

public:
    static BentleyStatus ToPrimitiveType (ECN::PrimitiveType& primitiveType, Utf8StringCR type);
    static Utf8CP ToString(ECN::PrimitiveType type);
    static Utf8CP ToString(SqlCompareListType type);
    static Utf8CP ToString(JoinDirection direction);
    static Utf8CP ToString(ECSqlJoinType joinType);
    static Utf8CP ToString(SqlSetQuantifier setQuantifier);
    static Utf8CP ToString(SubqueryTestOperator op);
    static Utf8CP ToString(BinarySqlOperator op);
    static Utf8CP ToString(BooleanSqlOperator op);
    static Utf8CP ToString(UnarySqlOperator op);
    static Utf8CP ToString(ECSqlType);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE