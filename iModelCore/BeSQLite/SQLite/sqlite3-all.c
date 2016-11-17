/******************************************************************************
** This file is an amalgamation of many separate C source files from SQLite
** version 3.8.7.  By combining all the individual C code files into this 
** single large file, the entire code can be compiled as a single translation
** unit.  This allows many compilers to do optimizations that would not be
** possible if the files were compiled separately.  Performance improvements
** of 5% or more are commonly seen when SQLite is compiled as a single
** translation unit.
**
** This file is all you need to compile SQLite.  To use SQLite in other
** programs, you need this file and the "sqlite3.h" header file that defines
** the programming interface to the SQLite library.  (If you do not have 
** the "sqlite3.h" header file at hand, you will find a copy embedded within
** the text of this file.  Search for "Begin file sqlite3.h" to find the start
** of the embedded sqlite3.h header file.) Additional code files may be needed
** if you want a wrapper to interface SQLite with your choice of programming
** language. The code for the "sqlite3" command-line shell is also in a
** separate file. This file contains only code for the core SQLite library.
*/

#define SQLITE_OMIT_DEPRECATED 1
#define SQLITE_ENABLE_COLUMN_METADATA 1
#define SQLITE_DEFAULT_FOREIGN_KEYS 1
#define SQLITE_OMIT_AUTOINIT 1
#define SQLITE_ENABLE_SESSION 1
#define SQLITE_ENABLE_RTREE 1
#define SQLITE_ENABLE_PREUPDATE_HOOK 1
#define SQLITE_ENABLE_ZIPVFS 1
#define SQLITE_USE_URI 1

#define SQLITE_ENABLE_FTS5 1    // include support for full text search
#define SQLITE_ENABLE_JSON1 1   // include support for json

#define HAVE_STDINT_H

#define SQLITE_CORE 1
#define SQLITE_AMALGAMATION 1
#ifndef SQLITE_PRIVATE
# define SQLITE_PRIVATE static
#endif

#if defined (__BE_SQLITE_WINRT__)
    #define SQLITE_OS_WINRT 1
#endif

#if defined (_WIN32)
    #pragma warning (disable: 4244 4267 4018 4090 4101)
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

//#define SQLITE_ENABLE_SQLLOG 1

#include "sqlite3-1.c"
#include "sqlite3-2.c"
#include "sqlite3-3.c"
#include "sqlite3-4.c"
#include "sqlite3-5.c"
#include "sqlite3-6.c"
#include "sqlite3-7.c"
#include "zipvfs.c"
#include "closure.c"

#if defined (SQLITE_ENABLE_SQLLOG)
#include "test_sqllog.c"
#endif

#if !defined (NDEBUG)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
int checkNoActiveStatements(sqlite3* db)
    {
    Vdbe* stmt;
    if (0 == db->nVdbeActive)
        return SQLITE_OK;

    for (stmt = db->pVdbe; stmt != NULL; stmt = stmt->pNext)
        {
        if (stmt->magic != VDBE_MAGIC_RUN)
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
