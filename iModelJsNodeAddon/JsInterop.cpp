/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInterop.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (_WIN32) && !defined(BENTLEY_WINRT)
#include <windows.h>
#endif
#include "IModelJsNative.h"
#include <Bentley/Base64Utilities.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <DgnPlatform/FunctionalDomain.h>

#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

static Utf8String s_lastECDbIssue;
static BeFileName s_addonDllDir;
static BeFileName s_tempDir;
static bool s_useTileCache = true;

using namespace ElementDependency;

namespace IModelJsNative {

BE_JSON_NAME(parentId)
BE_JSON_NAME(pathname)
BE_JSON_NAME(containsSchemaChanges)
BE_JSON_NAME(codeSpecId)
BE_JSON_NAME(codeScope)
BE_JSON_NAME(value)
BE_JSON_NAME(state)

/*=================================================================================**//**
* An implementation of IKnownLocationsAdmin that is useful for desktop applications.
* This implementation works for Windows, Linux, and MacOS.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct KnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName m_tempDirectory;
    BeFileName m_assetsDirectory;

    BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tempDirectory;}
    BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_assetsDirectory;}

    //! Construct an instance of the KnownDesktopLocationsAdmin
    KnownLocationsAdmin()
        {
        m_tempDirectory = s_tempDir;
        m_assetsDirectory = s_addonDllDir;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
};

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  05/17
//=======================================================================================
struct JsDgnHost : DgnPlatformLib::Host {
private:
    BeMutex m_mutex;

    void _SupplyProductName(Utf8StringR name) override { name.assign("IModelJs"); }
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownLocationsAdmin(); }
    BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlang.AppendToPath(L"sqlang/iModelJsNodeAddon_en.sqlang.db3");
        return BeSQLite::L10N::SqlangFiles(sqlang);
        }

    RepositoryAdmin& _SupplyRepositoryAdmin() override {return JsInterop::GetRepositoryAdmin();}

public:
    JsDgnHost() { BeAssertFunctions::SetBeAssertHandler(&handleAssertion);}
};

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  02/18
//=======================================================================================
struct NativeLoggingShim : NativeLogging::Provider::ILogProvider
{
    int STDCALL_ATTRIBUTE Initialize() override {return SUCCESS;}

    int STDCALL_ATTRIBUTE Uninitialize() override {return SUCCESS;}

    int STDCALL_ATTRIBUTE CreateLogger(WCharCP nameSpace, NativeLogging::Provider::ILogProviderContext** ppContext) override
        {
        *ppContext = reinterpret_cast<NativeLogging::Provider::ILogProviderContext*>(new WString(nameSpace));
        return SUCCESS;
        }

    int STDCALL_ATTRIBUTE DestroyLogger(NativeLogging::Provider::ILogProviderContext* pContext) override
        {
        WString* ns = reinterpret_cast<WString*>(pContext);
        if(nullptr != ns)
            delete ns;
        return SUCCESS;
        }

    int STDCALL_ATTRIBUTE SetOption(WCharCP attribName, WCharCP attribValue) override {BeAssert(false); return SUCCESS;}

    int STDCALL_ATTRIBUTE GetOption(WCharCP attribName, WCharP attribValue, uint32_t valueSize) override {return ERROR;}

    void STDCALL_ATTRIBUTE LogMessage(NativeLogging::Provider::ILogProviderContext* context, NativeLogging::SEVERITY sev, WCharCP msg) override
        {
        LogMessage(context, sev, Utf8String(msg).c_str());
        }

    int  STDCALL_ATTRIBUTE SetSeverity(WCharCP nameSpace, NativeLogging::SEVERITY severity) override
        {
        BeAssert(false && "only the app (in TypeScript) sets severities");
        return ERROR;
        }

    void STDCALL_ATTRIBUTE LogMessage(NativeLogging::Provider::ILogProviderContext* context, NativeLogging::SEVERITY sev, Utf8CP msg) override
        {
        WString* ns = reinterpret_cast<WString*>(context);
        JsInterop::LogMessage(Utf8String(*ns).c_str(), sev, msg);
        }

    bool STDCALL_ATTRIBUTE IsSeverityEnabled(NativeLogging::Provider::ILogProviderContext* context, NativeLogging::SEVERITY sev) override
        {
        WString* ns = reinterpret_cast<WString*>(context);
        return JsInterop::IsSeverityEnabled(Utf8String(*ns).c_str(), sev);
        }

};

} // namespace IModelJsNative

using namespace IModelJsNative;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
void JsInterop::Initialize(BeFileNameCR addonDllDir, Napi::Env env, BeFileNameCR tempDir)
    {
    Env() = env;
    MainThreadId() = BeThreadUtilities::GetCurrentThreadId();
    s_addonDllDir = addonDllDir;
    s_tempDir = tempDir;

#if defined(BENTLEYCONFIG_OS_WINDOWS_DESKTOP) // excludes WinRT
    // Include this location for delay load of pskernel...
    WString newPath;
    newPath = L"PATH=" + addonDllDir + L";";
    newPath.append(::_wgetenv(L"PATH"));
    _wputenv(newPath.c_str());

    // Defeat node's attempt to turn off WER
    auto errMode = GetErrorMode();
    errMode &= ~SEM_NOGPFAULTERRORBOX;
    SetErrorMode(errMode);
#endif

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() 
        {
        auto jsHost = new JsDgnHost();
        DgnPlatformLib::Initialize(*jsHost);
        RegisterOptionalDomains();
        InitLogging();
        InitializeParasolid();
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                09/18
//---------------------------------------------------------------------------------------
void JsInterop::RegisterOptionalDomains()
    {
    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
void JsInterop::InitLogging()
    {
    NativeLogging::LoggingConfig::ActivateProvider(new NativeLoggingShim());
    }

#if defined (BENTLEYCONFIG_PARASOLID) 
static RefCountedPtr<PSolidThreadUtil::MainThreadMark> s_psolidMainThreadMark;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::InitializeParasolid()
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    PSolidKernelManager::StartSession();

    if (s_psolidMainThreadMark.IsNull())
        s_psolidMainThreadMark = new PSolidThreadUtil::MainThreadMark();
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
NativeLogging::ILogger& JsInterop::GetLogger()
    {
    static NativeLogging::ILogger* s_logger;
    if (nullptr == s_logger)
        s_logger = NativeLogging::LoggingManager::GetLogger("imodeljs-addon"); // This is thread-safe. The assignment is atomic, and GetLogger will always return the same value for a given key anyway.
    return *s_logger;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 02/18
//---------------------------------------------------------------------------------------
DgnDbPtr JsInterop::CreateIModel(DbResult& result, Utf8StringCR name, JsonValueCR in, Napi::Env env)
    {
    result = BE_SQLITE_NOTFOUND;
    JsonValueCR rootSubject = in[json_rootSubject()];
    if (rootSubject.isNull() || !rootSubject.isMember(json_name())) {
        Napi::TypeError::New(env, "Root subject name is missing").ThrowAsJavaScriptException();
        return nullptr;
    }

    BeFileName fileName(name);
    BeFileName path =fileName.GetDirectoryName();
    if (!path.DoesPathExist()) {
        Utf8String err = Utf8String("Path [") + path.GetNameUtf8() + "] does not exist"; 
        Napi::TypeError::New(env, err.c_str()).ThrowAsJavaScriptException();
        return nullptr;
    }

    CreateDgnDbParams params(rootSubject[json_name()].asCString());
    if (rootSubject.isMember(json_description()))
        params.SetRootSubjectDescription(rootSubject[json_description()].asCString());
    if (in.isMember(json_globalOrigin()))
        params.m_globalOrigin = JsonUtils::ToDPoint3d(in[json_globalOrigin()]);
    if (in.isMember(json_guid()))
        params.m_guid.FromString(in[json_guid()].asCString());
    if (in.isMember(json_projectExtents()))
        params.m_projectExtents.FromJson(in[json_projectExtents()]);
    if (in.isMember(json_client()))
        params.m_client = in[json_client()].asCString();

    DgnDbPtr db = DgnDb::CreateDgnDb(&result, fileName, params);
    if (!db.IsValid()) 
        return nullptr;

    // NEEDS_WORK - create GCS from ecef location
    // NEEDS_WORK - save thumbnail.

    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
DbResult JsInterop::OpenDgnDb(DgnDbPtr& db, BeFileNameCR fileOrPathname, DgnDb::OpenMode mode)
    {
    BeFileName pathname;
    if (fileOrPathname.DoesPathExist())
        {
        pathname = fileOrPathname;
        }
    else
        {
        // *** NEEDS WORK: To save the user the trouble of typing in a full path name, we'll let him
        //                  define an envvar that defines the directory.
        BeFileName dbDir;
#if defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)
        Utf8CP dbdirenv = getenv("NODE_DGNDB_DIR");
#else
        auto mobileDir = DgnPlatformLib::GetHost().GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName().GetNameUtf8();
        Utf8CP dbdirenv = mobileDir.c_str();
#endif
        if (nullptr != dbdirenv)
            dbDir.SetNameUtf8(dbdirenv);
        else
            {
            Desktop::FileSystem::GetCwd(dbDir);
            dbDir.AppendToPath(L"briefcases");

            if (!dbDir.DoesPathExist())
                return DbResult::BE_SQLITE_NOTFOUND;
            }
            
        pathname = dbDir;
        pathname.AppendToPath(L"../Documents/");
        pathname.AppendToPath(fileOrPathname.c_str());
        }

    DbResult result;

    SchemaUpgradeOptions schemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::CheckRequiredUpgrades);
    DgnDb::OpenParams openParams(mode, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);

    db = DgnDb::OpenDgnDb(&result, pathname, openParams);

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::ReadChangeSet(DgnRevisionPtr& revision, Utf8StringCR dbGuid, JsonValueCR changeSetToken)
    {
    PRECONDITION(changeSetToken.isMember("id") && changeSetToken.isMember("pathname"), RevisionStatus::FileNotFound);

    Utf8String id = changeSetToken["id"].asString();
    Utf8String parentId = changeSetToken["parentId"].asString();

    RevisionStatus revStatus;
    revision = DgnRevision::Create(&revStatus, id, parentId, dbGuid);
    PRECONDITION(revStatus == RevisionStatus::Success, revStatus);
    BeAssert(revision.IsValid());

    BeFileName changeSetPathname(changeSetToken["pathname"].asCString(), true);
    PRECONDITION(changeSetPathname.DoesPathExist(), RevisionStatus::FileNotFound);

    if (!changeSetToken["pushDate"].isNull())
        {
        Utf8String pushDate = changeSetToken["pushDate"].asString();
        DateTime date;
        DateTime::FromString(date, pushDate.c_str());
        revision->SetDateTime(date);
        }

    revision->SetRevisionChangesFile(changeSetPathname);
    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::DumpChangeSet(DgnDbR dgndb, JsonValueCR changeSetToken)
    {
    DgnRevisionPtr revision;
    RevisionStatus status = ReadChangeSet(revision, dgndb.GetDbGuid().ToString(), changeSetToken);
    if (RevisionStatus::Success != status)
        return status;
    revision->Dump(dgndb);
    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::ReadChangeSets(bvector<DgnRevisionPtr>& revisionPtrs, bool& containsSchemaChanges, Utf8StringCR dbGuid, JsonValueCR changeSetTokens)
    {
    revisionPtrs.clear();
    containsSchemaChanges = false;
    PRECONDITION(!changeSetTokens.isNull() && changeSetTokens.isArray(), RevisionStatus::FileNotFound);

    for (uint32_t ii = 0; ii < changeSetTokens.size(); ii++)
        {
        JsonValueCR changeSetToken = changeSetTokens[ii];

        DgnRevisionPtr revision;
        RevisionStatus status = ReadChangeSet(revision, dbGuid, changeSetToken);
        if (RevisionStatus::Success != status)
            return status;

        if (!containsSchemaChanges)
            containsSchemaChanges = changeSetToken.isMember("containsSchemaChanges") && changeSetToken["containsSchemaChanges"].asBool();

        revisionPtrs.push_back(revision);
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::ApplySchemaChangeSets(BeFileNameCR dbFileName, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption applyOption)
    {
    SchemaUpgradeOptions schemaUpgradeOptions(revisions, applyOption);
    schemaUpgradeOptions.SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);

    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);
    DbResult result;
    DgnDbPtr dgndb = DgnDb::OpenDgnDb(&result, dbFileName, openParams);
    POSTCONDITION(result == BE_SQLITE_OK, RevisionStatus::ApplyError);

    dgndb->CloseDb();
    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::ApplyDataChangeSets(DgnDbR dgndb, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption applyOption)
    {
    PRECONDITION(dgndb.IsDbOpen() && "Expected briefcase to be open when applying only data changes", RevisionStatus::ApplyError);
    return dgndb.Revisions().ProcessRevisions(revisions, applyOption);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::StartCreateChangeSet(JsonValueR changeSetInfo, DgnDbR dgndb)
    {
    RevisionManagerR revisions = dgndb.Revisions();

    if (revisions.IsCreatingRevision())
        revisions.AbandonCreateRevision();

    RevisionStatus status;
    DgnRevisionPtr revision = revisions.StartCreateRevision(&status);
    if (status != RevisionStatus::Success)
        return status;
    BeAssert(revision.IsValid());

    changeSetInfo = Json::objectValue;
    changeSetInfo[json_id()] = revision->GetId().c_str();
    changeSetInfo[json_parentId()] = revision->GetParentId().c_str();
    changeSetInfo[json_pathname()] = Utf8String(revision->GetRevisionChangesFile()).c_str();
    changeSetInfo[json_containsSchemaChanges()] = revision->ContainsSchemaChanges(dgndb) ? 1 : 0;
    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::FinishCreateChangeSet(DgnDbR dgndb)
    {
    return dgndb.Revisions().FinishCreateRevision();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              03/18
//---------------------------------------------------------------------------------------
void JsInterop::AbandonCreateChangeSet(DgnDbR dgndb)
    {
    RevisionManagerR revisions = dgndb.Revisions();

    if (revisions.IsCreatingRevision())
        revisions.AbandonCreateRevision();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              03/18
//---------------------------------------------------------------------------------------
static void convertCodeToJson(JsonValueR json, DgnCodeCR code, int codeState)
    {
    json = Json::objectValue;
    json[json_codeSpecId()] = code.GetCodeSpecId().ToHexStr();
    json[json_codeScope()] = code.GetScopeString();
    json[json_value()] = code.GetValueUtf8();
    json[json_state()] = codeState;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              03/18
//---------------------------------------------------------------------------------------
static void convertCodeSetToJson(JsonValueR json, DgnCodeSet const& codes, int codeState)
    {
    int i = json.size();
    for (DgnCodeCR code : codes)
        convertCodeToJson(json[i++], code, codeState);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              03/18
//---------------------------------------------------------------------------------------
DbResult ExtractCodesFromChangeSet(JsonValueR codes, DgnRevisionPtr revision, DgnDbR dgndb)
    {
    DgnCodeSet usedCodes;
    DgnCodeSet discardedCodes;
    revision->ExtractCodes(usedCodes, discardedCodes, dgndb);

    codes = Json::arrayValue;
    convertCodeSetToJson(codes, usedCodes, 2);
    convertCodeSetToJson(codes, discardedCodes, 3);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              03/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::ExtractCodes(JsonValueR codes, DgnDbR dgndb)
    {
    RevisionManagerR revisions = dgndb.Revisions();

    if (!revisions.IsCreatingRevision())
        return BE_SQLITE_ERROR;

    return ExtractCodesFromChangeSet(codes, revisions.GetCreatingRevision(), dgndb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              04/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::ExtractCodesFromFile(JsonValueR codes, DgnDbR dgndb, JsonValueCR changeSetToken)
    {
    bvector<DgnRevisionPtr> revisionPtrs;
    bool containsSchemaChanges;
    RevisionStatus status = ReadChangeSets(revisionPtrs, containsSchemaChanges, dgndb.GetDbGuid().ToString(), changeSetToken);
    if (RevisionStatus::Success != status)
        return BE_SQLITE_ERROR;

    if (revisionPtrs.empty())
        return BE_SQLITE_ERROR;
    
    return ExtractCodesFromChangeSet(codes, revisionPtrs[0], dgndb);
    }

#define PENDING_CHANGESET_PROPERTY_NAME "PendingChangeSets"

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              04/18
//---------------------------------------------------------------------------------------
DbResult SavePendingChangeSets(DgnDbR dgndb, JsonValueCR changeSets)
    {
    DbResult result = dgndb.SaveBriefcaseLocalValue(PENDING_CHANGESET_PROPERTY_NAME, changeSets.ToString());
    
    if (BE_SQLITE_DONE != result)
        return result;

    return dgndb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              04/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::GetPendingChangeSets(JsonValueR changeSets, DgnDbR dgndb)
    {
    Utf8String savedValue;

    dgndb.QueryBriefcaseLocalValue(savedValue, PENDING_CHANGESET_PROPERTY_NAME);
    if (Utf8String::IsNullOrEmpty(savedValue.c_str()))
        changeSets = Json::arrayValue;
    else
        Json::Reader::Parse(savedValue, changeSets);

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              04/18
//---------------------------------------------------------------------------------------
Json::ArrayIndex FindChangeSet(JsonValueR changeSets, Utf8StringCR changeSetId)
    {
    for (Json::ArrayIndex i = 0; i < changeSets.size(); ++i)
        {
        if (changeSets[i].asString() == changeSetId)
            return i;
        }
    return -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              04/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::AddPendingChangeSet(DgnDbR dgndb, Utf8StringCR changeSetId)
    {
    Json::Value changeSets;
    DbResult result = GetPendingChangeSets(changeSets, dgndb);
    if (BE_SQLITE_OK != result)
        return result;

    if (-1 != FindChangeSet(changeSets, changeSetId))
        return BE_SQLITE_OK;
    
    changeSets.append(changeSetId);

    return SavePendingChangeSets(dgndb, changeSets);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Karolis.Dziedzelis              04/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::RemovePendingChangeSet(DgnDbR dgndb, Utf8StringCR changeSetId)
    {
    Json::Value changeSets;
    DbResult result = GetPendingChangeSets(changeSets, dgndb);
    if (BE_SQLITE_OK != result)
        return result;

    Json::ArrayIndex foundIndex = FindChangeSet(changeSets, changeSetId);

    if (-1 == foundIndex)
        return BE_SQLITE_OK;
    
    changeSets.removeIndex(foundIndex);

    return SavePendingChangeSets(dgndb, changeSets);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
void JsInterop::GetRowAsJson(Json::Value& rowJson, ECSqlStatement& stmt) 
    {
    JsonECSqlSelectAdapter adapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsHexadecimalString));
    adapter.GetRow(rowJson, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
void JsInterop::GetECValuesCollectionAsJson(Json::Value& json, ECN::ECValuesCollectionCR props) 
    {
    for (ECN::ECPropertyValue const& prop : props)
        {
        JsonValueR pvalue = json[prop.GetValueAccessor().GetAccessString(prop.GetValueAccessor().GetDepth()-1)];

        if (prop.HasChildValues())
            GetECValuesCollectionAsJson(pvalue, *prop.GetChildValues());
        else 
            ECUtils::ConvertECValueToJson(pvalue, prop.GetValue());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult JsInterop::OpenECDb(ECDbR ecdb, BeFileNameCR pathname, BeSQLite::Db::OpenParams const& params)
    {
    if (!pathname.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    DbResult res = ecdb.OpenBeSQLiteDb(pathname, params);
    if (res != BE_SQLITE_OK)
        return res;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult JsInterop::CreateECDb(ECDbR ecdb, BeFileNameCR pathname)
    {
    BeFileName path = pathname.GetDirectoryName();
    if (!path.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    DbResult res = ecdb.CreateNewDb(pathname);
    if (res != BE_SQLITE_OK)
        return res;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult JsInterop::ImportSchema(ECDbR ecdb, BeFileNameCR pathname)
    {
    if (!pathname.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);
    schemaContext->SetFinalSchemaLocater(ecdb.GetSchemaLocater());

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile(schema, pathname.GetName(), *schemaContext);
    if (SchemaReadStatus::Success != schemaStatus)
        return BE_SQLITE_ERROR;

    bvector<ECSchemaCP> schemas;
    schemas.push_back(schema.get());
    BentleyStatus status = ecdb.Schemas().ImportSchemas(schemas);
    if (status != SUCCESS)
        return BE_SQLITE_ERROR;

    return ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult JsInterop::ImportSchemaDgnDb(DgnDbR dgndb, BeFileNameCR pathname)
    {
    if (!pathname.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);
    schemaContext->SetFinalSchemaLocater(dgndb.GetSchemaLocater());

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile(schema, pathname.GetName(), *schemaContext);
    if (SchemaReadStatus::Success != schemaStatus)
        return BE_SQLITE_ERROR;

    bvector<ECSchemaCP> schemas;
    schemas.push_back(schema.get());
    SchemaStatus status = dgndb.ImportSchemas(schemas);
    if (status != SchemaStatus::Success)
        return DgnDb::SchemaStatusToDbResult(status, true);

    return dgndb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                09/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::ImportFunctionalSchema(DgnDbR db)
    {
    return SchemaStatus::Success == FunctionalDomain::GetDomain().ImportSchema(db) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
ECClassCP JsInterop::GetClassFromInstance(ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    return ECJsonUtilities::GetClassFromClassNameJson(jsonInstance[ECJsonUtilities::json_className()], ecdb.GetClassLocater());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
ECInstanceId JsInterop::GetInstanceIdFromInstance(ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    if (!jsonInstance.isMember(ECJsonUtilities::json_id()))
        return ECInstanceId();

    ECInstanceId instanceId;
    if (SUCCESS != ECInstanceId::FromString(instanceId, jsonInstance[ECJsonUtilities::json_id()].asCString()))
        return ECInstanceId();

    return instanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::ThrowJsException(Utf8CP msg) { Napi::Error::New(Env(), msg).ThrowAsJavaScriptException(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::SetUseTileCache(bool use) { s_useTileCache = use; }
bool JsInterop::GetUseTileCache() { return s_useTileCache; }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
HexStrSqlFunction& HexStrSqlFunction::GetSingleton()
    {
    static HexStrSqlFunction* s_singleton = nullptr;
    if (s_singleton == nullptr)
        s_singleton = new HexStrSqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
void HexStrSqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& numValue = args[0];
    if (numValue.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    if (numValue.GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Argument of function HEXSTR is expected to be an integral number.");
        return;
        }

    static const size_t stringBufferLength = 19;
    Utf8Char stringBuffer[stringBufferLength];
    BeStringUtilities::FormatUInt64(stringBuffer, stringBufferLength, numValue.GetValueUInt64(), HexFormatOptions::IncludePrefix);
    ctx.SetResultText(stringBuffer, (int) strlen(stringBuffer), Context::CopyData::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
StrSqlFunction& StrSqlFunction::GetSingleton()
    {
    static StrSqlFunction* s_singleton = nullptr;
    if (s_singleton == nullptr)
        s_singleton = new StrSqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
void StrSqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& numValue = args[0];
    if (numValue.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    if (numValue.GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Argument of function STR is expected to be an integral number.");
        return;
        }

    static const size_t stringBufferLength = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

    Utf8Char stringBuffer[stringBufferLength]; //+1 for the trailing 0 character;
    BeStringUtilities::FormatUInt64(stringBuffer, numValue.GetValueUInt64());
    ctx.SetResultText(stringBuffer, (int) strlen(stringBuffer), Context::CopyData::Yes);
    }
