/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInterop.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <Bentley/Base64Utilities.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeomSerialization/GeomSerializationApi.h>

static Utf8String s_lastECDbIssue;
static BeFileName s_addonDllDir;
static IModelJsNative::JsInterop::T_AssertHandler s_assertHandler;

namespace IModelJsNative {

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
    {
    void _OnIssueReported(BentleyApi::Utf8CP message) const override { s_lastECDbIssue = message;}
    };

static ECDbIssueListener s_listener;

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
        Desktop::FileSystem::BeGetTempPath(m_tempDirectory);
        m_assetsDirectory = s_addonDllDir;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
};

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  05/17
//=======================================================================================
struct DgnPlatformHost : DgnPlatformLib::Host
{
private:
    void _SupplyProductName(Utf8StringR name) override { name.assign("IModelJs"); }
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownLocationsAdmin(); }
    BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlang.AppendToPath(L"sqlang/iModelJsNodeAddon_en.sqlang.db3");
        return BeSQLite::L10N::SqlangFiles(sqlang);
        }

    RepositoryAdmin& _SupplyRepositoryAdmin() override {return JsInterop::GetRepositoryAdmin();}

    static void OnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
        {
        if (s_assertHandler)
            s_assertHandler(msg, file, line, type);
        else
            printf("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
        }
public:
    DgnPlatformHost() { BeAssertFunctions::SetBeAssertHandler(&DgnPlatformHost::OnAssert); }
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
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
void JsInterop::InitLogging()
    {
    NativeLogging::LoggingConfig::ActivateProvider(new NativeLoggingShim());
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
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
void JsInterop::Initialize(BeFileNameCR addonDllDir, T_AssertHandler assertHandler)
    {
    s_addonDllDir = addonDllDir;
    s_assertHandler = assertHandler;

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() 
        {
        DgnPlatformLib::Initialize(*new DgnPlatformHost, true);
        InitLogging();
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
Utf8StringCR JsInterop::GetLastECDbIssue()
    {
    // It's up to the caller to serialize access to this.
    return s_lastECDbIssue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 02/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::CreateDgnDb(DgnDbPtr& db, BeFileNameCR pathname, Utf8StringCR rootSubjectName, Utf8StringCR rootSubjectDescription)
    {
    BeFileName path = pathname.GetDirectoryName();
    if (!path.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    DbResult result;
    CreateDgnDbParams createParams(rootSubjectName.c_str(), rootSubjectDescription.empty() ? nullptr : rootSubjectDescription.c_str());
    db = DgnDb::CreateDgnDb(&result, pathname, createParams);
    if (db.IsValid())
        db->AddIssueListener(s_listener);

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
DbResult JsInterop::OpenDgnDb(DgnDbPtr& db, BeFileNameCR fileOrPathname, DgnDb::OpenMode mode)
    {
    BeFileName pathname;
    if (!fileOrPathname.GetDirectoryName().empty())
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
        Utf8CP dbdirenv = nullptr;
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
        pathname.AppendToPath(fileOrPathname.c_str());
        }

    DbResult result;
    db = DgnDb::OpenDgnDb(&result, pathname, DgnDb::OpenParams(mode));
    if (db.IsValid())
        db->AddIssueListener(s_listener);

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::ReadChangeSets(bvector<DgnRevisionPtr>& revisionPtrs, bool& containsSchemaChanges, Utf8StringCR dbGuid, JsonValueCR changeSetTokens)
    {
    revisionPtrs.clear();
    containsSchemaChanges = false;
    PRECONDITION(!changeSetTokens.isNull() && changeSetTokens.isArray(), BE_SQLITE_ERROR);

    for (uint32_t ii = 0; ii < changeSetTokens.size(); ii++)
        {
        JsonValueCR changeSetToken = changeSetTokens[ii];
        PRECONDITION(changeSetToken.isMember("id") && changeSetToken.isMember("pathname"), BE_SQLITE_ERROR);

        if (!containsSchemaChanges)
            containsSchemaChanges = changeSetToken.isMember("containsSchemaChanges") && changeSetToken["containsSchemaChanges"].asBool();

        Utf8String id = changeSetToken["id"].asString();
        Utf8String parentId = changeSetToken["parentId"].asString();

        RevisionStatus revStatus;
        DgnRevisionPtr revision = DgnRevision::Create(&revStatus, id, parentId, dbGuid);
        if (!EXPECTED_CONDITION(revStatus == RevisionStatus::Success))
            return BE_SQLITE_ERROR;
        BeAssert(revision.IsValid());

        BeFileName changeSetPathname(changeSetToken["pathname"].asCString(), true);
        PRECONDITION(changeSetPathname.DoesPathExist(), BE_SQLITE_ERROR);

        revision->SetRevisionChangesFile(changeSetPathname);
        revisionPtrs.push_back(revision);
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::ProcessSchemaChangeSets(bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption, BeFileNameCR dbFileName)
    {
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(revisions, processOption));
    DbResult result;
    DgnDbPtr dgndb = DgnDb::OpenDgnDb(&result, dbFileName, openParams);
    POSTCONDITION(result == BE_SQLITE_OK, result);

    dgndb->CloseDb();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::ProcessDataChangeSets(DgnDbR dgndb, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption)
    {
    PRECONDITION(dgndb.IsDbOpen() && "Expected briefcase to be open when merging only data changes", BE_SQLITE_ERROR);

    RevisionStatus status = dgndb.Revisions().ProcessRevisions(revisions, processOption);
    POSTCONDITION(status == RevisionStatus::Success, BE_SQLITE_ERROR);

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::ProcessChangeSets(DgnDbPtr dgndb, JsonValueCR changeSetTokens, RevisionProcessOption processOption, Utf8StringCR dbGuid, BeFileNameCR dbFileName)
    {
    bvector<DgnRevisionPtr> revisionPtrs;
    bool containsSchemaChanges;
    DbResult result = ReadChangeSets(revisionPtrs, containsSchemaChanges, dbGuid, changeSetTokens);
    if (BE_SQLITE_OK != result)
        return result;

    bvector<DgnRevisionCP> revisions;
    for (uint32_t ii = 0; ii < revisionPtrs.size(); ii++)
        revisions.push_back(revisionPtrs[ii].get());

    return containsSchemaChanges ? ProcessSchemaChangeSets(revisions, processOption, dbFileName) : ProcessDataChangeSets(*dgndb, revisions, processOption);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::StartCreateChangeSet(JsonValueR changeSetInfo, DgnDbR dgndb)
    {
    RevisionManagerR revisions = dgndb.Revisions();

    if (revisions.IsCreatingRevision())
        revisions.AbandonCreateRevision();

    DgnRevisionPtr revision = revisions.StartCreateRevision(); // todo: better error reporting
    if (revision.IsNull())
        return BE_SQLITE_ERROR;

    changeSetInfo = Json::objectValue;
    changeSetInfo["id"] = revision->GetId().c_str();
    changeSetInfo["parentId"] = revision->GetParentId().c_str();
    changeSetInfo["pathname"] = Utf8String(revision->GetRevisionChangesFile()).c_str();
    changeSetInfo["containsSchemaChanges"] = revision->ContainsSchemaChanges(dgndb) ? 1 : 0;
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult JsInterop::FinishCreateChangeSet(DgnDbR dgndb)
    {
    RevisionStatus status = dgndb.Revisions().FinishCreateRevision();
    return (status == RevisionStatus::Success) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
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
// @bsimethod                               Ramanujam.Raman                 09/17
//---------------------------------------------------------------------------------------
DbResult JsInterop::SetupBriefcase(DgnDbPtr& outDb, JsonValueCR briefcaseToken)
    {
    PRECONDITION(!briefcaseToken.isNull() && briefcaseToken.isObject(), BE_SQLITE_ERROR);
    PRECONDITION(briefcaseToken.isMember("pathname") && briefcaseToken.isMember("briefcaseId") && briefcaseToken.isMember("openMode"), BE_SQLITE_ERROR);

    BeFileName briefcasePathname(briefcaseToken["pathname"].asCString(), true);
    int briefcaseId = briefcaseToken["briefcaseId"].asInt();
    //DgnDb::OpenMode mode = (DgnDb::OpenMode) briefcaseToken["openMode"].asInt();

    /** Open the first time to set the briefcase id and get the DbGuid (used for creating change sets) */
    DbResult result;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    DgnDbPtr db = DgnDb::OpenDgnDb(&result, briefcasePathname, openParams);
    if (!EXPECTED_CONDITION(result == BE_SQLITE_OK))
        return result;

    BeBriefcaseId newBriefcaseId((uint32_t)briefcaseId);
    if (db->GetBriefcaseId() != newBriefcaseId)
        {
        if (!EXPECTED_CONDITION(db->IsMasterCopy() && "Expected briefcase to be a master copy"))
            return BE_SQLITE_ERROR;
        result = db->SetAsBriefcase(newBriefcaseId);
        if (!EXPECTED_CONDITION(result == BE_SQLITE_OK && "Couldn't setup the briefcase id"))
            return BE_SQLITE_ERROR;
        }

    if (db.IsValid())
        db->AddIssueListener(s_listener);

    outDb = db;
    return result;
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
DbResult JsInterop::OpenECDb(ECDbR ecdb, BeFileNameCR pathname, BeSQLite::Db::OpenMode openMode)
    {
    if (!pathname.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    DbResult res = ecdb.OpenBeSQLiteDb(pathname, BeSQLite::Db::OpenParams(openMode));
    if (res != BE_SQLITE_OK)
        return res;

    ecdb.AddIssueListener(s_listener);
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

    ecdb.AddIssueListener(s_listener);
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

//************************************************************************************
// HexStrSqlFunction
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
HexStrSqlFunction* HexStrSqlFunction::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
HexStrSqlFunction& HexStrSqlFunction::GetSingleton()
    {
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

//************************************************************************************
// StrSqlFunction
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
StrSqlFunction* StrSqlFunction::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
StrSqlFunction& StrSqlFunction::GetSingleton()
    {
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