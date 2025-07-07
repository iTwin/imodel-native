/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SQLite/sqlite3.h"
#include "SQLite/bcvutil.h"

#ifdef __cplusplus
extern "C" {
#endif

int besqlite_custom_init_function(void) {
    // Enable the native CA store for HTTPS requests made via curl.
    sqlite3_bcv_global_config(SQLITE_BCVGLOBALCONFIG_NATIVECA, 1);
    // Enable best effort mode for certificate revocation on Windows.
    sqlite3_bcv_global_config(SQLITE_BCVGLOBALCONFIG_REVOKEBESTEFFORT, 1);
    return SQLITE_OK;
}

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif
