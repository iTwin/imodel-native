/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>

#if defined (__ECDB_BUILD__)
    #define ECDB_EXPORT EXPORT_ATTRIBUTE
#else
    #define ECDB_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_SQLITE_EC_NAMESPACE   BEGIN_BENTLEY_SQLITE_NAMESPACE namespace EC {
#define END_BENTLEY_SQLITE_EC_NAMESPACE     } END_BENTLEY_SQLITE_NAMESPACE
#define USING_NAMESPACE_BENTLEY_SQLITE_EC   using namespace BentleyApi::BeSQLite::EC;

