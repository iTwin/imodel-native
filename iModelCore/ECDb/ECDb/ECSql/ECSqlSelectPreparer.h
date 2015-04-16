/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlSelectPreparer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlSelectPreparer
    {
private:
    //static class
    ECSqlSelectPreparer ();
    ~ECSqlSelectPreparer ();

public:
    static ECSqlStatus Prepare(ECSqlPrepareContext& ctx, SelectStatementExp const& exp);
    static ECSqlStatus Prepare(ECSqlPrepareContext& ctx, UnionStatementExp const& exp);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE