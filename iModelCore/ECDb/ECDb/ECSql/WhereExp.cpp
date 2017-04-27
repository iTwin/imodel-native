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
WhereExp::WhereExp(std::unique_ptr<BooleanExp> expression) : Exp(Type::Where) { AddChild(std::move(expression)); }

            tmp.insert(&propertyMap->GetClassMap().GetJoinedTable());
END_BENTLEY_SQLITE_EC_NAMESPACE
