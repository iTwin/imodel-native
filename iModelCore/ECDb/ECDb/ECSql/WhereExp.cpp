/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/WhereExp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereExp::WhereExp(std::unique_ptr<BooleanExp> expression) : Exp(Type::Where)
    {
    AddChild(std::move(expression));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BooleanExp const* WhereExp::GetSearchConditionExp() const
    {
    return GetChild<BooleanExp>(0);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
