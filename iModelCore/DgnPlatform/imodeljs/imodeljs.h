/*--------------------------------------------------------------------------------------+
|
|     $Source: imodeljs/imodeljs.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>

BEGIN_BENTLEY_DGN_NAMESPACE

// Functions exposed to JavaScript.
//
// Errors vs. Empty results
// ------------------------
// These functions will return BentleyStatus::Error only when the inputs are in an invalid state or are malformed. The JS caller should get an exception. 
// The following are examples of real errors: 
// -- The elementid input to GetElementProperties cannot be parsed; 
// -- the DgnDb passed in is not open; 
// -- a statement has syntax errors or refers to non-existant classes and so fails to prepare. 
// In all such cases, Error is returned, and the JS caller should get an exception.
//
// If the inputs are in the correct state and proper form but yield no results, that is not an error. The following are examples of empty results from valid inputs:
// -- GetElementProperties fails to find an element specified by id or code.
// -- Stepping a statement finds no matching rows.
// In both cases, the return status is Success, and the resulting JSON is empty. The JS caller shoudl not get an exception, just an empty object.
struct imodeljs
{
    static Utf8String GetLastEcdbIssue();
    static Dgn::DgnDbPtr GetDbByName(BeSQLite::DbResult& dbres, Utf8StringR errmsg, BeFileNameCR fn, Dgn::DgnDb::OpenMode mode);
    static Json::Value& GetRowAsRawJson(Json::Value& rowAsJson, BeSQLite::EC::ECSqlStatement&);
    static void GetRowAsJson(Json::Value& json, BeSQLite::EC::ECSqlStatement&);
    static void GetECValuesCollectionAsJson(Json::Value& json, ECN::ECValuesCollectionCR) ;

    DGNPLATFORM_EXPORT static void Initialize(BeFileNameCR);
    DGNPLATFORM_EXPORT static BentleyStatus OpenDgnDb(BeSQLite::DbResult&, Utf8StringR errmsg, DgnDbPtr&, BeFileNameCR dbname, DgnDb::OpenMode mode);
    DGNPLATFORM_EXPORT static BentleyStatus GetECClassMetaData(DgnDbStatus&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, Utf8CP schema, Utf8CP ecclass);
    DGNPLATFORM_EXPORT static BentleyStatus GetElementProperties(DgnDbStatus&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, Json::Value const& inOpts);
    DGNPLATFORM_EXPORT static BentleyStatus GetElementPropertiesForDisplay(DgnDbStatus&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, Utf8CP id);
    DGNPLATFORM_EXPORT static BentleyStatus GetCachedECSqlStatement(BeSQLite::DbResult&, Utf8StringR errmsg, BeSQLite::EC::CachedECSqlStatementPtr&, DgnDbR db, Utf8CP ecsql);
    DGNPLATFORM_EXPORT static BentleyStatus StepStatementOnce(BeSQLite::DbResult&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, BeSQLite::EC::ECSqlStatement& s);
    DGNPLATFORM_EXPORT static BentleyStatus StepStatementAll(BeSQLite::DbResult&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, BeSQLite::EC::ECSqlStatement& s);
};

END_BENTLEY_DGN_NAMESPACE
