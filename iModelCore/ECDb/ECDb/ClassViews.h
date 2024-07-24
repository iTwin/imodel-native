/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include <functional>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//======================================================================================
// @bsiclass
//======================================================================================
struct ClassViews final {
    constexpr static char ViewSchemaName[] = "ECDbMap";
    constexpr static char ViewClassName[] = "QueryView";
    constexpr static char ViewClassQueryProp[] = "Query";

    static bool IsViewClass(ECN::ECClassCR viewClass);
    static bool TryGetQuery(Utf8StringR query, ECN::ECClassCR);
    static bvector<ECN::ECClassCP> FindViewClasses(ECDbCR conn);
    static bool IsValid(ECN::ECClassCR viewClass, ECDbCR conn);
    static bool CheckViews(ECDbCR conn);
};

END_BENTLEY_SQLITE_EC_NAMESPACE