/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/Tile.h>
#include <ECDb/ECDbApi.h>
#include <DgnPlatform/ECUtils.h>
#include <DgnPlatform/DgnFontData.h>
#include <Bentley/BeThread.h>
#include <Bentley/CancellationToken.h>
#include <Napi/napi.h>
#include <DgnPlatform/DgnGeoCoord.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_EC

BEGIN_TILE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Result);
DEFINE_POINTER_SUFFIX_TYPEDEFS(PollResult);

//=======================================================================================
// The current state of a Tile::Request.
// @bsistruct                                                   Paul.Connelly   02/19
//=======================================================================================
enum class State : uint8_t
{
    New, // Request was just created and enqueued
    Pending, // Request is already queued but not yet being processed
    Loading, // Request is being processed on worker thread
    Completed, // Request finished processing
};

//=======================================================================================
// The result of a Tile::Request.
// @bsistruct                                                   Paul.Connelly   02/19
//=======================================================================================
struct Result
{
    ContentCPtr m_content;
    DgnDbStatus m_status = DgnDbStatus::BadRequest;
};

//=======================================================================================
// The result of polling a request's current state.
// @bsistruct                                                   Paul.Connelly   02/19
//=======================================================================================
struct PollResult : Result
{
    State   m_state;

    explicit PollResult(State state) : m_state(state) { }
    explicit PollResult(Result const& result) : Result(result), m_state(State::Completed) { }
    explicit PollResult(DgnDbStatus status) : m_state(State::Completed)
        {
        m_status = status;
        BeAssert(DgnDbStatus::Success != status);
        }
};

END_TILE_NAMESPACE

namespace IModelJsNative {

void handleAssertion(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type);

struct JsInterop
{
    static bmap<Utf8String, Utf8String> s_crashReportProperties;
    static bmap<Dgn::DgnDb*, BeFileName> s_openDgnDbFileNames;

    struct CrashReportingConfig
        {
        BeFileName m_crashDir;
        bmap<Utf8String,Utf8String> m_params;
        size_t m_maxDumpsInDir;
        bool m_writeDumpsToCrashDir;
        bool m_wantFullMemory;
        bool m_needsVectorExceptionHandler;
        };

    // An indirect reference to an ObjectReference. Keeps the ObjectReference alive. Can be redeemed
    //  only on the main thread. Can be copied on other threads.
    struct ObjectReferenceClaimCheck
        {
        private:
        friend struct JsInterop;

        Utf8String m_id;

        explicit ObjectReferenceClaimCheck(Utf8StringCR);

        public:
        ObjectReferenceClaimCheck();
        ~ObjectReferenceClaimCheck();
        ObjectReferenceClaimCheck(ObjectReferenceClaimCheck const&);
        ObjectReferenceClaimCheck(ObjectReferenceClaimCheck&&);
        ObjectReferenceClaimCheck& operator=(ObjectReferenceClaimCheck const&);

        bool operator<(ObjectReferenceClaimCheck const&) const;

        Utf8StringCR GetId() const {return m_id;}

        void Dispose();
        };

    BE_JSON_NAME(briefcaseId)
    BE_JSON_NAME(openMode)
    BE_JSON_NAME(client)
    BE_JSON_NAME(description)
    BE_JSON_NAME(ecefLocation)
    BE_JSON_NAME(element)
    BE_JSON_NAME(fileName)
    BE_JSON_NAME(globalOrigin)
    BE_JSON_NAME(guid)
    BE_JSON_NAME(id)
    BE_JSON_NAME(modelExtents)
    BE_JSON_NAME(name)
    BE_JSON_NAME(namespace)
    BE_JSON_NAME(orientation)
    BE_JSON_NAME(origin)
    BE_JSON_NAME(projectExtents)
    BE_JSON_NAME(rootSubject)
    BE_JSON_NAME(subId)
    BE_JSON_NAME(value)

private:

    static RevisionStatus ReadChangeSet(DgnRevisionPtr& revisionPtr, Utf8StringCR dbGuid, JsonValueCR changeSetToken);
    static void GetRowAsJson(Json::Value &json, BeSQLite::EC::ECSqlStatement &);
    static void RegisterOptionalDomains();
    static void InitializeParasolid();
public:
    static void GetECValuesCollectionAsJson(Json::Value &json, ECN::ECValuesCollectionCR);
    static ECN::ECClassCP GetClassFromInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static BeSQLite::EC::ECInstanceId GetInstanceIdFromInstance(BeSQLite::EC::ECDbCR ecdb, JsonValueCR jsonInstance);
    static void InitLogging();
    static void Initialize(BeFileNameCR, Napi::Env, BeFileNameCR);
    static DgnDbPtr CreateIModel(DbResult& db, Utf8StringCR name, JsonValueCR, Napi::Env);
    static BeSQLite::DbResult OpenDgnDb(DgnDbPtr &, BeFileNameCR dbname, DgnDb::OpenMode mode);
    static void CloseDgnDb(DgnDbR dgndb);
    static DgnDbStatus GetECClassMetaData(JsonValueR results, DgnDbR db, Utf8CP schema, Utf8CP ecclass);
    static DgnDbStatus GetSchemaItem(JsonValueR results, DgnDbR db, Utf8CP schemaName, Utf8CP itemName);
    static DgnDbStatus GetSchema(JsonValueR results, DgnDbR db, Utf8CP name);
    static DgnDbStatus GetElement(JsonValueR results, DgnDbR db, Json::Value const &inOpts);
    static DgnDbStatus InsertElement(JsonValueR results, DgnDbR db, Json::Value &props);
    static DgnDbStatus UpdateElement(DgnDbR db, Json::Value &props);
    static DgnDbStatus DeleteElement(DgnDbR db, Utf8StringCR eidStr);
    static DgnDbStatus InsertElementAspect(DgnDbR db, JsonValueCR aspectProps);
    static DgnDbStatus UpdateElementAspect(DgnDbR db, JsonValueCR aspectProps);
    static DgnDbStatus DeleteElementAspect(DgnDbR db, Utf8StringCR aspectIdStr);
    static DgnDbStatus ExportGraphics(DgnDbR db, Napi::Object const& exportProps);
    static DbResult InsertLinkTableRelationship(JsonValueR results, DgnDbR db, Json::Value& props);
    static DbResult UpdateLinkTableRelationship(DgnDbR db, Json::Value& props);
    static DbResult DeleteLinkTableRelationship(DgnDbR db, Json::Value& props);
    static DgnDbStatus InsertCodeSpec(Utf8StringR idStr, DgnDbR db, Utf8StringCR name, CodeScopeSpec::Type cstype, CodeScopeSpec::ScopeRequirement cssreq);
    static DgnDbStatus InsertModel(JsonValueR results, DgnDbR db, Json::Value &props);
    static DgnDbStatus UpdateModel(DgnDbR db, Json::Value &props);
    static DgnDbStatus DeleteModel(DgnDbR db, Utf8StringCR idStr);
    static DgnDbStatus GetModel(JsonValueR results, DgnDbR db, Json::Value const &inOpts);
    static DgnDbStatus QueryModelExtents(JsonValueR extents, DgnDbR db, JsonValueCR options);
    static void UpdateProjectExtents(DgnDbR dgndb, JsonValueCR newExtents);
    static void UpdateIModelProps(DgnDbR dgndb, JsonValueCR);

    static BeSQLite::DbResult CreateECDb(ECDbR, BeFileNameCR pathname);
    static BeSQLite::DbResult OpenECDb(ECDbR, BeFileNameCR pathname, BeSQLite::Db::OpenParams const&);
    static BeSQLite::DbResult ImportSchema(BeSQLite::EC::ECDbR ecdb, BeFileNameCR pathname);
    static BeSQLite::DbResult ImportSchemaDgnDb(DgnDbR dgndb, BeFileNameCR pathname);
    static BeSQLite::DbResult ImportFunctionalSchema(DgnDbR);

    static RevisionStatus ReadChangeSets(bvector<DgnRevisionPtr>& revisionPtrs, bool& containsSchemaChanges, Utf8StringCR dbGuid, JsonValueCR changeSetTokens);
    static RevisionStatus ApplySchemaChangeSets(BeFileNameCR dbFileName, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption applyOption);
    static RevisionStatus ApplyDataChangeSets(DgnDbR dgndb, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption applyOption);
    static RevisionStatus StartCreateChangeSet(JsonValueR changeSetInfo, DgnDbR dgndb);
    static RevisionStatus FinishCreateChangeSet(DgnDbR dgndb);
    static void AbandonCreateChangeSet(DgnDbR dgndb);
    static RevisionStatus DumpChangeSet(DgnDbR dgndb, JsonValueCR changeSetToken);

    static BeSQLite::DbResult ExtractCodes(JsonValueR codes, DgnDbR db);
    static BeSQLite::DbResult ExtractCodesFromFile(JsonValueR codes, DgnDbR db, JsonValueCR changeSetToken);
    static BeSQLite::DbResult GetPendingChangeSets(JsonValueR changeSets, DgnDbR db);
    static BeSQLite::DbResult AddPendingChangeSet(DgnDbR db, Utf8StringCR changeSetId);
    static BeSQLite::DbResult RemovePendingChangeSet(DgnDbR db, Utf8StringCR changeSetId);

    static BentleyStatus GetGeoCoordsFromIModelCoords (JsonValueR, DgnDbR, JsonValueCR);
    static BentleyStatus GetIModelCoordsFromGeoCoords (JsonValueR, DgnDbR, JsonValueCR);

    static void GetIModelProps(JsonValueR, DgnDbCR dgndb);
    
    static DgnPlatformLib::Host::RepositoryAdmin& GetRepositoryAdmin();
    static bool SetOkEndBulkMode(bool);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestToInsertElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemProps);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForElementById(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemIdJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForCodeSpec(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestForModel(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op);
    static RepositoryStatus BuildBriefcaseManagerResourcesRequestToLockModel(IBriefcaseManager::Request& req, DgnDbR dgndb, DgnModelId mid, LockLevel level);
    static RepositoryStatus BriefcaseManagerStartBulkOperation(DgnDbR dgndb);
    static RepositoryStatus BriefcaseManagerEndBulkOperation(DgnDbR dgndb);

    static void GetTileTree(ICancellationTokenPtr, DgnDbR db, Utf8StringCR id, Napi::Function& callback);
    static void GetTileContent(ICancellationTokenPtr, DgnDbR db, Utf8StringCR treeId, Utf8StringCR tileId, Napi::Function& callback);
    static Tile::PollResult PollTileContent(ICancellationTokenPtr, DgnDbR db, Utf8StringCR treeId, Utf8StringCR tileId);
    static void SetUseTileCache(bool use);
    static bool GetUseTileCache();

    static void ThrowJsException(Utf8CP msg);
    static Json::Value ExecuteTest(DgnDbR, Utf8StringCR testName, Utf8StringCR params);
    static NativeLogging::ILogger &GetLogger();

    static void LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg);
    static bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev);
    static ObjectReferenceClaimCheck GetCurrentClientRequestContextForMainThread();
    static BentleyStatus SetCurrentClientRequestContextForWorkerThread(ObjectReferenceClaimCheck const&);
    static ObjectReferenceClaimCheck const& GetCurrentClientRequestContextForWorkerThread();
    static void LogMessageInContext(Utf8StringCR category, NativeLogging::SEVERITY sev, Utf8StringCR msg, ObjectReferenceClaimCheck const& ctx);
    static void DoDeferredLogging();

    static Napi::Env& Env() { static Napi::Env s_env(nullptr); return s_env; }
    static intptr_t& MainThreadId() {static intptr_t s_mainThreadId; return s_mainThreadId;}
    static bool IsMainThread() { return BeThreadUtilities::GetCurrentThreadId() == MainThreadId(); }

    static Tile::TreePtr GetTileTree(GeometricModelR, Tile::Tree::Id const&, bool createIfNotFound);

    static void StepAsync(Napi::Function& callback, Statement& stmt);
    static void StepAsync(Napi::Function& callback, ECSqlStatement& stmt, bool stepForInsert);

    static void AddCrashReportDgnDb(Dgn::DgnDbR);
    static void RemoveCrashReportDgnDb(Dgn::DgnDbR);

    static void SetCrashReportProperty(Utf8StringCR key, Utf8StringCR value);
    static void RemoveCrashReportProperty(Utf8StringCR key);

    static void MaintainCrashDumpDir(int& maxNativeCrashTxtFileNo, CrashReportingConfig const&);
    static bmap<Utf8String,Utf8String> GetCrashReportCustomProperties(CrashReportingConfig const&);

    static void InitializeCrashReporting(CrashReportingConfig const&)
#if defined (USING_GOOGLE_BREAKPAD)
        ;
#else
        {}
#endif
};

//=======================================================================================
//! TEXT HexStr(number INT)
// @bsiclass                                                   Krischan.Eberle       02/18
//=======================================================================================
struct HexStrSqlFunction final : ScalarFunction
    {
    private:
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
        StrSqlFunction() : ScalarFunction("Str", 1, DbValueType::TextVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~StrSqlFunction() {}

        static StrSqlFunction& GetSingleton();
    };

} // namespace IModelJsNative

