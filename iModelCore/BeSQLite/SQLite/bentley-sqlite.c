/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#define SQLITE_OMIT_DEPRECATED 1 // leave out all deprecated apis
#define SQLITE_ENABLE_COLUMN_METADATA 1
#define SQLITE_DEFAULT_FOREIGN_KEYS 1
#define SQLITE_OMIT_AUTOINIT 1
#define SQLITE_ENABLE_SESSION 1
#define SQLITE_ENABLE_RTREE 1
#define SQLITE_ENABLE_GEOPOLY 1
#define SQLITE_ENABLE_PREUPDATE_HOOK 1
#define SQLITE_USE_URI 1
#define SQLITE_ENABLE_NULL_TRIM 1 // trim null columns from end of rows. Experimental for now, per DRH
#define SQLITE_MAX_VARIABLE_NUMBER 20000 // this is the maximum number of variables in an SQL statement
#define SQLITE_OMIT_COMPLETE 1
#define SQLITE_OMIT_PROGRESS_CALLBACK 1
#define SQLITE_MAX_EXPR_DEPTH 2000
//Allocate around ~ 32 Mb
#define SQLITE_DEFAULT_CACHE_SIZE 8000
#define SQLITE_ENABLE_FTS5 1    // include support for full text search
#define SESSIONS_STRM_CHUNK_SIZE 64*1024
// https://sqlite.org/lang_mathfunc.html
#define SQLITE_ENABLE_MATH_FUNCTIONS 1
#define SQLITE_ENABLE_DBSTAT_VTAB 1
#define SQLITE_ENABLE_NORMALIZE 1
// Set max row or blob size to 2Gig-1 (SQLite's max). We override this when db is opened back to 1G.
#define SQLITE_MAX_LENGTH 2147483647

#define HAVE_STDINT_H

#if defined (__BE_SQLITE_WINRT__)
    #define SQLITE_OS_WINRT 1
#endif

#ifdef __APPLE__
    #include <TargetConditionals.h>
    #ifdef TARGET_OS_IPHONE
        #define HAVE_GETHOSTUUID 0
    #endif
#endif

#if defined (_WIN32)
    #pragma warning (disable: 4244 4267 4018 4090 4101 4996 4005 4098)
#elif defined (__clang__)
    #pragma clang diagnostic ignored "-Wunused-variable"
    #pragma clang diagnostic ignored "-Wunused-function"
    #pragma clang diagnostic ignored "-Wconstant-conversion"
#endif

#define SQLITE_API

#if defined (__APPLE__)
    #define SQLITE_HAVE_ISNAN 1
#endif

#if !defined(NDEBUG)
    #define SQLITE_DEBUG 1
#endif

#if defined (ANDROID)
    #define HAVE_STRCHRNUL 0
#endif

#include "sqlite3.c"
#include "closure.c"
#include "blockcachevfs.c"

#if defined (SQLITE_ENABLE_SQLLOG)
#include "test_sqllog.c"
#endif

#if !defined (NDEBUG)
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int checkNoActiveStatements(sqlite3* db)
    {
    Vdbe* stmt;
    if (0 == db->nVdbeActive)
        return SQLITE_OK;

    for (stmt = db->pVdbe; stmt != NULL; stmt = stmt->pNext)
        {
        if (stmt->eVdbeState != VDBE_RUN_STATE)
            {
            sqlite3_log(SQLITE_BUSY, "Active statement: %s", stmt->zSql);
            //assert(0);
            return SQLITE_ERROR;
            }
        }

    sqlite3_log(SQLITE_BUSY, "nVdbeActive=%d but no active statements detected?!)", db->nVdbeActive);
    return SQLITE_ERROR;
    }
#endif
