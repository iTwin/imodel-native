/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#define SESSIONS_STRM_CHUNK_SIZE        64*1024
#define SQLITE_DEFAULT_CACHE_SIZE       8000
#define SQLITE_DEFAULT_FOREIGN_KEYS     1
#define SQLITE_ENABLE_COLUMN_METADATA   1
#define SQLITE_ENABLE_DBSTAT_VTAB       1
#define SQLITE_ENABLE_FTS4              1
#define SQLITE_ENABLE_FTS5              1
#define SQLITE_ENABLE_GEOPOLY           1
#define SQLITE_ENABLE_MATH_FUNCTIONS    1
#define SQLITE_ENABLE_NORMALIZE         1
#define SQLITE_ENABLE_NULL_TRIM         1
#define SQLITE_ENABLE_PREUPDATE_HOOK    1
#define SQLITE_ENABLE_RTREE             1
#define SQLITE_ENABLE_SESSION           1
#define SQLITE_ENABLE_STMTVTAB          1
#define SQLITE_MAX_COLUMN               2200
#define SQLITE_MAX_EXPR_DEPTH           3000
#define SQLITE_MAX_LENGTH               2147483647
#define SQLITE_MAX_VARIABLE_NUMBER      20000
#define SQLITE_OMIT_AUTOINIT            1
#define SQLITE_OMIT_COMPLETE            1
#define SQLITE_OMIT_DEPRECATED          1
#define SQLITE_USE_URI                  1

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
#include "blockcachevfs.c"
#include "shathree.c"

#if defined (SQLITE_ENABLE_SQLLOG)
#include "test_sqllog.c"
#endif

#if !defined (NDEBUG)
/*---------------------------------------------------------------------------------**//**
* Logs every prepared statement on this connection that has been stepped but not yet reset
* or finalized, along with its state, and returns SQLITE_ERROR if any were found, SQLITE_OK
* otherwise. Such statements still hold locks on the table/db and are a common cause of a
* failed commit or close, after which it is unclear whether the transaction was committed.
* A prepared-but-not-stepped statement (e.g. one sitting in the statement cache) holds no
* lock and is therefore not reported.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int checkNoActiveStatements(sqlite3* db)
    {
    Vdbe* stmt;
    int found = 0;

    // A prepared statement is internally a Vdbe; sqlite3_stmt is just its opaque public handle.
    // The casts cross that public/private boundary and mirror what SQLite does internally.
    for (stmt = db->pVdbe; stmt != NULL; stmt = (Vdbe*)sqlite3_next_stmt(db, (sqlite3_stmt*)stmt))
        {
        const char* state;
        if (stmt->eVdbeState == VDBE_RUN_STATE)
            state = "stepped, more rows to go";
        else if (stmt->eVdbeState == VDBE_HALT_STATE)
            state = "stepped to end, not finalized";
        else
            continue; // not stepped or still under construction: holds no lock

        sqlite3_log(SQLITE_BUSY, "Unfinalized statement (%s): %s", state, stmt->zSql);
        found = 1;
        //assert(0);
        }

    return found ? SQLITE_ERROR : SQLITE_OK;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int getStatementState(sqlite3_stmt *pStmt)
    {
    Vdbe* stmt = (Vdbe*)pStmt;
    return stmt->eVdbeState;
    }
