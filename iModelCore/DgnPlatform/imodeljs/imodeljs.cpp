/*--------------------------------------------------------------------------------------+
|
|     $Source: imodeljs/imodeljs.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "imodeljs.h"
#include <Bentley/Base64Utilities.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeomSerialization/GeomSerializationApi.h>

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
JsECDb::JsECDb() : m_ecsqlCache(50, "ECDb")
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
void JsECDb::_OnDbClose()
    {
    m_ecsqlCache.Empty();
    T_Super::_OnDbClose();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr JsECDb::GetPreparedECSqlStatement(Utf8CP ecsql) const
    {
    return m_ecsqlCache.GetPreparedStatement(*this, ecsql, nullptr);
    }

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

static bmap<BeFileName, DgnDbPtr> s_dbs;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
DgnDbPtr IModelJs::GetDbByName(DbResult& dbres, BeFileNameCR fn, DgnDb::OpenMode mode)
    {
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
                return nullptr;
                }
            }
        dbfilename = dbDir;
        dbfilename.AppendToPath(fn.c_str());
        }

    auto db = DgnDb::OpenDgnDb(&dbres, dbfilename, DgnDb::OpenParams(mode));
    if (!db.IsValid())
        return nullptr;

    db->AddIssueListener(s_listener); 
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
            continue; // if the value is null, just skip it

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
            case ECN::PRIMITIVETYPE_Boolean:
                rowJson[name] = value.GetBoolean();
                break;
            case ECN::PRIMITIVETYPE_Long:
                rowJson[name] = BeInt64Id(value.GetInt64()).ToHexStr();
                break;
            case ECN::PRIMITIVETYPE_Integer:
                rowJson[name] = value.GetInt();
                break;
            case ECN::PRIMITIVETYPE_Double:
                rowJson[name] = value.GetDouble();
                break;
            case ECN::PRIMITIVETYPE_String: 
                rowJson[name] = value.GetText();
                break;
            case ECN::PRIMITIVETYPE_Binary:
                {
                int length;
                void const* blob = value.GetBlob(&length);
                ECJsonUtilities::BinaryToJson(rowJson[name], (Byte const*) blob, length);
                }
                break;
            case ECN::PRIMITIVETYPE_Point2d: 
                JsonUtils::DPoint2dToJson(rowJson[name], value.GetPoint2d());
                break;
            case ECN::PRIMITIVETYPE_Point3d:
                JsonUtils::DPoint3dToJson(rowJson[name], value.GetPoint3d());
                break;
            case ECN::PRIMITIVETYPE_DateTime:
                rowJson[name] = value.GetDateTime().ToString();
                break;

            default: 
                {
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

//========================================================================================
// @bsiclass                                                 Ramanujam.Raman      08/2017
//========================================================================================
struct JsonBinder
{
private:    
   static BentleyStatus BindPrimitiveValue(IECSqlBinder& binder, JsonValueCR bindingValue)
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

    static BentleyStatus BindArrayValue(IECSqlBinder& binder, JsonValueCR value)
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

    static BentleyStatus BindStructValue(IECSqlBinder& binder, JsonValueCR value)
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

    static BentleyStatus BindValue(IECSqlBinder& binder, JsonValueCR bindingValue)
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

public:
    static BentleyStatus BindValues(ECSqlStatement& stmt, JsonValueCR bindings)
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

        // if (bindings.isObject())
        for (Json::Value::iterator iter = bindings.begin(); iter != bindings.end(); iter++)
            {
            Utf8CP paramName = iter.memberName();
            int paramIndex = stmt.GetParameterIndex(paramName);
            if (!EXPECTED_CONDITION(paramIndex > 0))
                return ERROR;

            if (SUCCESS != BindValue(stmt.GetBinder(paramIndex), *iter))
                return ERROR;
            }

        return SUCCESS;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
BeSQLite::DbResult IModelJs::ExecuteQuery(JsonValueR results, ECSqlStatement& stmt, JsonValueCR bindings)
    {
    if (!bindings.isNull() && SUCCESS != JsonBinder::BindValues(stmt, bindings))
        return BE_SQLITE_ERROR;

    DbResult result;
    results = Json::arrayValue;

    while (BE_SQLITE_ROW == (result = stmt.Step()))
        {
        Json::Value row(Json::objectValue);
        IModelJs::GetRowAsJson(row, stmt);
        results.append(row);
        }

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
BeSQLite::DbResult IModelJs::ExecuteStatement(Utf8StringR instanceId, ECSqlStatement& stmt, bool isInsertStmt, JsonValueCR bindings)
    {
    if (SUCCESS != JsonBinder::BindValues(stmt, bindings))
        return BE_SQLITE_ERROR;

    ECInstanceKey instanceKey;
    DbResult result = isInsertStmt ? stmt.Step(instanceKey) : stmt.Step();
    if (BE_SQLITE_DONE != result)
        return result;

    if (isInsertStmt)
        instanceId = instanceKey.GetInstanceId().ToString(ECInstanceId::UseHex::Yes);

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
JsECDbPtr IModelJs::OpenECDb(DbResult &dbres, BeFileNameCR pathname, BeSQLite::Db::OpenMode openMode)
    {
    BeSystemMutexHolder threadSafeInScope;

    if (!pathname.DoesPathExist())
        {
        dbres = DbResult::BE_SQLITE_NOTFOUND;
        return nullptr;
        }

    JsECDbPtr ecdb = new JsECDb();
    dbres = ecdb->OpenBeSQLiteDb(pathname, BeSQLite::Db::OpenParams(openMode));
    if (dbres != BE_SQLITE_OK)
        return nullptr;

    ecdb->AddIssueListener(s_listener);
    return ecdb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
JsECDbPtr IModelJs::CreateECDb(DbResult &dbres, BeFileNameCR pathname)
    {
    BeSystemMutexHolder threadSafeInScope;
    
    BeFileName path = pathname.GetDirectoryName();
    if (!path.DoesPathExist())
        {
        dbres = DbResult::BE_SQLITE_NOTFOUND;
        return nullptr;
        }

    JsECDbPtr ecdb = new JsECDb();

    DbResult result = ecdb->CreateNewDb(pathname);
    if (result != BE_SQLITE_OK)
        return nullptr;

    ecdb->AddIssueListener(s_listener);
    return ecdb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult IModelJs::ImportSchema(ECDbR ecdb, BeFileNameCR pathname)
    {
    BeSystemMutexHolder threadSafeInScope;

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
ECClassCP IModelJs::GetClassFromInstance(ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    Utf8String classKey = jsonInstance["$ECClassKey"].asString();
    if (classKey.empty())
        return nullptr;

    Utf8String::size_type dotIndex = classKey.find('.');
    if (Utf8String::npos == dotIndex || classKey.length() == dotIndex + 1)
        return nullptr;

    Utf8String schemaName = classKey.substr(0, dotIndex);
    Utf8String className = classKey.substr(dotIndex + 1);

    return ecdb.Schemas().GetClass(schemaName, className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
ECInstanceId IModelJs::GetInstanceIdFromInstance(ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    if (!jsonInstance.isMember("$ECInstanceId"))
        return ECInstanceId();

    ECInstanceId instanceId;
    if (SUCCESS != ECInstanceId::FromString(instanceId, jsonInstance["$ECInstanceId"].asCString()))
        return ECInstanceId();

    return instanceId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult IModelJs::InsertInstance(Utf8StringR insertedId, ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    ECClassCP ecClass = GetClassFromInstance(ecdb, jsonInstance);
    if (!ecClass)
        return BE_SQLITE_ERROR;

    JsonInserter inserter(ecdb, *ecClass, nullptr);
    ECInstanceKey instanceKey;
    DbResult result = inserter.Insert(instanceKey, jsonInstance);
    if (result != BE_SQLITE_OK)
        return result;

    insertedId = BeInt64Id(instanceKey.GetInstanceId().GetValueUnchecked()).ToHexStr();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult IModelJs::UpdateInstance(ECDbCR ecdb, JsonValueCR jsonInstance)
    {
    ECClassCP ecClass = GetClassFromInstance(ecdb, jsonInstance);
    if (!ecClass)
        return BE_SQLITE_ERROR;

    ECInstanceId instanceId = GetInstanceIdFromInstance(ecdb, jsonInstance);
    if (!instanceId.IsValid())
        return BE_SQLITE_ERROR;

    JsonUpdater updater(ecdb, *ecClass, nullptr);
    return updater.Update(instanceId, jsonInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult IModelJs::ReadInstance(JsonValueR jsonInstance, ECDbCR ecdb, JsonValueCR instanceKey)
    {
    ECClassCP ecClass = GetClassFromInstance(ecdb, instanceKey);
    if (!ecClass)
        return BE_SQLITE_ERROR;

    ECInstanceId instanceId = GetInstanceIdFromInstance(ecdb, instanceKey);
    if (!instanceId.IsValid())
        return BE_SQLITE_ERROR;

    JsonReader reader(ecdb, ecClass->GetId());
    BentleyStatus status = reader.ReadInstance(jsonInstance, instanceId, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    if (status != BentleyStatus::SUCCESS)
        return BE_SQLITE_ERROR;

    jsonInstance["$ECInstanceId"] = BeInt64Id((int64_t) instanceId.GetValueUnchecked()).ToHexStr();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult IModelJs::DeleteInstance(ECDbCR ecdb, JsonValueCR instanceKey)
    {
    ECClassCP ecClass = GetClassFromInstance(ecdb, instanceKey);
    if (!ecClass)
        return BE_SQLITE_ERROR;

    ECInstanceId instanceId = GetInstanceIdFromInstance(ecdb, instanceKey);
    if (!instanceId.IsValid())
        return BE_SQLITE_ERROR;

    JsonDeleter deleter(ecdb, *ecClass, nullptr);
    return  deleter.Delete(instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
DbResult IModelJs::ContainsInstance(bool& containsInstance, JsECDbR ecdb, JsonValueCR instanceKey)
    {
    BeSqliteDbMutexHolder serializeAccess(ecdb); // hold mutex, so that I have a chance to get last ECDb error message

    if (!instanceKey.isMember("$ECClassKey"))
        return BE_SQLITE_ERROR;

    Utf8String classKey = instanceKey["$ECClassKey"].asString();
    if (classKey.empty())
        return BE_SQLITE_ERROR;

    ECInstanceId instanceId = GetInstanceIdFromInstance(ecdb, instanceKey);
    if (!instanceId.IsValid())
        return BE_SQLITE_ERROR;

    Utf8PrintfString ecsql("SELECT NULL FROM %s WHERE ECInstanceId=?", classKey.c_str());
    CachedECSqlStatementPtr stmt = ecdb.GetPreparedECSqlStatement(ecsql.c_str());
    if (!stmt.IsValid())
        return BE_SQLITE_ERROR;

    stmt->BindId(1, instanceId);

    DbResult result = stmt->Step();

    if (result != BE_SQLITE_ROW && result != BE_SQLITE_DONE)
        return result;

    containsInstance = (result == BE_SQLITE_ROW);
    return BE_SQLITE_OK;
    }
