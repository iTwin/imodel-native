/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! Marks a changeset generation, diff or apply operation running on this thread for a
//! connection. While active, an authorizer Ignore is treated as Deny so that redacted
//! reads cannot leak into changesets as NULL values.
// @bsiclass
//=======================================================================================
struct ForbidAuthorizerIgnore : NonCopyableClass
{
private:
    SqlDbP m_db;
    ForbidAuthorizerIgnore* m_prev;

public:
    explicit ForbidAuthorizerIgnore(SqlDbP db);
    ~ForbidAuthorizerIgnore();
    static bool IsActive(SqlDbP db);
};

END_BENTLEY_SQLITE_NAMESPACE
