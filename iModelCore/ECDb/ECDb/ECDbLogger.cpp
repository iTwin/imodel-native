/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbLogger.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static NativeLogging::CategoryLogger s_ecdbLog("ECDb");
NativeLogging::CategoryLogger* ECDbLogger::s_logger = &s_ecdbLog;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyApi::NativeLogging::CategoryLogger& ECDbLogger::Get()
    {
    return *s_logger;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
void ECDbLogger::LogSqliteError(BeSQLite::Db const& db, BeSQLite::DbResult sqliteStat, Utf8CP errorMessageHeader)
    {
    if (BE_SQLITE_OK == sqliteStat || BE_SQLITE_ROW == sqliteStat || BE_SQLITE_DONE == sqliteStat || BE_SQLITE_INTERRUPT == sqliteStat || !Get().isSeverityEnabled(NativeLogging::LOG_ERROR))
        return;

    if (errorMessageHeader == nullptr)
        errorMessageHeader = "SQLite error:";

    Utf8CP dbResultStr = BeSQLite::Db::InterpretDbResult(sqliteStat);

    Utf8String lastSqliteErrorMsg = db.GetLastError();
    //ECDb sometimes returns DbResult errors on its own. In that case there is no SQLite error to output
    if (lastSqliteErrorMsg.empty())
        Get().errorv("%s %s", errorMessageHeader, dbResultStr);
    else
        Get().errorv("%s %s: %s", errorMessageHeader, dbResultStr, lastSqliteErrorMsg.c_str());
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

