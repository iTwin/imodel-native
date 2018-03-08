/*--------------------------------------------------------------------------------------+
|
|     $Source: IModelJsNative.h $
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

namespace IModelJsNative {

struct JsInterop
{
    BE_JSON_NAME(rootSubject)
    BE_JSON_NAME(name)
    BE_JSON_NAME(description)
    BE_JSON_NAME(projectExtents)
    BE_JSON_NAME(globalOrigin)
    BE_JSON_NAME(ecefTrans)

private:
    static BeSQLite::DbResult ReadChangeSets(bvector<DgnRevisionPtr>& revisionPtrs, bool& containsSchemaChanges, Utf8StringCR dbGuid, JsonValueCR changeSetTokens);
    static BeSQLite::DbResult ProcessSchemaChangeSets(bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption, BeFileNameCR dbFileName);
    static BeSQLite::DbResult ProcessDataChangeSets(DgnDbR dgndb, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption);

    static void GetRowAsJson(Json::Value &json, BeSQLite::EC::ECSqlStatement &);

public:
    static void GetECValuesCollectionAsJson(Json::Value &json, ECN::ECValuesCollectionCR);
    static ECN::ECClassCP GetClassFromInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static BeSQLite::EC::ECInstanceId GetInstanceIdFromInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static void InitLogging();

    typedef std::function<void(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType)> T_AssertHandler;

    static void Initialize(BeFileNameCR, T_AssertHandler assertHandler);
    static BeSQLite::DbResult CreateDgnDb(DgnDbPtr& db, BeFileNameCR pathname, Utf8StringCR rootSubjectName, Utf8StringCR rootSubjectDescription);
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
    
    static BeSQLite::DbResult ProcessChangeSets(DgnDbPtr dgndb, JsonValueCR jsonChangeSetTokens, RevisionProcessOption processOption, Utf8StringCR dbGuid, BeFileNameCR dbFileName);
    static BeSQLite::DbResult StartCreateChangeSet(JsonValueR changeSetInfo, DgnDbR dgndb);
    static BeSQLite::DbResult FinishCreateChangeSet(DgnDbR dgndb);
    static void AbandonCreateChangeSet(DgnDbR dgndb);

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
    static Json::Value ExecuteTest(DgnDbR, Utf8StringCR testName, Utf8StringCR params);
    static NativeLogging::ILogger &GetLogger();

    static void LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg);
    static bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev);
};

//=======================================================================================
//! TEXT HexStr(number INT)
// @bsiclass                                                   Krischan.Eberle       02/18
//=======================================================================================
struct HexStrSqlFunction final : ScalarFunction
    {
    private:
        static HexStrSqlFunction* s_singleton;  //no need to release a static non-POD variable (Bentley C++ coding standards)

        HexStrSqlFunction() : ScalarFunction("HexStr", 1, DbValueType::TextVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~HexStrSqlFunction() {}

        static HexStrSqlFunction& GetSingleton();
    };

//=======================================================================================
//! TEXT Str(number INT)
// @bsiclass                                                   Krischan.Eberle       02/18
//=======================================================================================
struct StrSqlFunction final : ScalarFunction
    {
    private:
        static StrSqlFunction* s_singleton;  //no need to release a static non-POD variable (Bentley C++ coding standards)

        StrSqlFunction() : ScalarFunction("Str", 1, DbValueType::TextVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~StrSqlFunction() {}

        static StrSqlFunction& GetSingleton();
    };

} // namespace IModelJsNative

