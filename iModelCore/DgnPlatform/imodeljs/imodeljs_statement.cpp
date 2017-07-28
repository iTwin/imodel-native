/*--------------------------------------------------------------------------------------+
|
|     $Source: imodeljs/imodeljs_statement.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "imodeljs.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus IModelJs::StepStatementOnce(BeSQLite::DbResult& status, Utf8StringR errmsg, JsonValueR results, DgnDbR dgndb, BeSQLite::EC::ECSqlStatement& stmt)
    {
    BeSqliteDbMutexHolder serializeAccess(dgndb); // hold mutex, so that I have a chance to get last ECDb error message

    if (BE_SQLITE_ROW == (status = stmt.Step()))
        {
        IModelJs::GetRowAsJson(results, stmt);
        return BSISUCCESS;
        }

    if (BE_SQLITE_DONE == status)
        {
        results = Json::nullValue;
        return BSISUCCESS;      // Finding no matching rows is not an exception. It's just an empty result.
        }

    errmsg = GetLastEcdbIssue();
    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus IModelJs::StepStatementAll(BeSQLite::DbResult& status, Utf8StringR errmsg, JsonValueR results, DgnDbR dgndb, BeSQLite::EC::ECSqlStatement& stmt)
    {
    BeSqliteDbMutexHolder serializeAccess(dgndb); // hold mutex, so that I have a chance to get last ECDb error message

    results = Json::arrayValue;

    while (BE_SQLITE_ROW == (status = stmt.Step()))
        {
        Json::Value row(Json::objectValue);
        IModelJs::GetRowAsJson(row, stmt);
        results.append(row);
        }

    if (BE_SQLITE_DONE == status)
        return BSISUCCESS;      // Finding no matching rows is not an exception. It's just an empty result.

    errmsg = GetLastEcdbIssue();
    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus IModelJs::GetCachedECSqlStatement(BeSQLite::DbResult& status, Utf8StringR errmsg, BeSQLite::EC::CachedECSqlStatementPtr& stmt, DgnDbR dgndb, Utf8CP ecsql)
    {
    BeSqliteDbMutexHolder serializeAccess(dgndb); // hold mutex, so that I have a chance to get last ECDb error message

    stmt = dgndb.GetPreparedECSqlStatement(ecsql);
    
    if (stmt.IsValid())
        return BSISUCCESS;        
    
    // Failure to prepare is an exception.
    status = DbResult::BE_SQLITE_ERROR;
    errmsg = GetLastEcdbIssue();
    return BSIERROR;
    }
