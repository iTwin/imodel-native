/*--------------------------------------------------------------------------------------+
|
|     $Source: imodeljs/imodeljs.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "imodeljs.h"
#include <Bentley/Desktop/FileSystem.h>

static Utf8String s_lastEcdbIssue;
static BeFileName s_addonDllDir;

BEGIN_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
    {
    void _OnIssueReported(BentleyApi::Utf8CP message) const override {s_lastEcdbIssue = message;}
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
    void _SupplyProductName(Utf8StringR name) override { name.assign("IModelJs"); }
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownAddonLocationsAdmin(); }
    BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlang.AppendToPath(L"sqlang/DgnPlatform_en.sqlang.db3");
        return BeSQLite::L10N::SqlangFiles(sqlang);
        }

    static void OnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
        {
        printf("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
        //DebugBreak();
        }
public:
    AddonHost() { BeAssertFunctions::SetBeAssertHandler(&AddonHost::OnAssert); }
};

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
void IModelJs::InitLogging()
    {
#if defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)
    Utf8CP configFileEnv = getenv("CONFIG_OPTION_CONFIG_FILE");
    if (!configFileEnv)
        return;
         
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
void IModelJs::Initialize(BeFileNameCR addonDllDir)
    {
    s_addonDllDir = addonDllDir;

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
Utf8String IModelJs::GetLastEcdbIssue() 
    {
    // It's up to the caller to serialize access to this.
    return s_lastEcdbIssue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
DgnDbPtr IModelJs::GetDbByName(DbResult& dbres, Utf8String& errmsg, BeFileNameCR fn, DgnDb::OpenMode mode)
    {
    static bmap<BeFileName, DgnDbPtr>* s_dbs;
    
    // *** 
    // ***  TBD: sort out readonly vs. readwrite 
    // ***

    BeSystemMutexHolder threadSafeInScope;

    if (nullptr == s_dbs)
        s_dbs = new bmap<BeFileName, DgnDbPtr>();

    auto found = s_dbs->find(fn);
    if (found != s_dbs->end())
        return found->second;

    // *** TBD: keep some kind of last-used-time for each db
    // *** TBD: if we have too many Dbs open, then close some that have not been accessed for a while.

    BeFileName dbfilename;
    if (!fn.GetDirectoryName().empty())
        {
        dbfilename = fn;
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
                {
                dbres = DbResult::BE_SQLITE_NOTFOUND;
                errmsg = Utf8String(dbDir);
                errmsg.append(" - directory not found. Where are the .bim files? You can put them in a subdirectory called 'briefcases', or you can set NODE_DGNDB_DIR in your shell to point to a directory that contains them.");
                return nullptr;
                }
            }
        dbfilename = dbDir;
        dbfilename.AppendToPath(fn.c_str());
        }

    auto db = DgnDb::OpenDgnDb(&dbres, dbfilename, DgnDb::OpenParams(mode));
    if (!db.IsValid())
        {
        errmsg = DgnDb::InterpretDbResult(dbres);
        return nullptr;
        }

    db->AddIssueListener(s_listener); 

    s_dbs->insert(make_bpair(fn, db));
    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
Json::Value IModelJs::GetRowAsRawJson(ECSqlStatement& stmt)
    {
    Json::Value rowAsJson;
    JsonECSqlSelectAdapter adapter(stmt);
    adapter.GetRowAsIs(rowAsJson);
    return rowAsJson;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
void IModelJs::GetRowAsJson(Json::Value& rowJson, ECSqlStatement& stmt) 
    {
    int cols = stmt.GetColumnCount();

    for (int i = 0; i < cols; ++i)
        {
        IECSqlValue const& value = stmt.GetValue(i);
        ECSqlColumnInfoCR info = value.GetColumnInfo();
        BeAssert(info.IsValid());
    
        Utf8String name = info.GetProperty()->GetName();
        name[0] = tolower(name[0]);
        
        if (rowJson.isMember(name)) // we already added this member, skip it.
            continue;

        if (value.IsNull())
            {
            rowJson[name] = Json::Value();
            continue;
            }

        ECN::ECTypeDescriptor typedesc = info.GetDataType();
        if (typedesc.IsNavigation())
            {
            ECN::ECClassId relClassId;
            auto eid = value.GetNavigation<DgnElementId>(&relClassId);
            ECN::ECValue value(eid, relClassId);
            JsonUtils::NavigationPropertyToJson(rowJson[name], value.GetNavigationInfo());
            continue;
            }

        if (!typedesc.IsPrimitive())
            {
            rowJson[name] = GetRowAsRawJson(stmt)[i];     // *** WIP_NODE_ADDON
            continue;
            }

        switch (typedesc.GetPrimitiveType())
            {
            case ECN::PRIMITIVETYPE_Long:
                rowJson[name] = value.GetUInt64();
                break;
            case ECN::PRIMITIVETYPE_Integer:
                rowJson[name] = Json::Value(value.GetInt());
                break;
            case ECN::PRIMITIVETYPE_Double:
                rowJson[name] = Json::Value(value.GetDouble());
                break;
            case ECN::PRIMITIVETYPE_String: 
                rowJson[name] = Json::Value(value.GetText());
                break;
            case ECN::PRIMITIVETYPE_Binary:
            #ifdef NEEDS_WORK
                {
                int length;
                const void* blob = value.GetBlob(&length);
                row->push_back(new Values::Blob(name, length, blob));
                break;
                }
            #else
                rowJson[name] = Json::Value();
                break;
            #endif
            case ECN::PRIMITIVETYPE_Point2d: 
                JsonUtils::DPoint2dToJson(rowJson[name], value.GetPoint2d());                 // *** WIP_NODE_ADDON
                break;
            case ECN::PRIMITIVETYPE_Point3d:
                JsonUtils::DPoint3dToJson(rowJson[name], value.GetPoint3d());                 // *** WIP_NODE_ADDON
                break;
            case ECN::PRIMITIVETYPE_DateTime:
                rowJson[name] = Json::Value(value.GetDateTime().ToString().c_str());          // *** WIP_NODE_ADDON
                break;
            default: 
                {
                BeAssert(false && "TBD");
                rowJson[name] = GetRowAsRawJson(stmt)[i];     // *** WIP_NODE_ADDON
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
void IModelJs::GetECValuesCollectionAsJson(Json::Value& json, ECN::ECValuesCollectionCR props) 
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
JsECDbPtr IModelJs::OpenECDb(DbResult &dbres, Utf8StringR errmsg, BeFileNameCR pathname, BeSQLite::Db::OpenMode openMode)
    {
    BeSystemMutexHolder threadSafeInScope;

    if (!pathname.DoesPathExist())
        {
        dbres = DbResult::BE_SQLITE_NOTFOUND;
        errmsg = Utf8String(pathname);
        errmsg.append(" - not found.");
        return nullptr;
        }


    JsECDbPtr ecdb = new JsECDb();
    DbResult result = ecdb->OpenBeSQLiteDb(pathname, BeSQLite::Db::OpenParams(openMode));
    if (result != BE_SQLITE_OK)
        {
        errmsg = JsECDb::InterpretDbResult(dbres);
        return nullptr;
        }

    ecdb->AddIssueListener(s_listener);
    return ecdb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
JsECDbPtr IModelJs::CreateECDb(DbResult &dbres, Utf8String &errmsg, BeFileNameCR pathname)
    {
    BeSystemMutexHolder threadSafeInScope;
    
    BeFileName path = pathname.GetDirectoryName();
    if (!path.DoesPathExist())
        {
        dbres = DbResult::BE_SQLITE_NOTFOUND;
        errmsg = Utf8String(path);
        errmsg.append(" - path not found. Specify a location that exists");
        return nullptr;
        }

    JsECDbPtr ecdb = new JsECDb();

    DbResult result = ecdb->CreateNewDb(pathname);
    if (result != BE_SQLITE_OK)
        {
        errmsg = JsECDb::InterpretDbResult(dbres);
        return nullptr;
        }

    ecdb->AddIssueListener(s_listener);
    return ecdb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult IModelJs::ImportSchema(Utf8StringR errmsg, ECDbR ecdb, BeFileNameCR pathname)
    {
    BeSystemMutexHolder threadSafeInScope;

    if (!pathname.DoesPathExist())
        {
        errmsg = Utf8String(pathname);
        errmsg.append(" - not found.");
        return BE_SQLITE_NOTFOUND;
        }

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);
    schemaContext->SetFinalSchemaLocater(ecdb.GetSchemaLocater());

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile(schema, pathname.GetName(), *schemaContext);
    if (SchemaReadStatus::Success != schemaStatus)
        {
        errmsg = Utf8String(pathname);
        errmsg.append(" - could not be read.");
        return BE_SQLITE_ERROR;
        }

    bvector<ECSchemaCP> schemas;
    schemas.push_back(schema.get());
    BentleyStatus status = ecdb.Schemas().ImportSchemas(schemas);
    if (status != SUCCESS)
        {
        errmsg = Utf8String(pathname);
        errmsg.append(" - could not be imported");
        return BE_SQLITE_ERROR;
        }

    DbResult result = ecdb.SaveChanges();
    if (result != BE_SQLITE_OK)
        {
        errmsg = "Could not save ECDb after importing schema";
        return result;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
// static
ECClassCP IModelJs::GetClassFromInstance(Utf8StringR errmsg, ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    if (!jsonInstance.isMember("$ECClassKey"))
        {
        errmsg = "Could not determine the class. The JSON instance must include a valid $ECClassKey member";
        return nullptr;
        }
        
    Utf8String classKey = jsonInstance["$ECClassKey"].asString();
    if (classKey.empty())
        {
        errmsg = "Could not determine the class. The JSON instance must include a valid $ECClassKey member";
        return nullptr;
        }

    Utf8String::size_type dotIndex = classKey.find('.');
    if (Utf8String::npos == dotIndex || classKey.length() == dotIndex + 1)
        {
        errmsg = "Could not determine the class. The JSON instance contains an invalid $ECClassKey: ";
        errmsg.append(classKey.c_str());
        return nullptr;
        }

    Utf8String schemaName = classKey.substr(0, dotIndex);
    Utf8String className = classKey.substr(dotIndex + 1);

    ECClassCP ecClass = ecdb.Schemas().GetClass(schemaName, className);
    if (!ecClass)
        {
        errmsg = "Could not find a class with the key: ";
        errmsg.append(classKey.c_str());
        return nullptr;
        }

    return ecClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
// static
ECInstanceId IModelJs::GetInstanceIdFromInstance(Utf8StringR errmsg, ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    if (!jsonInstance.isMember("$ECInstanceId"))
        {
        errmsg = "Could not determine the instance id. The JSON instance does must include $ECInstanceId";
        return ECInstanceId();
        }

    ECInstanceId instanceId = ECInstanceId(BeStringUtilities::ParseUInt64(jsonInstance["$ECInstanceId"].asCString()));
    if (!instanceId.IsValid())
        {
        errmsg = "Could not parse the instance id from $ECInstanceId: ";
        errmsg.append(jsonInstance["$ECInstanceId"].asCString());
        return ECInstanceId();
        }

    return instanceId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
// static
DbResult IModelJs::InsertInstance(Utf8StringR errmsg, ECInstanceId& insertedId, ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    ECClassCP ecClass = GetClassFromInstance(errmsg, ecdb, jsonInstance);
    if (!ecClass)
        return BE_SQLITE_ERROR;

    JsonInserter inserter(ecdb, *ecClass, nullptr);
    ECInstanceKey instanceKey;
    DbResult result = inserter.Insert(instanceKey, jsonInstance);
    if (result != BE_SQLITE_OK)
        {
        errmsg.Sprintf("Could not insert instance with key %s", jsonInstance["$ECClassKey"].asCString());
        return result;
        }
    insertedId = instanceKey.GetInstanceId();

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
// static
DbResult IModelJs::UpdateInstance(Utf8StringR errmsg, ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    ECClassCP ecClass = GetClassFromInstance(errmsg, ecdb, jsonInstance);
    if (!ecClass)
        return BE_SQLITE_ERROR;

    ECInstanceId instanceId = GetInstanceIdFromInstance(errmsg, ecdb, jsonInstance);
    if (!instanceId.IsValid())
        return BE_SQLITE_ERROR;

    JsonUpdater updater(ecdb, *ecClass, nullptr);
    DbResult result = updater.Update(instanceId, jsonInstance);
    if (result != BE_SQLITE_OK)
        {
        errmsg.Sprintf("Could not update instance with key %s and id %s", jsonInstance["$ECClassKey"].asCString(), jsonInstance["$ECInstanceId"].asCString());
        return result;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
// static
DbResult IModelJs::ReadInstance(Utf8StringR errmsg, JsonValueR jsonInstance, ECDbCR ecdb, JsonValueCR instanceKey)
    {
    ECClassCP ecClass = GetClassFromInstance(errmsg, ecdb, instanceKey);
    if (!ecClass)
        return BE_SQLITE_ERROR;

    ECInstanceId instanceId = GetInstanceIdFromInstance(errmsg, ecdb, instanceKey);
    if (!instanceId.IsValid())
        return BE_SQLITE_ERROR;

    JsonReader reader(ecdb, ecClass->GetId());
    BentleyStatus status = reader.ReadInstance(jsonInstance, instanceId, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    if (status != BentleyStatus::SUCCESS)
        {
        errmsg.Sprintf("Could not read instance with key %s and id %s", instanceKey["$ECClassKey"].asCString(), instanceKey["$ECInstanceId"].asCString());
        return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
// static
DbResult IModelJs::DeleteInstance(Utf8StringR errmsg, ECDbCR ecdb, JsonValueCR instanceKey)
    {
    ECClassCP ecClass = GetClassFromInstance(errmsg, ecdb, instanceKey);
    if (!ecClass)
        return BE_SQLITE_ERROR;

    ECInstanceId instanceId = GetInstanceIdFromInstance(errmsg, ecdb, instanceKey);
    if (!instanceId.IsValid())
        return BE_SQLITE_ERROR;

    JsonDeleter deleter(ecdb, *ecClass, nullptr);
    DbResult result = deleter.Delete(instanceId);
    if (result != BE_SQLITE_OK)
        {
        errmsg.Sprintf("Could not delete instance with key %s and id %s", instanceKey["$ECClassKey"].asCString(), instanceKey["$ECInstanceId"].asCString());
        return result;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
// static
DbResult IModelJs::ContainsInstance(Utf8StringR errmsg, bool& containsInstance, ECDbCR ecdb, JsonValueCR instanceKey)
    {
    if (!instanceKey.isMember("$ECClassKey"))
        {
        errmsg = "Could not determine the class. The JSON instance must include a valid $ECClassKey member";
        return BE_SQLITE_ERROR;
        }

    Utf8String classKey = instanceKey["$ECClassKey"].asString();
    if (classKey.empty())
        {
        errmsg = "Could not determine the class. The JSON instance must include a valid $ECClassKey member";
        return BE_SQLITE_ERROR;
        }

    ECInstanceId instanceId = GetInstanceIdFromInstance(errmsg, ecdb, instanceKey);
    if (!instanceId.IsValid())
        return BE_SQLITE_ERROR;

    Utf8PrintfString ecsql("SELECT NULL FROM %s WHERE ECInstanceId=?", classKey.c_str());
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql.c_str()))
        {
        errmsg = IModelJs::GetLastEcdbIssue();
        return BE_SQLITE_ERROR;
        }

    stmt.BindId(1, instanceId);

    DbResult result = stmt.Step();

    if (result != BE_SQLITE_ROW && result != BE_SQLITE_DONE)
        {
        errmsg = IModelJs::GetLastEcdbIssue();
        return result;
        }

    containsInstance = (result == BE_SQLITE_ROW);
    return BE_SQLITE_OK;
    }
