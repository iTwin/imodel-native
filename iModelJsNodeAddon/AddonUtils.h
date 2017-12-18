/*--------------------------------------------------------------------------------------+
|
|     $Source: AddonUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <ECDb/ECDbApi.h>
#include <DgnPlatform/ECUtils.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_EC

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
    BeSQLite::EC::CachedECSqlStatementPtr GetPreparedECSqlStatement(Utf8CP ecsql) const;
};

typedef RefCountedPtr<JsECDb> JsECDbPtr;
typedef JsECDb& JsECDbR;
typedef JsECDb const& JsECDbCR;

struct AddonUtils
{
    static void GetRowAsJson(Json::Value& json, BeSQLite::EC::ECSqlStatement&);
    static void GetECValuesCollectionAsJson(Json::Value& json, ECN::ECValuesCollectionCR) ;
    static ECN::ECClassCP GetClassFromInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static BeSQLite::EC::ECInstanceId GetInstanceIdFromInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static void InitLogging();

    struct JsonBinder
        {
        private:
        static BentleyStatus BindPrimitiveValue(BeSQLite::EC::IECSqlBinder& binder, JsonValueCR bindingValue);
        static BentleyStatus BindArrayValue(BeSQLite::EC::IECSqlBinder& binder, JsonValueCR value);
        static BentleyStatus BindStructValue(BeSQLite::EC::IECSqlBinder& binder, JsonValueCR value);
        static BentleyStatus BindValue(BeSQLite::EC::IECSqlBinder& binder, JsonValueCR bindingValue);

        public:
        static BentleyStatus BindValues(BeSQLite::EC::ECSqlStatement& stmt, JsonValueCR bindings);
        };

    /** How to handle a conflict */
    enum class BriefcaseManagerConflictResolution {
    /** Reject the incoming change */
    Reject = 0,
    /** Accept the incoming change */
    Take = 1,
    };
    
    typedef std::function<void(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType)> T_AssertHandler;

    static void Initialize(BeFileNameCR, T_AssertHandler assertHandler);
    static BeSQLite::DbResult OpenDgnDb(DgnDbPtr&, BeFileNameCR dbname, DgnDb::OpenMode mode);
    static BeSQLite::DbResult OpenBriefcase(DgnDbPtr& db, JsonValueCR briefcaseToken, JsonValueCR changeSetTokens);
    static void CloseDgnDb(DgnDbR dgndb);
    static DgnDbStatus GetECClassMetaData(JsonValueR results, DgnDbR db, Utf8CP schema, Utf8CP ecclass);
    static DgnDbStatus GetElement(JsonValueR results, DgnDbR db, Json::Value const& inOpts);
    static DgnDbStatus InsertElement(JsonValueR results, DgnDbR db, Json::Value& props);
    static DgnDbStatus UpdateElement(DgnDbR db, Json::Value& props);
    static DgnDbStatus DeleteElement(DgnDbR db, Utf8StringCR eidStr);
    static DgnDbStatus InsertCodeSpec(Utf8StringR idStr, DgnDbR db, Utf8StringCR name, CodeScopeSpec::Type cstype, CodeScopeSpec::ScopeRequirement cssreq);
    static DgnDbStatus InsertModel(JsonValueR results, DgnDbR db, Json::Value& props);
    static DgnDbStatus UpdateModel(DgnDbR db, Json::Value& props);
    static DgnDbStatus DeleteModel(DgnDbR db, Utf8StringCR idStr);
    static DgnDbStatus GetModel(JsonValueR results, DgnDbR db, Json::Value const& inOpts);

    static JsECDbPtr CreateECDb(BeSQLite::DbResult& dbres, BeFileNameCR pathname);
    static JsECDbPtr OpenECDb(BeSQLite::DbResult& dbres, BeFileNameCR pathname, BeSQLite::Db::OpenMode openMode);
    static BeSQLite::DbResult ImportSchema(BeSQLite::EC::ECDbR ecdb, BeFileNameCR pathname);
    static BeSQLite::DbResult ImportSchemaDgnDb(DgnDbR dgndb, BeFileNameCR pathname);
    static BeSQLite::DbResult InsertInstance(Utf8StringR insertedId, BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static BeSQLite::DbResult UpdateInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static BeSQLite::DbResult ReadInstance(JsonValueR jsonInstance, BeSQLite::EC::ECDbCR ecdb, JsonValueCR instanceKey);
    static BeSQLite::DbResult DeleteInstance(JsECDbR ecdb, JsonValueCR instanceKey);
    static BeSQLite::DbResult ContainsInstance(bool& containsInstance, JsECDbR ecdb, JsonValueCR instanceKey);
    static BeSQLite::DbResult ExecuteQuery(JsonValueR results, BeSQLite::EC::ECSqlStatement& stmt, JsonValueCR bindings);
    static BeSQLite::DbResult ExecuteStatement(Utf8StringR instanceId, BeSQLite::EC::ECSqlStatement& stmt, bool isInsertStmt, JsonValueCR bindings);
    static Utf8String GetLastEcdbIssue();
    static BeSQLite::DbResult GetCachedBriefcaseInfos(JsonValueR jsonBriefcaseInfos, BeFileNameCR cachePath);
    static void GetRootSubjectInfo(JsonValueR rootSubjectInfo, DgnDbCR dgndb);
    static void GetExtents(JsonValueR extentsJson, DgnDbCR dgndb);

    static DgnPlatformLib::Host::RepositoryAdmin& GetRepositoryAdmin();
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestToInsertElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemProps);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForElementById(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemIdJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForModel(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestToLockModel(IBriefcaseManager::Request& req, DgnDbR dgndb, DgnModelId mid, LockLevel level);

    static RepositoryStatus SetBriefcaseManagerOptimisticConcurrencyControlPolicy(DgnDbR dgndb, BriefcaseManagerConflictResolution uu, BriefcaseManagerConflictResolution ud, BriefcaseManagerConflictResolution du);
    static RepositoryStatus SetBriefcaseManagerPessimisticConcurrencyControlPolicy(DgnDbR dgndb);
    static RepositoryStatus BriefcaseManagerStartBulkOperation(DgnDbR dgndb);
    static RepositoryStatus BriefcaseManagerEndBulkOperation(DgnDbR dgndb);

    static NativeLogging::ILogger& GetLogger();
    
};

END_BENTLEY_DGN_NAMESPACE
