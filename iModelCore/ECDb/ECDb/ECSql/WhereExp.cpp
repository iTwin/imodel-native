/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
WhereExp::WhereExp(std::unique_ptr<BooleanExp> expression) : Exp(Type::Where) { AddChild(std::move(expression)); }

END_BENTLEY_SQLITE_EC_NAMESPACE
