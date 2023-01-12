/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <imodeljs-nodeaddonapi.package.version.h>
#include "IModelJsNative.h"
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECName.h>
#include "ECSchemaXmlContextUtils.h"
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/PerformanceLogger.h>
#include <DgnPlatform/ElementGeometryCache.h>
#include <DgnPlatform/FenceContext.h>
#include <DgnPlatform/MeasureGeom.h>
#include <DgnPlatform/ChangedElementsManager.h>
#include "SignalTestUtility.h"
#include "presentation/ECPresentationUtils.h"
#include "presentation/UpdateRecordsHandler.h"
#include "presentation/UiStateProvider.h"
#include <BeSQLite/Profiler.h>
#include <folly/BeFolly.h>
#include "SchemaUtil.h"
#include "JsLogger.h"

// cspell:ignore napi strbuf propsize

namespace IModelJsNative {

static bool s_assertionsEnabled = true;
static BeMutex s_assertionMutex;
static Utf8String s_mobileResourcesDir;
static Utf8String s_mobileTempDir;
static JsLogger s_jsLogger;

// DON'T change this flag directly. It is managed by BeObjectWrap only.
bool s_BeObjectWrap_inDestructor = false;

template <typename STATUSTYPE>
static void SaveErrorValue(BeJsValue error, STATUSTYPE errCode, Utf8CP msg) {
    error["status"] = (int) errCode;
    if (nullptr != msg)
        error["message"] = msg;
}

Napi::Object CreateBentleyReturnSuccessObject(Napi::Value goodVal) {
    auto retObj = Napi::Object::New(goodVal.Env());
    retObj["result"] = goodVal;
    return retObj;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsInterop::IsJsExecutionDisabled()  {
    return s_BeObjectWrap_inDestructor || !IsMainThread() || Env().IsExceptionPending();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::InitLogging()  {
    NativeLogging::Logging::SetLogger(&s_jsLogger);
}

//=======================================================================================
// api for embedding files in a SQLite file. This class is necessary because the typescript class DgnDb is not a subclass of SQLiteDb,
// unfortunately, and this applies to both.
// @bsiclass
//=======================================================================================
struct FileProps {
    Utf8String m_name;
    Utf8String m_fileExt;
    Utf8String m_localFileName;
    bool m_compress = true;
    DateTime m_date;
    FileProps(Utf8StringCR name, Utf8StringCR fileExt,Utf8StringCR localFileName): m_name(name), m_fileExt(fileExt), m_localFileName(localFileName) {}
};

struct SQLiteOps {
    virtual Db* _GetMyDb() = 0;

    static FileProps getEmbedFileProps(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, optObj);
        BeJsValue opts(optObj);
        if (!opts.isStringMember(JsInterop::json_name()))
            BeNapi::ThrowJsException(info.Env(), "name argument missing");
        if (!opts.isStringMember(JsInterop::json_localFileName()))
            BeNapi::ThrowJsException(info.Env(), "localFileName argument missing");

        Utf8String fileExt;
        if (opts.isStringMember(JsInterop::json_fileExt()))
            fileExt = opts[JsInterop::json_fileExt()].asString();

        return FileProps(opts[JsInterop::json_name()].asString(), fileExt,  opts[JsInterop::json_localFileName()].asString());
    }

    static FileProps getEmbedFileArg(NapiInfoCR info) {
        auto props = getEmbedFileProps(info);

        BeJsValue opts(info[0]); // getEmbedFileProps would have thrown if this isn't an object
        props.m_date = DateTime::FromUnixMilliseconds(opts[JsInterop::json_date()].asInt64());
        if (!props.m_date.IsValid())
            BeNapi::ThrowJsException(info.Env(), "invalid date");

        props.m_compress = opts[JsInterop::json_compress()].asBool(true);
        return props;
    }

    Db& GetOpenedDb(NapiInfoCR info) {
        RequireDbIsOpen(info);
        return *_GetMyDb();
    }

    void EmbedFile(NapiInfoCR info) {
        Db& db = GetOpenedDb(info);
        auto props = getEmbedFileArg(info);
        DbResult stat;
        db.EmbeddedFiles().Import(&stat, props.m_compress, props.m_name.c_str(), props.m_localFileName.c_str(), &props.m_date, props.m_fileExt.c_str());
        if (stat != BE_SQLITE_OK) {
            Utf8String err = "error embedding [" + props.m_localFileName + "] in";
            JsInterop::throwSqlResult(err.c_str(), db.GetDbFileName(), stat);
        }
        db.SaveChanges();
    }

     Napi::Value QueryEmbeddedFile(NapiInfoCR info) {
        Db& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, name);

        uint64_t totalSize;
        DateTime lastModified;
        Utf8String fileExt;

        auto id = db.EmbeddedFiles().QueryFile(name.c_str(), &totalSize, &lastModified, nullptr, nullptr, &fileExt);
        if (!id.IsValid())
            return info.Env().Undefined();

        BeJsNapiObject out(info.Env());
        out[JsInterop::json_size()] = (int64_t)totalSize;
        int64_t date;
        lastModified.ToUnixMilliseconds(date);
        out[JsInterop::json_date()] = date;
        out[JsInterop::json_fileExt()] = fileExt;
        return out;
    }

    void ExtractEmbeddedFile(NapiInfoCR info) {
        Db& db = GetOpenedDb(info);
        auto props = getEmbedFileProps(info);
        auto stat = db.EmbeddedFiles().Export(props.m_localFileName.c_str(), props.m_name.c_str());
        if (stat != BE_SQLITE_OK)
            JsInterop::throwNotFound();
    }

    void ReplaceEmbeddedFile(NapiInfoCR info) {
        Db& db = GetOpenedDb(info);
        auto props = getEmbedFileArg(info);
        auto stat = db.EmbeddedFiles().Replace(props.m_name.c_str(), props.m_localFileName.c_str(), &props.m_date);
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error replacing embedded file", db.GetDbFileName(), stat);
        db.SaveChanges();
    }

    void RemoveEmbeddedFile(NapiInfoCR info) {
        Db& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, name);
        auto stat = db.EmbeddedFiles().Remove(name.c_str());
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error removing embedded file", db.GetDbFileName(), stat);
        db.SaveChanges();
    }

    // query a property from the be_prop table.
    Napi::Value QueryFileProperty(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, fileProps);
        REQUIRE_ARGUMENT_BOOL(1, wantString); // boolean indicating whether the desired property is a string or blob.
        BeJsConst propsJson(fileProps);
        if (!propsJson.isStringMember(JsInterop::json_namespace()) || !propsJson.isStringMember(JsInterop::json_name()))
            THROW_JS_EXCEPTION("Invalid FilePropertyProps");

        Utf8String nameProp = propsJson[JsInterop::json_name()].asString();
        Utf8String nsProp = propsJson[JsInterop::json_namespace()].asString();
        PropertySpec spec(nameProp.c_str(), nsProp.c_str()); // NOTE: PropertySpec does *not* make a copy - variables above hold values in memory;
        Db& db = GetOpenedDb(info);
        uint64_t id = propsJson[JsInterop::json_id()].asUInt64();
        uint64_t subId = propsJson[JsInterop::json_subId()].asUInt64();

        if (wantString) {
            Utf8String strVal;
            auto stat = db.QueryProperty(strVal, spec, id, subId);
            return (stat != BE_SQLITE_ROW) ? info.Env().Undefined() : toJsString(info.Env(), strVal);
        }

        uint32_t size;
        auto stat = db.QueryPropertySize(size, spec, id, subId);
        if (stat != BE_SQLITE_ROW || size == 0)
            return info.Env().Undefined();

        auto blob = Napi::Uint8Array::New(info.Env(), size);
        db.QueryProperty(blob.Data(), size, spec, id, subId);
        return blob;
    }

    // save a property to the be_prop table
    void SaveFileProperty(NapiInfoCR info) {
        if (info.Length() < 2)
            THROW_JS_EXCEPTION("saveFileProperty requires 2 arguments");

        REQUIRE_ARGUMENT_ANY_OBJ(0, fileProps);
        BeJsConst propsJson(fileProps);
        if (!propsJson.isMember(JsInterop::json_namespace()) || !propsJson.isMember(JsInterop::json_name()))
            THROW_JS_EXCEPTION("Invalid FilePropertyProps");

        Utf8String nameProp = propsJson[JsInterop::json_name()].asString();
        Utf8String nsProp = propsJson[JsInterop::json_namespace()].asString();
        PropertySpec spec(nameProp.c_str(), nsProp.c_str()); // NOTE: PropertySpec does *not* make a copy - variables above hold values in memory;
        auto& db = GetOpenedDb(info);
        uint64_t id = propsJson[JsInterop::json_id()].asUInt64();
        uint64_t subId = propsJson[JsInterop::json_subId()].asUInt64();

        DbResult stat;
        Utf8String strbuf;
        Utf8StringCP strDataP = nullptr;
        void const* value = nullptr;
        uint32_t propsize = 0;

        if (info[1].IsString()) {
            strbuf = info[1].ToString().Utf8Value().c_str();
            strDataP = &strbuf;
        }
        if (info[2].IsTypedArray()) {
            auto arrayBuf = info[2].As<Napi::Uint8Array>();
            value = arrayBuf.Data();
            propsize = arrayBuf.ByteLength();
        }
        if (nullptr == strDataP && nullptr == value) {
            stat = db.DeleteProperty(spec, id, subId);
            if (stat == BE_SQLITE_DONE)
                stat = BE_SQLITE_OK;
        } else
            stat = strDataP ? db.SaveProperty(spec, *strDataP, value, propsize, id, subId) : db.SaveProperty(spec, value, propsize, id, subId);
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error saving property", spec.GetName(), stat);
    }

    Napi::Value QueryNextAvailableFileProperty(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, fileProps);
        BeJsConst propsJson(fileProps);
        if (!propsJson.isStringMember(JsInterop::json_namespace()) || !propsJson.isStringMember(JsInterop::json_name()))
            THROW_JS_EXCEPTION("Invalid FilePropertyProps");

        auto& db = GetOpenedDb(info);
        Statement stmt(db, "SELECT count(Id),max(Id) FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=?");
        stmt.BindText(1, propsJson[JsInterop::json_namespace()].asCString(), Statement::MakeCopy::Yes);
        stmt.BindText(2, propsJson[JsInterop::json_name()].asCString(), Statement::MakeCopy::Yes);
        DbResult result = stmt.Step();
        uint64_t count = stmt.GetValueUInt64(0);
        uint64_t max = stmt.GetValueUInt64(1);
        uint64_t next = (result != BE_SQLITE_ROW || 0 == count) ? 0 : max + 1;
        return Napi::Number::New(info.Env(), next);
    }

    void EmbedFont(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, arg);
        BeJsConst argJson(arg);

        bool compressFont = argJson[JsInterop::json_compress()].asBool(false);
        auto db = &GetOpenedDb(info);
        auto dgnDb = dynamic_cast<DgnDbP>(db);
        std::unique_ptr<FontDb> fontDbHolder;

        FontDbP fontDb;
        if (nullptr != dgnDb)
            fontDb = &dgnDb->Fonts().m_fontDb;
        else {
            fontDb = new FontDb(*db, true);
            fontDbHolder.reset(fontDb);
        }

        if (argJson.isMember(JsInterop::json_data())) {
            bvector<FontFace> faces;
            FontFace face(argJson[JsInterop::json_face()]);
            if (face.m_familyName.empty())
                BeNapi::ThrowJsException(info.Env(), "invalid face");
            faces.emplace_back(face);

            auto napiData = argJson[JsInterop::json_data()].AsNapiValueRef();
            if (!napiData->m_napiVal.IsTypedArray())
                BeNapi::ThrowJsException(info.Env(), "font data not valid");

            auto arrayBuf = napiData->m_napiVal.As<Napi::Uint8Array>();
            if (SUCCESS == fontDb->EmbedFont(faces, ByteStream(arrayBuf.Data(), arrayBuf.ByteLength()), compressFont))
                return;
        }
        if (SUCCESS == fontDb->EmbedFontFile(argJson[JsInterop::json_fileName()].asString().c_str(), compressFont))
            return;

        if (SystemTrueTypeFont(argJson[JsInterop::json_systemFont()].asString().c_str(), compressFont).Embed(*fontDb))
            return;

        BeNapi::ThrowJsException(info.Env(), "unable to embed font");
    }

    void RequireDbIsOpen(NapiInfoCR info) {
        if (!_GetMyDb()->IsDbOpen())
            BeNapi::ThrowJsException(info.Env(), "db is not open");
    }

    Napi::Value IsOpen(NapiInfoCR info) {
        return Napi::Boolean::New(info.Env(), _GetMyDb()->IsDbOpen());
    }

    Napi::Value IsReadonly(NapiInfoCR info) {
        return Napi::Boolean::New(info.Env(), GetOpenedDb(info).IsReadonly());
    }

    Napi::Value GetFilePath(NapiInfoCR info) {
        return toJsString(info.Env(), GetOpenedDb(info).GetDbFileName());
    }

    void Vacuum(NapiInfoCR info) {
        Db& db = GetOpenedDb(info);
        int pageSize = 0;
        Utf8String into;
        if (info[0].IsObject()) {
            auto opts = info[0].As<Napi::Object>();
            pageSize = intMember(opts, "pageSize", 0);
            into = stringMember(opts, "into");
        }

        DbResult status = into.empty() ? db.Vacuum(pageSize) : db.VacuumInto(into.c_str());
        if (status != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error vacuuming", db.GetDbFileName(), status);
    }

    void EnableWalMode(Napi::CallbackInfo const& info) {
        Db& db = GetOpenedDb(info);
        OPTIONAL_ARGUMENT_BOOL(0, yesNo, true);
        auto status = db.EnableWalMode(yesNo);
        if (BE_SQLITE_OK != status)
            JsInterop::throwSqlResult("error changing WAL mode", db.GetDbFileName(), status);
    }

    void SetAutoCheckpointThreshold(Napi::CallbackInfo const& info) {
        Db& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_INTEGER(0, frames);
        auto status = db.SetAutoCheckpointThreshold(frames);
        if (status != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error setting autoCheckpoint threshold", db.GetDbFileName(), status);
    }
    void PerformCheckpoint(Napi::CallbackInfo const& info) {
        Db& db = GetOpenedDb(info);
        OPTIONAL_ARGUMENT_INTEGER(0, mode, 3);
        DbResult status = db.PerformCheckpoint((WalCheckpointMode)mode);
        if (status != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error checkpointing", db.GetDbFileName(), status);
    }

    void RestartDefaultTxn(NapiInfoCR info) {
        GetOpenedDb(info).RestartDefaultTxn();
    }

    Napi::Value GetLastInsertRowId(NapiInfoCR info) {
        return Napi::Number::New(info.Env(), GetOpenedDb(info).GetLastInsertRowId());
    }
    Napi::Value GetLastError(NapiInfoCR info) {
        return Napi::String::New(info.Env(), GetOpenedDb(info).GetLastError().c_str());
    }
};

//=======================================================================================
// Projects the ECDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeECDb : BeObjectWrap<NativeECDb>, SQLiteOps {
private:
    DEFINE_CONSTRUCTOR
    ECDb m_ecdb;

public:
    NativeECDb(NapiInfoCR info) : BeObjectWrap<NativeECDb>(info) {}
    ~NativeECDb() { SetInDestructor(); CloseDbIfOpen(); }
    ECDbR GetECDb() { return m_ecdb; }
    Db* _GetMyDb() override { return &m_ecdb; }

    // Check if val is really a NativeECDb peer object
    static bool InstanceOf(Napi::Value val) {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
    }

    Napi::Value CreateDb(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        DbResult status = JsInterop::CreateECDb(m_ecdb, BeFileName(dbName.c_str(), true));
        if (BE_SQLITE_OK == status) {
            m_ecdb.AddFunction(HexStrSqlFunction::GetSingleton());
            m_ecdb.AddFunction(StrSqlFunction::GetSingleton());
        }

        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value OpenDb(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        OPTIONAL_ARGUMENT_BOOL(2, upgrade, false);

        Db::OpenParams params((Db::OpenMode)mode);
        if (upgrade)
            params.SetProfileUpgradeOptions(Db::ProfileUpgradeOptions::Upgrade);

        DbResult status = JsInterop::OpenECDb(m_ecdb, BeFileName(dbName.c_str(), true), params);
        if (BE_SQLITE_OK == status) {
            m_ecdb.AddFunction(HexStrSqlFunction::GetSingleton());
            m_ecdb.AddFunction(StrSqlFunction::GetSingleton());
        }

        return Napi::Number::New(Env(), (int)status);
    }

    void ConcurrentQueryExecute(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestObj);
        REQUIRE_ARGUMENT_FUNCTION(1, callback);
        JsInterop::ConcurrentQueryExecute(m_ecdb, requestObj, callback);
    }

    Napi::Value ConcurrentQueryResetConfig(NapiInfoCR info) {
        if (info.Length() > 0 && info[0].IsObject()) {
            Napi::Object inConf = info[0].As<Napi::Object>();
            return JsInterop::ConcurrentQueryResetConfig(Env(), m_ecdb, inConf);
        }
        return JsInterop::ConcurrentQueryResetConfig(Env(), m_ecdb);
    }
    void ConcurrentQueryShutdown(NapiInfoCR info) {
        ConcurrentQueryMgr::Shutdown(m_ecdb);
    }
    void CloseDbIfOpen() {
        if (m_ecdb.IsDbOpen()) {
            m_ecdb.AbandonChanges();
            m_ecdb.RemoveFunction(HexStrSqlFunction::GetSingleton());
            m_ecdb.RemoveFunction(StrSqlFunction::GetSingleton());
            m_ecdb.CloseDb();
        }
    }

    Napi::Value CloseDb(NapiInfoCR info) {
        CloseDbIfOpen();
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
    }

    void Dispose(NapiInfoCR info) {
        CloseDb(info);
    }

    Napi::Value SaveChanges(NapiInfoCR info) {
        OPTIONAL_ARGUMENT_STRING(0, changeSetName);
        DbResult status = m_ecdb.SaveChanges(changeSetName.empty() ? nullptr : changeSetName.c_str());
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value AbandonChanges(NapiInfoCR info) {
        DbResult status = m_ecdb.AbandonChanges();
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value ImportSchema(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaPathName);
        DbResult status = JsInterop::ImportSchema(m_ecdb, BeFileName(schemaPathName.c_str(), true));
        return Napi::Number::New(Env(), (int)status);
    }
    void DropSchema(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        auto rc = m_ecdb.Schemas().DropSchema(schemaName);
        if (rc.GetStatus() != DropSchemaResult::Success) {
            THROW_JS_EXCEPTION(rc.GetStatusAsString());
        }
    }
    static Napi::Value EnableSharedCache(NapiInfoCR info) {
        REQUIRE_ARGUMENT_BOOL(0, enabled);
        DbResult r = BeSQLiteLib::EnableSharedCache(enabled);
        return Napi::Number::New(info.Env(), (int)r);
    }

    static void Init(Napi::Env env, Napi::Object exports) {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECDb", {
            InstanceMethod("abandonChanges", &NativeECDb::AbandonChanges),
            InstanceMethod("closeDb", &NativeECDb::CloseDb),
            InstanceMethod("concurrentQueryExecute", &NativeECDb::ConcurrentQueryExecute),
            InstanceMethod("concurrentQueryResetConfig", &NativeECDb::ConcurrentQueryResetConfig),
            InstanceMethod("concurrentQueryShutdown", &NativeECDb::ConcurrentQueryShutdown),
            InstanceMethod("createDb", &NativeECDb::CreateDb),
            InstanceMethod("dispose", &NativeECDb::Dispose),
            InstanceMethod("dropSchema", &NativeECDb::DropSchema),
            InstanceMethod("getFilePath", &NativeECDb::GetFilePath),
            InstanceMethod("getLastError", &NativeECDb::GetLastError),
            InstanceMethod("getLastInsertRowId", &NativeECDb::GetLastInsertRowId),
            InstanceMethod("importSchema", &NativeECDb::ImportSchema),
            InstanceMethod("isOpen", &NativeECDb::IsOpen),
            InstanceMethod("openDb", &NativeECDb::OpenDb),
            InstanceMethod("saveChanges", &NativeECDb::SaveChanges),
            StaticMethod("enableSharedCache", &NativeECDb::EnableSharedCache),
        });

        exports.Set("ECDb", t);
        SET_CONSTRUCTOR(t)
    }
};

/** Add the container to the openParms, if the argument is a container object */
static void addContainerParams(Napi::Object db, Utf8StringR dbName, Db::OpenParams& params, Napi::Value arg) {
    auto jsContainer = getJsCloudContainer(arg);
    if (!jsContainer.IsObject()) { // did they supply a container argument?
        db.Set(JSON_NAME(cloudContainer), db.Env().Undefined());
        return;
    }

    db.Set(JSON_NAME(cloudContainer), jsContainer);

    auto container = getCloudContainer(jsContainer);
    if (!params.IsReadonly() && !container->m_writeLockHeld)
        BeNapi::ThrowJsException(arg.Env(), "cannot open for database for write - container write lock not held");

    dbName = params.SetFromContainer(dbName.c_str(), container);
}

//=======================================================================================
// Projects the BeSQLite::Db class into JS
//! @bsiclass
//=======================================================================================
struct SQLiteDb : Napi::ObjectWrap<SQLiteDb>, SQLiteOps {
private:
    DEFINE_CONSTRUCTOR
    Db m_db;
    Db* _GetMyDb() override { return &m_db; }

public:
    SQLiteDb(NapiInfoCR info) : Napi::ObjectWrap<SQLiteDb>(info) {}
    ~SQLiteDb() { CloseDbIfOpen(true); }
    DbR GetDb() { return m_db; }

    static bool InstanceOf(Napi::Value val) {
        if (!val.IsObject())
            return false;
        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
    }

    void useOpenParams(Db::OpenParams& params, BeJsValue args) {
        if (args[JSON_NAME(openMode)].isNumeric())
            params.m_openMode = (Db::OpenMode)args[JSON_NAME(openMode)].asInt();
        params.m_rawSQLite = args["rawSQLite"].asBool();
        params.m_skipFileCheck = args["skipFileCheck"].asBool();
        if (args["immutable"].asBool())
            params.SetImmutable();
        if (args[JSON_NAME(defaultTxn)].isNumeric())
            params.m_startDefaultTxn = (DefaultTxn)args[JSON_NAME(defaultTxn)].asInt();
        if (args[JSON_NAME(queryParam)].isString())
            params.AddQueryParam(args[JSON_NAME(queryParam)].asCString());
    }

    void CreateDb(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        Db::CreateParams params;
        addContainerParams(Value(), dbName, params, info[1]);

        if (info[2].IsObject()) {
            auto args = BeJsValue(info[2]);
            useOpenParams(params, args);
            if (args[JSON_NAME(pageSize)].isNumeric())
                params.m_pagesize = (Db::PageSize) args[JSON_NAME(pageSize)].asInt();
        }

        auto stat = m_db.CreateNewDb(dbName.c_str(), params);
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("cannot create database", dbName.c_str(), stat);
    }

    void OpenDb(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        Db::OpenParams params(Db::OpenMode::Readonly);
        if (info[1].IsObject()) {
            useOpenParams(params, info[1]);
        } else {
            REQUIRE_ARGUMENT_INTEGER(1, openMode);
            params.m_openMode = (Db::OpenMode) openMode;
        }

        addContainerParams(Value(), dbName, params, info[2]);

        auto stat = m_db.OpenBeSQLiteDb(dbName.c_str(), params);
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("cannot open database", dbName.c_str(), stat);
    }

    void CloseDbIfOpen(bool fromDestructor) {
        if (m_db.IsDbOpen()) {
            m_db.AbandonChanges();
            m_db.CloseDb();
            if (!fromDestructor)
                Value().Set(JSON_NAME(cloudContainer), Env().Undefined());
        }
    }

    void CloseDb(NapiInfoCR info) { CloseDbIfOpen(false);}
    void Dispose(NapiInfoCR info) { CloseDbIfOpen(false);}

    void SaveChanges(NapiInfoCR info) {
        RequireDbIsOpen(info);
        DbResult status = m_db.SaveChanges();
        if (status != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error in saveChanges", m_db.GetDbFileName(), status);
    }

    void AbandonChanges(NapiInfoCR info) {
        RequireDbIsOpen(info);
        DbResult status = m_db.AbandonChanges();
        if (status != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error in abandonChanges", m_db.GetDbFileName(), status);
    }

    static void Init(Napi::Env env, Napi::Object exports) {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "SQLiteDb", {
            InstanceMethod("abandonChanges", &SQLiteDb::AbandonChanges),
            InstanceMethod("closeDb", &SQLiteDb::CloseDb),
            InstanceMethod("createDb", &SQLiteDb::CreateDb),
            InstanceMethod("dispose", &SQLiteDb::Dispose),
            InstanceMethod("embedFile", &SQLiteDb::EmbedFile),
            InstanceMethod("embedFont", &SQLiteDb::EmbedFont),
            InstanceMethod("extractEmbeddedFile", &SQLiteDb::ExtractEmbeddedFile),
            InstanceMethod("getFilePath", &SQLiteDb::GetFilePath),
            InstanceMethod("getLastInsertRowId", &SQLiteDb::GetLastInsertRowId),
            InstanceMethod("getLastError", &SQLiteDb::GetLastError),
            InstanceMethod("isOpen", &SQLiteDb::IsOpen),
            InstanceMethod("isReadonly", &SQLiteDb::IsReadonly),
            InstanceMethod("openDb", &SQLiteDb::OpenDb),
            InstanceMethod("queryEmbeddedFile", &SQLiteDb::QueryEmbeddedFile),
            InstanceMethod("queryFileProperty", &SQLiteDb::QueryFileProperty),
            InstanceMethod("queryNextAvailableFileProperty", &SQLiteDb::QueryNextAvailableFileProperty),
            InstanceMethod("removeEmbeddedFile", &SQLiteDb::RemoveEmbeddedFile),
            InstanceMethod("replaceEmbeddedFile", &SQLiteDb::ReplaceEmbeddedFile),
            InstanceMethod("restartDefaultTxn", &SQLiteDb::RestartDefaultTxn),
            InstanceMethod("saveChanges", &SQLiteDb::SaveChanges),
            InstanceMethod("saveFileProperty", &SQLiteDb::SaveFileProperty),
            InstanceMethod("vacuum", &SQLiteDb::Vacuum),
            InstanceMethod("enableWalMode", &SQLiteDb::EnableWalMode),
            InstanceMethod("performCheckpoint", &SQLiteDb::PerformCheckpoint),
            InstanceMethod("setAutoCheckpointThreshold", &SQLiteDb::SetAutoCheckpointThreshold),
        });

        exports.Set("SQLiteDb", t);
        SET_CONSTRUCTOR(t)
    }
};

//=======================================================================================
// Projects the NativeECSchemaXmlContext class into JS
//! @bsiclass
//=======================================================================================
struct NativeECSchemaXmlContext : BeObjectWrap<NativeECSchemaXmlContext>
    {
    private:
        DEFINE_CONSTRUCTOR
        ECSchemaReadContextPtr m_context;
        ECSchemaXmlContextUtils::LocaterCallbackUPtr m_locater;

    public:
        NativeECSchemaXmlContext(NapiInfoCR info) : BeObjectWrap<NativeECSchemaXmlContext>(info)
            {
            m_context = ECSchemaXmlContextUtils::CreateSchemaReadContext(T_HOST.GetIKnownLocationsAdmin());
            }

        ~NativeECSchemaXmlContext() {SetInDestructor();}

        // Check if val is really a NativeECSchemaXmlContext peer object
        static bool HasInstance(Napi::Value val) {
            if (!val.IsObject())
                return false;
            Napi::Object obj = val.As<Napi::Object>();
            return obj.InstanceOf(Constructor().Value());
            }

        void SetSchemaLocater(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_FUNCTION(0, locaterCallback);
            ECSchemaXmlContextUtils::SetSchemaLocater(*m_context, m_locater, Napi::Persistent(locaterCallback));
            }

        void SetFirstSchemaLocater(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_FUNCTION(0, locaterCallback);
            ECSchemaXmlContextUtils::SetFirstSchemaLocater(*m_context, m_locater, Napi::Persistent(locaterCallback));
            }

        void AddSchemaPath(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_STRING(0, schemaPath);
            ECSchemaXmlContextUtils::AddSchemaPath(*m_context, schemaPath);
            }

        Napi::Value ReadSchemaFromXmlFile(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_STRING(0, filePath);
            BeJsDocument schemaJson;
            auto status = ECSchemaXmlContextUtils::ConvertECSchemaXmlToJson(schemaJson, *m_context, filePath);

            if (ECSchemaXmlContextUtils::SchemaConversionStatus::Success == status)
                return CreateBentleyReturnSuccessObject(Napi::String::New(Env(), schemaJson.Stringify().c_str()));

            return CreateBentleyReturnErrorObject(BentleyStatus::ERROR, ECSchemaXmlContextUtils::SchemaConversionStatusToString(status), Env());
            }

        static void Init(Napi::Env env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ECSchemaXmlContext", {
                InstanceMethod("addSchemaPath", &NativeECSchemaXmlContext::AddSchemaPath),
                InstanceMethod("readSchemaFromXmlFile", &NativeECSchemaXmlContext::ReadSchemaFromXmlFile),
                InstanceMethod("setSchemaLocater", &NativeECSchemaXmlContext::SetSchemaLocater),
                InstanceMethod("setFirstSchemaLocater", &NativeECSchemaXmlContext::SetFirstSchemaLocater)
            });

            exports.Set("ECSchemaXmlContext", t);

            SET_CONSTRUCTOR(t);
            }
    };

//=======================================================================================
// Async Worker that classifies geometry clip containment on another thread.
//! @bsiclass
//=======================================================================================
struct FenceAsyncWorker : DgnDbWorker
{
    BeJsDocument m_input;
    void Execute() final {FenceContext::DoClassify(m_output, m_input, GetDb(), this);}
    FenceAsyncWorker(DgnDbR db, Napi::Object input) : DgnDbWorker(db, input.Env()) { m_input.From(input); }
};

//=======================================================================================
// Async Worker that calculates geometry mass properties on another thread.
//! @bsiclass
//=======================================================================================
struct MassPropertiesAsyncWorker : DgnDbWorker
{
    BeJsDocument m_input;
    void Execute() final {MeasureGeomCollector::DoMeasure(m_output, m_input, GetDb(), this);}
    MassPropertiesAsyncWorker(DgnDbR db, Napi::Object input) : DgnDbWorker(db, input.Env()) { m_input.From(input); }
};

//=======================================================================================
// Async Worker that caches element geometry in app data on another thread.
//! @bsiclass
//=======================================================================================
struct ElementGeometryCacheAsyncWorker : DgnDbWorker
{
    BeJsDocument m_input;
    void Execute() final  {ElementGeometryCache::Populate(GetDb(), m_output, m_input, *this);}
    ElementGeometryCacheAsyncWorker(DgnDbR db, Napi::Object input) : DgnDbWorker(db, input.Env()) {m_input.From(input); }
};

//=======================================================================================
// Async Worker that serializes the schema to JSON on another thread.
//! @bsiclass
//=======================================================================================
struct ECSchemaSerializationAsyncWorker : DgnDbWorker {
    Utf8String m_schemaName;

    void Execute() final {
        auto schema = GetDb().Schemas().GetSchema(m_schemaName, true);
        if (nullptr == schema) {
            SetError("schema not found");
            return;
        }
        if (!schema->WriteToJsonValue(m_output))
            SetError("schema serialization error");
    }
    ECSchemaSerializationAsyncWorker(DgnDbR db, Napi::Env env, Utf8StringCR schemaName) : DgnDbWorker(db,env), m_schemaName(schemaName) {}
};

struct QueryModelExtentsWorker : DgnDbWorker {
private:
  bvector<DgnModelId> m_modelIds;

  void AppendStatus(DgnModelId modelId, DgnDbStatus status) {
    Append(modelId, status, AxisAlignedBox3d());
  }

  void AppendExtents(DgnModelId modelId, AxisAlignedBox3dCR extents) {
    Append(modelId, DgnDbStatus::Success, extents);
  }

  void Append(DgnModelId modelId, DgnDbStatus status, AxisAlignedBox3dCR extents) {
    auto entry = m_output.appendObject();
    entry["id"] = modelId;
    entry["status"] = static_cast<int>(status);
    extents.ToJson(entry["extents"]);
  }
public:
  QueryModelExtentsWorker(DgnDbR db, Napi::Env env, bvector<DgnModelId>&& modelIds) : DgnDbWorker(db, env), m_modelIds(std::move(modelIds)) { }

  void Execute() final {
    auto& db = GetDb();
    for (auto const& modelId : m_modelIds) {
      if (!modelId.IsValid()) {
        AppendStatus(modelId, DgnDbStatus::InvalidId);
        continue;
      }

      auto model = db.Models().GetModel(modelId);
      if (model.IsNull()) {
        AppendStatus(modelId, DgnDbStatus::NotFound);
        continue;
      }

      auto geomModel = model->ToGeometricModel();
      if (nullptr == geomModel) {
        AppendStatus(modelId, DgnDbStatus::WrongModel);
        continue;
      }

      AppendExtents(modelId, geomModel->QueryElementsRange());
    }
  }

  // We can't use the base implementation if we want to return an array.
  void OnOK() final {
    Napi::Array retVal = Napi::Array::New(Env());
    BeJsValue result(retVal);
    result.From(m_output);
    m_promise.Resolve(retVal);
  }
};

//=======================================================================================
// Projects the DgnDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeDgnDb : BeObjectWrap<NativeDgnDb>, SQLiteOps
    {
    static constexpr int ERROR_UsageTrackingFailed = -100;

    DEFINE_CONSTRUCTOR;

    Db* _GetMyDb() override { return m_dgndb.get(); }
    Dgn::DgnDbPtr m_dgndb;
    // Cancellation token associated with the currently-open DgnDb.
    // The 'cancelled' flag is set on the main thread when the DgnDb is being closed.
    // The token can be passed into functions which create async workers and stored on those
    // workers such that it can be checked for cancellation in the worker's Execute() function to
    // trivially exit if the DgnDb was closed in the interim.
    AtomicCancellablePtr m_cancellationToken;
    ElementGraphicsRequestsUPtr m_elemGraphicsRequests;

    NativeDgnDb(NapiInfoCR info) : BeObjectWrap<NativeDgnDb>(info) {}
    ~NativeDgnDb() {SetInDestructor(); CloseDgnDb(true);}

    //  Check if val is really a NativeDgnDb peer object
    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    DgnDbR GetDgnDb() {return *m_dgndb;}

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg = nullptr) {return IModelJsNative::CreateBentleyReturnErrorObject(errCode, msg, Env());}

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode, Napi::Value goodValue) {return ((STATUSTYPE)0 != errCode) ? CreateBentleyReturnErrorObject(errCode) : CreateBentleyReturnSuccessObject(goodValue); }

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode) {return CreateBentleyReturnObject(errCode, Env().Undefined());}

    bool IsOpen() const {return m_dgndb.IsValid();}

    Napi::Value IsDgnDbOpen(NapiInfoCR info) { return Napi::Boolean::New(Env(), IsOpen()); }

    Napi::Value IsReadonly(NapiInfoCR info) { return Napi::Boolean::New(Env(), m_dgndb->IsReadonly()); }

    void SetIModelDb(NapiInfoCR info) {
        Napi::Value obj = info[0];
        if (m_dgndb.IsValid()) {
            if (obj.IsObject())
                m_dgndb->m_private_iModelDbJs.Reset(obj.As<Napi::Object>(), 1);
            else
                m_dgndb->m_private_iModelDbJs.Reset();
        }
    }

    void SetDgnDb(DgnDbR dgndb)
        {
        JsInterop::AddCrashReportDgnDb(dgndb);
        dgndb.AddFunction(HexStrSqlFunction::GetSingleton());
        dgndb.AddFunction(StrSqlFunction::GetSingleton());

        m_dgndb = &dgndb;
        m_cancellationToken = new AtomicCancellable();
        }

    void ClearDgnDb()
        {
        // Schedule cancellation of any async workers (but do not wait for them to finish).
        auto workers = DgnDbWorkers::Get(*m_dgndb);
        if (workers.IsValid())
            workers->CancelAll(false);

        // Wait for all tile graphics requests to be cancelled and removed.
        if (m_elemGraphicsRequests)
            {
            m_elemGraphicsRequests->Terminate();
            m_elemGraphicsRequests.reset(nullptr);
            }

        // Wait for all async workers to be cancelled and removed.
        if (workers.IsValid())
            {
            workers->CancelAll(true);
            m_dgndb->DropAppData(DgnDbWorkers::GetKey());
            }

        BeAssert(m_cancellationToken.IsValid());
        m_cancellationToken->Cancel();
        m_cancellationToken = nullptr;

        m_dgndb->RemoveFunction(HexStrSqlFunction::GetSingleton());
        m_dgndb->RemoveFunction(StrSqlFunction::GetSingleton());
        m_dgndb->m_private_iModelDbJs.Reset(); // disconnect the iModelDb object
        JsInterop::RemoveCrashReportDgnDb(*m_dgndb);

        m_dgndb = nullptr;
        }

    void OpenIModelDb(BeFileNameCR dbname, DgnDb::OpenParams& openParams) {
        if (!openParams.IsReadonly())
            openParams.SetBusyRetry(new BeSQLite::BusyRetry(40, 500)); // retry 40 times, 1/2 second intervals (20 seconds total)
        NativeLogging::CategoryLogger("BeSQLite").infov(L"Opening DgnDb %ls", dbname.c_str());

        DbResult result;
        auto dgndb = DgnDb::OpenIModelDb(&result, dbname, openParams);
        if (BE_SQLITE_OK != result)
            JsInterop::throwSqlResult("error opening iModel", dbname.GetNameUtf8().c_str(), result);

        SetDgnDb(*dgndb);
    }

    void CloseDgnDb(bool fromDestructor) {
        if (!m_dgndb.IsValid())
            return;

        if (m_dgndb->Txns().HasChanges())
            m_dgndb->SaveChanges();

        DgnDbPtr dgndb = m_dgndb;
        ClearDgnDb();
        dgndb->CloseDb();

        if (!fromDestructor)
            Value().Set(JSON_NAME(cloudContainer), Env().Undefined());
    }

    void OpenIModel(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        REQUIRE_ARGUMENT_INTEGER(1, mode);

        SchemaUpgradeOptions::DomainUpgradeOptions domainOptions = SchemaUpgradeOptions::DomainUpgradeOptions::CheckRequiredUpgrades;
        BeSQLite::Db::ProfileUpgradeOptions profileOptions = BeSQLite::Db::ProfileUpgradeOptions::None;
        if (!ARGUMENT_IS_EMPTY(2)) {
            Napi::Object upgradeOptions = info[2].As<Napi::Object>();

            Napi::Number valDomain;
            valDomain = upgradeOptions.Get("domain").ToNumber();
            if (!valDomain.IsUndefined() && !valDomain.IsNull()) {
                domainOptions = (SchemaUpgradeOptions::DomainUpgradeOptions) valDomain.Uint32Value();
            }

            Napi::Number valProfile;
            valProfile = upgradeOptions.Get("profile").ToNumber();
            if (!valProfile.IsUndefined() && !valProfile.IsNull()) {
                profileOptions = (BeSQLite::Db::ProfileUpgradeOptions) valProfile.Uint32Value();
            }
        }
        SchemaUpgradeOptions schemaUpgradeOptions(domainOptions);
        DgnDb::OpenParams openParams((Db::OpenMode)mode, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);
        openParams.SetProfileUpgradeOptions(profileOptions);

        if (info[3].IsObject()) {
            auto props = BeJsConst(info[3].As<Napi::Object>());
            auto tempFileBase = props[JSON_NAME(tempFileBase)];
            if (tempFileBase.isString())
                openParams.m_tempfileBase = tempFileBase.asString();
        }

        addContainerParams(Value(), dbName, openParams, info[4]);
        OpenIModelDb(BeFileName(dbName), openParams);
    }

    void CreateIModel(NapiInfoCR info)  {
        REQUIRE_ARGUMENT_STRING(0, filename);
        REQUIRE_ARGUMENT_ANY_OBJ(1, props);
        SetDgnDb(*JsInterop::CreateIModel(filename, props)); // CreateIModel throws on errors
    }

    Napi::Value GetECClassMetaData(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, s);
        REQUIRE_ARGUMENT_STRING(1, c);
        BeJsNapiObject metaData(Env());
        auto status = JsInterop::GetECClassMetaData(metaData, GetDgnDb(), s.c_str(), c.c_str());
        return CreateBentleyReturnObject(status, NapiValueRef::Stringify(metaData));
        }

    Napi::Value GetSchemaItem(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        REQUIRE_ARGUMENT_STRING(1, itemName);
        BeJsNapiObject metaData(Env());
        auto status = JsInterop::GetSchemaItem(metaData, GetDgnDb(), schemaName.c_str(), itemName.c_str());
        return CreateBentleyReturnObject(status, NapiValueRef::Stringify(metaData));
        }

    Napi::Value GetSchemaProps(NapiInfoCR info)  {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        auto schema = m_dgndb->Schemas().GetSchema(schemaName, true);
        if (nullptr == schema)
            BeNapi::ThrowJsException(info.Env(), "schema not found");

        BeJsNapiObject props(info.Env());
        if (!schema->WriteToJsonValue(props))
            BeNapi::ThrowJsException(info.Env(), "unable to serialize schema");
        return props;
    }

    Napi::Value GetSchemaPropsAsync(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        DgnDbWorkerPtr worker = new ECSchemaSerializationAsyncWorker(*m_dgndb, info.Env(), schemaName);
        return worker->Queue();  // Schema serialization happens in another thread
    }

    Napi::Value GetElement(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, opts);
        BeJsNapiObject jsValue(Env());
        auto status = JsInterop::GetElement(jsValue, GetDgnDb(), opts);
        if (DgnDbStatus::Success != status)
            BeNapi::ThrowJsException(Env(), "error reading element", (int)status);
        return jsValue;
    }

    Napi::Value GetModel(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, opts);
        BeJsNapiObject modelJson(Env());
        DgnDbStatus status = JsInterop::GetModel(modelJson, GetDgnDb(), opts);
        if (DgnDbStatus::Success != status)
            BeNapi::ThrowJsException(Env(), "error reading model", (int)status);
        return modelJson;
    }

    Napi::Value GetTempFileBaseName(NapiInfoCR info) {
        RequireDbIsOpen(info);
        return toJsString(Env(), GetDgnDb().GetTempFileBaseName());
    }

    Napi::Value SetGeometricModelTrackingEnabled(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_BOOL(0, enable);

        auto& modelChanges = GetDgnDb().Txns().m_modelChanges;
        if (enable != modelChanges.IsTrackingGeometry())
            {
            auto status = modelChanges.SetTrackingGeometry(enable);
            auto readonly = false;
            switch (status)
                {
                case TxnManager::ModelChanges::Status::Readonly:
                    readonly = true;
                    // fall-through intentional.
                case TxnManager::ModelChanges::Status::VersionTooOld:
                    return CreateBentleyReturnErrorObject(readonly ? DgnDbStatus::ReadOnly : DgnDbStatus::VersionTooOld);
                }
            }

        return CreateBentleyReturnSuccessObject(Napi::Boolean::New(Env(), modelChanges.IsTrackingGeometry()));
        }

    Napi::Value IsGeometricModelTrackingSupported(NapiInfoCR info)

        {
        RequireDbIsOpen(info);
        return Napi::Boolean::New(Env(), TxnManager::ModelChanges::Status::Success == GetDgnDb().Txns().m_modelChanges.DetermineStatus());
        }

    Napi::Value QueryModelExtents(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, options);
        BeJsNapiObject extentsJson(Env());
        JsInterop::QueryModelExtents(extentsJson, GetDgnDb(), options);
        return extentsJson;
        }

    Napi::Value QueryModelExtentsAsync(NapiInfoCR info) {
        RequireDbIsOpen(info);
        if (ARGUMENT_IS_NOT_PRESENT(0) || !info[0].IsArray()) {
            THROW_JS_TYPE_EXCEPTION("Argument 0 must be an array of model Ids")
        }

      bvector<DgnModelId> modelIds;
      Napi::Array napiIds = info[0].As<Napi::Array>();
      for (uint32_t i = 0; i < napiIds.Length(); i++) {
        DgnModelId modelId;
        Napi::Value napiId = napiIds[i];
        if (napiId.IsString())
          modelId = DgnModelId(BeInt64Id::FromString(napiId.As<Napi::String>().Utf8Value().c_str()).GetValueUnchecked());

        modelIds.push_back(modelId);
      }

      DgnDbWorkerPtr worker = new QueryModelExtentsWorker(*m_dgndb, info.Env(), std::move(modelIds));
      return worker->Queue();
    }

    Napi::Value DumpChangeSet(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, changeSet);

        RevisionStatus status = JsInterop::DumpChangeSet(*m_dgndb, changeSet);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ExtractChangedInstanceIdsFromChangeSets(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        if (ARGUMENT_IS_NOT_PRESENT(0) || !info[0].IsArray()) {
            THROW_JS_TYPE_EXCEPTION("Argument 0 must be an array of strings")
        }
        bvector<BeFileName> changeSetFiles;
        Napi::Array arr = info[0].As<Napi::Array>();
        for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {
            Napi::Value arrValue = arr[arrIndex];
            if (arrValue.IsString())
                changeSetFiles.emplace_back(arrValue.As<Napi::String>().Utf8Value());
        }

        Napi::Object napiResult = Napi::Object::New(info.Env());
        BeJsValue json{napiResult};
        DgnDbStatus status = JsInterop::ExtractChangedInstanceIdsFromChangeSets(json, *m_dgndb, changeSetFiles);
        return CreateBentleyReturnObject(status, napiResult);
        }

    static TxnManager::TxnId TxnIdFromString(Utf8StringCR str) {
        return TxnManager::TxnId(BeInt64Id::FromString(str.c_str()).GetValueUnchecked());
    }
    static Utf8String TxnIdToString(TxnManager::TxnId txnId) {return BeInt64Id(txnId.GetValue()).ToHexStr();}

    Napi::Value GetCurrentTxnId(NapiInfoCR info)
        {
        return toJsString(Env(), TxnIdToString(m_dgndb->Txns().GetCurrentTxnId()));
        }

    Napi::Value QueryFirstTxnId(NapiInfoCR info) {
        OPTIONAL_ARGUMENT_BOOL(0, allowCrossSessions, true); // default to true for backwards compatibility
        auto& txns = m_dgndb->Txns();
        TxnManager::TxnId startTxnId = allowCrossSessions ? txns.QueryNextTxnId(TxnManager::TxnId(0)) : txns.GetSessionStartId();
        return toJsString(Env(), TxnIdToString(startTxnId));
    }

    Napi::Value QueryNextTxnId(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        auto next = m_dgndb->Txns().QueryNextTxnId(TxnIdFromString(txnIdHexStr));
        return toJsString(Env(), TxnIdToString(next));
    }
    Napi::Value QueryPreviousTxnId(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        auto next = m_dgndb->Txns().QueryPreviousTxnId(TxnIdFromString(txnIdHexStr));
        return toJsString(Env(), TxnIdToString(next));
    }
    Napi::Value GetTxnDescription(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return toJsString(Env(), m_dgndb->Txns().GetTxnDescription(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value IsTxnIdValid(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return Napi::Boolean::New(Env(), TxnIdFromString(txnIdHexStr).IsValid());
    }
    Napi::Value HasFatalTxnError(NapiInfoCR info) { return Napi::Boolean::New(Env(), m_dgndb->Txns().HasFatalError()); }
    void LogTxnError(NapiInfoCR info) {
        REQUIRE_ARGUMENT_BOOL(0, fatal);
        m_dgndb->Txns().LogError(fatal);
    }
    void EnableTxnTesting(NapiInfoCR info) {
        m_dgndb->Txns().EnableTracking(true);
        m_dgndb->Txns().InitializeTableHandlers();
    }
    Napi::Value BeginMultiTxnOperation(NapiInfoCR info) {return Napi::Number::New(Env(),  (int) m_dgndb->Txns().BeginMultiTxnOperation());}
    Napi::Value EndMultiTxnOperation(NapiInfoCR info) {return Napi::Number::New(Env(),  (int) m_dgndb->Txns().EndMultiTxnOperation());}
    Napi::Value GetMultiTxnOperationDepth(NapiInfoCR info) {return Napi::Number::New(Env(),  (int) m_dgndb->Txns().GetMultiTxnOperationDepth());}
    Napi::Value GetUndoString(NapiInfoCR info) {
        return toJsString(Env(), m_dgndb->Txns().GetUndoString());
    }
    Napi::Value GetRedoString(NapiInfoCR info) {return toJsString(Env(), m_dgndb->Txns().GetRedoString());}
    Napi::Value HasUnsavedChanges(NapiInfoCR info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().HasChanges());}
    Napi::Value HasPendingTxns(NapiInfoCR info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().HasPendingTxns());}
    Napi::Value IsIndirectChanges(NapiInfoCR info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().IsIndirectChanges());}
    Napi::Value IsRedoPossible(NapiInfoCR info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().IsRedoPossible());}
    Napi::Value IsUndoPossible(NapiInfoCR info) {
        return Napi::Boolean::New(Env(), m_dgndb->Txns().IsUndoPossible());
    }
    void RestartTxnSession(NapiInfoCR info) {m_dgndb->Txns().Initialize();}
    Napi::Value ReinstateTxn(NapiInfoCR info) {return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReinstateTxn());}
    Napi::Value ReverseAll(NapiInfoCR info) {return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseAll());}
    Napi::Value ReverseTo(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseTo(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value CancelTo(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().CancelTo(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value ReverseTxns(NapiInfoCR info) {
        REQUIRE_ARGUMENT_NUMBER(0, numTxns );
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseTxns(numTxns));
    }
    Napi::Value ClassNameToId(NapiInfoCR info) {
        auto classId = ECJsonUtilities::GetClassIdFromClassNameJson(info[0], m_dgndb->GetClassLocater());
        return toJsString(Env(), classId);
    }
    Napi::Value ClassIdToName(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, idString);
        DgnClassId classId;
        DgnClassId::FromString(classId, idString.c_str());
        auto ecClass = m_dgndb->Schemas().GetClass(classId);
        return (nullptr == ecClass) ? Env().Undefined() : toJsString(Env(), ecClass->GetFullName());
    }

    Napi::Value StartCreateChangeset(NapiInfoCR info) {
        RequireDbIsOpen(info);
        RevisionManagerR revisions = m_dgndb->Revisions();

        if (revisions.IsCreatingRevision())
            revisions.AbandonCreateRevision();

        RevisionStatus status;
        DgnRevisionPtr revision = revisions.StartCreateRevision(&status);
        if (status != RevisionStatus::Success)
            BeNapi::ThrowJsException(Env(), "Error creating changeset", (int) status);

        BeJsNapiObject changesetInfo(Env());
        changesetInfo[JsInterop::json_id()] = revision->GetChangesetId().c_str();
        changesetInfo[JsInterop::json_index()] = 0;
        changesetInfo[JsInterop::json_parentId()] = revision->GetParentId().c_str();
        changesetInfo[JsInterop::json_pathname()] = Utf8String(revision->GetRevisionChangesFile()).c_str();
        changesetInfo[JsInterop::json_changesType()] = (int)(revision->ContainsSchemaChanges(*m_dgndb) ? 1 : 0);
        return changesetInfo;
    }

    void CompleteCreateChangeset(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, optObj);
        BeJsConst opts(optObj);
        if (!opts.isNumericMember(JsInterop::json_index()))
            BeNapi::ThrowJsException(Env(), "changeset index must be supplied");
        int32_t index = opts[JsInterop::json_index()].GetInt();

        auto stat = m_dgndb->Revisions().FinishCreateRevision(index);
        if (stat != RevisionStatus::Success)
            BeNapi::ThrowJsException(Env(), "Error finishing changeset", (int) stat);
    }

    void AbandonCreateChangeset(NapiInfoCR info) {
        RequireDbIsOpen(info);
        RevisionManagerR revisions = m_dgndb->Revisions();
        if (revisions.IsCreatingRevision())
            revisions.AbandonCreateRevision();
    }

    Napi::Value AddChildPropagatesChangesToParentRelationship(NapiInfoCR info)     {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        REQUIRE_ARGUMENT_STRING(1, className);
        auto status = m_dgndb->Txns().AddChildPropagatesChangesToParentRelationship(schemaName, className);
        return Napi::Number::New(Env(), (int) status);
    }

    Napi::Value AddNewFont(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, fontPropObj);
        BeJsConst fontProps(fontPropObj);
        int fontTypeVal = fontProps[JsInterop::json_type()].asInt(1);
        FontType fontType = fontTypeVal==3 ? FontType::Shx : fontTypeVal==2 ? FontType::Rsc : FontType::TrueType;
        Utf8String name = fontProps[JsInterop::json_name()].asString();
        if (name.empty())
            BeNapi::ThrowJsException(Env(), "Font name is invalid");
        auto id = m_dgndb->Fonts().GetId(fontType, name.c_str());
        return Napi::Number::New(Env(), (int) id.GetValue());
    }

    Napi::Value WriteFullElementDependencyGraphToFile(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, dotFileName);
        JsInterop::WriteFullElementDependencyGraphToFile(*m_dgndb, dotFileName);
        return Napi::Number::New(Env(), 0);
        }

    Napi::Value WriteAffectedElementDependencyGraphToFile(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, dotFileName);
        REQUIRE_ARGUMENT_STRING_ARRAY(1, id64Array);
        JsInterop::WriteAffectedElementDependencyGraphToFile(*m_dgndb, dotFileName, id64Array);
        return Napi::Number::New(Env(), 0);
        }

    Napi::Value GetMassProperties(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, request);
        DgnDbWorkerPtr worker = new MassPropertiesAsyncWorker(*m_dgndb, request); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        return worker->Queue();  // Measure happens in another thread
        }

    Napi::Value UpdateElementGeometryCache(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, request);
        DgnDbWorkerPtr worker = new ElementGeometryCacheAsyncWorker(*m_dgndb, request); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        return worker->Queue();  // Happens in another thread
        }

    Napi::Value ElementGeometryCacheOperation(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestProps);

        Napi::Value elementIdVal = requestProps.Get("id");
        if (!elementIdVal.IsString())
            THROW_JS_TYPE_EXCEPTION("target element id must be specified");

        try
            {
            return Napi::Number::New(Env(), (int) ElementGeometryCache::Operation(GetDgnDb(), requestProps, Env()));
            }
        catch (std::exception const& e)
            {
            THROW_JS_EXCEPTION(e.what());
            }
        }

    Napi::Value GetGeometryContainment(NapiInfoCR info)        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, request);
        DgnDbWorkerPtr worker = new FenceAsyncWorker(*m_dgndb, request); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        return worker->Queue();  // Containment check happens in another thread
    }

    Napi::Value GetIModelCoordsFromGeoCoords(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, geoCoordProps);
        BeJsNapiObject results(Env());
        JsInterop::GetIModelCoordsFromGeoCoords(results, GetDgnDb(), geoCoordProps);
        return results;
    }

    Napi::Value GetGeoCoordsFromIModelCoords(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, iModelCoordProps);
        BeJsNapiObject results(Env());
        JsInterop::GetGeoCoordsFromIModelCoords(results, GetDgnDb(), iModelCoordProps);
        return results;
    }

    Napi::Value GetIModelProps(NapiInfoCR info) {
        RequireDbIsOpen(info);
        BeJsNapiObject props(Env());
        JsInterop::GetIModelProps(props, *m_dgndb);
        return props;
    }

    Napi::Value InsertElement(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, elemProps);
        Napi::Value insertOptions;
        if (ARGUMENT_IS_PRESENT(1)) {\
            insertOptions = info[1].As<Napi::Object>();
        } else {
            insertOptions = Env().Undefined();
        }
        return JsInterop::InsertElement(GetDgnDb(), elemProps, insertOptions);
    }

    void UpdateElement(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, elemProps);
        JsInterop::UpdateElement(GetDgnDb(), elemProps);
    }

    void DeleteElement(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, elemIdStr);
        JsInterop::DeleteElement(GetDgnDb(), elemIdStr);
    }

    Napi::Value QueryDefinitionElementUsage(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING_ARRAY(0, hexStringIds);
        Napi::Object usageInfoObj = Napi::Object::New(Env());
        BeJsValue usageInfo = BeJsValue(usageInfoObj);
        DgnDbStatus status = JsInterop::QueryDefinitionElementUsage(usageInfo, GetDgnDb(), hexStringIds);
        return DgnDbStatus::Success == status ? usageInfoObj : Env().Undefined();
        }

    Napi::Value BeginPurgeOperation(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        GetDgnDb().BeginPurgeOperation();
        return Napi::Number::New(Env(), (int)DgnDbStatus::Success);
        }

    Napi::Value EndPurgeOperation(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        GetDgnDb().EndPurgeOperation();
        return Napi::Number::New(Env(), (int)DgnDbStatus::Success);
        }

    Napi::Value SimplifyElementGeometry(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, simplifyArgs);
        auto status = JsInterop::SimplifyElementGeometry(GetDgnDb(), simplifyArgs);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value InlineGeometryPartReferences(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        auto result = JsInterop::InlineGeometryParts(GetDgnDb());

        auto ret = Napi::Object::New(Env());
        ret.Set(Napi::String::New(Env(), "numCandidateParts"), Napi::Number::New(Env(), result.m_numCandidateParts));
        ret.Set(Napi::String::New(Env(), "numRefsInlined"), Napi::Number::New(Env(), result.m_numRefsInlined));
        ret.Set(Napi::String::New(Env(), "numPartsDeleted"), Napi::Number::New(Env(), result.m_numPartsDeleted));

        return ret;
        }

    Napi::Value InsertElementAspect(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, aspectProps);
        return JsInterop::InsertElementAspect(GetDgnDb(), aspectProps);
    }

    void UpdateElementAspect(NapiInfoCR info)     {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, aspectProps);
        JsInterop::UpdateElementAspect(GetDgnDb(), aspectProps);
    }

    void DeleteElementAspect(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, aspectIdStr);
        JsInterop::DeleteElementAspect(GetDgnDb(), aspectIdStr);
    }

    Napi::Value ExportGraphics(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, exportProps);

        Napi::Value onGraphicsVal = exportProps.Get("onGraphics");
        if (!onGraphicsVal.IsFunction())
            THROW_JS_TYPE_EXCEPTION("onGraphics must be a function");

        Napi::Value elementIdArrayVal = exportProps.Get("elementIdArray");
        if (!elementIdArrayVal.IsArray())
            THROW_JS_TYPE_EXCEPTION("elementIdArray must be an array");

        DgnDbStatus status = JsInterop::ExportGraphics(GetDgnDb(), exportProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ExportPartGraphics(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, exportProps);

        Napi::Value onPartGraphicsVal = exportProps.Get("onPartGraphics");
        if (!onPartGraphicsVal.IsFunction())
            THROW_JS_TYPE_EXCEPTION("onPartsGraphics must be a function");

        Napi::Value displayPropsVal = exportProps.Get("displayProps");
        if (!displayPropsVal.IsObject())
            THROW_JS_TYPE_EXCEPTION("displayProps must be an object");

        Napi::Value elementIdVal = exportProps.Get("elementId");
        if (!elementIdVal.IsString())
            THROW_JS_TYPE_EXCEPTION("elementId must be a string");

        DgnDbStatus status = JsInterop::ExportPartGraphics(GetDgnDb(), exportProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ProcessGeometryStream(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestProps);

        Napi::Value onGeometryVal = requestProps.Get("onGeometry");
        if (!onGeometryVal.IsFunction())
            THROW_JS_TYPE_EXCEPTION("onGeometry must be a function");

        Napi::Value elementIdVal = requestProps.Get("elementId");
        if (!elementIdVal.IsString())
            THROW_JS_TYPE_EXCEPTION("elementId must be specified");

        try
            {
            return Napi::Number::New(Env(), (int) GeometryStreamIO::ProcessGeometryStream(GetDgnDb(), requestProps, Env()));
            }
        catch (std::exception const& e)
            {
            THROW_JS_EXCEPTION(e.what());
            }
        }

    Napi::Value CreateBRepGeometry(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, createProps);

        Napi::Number operationVal = createProps.Get("operation").As<Napi::Number>();
        if (!operationVal.IsNumber())
            THROW_JS_EXCEPTION("operation must be a specified");

        Napi::Value onResultVal = createProps.Get("onResult");
        if (!onResultVal.IsFunction())
            THROW_JS_EXCEPTION("onResult must be a function");

        Napi::Array entryArrayVal = createProps.Get("entryArray").As<Napi::Array>();
        if (!entryArrayVal.IsArray())
            THROW_JS_TYPE_EXCEPTION("entryArray must be specified");

        try
            {
            return Napi::Number::New(Env(), (int) GeometryStreamIO::CreateBRepGeometry(GetDgnDb(), createProps, Env()));
            }
        catch (std::exception const& e)
            {
            THROW_JS_EXCEPTION(e.what());
            }
        }

    void GetTileTree(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, idStr);
        REQUIRE_ARGUMENT_FUNCTION(1, responseCallback);

        JsInterop::GetTileTree(m_cancellationToken.get(), GetDgnDb(), idStr, responseCallback);
        }

    void GetTileContent(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, treeIdStr);
        REQUIRE_ARGUMENT_STRING(1, tileIdStr);
        REQUIRE_ARGUMENT_FUNCTION(2, responseCallback);

        JsInterop::GetTileContent(m_cancellationToken.get(), GetDgnDb(), treeIdStr, tileIdStr, responseCallback);
        }

    void PurgeTileTrees(NapiInfoCR info)
        {
        bvector<DgnModelId> modelIds;
        if (info.Length() == 1 && !info[0].IsUndefined())
            {
            if (info[0].IsArray())
                {
                Napi::Array arr = info[0].As<Napi::Array>();
                for (uint32_t index = 0; index < arr.Length(); ++index)
                    {
                    Napi::Value value = arr[index];
                    if (value.IsString())
                        {
                        auto modelId = BeInt64Id::FromString(value.As<Napi::String>().Utf8Value().c_str());
                        if (modelId.IsValid())
                            modelIds.push_back(DgnModelId(modelId.GetValue()));
                        }
                    }
                }

            if (modelIds.empty())
                THROW_JS_TYPE_EXCEPTION("Argument 0 must be a non-empty array of valid Id64Strings");
            }

        T_HOST.Visualization().PurgeTileTrees(GetDgnDb(), modelIds.empty() ? nullptr : &modelIds);
        }

    Napi::Value PollTileContent(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, treeIdStr);
        REQUIRE_ARGUMENT_STRING(1, tileIdStr);

        auto result = T_HOST.Visualization().PollTileContent(m_cancellationToken.get(), GetDgnDb(), treeIdStr, tileIdStr);
#ifdef BENTLEY_HAVE_VARIANT
        if (std::holds_alternative<Dgn::Visualization::PollState>(result))
          return CreateBentleyReturnSuccessObject(Napi::Number::New(Env(), static_cast<int>(std::get<Dgn::Visualization::PollState>(result))));
        else if (std::holds_alternative<DgnDbStatus>(result))
          return CreateBentleyReturnErrorObject(std::get<DgnDbStatus>(result));

        auto pContent = std::get_if<Dgn::Visualization::PolledTileContent>(&result);
#else
        if (DgnDbStatus::Success != result.m_status)
          return CreateBentleyReturnErrorObject(result.m_status);
        else if (!result.m_content.IsValid())
          return CreateBentleyReturnSuccessObject(Napi::Number::New(Env(), static_cast<int>(result.m_state)));

        auto pContent = &result.m_content;
#endif
        BeAssert(nullptr != pContent);
        if (nullptr == pContent)
          return CreateBentleyReturnErrorObject(DgnDbStatus::NotFound);

        ByteStreamCR geometry = pContent->m_content->GetBytes();
        auto blob = Napi::Uint8Array::New(Env(), geometry.size());
        memcpy(blob.Data(), geometry.data(), geometry.size());

        Napi::Object jsTileContent = Napi::Object::New(Env());
        jsTileContent.Set(Napi::String::New(Env(), "content"), blob);
        jsTileContent.Set(Napi::String::New(Env(), "elapsedSeconds"), Napi::Number::New(Env(), pContent->m_elapsedSeconds));

        return CreateBentleyReturnSuccessObject(jsTileContent);
        }

    void CancelTileContentRequests(NapiInfoCR info)
        {
        if (!IsOpen())
            return;

        REQUIRE_ARGUMENT_STRING(0, treeId);
        REQUIRE_ARGUMENT_STRING_ARRAY(1, tileIds);

        if (!tileIds.empty())
            T_HOST.Visualization().CancelContentRequests(GetDgnDb(), treeId, tileIds);
        }

    void CancelElementGraphicsRequests(NapiInfoCR info)
        {
        if (!IsOpen())
            return;

        REQUIRE_ARGUMENT_STRING_ARRAY(0, requestIds);
        if (!requestIds.empty())
            ObtainTileGraphicsRequests().Cancel(requestIds);
        }

    Napi::Value GenerateElementGraphics(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestProps);
        return ObtainTileGraphicsRequests().Enqueue(requestProps, info.Env());
        }

    Napi::Value GenerateElementMeshes(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestProps);
        return JsInterop::GenerateElementMeshes(GetDgnDb(), requestProps);
        }

    Napi::Value IsLinkTableRelationship(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_STRING(0, qualifiedClassName);
        Utf8String alias, className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, className, qualifiedClassName.c_str()))
            return Env().Undefined();

        ClassMapStrategy strategy = GetDgnDb().Schemas().GetClassMapStrategy(alias, className, SchemaLookupMode::AutoDetect);
        if (strategy.IsEmpty())
            return Env().Undefined();

        return Napi::Boolean::New(Env(), strategy.IsLinkTableRelationship());
        }

    Napi::Value InsertLinkTableRelationship(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, props);
        return JsInterop::InsertLinkTableRelationship(GetDgnDb(), props);
    }

    void UpdateLinkTableRelationship(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, props);
        JsInterop::UpdateLinkTableRelationship(GetDgnDb(), props);
    }

    void DeleteLinkTableRelationship(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, props);
        JsInterop::DeleteLinkTableRelationship(GetDgnDb(), props);
    }

    Napi::Value InsertCodeSpec(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, name);
        REQUIRE_ARGUMENT_ANY_OBJ(1, jsonProperties);
        return JsInterop::InsertCodeSpec(GetDgnDb(), name, jsonProperties);
    }

    void UpdateCodeSpec(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, codeSpecIdStr);
        CodeSpecId codeSpecId(BeInt64Id::FromString(codeSpecIdStr.c_str()).GetValueUnchecked());
        REQUIRE_ARGUMENT_ANY_OBJ(1, jsonProperties);
        return JsInterop::UpdateCodeSpec(GetDgnDb(), codeSpecId, jsonProperties);
    }

    Napi::Value InsertModel(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, modelProps);
        return JsInterop::InsertModel(GetDgnDb(), modelProps);
        }

    void UpdateModel(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, modelProps);
        JsInterop::UpdateModel(GetDgnDb(), modelProps);
    }

    Napi::Value UpdateModelGeometryGuid(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_STRING(0, modelIdStr);
        DgnModelId modelId(BeInt64Id::FromString(modelIdStr.c_str()).GetValueUnchecked());
        auto status = JsInterop::UpdateModelGeometryGuid(GetDgnDb(), modelId);
        return Napi::Number::New(Env(), static_cast<int>(status));
        }

    void DeleteModel(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, elemIdStr);
        JsInterop::DeleteModel(GetDgnDb(), elemIdStr);
    }

    Napi::Value SaveChanges(NapiInfoCR info)
        {
        OPTIONAL_ARGUMENT_STRING(0, description);
        RequireDbIsOpen(info);
        auto stat = GetDgnDb().SaveChanges(description);
        return Napi::Number::New(Env(), (int)stat);
        }

    Napi::Value AbandonChanges(NapiInfoCR info)
        {
        DbResult status = GetDgnDb().AbandonChanges();
        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value ImportFunctionalSchema(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        DbResult result = JsInterop::ImportFunctionalSchema(GetDgnDb());
        return Napi::Number::New(Env(), (int)result);
        }
    void DropSchema(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        auto rc = GetDgnDb().DropSchema(schemaName);
        if (rc.GetStatus() != DropSchemaResult::Success) {
            THROW_JS_EXCEPTION(rc.GetStatusAsString());
        }
    }
    Napi::Value ImportSchemas(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING_ARRAY(0, schemaFileNames);
        DbResult result = JsInterop::ImportSchemasDgnDb(GetDgnDb(), schemaFileNames);
        return Napi::Number::New(Env(), (int)result);
        }

    Napi::Value ImportXmlSchemas(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING_ARRAY(0, serializedXmlSchemas);
        DbResult result = JsInterop::ImportXmlSchemas(GetDgnDb(), serializedXmlSchemas);
        return Napi::Number::New(Env(), (int)result);
        }

    Napi::Value FindGeometryPartReferences(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING_ARRAY(0, partIds);
        REQUIRE_ARGUMENT_BOOL(1, is2d);

        auto elemIds = JsInterop::FindGeometryPartReferences(partIds, is2d, GetDgnDb());
        uint32_t index = 0;
        auto ret = Napi::Array::New(Env(), elemIds.size());
        for (auto elemId : elemIds)
            ret.Set(index++, Napi::String::New(Env(), elemId.ToHexStr().c_str()));

        return ret;
        }

    Napi::Value ExportSchema(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        REQUIRE_ARGUMENT_STRING(1, exportDirectory);

        ECSchemaCP schema = GetDgnDb().Schemas().GetSchema(schemaName);
        if (nullptr == schema)
            BeNapi::ThrowJsException(info.Env(), "specified schema was not found");

        BeFileName schemaFileName(exportDirectory);
        schemaFileName.AppendSeparator();
        schemaFileName.AppendUtf8(schema->GetFullSchemaName().c_str());
        schemaFileName.AppendExtension(L"ecschema.xml");
        ECVersion xmlVersion = ECSchema::ECVersionToWrite(schema->GetOriginalECXmlVersionMajor(), schema->GetOriginalECXmlVersionMinor());

        SchemaWriteStatus status = schema->WriteToXmlFile(schemaFileName.GetName(), xmlVersion);
        if (SchemaWriteStatus::Success != status)
            return Napi::Number::New(Env(), (int) status);

        return Napi::Number::New(Env(), (int) SchemaWriteStatus::Success);
        }

    Napi::Value ExportSchemas(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, exportDirectory);
        bvector<ECN::ECSchemaCP> schemas = GetDgnDb().Schemas().GetSchemas();
        for (ECSchemaCP schema : schemas)
            {
            BeFileName schemaFileName(exportDirectory);
            schemaFileName.AppendSeparator();
            schemaFileName.AppendUtf8(schema->GetFullSchemaName().c_str());
            schemaFileName.AppendExtension(L"ecschema.xml");
            ECVersion xmlVersion = ECSchema::ECVersionToWrite(schema->GetOriginalECXmlVersionMajor(), schema->GetOriginalECXmlVersionMinor());

            SchemaWriteStatus status = schema->WriteToXmlFile(schemaFileName.GetName(), xmlVersion);
            if (SchemaWriteStatus::Success != status)
                return Napi::Number::New(Env(), (int) status);
            }
        return Napi::Number::New(Env(), (int) SchemaWriteStatus::Success);
        }

    Napi::Value SchemaToXmlString(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        OPTIONAL_ARGUMENT_UINTEGER(1, version, (uint32_t)ECVersion::Latest);
        auto schema = GetDgnDb().Schemas().GetSchema(schemaName, true);
        if (nullptr == schema)
            return info.Env().Undefined();

        ECN::SchemaWriteStatus status;
        Utf8String xml;
        if (ECVersion::V2_0 == (ECVersion)version)
            status = ECN::ECSchema::WriteToEC2XmlString(xml, const_cast<ECN::ECSchemaP>(schema)); // I checked:  WriteToEC2XmlString does NOT modify the input schema. (In fact, it makes a copy of it and then transforms the copy)
        else
            status = schema->WriteToXmlString(xml, (ECVersion)version);

        if (ECN::SchemaWriteStatus::Success != status)
            return info.Env().Undefined();

        return Napi::String::New(info.Env(), xml.c_str());
        }

    void CloseIModel(NapiInfoCR info) { CloseDgnDb(false); }

    Napi::Value CreateClassViewsInDb(NapiInfoCR info) {
        RequireDbIsOpen(info);
        BentleyStatus status = GetDgnDb().Schemas().CreateClassViewsInDb();
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value CreateChangeCache(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_OBJ(0, NativeECDb, changeCacheECDb);
        REQUIRE_ARGUMENT_STRING(1, changeCachePathStr);
        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetDgnDb().CreateChangeCache(changeCacheECDb->GetECDb(), changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value AttachChangeCache(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, changeCachePathStr);
        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetDgnDb().AttachChangeCache(changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value DetachChangeCache(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        DbResult stat = GetDgnDb().DetachChangeCache();
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value IsChangeCacheAttached(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        bool isAttached = GetDgnDb().IsChangeCacheAttached();
        return Napi::Boolean::New(Env(), isAttached);
        }

    Napi::Value ExtractChangeSummary(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_OBJ(0, NativeECDb, changeCacheECDb);
        REQUIRE_ARGUMENT_STRING(1, changesetFilePathStr);
        BeFileName changesetFilePath(changesetFilePathStr.c_str(), true);
        RevisionChangesFileReader changeStream(changesetFilePath, GetDgnDb());
        PERFLOG_START("iModelJsNative", "ExtractChangeSummary>ECDb::ExtractChangeSummary");
        ECInstanceKey changeSummaryKey;
        if (SUCCESS != ECDb::ExtractChangeSummary(changeSummaryKey, changeCacheECDb->GetECDb(), GetDgnDb(), ChangeSetArg(changeStream)))
            return CreateBentleyReturnErrorObject(BE_SQLITE_ERROR, Utf8PrintfString("Failed to extract ChangeSummary for ChangeSet file '%s'.", changesetFilePathStr.c_str()).c_str());
        PERFLOG_FINISH("iModelJsNative", "ExtractChangeSummary>ECDb::ExtractChangeSummary");

        return CreateBentleyReturnSuccessObject(toJsString(Env(), changeSummaryKey.GetInstanceId()));
        }


    Napi::Value GetBriefcaseId(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        return Napi::Number::New(Env(), m_dgndb->GetBriefcaseId().GetValue());
        }

    Napi::Value GetCurrentChangeset(NapiInfoCR info) {
        RequireDbIsOpen(info);
        int32_t index;
        Utf8String id;
        m_dgndb->Revisions().GetParentRevision(index, id);
        BeJsNapiObject retVal(Env());
        retVal[JsInterop::json_id()] = id;
        if (index >= 0)
            retVal[JsInterop::json_index()] = index;
        return retVal;
    }

    Napi::Value GetIModelId(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        BeGuid beGuid = m_dgndb->GetDbGuid();
        return toJsString(Env(), beGuid.ToString());
        }

    Napi::Value SetIModelId(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, guidStr);
        BeGuid guid;
        guid.FromString(guidStr.c_str());
        m_dgndb->ChangeDbGuid(guid);
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
        }

    Napi::Value SetITwinId(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, guidStr);
        BeGuid guid;
        guid.FromString(guidStr.c_str());
        m_dgndb->SaveProjectGuid(guid);
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
        }

    Napi::Value GetITwinId(NapiInfoCR info) {
        RequireDbIsOpen(info);
        BeGuid beGuid = m_dgndb->QueryProjectGuid();
        return toJsString(Env(), beGuid.ToString());
    }

    void ResetBriefcaseId(NapiInfoCR info) {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_INTEGER(0, newId);
        DbResult stat;
        try {
            stat = m_dgndb->ResetBriefcaseId(BeSQLite::BeBriefcaseId(newId));
        } catch(std::runtime_error e) {
            THROW_JS_EXCEPTION(e.what());
        }
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("Cannot reset briefcaseId for", m_dgndb->GetDbFileName(), stat);
    }

    static Napi::Value GetAssetDir(NapiInfoCR info)
        {
        BeFileName asset = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        return Napi::String::New(info.Env(), asset.GetNameUtf8().c_str());
        }

    static Napi::Value EnableSharedCache(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_BOOL(0, enabled);
        DbResult r = BeSQLiteLib::EnableSharedCache(enabled);
        return Napi::Number::New(info.Env(), (int) r);
        }

    void UpdateProjectExtents(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, newExtentsJson)
        JsInterop::UpdateProjectExtents(GetDgnDb(), BeJsDocument(newExtentsJson));
        }

    Napi::Value ComputeProjectExtents(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_BOOL(0, wantFullExtents);
        REQUIRE_ARGUMENT_BOOL(1, wantOutliers);

        DRange3d fullExtents;
        bvector<BeInt64Id> outliers;
        auto extents = GetDgnDb().GeoLocation().ComputeProjectExtents(wantFullExtents ? &fullExtents : nullptr, wantOutliers ? &outliers : nullptr);

        BeJsNapiObject result(info.Env());
        BeJsGeomUtils::DRange3dToJson(result["extents"], extents);

        if (wantFullExtents)
            BeJsGeomUtils::DRange3dToJson(result["fullExtents"], fullExtents);

        if (wantOutliers)
            {
            auto list = result["outliers"];
            list.toArray();
            int i=0;
            for (auto const& id : outliers)
                list[i++] = id;
            }

        return result;
    }

    void UpdateIModelProps(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, props)
        JsInterop::UpdateIModelProps(GetDgnDb(), BeJsConst(props));
    }

    Napi::Value ReadFontMap(NapiInfoCR info) {
        auto const& fontMap = GetDgnDb().Fonts().FontMap();
        BeJsNapiObject fonts(Env());
        auto fontList = fonts[JsInterop::json_fonts()];
        fontList.toArray();
        for (auto font : fontMap) {
            auto thisFont = fontList.appendValue();
            thisFont[JsInterop::json_id()] = (int)font.first.GetValue();
            thisFont[JsInterop::json_type()] = (int)font.second.m_type;
            thisFont[JsInterop::json_name()] = font.second.m_fontName;
        }
        return fonts;
    }

    // query a value from the be_local table.
    Napi::Value QueryLocalValue(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, name)
        Utf8String val;
        auto stat = GetDgnDb().QueryBriefcaseLocalValue(val, name.c_str());
        return (stat != BE_SQLITE_ROW) ? Env().Undefined() : toJsString(Env(), val);
    }

    // save a value to the be_local table.
    void SaveLocalValue(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, name)
        if (info[1].IsUndefined())
            return DeleteLocalValue(info);

        REQUIRE_ARGUMENT_STRING(1, val)
        auto stat = GetDgnDb().SaveBriefcaseLocalValue(name.c_str(), val);
        if (stat != BE_SQLITE_DONE)
            JsInterop::throwSqlResult("error saving local value", name.c_str(), stat);
    }

    // delete a value from the be_local table.
    void DeleteLocalValue(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, name)
        auto stat = GetDgnDb().DeleteBriefcaseLocalValue(name.c_str());
        if (stat != BE_SQLITE_DONE)
            JsInterop::throwSqlResult("error deleting local value", name.c_str(), stat);
    }

    // delete all local Txns. THIS IS VERY DANGEROUS! Don't use it unless you know what you're doing!
    void DeleteAllTxns(NapiInfoCR info) {
        GetDgnDb().Txns().DeleteAllTxns();
    }

    // get a texture image as a TextureData blob from the db. possibly downsample it to fit the client's maximum texture size, if specified.
    Napi::Value QueryTextureData(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        auto worker = DgnDbWorker::CreateTextureImageWorker(GetDgnDb(), info);
        return worker->Queue();
        }

    ElementGraphicsRequestsR ObtainTileGraphicsRequests()
        {
        BeAssert(IsOpen());
        if (!m_elemGraphicsRequests)
            {
            m_elemGraphicsRequests = T_HOST.Visualization().CreateElementGraphicsRequests(*m_dgndb);
            BeAssert(nullptr != m_elemGraphicsRequests);
            }

        return *m_elemGraphicsRequests;
        }
    Napi::Value GetChangesetSize(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        auto changesetSize = GetDgnDb().Txns().GetChangesetSize();
        return Napi::Number::New(Env(), changesetSize);
        }
    Napi::Value EnableChangesetSizeStats(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_BOOL(0, enabled);
        auto rc = GetDgnDb().Txns().EnableChangesetSizeStats(enabled);
        return Napi::Number::New(Env(), rc);
        }
    Napi::Value GetChangeTrackingMemoryUsed(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        auto memoryUsed = GetDgnDb().Txns().GetMemoryUsed();
        return Napi::Number::New(Env(), memoryUsed);
        }

    Napi::Value StartProfiler(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        OPTIONAL_ARGUMENT_STRING(0, scopeName);
        OPTIONAL_ARGUMENT_STRING(1, scenarioName);
        OPTIONAL_ARGUMENT_BOOL(2, overrideFlag, false);
        OPTIONAL_ARGUMENT_BOOL(4, computeExecutionPlan, true);

        auto rc = BeSQLite::Profiler::InitScope(GetDgnDb(), scopeName.c_str(), scenarioName.c_str(), BeSQLite::Profiler::Params(overrideFlag, computeExecutionPlan));
        if (rc == BE_SQLITE_OK)
            rc = BeSQLite::Profiler::GetScope(GetDgnDb())->Start();

        return Napi::Number::New(Env(), rc);
        }

    Napi::Value StopProfiler(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        auto scope = BeSQLite::Profiler::GetScope(GetDgnDb());
        DbResult rc;
        if (scope == nullptr)
            rc = BE_SQLITE_ERROR;
        else
            rc = scope->Stop();

        auto resultObj = Napi::Object::New(Env());
        resultObj["rc"] = Napi::Number::New(Env(), rc);
        if (rc == BE_SQLITE_OK) {
            resultObj["elapsedTime"] = Napi::Number::New(Env(), scope->GetElapsedTime());
            resultObj["scopeId"] = Napi::Number::New(Env(), scope->GetScopeId());
            resultObj["fileName"] = Napi::String::New(Env(), scope->GetProfileDbFileName().GetNameUtf8().c_str());
        }
        return resultObj;
        }

    Napi::Value PauseProfiler(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        auto scope = BeSQLite::Profiler::GetScope(GetDgnDb());
        DbResult rc;
        if (scope == nullptr)
            rc = BE_SQLITE_ERROR;
        else
            rc = scope->Pause();

        return Napi::Number::New(Env(), rc);
        }

    Napi::Value ResumeProfiler(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        auto scope = BeSQLite::Profiler::GetScope(GetDgnDb());
        DbResult rc;
        if (scope == nullptr)
            rc = BE_SQLITE_ERROR;
        else
            rc = scope->Resume();

        return Napi::Number::New(Env(), rc);;
        }

    Napi::Value IsProfilerRunning(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        auto scope = BeSQLite::Profiler::GetScope(GetDgnDb());
        if (scope == nullptr)
            return Napi::Boolean::New(Env(), false);;

        auto isRunning = scope->IsRunning();
        return Napi::Boolean::New(Env(), isRunning);
        }

    Napi::Value IsProfilerPaused(NapiInfoCR info)
        {
        RequireDbIsOpen(info);;
        auto scope = BeSQLite::Profiler::GetScope(GetDgnDb());
        if (scope == nullptr)
            return Napi::Boolean::New(Env(), false);;

        auto isPaused = scope->IsPaused();
        return Napi::Boolean::New(Env(), isPaused);
    }

    void ApplyChangeset(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, changeset);

        auto& db = GetDgnDb();
        auto revision = JsInterop::GetRevision(db.GetDbGuid().ToString(), changeset);

        auto currentId = db.Revisions().GetParentRevisionId();
        RevisionStatus stat =  RevisionStatus::ParentMismatch;
        if (revision->GetParentId() == currentId)  // merge
            stat = db.Revisions().MergeRevision(*revision);
        else if (revision->GetChangesetId() == currentId) //reverse
            stat = db.Revisions().ReverseRevision(*revision);
        if (RevisionStatus::Success != stat)
            BeNapi::ThrowJsException(Env(), "error applying changeset", (int)stat);
    }

    void ConcurrentQueryExecute(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestObj);
        REQUIRE_ARGUMENT_FUNCTION(1, callback);
        JsInterop::ConcurrentQueryExecute(GetDgnDb(), requestObj, callback);
    }

    Napi::Value ConcurrentQueryResetConfig(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        if (info.Length() > 0 && info[0].IsObject()) {
            Napi::Object inConf = info[0].As<Napi::Object>();
            return JsInterop::ConcurrentQueryResetConfig(Env(), GetDgnDb(), inConf);
        }
        return JsInterop::ConcurrentQueryResetConfig(Env(), GetDgnDb());
    }

    void ConcurrentQueryShutdown(NapiInfoCR info) {
        RequireDbIsOpen(info);;
        ConcurrentQueryMgr::Shutdown(GetDgnDb());
    }
    // ========================================================================================
    // Test method handler
    // ========================================================================================
    Napi::Value ExecuteTest(NapiInfoCR info)
        {
        RequireDbIsOpen(info);
        REQUIRE_ARGUMENT_STRING(0, testName);
        REQUIRE_ARGUMENT_STRING(1, params);
        return toJsString(Env(), JsInterop::ExecuteTest(GetDgnDb(), testName, params).ToString());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "DgnDb", {
            InstanceMethod("abandonChanges", &NativeDgnDb::AbandonChanges),
            InstanceMethod("abandonCreateChangeset", &NativeDgnDb::AbandonCreateChangeset),
            InstanceMethod("addChildPropagatesChangesToParentRelationship", &NativeDgnDb::AddChildPropagatesChangesToParentRelationship),
            InstanceMethod("addNewFont", &NativeDgnDb::AddNewFont),
            InstanceMethod("applyChangeset", &NativeDgnDb::ApplyChangeset),
            InstanceMethod("attachChangeCache", &NativeDgnDb::AttachChangeCache),
            InstanceMethod("beginMultiTxnOperation", &NativeDgnDb::BeginMultiTxnOperation),
            InstanceMethod("beginPurgeOperation", &NativeDgnDb::BeginPurgeOperation),
            InstanceMethod("cancelElementGraphicsRequests", &NativeDgnDb::CancelElementGraphicsRequests),
            InstanceMethod("cancelTileContentRequests", &NativeDgnDb::CancelTileContentRequests),
            InstanceMethod("cancelTo", &NativeDgnDb::CancelTo),
            InstanceMethod("classIdToName", &NativeDgnDb::ClassIdToName),
            InstanceMethod("classNameToId", &NativeDgnDb::ClassNameToId),
            InstanceMethod("closeIModel", &NativeDgnDb::CloseIModel),
            InstanceMethod("completeCreateChangeset", &NativeDgnDb::CompleteCreateChangeset),
            InstanceMethod("computeProjectExtents", &NativeDgnDb::ComputeProjectExtents),
            InstanceMethod("concurrentQueryExecute", &NativeDgnDb::ConcurrentQueryExecute),
            InstanceMethod("concurrentQueryResetConfig", &NativeDgnDb::ConcurrentQueryResetConfig),
            InstanceMethod("concurrentQueryShutdown", &NativeDgnDb::ConcurrentQueryShutdown),
            InstanceMethod("createBRepGeometry", &NativeDgnDb::CreateBRepGeometry),
            InstanceMethod("createChangeCache", &NativeDgnDb::CreateChangeCache),
            InstanceMethod("createClassViewsInDb", &NativeDgnDb::CreateClassViewsInDb),
            InstanceMethod("createIModel", &NativeDgnDb::CreateIModel),
            InstanceMethod("deleteAllTxns", &NativeDgnDb::DeleteAllTxns),
            InstanceMethod("deleteElement", &NativeDgnDb::DeleteElement),
            InstanceMethod("deleteElementAspect", &NativeDgnDb::DeleteElementAspect),
            InstanceMethod("deleteLinkTableRelationship", &NativeDgnDb::DeleteLinkTableRelationship),
            InstanceMethod("deleteLocalValue", &NativeDgnDb::DeleteLocalValue),
            InstanceMethod("deleteModel", &NativeDgnDb::DeleteModel),
            InstanceMethod("detachChangeCache", &NativeDgnDb::DetachChangeCache),
            InstanceMethod("dropSchema",&NativeDgnDb::DropSchema),
            InstanceMethod("dumpChangeset", &NativeDgnDb::DumpChangeSet),
            InstanceMethod("elementGeometryCacheOperation", &NativeDgnDb::ElementGeometryCacheOperation),
            InstanceMethod("embedFile", &NativeDgnDb::EmbedFile),
            InstanceMethod("embedFont", &NativeDgnDb::EmbedFont),
            InstanceMethod("enableChangesetSizeStats", &NativeDgnDb::EnableChangesetSizeStats),
            InstanceMethod("enableTxnTesting", &NativeDgnDb::EnableTxnTesting),
            InstanceMethod("endMultiTxnOperation", &NativeDgnDb::EndMultiTxnOperation),
            InstanceMethod("endPurgeOperation", &NativeDgnDb::EndPurgeOperation),
            InstanceMethod("executeTest", &NativeDgnDb::ExecuteTest),
            InstanceMethod("exportGraphics", &NativeDgnDb::ExportGraphics),
            InstanceMethod("exportPartGraphics", &NativeDgnDb::ExportPartGraphics),
            InstanceMethod("exportSchema", &NativeDgnDb::ExportSchema),
            InstanceMethod("exportSchemas", &NativeDgnDb::ExportSchemas),
            InstanceMethod("extractChangedInstanceIdsFromChangeSets", &NativeDgnDb::ExtractChangedInstanceIdsFromChangeSets),
            InstanceMethod("extractChangeSummary", &NativeDgnDb::ExtractChangeSummary),
            InstanceMethod("extractEmbeddedFile", &NativeDgnDb::ExtractEmbeddedFile),
            InstanceMethod("findGeometryPartReferences", &NativeDgnDb::FindGeometryPartReferences),
            InstanceMethod("generateElementGraphics", &NativeDgnDb::GenerateElementGraphics),
            InstanceMethod("generateElementMeshes", &NativeDgnDb::GenerateElementMeshes),
            InstanceMethod("getBriefcaseId", &NativeDgnDb::GetBriefcaseId),
            InstanceMethod("getChangesetSize", &NativeDgnDb::GetChangesetSize),
            InstanceMethod("getChangeTrackingMemoryUsed", &NativeDgnDb::GetChangeTrackingMemoryUsed),
            InstanceMethod("getCurrentChangeset", &NativeDgnDb::GetCurrentChangeset),
            InstanceMethod("getCurrentTxnId", &NativeDgnDb::GetCurrentTxnId),
            InstanceMethod("getECClassMetaData", &NativeDgnDb::GetECClassMetaData),
            InstanceMethod("getElement", &NativeDgnDb::GetElement),
            InstanceMethod("getFilePath", &NativeDgnDb::GetFilePath),
            InstanceMethod("getGeoCoordinatesFromIModelCoordinates", &NativeDgnDb::GetGeoCoordsFromIModelCoords),
            InstanceMethod("getGeometryContainment", &NativeDgnDb::GetGeometryContainment),
            InstanceMethod("getIModelCoordinatesFromGeoCoordinates", &NativeDgnDb::GetIModelCoordsFromGeoCoords),
            InstanceMethod("getIModelId", &NativeDgnDb::GetIModelId),
            InstanceMethod("getIModelProps", &NativeDgnDb::GetIModelProps),
            InstanceMethod("getITwinId", &NativeDgnDb::GetITwinId),
            InstanceMethod("getLastInsertRowId", &NativeDgnDb::GetLastInsertRowId),
            InstanceMethod("getLastError", &NativeDgnDb::GetLastError),
            InstanceMethod("getMassProperties", &NativeDgnDb::GetMassProperties),
            InstanceMethod("getModel", &NativeDgnDb::GetModel),
            InstanceMethod("getMultiTxnOperationDepth", &NativeDgnDb::GetMultiTxnOperationDepth),
            InstanceMethod("getRedoString", &NativeDgnDb::GetRedoString),
            InstanceMethod("getSchemaProps", &NativeDgnDb::GetSchemaProps),
            InstanceMethod("getSchemaPropsAsync", &NativeDgnDb::GetSchemaPropsAsync),
            InstanceMethod("getSchemaItem", &NativeDgnDb::GetSchemaItem),
            InstanceMethod("getTempFileBaseName", &NativeDgnDb::GetTempFileBaseName),
            InstanceMethod("getTileContent", &NativeDgnDb::GetTileContent),
            InstanceMethod("getTileTree", &NativeDgnDb::GetTileTree),
            InstanceMethod("getTxnDescription", &NativeDgnDb::GetTxnDescription),
            InstanceMethod("getUndoString", &NativeDgnDb::GetUndoString),
            InstanceMethod("hasFatalTxnError", &NativeDgnDb::HasFatalTxnError),
            InstanceMethod("hasPendingTxns", &NativeDgnDb::HasPendingTxns),
            InstanceMethod("hasUnsavedChanges", &NativeDgnDb::HasUnsavedChanges),
            InstanceMethod("importFunctionalSchema", &NativeDgnDb::ImportFunctionalSchema),
            InstanceMethod("importSchemas", &NativeDgnDb::ImportSchemas),
            InstanceMethod("importXmlSchemas", &NativeDgnDb::ImportXmlSchemas),
            InstanceMethod("inlineGeometryPartReferences", &NativeDgnDb::InlineGeometryPartReferences),
            InstanceMethod("insertCodeSpec", &NativeDgnDb::InsertCodeSpec),
            InstanceMethod("updateCodeSpec", &NativeDgnDb::UpdateCodeSpec),
            InstanceMethod("insertElement", &NativeDgnDb::InsertElement),
            InstanceMethod("insertElementAspect", &NativeDgnDb::InsertElementAspect),
            InstanceMethod("insertLinkTableRelationship", &NativeDgnDb::InsertLinkTableRelationship),
            InstanceMethod("insertModel", &NativeDgnDb::InsertModel),
            InstanceMethod("isChangeCacheAttached", &NativeDgnDb::IsChangeCacheAttached),
            InstanceMethod("isGeometricModelTrackingSupported", &NativeDgnDb::IsGeometricModelTrackingSupported),
            InstanceMethod("isIndirectChanges", &NativeDgnDb::IsIndirectChanges),
            InstanceMethod("isLinkTableRelationship", &NativeDgnDb::IsLinkTableRelationship),
            InstanceMethod("isOpen", &NativeDgnDb::IsDgnDbOpen),
            InstanceMethod("isProfilerPaused", &NativeDgnDb::IsProfilerPaused),
            InstanceMethod("isProfilerRunning", &NativeDgnDb::IsProfilerRunning),
            InstanceMethod("isReadonly", &NativeDgnDb::IsReadonly),
            InstanceMethod("isRedoPossible", &NativeDgnDb::IsRedoPossible),
            InstanceMethod("isTxnIdValid", &NativeDgnDb::IsTxnIdValid),
            InstanceMethod("isUndoPossible", &NativeDgnDb::IsUndoPossible),
            InstanceMethod("logTxnError", &NativeDgnDb::LogTxnError),
            InstanceMethod("openIModel", &NativeDgnDb::OpenIModel),
            InstanceMethod("pauseProfiler", &NativeDgnDb::PauseProfiler),
            InstanceMethod("pollTileContent", &NativeDgnDb::PollTileContent),
            InstanceMethod("processGeometryStream", &NativeDgnDb::ProcessGeometryStream),
            InstanceMethod("purgeTileTrees", &NativeDgnDb::PurgeTileTrees),
            InstanceMethod("queryDefinitionElementUsage", &NativeDgnDb::QueryDefinitionElementUsage),
            InstanceMethod("queryEmbeddedFile", &NativeDgnDb::QueryEmbeddedFile),
            InstanceMethod("queryFileProperty", &NativeDgnDb::QueryFileProperty),
            InstanceMethod("queryFirstTxnId", &NativeDgnDb::QueryFirstTxnId),
            InstanceMethod("queryLocalValue", &NativeDgnDb::QueryLocalValue),
            InstanceMethod("queryModelExtents", &NativeDgnDb::QueryModelExtents),
            InstanceMethod("queryModelExtentsAsync", &NativeDgnDb::QueryModelExtentsAsync),
            InstanceMethod("queryNextAvailableFileProperty", &NativeDgnDb::QueryNextAvailableFileProperty),
            InstanceMethod("queryNextTxnId", &NativeDgnDb::QueryNextTxnId),
            InstanceMethod("queryPreviousTxnId", &NativeDgnDb::QueryPreviousTxnId),
            InstanceMethod("queryTextureData", &NativeDgnDb::QueryTextureData),
            InstanceMethod("readFontMap", &NativeDgnDb::ReadFontMap),
            InstanceMethod("reinstateTxn", &NativeDgnDb::ReinstateTxn),
            InstanceMethod("removeEmbeddedFile", &NativeDgnDb::RemoveEmbeddedFile),
            InstanceMethod("replaceEmbeddedFile", &NativeDgnDb::ReplaceEmbeddedFile),
            InstanceMethod("resetBriefcaseId", &NativeDgnDb::ResetBriefcaseId),
            InstanceMethod("restartDefaultTxn", &NativeDgnDb::RestartDefaultTxn),
            InstanceMethod("restartTxnSession", &NativeDgnDb::RestartTxnSession),
            InstanceMethod("resumeProfiler", &NativeDgnDb::ResumeProfiler),
            InstanceMethod("reverseAll", &NativeDgnDb::ReverseAll),
            InstanceMethod("reverseTo", &NativeDgnDb::ReverseTo),
            InstanceMethod("reverseTxns", &NativeDgnDb::ReverseTxns),
            InstanceMethod("saveChanges", &NativeDgnDb::SaveChanges),
            InstanceMethod("saveFileProperty", &NativeDgnDb::SaveFileProperty),
            InstanceMethod("saveLocalValue", &NativeDgnDb::SaveLocalValue),
            InstanceMethod("schemaToXmlString", &NativeDgnDb::SchemaToXmlString),
            InstanceMethod("setGeometricModelTrackingEnabled", &NativeDgnDb::SetGeometricModelTrackingEnabled),
            InstanceMethod("setIModelDb", &NativeDgnDb::SetIModelDb),
            InstanceMethod("setIModelId", &NativeDgnDb::SetIModelId),
            InstanceMethod("setITwinId", &NativeDgnDb::SetITwinId),
            InstanceMethod("simplifyElementGeometry", &NativeDgnDb::SimplifyElementGeometry),
            InstanceMethod("startCreateChangeset", &NativeDgnDb::StartCreateChangeset),
            InstanceMethod("startProfiler", &NativeDgnDb::StartProfiler),
            InstanceMethod("stopProfiler", &NativeDgnDb::StopProfiler),
            InstanceMethod("updateElement", &NativeDgnDb::UpdateElement),
            InstanceMethod("updateElementAspect", &NativeDgnDb::UpdateElementAspect),
            InstanceMethod("updateElementGeometryCache", &NativeDgnDb::UpdateElementGeometryCache),
            InstanceMethod("updateIModelProps", &NativeDgnDb::UpdateIModelProps),
            InstanceMethod("updateLinkTableRelationship", &NativeDgnDb::UpdateLinkTableRelationship),
            InstanceMethod("updateModel", &NativeDgnDb::UpdateModel),
            InstanceMethod("updateModelGeometryGuid", &NativeDgnDb::UpdateModelGeometryGuid),
            InstanceMethod("updateProjectExtents", &NativeDgnDb::UpdateProjectExtents),
            InstanceMethod("writeAffectedElementDependencyGraphToFile", &NativeDgnDb::WriteAffectedElementDependencyGraphToFile),
            InstanceMethod("writeFullElementDependencyGraphToFile", &NativeDgnDb::WriteFullElementDependencyGraphToFile),
            InstanceMethod("vacuum", &NativeDgnDb::Vacuum),
            InstanceMethod("enableWalMode", &NativeDgnDb::EnableWalMode),
            InstanceMethod("performCheckpoint", &NativeDgnDb::PerformCheckpoint),
            InstanceMethod("setAutoCheckpointThreshold", &NativeDgnDb::SetAutoCheckpointThreshold),
            StaticMethod("enableSharedCache", &NativeDgnDb::EnableSharedCache),
            StaticMethod("getAssetsDir", &NativeDgnDb::GetAssetDir),
        });

        exports.Set("DgnDb", t);

        SET_CONSTRUCTOR(t);
        }
};

//=======================================================================================
// Projects the GeoServices class into JS
//! @bsiclass
//=======================================================================================
struct NativeGeoServices : BeObjectWrap<NativeGeoServices>
    {
    private:
      DEFINE_CONSTRUCTOR;

    public:
    NativeGeoServices(NapiInfoCR info) : BeObjectWrap<NativeGeoServices>(info) {}
    ~NativeGeoServices() {}

    //  Check if val is really a GeoServices peer object
    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    static Napi::Value GetGeographicCRSInterpretation(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, interpretGCSProps);
        BeJsNapiObject results(info.Env());
        GeoServicesInterop::GetGeographicCRSInterpretation(results, interpretGCSProps);
        return results;
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "GeoServices", {
            StaticMethod("getGeographicCRSInterpretation", &NativeGeoServices::GetGeographicCRSInterpretation)
        });

        exports.Set("GeoServices", t);

        SET_CONSTRUCTOR(t);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb* extractDgnDbFromNapiValue(Napi::Value value)
    {
    auto nativeDb = Napi::ObjectWrap<NativeDgnDb>::Unwrap(value.As<Napi::Object>());
    return nullptr != nativeDb && nativeDb->IsOpen() ? &nativeDb->GetDgnDb() : nullptr;
    }

//=======================================================================================
//  RevisionUtility class into JS
//! @bsiclass
//=======================================================================================
struct NativeRevisionUtility : BeObjectWrap<NativeRevisionUtility>
    {
    private:
        DEFINE_CONSTRUCTOR

    public:
        NativeRevisionUtility(NapiInfoCR info) : BeObjectWrap<NativeRevisionUtility>(info) {}
        ~NativeRevisionUtility() {SetInDestructor();}

    // Check if val is really a NativeRevisionUtility peer object
    static bool InstanceOf(Napi::Value val) {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
    }

    static Napi::Value RecompressRevision(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, sourceChangeSetFile);
        REQUIRE_ARGUMENT_STRING(1, targetChangeSetFile);
        OPTIONAL_ARGUMENT_STRING(2, lzmaProperties);
        LzmaEncoder::LzmaParams params;
        if (!lzmaProperties.empty())
            params.FromJson(BeJsDocument(lzmaProperties));

        BentleyStatus status = RevisionUtility::RecompressRevision(sourceChangeSetFile.c_str(), targetChangeSetFile.c_str(), params);
        return Napi::Number::New(info.Env(), (int)status);
        }
    static Napi::Value DisassembleRevision(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, sourceFile);
        REQUIRE_ARGUMENT_STRING(1, targetDir);
        BentleyStatus status = RevisionUtility::DisassembleRevision(sourceFile.c_str(), targetDir.c_str());
        return Napi::Number::New(info.Env(), (int)status);
        }
    static Napi::Value AssembleRevision(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, outputChangesetFile);
        REQUIRE_ARGUMENT_STRING(1, rawChangesetFile);
        OPTIONAL_ARGUMENT_STRING(2, prefixFile);
        OPTIONAL_ARGUMENT_STRING(3, lzmaProperties);
        LzmaEncoder::LzmaParams params;
        if (!lzmaProperties.empty())
            params.FromJson(BeJsDocument(lzmaProperties));

        BentleyStatus status = RevisionUtility::AssembleRevision(prefixFile.c_str(), rawChangesetFile.c_str(), outputChangesetFile.c_str(), params);
        return Napi::Number::New(info.Env(), (int)status);
        }
    static Napi::Value NormalizeLzmaParams(NapiInfoCR info)
        {
        OPTIONAL_ARGUMENT_STRING(0, lzmaProperties);
        LzmaEncoder::LzmaParams params;
        if (!lzmaProperties.empty())
            params.FromJson(BeJsDocument(lzmaProperties));

        BeJsDocument out;
        params.ToJson(out);
        return Napi::String::New(info.Env(), out.Stringify().c_str());
        }
    static Napi::Value ComputeStatistics(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, changesetFile);
        REQUIRE_ARGUMENT_BOOL(1, addPrefix);
        BeJsDocument out;
        if (SUCCESS != RevisionUtility::ComputeStatistics(changesetFile.c_str(), addPrefix, out))
            THROW_JS_EXCEPTION("Failed to compute statistics");

        return Napi::String::New(info.Env(), out.Stringify().c_str());
        }
    static Napi::Value GetUncompressSize(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, changesetFile);
        uint32_t compressSize, uncompressSize, prefixSize;
        if (SUCCESS != RevisionUtility::GetUncompressSize(changesetFile.c_str(), compressSize, uncompressSize, prefixSize))
            THROW_JS_EXCEPTION("Failed to get uncompress size");

        BeJsDocument out;
        out["compressSize"] = compressSize;
        out["uncompressSize"] = uncompressSize;
        out["prefixSize"] = prefixSize;
        return Napi::String::New(info.Env(), out.Stringify().c_str());
        }
    static Napi::Value DumpChangesetToDb(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, changesetFile);
        REQUIRE_ARGUMENT_STRING(1, sqliteFile);
        REQUIRE_ARGUMENT_BOOL(2, includeCols);

        BentleyStatus status = RevisionUtility::DumpChangesetToDb(changesetFile.c_str(), sqliteFile.c_str(), includeCols);
        return Napi::Number::New(info.Env(), (int)status);
        }
    static void Init(Napi::Env env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "RevisionUtility", {
            StaticMethod("recompressRevision", &NativeRevisionUtility::RecompressRevision),
            StaticMethod("disassembleRevision", &NativeRevisionUtility::DisassembleRevision),
            StaticMethod("assembleRevision", &NativeRevisionUtility::AssembleRevision),
            StaticMethod("normalizeLzmaParams", &NativeRevisionUtility::NormalizeLzmaParams),
            StaticMethod("computeStatistics", &NativeRevisionUtility::ComputeStatistics),
            StaticMethod("getUncompressSize", &NativeRevisionUtility::GetUncompressSize),
            StaticMethod("dumpChangesetToDb", &NativeRevisionUtility::DumpChangesetToDb),
        });

        exports.Set("RevisionUtility", t);

        SET_CONSTRUCTOR(t)
        }
    };

//=======================================================================================
// Projects the Changed Elements ECDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeChangedElementsECDb : BeObjectWrap<NativeChangedElementsECDb>
    {
    private:
        DEFINE_CONSTRUCTOR;
        ECDb m_ecdb;
        std::unique_ptr<ChangedElementsManager> m_manager;

        template <typename STATUSTYPE>
        Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg = nullptr) { return IModelJsNative::CreateBentleyReturnErrorObject(errCode, msg, Env()); }

        template <typename STATUSTYPE>
        Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode, Napi::Value goodValue)   {
            if ((STATUSTYPE)0 != errCode)
                return CreateBentleyReturnErrorObject(errCode);
            return CreateBentleyReturnSuccessObject(goodValue);
        }

      public:
        NativeChangedElementsECDb(NapiInfoCR info) : BeObjectWrap<NativeChangedElementsECDb>(info) {}
        ~NativeChangedElementsECDb() {SetInDestructor(); CloseDbIfOpen(); }

        // Check if val is really a NativeECDb peer object
        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(Constructor().Value());
            }
        ECDbR GetECDb() { return m_ecdb; }

        Napi::Value CreateDb(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, mainDb);
            REQUIRE_ARGUMENT_STRING(1, dbName);
            m_manager = std::make_unique<ChangedElementsManager>(&mainDb->GetDgnDb());
            BeFileName file(dbName);
            BeFileName path = file.GetDirectoryName();
            if (!path.DoesPathExist())
                return Napi::Number::New(Env(), (int) BE_SQLITE_NOTFOUND);

            DbResult status = m_manager->CreateChangedElementsCache(GetECDb(), file);
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value CleanCaches(NapiInfoCR info)
            {
            m_manager = nullptr;
            return Napi::Number::New(Env(), 0);
            }

        Napi::Value ProcessChangesets(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, mainDb);
            REQUIRE_ARGUMENT_ANY_OBJ(1, changeSets);
            REQUIRE_ARGUMENT_STRING(2, rulesetId);
            OPTIONAL_ARGUMENT_BOOL(3, filterSpatial, false);
            OPTIONAL_ARGUMENT_BOOL(4, wantParents, false);
            OPTIONAL_ARGUMENT_BOOL(5, wantPropertyChecksums, true);
            OPTIONAL_ARGUMENT_STRING(6, rulesetDir);
            OPTIONAL_ARGUMENT_STRING(7, tempDir);
            OPTIONAL_ARGUMENT_BOOL(8, wantChunkTraversal, false);

            if (GetECDb().IsReadonly())
                return Napi::Number::New(Env(), (int) BE_SQLITE_READONLY);

            if (!m_manager)
                m_manager = std::make_unique<ChangedElementsManager>(&mainDb->GetDgnDb());

            bool containsSchemaChanges;
            Utf8String dbGuid = mainDb->GetDgnDb().GetDbGuid().ToString();
            auto revisionPtrs = JsInterop::GetRevisions(containsSchemaChanges, dbGuid, changeSets);

            m_manager->SetFilterSpatial(filterSpatial);
            m_manager->SetWantParents(wantParents);
            m_manager->SetWantPropertyChecksums(wantPropertyChecksums);
            m_manager->SetWantChunkTraversal(wantChunkTraversal);

            if (!rulesetDir.empty())
                m_manager->SetPresentationRulesetDirectory(rulesetDir);
            if (!tempDir.empty())
                m_manager->SetTempLocation(tempDir.c_str());

            DbResult result = m_manager->ProcessChangesets(GetECDb(), Utf8String(rulesetId), revisionPtrs);
            return Napi::Number::New(Env(), (int) result);
            }

        Napi::Value ProcessChangesetsAndRoll(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbFilename);
            REQUIRE_ARGUMENT_STRING(1, dbGuid)
            REQUIRE_ARGUMENT_ANY_OBJ(2, changeSets);
            REQUIRE_ARGUMENT_STRING(3, rulesetId);
            OPTIONAL_ARGUMENT_BOOL(4, filterSpatial, false);
            OPTIONAL_ARGUMENT_BOOL(5, wantParents, false);
            OPTIONAL_ARGUMENT_BOOL(6, wantPropertyChecksums, true);
            OPTIONAL_ARGUMENT_STRING(7, rulesetDir);
            OPTIONAL_ARGUMENT_STRING(8, tempDir);
            OPTIONAL_ARGUMENT_BOOL(9, wantRelationshipCaching, true);
            OPTIONAL_ARGUMENT_INTEGER(10, relationshipCacheSize, 200000);
            OPTIONAL_ARGUMENT_BOOL(11, wantChunkTraversal, false);

            if (GetECDb().IsReadonly())
                return Napi::Number::New(Env(), (int) BE_SQLITE_READONLY);

            if (!m_manager)
                m_manager = std::make_unique<ChangedElementsManager>(BeFileName(dbFilename));

            bool containsSchemaChanges;
            auto revisionPtrs = JsInterop::GetRevisions(containsSchemaChanges, dbGuid, changeSets);

            m_manager->SetFilterSpatial(filterSpatial);
            m_manager->SetWantParents(wantParents);
            m_manager->SetWantPropertyChecksums(wantPropertyChecksums);
            m_manager->SetWantBriefcaseRoll(true);
            m_manager->SetWantRelationshipCaching(wantRelationshipCaching);
            m_manager->SetRelationshipCacheSize(relationshipCacheSize);
            m_manager->SetWantChunkTraversal(wantChunkTraversal);

            if (!rulesetDir.empty())
                m_manager->SetPresentationRulesetDirectory(rulesetDir);
            if (!tempDir.empty())
                m_manager->SetTempLocation(tempDir.c_str());

            DbResult result = m_manager->ProcessChangesets(GetECDb(), Utf8String(rulesetId), revisionPtrs);
            return Napi::Number::New(Env(), (int) result);
            }

        Napi::Value IsProcessed(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_STRING(0, changesetId);
            BeJsDocument response;
            bool isProcessed = m_manager->IsProcessed(GetECDb(), changesetId);
            return Napi::Boolean::New(Env(), isProcessed);
            }

        Napi::Value GetChangedElements(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_STRING(0, startChangesetId);
            REQUIRE_ARGUMENT_STRING(1, endChangesetId);

            ChangedElementsMap map;
            DbResult status = m_manager->GetChangedElements(GetECDb(), map, startChangesetId, endChangesetId);

            BeJsNapiObject jsValue(Env());
            ChangedElementsManager::ChangedElementsToJSON(jsValue, map);
            return CreateBentleyReturnObject(status, jsValue);
            }

        Napi::Value IsOpen(NapiInfoCR info) { return Napi::Boolean::New(Env(), GetECDb().IsDbOpen()); }

        Napi::Value OpenDb(NapiInfoCR info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbName);
            REQUIRE_ARGUMENT_INTEGER(1, mode);

            Db::OpenParams params((Db::OpenMode) mode);
            DbResult status = JsInterop::OpenECDb(GetECDb(), BeFileName(dbName.c_str(), true), params);
            return Napi::Number::New(Env(), (int) status);
            }

        void CloseDbIfOpen() {
            if (m_ecdb.IsDbOpen())
                m_ecdb.CloseDb();
        }

        Napi::Value CloseDb(NapiInfoCR info) {
            CloseDbIfOpen();
            return Napi::Number::New(Env(), (int) BE_SQLITE_OK);
        }

        static void Init(Napi::Env env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ChangedElementsECDb", {
            InstanceMethod("createDb", &NativeChangedElementsECDb::CreateDb),
            InstanceMethod("processChangesets", &NativeChangedElementsECDb::ProcessChangesets),
            InstanceMethod("processChangesetsAndRoll", &NativeChangedElementsECDb::ProcessChangesetsAndRoll),
            InstanceMethod("getChangedElements", &NativeChangedElementsECDb::GetChangedElements),
            InstanceMethod("openDb", &NativeChangedElementsECDb::OpenDb),
            InstanceMethod("closeDb", &NativeChangedElementsECDb::CloseDb),
            InstanceMethod("isOpen", &NativeChangedElementsECDb::IsOpen),
            InstanceMethod("isProcessed", &NativeChangedElementsECDb::IsProcessed),
            InstanceMethod("cleanCaches", &NativeChangedElementsECDb::CleanCaches)
            });

            exports.Set("ChangedElementsECDb", t);

            SET_CONSTRUCTOR(t);
            }
    };

//=======================================================================================
// Projects the IECSqlBinder interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlBinder : BeObjectWrap<NativeECSqlBinder>
    {
private:
    DEFINE_CONSTRUCTOR;
    IECSqlBinder* m_binder = nullptr;
    ECDb const* m_ecdb = nullptr;

    static DbResult ToDbResult(ECSqlStatus status)
        {
        if (status.IsSuccess())
            return BE_SQLITE_OK;

        if (status.IsSQLiteError())
            return status.GetSQLiteError();

        return BE_SQLITE_ERROR;
        }

public:
    NativeECSqlBinder(NapiInfoCR info) : BeObjectWrap<NativeECSqlBinder>(info)
        {
        if (info.Length() != 2)
            THROW_JS_EXCEPTION("ECSqlBinder constructor expects two arguments.");

        m_binder = info[0].As<Napi::External<IECSqlBinder>>().Data();
        if (m_binder == nullptr)
            THROW_JS_TYPE_EXCEPTION("Invalid first arg for NativeECSqlBinder constructor. IECSqlBinder must not be nullptr");

        m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
        if (m_ecdb == nullptr)
            THROW_JS_TYPE_EXCEPTION("Invalid second arg for NativeECSqlBinder constructor. ECDb must not be nullptr");
        }

    ~NativeECSqlBinder() {SetInDestructor();}

    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECSqlBinder", {
            InstanceMethod("addArrayElement", &NativeECSqlBinder::AddArrayElement),
            InstanceMethod("bindBlob", &NativeECSqlBinder::BindBlob),
            InstanceMethod("bindBoolean", &NativeECSqlBinder::BindBoolean),
            InstanceMethod("bindDateTime", &NativeECSqlBinder::BindDateTime),
            InstanceMethod("bindDouble", &NativeECSqlBinder::BindDouble),
            InstanceMethod("bindGuid", &NativeECSqlBinder::BindGuid),
            InstanceMethod("bindId", &NativeECSqlBinder::BindId),
            InstanceMethod("bindIdSet", &NativeECSqlBinder::BindIdSet),
            InstanceMethod("bindInteger", &NativeECSqlBinder::BindInteger),
            InstanceMethod("bindMember", &NativeECSqlBinder::BindMember),
            InstanceMethod("bindNavigation", &NativeECSqlBinder::BindNavigation),
            InstanceMethod("bindNull", &NativeECSqlBinder::BindNull),
            InstanceMethod("bindPoint2d", &NativeECSqlBinder::BindPoint2d),
            InstanceMethod("bindPoint3d", &NativeECSqlBinder::BindPoint3d),
            InstanceMethod("bindString", &NativeECSqlBinder::BindString),
        });

        exports.Set("ECSqlBinder", t);

        SET_CONSTRUCTOR(t);
        }

    static Napi::Object New(Napi::Env const& env, IECSqlBinder& binder, ECDbCR ecdb)
        {
        return Constructor().New({Napi::External<IECSqlBinder>::New(env, &binder), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
        }

    Napi::Value BindNull(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        ECSqlStatus stat = m_binder->BindNull();
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBlob(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        if (info.Length() == 0)
            THROW_JS_EXCEPTION("BindBlob requires an argument");

        Napi::Value blobVal = info[0];
        if (blobVal.IsTypedArray())
            {
            Napi::TypedArray typedArray = blobVal.As<Napi::TypedArray>();
            if (typedArray.TypedArrayType() == napi_uint8_array)
                {
                Napi::Uint8Array uint8Array = typedArray.As<Napi::Uint8Array>();
                const auto length = (int)uint8Array.ByteLength();
                ECSqlStatus stat = m_binder->BindBlob(length > 0 ? uint8Array.Data(): (void*)"", length, IECSqlBinder::MakeCopy::Yes);
                return Napi::Number::New(Env(), (int) ToDbResult(stat));
                }
            }

        if (blobVal.IsArrayBuffer())
            {
            Napi::ArrayBuffer buf = blobVal.As<Napi::ArrayBuffer>();
            const auto length = (int) buf.ByteLength();
            ECSqlStatus stat = m_binder->BindBlob(length > 0 ? buf.Data(): (void*)"", length, IECSqlBinder::MakeCopy::Yes);
            return Napi::Number::New(Env(), (int) ToDbResult(stat));
            }

        if (!blobVal.IsString())
            THROW_JS_TYPE_EXCEPTION("BindBlob requires either a Uint8Buffer, ArrayBuffer or a base64-encoded string.");

        Utf8String base64Str(blobVal.ToString().Utf8Value().c_str());
        ByteStream blob;
        Base64Utilities::Decode(blob, base64Str);

        ECSqlStatus stat = m_binder->BindBlob(blob.data(), (int) blob.size(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBoolean(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        Napi::Value boolVal;
        if (info.Length() == 0 || !(boolVal = info[0]).IsBoolean())
            THROW_JS_TYPE_EXCEPTION("BindBoolean expects a boolean");

        ECSqlStatus stat = m_binder->BindBoolean(boolVal.ToBoolean());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDateTime(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_STRING(0, isoString);

        DateTime dt = DateTime::FromString(isoString);
        if (!dt.IsValid())
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = m_binder->BindDateTime(dt);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDouble(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_NUMBER(0, val);
        ECSqlStatus stat = m_binder->BindDouble(val.DoubleValue());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindGuid(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_STRING(0, guidString);
        BeGuid guid;
        if (SUCCESS != guid.FromString(guidString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = m_binder->BindGuid(guid, IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindId(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_STRING(0, hexString);
        BeInt64Id id;
        if (SUCCESS != BeInt64Id::FromString(id, hexString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = m_binder->BindId(id);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindIdSet(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");
        if (info.Length() == 0)
            THROW_JS_TYPE_EXCEPTION("BindVirtualSet requires an argument");

        REQUIRE_ARGUMENT_STRING_ARRAY(0, hexVector);
        std::shared_ptr<IdSet<BeInt64Id>> idSet = std::make_shared<IdSet<BeInt64Id>>();
        for (Utf8String hexString : hexVector)
        {
            BeInt64Id id;
            if (SUCCESS != BeInt64Id::FromString(id, hexString.c_str()))
                return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);
            idSet->insert(id);
        }
        ECSqlStatus stat = m_binder->BindVirtualSet(idSet);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindInteger(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        if (info.Length() == 0)
            THROW_JS_TYPE_EXCEPTION("BindInteger expects a string or number");

        Napi::Value val = info[0];
        if (!val.IsNumber() && !val.IsString())
            THROW_JS_TYPE_EXCEPTION("BindInteger expects a string or number");

        int64_t int64Val;
        if (val.IsNumber())
            int64Val = val.ToNumber().Int64Value();
        else
            {
            Utf8String strVal(val.ToString().Utf8Value().c_str());
            if (strVal.empty())
                THROW_JS_TYPE_EXCEPTION("Integral string passed to BindInteger must not be empty.");

            bool const isNegativeNumber = strVal[0] == '-';
            Utf8CP positiveNumberStr = isNegativeNumber ? strVal.c_str() + 1 : strVal.c_str();
            uint64_t uVal = 0;
            if (SUCCESS != BeStringUtilities::ParseUInt64(uVal, positiveNumberStr)) //also supports hex strings
                {
                Utf8String error;
                error.Sprintf("BindInteger failed. Could not parse string %s to a valid integer.", strVal.c_str());
                THROW_JS_TYPE_EXCEPTION(error.c_str());
                }

            if (isNegativeNumber && uVal > (uint64_t) std::numeric_limits<int64_t>::max())
                {
                Utf8String error;
                error.Sprintf("BindInteger failed. Number in string %s is too large to fit into a signed 64 bit integer value.", strVal.c_str());
                THROW_JS_TYPE_EXCEPTION(error.c_str());
                }

            int64Val = uVal;
            if (isNegativeNumber)
                int64Val *= -1;
            }

        ECSqlStatus stat = m_binder->BindInt64(int64Val);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint2d(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_NUMBER(0, x);
        REQUIRE_ARGUMENT_NUMBER(1, y);
        ECSqlStatus stat = m_binder->BindPoint2d(DPoint2d::From(x.DoubleValue(),y.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint3d(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_NUMBER(0, x);
        REQUIRE_ARGUMENT_NUMBER(1, y);
        REQUIRE_ARGUMENT_NUMBER(2, z);
        ECSqlStatus stat = m_binder->BindPoint3d(DPoint3d::From(x.DoubleValue(), y.DoubleValue(), z.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindString(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_STRING(0, val);
        ECSqlStatus stat = m_binder->BindText(val.c_str(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindNavigation(NapiInfoCR info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_STRING(0, navIdHexStr);
        OPTIONAL_ARGUMENT_STRING(1, relClassName);
        OPTIONAL_ARGUMENT_STRING(2, relClassTableSpaceName);

        BeInt64Id navId;
        if (SUCCESS != BeInt64Id::FromString(navId, navIdHexStr.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECClassId relClassId;
        if (!relClassName.empty())
            {
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(relClassName.c_str(), ".:", tokens);
            if (tokens.size() != 2)
                return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

            relClassId = m_ecdb->Schemas().GetClassId(tokens[0], tokens[1], SchemaLookupMode::AutoDetect, relClassTableSpaceName.c_str());
            }

        ECSqlStatus stat = m_binder->BindNavigation(navId, relClassId);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindMember(NapiInfoCR info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        REQUIRE_ARGUMENT_STRING(0, memberName);
        IECSqlBinder& memberBinder = m_binder->operator[](memberName.c_str());
        return New(info.Env(), memberBinder, *m_ecdb);
        }

    Napi::Value AddArrayElement(NapiInfoCR info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_JS_EXCEPTION("ECSqlBinder is not initialized.");

        IECSqlBinder& elementBinder = m_binder->AddArrayElement();
        return New(info.Env(), elementBinder, *m_ecdb);
        }
    };

//=======================================================================================
// Projects the NativeECSqlColumnInfo interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlColumnInfo : BeObjectWrap<NativeECSqlColumnInfo>
    {
    private:
        //Must match ECSqlValueType in imodeljs-core
        enum class Type
            {
            Blob = 1,
            Boolean = 2,
            DateTime = 3,
            Double = 4,
            Geometry = 5,
            Id = 6,
            Int = 7,
            Int64 = 8,
            Point2d = 9,
            Point3d = 10,
            String = 11,
            Navigation = 12,
            Struct = 13,
            PrimitiveArray = 14,
            StructArray = 15,
            Guid = 16
            };

        DEFINE_CONSTRUCTOR;
        ECSqlColumnInfo const* m_colInfo = nullptr;

    public:
        NativeECSqlColumnInfo(NapiInfoCR info) : BeObjectWrap<NativeECSqlColumnInfo>(info)
            {
            if (info.Length() != 1)
                THROW_JS_TYPE_EXCEPTION("ECSqlColumnInfo constructor expects one argument.");

            m_colInfo = info[0].As<Napi::External<ECSqlColumnInfo>>().Data();
            if (m_colInfo == nullptr)
                THROW_JS_TYPE_EXCEPTION("Invalid first arg for NativeECSqlColumnInfo constructor. ECSqlColumnInfo must not be nullptr");
            }

        ~NativeECSqlColumnInfo() {SetInDestructor();}

        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(Constructor().Value());
            }

        //  Create projections
        static void Init(Napi::Env& env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ECSqlColumnInfo", {
            InstanceMethod("getType", &NativeECSqlColumnInfo::GetType),
            InstanceMethod("getPropertyName", &NativeECSqlColumnInfo::GetPropertyName),
            InstanceMethod("getOriginPropertyName", &NativeECSqlColumnInfo::GetOriginPropertyName),
            InstanceMethod("getAccessString", &NativeECSqlColumnInfo::GetAccessString),
            InstanceMethod("isSystemProperty", &NativeECSqlColumnInfo::IsSystemProperty),
            InstanceMethod("isGeneratedProperty", &NativeECSqlColumnInfo::IsGeneratedProperty),
            InstanceMethod("isEnum", &NativeECSqlColumnInfo::IsEnum),
            InstanceMethod("getRootClassTableSpace", &NativeECSqlColumnInfo::GetRootClassTableSpace),
            InstanceMethod("getRootClassName", &NativeECSqlColumnInfo::GetRootClassName),
            InstanceMethod("getRootClassAlias", &NativeECSqlColumnInfo::GetRootClassAlias)});

            exports.Set("ECSqlColumnInfo", t);

            SET_CONSTRUCTOR(t);
            }

        static Napi::Object New(Napi::Env const& env, ECSqlColumnInfo const& colInfo)
            {
            return Constructor().New({Napi::External<ECSqlColumnInfo>::New(env, const_cast<ECSqlColumnInfo*>(&colInfo))});
            }

        Napi::Value GetType(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            ECTypeDescriptor const& dataType = m_colInfo->GetDataType();
            Type type = Type::Id;
            if (dataType.IsNavigation())
                type = Type::Navigation;
            else if (dataType.IsStruct())
                type = Type::Struct;
            else if (dataType.IsPrimitiveArray())
                type = Type::PrimitiveArray;
            else if (dataType.IsStructArray())
                type = Type::StructArray;
            else
                {
                BeAssert(dataType.IsPrimitive());
                switch (dataType.GetPrimitiveType())
                    {
                        case PRIMITIVETYPE_Binary:
                        {
                        ECPropertyCP prop = m_colInfo->GetProperty();
                        if (prop && prop->HasExtendedType())
                            {
                            BeAssert(prop->GetIsPrimitive());
                            Utf8StringCR extendedTypeName = prop->GetAsPrimitiveProperty()->GetExtendedTypeName();
                            if (extendedTypeName.EqualsIAscii("Guid") || extendedTypeName.EqualsIAscii("BeGuid"))
                                {
                                type = Type::Guid;
                                break;
                                }
                            }

                        type = Type::Blob;
                        break;
                        }
                        case PRIMITIVETYPE_Boolean:
                            type = Type::Boolean;
                            break;
                        case PRIMITIVETYPE_DateTime:
                            type = Type::DateTime;
                            break;
                        case PRIMITIVETYPE_Double:
                            type = Type::Double;
                            break;
                        case PRIMITIVETYPE_IGeometry:
                            type = Type::Geometry;
                            break;
                        case PRIMITIVETYPE_Integer:
                            type = Type::Int;
                            break;
                        case PRIMITIVETYPE_Long:
                        {
                        if (m_colInfo->IsSystemProperty())
                            {
                            type = Type::Id;
                            break;
                            }

                        ECPropertyCP prop = m_colInfo->GetProperty();
                        if (prop && prop->HasExtendedType())
                            {
                            BeAssert(prop->GetIsPrimitive());
                            if (prop->GetAsPrimitiveProperty()->GetExtendedTypeName().EndsWith("Id"))
                                {
                                type = Type::Id;
                                break;
                                }
                            }

                        type = Type::Int64;
                        break;
                        }
                        case PRIMITIVETYPE_Point2d:
                            type = Type::Point2d;
                            break;
                        case PRIMITIVETYPE_Point3d:
                            type = Type::Point3d;
                            break;
                        case PRIMITIVETYPE_String:
                            type = Type::String;
                            break;
                        default:
                            THROW_JS_EXCEPTION("Unsupported ECSqlValue primitive type.");
                            break;
                    }
                }

            return Napi::Number::New(Env(), (int) type);
            }

        Napi::Value GetPropertyName(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            ECPropertyCP prop = m_colInfo->GetProperty();
            if (prop == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo does not represent a property.");

            return toJsString(Env(), prop->GetName());
            }

        Napi::Value GetOriginPropertyName(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            ECPropertyCP prop = m_colInfo->GetOriginProperty();
            if (prop == nullptr)
                return Env().Undefined();

            return toJsString(Env(), prop->GetName());
            }

        Napi::Value GetAccessString(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            //if property is generated, the display label contains the select clause item as is.
            //The property name in contrast would have encoded special characters of the select clause item.
            //Ex: SELECT MyProp + 4 FROM Foo -> the name must be "MyProp + 4"
            if (m_colInfo->IsGeneratedProperty())
                {
                BeAssert(m_colInfo->GetPropertyPath().Size() == 1);
                ECPropertyCP prop = m_colInfo->GetProperty();
                if (prop == nullptr)
                    THROW_JS_EXCEPTION("ECSqlColumnInfo's Property must not be null for a generated property.");

                return toJsString(Env(), prop->GetDisplayLabel());
                }

            return toJsString(Env(), m_colInfo->GetPropertyPath().ToString());
            }

        Napi::Value IsEnum(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            return Napi::Boolean::New(Env(), m_colInfo->GetEnumType() != nullptr);
            }

        Napi::Value IsSystemProperty(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            return Napi::Boolean::New(Env(), m_colInfo->IsSystemProperty());
            }

        Napi::Value IsGeneratedProperty(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            return Napi::Boolean::New(Env(), m_colInfo->IsGeneratedProperty());
            }

        Napi::Value GetRootClassTableSpace(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            return toJsString(Env(), m_colInfo->GetRootClass().GetTableSpace());
            }

        Napi::Value GetRootClassName(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            return toJsString(Env(), ECJsonUtilities::FormatClassName(m_colInfo->GetRootClass().GetClass()));
            }

        Napi::Value GetRootClassAlias(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_EXCEPTION("ECSqlColumnInfo is not initialized.");

            return toJsString(Env(), m_colInfo->GetRootClass().GetAlias());
            }
    };

//=======================================================================================
// Projects the IECSqlValue interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlValue : BeObjectWrap<NativeECSqlValue>
    {
private:
    DEFINE_CONSTRUCTOR;
    IECSqlValue const* m_ecsqlValue = nullptr;
    ECDb const* m_ecdb = nullptr;

public:
    NativeECSqlValue(NapiInfoCR info) : BeObjectWrap<NativeECSqlValue>(info)
        {
        if (info.Length() < 2)
            THROW_JS_TYPE_EXCEPTION("ECSqlValue constructor expects two arguments.");

        m_ecsqlValue = info[0].As<Napi::External<IECSqlValue>>().Data();
        if (m_ecsqlValue == nullptr)
            THROW_JS_TYPE_EXCEPTION("Invalid first arg for NativeECSqlValue constructor. IECSqlValue must not be nullptr");

        m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
        if (m_ecdb == nullptr)
            THROW_JS_TYPE_EXCEPTION("Invalid second arg for NativeECSqlValue constructor. ECDb must not be nullptr");
        }

    ~NativeECSqlValue() {SetInDestructor();}

    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECSqlValue", {
            InstanceMethod("getArrayIterator", &NativeECSqlValue::GetArrayIterator),
            InstanceMethod("getBlob", &NativeECSqlValue::GetBlob),
            InstanceMethod("getBoolean", &NativeECSqlValue::GetBoolean),
            InstanceMethod("getClassNameForClassId", &NativeECSqlValue::GetClassNameForClassId),
            InstanceMethod("getColumnInfo", &NativeECSqlValue::GetColumnInfo),
            InstanceMethod("getDateTime", &NativeECSqlValue::GetDateTime),
            InstanceMethod("getDouble", &NativeECSqlValue::GetDouble),
            InstanceMethod("getGeometry", &NativeECSqlValue::GetGeometry),
            InstanceMethod("getGuid", &NativeECSqlValue::GetGuid),
            InstanceMethod("getId", &NativeECSqlValue::GetId),
            InstanceMethod("getInt", &NativeECSqlValue::GetInt),
            InstanceMethod("getInt64", &NativeECSqlValue::GetInt64),
            InstanceMethod("getNavigation", &NativeECSqlValue::GetNavigation),
            InstanceMethod("getPoint2d", &NativeECSqlValue::GetPoint2d),
            InstanceMethod("getPoint3d", &NativeECSqlValue::GetPoint3d),
            InstanceMethod("getString", &NativeECSqlValue::GetString),
            InstanceMethod("getEnum", &NativeECSqlValue::GetEnum),
            InstanceMethod("getStructIterator", &NativeECSqlValue::GetStructIterator),
            InstanceMethod("isNull", &NativeECSqlValue::IsNull),
        });

        exports.Set("ECSqlValue", t);

        SET_CONSTRUCTOR(t);
        }

    static Napi::Object New(Napi::Env const& env, IECSqlValue const& val, ECDbCR ecdb)
        {
        return Constructor().New({Napi::External<IECSqlValue>::New(env, const_cast<IECSqlValue*>(&val)), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
        }

    Napi::Value GetColumnInfo(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        return NativeECSqlColumnInfo::New(Env(), m_ecsqlValue->GetColumnInfo());
        }

    Napi::Value IsNull(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        return Napi::Boolean::New(Env(), m_ecsqlValue->IsNull());
        }

    Napi::Value GetBlob(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        int blobSize;
        void const* data = m_ecsqlValue->GetBlob(&blobSize);
        auto blob = Napi::Uint8Array::New(Env(), blobSize);
        memcpy(blob.Data(), data, blobSize);
        return blob;
        }

    Napi::Value GetBoolean(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        return Napi::Boolean::New(Env(), m_ecsqlValue->GetBoolean());
        }

    Napi::Value GetDateTime(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        DateTime dt = m_ecsqlValue->GetDateTime();
        return toJsString(Env(), dt.ToString());
        }

    Napi::Value GetDouble(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        return Napi::Number::New(Env(), m_ecsqlValue->GetDouble());
        }

    Napi::Value GetGeometry(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        IGeometryPtr geom = m_ecsqlValue->GetGeometry();
        BeJsDocument json;
        if (geom == nullptr || SUCCESS != ECJsonUtilities::IGeometryToIModelJson(json, *geom))
            THROW_JS_EXCEPTION("Could not convert IGeometry to JSON.");

        return toJsString(Env(), json.Stringify());
        }

    Napi::Value GetGuid(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        BeGuid guid = m_ecsqlValue->GetGuid();
        if (!guid.IsValid())
            return Env().Undefined();

        return toJsString(Env(), guid.ToString());
        }

    Napi::Value GetId(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        BeInt64Id id = m_ecsqlValue->GetId<BeInt64Id>();
        if (!id.IsValid())
            return Env().Undefined();

        return toJsString(Env(), id);
        }

    Napi::Value GetClassNameForClassId(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        ECClassId classId = m_ecsqlValue->GetId<ECClassId>();
        if (!classId.IsValid())
            THROW_JS_EXCEPTION("Failed to get class name from ECSqlValue: The ECSqlValue does not refer to a valid class id.");

        Utf8StringCR tableSpace = m_ecsqlValue->GetColumnInfo().GetRootClass().GetTableSpace();
        ECClassCP ecClass = m_ecdb->Schemas().GetClass(classId, tableSpace.c_str());
        if (ecClass == nullptr)
            {
            Utf8String err;
            err.Sprintf("Failed to get class name from ECSqlValue: Class not found for ECClassId %s.", classId.ToHexStr().c_str());
            THROW_JS_EXCEPTION(err.c_str());
            }

        return toJsString(Env(), ECJsonUtilities::FormatClassName(*ecClass));
        }

    Napi::Value GetInt(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        return Napi::Number::New(Env(), m_ecsqlValue->GetInt());
        }

    Napi::Value GetInt64(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        return Napi::Number::New(Env(), m_ecsqlValue->GetInt64());
        }

    Napi::Value GetPoint2d(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        DPoint2d pt = m_ecsqlValue->GetPoint2d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        return jsPt;
        }

    Napi::Value GetPoint3d(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        DPoint3d pt = m_ecsqlValue->GetPoint3d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Z(), Napi::Number::New(Env(), pt.z));
        return jsPt;
        }

    Napi::Value GetString(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        return toJsString(Env(), m_ecsqlValue->IsNull() ? "" : m_ecsqlValue->GetText());
        }

    Napi::Value GetEnum(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        ECEnumerationCP enumType = m_ecsqlValue->GetColumnInfo().GetEnumType();
        if (enumType == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not an ECEnumeration.");

        bvector<ECEnumeratorCP> enumerators;
        if (SUCCESS != m_ecsqlValue->TryGetContainedEnumerators(enumerators) || enumerators.empty())
            return Env().Undefined();

        Napi::Array jsEnumList = Napi::Array::New(Env(), enumerators.size());

        uint32_t arrayIndex = 0;
        for (ECEnumeratorCP enumerator : enumerators)
            {
            Napi::Object jsEnum = Napi::Object::New(Env());
            jsEnum.Set("schema", Napi::String::New(Env(), enumType->GetSchema().GetName().c_str()));
            jsEnum.Set("name", Napi::String::New(Env(), enumType->GetName().c_str()));
            jsEnum.Set("key", Napi::String::New(Env(), enumerator->GetName().c_str()));
            jsEnum.Set("value", enumerator->IsInteger() ? (napi_value)Napi::Number::New(Env(), enumerator->GetInteger()) : (napi_value)Napi::String::New(Env(), enumerator->GetString().c_str()));
            jsEnumList.Set(arrayIndex, jsEnum);
            arrayIndex++;
            }

        return jsEnumList;
        }

    Napi::Value GetNavigation(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
            THROW_JS_EXCEPTION("ECSqlValue is not initialized");

        ECClassId relClassId;
        BeInt64Id navId = m_ecsqlValue->GetNavigation(&relClassId);

        Napi::Object jsNavValue = Napi::Object::New(Env());
        jsNavValue.Set(ECN::ECJsonSystemNames::Navigation::Id(), Napi::String::New(Env(), navId.ToHexStr().c_str()));
        if (relClassId.IsValid())
            {
            Utf8StringCR relClassTableSpace = m_ecsqlValue->GetColumnInfo().GetRootClass().GetTableSpace();
            ECClassCP relClass = m_ecdb->Schemas().GetClass(relClassId, relClassTableSpace.c_str());
            if (relClass == nullptr)
                THROW_JS_EXCEPTION("Failed to find ECRelationhipClass for the Navigation Value's RelECClassId.");

            Utf8String relClassName = ECJsonUtilities::FormatClassName(*relClass);
            jsNavValue.Set(ECN::ECJsonSystemNames::Navigation::RelClassName(), Napi::String::New(Env(), relClassName.c_str()));
            }

        return jsNavValue;
        }

    //implementations are after NativeECSqlValueIterable as it needs to call into that class
    Napi::Value GetStructIterator(NapiInfoCR);
    Napi::Value GetArrayIterator(NapiInfoCR);
    };

//=======================================================================================
// Projects the IECSqlValueIterable interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlValueIterator : BeObjectWrap<NativeECSqlValueIterator>
    {
    private:
        DEFINE_CONSTRUCTOR;
        ECDb const* m_ecdb = nullptr;
        IECSqlValueIterable const* m_iterable = nullptr;
        bool m_isBeforeFirstElement = true;
        IECSqlValueIterable::const_iterator m_it;
        IECSqlValueIterable::const_iterator m_endIt;

    public:
        NativeECSqlValueIterator(NapiInfoCR info) : BeObjectWrap<NativeECSqlValueIterator>(info)
            {
            if (info.Length() < 2)
                THROW_JS_EXCEPTION("ECSqlValueIterator constructor expects two argument.");

            m_iterable = info[0].As<Napi::External<IECSqlValueIterable>>().Data();
            if (m_iterable == nullptr)
                THROW_JS_EXCEPTION("Invalid first arg for NativeECSqlValueIterator constructor. IECSqlValueIterable must not be nullptr");

            m_endIt = m_iterable->end();

            m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
            if (m_ecdb == nullptr)
                THROW_JS_EXCEPTION("Invalid second arg for NativeECSqlValueIterator constructor. ECDb must not be nullptr");
            }

        ~NativeECSqlValueIterator() {SetInDestructor();}

        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(Constructor().Value());
            }

        //  Create projections
        static void Init(Napi::Env& env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ECSqlValueIterator", {
            InstanceMethod("moveNext", &NativeECSqlValueIterator::MoveNext),
            InstanceMethod("getCurrent", &NativeECSqlValueIterator::GetCurrent)});

            exports.Set("ECSqlValueIterator", t);

            SET_CONSTRUCTOR(t);
            }

        static Napi::Object New(Napi::Env const& env, IECSqlValueIterable const& iterable, ECDb const& ecdb)
            {
            return Constructor().New({Napi::External<IECSqlValueIterable>::New(env, const_cast<IECSqlValueIterable*>(&iterable)), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
            }

        // A JS iterator expects the initial state of an iterator to be before the first element.
        // So on the first call to MoveNext, the C++ iterator must be created, and not incremented.
        Napi::Value MoveNext(NapiInfoCR info)
            {
            if (m_isBeforeFirstElement)
                {
                m_it = m_iterable->begin();
                m_isBeforeFirstElement = false;
                }
            else
                {
                if (m_it != m_endIt)
                    ++m_it; //don't increment if the iterator is already at its end.
                }

            return Napi::Boolean::New(info.Env(), m_it != m_endIt);
            }

        Napi::Value GetCurrent(NapiInfoCR info)
            {
            return NativeECSqlValue::New(Env(), *m_it, *m_ecdb);
            }
    };

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeECSqlValue::GetStructIterator(NapiInfoCR info)
    {
    if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
        THROW_JS_EXCEPTION("ECSqlValue is not initialized");

    return NativeECSqlValueIterator::New(info.Env(), m_ecsqlValue->GetStructIterable(), *m_ecdb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeECSqlValue::GetArrayIterator(NapiInfoCR info)
    {
    if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
        THROW_JS_EXCEPTION("ECSqlValue is not initialized");

    return NativeECSqlValueIterator::New(info.Env(), m_ecsqlValue->GetArrayIterable(), *m_ecdb);
    }

//=======================================================================================
// Projects the changeset reader into JS.
//! @bsiclass
//=======================================================================================
struct NativeChangesetReader : BeObjectWrap<NativeChangesetReader>
{
private:
    DEFINE_CONSTRUCTOR;
    NativeChangeset m_changeset;


public:
    NativeChangesetReader(NapiInfoCR info) : BeObjectWrap<NativeChangesetReader>(info){}
    ~NativeChangesetReader() {SetInDestructor();}

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)       {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ChangesetReader", {
          InstanceMethod("close", &NativeChangesetReader::Close),
          InstanceMethod("getColumnCount", &NativeChangesetReader::GetColumnCount),
          InstanceMethod("getColumnValue", &NativeChangesetReader::GetColumnValue),
          InstanceMethod("getColumnValueType", &NativeChangesetReader::GetColumnValueType),
          InstanceMethod("getFileName", &NativeChangesetReader::GetFileName),
          InstanceMethod("getOpCode", &NativeChangesetReader::GetOpCode),
          InstanceMethod("getRow", &NativeChangesetReader::GetRow),
          InstanceMethod("getDdlChanges", &NativeChangesetReader::GetDdlChanges),
          InstanceMethod("getTableName", &NativeChangesetReader::GetTableName),
          InstanceMethod("isIndirectChange", &NativeChangesetReader::IsIndirectChange),
          InstanceMethod("isPrimaryKeyColumn", &NativeChangesetReader::IsPrimaryKeyColumn),
          InstanceMethod("open", &NativeChangesetReader::Open),
          InstanceMethod("reset", &NativeChangesetReader::Reset),
          InstanceMethod("step", &NativeChangesetReader::Step),
        });

        exports.Set("ChangesetReader", t);
        SET_CONSTRUCTOR(t);
        }

    Napi::Value GetRow(NapiInfoCR info)
        {
        return m_changeset.GetRow(Env());
        }
    Napi::Value GetColumnValue(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        REQUIRE_ARGUMENT_INTEGER(1, valueKind);
        return m_changeset.GetColumnValue(Env(), columnIndex, valueKind);
        }
    Napi::Value GetColumnValueType(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        REQUIRE_ARGUMENT_INTEGER(1, valueKind);
        return m_changeset.GetColumnValueType(Env(), columnIndex, valueKind);
        }
    Napi::Value IsPrimaryKeyColumn(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        return m_changeset.IsPrimaryKeyColumn(Env(), columnIndex);
        }
    Napi::Value IsIndirectChange(NapiInfoCR info)
        {
        return m_changeset.IsIndirectChange(Env());
        }
    Napi::Value GetColumnCount(NapiInfoCR info)
        {
        return m_changeset.GetColumnCount(Env());
        }
    Napi::Value GetOpCode(NapiInfoCR info)
        {
        return m_changeset.GetOpCode(Env());;
        }
    Napi::Value GetDdlChanges(NapiInfoCR info)
        {
        return m_changeset.GetDdlChanges(Env());;
        }
    Napi::Value Open(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, fileName);
        REQUIRE_ARGUMENT_BOOL(1, invert);
        return m_changeset.Open(Env(), fileName.c_str(), invert);
        }
    Napi::Value Step(NapiInfoCR info)
        {
        return m_changeset.Step(Env());
        }
    Napi::Value Close(NapiInfoCR info)
        {
        return m_changeset.Close(Env());
        }
    Napi::Value Reset(NapiInfoCR info)
        {
        return m_changeset.Reset(Env());
        }
    Napi::Value GetFileName(NapiInfoCR info)
        {
        return m_changeset.GetFileName(Env());
        }
    Napi::Value GetTableName(NapiInfoCR info)
        {
        return m_changeset.GetTableName(Env());
        }
};

//=======================================================================================
// Projects the ECSqlStatement class into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlStatement : BeObjectWrap<NativeECSqlStatement> {
private:
    DEFINE_CONSTRUCTOR;
    ECSqlStatement m_stmt;

    struct IssueListener : BentleyApi::ECN::IIssueListener {
        mutable Utf8String m_lastIssue;
        ECDbR m_db;
        bool m_active;

        void _OnIssueReported(BentleyApi::ECN::IssueSeverity severity, BentleyApi::ECN::IssueCategory category, BentleyApi::ECN::IssueType type, Utf8CP message) const override { m_lastIssue = message; }

        explicit IssueListener(ECDbR db) : m_active(SUCCESS == db.AddIssueListener(*this)), m_db(db) {}
        ~IssueListener() {
            if (m_active) m_db.RemoveIssueListener();
        }
    };

public:
    NativeECSqlStatement(NapiInfoCR info) : BeObjectWrap<NativeECSqlStatement>(info) {}
    ~NativeECSqlStatement() { SetInDestructor(); }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports) {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECSqlStatement", {
            InstanceMethod("prepare", &NativeECSqlStatement::Prepare),
            InstanceMethod("reset", &NativeECSqlStatement::Reset),
            InstanceMethod("dispose", &NativeECSqlStatement::Dispose),
            InstanceMethod("clearBindings", &NativeECSqlStatement::ClearBindings),
            InstanceMethod("getBinder", &NativeECSqlStatement::GetBinder),
            InstanceMethod("step", &NativeECSqlStatement::Step),
            InstanceMethod("stepForInsert", &NativeECSqlStatement::StepForInsert),
            InstanceMethod("stepAsync", &NativeECSqlStatement::StepAsync),
            InstanceMethod("stepForInsertAsync", &NativeECSqlStatement::StepForInsertAsync),
            InstanceMethod("getColumnCount", &NativeECSqlStatement::GetColumnCount),
            InstanceMethod("getValue", &NativeECSqlStatement::GetValue),
            InstanceMethod("getNativeSql", &NativeECSqlStatement::GetNativeSql)
        });

        exports.Set("ECSqlStatement", t);
        SET_CONSTRUCTOR(t);
    }

    Napi::Value Prepare(NapiInfoCR info) {
        if (info.Length() < 2)
            THROW_JS_TYPE_EXCEPTION("ECSqlStatement::Prepare requires two arguments");

        Napi::Object dbObj = info[0].As<Napi::Object>();

        ECDb* ecdb = nullptr;
        if (NativeDgnDb::InstanceOf(dbObj)) {
            NativeDgnDb* addonDgndb = NativeDgnDb::Unwrap(dbObj);
            if (!addonDgndb->IsOpen())
                return CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());

            ecdb = &addonDgndb->GetDgnDb();
        } else if (NativeECDb::InstanceOf(dbObj)) {
            NativeECDb* addonECDb = NativeECDb::Unwrap(dbObj);
            ecdb = &addonECDb->GetECDb();

            if (!ecdb->IsDbOpen())
                return CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());
        } else {
            THROW_JS_TYPE_EXCEPTION("ECSqlStatement::Prepare requires first argument to be a NativeDgnDb or NativeECDb object.");
        }

        REQUIRE_ARGUMENT_STRING(1, ecsql);
        OPTIONAL_ARGUMENT_BOOL(2,logErrors, true);
        IssueListener listener(*ecdb);

        ECSqlStatus status = m_stmt.Prepare(*ecdb, ecsql.c_str(), logErrors);
        return CreateErrorObject0(ToDbResult(status), !status.IsSuccess() ? listener.m_lastIssue.c_str() : nullptr, Env());
    }

    Napi::Value Reset(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("ECSqlStatement is not prepared.");

        ECSqlStatus status = m_stmt.Reset();
        return Napi::Number::New(Env(), (int)ToDbResult(status));
    }

    void Dispose(NapiInfoCR info) {
        m_stmt.Finalize();
    }

    Napi::Value ClearBindings(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("ECSqlStatement is not prepared.");

        auto status = m_stmt.ClearBindings();
        return Napi::Number::New(Env(), (int)ToDbResult(status));
    }

    Napi::Value GetBinder(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("ECSqlStatement is not prepared.");

        if (info.Length() != 1)
            THROW_JS_EXCEPTION("GetBinder requires a parameter index or name as argument");

        Napi::Value paramArg = info[0];
        if (!paramArg.IsNumber() && !paramArg.IsString())
            THROW_JS_EXCEPTION("GetBinder requires a parameter index or name as argument");

        int paramIndex = -1;
        if (paramArg.IsNumber())
            paramIndex = (int)paramArg.ToNumber().Int32Value();
        else
            paramIndex = m_stmt.GetParameterIndex(paramArg.ToString().Utf8Value().c_str());

        IECSqlBinder& binder = m_stmt.GetBinder(paramIndex);
        return NativeECSqlBinder::New(info.Env(), binder, *m_stmt.GetECDb());
    }

    Napi::Value Step(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("ECSqlStatement is not prepared.");

        DbResult status = m_stmt.Step();
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value StepForInsert(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("ECSqlStatement is not prepared.");

        ECInstanceKey key;
        DbResult status = m_stmt.Step(key);

        Napi::Object ret = Napi::Object::New(Env());
        ret.Set(Napi::String::New(Env(), "status"), Napi::Number::New(Env(), (int)status));
        if (BE_SQLITE_DONE == status)
            ret.Set(toJsString(Env(), "id"), toJsString(Env(), key.GetInstanceId()));

        return ret;
    }

    void StepAsync(NapiInfoCR info) {
        REQUIRE_ARGUMENT_FUNCTION(0, responseCallback);
        JsInterop::StepAsync(responseCallback, m_stmt, false);
    }

    void StepForInsertAsync(NapiInfoCR info) {
        REQUIRE_ARGUMENT_FUNCTION(0, responseCallback);
        JsInterop::StepAsync(responseCallback, m_stmt, true);
    }

    Napi::Value GetColumnCount(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("ECSqlStatement is not prepared.");

        int colCount = m_stmt.GetColumnCount();
        return Napi::Number::New(info.Env(), colCount);
    }

    Napi::Value GetValue(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("ECSqlStatement is not prepared.");

        REQUIRE_ARGUMENT_INTEGER(0, colIndex);

        IECSqlValue const& val = m_stmt.GetValue(colIndex);
        return NativeECSqlValue::New(info.Env(), val, *m_stmt.GetECDb());
    }

    Napi::Value GetNativeSql(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("ECSqlStatement is not prepared.");

        return Napi::String::New(Env(), m_stmt.GetNativeSql());
    }

    static DbResult ToDbResult(ECSqlStatus status) {
        if (status.IsSuccess())
            return BE_SQLITE_OK;
        return status.IsSQLiteError() ? status.GetSQLiteError() : BE_SQLITE_ERROR;
    }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NativeBlobIo : BeObjectWrap<NativeBlobIo> {
private:
    DEFINE_CONSTRUCTOR;
    BlobIO m_blobIO;

public:
    NativeBlobIo(NapiInfoCR info) : BeObjectWrap<NativeBlobIo>(info) {}
    ~NativeBlobIo() { SetInDestructor(); }
    static void Init(Napi::Env& env, Napi::Object exports) {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "BlobIO", {
            InstanceMethod("close", &NativeBlobIo::Close),
            InstanceMethod("open", &NativeBlobIo::Open),
            InstanceMethod("changeRow", &NativeBlobIo::ChangeRow),
            InstanceMethod("read", &NativeBlobIo::Read),
            InstanceMethod("write", &NativeBlobIo::Write),
            InstanceMethod("getNumBytes", &NativeBlobIo::GetNumBytes),
            InstanceMethod("isValid", &NativeBlobIo::IsValid),
        });
        exports.Set("BlobIO", t);
        SET_CONSTRUCTOR(t);
    }
    Napi::Value IsValid(NapiInfoCR info) {
        return Napi::Boolean::New(info.Env(), m_blobIO.IsValid());
    }
    void Close(NapiInfoCR info) {
        if (m_blobIO.IsValid())
            m_blobIO.Close();
    }
    void Open(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, dbObj);
        Db* db = nullptr;
        if (NativeDgnDb::InstanceOf(dbObj)) {
            db = &NativeDgnDb::Unwrap(dbObj)->GetDgnDb();
        } else if (NativeECDb::InstanceOf(dbObj)) {
            db = &NativeECDb::Unwrap(dbObj)->GetECDb();
        } else if (SQLiteDb::InstanceOf(dbObj)) {
            db = &SQLiteDb::Unwrap(dbObj)->GetDb();
        } else {
            THROW_JS_EXCEPTION("requires an open database");
        }
        REQUIRE_ARGUMENT_ANY_OBJ(1, args);
        auto tableName = stringMember(args, JsInterop::json_tableName());
        if (tableName == "")
            BeNapi::ThrowJsException(info.Env(), "tableName missing");
        auto columnName = stringMember(args, JsInterop::json_columnName());
        if (columnName == "")
            BeNapi::ThrowJsException(info.Env(), "columnName missing");
        auto row = intMember(args, JsInterop::json_row(), 0);
        if (row == 0)
            BeNapi::ThrowJsException(info.Env(), "invalid row");
        auto writeable = boolMember(args, JsInterop::json_writeable(), false);
        auto stat = m_blobIO.Open(*db, tableName.c_str(), columnName.c_str(), row, writeable);
        if (BE_SQLITE_OK != stat)
            JsInterop::throwSqlResult("cannot open blob", db->GetDbFileName(), stat);
    }
    void ChangeRow(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, row);
        auto stat = m_blobIO.ReOpen(row);
        if (BE_SQLITE_OK != stat)
            JsInterop::throwSqlResult("cannot reopen blob", "", stat);
    }
    Napi::Value Read(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, args);
        auto numBytes = intMember(args, JsInterop::json_numBytes(), 0);
        auto offset = intMember(args, JsInterop::json_offset(), 0);
        auto blobObj = args.Get(JsInterop::json_blob());
        auto blob = (blobObj.IsTypedArray() && blobObj.As<Napi::Uint8Array>().ByteLength() >= numBytes) ? blobObj.As<Napi::Uint8Array>() : Napi::Uint8Array::New(info.Env(), numBytes);
        auto stat = m_blobIO.Read(blob.Data(), numBytes, offset);
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("cannot read from blob", "", stat);
        return blob;
    }
    void Write(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, args);
        auto numBytes = intMember(args, JsInterop::json_numBytes(), 0);
        auto offset = intMember(args, JsInterop::json_offset(), 0);
        auto blobObj = args.Get(JsInterop::json_blob());
        if (!blobObj.IsTypedArray() || blobObj.As<Napi::Uint8Array>().ByteLength() < numBytes)
            BeNapi::ThrowJsException(info.Env(), "blob invalid or too small");
        auto stat = m_blobIO.Write(Napi::Uint8Array(blobObj.Env(), blobObj).Data(), numBytes, offset);
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("cannot write to blob", "", stat);
    }
    Napi::Value GetNumBytes(NapiInfoCR info) {
        return Napi::Number::New(info.Env(), m_blobIO.GetNumBytes());
    }
};

//=======================================================================================
// Projects the BeSQLite::Statement class into JS.
//! @bsiclass
//=======================================================================================
struct NativeSqliteStatement : BeObjectWrap<NativeSqliteStatement> {
private:
    DEFINE_CONSTRUCTOR;
    Statement m_stmt;

    int GetParameterIndex(NapiInfoCR info, int minArgs) {
        if (info.Length() < minArgs)
            return -1;

        auto arg = info[0];
        if (arg.IsNumber())
            return (int)arg.ToNumber().Int32Value();

        if (!arg.IsString())
            return -1;

        return m_stmt.GetParameterIndex(arg.ToString().Utf8Value().c_str());
    }

public:
    NativeSqliteStatement(NapiInfoCR info) : BeObjectWrap<NativeSqliteStatement>(info) {}
    ~NativeSqliteStatement() { SetInDestructor(); }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports) {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "SqliteStatement", {
            InstanceMethod("dispose", &NativeSqliteStatement::Dispose),
            InstanceMethod("prepare", &NativeSqliteStatement::Prepare),
            InstanceMethod("isReadonly", &NativeSqliteStatement::IsReadonly),
            InstanceMethod("bindNull", &NativeSqliteStatement::BindNull),
            InstanceMethod("bindBlob", &NativeSqliteStatement::BindBlob),
            InstanceMethod("bindDouble", &NativeSqliteStatement::BindDouble),
            InstanceMethod("bindInteger", &NativeSqliteStatement::BindInteger),
            InstanceMethod("bindString", &NativeSqliteStatement::BindString),
            InstanceMethod("bindId", &NativeSqliteStatement::BindId),
            InstanceMethod("bindGuid", &NativeSqliteStatement::BindGuid),
            InstanceMethod("clearBindings", &NativeSqliteStatement::ClearBindings),
            InstanceMethod("step", &NativeSqliteStatement::Step),
            InstanceMethod("stepAsync", &NativeSqliteStatement::StepAsync),
            InstanceMethod("reset", &NativeSqliteStatement::Reset),
            InstanceMethod("getColumnBytes", &NativeSqliteStatement::GetColumnBytes),
            InstanceMethod("getColumnCount", &NativeSqliteStatement::GetColumnCount),
            InstanceMethod("getColumnType", &NativeSqliteStatement::GetColumnType),
            InstanceMethod("getColumnName", &NativeSqliteStatement::GetColumnName),
            InstanceMethod("isValueNull", &NativeSqliteStatement::IsValueNull),
            InstanceMethod("getValueBlob", &NativeSqliteStatement::GetValueBlob),
            InstanceMethod("getValueDouble", &NativeSqliteStatement::GetValueDouble),
            InstanceMethod("getValueInteger", &NativeSqliteStatement::GetValueInteger),
            InstanceMethod("getValueString", &NativeSqliteStatement::GetValueString),
            InstanceMethod("getValueId", &NativeSqliteStatement::GetValueId),
            InstanceMethod("getValueGuid", &NativeSqliteStatement::GetValueGuid),
        });

        exports.Set("SqliteStatement", t);
        SET_CONSTRUCTOR(t);
    }

    void Dispose(NapiInfoCR info) {
        m_stmt.Finalize();
    }

    void Prepare(NapiInfoCR info) {
        Napi::Object dbObj = info[0].As<Napi::Object>();
        Db* db = nullptr;
        if (NativeDgnDb::InstanceOf(dbObj)) {
            db = &NativeDgnDb::Unwrap(dbObj)->GetDgnDb();
        } else if (SQLiteDb::InstanceOf(dbObj)) {
            db = &SQLiteDb::Unwrap(dbObj)->GetDb();
        } else if (NativeECDb::InstanceOf(dbObj)) {
            db = &NativeECDb::Unwrap(dbObj)->GetECDb();
        } else {
            THROW_JS_EXCEPTION("invalid database object");
        }
        if (!db->IsDbOpen())
            THROW_JS_EXCEPTION("Prepare requires an open database");

        REQUIRE_ARGUMENT_STRING(1, sql);
        OPTIONAL_ARGUMENT_BOOL(2,logErrors, true);
        // Prepare *without* the mutex held. So, if there's an error, potentially the error message will be wrong.
        // Oh well, it's just for debugging and it will be very rare that another thread causes an error here.
        DbResult status;
        if (logErrors)
            status = m_stmt.Prepare(*db, sql.c_str());
        else
            status = m_stmt.TryPrepare(*db, sql.c_str());

        if (status != BE_SQLITE_OK)
            BeNapi::ThrowJsException(Env(), db->GetLastError().c_str(), status);
    }

    Napi::Value IsReadonly(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_EXCEPTION("Cannot call IsReadonly on unprepared statement.");
        return Napi::Boolean::New(Env(), m_stmt.IsReadonly());
    }

    Napi::Value BindNull(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 1);
        if (paramIndex < 1)
            THROW_JS_EXCEPTION("Invalid parameters");
        return Napi::Number::New(Env(), (int)m_stmt.BindNull(paramIndex));
    }

    Napi::Value BindBlob(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_EXCEPTION("Invalid parameters");

        Napi::Value const& blobVal = info[1];
        if (blobVal.IsTypedArray()) {
            Napi::TypedArray typedArray = blobVal.As<Napi::TypedArray>();
            if (typedArray.TypedArrayType() == napi_uint8_array) {
                Napi::Uint8Array uint8Array = typedArray.As<Napi::Uint8Array>();
                const auto length = (int)uint8Array.ByteLength();
                const DbResult stat = m_stmt.BindBlob(paramIndex, length > 0 ? uint8Array.Data() : (void*)"", length, Statement::MakeCopy::Yes);
                return Napi::Number::New(Env(), (int)stat);
            }
        }

        if (blobVal.IsArrayBuffer()) {
            Napi::ArrayBuffer buf = blobVal.As<Napi::ArrayBuffer>();
            const auto length = (int)buf.ByteLength();
            const DbResult stat = m_stmt.BindBlob(paramIndex, length > 0 ? buf.Data() : (void*)"", length, Statement::MakeCopy::Yes);
            return Napi::Number::New(Env(), (int)stat);
        }

        THROW_JS_EXCEPTION("BindBlob requires a Uint8Array or ArrayBuffer arg");
    }

    Napi::Value BindDouble(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_EXCEPTION("Invalid parameters");

        REQUIRE_ARGUMENT_NUMBER(1, val);
        const DbResult stat = m_stmt.BindDouble(paramIndex, val.DoubleValue());
        return Napi::Number::New(Env(), (int)stat);
    }

    Napi::Value BindInteger(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_EXCEPTION("Invalid parameters");

        Napi::Value const& val = info[1];
        if (!val.IsNumber() && !val.IsString())
            THROW_JS_EXCEPTION("BindInteger expects a string or number value.");

        int64_t int64Val;
        if (val.IsNumber())
            int64Val = val.ToNumber().Int64Value();
        else {
            Utf8String strVal(val.ToString().Utf8Value().c_str());
            if (strVal.empty())
                THROW_JS_EXCEPTION("Integral string passed to BindInteger must not be empty.");

            bool const isNegativeNumber = strVal[0] == '-';
            Utf8CP positiveNumberStr = isNegativeNumber ? strVal.c_str() + 1 : strVal.c_str();
            uint64_t uVal = 0;
            if (SUCCESS != BeStringUtilities::ParseUInt64(uVal, positiveNumberStr)) //also supports hex strings
            {
                Utf8String error;
                error.Sprintf("BindInteger failed. Could not parse string %s to a valid integer.", strVal.c_str());
                THROW_JS_EXCEPTION(error.c_str());
            }

            if (isNegativeNumber && uVal > (uint64_t)std::numeric_limits<int64_t>::max()) {
                Utf8String error;
                error.Sprintf("BindInteger failed. Number in string %s is too large to fit into a signed 64 bit integer value.", strVal.c_str());
                THROW_JS_EXCEPTION(error.c_str());
            }

            int64Val = uVal;
            if (isNegativeNumber)
                int64Val *= -1;
        }

        const DbResult stat = m_stmt.BindInt64(paramIndex, int64Val);
        return Napi::Number::New(Env(), (int)stat);
    }

    Napi::Value BindString(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_EXCEPTION("Invalid parameters");

        REQUIRE_ARGUMENT_STRING(1, val);
        const DbResult stat = m_stmt.BindText(paramIndex, val.c_str(), Statement::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int)stat);
    }

    Napi::Value BindGuid(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_EXCEPTION("Invalid parameters");

        REQUIRE_ARGUMENT_STRING(1, guidString);
        BeGuid guid;
        if (SUCCESS != guid.FromString(guidString.c_str()))
            return Napi::Number::New(Env(), (int)BE_SQLITE_ERROR);

        const DbResult stat = m_stmt.BindGuid(paramIndex, guid);
        return Napi::Number::New(Env(), (int)stat);
    }

    Napi::Value BindId(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_EXCEPTION("Invalid parameters");

        REQUIRE_ARGUMENT_STRING(1, idString);
        BeInt64Id id;
        if (SUCCESS != BeInt64Id::FromString(id, idString.c_str()))
            return Napi::Number::New(Env(), (int)BE_SQLITE_ERROR);

        const DbResult stat = m_stmt.BindId(paramIndex, id);
        return Napi::Number::New(Env(), (int)stat);
    }

    Napi::Value ClearBindings(NapiInfoCR info) {
        return Napi::Number::New(Env(), (int)m_stmt.ClearBindings());
    }

    Napi::Value Step(NapiInfoCR info) {
        return Napi::Number::New(Env(), (int)m_stmt.Step());
    }

    void StepAsync(NapiInfoCR info) {
        REQUIRE_ARGUMENT_FUNCTION(0, responseCallback);
        JsInterop::StepAsync(responseCallback, m_stmt);
    }

    Napi::Value GetColumnBytes(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);
        return Napi::Number::New(info.Env(), m_stmt.GetColumnBytes(colIndex));
    }

    Napi::Value GetColumnCount(NapiInfoCR info) {
        const int colCount = m_stmt.GetColumnCount();
        return Napi::Number::New(info.Env(), colCount);
    }

    Napi::Value GetColumnType(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);
        return Napi::Number::New(Env(), (int)m_stmt.GetColumnType(colIndex));
    }

    Napi::Value GetColumnName(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);
        return toJsString(Env(), m_stmt.GetColumnName(colIndex));
    }

    Napi::Value IsValueNull(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);
        return Napi::Boolean::New(Env(), m_stmt.IsColumnNull(colIndex));
    }

    Napi::Value GetValueBlob(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);

        void const* data = m_stmt.GetValueBlob(colIndex);
        int blobSize = m_stmt.GetColumnBytes(colIndex);
        Napi::Uint8Array blob = Napi::Uint8Array::New(Env(), blobSize);
        memcpy(blob.Data(), data, blobSize);
        return blob;
    }

    Napi::Value GetValueDouble(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);
        return Napi::Number::New(Env(), m_stmt.GetValueDouble(colIndex));
    }

    Napi::Value GetValueInteger(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);
        return Napi::Number::New(Env(), m_stmt.GetValueInt64(colIndex));
    }

    Napi::Value GetValueString(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);
        return toJsString(Env(), m_stmt.GetValueText(colIndex));
    }

    Napi::Value GetValueId(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);

        BeInt64Id id = m_stmt.GetValueId<BeInt64Id>(colIndex);
        if (!id.IsValid())
            return Env().Undefined();

        return toJsString(Env(), id);
    }

    Napi::Value GetValueGuid(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, colIndex);

        BeGuid guid = m_stmt.GetValueGuid(colIndex);
        if (!guid.IsValid())
            return Env().Undefined();

        return toJsString(Env(), guid.ToString());
    }

    Napi::Value Reset(NapiInfoCR info) {
        const DbResult status = m_stmt.Reset();
        return Napi::Number::New(Env(), (int)status);
    }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ECSqlStepWorker : Napi::AsyncWorker
{
private:
    ECSqlStatement& m_stmt;
    int m_status;
    ECInstanceKey m_instanceKey;
    bool m_stepForInsert;

    void Execute() override
        {
        // NativeLogging::LoggingManager::GetLogger("ECSqlStepWorkerTestCategory")->error("ECSqlStepWorker: In worker thread");

        if (m_stmt.IsPrepared())
            {
            if (m_stepForInsert)
                m_status = m_stmt.Step(m_instanceKey);
            else
                m_status = m_stmt.Step();
            }
        else
            {
            m_status = (int) BE_SQLITE_MISUSE;
            }
        }

    void OnOK() override
        {
        if (m_stepForInsert)
            {
            Napi::Object retval = Napi::Object::New(Env());
            retval.Set(Napi::String::New(Env(), "status"), Napi::Number::New(Env(), (int)m_status));
            if (BE_SQLITE_DONE == m_status)
                retval.Set(toJsString(Env(), "id"), toJsString(Env(), m_instanceKey.GetInstanceId()));

            Callback().MakeCallback(Receiver().Value(), {retval});
            }
        else
            {
            Napi::Number retval = Napi::Number::New(Env(), (int)m_status);
            Callback().MakeCallback(Receiver().Value(), {retval});
            }
        }
public:
    ECSqlStepWorker( Napi::Function& callback, ECSqlStatement& stmt, bool stepForInsert)
        : Napi::AsyncWorker(callback), m_stmt(stmt), m_stepForInsert(stepForInsert)
        {
        NativeLogging::CategoryLogger("ECSqlStepWorkerTestCategory").error("ECSqlStepWorker: Start on main thread");
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::StepAsync(Napi::Function& callback, ECSqlStatement& stmt, bool stepForInsert)
    {
    auto worker = new ECSqlStepWorker(callback, stmt, stepForInsert);
    worker->Queue();
    }

//=======================================================================================
// @bsistruct
//=======================================================================================
struct SqliteStepWorker : Napi::AsyncWorker
{
private:
    Statement& m_stmt;
    int m_status;
    void Execute() override
        {
        if (m_stmt.IsPrepared())
            m_status = m_stmt.Step();
        else
            m_status = (int) BE_SQLITE_MISUSE;
        }

    void OnOK() override
        {
        auto retval = Napi::Number::New(Env(), (int)m_status);
        Callback().MakeCallback(Receiver().Value(), {retval});
        }
public:
    SqliteStepWorker( Napi::Function& callback, Statement& stmt)
        : Napi::AsyncWorker(callback), m_stmt(stmt)
        {
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::StepAsync(Napi::Function& callback, Statement& stmt)
    {
    auto worker = new SqliteStepWorker(callback, stmt);
    worker->Queue();
    }

//=======================================================================================
// A request to generate a snap point given an element and additional parameters.
//! @bsiclass
//=======================================================================================
struct SnapRequest : BeObjectWrap<SnapRequest>
{
    //=======================================================================================
    // Async Worker that does a snap on another thread.
    //! @bsiclass
    //=======================================================================================
    struct Snapper : DgnDbWorker
    {
        Napi::ObjectReference m_snapRequest;
        BeJsDocument m_input;
        void OnComplete()
            {
            auto* request = SnapRequest::Unwrap(m_snapRequest.Value());
            BeMutexHolder holder(request->m_mutex);
            request->m_pending = nullptr;
            }

        void Execute() final
            {
            SnapContext::DoSnap(m_output, m_input, GetDb(), *this);
            if (IsCanceled())
                OnSkipped();
            }

        void OnOK() final
            {
            OnComplete();
            DgnDbWorker::OnOK();
            }

        void OnError(Napi::Error const& e) final
            {
            OnComplete();
            DgnDbWorker::OnError(e);
            }

        void OnSkipped() final
            {
            // SetError will trigger OnError which will call OnComplete.
            SetError("aborted");
            }

        Snapper(SnapRequest const& request, DgnDbR db, Napi::Object input) : DgnDbWorker(db, input.Env())
            {
            m_snapRequest.Reset(request.Value(), 1);
            m_input.From(input);
            }

        ~Snapper() { m_snapRequest.Reset(); }
    };

    DEFINE_CONSTRUCTOR;
    mutable BeMutex m_mutex;
    RefCountedPtr<Snapper> m_pending;

    Napi::Value DoSnap(NapiInfoCR info)
        {
        BeMutexHolder holder(m_mutex);
        if (m_pending.IsValid())
            CancelSnap(info);

        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db);
        REQUIRE_ARGUMENT_ANY_OBJ(1, request);
        m_pending = new Snapper(*this, db->GetDgnDb(), request); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        return m_pending->Queue();  // Snap happens in another thread
        }

    // Cancel a previous request for a snap. If no snap is pending, does nothing
    void CancelSnap(NapiInfoCR info)
        {
        BeMutexHolder holder(m_mutex);
        if (m_pending.IsValid())
            {
            m_pending->Cancel();
            m_pending = nullptr;
            }
        }

    SnapRequest(NapiInfoCR info) : BeObjectWrap<SnapRequest>(info) {}
    ~SnapRequest() {SetInDestructor();}

    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "SnapRequest", {
          InstanceMethod("doSnap", &SnapRequest::DoSnap),
          InstanceMethod("cancelSnap", &SnapRequest::CancelSnap)
        });

        exports.Set("SnapRequest", t);

        SET_CONSTRUCTOR(t);
        }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct TileWorker : Napi::AsyncWorker
{
protected:
    // Inputs
    DgnDbPtr m_db;
    Utf8String m_treeId;
    ICancellablePtr m_cancellationToken;

    // Outputs
    DgnDbStatus m_status;

    TileWorker(ICancellableP cancellationToken, Napi::Function& callback, DgnDbR db, Utf8StringCR treeId)
        : Napi::AsyncWorker(callback), m_db(&db), m_treeId(treeId), m_cancellationToken(cancellationToken), m_status(DgnDbStatus::BadRequest)
        {
        BeAssert(m_cancellationToken.IsValid());
        }

    void OnError(Napi::Error const& e) override
        {
        auto retval = CreateBentleyReturnErrorObject(1, e.Message().c_str(), Env());
        Callback().MakeCallback(Receiver().Value(), {retval});
        }

    void OnOK() override
        {
        if (DgnDbStatus::Success == m_status)
            {
            Napi::Value jsValue = GetTileResult();
            auto retval = CreateBentleyReturnSuccessObject(jsValue);
            Callback().MakeCallback(Receiver().Value(), {retval});
            }
        else
            {
            auto retval = CreateBentleyReturnErrorObject(m_status, nullptr, Env());
            Callback().MakeCallback(Receiver().Value(), {retval});
            }
        }

    virtual Napi::Value GetTileResult() = 0;

    bool IsCanceled() const { return m_cancellationToken.IsValid() && m_cancellationToken->IsCanceled(); }

    bool PreExecute()
        {
        if (!IsCanceled())
            return true;

        m_status = DgnDbStatus::NotOpen;
        return false;
        }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct GetTileTreeWorker : TileWorker
{
private:
    // Output
    Json::Value m_result;

    void Execute() final
        {
        if (!PreExecute())
            return;

        // JsInterop::GetLogger().debugv("GetTileTreeWorker::Execute: %s", m_treeId.ToString().c_str());
        m_status = T_HOST.Visualization().GetTileTreeJson(m_result, *m_db, m_treeId, true);
        }

    Napi::Value GetTileResult() final
        {
        BeJsNapiObject ret(Env());
        ret.From(m_result);
        return ret;
        }
public:
    GetTileTreeWorker(ICancellableP cancel, Napi::Function& callback, DgnDbR db, Utf8StringCR treeId) : TileWorker(cancel, callback, db, treeId)
        {
        // JsInterop::GetLogger().debugv("GetTileTreeWorker ctor: %s", m_treeId.ToString().c_str());
        }
    ~GetTileTreeWorker()
        {
        // JsInterop::GetLogger().debugv("GetTileTreeWorker dtor: %s", m_treeId.ToString().c_str());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::GetTileTree(ICancellableP cancel, DgnDbR db, Utf8StringCR idStr, Napi::Function& callback)
    {
    // If the tree already exists, return it from the main thread rather than sending to thread pool...
    BeJsNapiObject result(Env());
    auto status = T_HOST.Visualization().GetTileTreeJson(result, db, idStr, false);
    if (DgnDbStatus::NotFound == status)
      {
      // Initialize on thread pool.
      auto worker = new GetTileTreeWorker(cancel, callback, db, idStr);
      worker->Queue();
    }
    else
      {
      auto retval = DgnDbStatus::Success == status ? CreateBentleyReturnSuccessObject(result) : CreateBentleyReturnErrorObject(status, nullptr, Env());
      callback.Call({retval});
      }
    }

//=======================================================================================
// @bsistruct
//=======================================================================================
struct GetTileContentWorker : TileWorker
{
private:
    // Input
    Utf8String m_contentId;

    // Output
    TileContentCPtr m_result;

    void Execute() final
        {
        if (!PreExecute())
            return;

        // JsInterop::GetLogger().debugv("GetTileContentWorker::Execute: %s %s", m_treeId.ToString().c_str(), m_contentId.ToString().c_str());
        auto result = T_HOST.Visualization().GetTileContent(*m_db, m_treeId, m_contentId);
#ifdef BENTLEY_HAVE_VARIANT
        if (std::holds_alternative<TileContentCPtr>(result)) {
          m_result = std::get<TileContentCPtr>(result);
          BeAssert(m_result.IsValid());
          m_status = m_result.IsValid() ? DgnDbStatus::Success : DgnDbStatus::NotFound;
        } else {
          BeAssert(std::holds_alternative<DgnDbStatus>(result));
          m_status = std::get<DgnDbStatus>(result);
        }
#else
        if (result.m_content.IsValid()) {
          m_result = result.m_content;
          m_status = DgnDbStatus::Success;
        } else {
          m_status = result.m_status;
        }
#endif
        }

    Napi::Value GetTileResult() final
        {
        BeAssert(m_result.IsValid());

        ByteStreamCR geometry = m_result->GetBytes();
        auto blob = Napi::Uint8Array::New(Env(), geometry.size());
        memcpy(blob.Data(), geometry.data(), geometry.size());

        return blob;
        }
public:
    GetTileContentWorker(ICancellableP cancel, Napi::Function& callback, DgnDbR db, Utf8StringCR treeId, Utf8StringCR contentId)
        : TileWorker(cancel, callback, db, treeId), m_contentId(contentId)
        {
        // JsInterop::GetLogger().debugv("GetTileContentWorker ctor: %s %s", m_treeId.ToString().c_str(), m_contentId.ToString().c_str());
        }

    ~GetTileContentWorker()
        {
        // JsInterop::GetLogger().debugv("GetTileContentWorker dtor: %s %s", m_treeId.ToString().c_str(), m_contentId.ToString().c_str());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::GetTileContent(ICancellableP cancel, DgnDbR db, Utf8StringCR treeIdStr, Utf8StringCR tileIdStr, Napi::Function& callback)
    {
    auto worker = new GetTileContentWorker(cancel, callback, db, treeIdStr, tileIdStr);
    worker->Queue();
    }

//=======================================================================================
// Projects the NativeECPresentationManager class into JS.
//! @bsiclass
//=======================================================================================
struct NativeECPresentationManager : BeObjectWrap<NativeECPresentationManager>
    {
    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct SchemasLoader : DgnDbWorker
        {
        private:
            std::shared_ptr<ECPresentation::Diagnostics::Scope> m_diagnostics;
        protected:
            void Execute() final
                {
                ECPresentation::Diagnostics::Scope::Holder scope(m_diagnostics);
                if (!GetDb().IsDbOpen())
                    {
                    SetError("iModel not open");
                    return;
                    }
                GetDb().Schemas().GetSchemas(true);

                m_output.SetEmptyObject();
                m_output["result"].SetNull();
                m_output["diagnostics"].From(m_diagnostics->BuildJson());
                }

        public:
            SchemasLoader(Napi::Env env, DgnDb& db) : DgnDbWorker(db, env)
                {
                ECPresentation::Diagnostics::Options loggerOptions;
                loggerOptions.SetMinimumDuration((uint64_t)0);
                m_diagnostics = *ECPresentation::Diagnostics::Scope::ResetAndCreate("Preloading ECSchemas", loggerOptions);
                }
        };

    DEFINE_CONSTRUCTOR;

    std::unique_ptr<ECPresentationManager> m_presentationManager;
    RefCountedPtr<SimpleRuleSetLocater> m_ruleSetLocater;
    RuntimeJsonLocalState m_localState;
    std::shared_ptr<IModelJsECPresentationUpdateRecordsHandler> m_updateRecords;
    std::shared_ptr<IModelJsECPresentationUiStateProvider> m_uiStateProvider;
    Napi::ThreadSafeFunction m_threadSafeFunc;

    static bool InstanceOf(Napi::Value val) {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECPresentationManager", {
            InstanceMethod("forceLoadSchemas", &NativeECPresentationManager::ForceLoadSchemas),
            InstanceMethod("setupRulesetDirectories", &NativeECPresentationManager::SetupRulesetDirectories),
            InstanceMethod("setupSupplementalRulesetDirectories", &NativeECPresentationManager::SetupSupplementalRulesetDirectories),
            InstanceMethod("setRulesetVariableValue", &NativeECPresentationManager::SetRulesetVariableValue),
            InstanceMethod("unsetRulesetVariableValue", &NativeECPresentationManager::UnsetRulesetVariableValue),
            InstanceMethod("getRulesetVariableValue", &NativeECPresentationManager::GetRulesetVariableValue),
            InstanceMethod("getRulesets", &NativeECPresentationManager::GetRulesets),
            InstanceMethod("addRuleset", &NativeECPresentationManager::AddRuleset),
            InstanceMethod("removeRuleset", &NativeECPresentationManager::RemoveRuleset),
            InstanceMethod("clearRulesets", &NativeECPresentationManager::ClearRulesets),
            InstanceMethod("handleRequest", &NativeECPresentationManager::HandleRequest),
            InstanceMethod("getUpdateInfo", &NativeECPresentationManager::GetUpdateInfo),
            InstanceMethod("updateHierarchyState", &NativeECPresentationManager::UpdateHierarchyState),
            InstanceMethod("dispose", &NativeECPresentationManager::Terminate)
            });

        exports.Set("ECPresentationManager", t);

        SET_CONSTRUCTOR(t);
        }

    static Napi::Value CreateReturnValue(Napi::Env env, ECPresentationResult const& result, bool serializeResponse = false)
        {
        BeJsNapiObject retVal(env);
        if (result.IsError())
            {
            // error
            SaveErrorValue(retVal["error"], result.GetStatus(), result.GetErrorMessage().c_str());
            return retVal;
            }
        else
            {
            // success
            if (result.IsJsonCppResponse())
                {
                // jsoncpp response
                if (serializeResponse)
                    retVal["result"] = result.GetSerializedSuccessResponse();
                else
                    retVal["result"].From(result.GetJsonCppSuccessResponse());
                }
            else
                {
                // rapidjson response
                if (serializeResponse) {
                    auto str = result.GetSerializedSuccessResponse();
                    retVal["result"] = str.empty() ? "\"null\"" : str; // see note about null values for BeJsValue::Stringify
                } else
                    retVal["result"].From(result.GetSuccessResponse());
                }
            }
        if (!result.GetDiagnostics().IsNull())
            retVal["diagnostics"].From(result.GetDiagnostics());

        return retVal;
        }

    NativeECPresentationManager(NapiInfoCR info)
        : BeObjectWrap<NativeECPresentationManager>(info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, props);
        if (!props.Get("id").IsString())
            THROW_JS_TYPE_EXCEPTION("props.id must be a string");
        if (!props.Get("isChangeTrackingEnabled").IsBoolean())
            THROW_JS_TYPE_EXCEPTION("props.isChangeTrackingEnabled must be a boolean");
        if (!props.Get("taskAllocationsMap").IsObject())
            THROW_JS_TYPE_EXCEPTION("props.taskAllocationsMap must be an object");
        if (!props.Get("defaultFormats").IsObject())
            THROW_JS_TYPE_EXCEPTION("props.defaultFormats must be an object");
        if (!props.Get("cacheConfig").IsObject())
            THROW_JS_TYPE_EXCEPTION("props.cacheConfig must be an object");

        try
            {
            m_updateRecords = std::make_shared<IModelJsECPresentationUpdateRecordsHandler>();
            m_uiStateProvider = std::make_shared<IModelJsECPresentationUiStateProvider>();
            m_presentationManager = std::unique_ptr<ECPresentationManager>(ECPresentationUtils::CreatePresentationManager(T_HOST.GetIKnownLocationsAdmin(),
                m_localState, m_updateRecords, m_uiStateProvider, props));
            m_ruleSetLocater = SimpleRuleSetLocater::Create();
            m_presentationManager->GetLocaters().RegisterLocater(*m_ruleSetLocater);
            m_threadSafeFunc = Napi::ThreadSafeFunction::New(Env(), Napi::Function::New(Env(), [](NapiInfoCR info) {}), "NativeECPresentationManager", 0, 1);
            }
        catch (std::exception const& e)
            {
            THROW_JS_EXCEPTION(e.what());
            }
        catch (...)
            {
            THROW_JS_EXCEPTION("Unknown initialization error");
            }
        }

    ~NativeECPresentationManager()
        {
        SetInDestructor();  // Must be the first line of every ObjectWrap destructor
        // 'terminate' not called
        BeAssert(m_presentationManager == nullptr);
        }

    Napi::Value CreateReturnValue(ECPresentationResult const& result, bool serializeResponse = false)
        {
        return CreateReturnValue(Env(), result, serializeResponse);
        }

    void ResolvePromise(Napi::Promise::Deferred const& deferred, ECPresentationResult&& result)
        {
        std::shared_ptr<ECPresentationResult> resultPtr = std::make_shared<ECPresentationResult>(std::move(result));
        m_threadSafeFunc.BlockingCall([this, resultPtr, deferred = std::move(deferred)](Napi::Env, Napi::Function)
            {
            deferred.Resolve(CreateReturnValue(*resultPtr, true));
            });
        }

    Napi::Value HandleRequest(NapiInfoCR info)
        {
        Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(Env());
        Napi::Object response = Napi::Object::New(Env());
        response.Set("result", deferred.Promise());
        response.Set("cancel", Napi::Function::New(Env(), [](NapiInfoCR info) {}));

        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db);
        if (!db->IsOpen())
            {
            deferred.Resolve(CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "db: not open")));
            return response;
            }

        REQUIRE_ARGUMENT_STRING(1, serializedRequest);
        rapidjson::Document requestJson;
        requestJson.Parse(serializedRequest.c_str());
        if (requestJson.IsNull())
            {
            deferred.Resolve(CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "request")));
            return response;
            }

        Utf8CP requestId = requestJson["requestId"].GetString();
        if (Utf8String::IsNullOrEmpty(requestId))
            {
            deferred.Resolve(CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "request.requestId")));
            return response;
            }

        auto startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        auto requestGuid = BeGuid(true).ToString();

        static rapidjson::Value const s_nullValue;
        RapidJsonValueCR params = requestJson.HasMember("params") ? requestJson["params"] : s_nullValue;

        ECPresentationUtils::GetLogger().debugv("Received request: %s. Assigned GUID: %s. Request params: %s",
            requestId, requestGuid.c_str(), BeRapidJsonUtilities::ToString(params).c_str());

        m_presentationManager->GetConnections().CreateConnection(db->GetDgnDb());

        try
            {
            auto diagnostics = ECPresentation::Diagnostics::Scope::ResetAndCreate(requestId, ECPresentationUtils::CreateDiagnosticsOptions(params));
            std::shared_ptr<folly::Future<ECPresentationResult>> result = std::make_shared<folly::Future<ECPresentationResult>>(folly::makeFuture(ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("request.requestId = '%s'", requestId))));
            if (0 == strcmp("GetRootNodesCount", requestId))
                *result = ECPresentationUtils::GetRootNodesCount(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetRootNodes", requestId))
                *result = ECPresentationUtils::GetRootNodes(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetChildrenCount", requestId))
                *result = ECPresentationUtils::GetChildrenCount(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetChildren", requestId))
                *result = ECPresentationUtils::GetChildren(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetNodePaths", requestId))
                *result = ECPresentationUtils::GetNodesPaths(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetFilteredNodePaths", requestId))
                *result = ECPresentationUtils::GetFilteredNodesPaths(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetContentSources", requestId))
                *result = ECPresentationUtils::GetContentSources(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetContentDescriptor", requestId))
                *result = ECPresentationUtils::GetContentDescriptor(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetContent", requestId))
                *result = ECPresentationUtils::GetContent(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetContentSetSize", requestId))
                *result = ECPresentationUtils::GetContentSetSize(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetPagedDistinctValues", requestId))
                *result = ECPresentationUtils::GetPagedDistinctValues(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetDisplayLabel", requestId))
                *result = ECPresentationUtils::GetDisplayLabel(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("CompareHierarchies", requestId))
                *result = ECPresentationUtils::CompareHierarchies(*m_presentationManager, db->GetDgnDb(), params);

            (*result)
            .then([this, requestGuid, startTime, diagnostics = *diagnostics, deferred = std::move(deferred)](ECPresentationResult result)
                {
                result.SetDiagnostics(diagnostics->BuildJson());
                ResolvePromise(deferred, std::move(result));
                ECPresentationUtils::GetLogger().debugv("Request %s completed successfully in %" PRIu64 " ms.",
                    requestGuid.c_str(), (BeTimeUtilities::GetCurrentTimeAsUnixMillis() - startTime));
                })
            .onError([this, requestGuid, startTime, diagnostics = *diagnostics, deferred = std::move(deferred)](folly::exception_wrapper e)
                {
                ECPresentationResult result = ECPresentationUtils::CreateResultFromException(e);
                result.SetDiagnostics(diagnostics->BuildJson());

                if (ECPresentationStatus::Canceled == result.GetStatus())
                    {
                    ECPresentationUtils::GetLogger().debugv("Request %s cancelled after %" PRIu64 " ms.",
                        requestGuid.c_str(), (BeTimeUtilities::GetCurrentTimeAsUnixMillis() - startTime));
                    }
                else
                    {
                    ECPresentationUtils::GetLogger().errorv("Request %s completed with error '%s' in %" PRIu64 " ms.",
                        requestGuid.c_str(), result.GetErrorMessage().c_str(), (BeTimeUtilities::GetCurrentTimeAsUnixMillis() - startTime));
                    }

                ResolvePromise(deferred, std::move(result));
                });

            response.Set("cancel", Napi::Function::New(Env(), [result](NapiInfoCR info)
                {
                result->cancel();
                }));
            }
        catch (...)
            {
            ECPresentationUtils::GetLogger().errorv("Failed to queue request %s", requestGuid.c_str());
            deferred.Resolve(CreateReturnValue(ECPresentationResult(ECPresentationStatus::Error, "Unknown error creating request")));
            }

        return response;
        }

    Napi::Value ForceLoadSchemas(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db);
        if (!db->IsOpen())
            THROW_JS_EXCEPTION("NativeECPresentationManager::ForceLoadSchemas: iModel not open");

        DgnDbWorkerPtr worker = new SchemasLoader(info.Env(), db->GetDgnDb());
        return worker->Queue();
        }

    Napi::Value SetupRulesetDirectories(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, directories);
        ECPresentationResult result = ECPresentationUtils::SetupRulesetDirectories(*m_presentationManager, directories);
        return CreateReturnValue(result);
        }

    Napi::Value SetupSupplementalRulesetDirectories(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, directories);
        ECPresentationResult result = ECPresentationUtils::SetupSupplementalRulesetDirectories(*m_presentationManager, directories);
        return CreateReturnValue(result);
        }

    Napi::Value GetRulesets(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId);
        ECPresentationResult result = ECPresentationUtils::GetRulesets(*m_ruleSetLocater, rulesetId);
        return CreateReturnValue(result, true);
        }

    Napi::Value AddRuleset(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetJsonString);
        ECPresentationResult result = ECPresentationUtils::AddRuleset(*m_ruleSetLocater, rulesetJsonString);
        return CreateReturnValue(result);
        }

    Napi::Value RemoveRuleset(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId);
        REQUIRE_ARGUMENT_STRING(1, hash);
        ECPresentationResult result = ECPresentationUtils::RemoveRuleset(*m_ruleSetLocater, rulesetId, hash);
        return CreateReturnValue(result);
        }

    Napi::Value ClearRulesets(NapiInfoCR info)
        {
        ECPresentationResult result = ECPresentationUtils::ClearRulesets(*m_ruleSetLocater);
        return CreateReturnValue(result);
        }

    Napi::Value GetRulesetVariableValue(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId);
        REQUIRE_ARGUMENT_STRING(1, variableId);
        REQUIRE_ARGUMENT_STRING(2, type);
        ECPresentationResult result = ECPresentationUtils::GetRulesetVariableValue(*m_presentationManager, rulesetId, variableId, type);
        return CreateReturnValue(result);
        }

    Napi::Value SetRulesetVariableValue(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, ruleSetId);
        REQUIRE_ARGUMENT_STRING(1, variableId);
        REQUIRE_ARGUMENT_STRING(2, variableType);
        REQUIRE_ARGUMENT_ANY_OBJ(3, value);
        ECPresentationResult result = ECPresentationUtils::SetRulesetVariableValue(*m_presentationManager, ruleSetId, variableId, variableType, value);
        return CreateReturnValue(result);
        }

    Napi::Value UnsetRulesetVariableValue(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, ruleSetId);
        REQUIRE_ARGUMENT_STRING(1, variableId);
        ECPresentationResult result = ECPresentationUtils::UnsetRulesetVariableValue(*m_presentationManager, ruleSetId, variableId);
        return CreateReturnValue(result);
        }

    Napi::Value UpdateHierarchyState(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db);
        REQUIRE_ARGUMENT_STRING(1, ruleSetId);
        REQUIRE_ARGUMENT_ANY_OBJ(2, stateChanges);
        ECPresentationResult result = m_uiStateProvider->UpdateHierarchyState(*m_presentationManager, db->GetDgnDb(), ruleSetId, stateChanges);
        return CreateReturnValue(result);
        }

    Napi::Value GetUpdateInfo(NapiInfoCR info)
        {
        return CreateReturnValue(ECPresentationResult(m_updateRecords->GetReport(), false));
        }

    void Terminate(NapiInfoCR info)
        {
        m_presentationManager.reset();
        m_threadSafeFunc.Release();
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void JsInterop::HandleAssertion(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
    {
    BeMutexHolder lock(s_assertionMutex);
    if (!s_assertionsEnabled)
        return;

    if (IsJsExecutionDisabled())
        {
        fwprintf(stderr, L"%ls %ls %d", msg, file, (int)line);
        return;
        }

    BeNapi::ThrowJsException(JsInterop::Env(), Utf8PrintfString("Native Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str());
    }

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct DisableNativeAssertions : BeObjectWrap<DisableNativeAssertions>
    {
    private:
        DEFINE_CONSTRUCTOR;
        bool m_enableOnDispose = false;

        void DoDispose()
            {
            if (!m_enableOnDispose)
                return;
            BeMutexHolder lock(s_assertionMutex);
            s_assertionsEnabled = true;
            //only re-enable assertions once. DoDispose is be called from the explicit
            //call to Dispose and later when the destructor is called
            m_enableOnDispose = false;
            }

    public:
        DisableNativeAssertions(NapiInfoCR info) : BeObjectWrap<DisableNativeAssertions>(info)
            {
            BeMutexHolder lock(s_assertionMutex);
            m_enableOnDispose = s_assertionsEnabled;
            s_assertionsEnabled = false;
            }

        ~DisableNativeAssertions() { SetInDestructor(); DoDispose(); }

        void Dispose(NapiInfoCR info) { DoDispose(); }

        static void Init(Napi::Env env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "DisableNativeAssertions", {
            InstanceMethod("dispose", &DisableNativeAssertions::Dispose),
            });

            exports.Set("DisableNativeAssertions", t);

            SET_CONSTRUCTOR(t);
            }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NativeImportContext : BeObjectWrap<NativeImportContext>
{
private:
    DEFINE_CONSTRUCTOR;
    DgnImportContext* m_importContext = nullptr;

public:
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ImportContext", {
            InstanceMethod("addClass", &NativeImportContext::AddClass),
            InstanceMethod("addCodeSpecId", &NativeImportContext::AddCodeSpecId),
            InstanceMethod("addElementId", &NativeImportContext::AddElementId),
            InstanceMethod("cloneElement", &NativeImportContext::CloneElement),
            InstanceMethod("dispose", &NativeImportContext::Dispose),
            InstanceMethod("dump", &NativeImportContext::Dump),
            InstanceMethod("filterSubCategoryId", &NativeImportContext::FilterSubCategoryId),
            InstanceMethod("findCodeSpecId", &NativeImportContext::FindCodeSpecId),
            InstanceMethod("findElementId", &NativeImportContext::FindElementId),
            InstanceMethod("hasSubCategoryFilter", &NativeImportContext::HasSubCategoryFilter),
            InstanceMethod("importCodeSpec", &NativeImportContext::ImportCodeSpec),
            InstanceMethod("importFont", &NativeImportContext::ImportFont),
            InstanceMethod("isSubCategoryFiltered", &NativeImportContext::IsSubCategoryFiltered),
            InstanceMethod("loadStateFromDb", &NativeImportContext::LoadStateFromDb),
            InstanceMethod("removeElementId", &NativeImportContext::RemoveElementId),
            InstanceMethod("saveStateToDb", &NativeImportContext::SaveStateToDb),
        });
        exports.Set("ImportContext", t);
        SET_CONSTRUCTOR(t);
        }

    NativeImportContext(NapiInfoCR info) : BeObjectWrap<NativeImportContext>(info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, sourceDb);
        REQUIRE_ARGUMENT_OBJ(1, NativeDgnDb, targetDb);
        m_importContext = new DgnImportContext(sourceDb->GetDgnDb(), targetDb->GetDgnDb());
        }

    ~NativeImportContext() {SetInDestructor();}

    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    void Dispose(NapiInfoCR info)
        {
        if (nullptr != m_importContext)
            {
            delete m_importContext;
            m_importContext = nullptr;
            }
        }

    Napi::Value Dump(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING(0, outputFileName);
        BentleyStatus status = m_importContext->Dump(outputFileName);
        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value AddClass(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING(0, sourceClassFullName);
        REQUIRE_ARGUMENT_STRING(1, targetClassFullName);
        bvector<Utf8String> sourceTokens;
        bvector<Utf8String> targetTokens;
        BeStringUtilities::Split(sourceClassFullName.c_str(), ".:", sourceTokens);
        BeStringUtilities::Split(targetClassFullName.c_str(), ".:", targetTokens);
        if ((2 != sourceTokens.size()) || (2 != targetTokens.size()))
            return Napi::Number::New(Env(), (int) DgnDbStatus::InvalidName);

        DgnClassId sourceClassId = m_importContext->GetSourceDb().Schemas().GetClassId(sourceTokens[0].c_str(), sourceTokens[1].c_str());
        DgnClassId targetClassId = m_importContext->GetDestinationDb().Schemas().GetClassId(targetTokens[0].c_str(), targetTokens[1].c_str());
        if ((!sourceClassId.IsValid()) || (!targetClassId.IsValid()))
            return Napi::Number::New(Env(), (int) DgnDbStatus::InvalidName);

        m_importContext->AddClassId(sourceClassId, targetClassId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value AddCodeSpecId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId);
        REQUIRE_ARGUMENT_STRING_ID(1, targetIdStr, CodeSpecId, targetId);
        m_importContext->AddCodeSpecId(sourceId, targetId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value AddElementId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceId);
        REQUIRE_ARGUMENT_STRING_ID(1, targetIdStr, DgnElementId, targetId);
        m_importContext->AddElementId(sourceId, targetId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value RemoveElementId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceId);
        m_importContext->AddElementId(sourceId, DgnElementId());
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value FindCodeSpecId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId);
        CodeSpecId targetId = m_importContext->FindCodeSpecId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value FindElementId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceId);
        DgnElementId targetId = m_importContext->FindElementId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value CloneElement(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceElementId);
        bool binaryGeometry = false;
        if (!ARGUMENT_IS_EMPTY(1))
            {
            Napi::Object cloneOptions = info[1].As<Napi::Object>();
            binaryGeometry = cloneOptions.Get("binaryGeometry").ToBoolean();
            }

        DgnElementCPtr sourceElement = m_importContext->GetSourceDb().Elements().GetElement(sourceElementId);
        if (!sourceElement.IsValid())
            THROW_JS_EXCEPTION("Invalid source ElementId");

        // NOTE: Elements and Models share the same mapping table. However, the root Subject can be remapped but not the RepositoryModel
        DgnModelId targetModelId;
        if (sourceElement->GetModelId() == DgnModel::RepositoryModelId())
            targetModelId = DgnModel::RepositoryModelId();
        else
            targetModelId = m_importContext->FindModelId(sourceElement->GetModelId());

        DgnModelPtr targetModel = m_importContext->GetDestinationDb().Models().GetModel(targetModelId);
        if (!targetModel.IsValid())
            THROW_JS_EXCEPTION("Invalid target model");

        DgnElementPtr targetElement = sourceElement->CloneForImport(nullptr, *targetModel, *m_importContext);
        if (!targetElement.IsValid())
            THROW_JS_EXCEPTION("Unable to clone element");

        GeometryStreamCP geometryStream = nullptr;
        GeometrySourceCP geometrySource = targetElement->ToGeometrySource();
        if (nullptr != geometrySource)
            {
            if (geometrySource->GetGeometryStream().HasGeometry())
                geometryStream = &geometrySource->GetGeometryStream();
            }
        else
            {
            DgnGeometryPartCP geometryPart = targetElement->ToGeometryPart();
            if ((nullptr != geometryPart) && (geometryPart->GetGeometryStream().HasGeometry()))
                geometryStream = &geometryPart->GetGeometryStream();
            }

        // -------------------------------------------------------------------------------------------------------------
        // NOTE: Must include geometry (if present) either as JSON or binary since insert will happen on TypeScript side
        // NOTE: JSON is easier to work with, but binary is significantly faster
        // -------------------------------------------------------------------------------------------------------------

        BeJsDocument toJsonOptions;
        if ((nullptr != geometryStream) && !binaryGeometry)
            {
            toJsonOptions["wantGeometry"] = true;
            toJsonOptions["wantBRepData"] = true;
            }

        BeJsNapiObject val(Env());
        targetElement->ToJson(val, toJsonOptions);

        if ((nullptr != geometryStream) && binaryGeometry)
            val["geomBinary"].SetBinary(geometryStream->GetData(), geometryStream->GetSize());

        return val;
        }

    Napi::Value ImportCodeSpec(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId);
        CodeSpecId targetId = m_importContext->RemapCodeSpecId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value ImportFont(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_NUMBER(0, sourceFontNumber);
        FontId sourceFontId(static_cast<uint64_t>(sourceFontNumber.Uint32Value())); // BESERVER_ISSUED_ID_CLASS can be represented as a number in TypeScript
        FontId targetFontId = m_importContext->RemapFont(sourceFontId);
        return Napi::Number::New(Env(), targetFontId.GetValue());
        }

    Napi::Value HasSubCategoryFilter(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        return Napi::Boolean::New(Env(), m_importContext->HasSubCategoryFilter());
        }

    Napi::Value IsSubCategoryFiltered(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, subCategoryIdStr, DgnSubCategoryId, subCategoryId);
        return Napi::Boolean::New(Env(), m_importContext->IsSubCategoryFiltered(subCategoryId));
        }

    Napi::Value FilterSubCategoryId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_STRING_ID(0, subCategoryIdStr, DgnSubCategoryId, subCategoryId);
        m_importContext->FilterSubCategoryId(subCategoryId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value SaveStateToDb(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_OBJ(0, SQLiteDb, db);
        if (nullptr == db)
            THROW_JS_EXCEPTION("Invalid SQLiteDb");
        DbResult result = m_importContext->SaveStateToDb(db->GetDb());
        if (result != DbResult::BE_SQLITE_OK) THROW_JS_EXCEPTION("Failed to serialize the state");
        return Env().Undefined();
        }

    Napi::Value LoadStateFromDb(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_EXCEPTION("Invalid NativeImportContext");

        REQUIRE_ARGUMENT_OBJ(0, SQLiteDb, db);
        if (nullptr == db)
            THROW_JS_EXCEPTION("Invalid SQLiteDb");
        DbResult result = m_importContext->LoadStateFromDb(db->GetDb());
        if (result != DbResult::BE_SQLITE_OK) THROW_JS_EXCEPTION("Failed to load the state");
        return Env().Undefined();
        }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct NativeDevTools : BeObjectWrap<NativeDevTools>
{
private:
static Napi::Value Signal(NapiInfoCR info)
    {
    if (info.Length() == 0 || !info[0].IsNumber())
        THROW_JS_TYPE_EXCEPTION("Must supply SignalType");
    SignalType signalType = (SignalType) info[0].As<Napi::Number>().Int32Value();
    bool status = SignalTestUtility::Signal(signalType);
    return Napi::Number::New(info.Env(), status);
    }

public:
NativeDevTools(NapiInfoCR info) : BeObjectWrap<NativeDevTools>(info) {}
~NativeDevTools() {SetInDestructor();}

static void Init(Napi::Env env, Napi::Object exports)
    {
    Napi::HandleScope scope(env);
    Napi::Function t = DefineClass(env, "NativeDevTools", {
        StaticMethod("signal", &NativeDevTools::Signal),
    });
    exports.Set("NativeDevTools", t);
    }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECNameValidation : BeObjectWrap<ECNameValidation>
{
public:
    ECNameValidation(NapiInfoCR info) : BeObjectWrap<ECNameValidation>(info) {}
    ~ECNameValidation() {SetInDestructor();}

    static Napi::Value EncodeToValidName(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, name);
        Utf8String encoded;
        ECN::ECNameValidation::EncodeToValidName(encoded, name);
        return Napi::String::New(info.Env(), encoded.c_str());
        }

    static Napi::Value DecodeFromValidName(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, name);
        Utf8String decoded;
        ECN::ECNameValidation::DecodeFromValidName(decoded, name);
        return Napi::String::New(info.Env(), decoded.c_str());
        }

    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECNameValidation", {
            StaticMethod("encodeToValidName", &ECNameValidation::EncodeToValidName),
            StaticMethod("decodeFromValidName", &ECNameValidation::DecodeFromValidName),
        });
        exports.Set("ECNameValidation", t);
        }
};

/** compute the SHA1 checksum for a schema XML file */
static Napi::Value computeSchemaChecksum(NapiInfoCR info) {
    REQUIRE_ARGUMENT_ANY_OBJ(0, arg);
    auto schemaPath = requireString(arg, "schemaXmlPath");
    auto refPaths = requireArray(arg, "referencePaths");

    std::unordered_set<Utf8String> paths;
    auto num_locations = refPaths.Length();
    for (unsigned int i = 0; i < num_locations; i++) {
        auto value = refPaths.Get(i).As<Napi::String>();
        paths.emplace(value.Utf8Value());
    }

    auto exactMatch = boolMember(arg, "exactMatch", false);
    Utf8String message;
    auto sha1 = exactMatch ? SchemaUtil::ComputeChecksumWithExactRefMatch(message, schemaPath, paths) : SchemaUtil::ComputeChecksum(message, schemaPath, paths);
    if ("" == sha1)
        THROW_JS_EXCEPTION(message.c_str());

    return Napi::String::New(info.Env(), sha1);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void setMaxTileCacheSize(NapiInfoCR info) {
  if (info.Length() == 1 && info[0].IsNumber()) {
    auto maxBytes = info[0].As<Napi::Number>().DoubleValue();
    if (maxBytes >= 0) {
      JsInterop::SetMaxTileCacheSize(static_cast<uint64_t>(maxBytes));
      return;
    }
  }

  JsInterop::GetNativeLogger().error("Invalid argument for setMaxTileCacheSize: expected an unsigned integer");
}

static void flushLog(NapiInfoCR info) {
  s_jsLogger.FlushDeferred();
}
static Napi::Value getLogger(NapiInfoCR info) {
  return s_jsLogger.getJsLogger();
}
static void setLogger(NapiInfoCR info) {
  s_jsLogger.setJsLogger(info);
}
static void clearLogLevelCache(NapiInfoCR){
    s_jsLogger.SyncLogLevels();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getTileVersionInfo(NapiInfoCR info)
    {
    auto formatVersion = T_HOST.Visualization().GetCurrentFormatVersion();
    auto result = Napi::Object::New(info.Env());
    result["formatVersion"] = formatVersion;
    return result;
    }

/** add a WorkspaceDb to the list of available gcs workspaceDbs */
static Napi::Value addGcsWorkspaceDb(NapiInfoCR info) {
    REQUIRE_ARGUMENT_STRING(0, dbName);
    OPTIONAL_ARGUMENT_INTEGER(2, priority, 100);
    return Napi::Boolean::New(info.Env(), GeoCoordinates::BaseGCS::AddWorkspaceDb(dbName, getCloudContainer(info[1]), priority));
}

/** enable loading a gcs data files from the local directory in addition to workspaceDbs */
static void enableLocalGcsFiles(NapiInfoCR info) {
    REQUIRE_ARGUMENT_BOOL(0, yesNo);
    GeoCoordinates::BaseGCS::EnableLocalGcsFiles(yesNo);
}

/** add a WorkspaceDb to the list of available font workspaceDbs */
static Napi::Value addFontWorkspace(NapiInfoCR info) {
   REQUIRE_ARGUMENT_STRING(0, fileName);
   return Napi::Boolean::New(info.Env(), FontManager::AddWorkspaceDb(fileName.c_str(), getCloudContainer(info[1])));
}

static bool s_crashReportingInitialized;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void setCrashReporting(NapiInfoCR info)
    {
    if (s_crashReportingInitialized)
        {
        JsInterop::GetNativeLogger().warning("Crash reporting is already initialized.");
        return;
        }

    if ((info.Length() != 1) || !info[0].IsObject())
        THROW_JS_TYPE_EXCEPTION("Argument must be CrashReportingConfig object");

    Napi::Object obj = info[0].As<Napi::Object>();

    JsInterop::CrashReportingConfig ccfg;
    ccfg.m_crashDir.SetNameA(stringMember(obj, "crashDir").c_str());
    ccfg.m_dumpProcessorScriptFileName.SetNameA(stringMember(obj, "dumpProcessorScriptFileName").c_str());
    ccfg.m_enableCrashDumps         = boolMember(obj, "enableCrashDumps", false);
    ccfg.m_maxDumpsInDir            = intMember(obj, "maxDumpsInDir", 50);
    ccfg.m_wantFullMemory           = boolMember(obj, "wantFullMemory", false);
    if (obj.Has("params"))
        {
        Napi::Array arr = obj.Get("params").As<Napi::Array>();
        for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {
            Napi::Value arrValue = arr[arrIndex];
            if (arrValue.IsObject())
                {
                auto item = arrValue.As<Napi::Object>();
                auto name = stringMember(item, "name");
                auto value = stringMember(item, "value");
                if (!name.empty())
                    ccfg.m_params[name] = value;
                }
            }
        }
#ifdef WIP_DUMP_UPLOAD
    ccfg.m_maxUploadRetries         = intMember(obj, "maxUploadRetries", 5);
    ccfg.m_uploadRetryWaitInterval  = intMember(obj, "uploadRetryWaitInterval", 10000);
    ccfg.m_maxReportsPerDay         = intMember(obj, "maxReportsPerDay", 1000);
    ccfg.m_uploadUrl                = stringMember(obj, "uploadUrl");
#endif
    ccfg.m_needsVectorExceptionHandler = true;
    JsInterop::InitializeCrashReporting(ccfg);

    s_crashReportingInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void setCrashReportProperty(NapiInfoCR info)
    {
    if (!s_crashReportingInitialized)
        THROW_JS_EXCEPTION("Crash reporting is not initialized.");

    REQUIRE_ARGUMENT_STRING(0, key);

    Utf8String value;
    Utf8CP valueStr = nullptr;
    if (!info[1].IsUndefined()) {
        if (ARGUMENT_IS_NOT_STRING(1)) {
            THROW_JS_TYPE_EXCEPTION("Argument 1 must be a string or undefined");
        }
        value = info[1].As<Napi::String>().Utf8Value();
        valueStr = value.c_str();
    }

    JsInterop::SetCrashReportProperty(key.c_str(), valueStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getCrashReportProperties(NapiInfoCR info)
    {
    if (!s_crashReportingInitialized)
        THROW_JS_EXCEPTION("Crash reporting is not initialized.");

    auto env = info.Env();

    auto props = JsInterop::GetCrashReportProperties();

    Napi::Object propertyArray = Napi::Array::New(env, props.size());

    int i = 0;
    for (auto const& prop : props)
        {
        Napi::Object nvpair = Napi::Object::New(env); // must conform to IModelJsNative.NativeCrashReportingConfigNameValuePair
        nvpair.Set(Napi::String::New(env, "name"), Napi::String::New(env, prop.first.c_str()));
        nvpair.Set(Napi::String::New(env, "value"), Napi::String::New(env, prop.second.c_str()));

        propertyArray.Set(Napi::Number::New(env, i), nvpair);
        ++i;
        }

    return propertyArray;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C"
    {
    void imodeljs_addon_setMobileResourcesDir(Utf8CP d) {s_mobileResourcesDir = d;}
    void imodeljs_addon_setMobileTempDir(Utf8CP d) {s_mobileTempDir = d;}
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void onNodeExiting(void*) {
    s_jsLogger.OnExit();
    PlatformLib::Terminate(true); // for orderly shut down of static objects
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value queryConcurrency(NapiInfoCR info)
    {
    size_t concurrency = 0;
    if (ARGUMENT_IS_STRING(0))
        {
        Utf8String poolName = info[0].As<Napi::String>().Utf8Value();
        if (poolName.Equals("cpu"))
            concurrency = BeFolly::ThreadPool::GetCpuPool().size();
        else if (poolName.Equals("io"))
            concurrency = BeFolly::ThreadPool::GetIoPool().size();
        }

    return Napi::Number::New(info.Env(), static_cast<int>(concurrency));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object registerModule(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

#if (defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)) || defined(BENTLEYCONFIG_OS_LINUX) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
    BeFileName addondir = Desktop::FileSystem::GetLibraryDir();
    BeFileName tempdir;
    Desktop::FileSystem::BeGetTempPath(tempdir);
#else
    BeFileName addondir(s_mobileResourcesDir);
    BeFileName tempdir(s_mobileTempDir);
#endif

    JsInterop::Initialize(addondir, env, tempdir);

    SQLiteDb::Init(env, exports);
    NativeBlobIo::Init(env, exports);
    NativeDgnDb::Init(env, exports);
    NativeGeoServices::Init(env, exports);
    NativeRevisionUtility::Init(env, exports);
    NativeECDb::Init(env, exports);
    NativeChangesetReader::Init(env, exports);
    NativeChangedElementsECDb::Init(env, exports);
    NativeECSqlStatement::Init(env, exports);
    NativeECSqlBinder::Init(env, exports);
    NativeECSqlValue::Init(env, exports);
    NativeECSqlColumnInfo::Init(env, exports);
    NativeECSqlValueIterator::Init(env, exports);
    NativeSqliteStatement::Init(env, exports);
    NativeECPresentationManager::Init(env, exports);
    NativeECSchemaXmlContext::Init(env, exports);
    SnapRequest::Init(env, exports);
    DisableNativeAssertions::Init(env, exports);
    DgnDbWorker::Init(env, exports);
    NativeImportContext::Init(env, exports);
    NativeDevTools::Init(env, exports);
    ECNameValidation::Init(env, exports);

    napi_add_env_cleanup_hook(env, onNodeExiting, nullptr);

    exports.DefineProperties({
        Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)),
        Napi::PropertyDescriptor::Accessor(env, exports, "logger", &getLogger, &setLogger),
        Napi::PropertyDescriptor::Function(env, exports, "addFontWorkspace", &addFontWorkspace),
        Napi::PropertyDescriptor::Function(env, exports, "addGcsWorkspaceDb", &addGcsWorkspaceDb),
        Napi::PropertyDescriptor::Function(env, exports, "clearLogLevelCache", &clearLogLevelCache),
        Napi::PropertyDescriptor::Function(env, exports, "computeSchemaChecksum", &computeSchemaChecksum),
        Napi::PropertyDescriptor::Function(env, exports, "enableLocalGcsFiles", &enableLocalGcsFiles),
        Napi::PropertyDescriptor::Function(env, exports, "flushLog", &flushLog),
        Napi::PropertyDescriptor::Function(env, exports, "getCrashReportProperties", &getCrashReportProperties),
        Napi::PropertyDescriptor::Function(env, exports, "getTileVersionInfo", &getTileVersionInfo),
        Napi::PropertyDescriptor::Function(env, exports, "queryConcurrency", &queryConcurrency),
        Napi::PropertyDescriptor::Function(env, exports, "setCrashReporting", &setCrashReporting),
        Napi::PropertyDescriptor::Function(env, exports, "setCrashReportProperty", &setCrashReportProperty),
        Napi::PropertyDescriptor::Function(env, exports, "setMaxTileCacheSize", &setMaxTileCacheSize),
});

    registerCloudSqlite(env, exports);

    return exports;
}

NODE_API_MODULE(iModelJsNative, registerModule)
} // namespace IModelJsNative
