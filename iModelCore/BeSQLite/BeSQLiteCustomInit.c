/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "SQLite/sqlite3.h"
#include "SQLite/bcvutil.h"

#ifdef __cplusplus
extern "C" {
#endif

int besqlite_custom_init_function(void) {
    char *revokeBestEffort = getenv("BESQLITE_REVOKE_BEST_EFFORT");
    if (revokeBestEffort && strcmp(revokeBestEffort, "1") == 0) {
        sqlite3_bcv_global_config(SQLITE_BCVGLOBALCONFIG_REVOKEBESTEFFORT, 1);
    }
    return SQLITE_OK;
}

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif
