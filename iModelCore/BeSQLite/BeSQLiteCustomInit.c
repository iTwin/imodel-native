/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SQLite/sqlite3.h"
#include "SQLite/bcvutil.h"
#ifdef __APPLE__
#include <stdlib.h>
#include <string.h>
#include <mach-o/dyld.h>
#endif // __APPLE__

#ifdef __cplusplus
extern "C" {
#endif

int besqlite_custom_init_function(void) {
    // Enable best effort mode for certificate revocation on Windows.
    sqlite3_bcv_global_config(SQLITE_BCVGLOBALCONFIG_REVOKEBESTEFFORT, 1);
#ifdef __APPLE__
    uint32_t size = 0;
    _NSGetExecutablePath(NULL, &size); // get the size needed
    if (size == 0) {
        return SQLITE_ERROR;
    }
    char *executablePath = (char*)malloc(size);
    if (_NSGetExecutablePath(executablePath, &size) != 0) {
        free(executablePath);
        return SQLITE_ERROR;
    }
    char *certsPath;
    const char *suffix = "/Assets/cacert.pem";
    const char *lastSlash = strrchr(executablePath, '/');
    if (lastSlash != NULL) {
        size_t baseLen = lastSlash - executablePath;
        certsPath = (char*)malloc(baseLen + strlen(suffix) + 1);
        strncpy(certsPath, executablePath, baseLen);
        certsPath[baseLen] = '\0';
    } else {
        // If there aren't any slashes in the executable path, just use the current directory.
        certsPath = (char*)malloc(strlen(suffix) + 2);
        strcpy(certsPath, ".");
    }
    free(executablePath);
    strcat(certsPath, suffix);
    sqlite3_bcv_global_config(SQLITE_BCVGLOBALCONFIG_CAFILE, certsPath);
    free(certsPath);
#endif // __APPLE__
    return SQLITE_OK;
}

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif
