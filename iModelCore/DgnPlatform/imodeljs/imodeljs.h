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
#include <ECDb/ECdbApi.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// RefCounted wrapper around ECDb
//! @bsiclass
//=======================================================================================
struct JsECDb : RefCounted<BeSQLite::EC::ECDb>
{
DEFINE_T_SUPER(BeSQLite::EC::ECDb)
private:
    mutable BeSQLite::EC::ECSqlStatementCache m_ecsqlCache;
    void _OnDbClose() override;
public:
    JsECDb();

    //! Gets a cached and prepared ECSqlStatement
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetPreparedECSqlStatement(Utf8CP ecsql) const;
};

typedef RefCountedPtr<JsECDb> JsECDbPtr;
typedef JsECDb& JsECDbR;
typedef JsECDb const& JsECDbCR;

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
struct IModelJs
{
    static Dgn::DgnDbPtr GetDbByName(BeSQLite::DbResult& dbres, Utf8StringR errmsg, BeFileNameCR fn, Dgn::DgnDb::OpenMode mode);
    static Json::Value GetRowAsRawJson(BeSQLite::EC::ECSqlStatement&);
    static void GetRowAsJson(Json::Value& json, BeSQLite::EC::ECSqlStatement&);
    static void GetECValuesCollectionAsJson(Json::Value& json, ECN::ECValuesCollectionCR) ;
    static ECN::ECClassCP GetClassFromInstance(Utf8StringR errmsg, BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static BeSQLite::EC::ECInstanceId GetInstanceIdFromInstance(Utf8StringR errmsg, BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static void InitLogging();

    DGNPLATFORM_EXPORT static void Initialize(BeFileNameCR);
    DGNPLATFORM_EXPORT static BentleyStatus OpenDgnDb(BeSQLite::DbResult&, Utf8StringR errmsg, DgnDbPtr&, BeFileNameCR dbname, DgnDb::OpenMode mode);
    DGNPLATFORM_EXPORT static BentleyStatus GetECClassMetaData(DgnDbStatus&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, Utf8CP schema, Utf8CP ecclass);
    DGNPLATFORM_EXPORT static BentleyStatus GetElement(DgnDbStatus&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, Json::Value const& inOpts);
    DGNPLATFORM_EXPORT static BentleyStatus GetModel(DgnDbStatus&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, Json::Value const& inOpts);
    DGNPLATFORM_EXPORT static BentleyStatus GetElementPropertiesForDisplay(DgnDbStatus&, Utf8StringR errmsg, JsonValueR results, DgnDbR db, Utf8CP id);
    
    DGNPLATFORM_EXPORT static JsECDbPtr CreateECDb(BeSQLite::DbResult& dbres, Utf8StringR errmsg, BeFileNameCR pathname);
    DGNPLATFORM_EXPORT static JsECDbPtr OpenECDb(BeSQLite::DbResult& dbres, Utf8StringR errmsg, BeFileNameCR pathname, BeSQLite::Db::OpenMode openMode);
    DGNPLATFORM_EXPORT static BeSQLite::DbResult ImportSchema(Utf8StringR errmsg, BeSQLite::EC::ECDbR ecdb, BeFileNameCR pathname);
    DGNPLATFORM_EXPORT static BeSQLite::DbResult InsertInstance(Utf8StringR errmsg, BeSQLite::EC::ECInstanceId& insertedId, BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    DGNPLATFORM_EXPORT static BeSQLite::DbResult UpdateInstance(Utf8StringR errmsg, BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    DGNPLATFORM_EXPORT static BeSQLite::DbResult ReadInstance(Utf8StringR errmsg, JsonValueR jsonInstance, BeSQLite::EC::ECDbCR ecdb, JsonValueCR instanceKey);
    DGNPLATFORM_EXPORT static BeSQLite::DbResult DeleteInstance(Utf8StringR errmsg, BeSQLite::EC::ECDbCR ecdb, JsonValueCR instanceKey);
    DGNPLATFORM_EXPORT static BeSQLite::DbResult ContainsInstance(Utf8StringR errmsg, bool& containsInstance, JsECDbR ecdb, JsonValueCR instanceKey);
    DGNPLATFORM_EXPORT static BeSQLite::DbResult ExecuteQuery(Utf8StringR errmsg, JsonValueR results, BeSQLite::EC::ECSqlStatement& stmt);
    DGNPLATFORM_EXPORT static Utf8String GetLastEcdbIssue();

};

END_BENTLEY_DGN_NAMESPACE
