/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/WhereExp.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#include "WhereExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using namespace std;

//****************************** WhereExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereExp::WhereExp (unique_ptr<BooleanExp> expression)
    {
    AddChild (move (expression));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BooleanExp const* WhereExp::GetExpression() const
    {
    return GetChild<BooleanExp> (0);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WhereExp::ToECSql() const
    {
    return "WHERE " + GetExpression ()->ToECSql();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WhereExp::_ToString() const 
    {
    return "Where";
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
