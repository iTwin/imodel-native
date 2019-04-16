/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereExp::WhereExp(std::unique_ptr<BooleanExp> expression) : Exp(Type::Where) { AddChild(std::move(expression)); }

END_BENTLEY_SQLITE_EC_NAMESPACE
