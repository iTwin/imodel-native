/*--------------------------------------------------------------------------------------+
|
|     $Source: AddonUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

struct AddonUtils
{
    BE_JSON_NAME(rootSubject)
    BE_JSON_NAME(name)
    BE_JSON_NAME(description)
    BE_JSON_NAME(projectExtents)
    BE_JSON_NAME(globalOrigin)
    BE_JSON_NAME(ecefTrans)

private:
    static BeSQLite::DbResult ReadChangeSets(bvector<DgnRevisionPtr>& revisionPtrs, bool& containsSchemaChanges, DgnDbR dgndb, JsonValueCR changeSetTokens);
    static BeSQLite::DbResult ProcessSchemaChangeSets(DgnDbPtr& dgndb, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption);
    static BeSQLite::DbResult ProcessDataChangeSets(DgnDbR dgndb, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption);

public:
    static void GetRowAsJson(Json::Value &json, BeSQLite::EC::ECSqlStatement &);
    static void GetECValuesCollectionAsJson(Json::Value &json, ECN::ECValuesCollectionCR);
    static ECN::ECClassCP GetClassFromInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static BeSQLite::EC::ECInstanceId GetInstanceIdFromInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static void InitLogging();

    //! @deprecated Use NodeAddonECSqlStatement::GetBinder instead
    struct JsonBinder
    {
      private:
        static BentleyStatus BindPrimitiveValue(BeSQLite::EC::IECSqlBinder &binder, JsonValueCR bindingValue);
        static BentleyStatus BindArrayValue(BeSQLite::EC::IECSqlBinder &binder, JsonValueCR value);
        static BentleyStatus BindStructValue(BeSQLite::EC::IECSqlBinder &binder, JsonValueCR value);
        static BentleyStatus BindValue(BeSQLite::EC::IECSqlBinder &binder, JsonValueCR bindingValue);

      public:
        static BentleyStatus BindValues(BeSQLite::EC::ECSqlStatement &stmt, JsonValueCR bindings);
    };
    
    typedef std::function<void(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType)> T_AssertHandler;

    static void Initialize(BeFileNameCR, T_AssertHandler assertHandler);
    static BeSQLite::DbResult OpenDgnDb(DgnDbPtr &, BeFileNameCR dbname, DgnDb::OpenMode mode);
    static BeSQLite::DbResult SetupBriefcase(DgnDbPtr &db, JsonValueCR briefcaseToken);
    static void CloseDgnDb(DgnDbR dgndb);
    static DgnDbStatus GetECClassMetaData(JsonValueR results, DgnDbR db, Utf8CP schema, Utf8CP ecclass);
    static DgnDbStatus GetElement(JsonValueR results, DgnDbR db, Json::Value const &inOpts);
    static DgnDbStatus InsertElement(JsonValueR results, DgnDbR db, Json::Value &props);
    static DgnDbStatus UpdateElement(DgnDbR db, Json::Value &props);
    static DgnDbStatus DeleteElement(DgnDbR db, Utf8StringCR eidStr);
    static DbResult InsertLinkTableRelationship(JsonValueR results, DgnDbR db, Json::Value& props);
    static DbResult UpdateLinkTableRelationship(DgnDbR db, Json::Value& props);
    static DbResult DeleteLinkTableRelationship(DgnDbR db, Json::Value& props);
    static DgnDbStatus InsertCodeSpec(Utf8StringR idStr, DgnDbR db, Utf8StringCR name, CodeScopeSpec::Type cstype, CodeScopeSpec::ScopeRequirement cssreq);
    static DgnDbStatus InsertModel(JsonValueR results, DgnDbR db, Json::Value &props);
    static DgnDbStatus UpdateModel(DgnDbR db, Json::Value &props);
    static DgnDbStatus DeleteModel(DgnDbR db, Utf8StringCR idStr);
    static DgnDbStatus GetModel(JsonValueR results, DgnDbR db, Json::Value const &inOpts);
	static DgnDbStatus UpdateProjectExtents(DgnDbR dgndb, JsonValueCR newExtents);

    static BeSQLite::DbResult CreateECDb(ECDbR, BeFileNameCR pathname);
    static BeSQLite::DbResult OpenECDb(ECDbR, BeFileNameCR pathname, BeSQLite::Db::OpenMode openMode);
    static BeSQLite::DbResult ImportSchema(BeSQLite::EC::ECDbR ecdb, BeFileNameCR pathname);
    static BeSQLite::DbResult ImportSchemaDgnDb(DgnDbR dgndb, BeFileNameCR pathname);
    static Utf8StringCR GetLastECDbIssue();
    static BeSQLite::DbResult GetCachedBriefcaseInfos(JsonValueR jsonBriefcaseInfos, BeFileNameCR cachePath);
    
    static BeSQLite::DbResult ProcessChangeSets(DgnDbPtr& dgndb, JsonValueCR jsonChangeSetTokens, RevisionProcessOption processOption);
    static BeSQLite::DbResult StartCreateChangeSet(JsonValueR changeSetInfo, DgnDbR dgndb);
    static BeSQLite::DbResult FinishCreateChangeSet(DgnDbR dgndb);

    static void GetIModelProps(JsonValueR, DgnDbCR dgndb);
    
    static DgnPlatformLib::Host::RepositoryAdmin& GetRepositoryAdmin();
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestToInsertElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemProps);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForElementById(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemIdJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForCodeSpec(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForModel(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestToLockModel(IBriefcaseManager::Request& req, DgnDbR dgndb, DgnModelId mid, LockLevel level);
    static RepositoryStatus BriefcaseManagerStartBulkOperation(DgnDbR dgndb);
    static RepositoryStatus BriefcaseManagerEndBulkOperation(DgnDbR dgndb);

    static void ThrowJsException(Utf8CP msg);

    static NativeLogging::ILogger& GetLogger();
    
};

END_BENTLEY_DGN_NAMESPACE
