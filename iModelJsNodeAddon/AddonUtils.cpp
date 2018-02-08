/*--------------------------------------------------------------------------------------+
|
|     $Source: AddonUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AddonUtils.h"
#include <Bentley/Base64Utilities.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeomSerialization/GeomSerializationApi.h>

static Utf8String s_lastECDbIssue;
static BeFileName s_addonDllDir;
static AddonUtils::T_AssertHandler s_assertHandler;

BEGIN_UNNAMED_NAMESPACE

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
struct KnownAddonLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName m_tempDirectory;
    BeFileName m_assetsDirectory;

    BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tempDirectory;}
    BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_assetsDirectory;}

    //! Construct an instance of the KnownDesktopLocationsAdmin
    KnownAddonLocationsAdmin()
        {
        Desktop::FileSystem::BeGetTempPath(m_tempDirectory);
        m_assetsDirectory = s_addonDllDir;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
};

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  05/17
//=======================================================================================
struct AddonHost : DgnPlatformLib::Host
{
private:
    void _SupplyProductName(Utf8StringR name) override { name.assign("AddonUtils"); }
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownAddonLocationsAdmin(); }
    BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlang.AppendToPath(L"sqlang/iModelJsNodeAddon_en.sqlang.db3");
        return BeSQLite::L10N::SqlangFiles(sqlang);
        }

    RepositoryAdmin& _SupplyRepositoryAdmin() override {return AddonUtils::GetRepositoryAdmin();}

    static void OnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
        {
        if (s_assertHandler)
            s_assertHandler(msg, file, line, type);
        else
            printf("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
        }
public:
    AddonHost() { BeAssertFunctions::SetBeAssertHandler(&AddonHost::OnAssert); }
};

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
void AddonUtils::InitLogging()
    {
#if defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)
    Utf8CP configFileEnv = getenv("CONFIG_OPTION_CONFIG_FILE");
    if (!configFileEnv)
        {
        // fprintf(stderr, "CONFIG_OPTION_CONFIG_FILE envvar not defined. Activating default logging using console provider.\n");
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        NativeLogging::LoggingConfig::SetSeverity(L"ECDb", NativeLogging::LOG_ERROR);
        NativeLogging::LoggingConfig::SetSeverity(L"DgnCore", NativeLogging::LOG_ERROR);
        // NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
        // NativeLogging::LoggingConfig::SetSeverity(L"BeSQLite", NativeLogging::LOG_TRACE);
        return;
        }

    BeFileName configPathname(configFileEnv, true);
    if (!BeFileName::DoesPathExist(configPathname.c_str()))
        return;

    NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configPathname.GetName());
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
NativeLogging::ILogger& AddonUtils::GetLogger()
    {
    static NativeLogging::ILogger* s_logger;
    if (nullptr == s_logger)
        s_logger = NativeLogging::LoggingManager::GetLogger("imodeljs-addon"); // This is thread-safe. The assignment is atomic, and GetLogger will always return the same value for a given key anyway.
    return *s_logger;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
void AddonUtils::Initialize(BeFileNameCR addonDllDir, T_AssertHandler assertHandler)
    {
    s_addonDllDir = addonDllDir;
    s_assertHandler = assertHandler;

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() 
        {
        DgnPlatformLib::Initialize(*new AddonHost, true);
        InitLogging();
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
Utf8StringCR AddonUtils::GetLastECDbIssue()
    {
    // It's up to the caller to serialize access to this.
    return s_lastECDbIssue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 02/18
//---------------------------------------------------------------------------------------
DbResult AddonUtils::CreateDgnDb(DgnDbPtr& db, BeFileNameCR pathname, Utf8StringCR rootSubjectName, Utf8StringCR rootSubjectDescription)
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
DbResult AddonUtils::OpenDgnDb(DgnDbPtr& db, BeFileNameCR fileOrPathname, DgnDb::OpenMode mode)
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
DbResult AddonUtils::ReadChangeSets(bvector<DgnRevisionPtr>& revisionPtrs, bool& containsSchemaChanges, DgnDbR dgndb, JsonValueCR changeSetTokens)
    {
    revisionPtrs.clear();
    containsSchemaChanges = false;
    PRECONDITION(!changeSetTokens.isNull() && changeSetTokens.isArray(), BE_SQLITE_ERROR);

    Utf8String dbGuid = dgndb.GetDbGuid().ToString();
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
DbResult AddonUtils::ProcessSchemaChangeSets(DgnDbPtr& dgndb, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption)
    {
    PRECONDITION(!dgndb->IsDbOpen() && "Expected briefcase to be closed when merging schema changes", BE_SQLITE_ERROR);
        
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(revisions, processOption));
    DbResult result;
    dgndb = DgnDb::OpenDgnDb(&result, dgndb->GetFileName(), openParams);
    POSTCONDITION(result == BE_SQLITE_OK, result);

    dgndb->CloseDb();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult AddonUtils::ProcessDataChangeSets(DgnDbR dgndb, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption)
    {
    PRECONDITION(dgndb.IsDbOpen() && "Expected briefcase to be open when merging only data changes", BE_SQLITE_ERROR);

    RevisionStatus status = dgndb.Revisions().ProcessRevisions(revisions, processOption);
    POSTCONDITION(status == RevisionStatus::Success, BE_SQLITE_ERROR);

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult AddonUtils::ProcessChangeSets(DgnDbPtr& dgndb, JsonValueCR changeSetTokens, RevisionProcessOption processOption)
    {
    bvector<DgnRevisionPtr> revisionPtrs;
    bool containsSchemaChanges;
    DbResult result = ReadChangeSets(revisionPtrs, containsSchemaChanges, *dgndb, changeSetTokens);
    if (BE_SQLITE_OK != result)
        return result;

    bvector<DgnRevisionCP> revisions;
    for (uint32_t ii = 0; ii < revisionPtrs.size(); ii++)
        revisions.push_back(revisionPtrs[ii].get());

    return containsSchemaChanges ? ProcessSchemaChangeSets(dgndb, revisions, processOption) : ProcessDataChangeSets(*dgndb, revisions, processOption);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 01/18
//---------------------------------------------------------------------------------------
DbResult AddonUtils::StartCreateChangeSet(JsonValueR changeSetInfo, DgnDbR dgndb)
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
DbResult AddonUtils::FinishCreateChangeSet(DgnDbR dgndb)
    {
    RevisionStatus status = dgndb.Revisions().FinishCreateRevision();
    return (status == RevisionStatus::Success) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 09/17
//---------------------------------------------------------------------------------------
DbResult AddonUtils::SetupBriefcase(DgnDbPtr& outDb, JsonValueCR briefcaseToken)
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
void AddonUtils::GetRowAsJson(Json::Value& rowJson, ECSqlStatement& stmt) 
    {
    JsonECSqlSelectAdapter adapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsHexadecimalString));
    adapter.GetRow(rowJson, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
void AddonUtils::GetECValuesCollectionAsJson(Json::Value& json, ECN::ECValuesCollectionCR props) 
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
BentleyStatus AddonUtils::JsonBinder::BindPrimitiveValue(IECSqlBinder& binder, JsonValueCR bindingValue)
    {
    ECN::PrimitiveType primitiveType = (ECN::PrimitiveType) bindingValue["type"].asInt();
    JsonValueCR value = bindingValue["value"];
    BeAssert(!value.isNull());

    Json::ValueType valueType = value.type();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            {
            bvector<Byte> blob;
            if (SUCCESS != ECJsonUtilities::JsonToBinary(blob, value))
                return ERROR;
            if (ECSqlStatus::Success != binder.BindBlob(&blob, (int) blob.size(), IECSqlBinder::MakeCopy::Yes))
                return ERROR;
            return SUCCESS;
            }
        case PRIMITIVETYPE_Boolean:
            {
            if (!EXPECTED_CONDITION(valueType == Json::booleanValue))
                return ERROR;
            if (ECSqlStatus::Success != binder.BindBoolean(value.asBool()))
                return ERROR;
            return SUCCESS;
            }
        case PRIMITIVETYPE_DateTime:
            {
            if (!EXPECTED_CONDITION(valueType == Json::stringValue))
                return ERROR;
            DateTime dateTime;
            DateTime::FromString(dateTime, value.asString().c_str());
            if (ECSqlStatus::Success != binder.BindDateTime(dateTime))
                return ERROR;
            return SUCCESS;
            }
        case PRIMITIVETYPE_Double:
            {
            if (!value.isConvertibleTo(Json::ValueType::realValue) && !value.isString())
                return ERROR;
            double doubleValue;
            if (value.isDouble())
                doubleValue = value.asDouble();
            else if (value.isInt())
                doubleValue = (double) value.asInt();
            else if (value.isString())
                doubleValue = std::stod(value.asCString());
            else
                {
                EXPECTED_CONDITION(false);
                return ERROR;
                }
            if (ECSqlStatus::Success != binder.BindDouble(doubleValue))
                return ERROR;
            return SUCCESS;
            }
        case PRIMITIVETYPE_IGeometry:
            {
            if (!EXPECTED_CONDITION(valueType == Json::objectValue))
                return ERROR;
            if (value.isNull())
                return SUCCESS;
            bvector<IGeometryPtr> geometry;
            if (!BentleyGeometryJson::TryJsonValueToGeometry(value, geometry))
                return ERROR;
            BeAssert(geometry.size() == 1);
            if (ECSqlStatus::Success != binder.BindGeometry(*geometry[0]))
                return ERROR;
            return SUCCESS;
            }

        case PRIMITIVETYPE_Integer:
            {
            if (!EXPECTED_CONDITION(valueType == Json::intValue || valueType == Json::stringValue))
                return ERROR;
            int intValue;
            if (value.isInt())
                intValue = value.asInt();
            else if (value.isString())
                intValue = std::stoi(value.asCString());
            else
                {
                EXPECTED_CONDITION(false);
                return ERROR;
                }
            if (ECSqlStatus::Success != binder.BindInt(intValue))
                return ERROR;
            return SUCCESS;
            }
        case PRIMITIVETYPE_Long:
            {
            if (!EXPECTED_CONDITION(valueType == Json::stringValue  && "int64_t values need to be serialized as strings to allow use in Javascript"))
                return ERROR;
            BentleyStatus status;
            uint64_t longValue = BeStringUtilities::ParseHex(value.asCString(), &status);
            if (!EXPECTED_CONDITION(status == SUCCESS))
                return ERROR;
            if (ECSqlStatus::Success != binder.BindInt64((int64_t) longValue))
                return ERROR;
            return SUCCESS;
            }
        case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            if (ECJsonUtilities::JsonToPoint2d(point2d, value))
                return ERROR;
            if (ECSqlStatus::Success != binder.BindPoint2d(point2d))
                return ERROR;
            return SUCCESS;
            }
        case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            if (ECJsonUtilities::JsonToPoint3d(point3d, value))
                return ERROR;
            if (ECSqlStatus::Success != binder.BindPoint3d(point3d))
                return ERROR;
            return SUCCESS;
            }
        case PRIMITIVETYPE_String:
            {
            if (!EXPECTED_CONDITION(valueType == Json::stringValue))
                return ERROR;
            if (ECSqlStatus::Success != binder.BindText(value.asString().c_str(), IECSqlBinder::MakeCopy::Yes))
                return ERROR;
            return SUCCESS;
            }
        default:
            return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
BentleyStatus AddonUtils::JsonBinder::BindArrayValue(IECSqlBinder& binder, JsonValueCR value)
    {
    BeAssert(value.isArray() && !value.isNull());
    for (int ii = 0; ii < (int) value.size(); ii++)
        {
        IECSqlBinder& childBinder = binder.AddArrayElement();
        if (SUCCESS != BindValue(childBinder, value[ii]))
            return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
BentleyStatus AddonUtils::JsonBinder::BindStructValue(IECSqlBinder& binder, JsonValueCR value)
    {
    BeAssert(value.isObject() && !value.isNull());
    for (Json::Value::iterator iter = value.begin(); iter != value.end(); iter++)
        {
        Utf8CP paramName = iter.memberName();
        JsonValueCR childValue = *iter;

        IECSqlBinder& childBinder = binder[paramName];
        if (SUCCESS != BindValue(childBinder, childValue))
            return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
BentleyStatus AddonUtils::JsonBinder::BindValue(IECSqlBinder& binder, JsonValueCR bindingValue)
    {
    PRECONDITION(!bindingValue.isNull() && bindingValue.isMember("kind") && bindingValue.isMember("type") && bindingValue.isMember("value"), ERROR);
    if (bindingValue["value"].isNull())
        {
        binder.BindNull();
        return SUCCESS;
        }

    BentleyStatus status;
    ECN::ValueKind valueKind = (ECN::ValueKind) bindingValue["kind"].asInt();
    if (valueKind == VALUEKIND_Array)
        status = BindArrayValue(binder, bindingValue);
    else if (valueKind == VALUEKIND_Struct)
        status = BindStructValue(binder, bindingValue);
    else if (valueKind == VALUEKIND_Primitive)
        status = BindPrimitiveValue(binder, bindingValue);
    else
        {
        EXPECTED_CONDITION(false);
        status = ERROR;
        }
            
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
BentleyStatus AddonUtils::JsonBinder::BindValues(ECSqlStatement& stmt, JsonValueCR bindings)
    {
    PRECONDITION(!bindings.isNull(), ERROR);
    PRECONDITION(bindings.isObject() || bindings.isArray(), ERROR);

    if (bindings.isArray())
        {
        for (int ii = 0; ii < (int) bindings.size(); ii++)
            {
            int paramIndex = ii + 1;
            if (SUCCESS != BindValue(stmt.GetBinder(paramIndex), bindings[ii]))
                return ERROR;
            }
        return SUCCESS;
        }

    BeAssert(bindings.isObject());
    for (Json::Value::iterator iter = bindings.begin(); iter != bindings.end(); iter++)
        {
        Utf8CP paramName = iter.memberName();
        int paramIndex = stmt.GetParameterIndex(paramName);
        if (paramIndex <= 0)
            return ERROR;

        if (SUCCESS != BindValue(stmt.GetBinder(paramIndex), *iter))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult AddonUtils::OpenECDb(ECDbR ecdb, BeFileNameCR pathname, BeSQLite::Db::OpenMode openMode)
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
DbResult AddonUtils::CreateECDb(ECDbR ecdb, BeFileNameCR pathname)
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
DbResult AddonUtils::ImportSchema(ECDbR ecdb, BeFileNameCR pathname)
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
DbResult AddonUtils::ImportSchemaDgnDb(DgnDbR dgndb, BeFileNameCR pathname)
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
ECClassCP AddonUtils::GetClassFromInstance(ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    return ECJsonUtilities::GetClassFromClassNameJson(jsonInstance[ECJsonUtilities::json_className()], ecdb.GetClassLocater());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
ECInstanceId AddonUtils::GetInstanceIdFromInstance(ECDbCR ecdb, JsonValueCR jsonInstance)
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
        ctx.SetResultError("Argument of function HexStr is expected to be a number.");
        return;
        }

    static const size_t stringBufferLength = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

    Utf8Char stringBuffer[stringBufferLength]; //+1 for the trailing 0 character;
    BeStringUtilities::FormatUInt64(stringBuffer, stringBufferLength, numValue.GetValueUInt64(), (HexFormatOptions) ((int) HexFormatOptions::IncludePrefix));

    ctx.SetResultText(stringBuffer, strlen(stringBuffer), Context::CopyData::Yes);
    }