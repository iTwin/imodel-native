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

template<typename T_Db> struct SQLiteOps {
    virtual T_Db* _GetMyDb() = 0;

    static FileProps getEmbedFileProps(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, optObj);
        BeJsValue opts(optObj);
        if (!opts.isStringMember(JsInterop::json_name()))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "name argument missing", IModelJsNativeErrorKey::BadArg);
        if (!opts.isStringMember(JsInterop::json_localFileName()))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "localFileName argument missing", IModelJsNativeErrorKey::BadArg);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "invalid date", IModelJsNativeErrorKey::BadArg);

        props.m_compress = opts[JsInterop::json_compress()].asBool(true);
        return props;
    }

    T_Db& GetOpenedDb(NapiInfoCR info) {
        auto* db = _GetMyDb();
        if (db == nullptr || !db->IsDbOpen())
           THROW_JS_DGN_DB_EXCEPTION(info.Env(), "db is not open", DgnDbStatus::NotOpen);

        return *db;
    }

    T_Db& GetWritableDb(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        if (db.IsReadonly())
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "db is not open for write", DgnDbStatus::NotOpenForWrite);

        return db;
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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid FilePropertyProps", IModelJsNativeErrorKey::BadArg);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "saveFileProperty requires 2 arguments", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_ANY_OBJ(0, fileProps);
        BeJsConst propsJson(fileProps);
        if (!propsJson.isMember(JsInterop::json_namespace()) || !propsJson.isMember(JsInterop::json_name()))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid FilePropertyProps", IModelJsNativeErrorKey::BadArg);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid FilePropertyProps", IModelJsNativeErrorKey::BadArg);

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

    void EmbedFontFile(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, id);
        REQUIRE_ARGUMENT_ANY_OBJ(1, facesObj);
        REQUIRE_ARGUMENT_ANY_OBJ(2, dataObj);
        REQUIRE_ARGUMENT_BOOL(3, compress);

        if (!dataObj.IsTypedArray() || !facesObj.IsArray()) {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "font data not valid", IModelJsNativeErrorKey::BadArg);
        }

        bvector<FontFace> faces;
        auto arr = facesObj.As<Napi::Array>();
        for (uint32_t i = 0; i < arr.Length(); i++) {
            Napi::Value v = arr[i];
            if (!v.IsObject()) {
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "font data not valid", IModelJsNativeErrorKey::BadArg);
            }

            FontFace face(v);
            faces.push_back(face);
        }

        if (faces.empty()) {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "font data not valid", IModelJsNativeErrorKey::BadArg);
        }

        auto db = &GetOpenedDb(info);
        if (db == nullptr)
            {
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "db is not open", DgnDbStatus::NotOpen);
            return;
            }
        auto dgnDb = dynamic_cast<DgnDbP>(db);
        std::unique_ptr<FontDb> fontDbHolder;

        FontDbP fontDb;
        if (dgnDb) {
            fontDb = &dgnDb->Fonts().m_fontDb;
        } else {
            fontDbHolder.reset(fontDb = new FontDb(*db, true));
        }

        auto arrayBuf = dataObj.As<Napi::Uint8Array>();
        if (SUCCESS != fontDb->EmbedFont(id, faces, ByteStream(arrayBuf.Data(), arrayBuf.ByteLength()), compress)) {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "unable to embed font", IModelJsNativeErrorKey::FontError);
        }
    }

    Napi::Value IsOpen(NapiInfoCR info) {
        auto db = _GetMyDb();
        return Napi::Boolean::New(info.Env(), nullptr != db && db->IsDbOpen());
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
struct NativeECDb : BeObjectWrap<NativeECDb>, SQLiteOps<ECDb> {
private:
    DEFINE_CONSTRUCTOR
    ECDb m_ecdb;

public:
    NativeECDb(NapiInfoCR info) : BeObjectWrap<NativeECDb>(info) {}
    ~NativeECDb() { SetInDestructor(); CloseDbIfOpen(); }
    ECDbR GetECDb() { return m_ecdb; }
    ECDb* _GetMyDb() override { return &m_ecdb; }

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
    void AttachDb(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, fileName);
        REQUIRE_ARGUMENT_STRING(1, alias);
        auto rc = GetOpenedDb(info).AttachDb(fileName.c_str(), alias.c_str());
        if (rc != BE_SQLITE_OK) {
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to attach file", rc);
        }
    }
    void DetachDb(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, alias);
        auto rc = GetOpenedDb(info).DetachDb(alias.c_str());
        if (rc != BE_SQLITE_OK) {
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to detach file", rc);
        }
    }
    void ConcurrentQueryExecute(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestObj);
        REQUIRE_ARGUMENT_FUNCTION(1, callback);
        JsInterop::ConcurrentQueryExecute(m_ecdb, requestObj, callback);
    }
    void ClearECDbCache(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::ClearECDbCache(db, info);
    }
    Napi::Value PatchJsonProperties(NapiInfoCR info) {
        return JsInterop::PatchJsonProperties(info);
    }
    Napi::Value ReadInstance(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::ReadInstance(db, info);
    }
    Napi::Value InsertInstance(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::InsertInstance(db, info);
    }
    Napi::Value UpdateInstance(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::UpdateInstance(db, info);
    }
    Napi::Value DeleteInstance(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::DeleteInstance(db, info);
    }
    Napi::Value ConcurrentQueryResetConfig(NapiInfoCR info) {
        if (info.Length() > 0 && info[0].IsObject()) {
            Napi::Object inConf = info[0].As<Napi::Object>();
            return JsInterop::ConcurrentQueryResetConfig(Env(), inConf);
        }
        return JsInterop::ConcurrentQueryResetConfig(Env());
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


    Napi::Value GetSchemaProps(NapiInfoCR info)  {
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        auto schema = m_ecdb.Schemas().GetSchema(schemaName, true);
        if (nullptr == schema)
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "schema not found", DgnDbStatus::NotFound);

        BeJsNapiObject props(info.Env());
        if (!schema->WriteToJsonValue(props))
           THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "unable to serialize schema", IModelJsNativeErrorKey::SchemaError);
        return props;
    }

    Napi::Value ImportSchema(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaPathName);
        DbResult status = JsInterop::ImportSchema(m_ecdb, BeFileName(schemaPathName.c_str(), true));
        return Napi::Number::New(Env(), (int)status);
    }
    void DropSchema(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        DropSchemaResult rc = m_ecdb.Schemas().DropSchema(schemaName);
        if (rc.GetStatus() != DropSchemaResult::Success) {
            BeNapi::ThrowJsException(info.Env(), rc.GetStatusAsString(), (int)rc.GetStatus(), {"schema-sync", rc.GetStatusAsString()});
        }
    }
    void SchemaSyncSetDefaultUri(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaSyncDbUriStr);
        LastErrorListener lastError(m_ecdb);
        auto rc = m_ecdb.Schemas().GetSchemaSync().SetDefaultSyncDbUri(schemaSyncDbUriStr.c_str());
        if (rc != SchemaSync::Status::OK) {
            if (lastError.HasError()) {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), rc);
            } else {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), Utf8PrintfString("fail to set default shared schema channel uri: %s", schemaSyncDbUriStr.c_str()).c_str(), rc);
            }
        }
    }
    Napi::Value SchemaSyncGetDefaultUri(NapiInfoCR info) {
        auto& syncDbUri = m_ecdb.Schemas().GetSchemaSync().GetDefaultSyncDbUri();
        if (syncDbUri.IsEmpty())
            return Env().Undefined();

        return Napi::String::New(Env(), syncDbUri.GetUri().c_str());
    }
    void SchemaSyncInit(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaSyncDbUriStr);
        REQUIRE_ARGUMENT_STRING(1, containerId);
        REQUIRE_ARGUMENT_BOOL(2, overrideContainer);
        auto syncDbUri = SchemaSync::SyncDbUri(schemaSyncDbUriStr.c_str());
        LastErrorListener lastError(m_ecdb);
        auto rc = m_ecdb.Schemas().GetSchemaSync().Init(syncDbUri, containerId, overrideContainer);
        if (rc != SchemaSync::Status::OK) {
            if (lastError.HasError()) {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), rc);
            } else {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), Utf8PrintfString("fail to initialize shared schema channel: %s", schemaSyncDbUriStr.c_str()).c_str(), rc);
            }
        }
    }

    Napi::Value SchemaSyncEnabled(NapiInfoCR info) {
        const auto isEnabled = !m_ecdb.Schemas().GetSchemaSync().GetInfo().IsEmpty();
        return Napi::Boolean::New(Env(), isEnabled);
    }

    Napi::Value SchemaSyncGetLocalDbInfo(NapiInfoCR info) {
        auto localDbInfo = m_ecdb.Schemas().GetSchemaSync().GetInfo();
        if (localDbInfo.IsEmpty()) {
            return Env().Undefined();
        }
        BeJsNapiObject obj(Env());
        localDbInfo.To(obj);
        return obj;
    }

    Napi::Value SchemaSyncGetSyncDbInfo(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaSyncDbUriStr);
         auto syncDbUri = SchemaSync::SyncDbUri(schemaSyncDbUriStr.c_str());
         auto syncDbInfo = syncDbUri.GetInfo();
         if (syncDbInfo.IsEmpty()) {
            return Env().Undefined();
        }
        BeJsNapiObject obj(Env());
        syncDbInfo.To(obj);
        return obj;
    }

    void SchemaSyncPull(NapiInfoCR info) {
        OPTIONAL_ARGUMENT_STRING(0, schemaSyncDbUriStr);
        auto syncDbUri = SchemaSync::SyncDbUri(schemaSyncDbUriStr.c_str());
        LastErrorListener lastError(m_ecdb);
        auto rc = m_ecdb.Schemas().GetSchemaSync().Pull(syncDbUri);
        if (rc != SchemaSync::Status::OK) {
            if (lastError.HasError()) {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), rc);
            } else {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), Utf8PrintfString("fail to pull changes from channel: %s", schemaSyncDbUriStr.c_str()).c_str(), rc);
            }
        }
    }

    void SchemaSyncPush(NapiInfoCR info) {
        OPTIONAL_ARGUMENT_STRING(0, schemaSyncDbUriStr);
        auto syncDbUri = SchemaSync::SyncDbUri(schemaSyncDbUriStr.c_str());
        LastErrorListener lastError(m_ecdb);
        auto rc = m_ecdb.Schemas().GetSchemaSync().Push(syncDbUri);
        if (rc != SchemaSync::Status::OK) {
            if (lastError.HasError()) {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), rc);
            } else {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), Utf8PrintfString("fail to push changes from channel: %s", schemaSyncDbUriStr.c_str()).c_str(), rc);
            }
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
            InstanceMethod("attachDb", &NativeECDb::AttachDb),
            InstanceMethod("detachDb", &NativeECDb::DetachDb),
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
            InstanceMethod("getSchemaProps", &NativeECDb::GetSchemaProps),
            InstanceMethod("importSchema", &NativeECDb::ImportSchema),
            InstanceMethod("isOpen", &NativeECDb::IsOpen),
            InstanceMethod("schemaSyncSetDefaultUri", &NativeECDb::SchemaSyncSetDefaultUri),
            InstanceMethod("schemaSyncGetDefaultUri", &NativeECDb::SchemaSyncGetDefaultUri),
            InstanceMethod("schemaSyncPull", &NativeECDb::SchemaSyncPull),
            InstanceMethod("schemaSyncPush", &NativeECDb::SchemaSyncPush),
            InstanceMethod("schemaSyncInit", &NativeECDb::SchemaSyncInit),
            InstanceMethod("schemaSyncEnabled", &NativeECDb::SchemaSyncEnabled),
            InstanceMethod("schemaSyncGetLocalDbInfo", &NativeECDb::SchemaSyncGetLocalDbInfo),
            InstanceMethod("schemaSyncGetSyncDbInfo", &NativeECDb::SchemaSyncGetSyncDbInfo),
            InstanceMethod("openDb", &NativeECDb::OpenDb),
            InstanceMethod("readInstance", &NativeECDb::ReadInstance),
            InstanceMethod("insertInstance", &NativeECDb::InsertInstance),
            InstanceMethod("updateInstance", &NativeECDb::UpdateInstance),
            InstanceMethod("deleteInstance", &NativeECDb::DeleteInstance),
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
        THROW_JS_IMODEL_NATIVE_EXCEPTION(arg.Env(), "cannot open for database for write - container write lock not held", IModelJsNativeErrorKey::LockNotHeld);

    dbName = params.SetFromContainer(dbName.c_str(), container);
};

//=======================================================================================
// Projects the BeSQLite::Db class into JS
//! @bsiclass
//=======================================================================================
struct SQLiteDb : Napi::ObjectWrap<SQLiteDb>, SQLiteOps<Db> {
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
        auto& db = GetOpenedDb(info);
        DbResult status = db.SaveChanges();
        if (status != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error in saveChanges", db.GetDbFileName(), status);
    }

    void AbandonChanges(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        DbResult status = db.AbandonChanges();
        if (status != BE_SQLITE_OK)
            JsInterop::throwSqlResult("error in abandonChanges", db.GetDbFileName(), status);
    }

    static void Init(Napi::Env env, Napi::Object exports) {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "SQLiteDb", {
            InstanceMethod("abandonChanges", &SQLiteDb::AbandonChanges),
            InstanceMethod("closeDb", &SQLiteDb::CloseDb),
            InstanceMethod("createDb", &SQLiteDb::CreateDb),
            InstanceMethod("dispose", &SQLiteDb::Dispose),
            InstanceMethod("embedFile", &SQLiteDb::EmbedFile),
            InstanceMethod("embedFontFile", &SQLiteDb::EmbedFontFile),
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

        ECN::ECSchemaReadContextPtr GetContext() { return m_context; }

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

    void OnError(Napi::Error const& e) {
        if (e.Message() == "schema not found")
            e.Value()["errorNumber"] = (int) DgnDbStatus::NotFound;
        DgnDbWorker::OnError(e);
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
struct NativeDgnDb : BeObjectWrap<NativeDgnDb>, SQLiteOps<DgnDb>
    {
    static constexpr int ERROR_UsageTrackingFailed = -100;

    DEFINE_CONSTRUCTOR;

    DgnDb* _GetMyDb() override { return m_dgndb.get(); }

    // Access this only in contexts where we are manipulating the pointer or directly, or have verified it is non-null.
    // Most methods should use GetOpenedDb or GetWritableDb.
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

    // Throws a C++ exception if m_dgndb is null.
    // Most methods should use GetOpenedDb or GetWritableDb.
    // Use this only in rare contexts where you have previously verified that m_dgndb is non-null, e.g., via GetOpenedDb().
    DgnDbR GetDgnDb() {
        if (m_dgndb.IsNull())
            throw std::runtime_error("DgnDb is null");

        return *m_dgndb;
    }

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg = nullptr) {return IModelJsNative::CreateBentleyReturnErrorObject(errCode, msg, Env());}

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode, Napi::Value goodValue) {return ((STATUSTYPE)0 != errCode) ? CreateBentleyReturnErrorObject(errCode) : CreateBentleyReturnSuccessObject(goodValue); }

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode) {return CreateBentleyReturnObject(errCode, Env().Undefined());}

    bool IsOpen() const {return m_dgndb.IsValid();}

    Napi::Value IsDgnDbOpen(NapiInfoCR info) { return Napi::Boolean::New(Env(), IsOpen()); }

    // Napi::Value IsReadonly(NapiInfoCR info) { return Napi::Boolean::New(Env(), m_dgndb->IsReadonly()); }

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

        DgnDbPtr dgndb = m_dgndb;
        ClearDgnDb();
        dgndb->CloseDb();

        if (!fromDestructor)
            Value().Set(JSON_NAME(cloudContainer), Env().Undefined());
    }

    void SetBusyTimeout(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, ms);
        if (!m_dgndb.IsValid() || BE_SQLITE_OK != m_dgndb->SetBusyTimeout(ms))
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "unable to set busyTimeout", DgnDbStatus::TimeoutFailed);
    }

    void OpenIModel(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        REQUIRE_ARGUMENT_INTEGER(1, mode);

        SchemaUpgradeOptions::DomainUpgradeOptions domainOptions = SchemaUpgradeOptions::DomainUpgradeOptions::CheckRequiredUpgrades;
        BeSQLite::Db::ProfileUpgradeOptions profileOptions = BeSQLite::Db::ProfileUpgradeOptions::None;
        bool schemaLockHeld = false;

        BeJsConst opts(info[2]);
        if (opts.isObject()) {
            if (opts.isNumericMember("domain"))
                domainOptions = (SchemaUpgradeOptions::DomainUpgradeOptions) opts["domain"].asUInt();

            if (opts.isNumericMember("profile"))
                profileOptions = (BeSQLite::Db::ProfileUpgradeOptions) opts["profile"].asUInt();

            schemaLockHeld = opts[JSON_NAME(schemaLockHeld)].asBool(false);
        }

        SchemaUpgradeOptions schemaUpgradeOptions(domainOptions);
        DgnDb::OpenParams openParams((Db::OpenMode)mode, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);
        openParams.SetProfileUpgradeOptions(profileOptions);
        openParams.m_schemaLockHeld = schemaLockHeld;


        BeJsConst props(info[3]);
        if (props.isObject()) {
            auto tempFileBase = props[JSON_NAME(tempFileBase)];
            if (tempFileBase.isString())
                openParams.m_tempfileBase = tempFileBase.asString();
        }

        addContainerParams(Value(), dbName, openParams, info[4]);

        if (!openParams.IsReadonly()) {
            // 20 sec to retain previous behaviour.
            openParams.m_busyTimeout =  20000;
        }
        if (info.Length() > 5) {
            BeJsConst sqliteOptions(info[5]);
            if (sqliteOptions.isObject()) {
                if (sqliteOptions.isNumericMember("busyTimeout")) {
                    openParams.m_busyTimeout = sqliteOptions["busyTimeout"].asInt();
                }
            }
        }
        OpenIModelDb(BeFileName(dbName), openParams);
    }
    Napi::Value GetNoCaseCollation(NapiInfoCR info){
        auto& db = GetOpenedDb(info);
        if (db.GetNoCaseCollation() == NoCaseCollation::ASCII)
            return Napi::String::New(Env(), "ASCII");
        else if (db.GetNoCaseCollation() == NoCaseCollation::Latin1)
            return Napi::String::New(Env(), "Latin1");
        THROW_JS_TYPE_EXCEPTION("unknown collation");
    }

    void SetNoCaseCollation(NapiInfoCR info){
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, collationName);
        if (collationName.EqualsIAscii("ASCII")) {
            if(db.GetNoCaseCollation() != NoCaseCollation::ASCII){
                auto rc = db.SetNoCaseCollation(NoCaseCollation::ASCII);
                if (rc != BE_SQLITE_OK)
                    THROW_JS_TYPE_EXCEPTION("failed to set case collation.");
            }
        } else  if (collationName.EqualsIAscii("Latin1")) {
            if(db.GetNoCaseCollation() != NoCaseCollation::Latin1){
                db.ClearECDbCache();
                db.GetStatementCache().Empty();
                auto rc = db.SetNoCaseCollation(NoCaseCollation::Latin1);
                if (rc != BE_SQLITE_OK)
                    THROW_JS_TYPE_EXCEPTION("failed to set case collation.");
            }
        } else {
            THROW_JS_TYPE_EXCEPTION("unknown collation");
        }
    }
    void RestartDefaultTxn(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        auto& txns = db.Txns();
        auto last = txns.GetLastTxnId(); // save this before we restart
        db.RestartDefaultTxn();
        txns.ReplayExternalTxns(last); // if there were changes from other connections, replay their side effects for listeners
    }

    void CreateIModel(NapiInfoCR info)  {
        REQUIRE_ARGUMENT_STRING(0, filename);
        REQUIRE_ARGUMENT_ANY_OBJ(1, props);
        SetDgnDb(*JsInterop::CreateIModel(filename, props)); // CreateIModel throws on errors
    }

    Napi::Value IsSubClassOf(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, childClassFullName);
        REQUIRE_ARGUMENT_STRING(1, parentClassFullName);
        auto& db = GetOpenedDb(info);;
        return Napi::Boolean::New(Env(), db.Schemas().IsSubClassOf(childClassFullName, parentClassFullName));
    }

    Napi::Value GetECClassMetaData(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, s);
        REQUIRE_ARGUMENT_STRING(1, c);
        BeJsNapiObject metaData(Env());
        auto status = JsInterop::GetECClassMetaData(metaData, GetOpenedDb(info), s.c_str(), c.c_str());
        return CreateBentleyReturnObject(status, NapiValueRef::Stringify(metaData));
        }

    Napi::Value GetSchemaItem(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        REQUIRE_ARGUMENT_STRING(1, itemName);
        BeJsNapiObject metaData(Env());
        auto status = JsInterop::GetSchemaItem(metaData, GetOpenedDb(info), schemaName.c_str(), itemName.c_str());
        return CreateBentleyReturnObject(status, NapiValueRef::Stringify(metaData));
        }

    Napi::Value GetSchemaProps(NapiInfoCR info)  {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        auto schema = db.Schemas().GetSchema(schemaName, true);
        if (nullptr == schema)
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "schema not found", DgnDbStatus::NotFound);

        BeJsNapiObject props(info.Env());
        if (!schema->WriteToJsonValue(props))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "unable to serialize schema", IModelJsNativeErrorKey::SchemaError);

        return props;
    }

    Napi::Value GetSchemaPropsAsync(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        DgnDbWorkerPtr worker = new ECSchemaSerializationAsyncWorker(db, info.Env(), schemaName);
        return worker->Queue();  // Schema serialization happens in another thread
    }

    Napi::Value GetElement(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, opts);
        BeJsNapiObject jsValue(Env());
        auto status = JsInterop::GetElement(jsValue, GetOpenedDb(info), opts);
        if (DgnDbStatus::Success != status)
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "error reading element", status);
        return jsValue;
    }

    Napi::Value GetModel(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, opts);
        BeJsNapiObject modelJson(Env());
        DgnDbStatus status = JsInterop::GetModel(modelJson, GetOpenedDb(info), opts);
        if (DgnDbStatus::Success != status)
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "error reading model", status);
        return modelJson;
    }

    Napi::Value GetTempFileBaseName(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return toJsString(Env(), db.GetTempFileBaseName());
    }

    Napi::Value SetGeometricModelTrackingEnabled(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_BOOL(0, enable);

        auto& modelChanges = db.Txns().m_modelChanges;
        if (enable != modelChanges.IsTrackingGeometry())
            {
            auto mode = modelChanges.SetTrackingGeometry(enable);
            auto readonly = false;
            switch (mode)
                {
                case TxnManager::ModelChanges::Mode::Readonly:
                    readonly = true;
                    // fall-through intentional.
                case TxnManager::ModelChanges::Mode::Legacy:
                    return CreateBentleyReturnErrorObject(readonly ? DgnDbStatus::ReadOnly : DgnDbStatus::VersionTooOld);
                }
            }

        return CreateBentleyReturnSuccessObject(Napi::Boolean::New(Env(), modelChanges.IsTrackingGeometry()));
        }

    Napi::Value IsGeometricModelTrackingSupported(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        return Napi::Boolean::New(Env(), TxnManager::ModelChanges::Mode::Full == db.Txns().m_modelChanges.DetermineMode());
        }

    Napi::Value QueryModelExtents(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, options);
        BeJsNapiObject extentsJson(Env());
        JsInterop::QueryModelExtents(extentsJson, db, options);
        return extentsJson;
        }

    Napi::Value QueryModelExtentsAsync(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
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

      DgnDbWorkerPtr worker = new QueryModelExtentsWorker(db, info.Env(), std::move(modelIds));
      return worker->Queue();
    }

    Napi::Value ComputeRangesForText(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, text);
        REQUIRE_ARGUMENT_UINTEGER(1, fontId);
        REQUIRE_ARGUMENT_BOOL(2, bold);
        REQUIRE_ARGUMENT_BOOL(3, italic);
        REQUIRE_ARGUMENT_NUMBER(4, widthFactor);
        REQUIRE_ARGUMENT_NUMBER(5, height);

        auto emphasis = bold ? TextEmphasis::Bold : TextEmphasis::None;
        if (italic) {
            emphasis = emphasis | TextEmphasis::Italic;
        }

        BeJsNapiObject result(Env());
        JsInterop::ComputeRangeForText(result, db, text, FontId(static_cast<uint64_t>(fontId)), emphasis, widthFactor, height);
        return result;
    }

    Napi::Value DumpChangeSet(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, changeSet);

        ChangesetStatus status = JsInterop::DumpChangeSet(db, changeSet);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ExtractChangedInstanceIdsFromChangeSets(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
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
        DgnDbStatus status = JsInterop::ExtractChangedInstanceIdsFromChangeSets(json, db, changeSetFiles);
        return CreateBentleyReturnObject(status, napiResult);
        }

    static TxnManager::TxnId TxnIdFromString(Utf8StringCR str) {
        return TxnManager::TxnId(BeInt64Id::FromString(str.c_str()).GetValueUnchecked());
    }

    static Utf8String TxnIdToString(TxnManager::TxnId txnId) {return BeInt64Id(txnId.GetValue()).ToHexStr();}

    Napi::Value GetCurrentTxnId(NapiInfoCR info)
        {
        return toJsString(Env(), TxnIdToString(GetOpenedDb(info).Txns().GetCurrentTxnId()));
        }

    Napi::Value QueryFirstTxnId(NapiInfoCR info) {
        OPTIONAL_ARGUMENT_BOOL(0, allowCrossSessions, true); // default to true for backwards compatibility
        auto& txns = GetOpenedDb(info).Txns();
        TxnManager::TxnId startTxnId = allowCrossSessions ? txns.QueryNextTxnId(TxnManager::TxnId(0)) : txns.GetSessionStartId();
        return toJsString(Env(), TxnIdToString(startTxnId));
    }

    Napi::Value QueryNextTxnId(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        auto next = GetOpenedDb(info).Txns().QueryNextTxnId(TxnIdFromString(txnIdHexStr));
        return toJsString(Env(), TxnIdToString(next));
    }
    Napi::Value QueryPreviousTxnId(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        auto next = GetOpenedDb(info).Txns().QueryPreviousTxnId(TxnIdFromString(txnIdHexStr));
        return toJsString(Env(), TxnIdToString(next));
    }
    Napi::Value GetTxnDescription(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return toJsString(Env(), GetOpenedDb(info).Txns().GetTxnDescription(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value IsTxnIdValid(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return Napi::Boolean::New(Env(), TxnIdFromString(txnIdHexStr).IsValid());
    }
    Napi::Value HasFatalTxnError(NapiInfoCR info) { return Napi::Boolean::New(Env(), GetOpenedDb(info).Txns().HasFatalError()); }
    void LogTxnError(NapiInfoCR info) {
        REQUIRE_ARGUMENT_BOOL(0, fatal);
        GetOpenedDb(info).Txns().LogError(fatal);
    }
    void EnableTxnTesting(NapiInfoCR info) {
        GetOpenedDb(info).Txns().EnableTracking(true);
        GetOpenedDb(info).Txns().InitializeTableHandlers();
    }
    Napi::Value BeginMultiTxnOperation(NapiInfoCR info) {return Napi::Number::New(Env(),  (int) GetOpenedDb(info).Txns().BeginMultiTxnOperation());}
    Napi::Value EndMultiTxnOperation(NapiInfoCR info) {return Napi::Number::New(Env(),  (int) GetOpenedDb(info).Txns().EndMultiTxnOperation());}
    Napi::Value GetMultiTxnOperationDepth(NapiInfoCR info) {return Napi::Number::New(Env(),  (int) GetOpenedDb(info).Txns().GetMultiTxnOperationDepth());}
    Napi::Value GetUndoString(NapiInfoCR info) {
        return toJsString(Env(), GetOpenedDb(info).Txns().GetUndoString());
    }
    Napi::Value GetRedoString(NapiInfoCR info) {return toJsString(Env(), GetOpenedDb(info).Txns().GetRedoString());}
    Napi::Value HasUnsavedChanges(NapiInfoCR info) {return Napi::Boolean::New(Env(), GetOpenedDb(info).Txns().HasChanges());}
    Napi::Value HasPendingTxns(NapiInfoCR info) {return Napi::Boolean::New(Env(), GetOpenedDb(info).Txns().HasPendingTxns());}
    Napi::Value IsRedoPossible(NapiInfoCR info) {return Napi::Boolean::New(Env(), GetOpenedDb(info).Txns().IsRedoPossible());}
    Napi::Value IsUndoPossible(NapiInfoCR info) {
        return Napi::Boolean::New(Env(), GetOpenedDb(info).Txns().IsUndoPossible());
    }
    void RestartTxnSession(NapiInfoCR info) {GetOpenedDb(info).Txns().Initialize();}
    Napi::Value ReinstateTxn(NapiInfoCR info) {return Napi::Number::New(Env(), (int) GetOpenedDb(info).Txns().ReinstateTxn());}
    Napi::Value ReverseAll(NapiInfoCR info) {return Napi::Number::New(Env(), (int) GetOpenedDb(info).Txns().ReverseAll());}
    Napi::Value ReverseTo(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return Napi::Number::New(Env(), (int) GetOpenedDb(info).Txns().ReverseTo(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value CancelTo(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return Napi::Number::New(Env(), (int) GetOpenedDb(info).Txns().CancelTo(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value ReverseTxns(NapiInfoCR info) {
        REQUIRE_ARGUMENT_NUMBER(0, numTxns );
        return Napi::Number::New(Env(), (int) GetOpenedDb(info).Txns().ReverseTxns(numTxns));
    }
    Napi::Value ClassNameToId(NapiInfoCR info) {
        auto classId = ECJsonUtilities::GetClassIdFromClassNameJson(info[0], GetOpenedDb(info).GetClassLocater());
        return toJsString(Env(), classId);
    }
    Napi::Value ClassIdToName(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, idString);
        DgnClassId classId;
        DgnClassId::FromString(classId, idString.c_str());
        auto ecClass = GetOpenedDb(info).Schemas().GetClass(classId);
        return (nullptr == ecClass) ? Env().Undefined() : toJsString(Env(), ecClass->GetFullName());
    }

    Napi::Value StartCreateChangeset(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        TxnManagerR txns = db.Txns();
        txns.StopCreateChangeset(false); // if there's one in progress, just abandon it.
        ChangesetPropsPtr changeset = txns.StartCreateChangeset();
        if (!changeset.IsValid())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Error creating changeset", IModelJsNativeErrorKey::ChangesetError);

        BeJsNapiObject changesetInfo(Env());
        changesetInfo[JsInterop::json_id()] = changeset->GetChangesetId().c_str();
        changesetInfo[JsInterop::json_index()] = 0;
        changesetInfo[JsInterop::json_parentId()] = changeset->GetParentId().c_str();
        changesetInfo[JsInterop::json_pathname()] = Utf8String(changeset->GetFileName()).c_str();
        changesetInfo[JsInterop::json_changesType()] = (int)changeset->GetChangesetType();
        changesetInfo[JsInterop::json_uncompressedSize()] = static_cast<int64_t>(changeset->GetUncompressedSize());
        return changesetInfo;
    }

    void EnableChangesetStatsTracking(NapiInfoCR info) {
        GetWritableDb(info).Txns().EnableChangesetHealthStatsTracking();
    }

    void DisableChangesetStatsTracking(NapiInfoCR info) {
        GetWritableDb(info).Txns().DisableChangesetHealthStatsTracking();
    }

    Napi::Value GetChangesetHealthData(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(1, changesetId);
        return BeJsNapiObject(Env(), GetWritableDb(info).Txns().GetChangesetHealthStatistics(changesetId).Stringify());
    }

    Napi::Value GetAllChangesetHealthData(NapiInfoCR info) {
        auto statsDoc = GetWritableDb(info).Txns().GetAllChangesetHealthStatistics();
        auto changesets = statsDoc["changesets"];
        auto env = info.Env();

        if (!changesets.isArray())
            return Napi::Array::New(env);

        auto jsArray = Napi::Array::New(env, changesets.size());
        changesets.ForEachArrayMember([&](BeJsValue::ArrayIndex changesetIdx, BeJsConst changeset) {
            auto jsObj = Napi::Object::New(env);

            // Map top-level fields
            jsObj.Set("changesetId", Napi::String::New(env, changeset["changeset_id"].asString().c_str()));
            jsObj.Set("uncompressedSizeBytes", Napi::Number::New(env, changeset["uncompressed_size_bytes"].asUInt()));
            jsObj.Set("sha1ValidationTimeMs", Napi::Number::New(env, changeset["sha1_validation_time_ms"].asUInt()));
            jsObj.Set("insertedRows", Napi::Number::New(env, changeset["inserted_rows"].asUInt()));
            jsObj.Set("updatedRows", Napi::Number::New(env, changeset["updated_rows"].asUInt()));
            jsObj.Set("deletedRows", Napi::Number::New(env, changeset["deleted_rows"].asUInt()));
            jsObj.Set("totalElapsedMs", Napi::Number::New(env, changeset["total_elapsed_ms"].asUInt()));
            jsObj.Set("totalFullTableScans", Napi::Number::New(env, changeset["scan_count"].asUInt()));

            // Map health_stats array to perStatementStats
            auto perStmtArr = Napi::Array::New(env);
            if (const auto healthStats = changeset["health_stats"]; healthStats.isArray()) {
                healthStats.ForEachArrayMember([&](BeJsValue::ArrayIndex stmtIdx, BeJsConst stmt) {
                    auto stmtObj = Napi::Object::New(env);
                    stmtObj.Set("sqlStatement", Napi::String::New(env, stmt["statement"].asString().c_str()));
                    stmtObj.Set("dbOperation", Napi::String::New(env, stmt["op"].asString().c_str()));
                    stmtObj.Set("rowCount", Napi::Number::New(env, stmt["row_count"].asUInt()));
                    stmtObj.Set("elapsedMs", Napi::Number::New(env, stmt["elapsed_ms"].asUInt()));
                    stmtObj.Set("fullTableScans", Napi::Number::New(env, stmt["scan_count"].asUInt()));
                    perStmtArr.Set(stmtIdx, stmtObj);
                    return false;
                });
            }
            jsObj.Set("perStatementStats", perStmtArr);
            jsArray.Set(changesetIdx, jsObj);
            return false;
        });
        return jsArray;
    }

    void CompleteCreateChangeset(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, optObj);
        BeJsConst opts(optObj);
        if (!opts.isNumericMember(JsInterop::json_index()))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "changeset index must be supplied", IModelJsNativeErrorKey::BadArg);
        int32_t index = opts[JsInterop::json_index()].GetInt();

        db.Txns().FinishCreateChangeset(index);
    }

    void AbandonCreateChangeset(NapiInfoCR info) {
        GetWritableDb(info).Txns().StopCreateChangeset(false);
    }

    Napi::Value AddChildPropagatesChangesToParentRelationship(NapiInfoCR info)     {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        REQUIRE_ARGUMENT_STRING(1, className);
        auto status = db.Txns().AddChildPropagatesChangesToParentRelationship(schemaName, className);
        return Napi::Number::New(Env(), (int) status);
    }

    void InvalidateFontMap(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        BeMutexHolder lock(FontManager::GetMutex());
        db.Fonts().Invalidate();
    }

    Napi::Value WriteFullElementDependencyGraphToFile(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, dotFileName);
        JsInterop::WriteFullElementDependencyGraphToFile(db, dotFileName);
        return Napi::Number::New(Env(), 0);
        }

    Napi::Value WriteAffectedElementDependencyGraphToFile(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, dotFileName);
        REQUIRE_ARGUMENT_STRING_ARRAY(1, id64Array);
        JsInterop::WriteAffectedElementDependencyGraphToFile(db, dotFileName, id64Array);
        return Napi::Number::New(Env(), 0);
        }

    Napi::Value GetMassProperties(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, request);
        DgnDbWorkerPtr worker = new MassPropertiesAsyncWorker(GetOpenedDb(info), request); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        return worker->Queue();  // Measure happens in another thread
        }

    Napi::Value UpdateElementGeometryCache(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, request);
        DgnDbWorkerPtr worker = new ElementGeometryCacheAsyncWorker(GetOpenedDb(info), request); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        return worker->Queue();  // Happens in another thread
        }

    Napi::Value ElementGeometryCacheOperation(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestProps);

        Napi::Value elementIdVal = requestProps.Get("id");
        if (!elementIdVal.IsString())
            THROW_JS_TYPE_EXCEPTION("target element id must be specified");

        try
            {
            return Napi::Number::New(Env(), (int) ElementGeometryCache::Operation(db, requestProps, Env()));
            }
        catch (std::exception const& e)
            {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), e.what(), IModelJsNativeErrorKey::ElementGeometryCacheError);
            }
        }

    Napi::Value GetGeometryContainment(NapiInfoCR info)        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, request);
        DgnDbWorkerPtr worker = new FenceAsyncWorker(GetOpenedDb(info), request); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        return worker->Queue();  // Containment check happens in another thread
    }

    Napi::Value GetLocalChanges(NapiInfoCR info)        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, rootClassFilter);
        REQUIRE_ARGUMENT_BOOL(1, includeInMemChanges);
        auto results = Napi::Array::New(Env());
        int i = 0;

        auto& db = GetOpenedDb(info);
        db.Txns().ForEachLocalChange(
            [&](ECInstanceKey const& key, DbOpcode changeType) {
                auto result = Napi::Object::New(Env());
                result["id"] = Napi::String::New(Env(), key.GetInstanceId().ToHexStr());
                result["classFullName"] = Napi::String::New(Env(), db.Schemas().GetClass(key.GetClassId())->GetFullName());
                result["changeType"] = Napi::String::New(Env(), (changeType == DbOpcode::Delete ? "deleted" : (changeType == DbOpcode::Insert ? "inserted" : "updated")));
                results[i++] = result;
            }, rootClassFilter, includeInMemChanges);
        return results;
    }

    Napi::Value GetIModelCoordsFromGeoCoords(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, geoCoordProps);
        BeJsNapiObject results(Env());
        JsInterop::GetIModelCoordsFromGeoCoords(results, GetOpenedDb(info), geoCoordProps);
        return results;
    }

    Napi::Value GetGeoCoordsFromIModelCoords(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, iModelCoordProps);
        BeJsNapiObject results(Env());
        JsInterop::GetGeoCoordsFromIModelCoords(results, GetOpenedDb(info), iModelCoordProps);
        return results;
    }

    Napi::Value GetIModelProps(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        OPTIONAL_ARGUMENT_STRING(0, when);
        BeJsNapiObject props(Env());
        JsInterop::GetIModelProps(props, db, when);
        return props;
    }

    Napi::Value InsertElement(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, elemProps);
        Napi::Value insertOptions;
        if (ARGUMENT_IS_PRESENT(1)) {\
            insertOptions = info[1].As<Napi::Object>();
        } else {
            insertOptions = Env().Undefined();
        }
        return JsInterop::InsertElement(db, elemProps, insertOptions);
    }

    void UpdateElement(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);;
        REQUIRE_ARGUMENT_ANY_OBJ(0, elemProps);
        JsInterop::UpdateElement(db, elemProps);
    }

    void DeleteElement(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, elemIdStr);
        JsInterop::DeleteElement(db, elemIdStr);
    }

    Napi::Value QueryDefinitionElementUsage(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING_ARRAY(0, hexStringIds);
        Napi::Object usageInfoObj = Napi::Object::New(Env());
        BeJsValue usageInfo = BeJsValue(usageInfoObj);
        DgnDbStatus status = JsInterop::QueryDefinitionElementUsage(usageInfo, db, hexStringIds);
        return DgnDbStatus::Success == status ? usageInfoObj : Env().Undefined();
        }

    Napi::Value BeginPurgeOperation(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        db.BeginPurgeOperation();
        return Napi::Number::New(Env(), (int)DgnDbStatus::Success);
        }

    Napi::Value EndPurgeOperation(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        db.EndPurgeOperation();
        return Napi::Number::New(Env(), (int)DgnDbStatus::Success);
        }

    Napi::Value SimplifyElementGeometry(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, simplifyArgs);
        auto status = JsInterop::SimplifyElementGeometry(db, simplifyArgs);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value InlineGeometryPartReferences(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        auto result = JsInterop::InlineGeometryParts(db);

        auto ret = Napi::Object::New(Env());
        ret.Set(Napi::String::New(Env(), "numCandidateParts"), Napi::Number::New(Env(), result.m_numCandidateParts));
        ret.Set(Napi::String::New(Env(), "numRefsInlined"), Napi::Number::New(Env(), result.m_numRefsInlined));
        ret.Set(Napi::String::New(Env(), "numPartsDeleted"), Napi::Number::New(Env(), result.m_numPartsDeleted));

        return ret;
        }

    Napi::Value InsertElementAspect(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, aspectProps);
        return JsInterop::InsertElementAspect(GetOpenedDb(info), aspectProps);
    }

    void UpdateElementAspect(NapiInfoCR info)     {
        REQUIRE_ARGUMENT_ANY_OBJ(0, aspectProps);
        JsInterop::UpdateElementAspect(GetOpenedDb(info), aspectProps);
    }

    void DeleteElementAspect(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, aspectIdStr);
        JsInterop::DeleteElementAspect(GetOpenedDb(info), aspectIdStr);
    }

    Napi::Value ExportGraphics(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, exportProps);

        Napi::Value onGraphicsVal = exportProps.Get("onGraphics");
        if (!onGraphicsVal.IsFunction())
            THROW_JS_TYPE_EXCEPTION("onGraphics must be a function");

        Napi::Value elementIdArrayVal = exportProps.Get("elementIdArray");
        if (!elementIdArrayVal.IsArray())
            THROW_JS_TYPE_EXCEPTION("elementIdArray must be an array");

        DgnDbStatus status = JsInterop::ExportGraphics(db, exportProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ExportPartGraphics(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
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

        DgnDbStatus status = JsInterop::ExportPartGraphics(db, exportProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ProcessGeometryStream(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestProps);

        Napi::Value onGeometryVal = requestProps.Get("onGeometry");
        if (!onGeometryVal.IsFunction())
            THROW_JS_TYPE_EXCEPTION("onGeometry must be a function");

        Napi::Value elementIdVal = requestProps.Get("elementId");
        if (!elementIdVal.IsString())
            THROW_JS_TYPE_EXCEPTION("elementId must be specified");

        try
            {
            return Napi::Number::New(Env(), (int) GeometryStreamIO::ProcessGeometryStream(db, requestProps, Env()));
            }
        catch (std::exception const& e)
            {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), e.what(), IModelJsNativeErrorKey::GeometryStreamError);
            }
        }

    Napi::Value CreateBRepGeometry(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, createProps);

        Napi::Number operationVal = createProps.Get("operation").As<Napi::Number>();
        if (!operationVal.IsNumber())
            THROW_JS_TYPE_EXCEPTION("operation must be a specified");

        Napi::Value onResultVal = createProps.Get("onResult");
        if (!onResultVal.IsFunction())
            THROW_JS_TYPE_EXCEPTION("onResult must be a function");

        Napi::Array entryArrayVal = createProps.Get("entryArray").As<Napi::Array>();
        if (!entryArrayVal.IsArray())
            THROW_JS_TYPE_EXCEPTION("entryArray must be specified");

        try
            {
            return Napi::Number::New(Env(), (int) GeometryStreamIO::CreateBRepGeometry(db, createProps, Env()));
            }
        catch (std::exception const& e)
            {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), e.what(), IModelJsNativeErrorKey::GeometryStreamError);
            }
        }

    void GetTileTree(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, idStr);
        REQUIRE_ARGUMENT_FUNCTION(1, responseCallback);

        JsInterop::GetTileTree(m_cancellationToken.get(), GetOpenedDb(info), idStr, responseCallback);
        }

    void GetTileContent(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, treeIdStr);
        REQUIRE_ARGUMENT_STRING(1, tileIdStr);
        REQUIRE_ARGUMENT_FUNCTION(2, responseCallback);

        JsInterop::GetTileContent(m_cancellationToken.get(), GetOpenedDb(info), treeIdStr, tileIdStr, responseCallback);
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

        T_HOST.Visualization().PurgeTileTrees(GetOpenedDb(info), modelIds.empty() ? nullptr : &modelIds);
        }

    Napi::Value PollTileContent(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, treeIdStr);
        REQUIRE_ARGUMENT_STRING(1, tileIdStr);

        auto result = T_HOST.Visualization().PollTileContent(m_cancellationToken.get(), db, treeIdStr, tileIdStr);
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
            T_HOST.Visualization().CancelContentRequests(GetOpenedDb(info), treeId, tileIds);
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
        GetOpenedDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestProps);
        return ObtainTileGraphicsRequests().Enqueue(requestProps, info.Env());
        }

    Napi::Value GenerateElementMeshes(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestProps);
        return JsInterop::GenerateElementMeshes(GetOpenedDb(info), requestProps);
        }

    Napi::Value IsLinkTableRelationship(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, qualifiedClassName);
        Utf8String alias, className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, className, qualifiedClassName.c_str()))
            return Env().Undefined();

        ClassMapStrategy strategy = GetOpenedDb(info).Schemas().GetClassMapStrategy(alias, className, SchemaLookupMode::AutoDetect);
        if (strategy.IsEmpty())
            return Env().Undefined();

        return Napi::Boolean::New(Env(), strategy.IsLinkTableRelationship());
        }

    Napi::Value InsertLinkTableRelationship(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, props);
        return JsInterop::InsertLinkTableRelationship(GetOpenedDb(info), props);
    }

    void UpdateLinkTableRelationship(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, props);
        JsInterop::UpdateLinkTableRelationship(GetOpenedDb(info), props);
    }

    void DeleteLinkTableRelationship(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, props);
        JsInterop::DeleteLinkTableRelationship(GetOpenedDb(info), props);
    }

    Napi::Value InsertCodeSpec(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, name);
        REQUIRE_ARGUMENT_ANY_OBJ(1, jsonProperties);
        return JsInterop::InsertCodeSpec(GetOpenedDb(info), name, jsonProperties);
    }

    Napi::Value InsertModel(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, modelProps);
        return JsInterop::InsertModel(GetOpenedDb(info), modelProps);
        }

    void UpdateModel(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, modelProps);
        JsInterop::UpdateModel(GetOpenedDb(info), modelProps);
    }

    Napi::Value UpdateModelGeometryGuid(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, modelIdStr);
        DgnModelId modelId(BeInt64Id::FromString(modelIdStr.c_str()).GetValueUnchecked());
        auto status = JsInterop::UpdateModelGeometryGuid(GetOpenedDb(info), modelId);
        return Napi::Number::New(Env(), static_cast<int>(status));
        }

    void DeleteModel(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, elemIdStr);
        JsInterop::DeleteModel(GetOpenedDb(info), elemIdStr);
    }

    Napi::Value SaveChanges(NapiInfoCR info)
        {
        OPTIONAL_ARGUMENT_STRING(0, description);
        auto stat = GetOpenedDb(info).SaveChanges(description);
        return Napi::Number::New(Env(), (int)stat);
        }

    Napi::Value AbandonChanges(NapiInfoCR info)
        {
        DbResult status = GetOpenedDb(info).AbandonChanges();
        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value ImportFunctionalSchema(NapiInfoCR info)
        {
        DbResult result = JsInterop::ImportFunctionalSchema(GetOpenedDb(info));
        return Napi::Number::New(Env(), (int)result);
        }

    void DropSchema(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaName);
        auto rc = GetOpenedDb(info).DropSchema(schemaName);
        if (rc.GetStatus() != DropSchemaResult::Success) {
            BeNapi::ThrowJsException(info.Env(), rc.GetStatusAsString(), (int)rc.GetStatus(), {"schema-sync", "DropSchemaError"});
        }
    }
    void SchemaSyncSetDefaultUri(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, schemaSyncDbUriStr);
        LastErrorListener lastError(db);
        auto rc = db.Schemas().GetSchemaSync().SetDefaultSyncDbUri(schemaSyncDbUriStr.c_str());
        if (rc != SchemaSync::Status::OK) {
            if (lastError.HasError()) {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), rc);
            } else {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), Utf8PrintfString("fail to set default shared schema channel uri: %s", schemaSyncDbUriStr.c_str()).c_str(), rc);
            }
        }
    }
    Napi::Value SchemaSyncGetDefaultUri(NapiInfoCR info) {
        auto& syncDbUri = GetOpenedDb(info).Schemas().GetSchemaSync().GetDefaultSyncDbUri();
        if (syncDbUri.IsEmpty())
            return Env().Undefined();

        return Napi::String::New(Env(), syncDbUri.GetUri().c_str());
        }
    void SchemaSyncInit(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, schemaSyncDbUriStr);
        REQUIRE_ARGUMENT_STRING(1, containerId);
        REQUIRE_ARGUMENT_BOOL(2, overrideContainer);
        auto syncDbUri = SchemaSync::SyncDbUri(schemaSyncDbUriStr.c_str());
        LastErrorListener lastError(GetOpenedDb(info));
        auto rc = GetOpenedDb(info).Schemas().GetSchemaSync().Init(syncDbUri, containerId, overrideContainer);
        if (rc != SchemaSync::Status::OK) {
            if (lastError.HasError()) {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), rc);
            } else {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), Utf8PrintfString("fail to initialize shared schema channel: %s", schemaSyncDbUriStr.c_str()).c_str(), rc);
            }
        }
    }

    Napi::Value SchemaSyncEnabled(NapiInfoCR info) {
        const auto isEnabled = !GetOpenedDb(info).Schemas().GetSchemaSync().GetInfo().IsEmpty();
        return Napi::Boolean::New(Env(), isEnabled);
    }

    Napi::Value SchemaSyncGetLocalDbInfo(NapiInfoCR info) {
        auto localDbInfo = GetOpenedDb(info).Schemas().GetSchemaSync().GetInfo();
        if (localDbInfo.IsEmpty()) {
            return Env().Undefined();
        }

        BeJsNapiObject obj(Env());
        localDbInfo.To(obj);
        return obj;
    }

    Napi::Value SchemaSyncGetSyncDbInfo(NapiInfoCR info) {
        GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING(0, schemaSyncDbUriStr);
         auto syncDbUri = SchemaSync::SyncDbUri(schemaSyncDbUriStr.c_str());
         auto syncDbInfo = syncDbUri.GetInfo();
         if (syncDbInfo.IsEmpty()) {
            return Env().Undefined();
        }
        BeJsNapiObject obj(Env());
        syncDbInfo.To(obj);
        return obj;
    }

    void SchemaSyncPull(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        OPTIONAL_ARGUMENT_STRING(0, schemaSyncDbUriStr);
        auto syncDbUri = SchemaSync::SyncDbUri(schemaSyncDbUriStr.c_str());
        LastErrorListener lastError(GetOpenedDb(info));
        auto rc = db.PullSchemaChanges(syncDbUri);
        if (rc != SchemaSync::Status::OK) {
            if (lastError.HasError()) {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), rc);
            } else {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), Utf8PrintfString("fail to pull changes to schema sync db: %s", schemaSyncDbUriStr.c_str()).c_str(), rc);
            }
        }
    }

    void SchemaSyncPush(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        OPTIONAL_ARGUMENT_STRING(0, schemaSyncDbUriStr);
        auto syncDbUri = SchemaSync::SyncDbUri(schemaSyncDbUriStr.c_str());
        LastErrorListener lastError(GetOpenedDb(info));
        auto rc = db.Schemas().GetSchemaSync().Push(syncDbUri);
        if (rc != SchemaSync::Status::OK) {
            if (lastError.HasError()) {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), rc);
            } else {
                THROW_JS_SCHEMA_SYNC_EXCEPTION(info.Env(), Utf8PrintfString("fail to push changes to schema sync db: %s", schemaSyncDbUriStr.c_str()).c_str(), rc);
            }
        }
    }
    void ImportSchemas(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING_ARRAY(0, schemaFileNames);
        OPTIONAL_ARGUMENT_ANY_OBJ(1, jsOpts, Napi::Object::New(Env()));
        ECSchemaReadContextPtr customContext = nullptr;

        JsInterop::SchemaImportOptions options;
        const auto maybeEcSchemaContextVal = jsOpts.Get(JsInterop::json_ecSchemaXmlContext());
        options.m_schemaLockHeld = jsOpts.Get(JsInterop::json_schemaLockHeld()).ToBoolean();
        auto jsSyncDbUri = jsOpts.Get(JsInterop::json_schemaSyncDbUri());
        if (jsSyncDbUri.IsString())
            options.m_schemaSyncDbUri = jsSyncDbUri.ToString().Utf8Value();
        if (!maybeEcSchemaContextVal.IsUndefined())
            {
            if (!NativeECSchemaXmlContext::HasInstance(maybeEcSchemaContextVal))
                THROW_JS_TYPE_EXCEPTION("if SchemaImportOptions.ecSchemaXmlContext is defined, it must be an object of type NativeECSchemaXmlContext")
            options.m_customSchemaContext = NativeECSchemaXmlContext::Unwrap(maybeEcSchemaContextVal.As<Napi::Object>())->GetContext();
            }

        LastErrorListener lastError(db);
        DbResult result = JsInterop::ImportSchemas(db, schemaFileNames, SchemaSourceType::File, options);
        if (DbResult::BE_SQLITE_OK != result)
            {
                if (lastError.HasError()) {
                    THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), result);
                } else {
                    THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to import schemas", result);
                }
            }
        }

    void ImportXmlSchemas(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING_ARRAY(0, schemaFileNames);
        OPTIONAL_ARGUMENT_ANY_OBJ(1, jsOpts, Napi::Object::New(Env()));
        JsInterop::SchemaImportOptions options;
        options.m_schemaLockHeld = jsOpts.Get(JsInterop::json_schemaLockHeld()).ToBoolean();
        auto jsSyncDbUri = jsOpts.Get(JsInterop::json_schemaSyncDbUri());
        if (jsSyncDbUri.IsString())
            options.m_schemaSyncDbUri = jsSyncDbUri.ToString().Utf8Value();

        LastErrorListener lastError(db);
        DbResult result = JsInterop::ImportSchemas(db, schemaFileNames, SchemaSourceType::XmlString, options);
        if (DbResult::BE_SQLITE_OK != result)
            {
                if (lastError.HasError()) {
                    THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), lastError.GetLastError().c_str(), result);
                } else {
                    THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to import schemas", result);
                }
            }
        }

    Napi::Value FindGeometryPartReferences(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_STRING_ARRAY(0, partIds);
        REQUIRE_ARGUMENT_BOOL(1, is2d);

        auto elemIds = JsInterop::FindGeometryPartReferences(partIds, is2d, db);
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
        OPTIONAL_ARGUMENT_STRING(2, maybeOutFileName);

        ECSchemaCP schema = GetOpenedDb(info).Schemas().GetSchema(schemaName);
        if (nullptr == schema)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "specified schema was not found", IModelJsNativeErrorKey::SchemaError);

        BeFileName schemaFileName(exportDirectory);
        schemaFileName.AppendSeparator();
        if (maybeOutFileName == "")
            {
            schemaFileName.AppendUtf8(schema->GetFullSchemaName().c_str());
            schemaFileName.AppendExtension(L"ecschema.xml");
            }
        else
            schemaFileName.AppendUtf8(maybeOutFileName.c_str());
        ECVersion xmlVersion = ECSchema::ECVersionToWrite(schema->GetOriginalECXmlVersionMajor(), schema->GetOriginalECXmlVersionMinor());

        SchemaWriteStatus status = schema->WriteToXmlFile(schemaFileName.GetName(), xmlVersion);
        if (SchemaWriteStatus::Success != status)
            return Napi::Number::New(Env(), (int) status);

        return Napi::Number::New(Env(), (int) SchemaWriteStatus::Success);
        }

    Napi::Value ExportSchemas(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, exportDirectory);
        bvector<ECN::ECSchemaCP> schemas = GetOpenedDb(info).Schemas().GetSchemas();
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
        auto schema = GetOpenedDb(info).Schemas().GetSchema(schemaName, true);
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

    void CloseFile(NapiInfoCR info) { CloseDgnDb(false); }

    Napi::Value CreateClassViewsInDb(NapiInfoCR info) {
        BentleyStatus status = GetOpenedDb(info).Schemas().CreateClassViewsInDb();
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value CreateChangeCache(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeECDb, changeCacheECDb);
        REQUIRE_ARGUMENT_STRING(1, changeCachePathStr);
        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetOpenedDb(info).CreateChangeCache(changeCacheECDb->GetECDb(), changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value AttachChangeCache(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, changeCachePathStr);
        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetOpenedDb(info).AttachChangeCache(changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value DetachChangeCache(NapiInfoCR info)
        {
        DbResult stat = GetOpenedDb(info).DetachChangeCache();
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value IsChangeCacheAttached(NapiInfoCR info)
        {
        bool isAttached = GetOpenedDb(info).IsChangeCacheAttached();
        return Napi::Boolean::New(Env(), isAttached);
        }

    Napi::Value ExtractChangeSummary(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_OBJ(0, NativeECDb, changeCacheECDb);
        REQUIRE_ARGUMENT_STRING(1, changesetFilePathStr);
        BeFileName changesetFilePath(changesetFilePathStr.c_str(), true);
        ChangesetFileReader changeStream(changesetFilePath, &db);
        PERFLOG_START("iModelJsNative", "ExtractChangeSummary>ECDb::ExtractChangeSummary");
        ECInstanceKey changeSummaryKey;
        if (SUCCESS != ECDb::ExtractChangeSummary(changeSummaryKey, changeCacheECDb->GetECDb(), GetOpenedDb(info), ChangeSetArg(changeStream)))
            return CreateBentleyReturnErrorObject(BE_SQLITE_ERROR, Utf8PrintfString("Failed to extract ChangeSummary for ChangeSet file '%s'.", changesetFilePathStr.c_str()).c_str());

        PERFLOG_FINISH("iModelJsNative", "ExtractChangeSummary>ECDb::ExtractChangeSummary");

        return CreateBentleyReturnSuccessObject(toJsString(Env(), changeSummaryKey.GetInstanceId()));
        }


    Napi::Value GetBriefcaseId(NapiInfoCR info)
        {
        return Napi::Number::New(Env(), GetOpenedDb(info).GetBriefcaseId().GetValue());
        }

    Napi::Value GetCurrentChangeset(NapiInfoCR info) {
        int32_t index;
        Utf8String id;
        GetOpenedDb(info).Txns().GetParentChangesetIndex(index, id);
        BeJsNapiObject retVal(Env());
        retVal[JsInterop::json_id()] = id;
        if (index >= 0)
            retVal[JsInterop::json_index()] = index;

        return retVal;
    }

    Napi::Value GetIModelId(NapiInfoCR info)
        {
        BeGuid beGuid = GetOpenedDb(info).GetDbGuid();
        return toJsString(Env(), beGuid.ToString());
        }

    Napi::Value SetIModelId(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, guidStr);
        BeGuid guid;
        guid.FromString(guidStr.c_str());
        GetOpenedDb(info).ChangeDbGuid(guid);
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
        }

    Napi::Value SetITwinId(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, guidStr);
        BeGuid guid;
        guid.FromString(guidStr.c_str());
        GetOpenedDb(info).SaveProjectGuid(guid);
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
        }

    Napi::Value GetITwinId(NapiInfoCR info) {
        BeGuid beGuid = GetOpenedDb(info).QueryProjectGuid();
        return toJsString(Env(), beGuid.ToString());
    }
    Napi::Value ExecuteSql(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, sql);
        auto& db = GetOpenedDb(info);
        return Napi::Number::New(Env(), (int)db.ExecuteSql(sql.c_str()));
    }
    Napi::Value ConvertOrUpdateGeometrySource(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::ConvertOrUpdateGeometrySource(db, info);
    }
    Napi::Value ConvertOrUpdateGeometryPart(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::ConvertOrUpdateGeometryPart(db, info);
    }
    Napi::Value NewBeGuid(NapiInfoCR info) {
        BeGuid guid(true);
        return toJsString(Env(), guid.ToString());
    }
    void ClearECDbCache(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::ClearECDbCache(db, info);
    }
    Napi::Value PatchJsonProperties(NapiInfoCR info) {
        return JsInterop::PatchJsonProperties(info);
    }
    Napi::Value ResolveInstanceKey(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::ResolveInstanceKey(db, info);
    }
    Napi::Value ReadInstance(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        return JsInterop::ReadInstance(db, info);
    }
    Napi::Value InsertInstance(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        return JsInterop::InsertInstance(db, info);
    }
    Napi::Value UpdateInstance(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        return JsInterop::UpdateInstance(db, info);
    }
    Napi::Value DeleteInstance(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        return JsInterop::DeleteInstance(db, info);
    }
    void ResetBriefcaseId(NapiInfoCR info) {
        auto& db = GetOpenedDb(info);
        REQUIRE_ARGUMENT_INTEGER(0, newId);
        DbResult stat;
        try {
            stat = db.ResetBriefcaseId(BeSQLite::BeBriefcaseId(newId));
        } catch(std::runtime_error e) {
            BeNapi::ThrowJsException(info.Env(), e.what(), {"be-sqlite", "RuntimeError"});
        }
        if (stat != BE_SQLITE_OK)
            JsInterop::throwSqlResult("Cannot reset briefcaseId for", db.GetDbFileName(), stat);
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
        JsInterop::UpdateProjectExtents(GetOpenedDb(info), BeJsDocument(newExtentsJson));
        }

    Napi::Value GetCodeValueBehavior(NapiInfoCR info) {
        switch (GetOpenedDb(info).m_codeValueBehavior) {
            case DgnCodeValue::Behavior::Exact: return Napi::String::New(info.Env(), "exact");
            case DgnCodeValue::Behavior::TrimUnicodeWhitespace: return Napi::String::New(info.Env(), "trim-unicode-whitespace");
            default: THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Behavior was invalid. This is a bug.", IModelJsNativeErrorKey::BadArg);
        }
    }

    void SetCodeValueBehavior(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, codeValueBehaviorStr)
        DgnCodeValue::Behavior newBehavior;
        if (codeValueBehaviorStr == "exact")
            newBehavior = DgnCodeValue::Behavior::Exact;
        else if (codeValueBehaviorStr == "trim-unicode-whitespace")
            newBehavior = DgnCodeValue::Behavior::TrimUnicodeWhitespace;
        else
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Unsupported argument, should be one of the strings 'exact' or 'trim-unicode-whitespace'", IModelJsNativeErrorKey::BadArg);
        GetOpenedDb(info).m_codeValueBehavior = newBehavior;
    }

    Napi::Value ComputeProjectExtents(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_BOOL(0, wantFullExtents);
        REQUIRE_ARGUMENT_BOOL(1, wantOutliers);

        DRange3d fullExtents;
        bvector<BeInt64Id> outliers;
        auto extents = GetOpenedDb(info).GeoLocation().ComputeProjectExtents(wantFullExtents ? &fullExtents : nullptr, wantOutliers ? &outliers : nullptr);

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
        JsInterop::UpdateIModelProps(GetOpenedDb(info), BeJsConst(props));
    }

    Napi::Value ReadFontMap(NapiInfoCR info) {
        auto const& fontMap = GetOpenedDb(info).Fonts().FontMap();
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
        auto stat = GetOpenedDb(info).QueryBriefcaseLocalValue(val, name.c_str());
        return (stat != BE_SQLITE_ROW) ? Env().Undefined() : toJsString(Env(), val);
    }

    // save a value to the be_local table.
    void SaveLocalValue(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, name)
        if (info[1].IsUndefined())
            return DeleteLocalValue(info);

        REQUIRE_ARGUMENT_STRING(1, val)
        auto stat = GetOpenedDb(info).SaveBriefcaseLocalValue(name.c_str(), val);
        if (stat != BE_SQLITE_DONE)
            JsInterop::throwSqlResult("error saving local value", name.c_str(), stat);
    }

    // delete a value from the be_local table.
    void DeleteLocalValue(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, name)
        auto stat = GetOpenedDb(info).DeleteBriefcaseLocalValue(name.c_str());
        if (stat != BE_SQLITE_DONE)
            JsInterop::throwSqlResult("error deleting local value", name.c_str(), stat);
    }

    // delete all local Txns. THIS IS VERY DANGEROUS! Don't use it unless you know what you're doing!
    void DeleteAllTxns(NapiInfoCR info) {
        GetOpenedDb(info).Txns().DeleteAllTxns();
    }

    // get a texture image as a TextureData blob from the db. possibly downsample it to fit the client's maximum texture size, if specified.
    Napi::Value QueryTextureData(NapiInfoCR info)
        {
        auto worker = DgnDbWorker::CreateTextureImageWorker(GetOpenedDb(info), info);
        return worker->Queue();
        }

    ElementGraphicsRequestsR ObtainTileGraphicsRequests()
        {
        BeAssert(IsOpen());
        if (!m_elemGraphicsRequests)
            {
            m_elemGraphicsRequests = T_HOST.Visualization().CreateElementGraphicsRequests(GetDgnDb());
            BeAssert(nullptr != m_elemGraphicsRequests);
            }

        return *m_elemGraphicsRequests;
        }

    Napi::Value GetChangesetSize(NapiInfoCR info)
        {
        auto changesetSize = GetOpenedDb(info).Txns().GetChangesetSize();
        return Napi::Number::New(Env(), changesetSize);
        }

    Napi::Value EnableChangesetSizeStats(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_BOOL(0, enabled);
        auto rc = GetOpenedDb(info).Txns().EnableChangesetSizeStats(enabled);
        return Napi::Number::New(Env(), rc);
        }

    Napi::Value GetChangeTrackingMemoryUsed(NapiInfoCR info)
        {
        auto memoryUsed = GetOpenedDb(info).Txns().GetMemoryUsed();
        return Napi::Number::New(Env(), memoryUsed);
        }

    Napi::Value StartProfiler(NapiInfoCR info)
        {
        auto& db = GetOpenedDb(info);;
        OPTIONAL_ARGUMENT_STRING(0, scopeName);
        OPTIONAL_ARGUMENT_STRING(1, scenarioName);
        OPTIONAL_ARGUMENT_BOOL(2, overrideFlag, false);
        OPTIONAL_ARGUMENT_BOOL(4, computeExecutionPlan, true);

        auto rc = BeSQLite::Profiler::InitScope(db, scopeName.c_str(), scenarioName.c_str(), BeSQLite::Profiler::Params(overrideFlag, computeExecutionPlan));
        if (rc == BE_SQLITE_OK)
            rc = BeSQLite::Profiler::GetScope(db)->Start();

        return Napi::Number::New(Env(), rc);
        }

    Napi::Value StopProfiler(NapiInfoCR info)
        {
        auto scope = BeSQLite::Profiler::GetScope(GetOpenedDb(info));
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
        auto scope = BeSQLite::Profiler::GetScope(GetOpenedDb(info));
        DbResult rc;
        if (scope == nullptr)
            rc = BE_SQLITE_ERROR;
        else
            rc = scope->Pause();

        return Napi::Number::New(Env(), rc);
        }

    Napi::Value ResumeProfiler(NapiInfoCR info)
        {
        auto scope = BeSQLite::Profiler::GetScope(GetOpenedDb(info));
        DbResult rc;
        if (scope == nullptr)
            rc = BE_SQLITE_ERROR;
        else
            rc = scope->Resume();

        return Napi::Number::New(Env(), rc);;
        }

    Napi::Value IsProfilerRunning(NapiInfoCR info)
        {
        auto scope = BeSQLite::Profiler::GetScope(GetOpenedDb(info));
        if (scope == nullptr)
            return Napi::Boolean::New(Env(), false);;

        auto isRunning = scope->IsRunning();
        return Napi::Boolean::New(Env(), isRunning);
        }

    Napi::Value IsProfilerPaused(NapiInfoCR info)
        {
        auto scope = BeSQLite::Profiler::GetScope(GetOpenedDb(info));
        if (scope == nullptr)
            return Napi::Boolean::New(Env(), false);;

        auto isPaused = scope->IsPaused();
        return Napi::Boolean::New(Env(), isPaused);
    }

    void ApplyChangeset(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        REQUIRE_ARGUMENT_ANY_OBJ(0, changeset);
        REQUIRE_ARGUMENT_BOOL(1, fastForward);

        auto revision = JsInterop::GetChangesetProps(db.GetDbGuid().ToString(), changeset);
        auto currentId = db.Txns().GetParentChangesetId();
        ChangesetStatus stat =  ChangesetStatus::Success;
        if (revision->GetParentId() == currentId)  // merge
            stat = db.Txns().MergeChangeset(*revision, fastForward);
        else if (revision->GetChangesetId() == currentId) //reverse
            db.Txns().ReverseChangeset(*revision);
        if (ChangesetStatus::Success != stat)
            BeNapi::ThrowJsException(info.Env(), "error applying changeset", (int)stat, IModelJsNativeErrorKeyHelper::GetITwinError(IModelJsNativeErrorKey::ChangesetError));
    }
    void RevertTimelineChanges(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        if (info.Length() < 1 || !info[0].IsArray()) {
            THROW_JS_TYPE_EXCEPTION("Argument 0 must be an array of changesets props")
        }
        REQUIRE_ARGUMENT_BOOL(1, skipSchemaChanges);

        std::vector<ChangesetPropsPtr> changesets;
        Napi::Array arr = info[0].As<Napi::Array>();
        for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {
            Napi::Value arrValue = arr[arrIndex];
             if (arrValue.IsObject()) {
                auto revision = JsInterop::GetChangesetProps(db.GetDbGuid().ToString(), arrValue);
                changesets.push_back(revision);
             } else {
                THROW_JS_TYPE_EXCEPTION("Expect an object in the array")
             }
        }
        db.Txns().RevertTimelineChanges(changesets, skipSchemaChanges);
    }
    void AttachDb(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, fileName);
        REQUIRE_ARGUMENT_STRING(1, alias);
        auto rc = GetOpenedDb(info).AttachDb(fileName.c_str(), alias.c_str());
        if (rc != BE_SQLITE_OK) {
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to attach file", rc);
        }
    }
    void DetachDb(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, alias);
        auto rc = GetOpenedDb(info).DetachDb(alias.c_str());
        if (rc != BE_SQLITE_OK) {
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to detach file", rc);
        }
    }
    void ConcurrentQueryExecute(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, requestObj);
        REQUIRE_ARGUMENT_FUNCTION(1, callback);
        JsInterop::ConcurrentQueryExecute(GetOpenedDb(info), requestObj, callback);
    }

    Napi::Value ConcurrentQueryResetConfig(NapiInfoCR info) {
        if (info.Length() > 0 && info[0].IsObject()) {
            Napi::Object inConf = info[0].As<Napi::Object>();
            return JsInterop::ConcurrentQueryResetConfig(Env(), inConf);
        }
        return JsInterop::ConcurrentQueryResetConfig(Env());
    }

    void ConcurrentQueryShutdown(NapiInfoCR info) {
        ConcurrentQueryMgr::Shutdown(GetOpenedDb(info));
    }
    static Napi::Value ZlibCompress(NapiInfoCR info) {
        if (info.Length() < 1 || !info[0].IsTypedArray()){
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "expect UInt8Array argument", IModelJsNativeErrorKey::BadArg);
        }
        Napi::TypedArray typedArray = info[0].As<Napi::TypedArray>();
        if (typedArray.TypedArrayType() != napi_uint8_array) {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "expect UInt8Array argument", IModelJsNativeErrorKey::BadArg);
        }
        Napi::Uint8Array uint8Array = typedArray.As<Napi::Uint8Array>();
        bvector<Byte> bytes(uint8Array.Data(), uint8Array.Data() + uint8Array.ElementLength());
        bvector<Byte> compressed;
        if (!BeSQLiteLib::ZlibCompress(compressed, bytes)){
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "failed to compress buffer", IModelJsNativeErrorKey::CompressionError);
        }

        auto blob = Napi::Uint8Array::New(info.Env(), compressed.size());
        memcpy(blob.Data(), compressed.data(), compressed.size());
        return blob;
    }

    static Napi::Value ZlibDecompress(NapiInfoCR info) {
        if (info.Length() < 1 || !info[0].IsTypedArray()){
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "expect UInt8Array as first argument", IModelJsNativeErrorKey::BadArg);
        }
        if (info.Length() < 2 || !info[1].IsNumber()){
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "expect int as second argument argument", IModelJsNativeErrorKey::BadArg);
        }
        Napi::TypedArray typedArray = info[0].As<Napi::TypedArray>();
        if (typedArray.TypedArrayType() != napi_uint8_array) {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "expect UInt8Array argument", IModelJsNativeErrorKey::BadArg);
        }

        Napi::Number uncompressSize = info[1].As<Napi::Number>();
        Napi::Uint8Array uint8Array = typedArray.As<Napi::Uint8Array>();
        bvector<Byte> bytes(uint8Array.Data(), uint8Array.Data() + uint8Array.ElementLength());
        bvector<Byte> uncompressed;
        if (!BeSQLiteLib::ZlibDecompress(uncompressed, bytes, uncompressSize.Uint32Value())){
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "failed to decompress buffer", IModelJsNativeErrorKey::CompressionError);
        }

        auto blob = Napi::Uint8Array::New(info.Env(), uncompressed.size());
        memcpy(blob.Data(), uncompressed.data(), uncompressed.size());
        return blob;
    }

    Napi::Value PullMergeReverseLocalChanges(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        auto txns = db.Txns().PullMergeReverseLocalChanges();
        auto array = Napi::Array::New(Env(), txns.size());        
        for (size_t i = 0; i < txns.size(); ++i) {
            array[i] = Napi::String::New(Env(), BeInt64Id(txns[i].GetValue()).ToHexStr().c_str());
        }
        return array;
    }

    Napi::Value PullMergeRebaseBegin(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        auto txns = db.Txns().PullMergeRebaseBegin();
        auto array = Napi::Array::New(Env(), txns.size());        
        for (size_t i = 0; i < txns.size(); ++i) {
            array[i] = Napi::String::New(Env(), BeInt64Id(txns[i].GetValue()).ToHexStr().c_str());
        }
        return array;
    }

    void PullMergeRebaseEnd(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        db.Txns().PullMergeRebaseEnd();
    }

    Napi::Value PullMergeRebaseNext(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        auto txnId = db.Txns().PullMergeRebaseNext();
        if (txnId.IsValid()){
            return Napi::String::New(Env(), BeInt64Id(txnId.GetValue()).ToHexStr().c_str());
        }
        return info.Env().Undefined();
    }

    void PullMergeRebaseAbortTxn(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        db.Txns().PullMergeRebaseAbortTxn();
    }
    void PullMergeRebaseUpdateTxn(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        db.Txns().PullMergeRebaseUpdateTxn();
    }
    void PullMergeRebaseReinstateTxn(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        db.Txns().PullMergeRebaseReinstateTxn();
    }
    Napi::Value PullMergeGetStage(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        if (db.Txns().PullMergeGetStage() == TxnManager::PullMergeStage::Merging)
            return Napi::String ::New(Env(), "Merging");
        if (db.Txns().PullMergeGetStage() == TxnManager::PullMergeStage::Rebasing)
            return Napi::String ::New(Env(), "Rebasing");
        return Napi::String ::New(Env(), "None");
    }
    void SetTxnMode(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, mode);
        auto& db = GetWritableDb(info);
        if (mode == "direct")
            db.Txns().SetMode(ChangeTracker::Mode::Direct);
        else if (mode == "indirect")
            db.Txns().SetMode(ChangeTracker::Mode::Indirect);
        else
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "invalid txn mode", DgnDbStatus::BadArg);
    }
    Napi::Value  GetTxnMode(NapiInfoCR info) {
        auto& db = GetWritableDb(info);
        switch (db.Txns().GetMode()) {
            case ChangeTracker::Mode::Direct:
                return Napi::String::New(info.Env(), "direct");
            case ChangeTracker::Mode::Indirect:
                return Napi::String::New(info.Env(), "indirect");
            default:
                THROW_JS_DGN_DB_EXCEPTION(info.Env(), "invalid txn mode", DgnDbStatus::BadArg);
        }
    }
    Napi::Value  GetTxnProps(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdStr);
        auto& db = GetWritableDb(info);
        auto id = BeInt64Id::FromString(txnIdStr.c_str());
        if (!id.IsValid()) {
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "invalid txnId", DgnDbStatus::BadArg);
        }
        BeJsNapiObject props(info.Env());        
        if (db.Txns().GetTxnProps(TxnManager::TxnId(id.GetValue()), BeJsValue(props)))
            return props;

        return info.Env().Undefined();
    }
    
    static Napi::Value ComputeChangesetId(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, args);
        auto parentId = args.Get("parentId").As<Napi::String>();
        auto pathname = args.Get("pathname").As<Napi::String>();
        if (!parentId.IsString() || !pathname.IsString())
            BeNapi::ThrowJsException(info.Env(), "parentId and pathname are required attribute of ChangesetFileProps", (int)ChangesetStatus::BadVersionId);

        auto id = ChangesetProps::ComputeChangesetId(
            parentId.Utf8Value().c_str(),
            BeFileName(pathname.Utf8Value()),
            info.Env()
        );

        return Napi::String::New(info.Env(), id.c_str());
    }
    // ========================================================================================
    // Test method handler
    // ========================================================================================
    Napi::Value ExecuteTest(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, testName);
        REQUIRE_ARGUMENT_STRING(1, params);
        return toJsString(Env(), JsInterop::ExecuteTest(GetOpenedDb(info), testName, params).ToString());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "DgnDb", {
            InstanceMethod("abandonChanges", &NativeDgnDb::AbandonChanges),
            InstanceMethod("attachDb", &NativeDgnDb::AttachDb),
            InstanceMethod("detachDb", &NativeDgnDb::DetachDb),
            InstanceMethod("abandonCreateChangeset", &NativeDgnDb::AbandonCreateChangeset),
            InstanceMethod("addChildPropagatesChangesToParentRelationship", &NativeDgnDb::AddChildPropagatesChangesToParentRelationship),
            InstanceMethod("invalidateFontMap", &NativeDgnDb::InvalidateFontMap),
            InstanceMethod("applyChangeset", &NativeDgnDb::ApplyChangeset),
            InstanceMethod("revertTimelineChanges", &NativeDgnDb::RevertTimelineChanges),
            InstanceMethod("attachChangeCache", &NativeDgnDb::AttachChangeCache),
            InstanceMethod("beginMultiTxnOperation", &NativeDgnDb::BeginMultiTxnOperation),
            InstanceMethod("beginPurgeOperation", &NativeDgnDb::BeginPurgeOperation),
            InstanceMethod("cancelElementGraphicsRequests", &NativeDgnDb::CancelElementGraphicsRequests),
            InstanceMethod("cancelTileContentRequests", &NativeDgnDb::CancelTileContentRequests),
            InstanceMethod("cancelTo", &NativeDgnDb::CancelTo),
            InstanceMethod("classIdToName", &NativeDgnDb::ClassIdToName),
            InstanceMethod("classNameToId", &NativeDgnDb::ClassNameToId),
            InstanceMethod("closeFile", &NativeDgnDb::CloseFile),
            InstanceMethod("completeCreateChangeset", &NativeDgnDb::CompleteCreateChangeset),
            InstanceMethod("computeProjectExtents", &NativeDgnDb::ComputeProjectExtents),
            InstanceMethod("computeRangesForText", &NativeDgnDb::ComputeRangesForText),
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
            InstanceMethod("embedFontFile", &NativeDgnDb::EmbedFontFile),
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
            InstanceMethod("getCodeValueBehavior", &NativeDgnDb::GetCodeValueBehavior),
            InstanceMethod("getCurrentChangeset", &NativeDgnDb::GetCurrentChangeset),
            InstanceMethod("getCurrentTxnId", &NativeDgnDb::GetCurrentTxnId),
            InstanceMethod("getECClassMetaData", &NativeDgnDb::GetECClassMetaData),
            InstanceMethod("isSubClassOf", &NativeDgnDb::IsSubClassOf),
            InstanceMethod("getElement", &NativeDgnDb::GetElement),
            InstanceMethod("convertOrUpdateGeometrySource", &NativeDgnDb::ConvertOrUpdateGeometrySource),
            InstanceMethod("convertOrUpdateGeometryPart", &NativeDgnDb::ConvertOrUpdateGeometryPart),
            InstanceMethod("newBeGuid", &NativeDgnDb::NewBeGuid),
            InstanceMethod("patchJsonProperties", &NativeDgnDb::PatchJsonProperties),
            InstanceMethod("clearECDbCache", &NativeDgnDb::ClearECDbCache),
            InstanceMethod("resolveInstanceKey", &NativeDgnDb::ResolveInstanceKey),
            InstanceMethod("readInstance", &NativeDgnDb::ReadInstance),
            InstanceMethod("insertInstance", &NativeDgnDb::InsertInstance),
            InstanceMethod("updateInstance", &NativeDgnDb::UpdateInstance),
            InstanceMethod("deleteInstance", &NativeDgnDb::DeleteInstance),
            InstanceMethod("executeSql", &NativeDgnDb::ExecuteSql),
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
            InstanceMethod("insertElement", &NativeDgnDb::InsertElement),
            InstanceMethod("insertElementAspect", &NativeDgnDb::InsertElementAspect),
            InstanceMethod("insertLinkTableRelationship", &NativeDgnDb::InsertLinkTableRelationship),
            InstanceMethod("insertModel", &NativeDgnDb::InsertModel),
            InstanceMethod("isChangeCacheAttached", &NativeDgnDb::IsChangeCacheAttached),
            InstanceMethod("isGeometricModelTrackingSupported", &NativeDgnDb::IsGeometricModelTrackingSupported),
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
            InstanceMethod("setCodeValueBehavior", &NativeDgnDb::SetCodeValueBehavior),
            InstanceMethod("setGeometricModelTrackingEnabled", &NativeDgnDb::SetGeometricModelTrackingEnabled),
            InstanceMethod("setIModelDb", &NativeDgnDb::SetIModelDb),
            InstanceMethod("setIModelId", &NativeDgnDb::SetIModelId),
            InstanceMethod("setITwinId", &NativeDgnDb::SetITwinId),
            InstanceMethod("setBusyTimeout", &NativeDgnDb::SetBusyTimeout),
            InstanceMethod("simplifyElementGeometry", &NativeDgnDb::SimplifyElementGeometry),
            InstanceMethod("startCreateChangeset", &NativeDgnDb::StartCreateChangeset),
            InstanceMethod("startProfiler", &NativeDgnDb::StartProfiler),
            InstanceMethod("stopProfiler", &NativeDgnDb::StopProfiler),
            InstanceMethod("schemaSyncSetDefaultUri", &NativeDgnDb::SchemaSyncSetDefaultUri),
            InstanceMethod("schemaSyncGetDefaultUri", &NativeDgnDb::SchemaSyncGetDefaultUri),
            InstanceMethod("schemaSyncPull", &NativeDgnDb::SchemaSyncPull),
            InstanceMethod("schemaSyncPush", &NativeDgnDb::SchemaSyncPush),
            InstanceMethod("schemaSyncInit", &NativeDgnDb::SchemaSyncInit),
            InstanceMethod("schemaSyncEnabled", &NativeDgnDb::SchemaSyncEnabled),
            InstanceMethod("schemaSyncGetLocalDbInfo", &NativeDgnDb::SchemaSyncGetLocalDbInfo),
            InstanceMethod("schemaSyncGetSyncDbInfo", &NativeDgnDb::SchemaSyncGetSyncDbInfo),
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
            InstanceMethod("enableChangesetStatsTracking", &NativeDgnDb::EnableChangesetStatsTracking),
            InstanceMethod("disableChangesetStatsTracking", &NativeDgnDb::DisableChangesetStatsTracking),
            InstanceMethod("getChangesetHealthData", &NativeDgnDb::GetChangesetHealthData),
            InstanceMethod("getAllChangesetHealthData", &NativeDgnDb::GetAllChangesetHealthData),
            InstanceMethod("setAutoCheckpointThreshold", &NativeDgnDb::SetAutoCheckpointThreshold),
            InstanceMethod("getLocalChanges", &NativeDgnDb::GetLocalChanges),
            InstanceMethod("getNoCaseCollation", &NativeDgnDb::GetNoCaseCollation),
            InstanceMethod("setNoCaseCollation", &NativeDgnDb::SetNoCaseCollation),
            InstanceMethod("pullMergeGetStage", &NativeDgnDb::PullMergeGetStage),
            InstanceMethod("pullMergeRebaseReinstateTxn", &NativeDgnDb::PullMergeRebaseReinstateTxn),
            InstanceMethod("pullMergeRebaseUpdateTxn", &NativeDgnDb::PullMergeRebaseUpdateTxn),
            InstanceMethod("pullMergeRebaseBegin", &NativeDgnDb::PullMergeRebaseBegin),
            InstanceMethod("pullMergeRebaseEnd", &NativeDgnDb::PullMergeRebaseEnd),
            InstanceMethod("pullMergeRebaseNext", &NativeDgnDb::PullMergeRebaseNext),
            InstanceMethod("pullMergeRebaseAbortTxn", &NativeDgnDb::PullMergeRebaseAbortTxn),
            InstanceMethod("pullMergeReverseLocalChanges", &NativeDgnDb::PullMergeReverseLocalChanges),
            InstanceMethod("getTxnProps", &NativeDgnDb::GetTxnProps),
            InstanceMethod("setTxnMode", &NativeDgnDb::SetTxnMode),
            InstanceMethod("getTxnMode", &NativeDgnDb::GetTxnMode),
            StaticMethod("enableSharedCache", &NativeDgnDb::EnableSharedCache),
            StaticMethod("getAssetsDir", &NativeDgnDb::GetAssetDir),
            StaticMethod("zlibCompress", &NativeDgnDb::ZlibCompress),
            StaticMethod("zlibDecompress", &NativeDgnDb::ZlibDecompress),
            StaticMethod("computeChangesetId", &NativeDgnDb::ComputeChangesetId),
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

    static Napi::Value GetListOfCRS(NapiInfoCR info)
        {
        DRange2d extentRange;
        bool extentIsValid = ARGUMENT_IS_ANY_OBJ(0);
        if (extentIsValid)
            BeJsGeomUtils::DRange2dFromJson(extentRange, info[0].As<Napi::Object>());

        bool includeWorld = ARGUMENT_IS_BOOL(1) ? info[1].As<Napi::Boolean>().Value() : false;
        bvector<CRSListResponseProps> listOfCRS = GeoServicesInterop::GetListOfCRS(extentIsValid ? &extentRange : nullptr, includeWorld );

        uint32_t index = 0;
        auto ret = Napi::Array::New(info.Env(), listOfCRS.size());
        for (auto& gcs : listOfCRS)
            {
            auto gcsDefinition = Napi::Object::New(info.Env());
            gcsDefinition.Set(Napi::String::New(info.Env(), "name"), Napi::String::New(info.Env(), gcs.m_name.c_str()));
            gcsDefinition.Set(Napi::String::New(info.Env(), "description"), Napi::String::New(info.Env(), gcs.m_description.c_str()));
            gcsDefinition.Set(("deprecated"), gcs.m_deprecated);
            Napi::Object crsExtent = Napi::Object::New(info.Env());
            BeJsGeomUtils::DRange2dToJson(crsExtent, gcs.m_crsExtent);
            gcsDefinition.Set("crsExtent", crsExtent);

            ret.Set(index++, gcsDefinition);
            }

        return ret;
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "GeoServices", {
            StaticMethod("getGeographicCRSInterpretation", &NativeGeoServices::GetGeographicCRSInterpretation),
            StaticMethod("getListOfCRS", &NativeGeoServices::GetListOfCRS)
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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Failed to compute statistics", IModelJsNativeErrorKey::BadArg);

        return Napi::String::New(info.Env(), out.Stringify().c_str());
        }
    static Napi::Value GetUncompressSize(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, changesetFile);
        uint32_t compressSize, uncompressSize, prefixSize;
        if (SUCCESS != RevisionUtility::GetUncompressSize(changesetFile.c_str(), compressSize, uncompressSize, prefixSize))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Failed to get uncompress size", IModelJsNativeErrorKey::CompressionError);

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
//  Projects the SchemaUtility class into JS.
//! @bsiclass
//=======================================================================================
struct NativeSchemaUtility : BeObjectWrap<NativeSchemaUtility>
    {
    private:
        DEFINE_CONSTRUCTOR

    public:
        NativeSchemaUtility(NapiInfoCR info) : BeObjectWrap<NativeSchemaUtility>(info) {}
        ~NativeSchemaUtility() {SetInDestructor();}

    static Napi::Value ConvertCustomAttributes(NapiInfoCR info)
        {
        return ConvertSchemas(info, true);
        }

    static Napi::Value ConvertEC2XmlSchemas(NapiInfoCR info)
        {
        return ConvertSchemas(info, false);
        }

    static Napi::Value ConvertSchemas(NapiInfoCR info, bool convertCA)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, inputXmlStrings);

        ECSchemaReadContextPtr customContext = nullptr;
        if (ARGUMENT_IS_PRESENT(1)) {
            const auto& maybeEcSchemaContextVal = info[1].As<Napi::Object>();
            if (!maybeEcSchemaContextVal.IsUndefined())
                {
                if (!NativeECSchemaXmlContext::HasInstance(maybeEcSchemaContextVal))
                    THROW_JS_TYPE_EXCEPTION("if ecSchemaXmlContext is passed as an argument, it must be an object of type NativeECSchemaXmlContext")
                customContext = NativeECSchemaXmlContext::Unwrap(maybeEcSchemaContextVal.As<Napi::Object>())->GetContext();
                }
        }

        bvector<Utf8String> outputXmlStrings;
        BentleyStatus result = JsInterop::ConvertSchemas(inputXmlStrings, outputXmlStrings, customContext, convertCA);
        if (result != BentleyStatus::SUCCESS)
            {
            Utf8String error = convertCA ? "Failed to convert custom attributes of given schemas" : "Failed to convert EC2 Xml schemas";
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), error.c_str(), IModelJsNativeErrorKey::SchemaError);
            }

        uint32_t index = 0;
        auto ret = Napi::Array::New(info.Env(), outputXmlStrings.size());
        for (auto& outputXmlString : outputXmlStrings)
            ret.Set(index++, Napi::String::New(info.Env(), outputXmlString.c_str()));

        return ret;
        }

    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "SchemaUtility", {
            StaticMethod("convertCustomAttributes", &NativeSchemaUtility::ConvertCustomAttributes),
            StaticMethod("convertEC2XmlSchemas", &NativeSchemaUtility::ConvertEC2XmlSchemas),
        });

        exports.Set("SchemaUtility", t);

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
            auto revisionPtrs = JsInterop::GetChangesetPropsVec(containsSchemaChanges, dbGuid, changeSets);

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
            OPTIONAL_ARGUMENT_BOOL(12, wantBoundingBoxes, false);

            if (GetECDb().IsReadonly())
                return Napi::Number::New(Env(), (int) BE_SQLITE_READONLY);

            if (!m_manager)
                m_manager = std::make_unique<ChangedElementsManager>(BeFileName(dbFilename));

            bool containsSchemaChanges;
            auto revisionPtrs = JsInterop::GetChangesetPropsVec(containsSchemaChanges, dbGuid, changeSets);

            m_manager->SetFilterSpatial(filterSpatial);
            m_manager->SetWantParents(wantParents);
            m_manager->SetWantPropertyChecksums(wantPropertyChecksums);
            m_manager->SetWantBriefcaseRoll(true);
            m_manager->SetWantRelationshipCaching(wantRelationshipCaching);
            m_manager->SetRelationshipCacheSize(relationshipCacheSize);
            m_manager->SetWantChunkTraversal(wantChunkTraversal);
            m_manager->SetWantBoundingBoxes(wantBoundingBoxes);

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
    ECSqlStatement* m_ecSqlStatement = nullptr;

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
        if (info.Length() < 2 || info.Length() > 3)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder constructor expects either two or three arguments.", IModelJsNativeErrorKey::BadArg);

        m_binder = info[0].As<Napi::External<IECSqlBinder>>().Data();
        if (m_binder == nullptr)
            THROW_JS_TYPE_EXCEPTION("Invalid first arg for NativeECSqlBinder constructor. IECSqlBinder must not be nullptr");

        m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
        if (m_ecdb == nullptr)
            THROW_JS_TYPE_EXCEPTION("Invalid second arg for NativeECSqlBinder constructor. ECDb must not be nullptr");

        m_ecSqlStatement = info[2].As<Napi::External<ECSqlStatement>>().Data();
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

    static Napi::Object New(Napi::Env const& env, IECSqlBinder& binder, ECDbCR ecdb, const ECSqlStatement* ecSqlStatement = nullptr)
        {
        return Constructor().New({Napi::External<IECSqlBinder>::New(env, &binder), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb)), Napi::External<ECSqlStatement>::New(env, const_cast<ECSqlStatement*>(ecSqlStatement))});
        }

    Napi::Value BindNull(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        ECSqlStatus stat = m_binder->BindNull();
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBlob(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        if (info.Length() == 0)
            THROW_JS_TYPE_EXCEPTION("BindBlob requires an argument");

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        Napi::Value boolVal;
        if (info.Length() == 0 || !(boolVal = info[0]).IsBoolean())
            THROW_JS_TYPE_EXCEPTION("BindBoolean expects a boolean");

        ECSqlStatus stat = m_binder->BindBoolean(boolVal.ToBoolean());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDateTime(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        REQUIRE_ARGUMENT_NUMBER(0, val);
        ECSqlStatus stat = m_binder->BindDouble(val.DoubleValue());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindGuid(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);
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
        ECSqlStatus stat;
        BinderInfo const& binderInfo = m_binder->GetBinderInfo();
        if(binderInfo.GetType() == BinderInfo::BinderType::VirtualSet)
            stat = m_binder->BindVirtualSet(idSet);
        else if(binderInfo.GetType() == BinderInfo::BinderType::Array && binderInfo.IsForIdSet())
        {
            bool allElementsAdded = true;
            for(auto it = idSet->begin(); it != idSet->end(); ++it)
            {
                if(!(*it).IsValid())
                {
                    allElementsAdded = false;
                    break;
                }
                stat = m_binder->AddArrayElement().BindInt64((int64_t) (*it).GetValue());
                if(!stat.IsSuccess())
                {
                    allElementsAdded = false;
                    break;
                }
            }
            if(allElementsAdded) // If even one array element has failed to be added we set the status for the entire operation as ECSqlStatus::Error
                stat = ECSqlStatus::Success;
            else
                stat = ECSqlStatus::Error;
        }
        else
            stat = ECSqlStatus::Error;
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindInteger(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        REQUIRE_ARGUMENT_NUMBER(0, x);
        REQUIRE_ARGUMENT_NUMBER(1, y);
        ECSqlStatus stat = m_binder->BindPoint2d(DPoint2d::From(x.DoubleValue(),y.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint3d(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        REQUIRE_ARGUMENT_NUMBER(0, x);
        REQUIRE_ARGUMENT_NUMBER(1, y);
        REQUIRE_ARGUMENT_NUMBER(2, z);
        ECSqlStatus stat = m_binder->BindPoint3d(DPoint3d::From(x.DoubleValue(), y.DoubleValue(), z.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindString(NapiInfoCR info)
        {
        if (m_binder == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        REQUIRE_ARGUMENT_STRING(0, val);
        ECSqlStatus stat = m_binder->BindText(val.c_str(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindNavigation(NapiInfoCR info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        REQUIRE_ARGUMENT_STRING(0, navIdHexStr);
        OPTIONAL_ARGUMENT_STRING(1, relClassName);
        OPTIONAL_ARGUMENT_STRING(2, relClassTableSpaceName);

        BeInt64Id navId;
        if (SUCCESS != BeInt64Id::FromString(navId, navIdHexStr.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        const auto validateRelECClassId = m_ecSqlStatement && m_ecSqlStatement->IsWriteStatement() && m_ecdb->GetECSqlConfig().IsWriteValueValidationEnabled();

        ECClassId relClassId;
        if (!relClassName.empty())
            {
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(relClassName.c_str(), ".:", tokens);
            if (tokens.size() < 2 || tokens.size() > 3)
                return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

            relClassId = m_ecdb->Schemas().GetClassId(tokens[0], tokens[1], SchemaLookupMode::AutoDetect, relClassTableSpaceName.c_str());

            if (validateRelECClassId)
                {
                auto relClass = m_ecdb->Schemas().GetClass(relClassId);
                if (!relClass)
                    THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), Utf8PrintfString("The ECSql statement contains a relationship class '%s' which does not correspond to any EC class.", relClassName.c_str()).c_str(), IModelJsNativeErrorKey::ECClassError);
        
                if (!relClass->IsRelationshipClass())
                    THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), Utf8PrintfString("The ECSql statement contains a relationship class '%s' which does not correspond to a valid ECRelationship class.", relClassName.c_str()).c_str(), IModelJsNativeErrorKey::ECClassError);
                }
            }

        ECSqlStatus stat = m_binder->BindNavigation(navId, relClassId);
        if (validateRelECClassId && stat == ECSqlStatus(BE_SQLITE_ERROR))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), Utf8PrintfString("The ECSql statement contains a relationship class '%s' which does not match the relationship class in the navigation property.", relClassName.c_str()).c_str(), IModelJsNativeErrorKey::ECClassError);

        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindMember(NapiInfoCR info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        REQUIRE_ARGUMENT_STRING(0, memberName);
        IECSqlBinder& memberBinder = m_binder->operator[](memberName.c_str());
        return New(info.Env(), memberBinder, *m_ecdb);
        }

    Napi::Value AddArrayElement(NapiInfoCR info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlBinder is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
                InstanceMethod("getRootClassAlias", &NativeECSqlColumnInfo::GetRootClassAlias),
                InstanceMethod("isDynamicProp", &NativeECSqlColumnInfo::IsDynamicProp),
            });

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
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
                            THROW_JS_TYPE_EXCEPTION("Unsupported ECSqlValue primitive type.");
                            break;
                    }
                }

            return Napi::Number::New(Env(), (int) type);
            }

        Napi::Value GetPropertyName(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            ECPropertyCP prop = m_colInfo->GetProperty();
            if (prop == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo does not represent a property.", IModelJsNativeErrorKey::NotFound);

            return toJsString(Env(), prop->GetName());
            }
        Napi::Value IsDynamicProp(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            return Napi::Boolean::New(Env(), m_colInfo->IsDynamic());
            }
        Napi::Value GetOriginPropertyName(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            ECPropertyCP prop = m_colInfo->GetOriginProperty();
            if (prop == nullptr)
                return Env().Undefined();

            return toJsString(Env(), prop->GetName());
            }

        Napi::Value GetAccessString(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            //if property is generated, the display label contains the select clause item as is.
            //The property name in contrast would have encoded special characters of the select clause item.
            //Ex: SELECT MyProp + 4 FROM Foo -> the name must be "MyProp + 4"
            if (m_colInfo->IsGeneratedProperty())
                {
                BeAssert(m_colInfo->GetPropertyPath().Size() == 1);
                ECPropertyCP prop = m_colInfo->GetProperty();
                if (prop == nullptr)
                    THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo's Property must not be null for a generated property.", IModelJsNativeErrorKey::NotFound);

                return toJsString(Env(), prop->GetDisplayLabel());
                }

            return toJsString(Env(), m_colInfo->GetPropertyPath().ToString());
            }

        Napi::Value IsEnum(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            return Napi::Boolean::New(Env(), m_colInfo->GetEnumType() != nullptr);
            }

        Napi::Value IsSystemProperty(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            return Napi::Boolean::New(Env(), m_colInfo->IsSystemProperty());
            }

        Napi::Value IsGeneratedProperty(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            return Napi::Boolean::New(Env(), m_colInfo->IsGeneratedProperty());
            }

        Napi::Value GetRootClassTableSpace(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            return toJsString(Env(), m_colInfo->GetRootClass().GetTableSpace());
            }

        Napi::Value GetRootClassName(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

            return toJsString(Env(), ECJsonUtilities::FormatClassName(m_colInfo->GetRootClass().GetClass()));
            }

        Napi::Value GetRootClassAlias(NapiInfoCR info)
            {
            if (m_colInfo == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlColumnInfo is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        return NativeECSqlColumnInfo::New(Env(), m_ecsqlValue->GetColumnInfo());
        }

    Napi::Value IsNull(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        return Napi::Boolean::New(Env(), m_ecsqlValue->IsNull());
        }

    Napi::Value GetBlob(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        int blobSize;
        void const* data = m_ecsqlValue->GetBlob(&blobSize);
        auto blob = Napi::Uint8Array::New(Env(), blobSize);
        memcpy(blob.Data(), data, blobSize);
        return blob;
        }

    Napi::Value GetBoolean(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        return Napi::Boolean::New(Env(), m_ecsqlValue->GetBoolean());
        }

    Napi::Value GetDateTime(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        DateTime dt = m_ecsqlValue->GetDateTime();
        return toJsString(Env(), dt.ToString());
        }

    Napi::Value GetDouble(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        return Napi::Number::New(Env(), m_ecsqlValue->GetDouble());
        }

    Napi::Value GetGeometry(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        IGeometryPtr geom = m_ecsqlValue->GetGeometry();
        BeJsDocument json;
        if (geom == nullptr || SUCCESS != ECJsonUtilities::IGeometryToIModelJson(json, *geom))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Could not convert IGeometry to JSON.", IModelJsNativeErrorKey::GeometryStreamError);

        return toJsString(Env(), json.Stringify());
        }

    Napi::Value GetGuid(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        BeGuid guid = m_ecsqlValue->GetGuid();
        if (!guid.IsValid())
            return Env().Undefined();

        return toJsString(Env(), guid.ToString());
        }

    Napi::Value GetId(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        BeInt64Id id = m_ecsqlValue->GetId<BeInt64Id>();
        if (!id.IsValid())
            return Env().Undefined();

        return toJsString(Env(), id);
        }

    Napi::Value GetClassNameForClassId(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        ECClassId classId = m_ecsqlValue->GetId<ECClassId>();
        if (!classId.IsValid())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Failed to get class name from ECSqlValue: The ECSqlValue does not refer to a valid class id.", IModelJsNativeErrorKey::NotFound);

        Utf8StringCR tableSpace = m_ecsqlValue->GetColumnInfo().GetRootClass().GetTableSpace();
        ECClassCP ecClass = m_ecdb->Schemas().GetClass(classId, tableSpace.c_str());
        if (ecClass == nullptr)
            {
            Utf8String err;
            err.Sprintf("Failed to get class name from ECSqlValue: Class not found for ECClassId %s.", classId.ToHexStr().c_str());
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), err.c_str(), IModelJsNativeErrorKey::ECClassError);
            }

        return toJsString(Env(), ECJsonUtilities::FormatClassName(*ecClass));
        }

    Napi::Value GetInt(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        return Napi::Number::New(Env(), m_ecsqlValue->GetInt());
        }

    Napi::Value GetInt64(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        return Napi::Number::New(Env(), m_ecsqlValue->GetInt64());
        }

    Napi::Value GetPoint2d(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        DPoint2d pt = m_ecsqlValue->GetPoint2d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        return jsPt;
        }

    Napi::Value GetPoint3d(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        return toJsString(Env(), m_ecsqlValue->IsNull() ? "" : m_ecsqlValue->GetText());
        }

    Napi::Value GetEnum(NapiInfoCR info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        ECEnumerationCP enumType = m_ecsqlValue->GetColumnInfo().GetEnumType();
        if (enumType == nullptr)
            THROW_JS_TYPE_EXCEPTION("ECSqlValue is not an ECEnumeration.");

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

        ECClassId relClassId;
        BeInt64Id navId = m_ecsqlValue->GetNavigation(&relClassId);

        Napi::Object jsNavValue = Napi::Object::New(Env());
        jsNavValue.Set(ECN::ECJsonSystemNames::Navigation::Id(), Napi::String::New(Env(), navId.ToHexStr().c_str()));
        if (relClassId.IsValid())
            {
            Utf8StringCR relClassTableSpace = m_ecsqlValue->GetColumnInfo().GetRootClass().GetTableSpace();
            ECClassCP relClass = m_ecdb->Schemas().GetClass(relClassId, relClassTableSpace.c_str());
            if (relClass == nullptr)
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Failed to find ECRelationhipClass for the Navigation Value's RelECClassId.", IModelJsNativeErrorKey::ECClassError);

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
                THROW_JS_TYPE_EXCEPTION("ECSqlValueIterator constructor expects two argument.");

            m_iterable = info[0].As<Napi::External<IECSqlValueIterable>>().Data();
            if (m_iterable == nullptr)
                THROW_JS_TYPE_EXCEPTION("Invalid first arg for NativeECSqlValueIterator constructor. IECSqlValueIterable must not be nullptr");

            m_endIt = m_iterable->end();

            m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
            if (m_ecdb == nullptr)
                THROW_JS_TYPE_EXCEPTION("Invalid second arg for NativeECSqlValueIterator constructor. ECDb must not be nullptr");
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
        THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

    return NativeECSqlValueIterator::New(info.Env(), m_ecsqlValue->GetStructIterable(), *m_ecdb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeECSqlValue::GetArrayIterator(NapiInfoCR info)
    {
    if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
        THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlValue is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
          InstanceMethod("getColumnValueBinary", &NativeChangesetReader::GetColumnValueBinary),
          InstanceMethod("getColumnValueDouble", &NativeChangesetReader::GetColumnValueDouble),
          InstanceMethod("getColumnValueId", &NativeChangesetReader::GetColumnValueId),
          InstanceMethod("getColumnValueInteger", &NativeChangesetReader::GetColumnValueInteger),
          InstanceMethod("getColumnValueText", &NativeChangesetReader::GetColumnValueText),
          InstanceMethod("getColumnValueType", &NativeChangesetReader::GetColumnValueType),
          InstanceMethod("getDdlChanges", &NativeChangesetReader::GetDdlChanges),
          InstanceMethod("getOpCode", &NativeChangesetReader::GetOpCode),
          InstanceMethod("getPrimaryKeys", &NativeChangesetReader::GetPrimaryKeys),
          InstanceMethod("getRow", &NativeChangesetReader::GetRow),
          InstanceMethod("getTableName", &NativeChangesetReader::GetTableName),
          InstanceMethod("hasRow", &NativeChangesetReader::HasRow),
          InstanceMethod("isColumnValueNull", &NativeChangesetReader::IsColumnValueNull),
          InstanceMethod("isIndirectChange", &NativeChangesetReader::IsIndirectChange),
          InstanceMethod("getPrimaryKeyColumnIndexes", &NativeChangesetReader::GetPrimaryKeyColumnIndexes),
          InstanceMethod("openFile", &NativeChangesetReader::OpenFile),
          InstanceMethod("openGroup", &NativeChangesetReader::OpenGroup),
          InstanceMethod("writeToFile", &NativeChangesetReader::WriteToFile),
          InstanceMethod("openLocalChanges", &NativeChangesetReader::OpenLocalChanges),
          InstanceMethod("openTxn", &NativeChangesetReader::OpenTxn),
          InstanceMethod("reset", &NativeChangesetReader::Reset),
          InstanceMethod("step", &NativeChangesetReader::Step),
        });

        exports.Set("ChangesetReader", t);
        SET_CONSTRUCTOR(t);
        }
    Napi::Value GetPrimaryKeyColumnIndexes(NapiInfoCR info)
        {
        return m_changeset.GetPrimaryKeyColumnIndexes(Env());
        }
    Napi::Value IsColumnValueNull(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        REQUIRE_ARGUMENT_INTEGER(1, valueKind);
        return m_changeset.IsColumnValueNull(Env(), columnIndex, valueKind);
        }
    Napi::Value GetColumnValueText(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        REQUIRE_ARGUMENT_INTEGER(1, valueKind);
        return m_changeset.GetColumnValueText(Env(), columnIndex, valueKind);
        }
    Napi::Value GetColumnValueInteger(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        REQUIRE_ARGUMENT_INTEGER(1, valueKind);
        return m_changeset.GetColumnValueInteger(Env(), columnIndex, valueKind);
        }
    Napi::Value GetColumnValueId(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        REQUIRE_ARGUMENT_INTEGER(1, valueKind);
        return m_changeset.GetColumnValueId(Env(), columnIndex, valueKind);
        }
    Napi::Value GetColumnValueDouble(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        REQUIRE_ARGUMENT_INTEGER(1, valueKind);
        return m_changeset.GetColumnValueDouble(Env(), columnIndex, valueKind);
        }
    Napi::Value GetColumnValueBinary(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, columnIndex);
        REQUIRE_ARGUMENT_INTEGER(1, valueKind);
        return m_changeset.GetColumnValueBinary(Env(), columnIndex, valueKind);
        }
    Napi::Value GetPrimaryKeys(NapiInfoCR info)
        {
        return m_changeset.GetPrimaryKeys(Env());
        }
    Napi::Value GetRow(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, valueKind);
        return m_changeset.GetRow(Env(), valueKind);
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
    void OpenFile(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, fileName);
        REQUIRE_ARGUMENT_BOOL(1, invert);
        m_changeset.OpenFile(Env(), fileName.c_str(), invert);
        }
    void OpenGroup(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, fileNames);
        REQUIRE_ARGUMENT_ANY_OBJ(1, dbObj);
        REQUIRE_ARGUMENT_BOOL(2, invert);

        ECDb* ecdb = nullptr;
        if (NativeDgnDb::InstanceOf(dbObj)) {
            NativeDgnDb* addonDgndb = NativeDgnDb::Unwrap(dbObj);
            ecdb = &addonDgndb->GetDgnDb();

        } else if (NativeECDb::InstanceOf(dbObj)) {
            NativeECDb* addonECDb = NativeECDb::Unwrap(dbObj);
            ecdb = &addonECDb->GetECDb();

        } else {
            THROW_JS_TYPE_EXCEPTION("Provided db must be a NativeDgnDb or NativeECDb object");
        }

        if (!ecdb || !ecdb->IsDbOpen())
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "Provided db is not open", DgnDbStatus::NotOpen);

        m_changeset.OpenGroup(Env(), fileNames, *ecdb, invert);
        }
    void WriteToFile(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, fileName);
        REQUIRE_ARGUMENT_BOOL(1, containsSchemaChanges);
        REQUIRE_ARGUMENT_BOOL(2, overrideFile);
        m_changeset.WriteToFile(Env(), fileName, containsSchemaChanges, overrideFile);
        }
    void OpenLocalChanges(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, dbObj);
        REQUIRE_ARGUMENT_BOOL(1, includeInMemoryChanges);
        REQUIRE_ARGUMENT_BOOL(2, invert);
        NativeDgnDb* nativeDgnDb = NativeDgnDb::Unwrap(dbObj);
        if (!nativeDgnDb->IsOpen())
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "Provided db is not open", DgnDbStatus::NotOpen);

        auto changeset = nativeDgnDb->GetDgnDb().Txns().CreateChangesetFromLocalChanges(includeInMemoryChanges);
        if (changeset == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "no local changes", IModelJsNativeErrorKey::ChangesetError);

        m_changeset.OpenChangeStream(Env(), std::move(changeset), invert);
        }
    void OpenTxn(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, dbObj);
        REQUIRE_ARGUMENT_STRING(1, idStr);
        REQUIRE_ARGUMENT_BOOL(2, invert);
        NativeDgnDb* nativeDgnDb = NativeDgnDb::Unwrap(dbObj);
        if (!nativeDgnDb->IsOpen())
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "Provided db is not open", DgnDbStatus::NotOpen);

        BeInt64Id id;
        if (SUCCESS != BeInt64Id::FromString(id, idStr.c_str())) {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "expect txnId to be a hex string", IModelJsNativeErrorKey::BadArg);
        }

        auto changeset = nativeDgnDb->GetDgnDb().Txns().OpenLocalTxn(TxnManager::TxnId(id.GetValueUnchecked()));
        if (changeset == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), SqlPrintfString("no local change with id: %s", idStr.c_str()).GetUtf8CP(), IModelJsNativeErrorKey::ChangesetError);

        m_changeset.OpenChangeStream(Env(), std::move(changeset), invert);

        }
    Napi::Value Step(NapiInfoCR info)
        {
        return m_changeset.Step(Env());
        }
    void Close(NapiInfoCR info)
        {
        m_changeset.Close(Env());
        }
    void Reset(NapiInfoCR info)
        {
        m_changeset.Reset(Env());
        }
    Napi::Value GetTableName(NapiInfoCR info)
        {
        return m_changeset.GetTableName(Env());
        }
    Napi::Value HasRow(NapiInfoCR info)
        {
        return m_changeset.GetHasRow(Env());
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

        void _OnIssueReported(BentleyApi::ECN::IssueSeverity severity, BentleyApi::ECN::IssueCategory category, BentleyApi::ECN::IssueType type, BentleyApi::ECN::IssueId id, Utf8CP message) const override { m_lastIssue = message; }

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
            InstanceMethod("getNativeSql", &NativeECSqlStatement::GetNativeSql),
            InstanceMethod("toRow", &NativeECSqlStatement::ToRow),
            InstanceMethod("getMetadata", &NativeECSqlStatement::GetMetadata)
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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        ECSqlStatus status = m_stmt.Reset();
        return Napi::Number::New(Env(), (int)ToDbResult(status));
    }

    void Dispose(NapiInfoCR info) {
        m_stmt.Finalize();
    }

    Napi::Value ClearBindings(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        auto status = m_stmt.ClearBindings();
        return Napi::Number::New(Env(), (int)ToDbResult(status));
    }

    Napi::Value GetBinder(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        if (info.Length() != 1)
            THROW_JS_TYPE_EXCEPTION("GetBinder requires a parameter index or name as argument");

        Napi::Value paramArg = info[0];
        if (!paramArg.IsNumber() && !paramArg.IsString())
            THROW_JS_TYPE_EXCEPTION("GetBinder requires a parameter index or name as argument");

        int paramIndex = -1;
        if (paramArg.IsNumber())
            paramIndex = (int)paramArg.ToNumber().Int32Value();
        else
            paramIndex = m_stmt.GetParameterIndex(paramArg.ToString().Utf8Value().c_str());

        IECSqlBinder& binder = m_stmt.GetBinder(paramIndex);
        return NativeECSqlBinder::New(info.Env(), binder, *m_stmt.GetECDb(), &m_stmt);
    }

    Napi::Value Step(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        DbResult status = m_stmt.Step();
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value StepForInsert(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        int colCount = m_stmt.GetColumnCount();
        return Napi::Number::New(info.Env(), colCount);
    }

    Napi::Value GetValue(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_INTEGER(0, colIndex);

        IECSqlValue const& val = m_stmt.GetValue(colIndex);
        return NativeECSqlValue::New(info.Env(), val, *m_stmt.GetECDb());
    }

    Napi::Value GetNativeSql(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        return Napi::String::New(Env(), m_stmt.GetNativeSql());
    }

    Napi::Value ToRow(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_ANY_OBJ(0, optObj);
        BeJsValue opts(optObj);
        ECSqlRowAdaptor adaptor(*m_stmt.GetECDb());
        adaptor.GetOptions().FromJson(opts);

        BeJsNapiObject out(info.Env());
        BeJsValue rowJson = out["data"];
        if (adaptor.RenderRowAsArray(rowJson, ECSqlStatementRow(m_stmt)) != SUCCESS)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to render row", BE_SQLITE_ERROR);

        return out;
    }

    Napi::Value GetMetadata(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "ECSqlStatement is not prepared.", IModelJsNativeErrorKey::BadArg);

        BeJsNapiObject out(info.Env());
        BeJsValue metaJson = out["meta"];
        ECSqlRowAdaptor adaptor(*m_stmt.GetECDb());
        ECSqlRowProperty::List props;
        adaptor.GetMetaData(props, m_stmt);
        props.ToJs(metaJson);
        return out;
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
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "requires an open database", DgnDbStatus::NotOpen);
        }
        REQUIRE_ARGUMENT_ANY_OBJ(1, args);
        auto tableName = stringMember(args, JsInterop::json_tableName());
        if (tableName == "")
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "tableName missing", IModelJsNativeErrorKey::BadArg);
        auto columnName = stringMember(args, JsInterop::json_columnName());
        if (columnName == "")
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "columnName missing", IModelJsNativeErrorKey::BadArg);
        auto row = intMember(args, JsInterop::json_row(), 0);
        if (row == 0)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "invalid row", IModelJsNativeErrorKey::BadArg);
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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "blob invalid or too small", IModelJsNativeErrorKey::BadArg);
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
            THROW_JS_TYPE_EXCEPTION("invalid database object");
        }
        if (!db->IsDbOpen())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Prepare requires an open database", IModelJsNativeErrorKey::NotOpen);

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
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), db->GetLastError().c_str(), status);
    }

    Napi::Value IsReadonly(NapiInfoCR info) {
        if (!m_stmt.IsPrepared())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Cannot call IsReadonly on unprepared statement.", IModelJsNativeErrorKey::BadArg);
        return Napi::Boolean::New(Env(), m_stmt.IsReadonly());
    }

    Napi::Value BindNull(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 1);
        if (paramIndex < 1)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid parameters", IModelJsNativeErrorKey::BadArg);
        return Napi::Number::New(Env(), (int)m_stmt.BindNull(paramIndex));
    }

    Napi::Value BindBlob(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid parameters", IModelJsNativeErrorKey::BadArg);

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

        THROW_JS_TYPE_EXCEPTION("BindBlob requires a Uint8Array or ArrayBuffer arg");
    }

    Napi::Value BindDouble(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid parameters", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_NUMBER(1, val);
        const DbResult stat = m_stmt.BindDouble(paramIndex, val.DoubleValue());
        return Napi::Number::New(Env(), (int)stat);
    }

    Napi::Value BindInteger(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid parameters", IModelJsNativeErrorKey::BadArg);

        Napi::Value const& val = info[1];
        if (!val.IsNumber() && !val.IsString())
            THROW_JS_TYPE_EXCEPTION("BindInteger expects a string or number value.");

        int64_t int64Val;
        if (val.IsNumber())
            int64Val = val.ToNumber().Int64Value();
        else {
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

            if (isNegativeNumber && uVal > (uint64_t)std::numeric_limits<int64_t>::max()) {
                Utf8String error;
                error.Sprintf("BindInteger failed. Number in string %s is too large to fit into a signed 64 bit integer value.", strVal.c_str());
                THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), error.c_str(), IModelJsNativeErrorKey::BadArg);
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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid parameters", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING(1, val);
        const DbResult stat = m_stmt.BindText(paramIndex, val.c_str(), Statement::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int)stat);
    }

    Napi::Value BindGuid(NapiInfoCR info) {
        int paramIndex = GetParameterIndex(info, 2);
        if (paramIndex < 1)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid parameters", IModelJsNativeErrorKey::BadArg);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid parameters", IModelJsNativeErrorKey::BadArg);

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
            if (e.Message() == "aborted")
                e.Value()["errorNumber"] = (int) DgnDbStatus::Aborted;
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
    RefCountedPtr<SimpleRuleSetLocater> m_supplementalRulesets;
    RefCountedPtr<SimpleRuleSetLocater> m_primaryRulesets;
    std::shared_ptr<IModelJsECPresentationUpdateRecordsHandler> m_updatesHandler;
    Napi::ThreadSafeFunction m_threadSafeFunc;
    Napi::ThreadSafeFunction m_updateCallback;

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
            InstanceMethod("registerSupplementalRuleset", &NativeECPresentationManager::RegisterSupplementalRuleset),
            InstanceMethod("getRulesets", &NativeECPresentationManager::GetRulesets),
            InstanceMethod("addRuleset", &NativeECPresentationManager::AddRuleset),
            InstanceMethod("removeRuleset", &NativeECPresentationManager::RemoveRuleset),
            InstanceMethod("clearRulesets", &NativeECPresentationManager::ClearRulesets),
            InstanceMethod("handleRequest", &NativeECPresentationManager::HandleRequest),
            InstanceMethod("dispose", &NativeECPresentationManager::Terminate)
            });

        exports.Set("ECPresentationManager", t);

        SET_CONSTRUCTOR(t);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetSerializedResponse(Napi::Env env, BeJsNapiObject obj, Utf8CP memberName, Utf8StringR serializedResponse)
        {
        if (serializedResponse.empty())
            {
            obj[memberName] = "null";
            return;
            }

        Utf8StringP strP = new Utf8String();
        strP->swap(serializedResponse);
        Napi::Value val = Napi::Buffer<Utf8Char>::NewOrCopy(env, strP->data(), strP->size(), [](Napi::Env, Utf8Char*, void* ptr)
            {
            delete reinterpret_cast<Utf8StringP>(ptr);
            }, strP);
        obj.AsNapiValueRef()->m_napiVal.As<Napi::Object>().Set(memberName, val);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Napi::Value CreateReturnValue(Napi::Env env, ECPresentationResult& result, bool serializeResponse = false)
        {
        BeJsNapiObject retVal(env);
        if (result.IsError())
            {
            // error
            SaveErrorValue(retVal["error"], result.GetStatus(), result.GetErrorMessage().c_str());
            }
        else
            {
            // success
            if (serializeResponse)
                SetSerializedResponse(env, retVal, "result", result.GetSerializedSuccessResponse());
            else
                retVal["result"].From(result.GetSuccessResponse());
            }

        if (result.GetDiagnostics())
            retVal["diagnostics"].From(*result.GetDiagnostics());

        return retVal;
        }

    NativeECPresentationManager(NapiInfoCR info)
        : BeObjectWrap<NativeECPresentationManager>(info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, props);
        if (!props.Get("id").IsString())
            THROW_JS_TYPE_EXCEPTION("props.id must be a string");
        if (!props.Get("updateCallback").IsFunction())
            THROW_JS_TYPE_EXCEPTION("props.updateCallback must be a function");
        if (!props.Get("taskAllocationsMap").IsObject())
            THROW_JS_TYPE_EXCEPTION("props.taskAllocationsMap must be an object");
        if (!props.Get("defaultFormats").IsObject())
            THROW_JS_TYPE_EXCEPTION("props.defaultFormats must be an object");
        if (!props.Get("cacheConfig").IsObject())
            THROW_JS_TYPE_EXCEPTION("props.cacheConfig must be an object");

        try
            {
            m_updateCallback = Napi::ThreadSafeFunction::New(Env(), props.Get("updateCallback").As<Napi::Function>(), "NativeECPresentationManager data update callback", 0, 1);
            m_updatesHandler = std::make_shared<IModelJsECPresentationUpdateRecordsHandler>([this](rapidjson::Document&& updateInfo)
                {
                auto updateInfoPtr = new rapidjson::Document(std::move(updateInfo));
                m_updateCallback.BlockingCall(updateInfoPtr, [](Napi::Env env, Napi::Function jsCallback, rapidjson::Document* updateInfoPtr)
                    {
                    BeJsNapiObject res(env);
                    res.From(*updateInfoPtr);
                    jsCallback.Call({ res });
                    delete updateInfoPtr;
                    });
                });
            m_presentationManager = ECPresentationUtils::CreatePresentationManager(T_HOST.GetIKnownLocationsAdmin(), m_updatesHandler, props);
            m_supplementalRulesets = SimpleRuleSetLocater::Create();
            m_presentationManager->GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*m_supplementalRulesets));
            m_primaryRulesets = SimpleRuleSetLocater::Create();
            m_presentationManager->GetLocaters().RegisterLocater(*NonSupplementalRuleSetLocater::Create(*m_primaryRulesets));
            m_threadSafeFunc = Napi::ThreadSafeFunction::New(Env(), Napi::Function::New(Env(), [](NapiInfoCR info) {}), "NativeECPresentationManager result resolver", 0, 1);
            }
        catch (std::exception const& e)
            {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), e.what(), IModelJsNativeErrorKey::RuntimeError);
            }
        catch (...)
            {
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Unknown initialization error", IModelJsNativeErrorKey::RuntimeError);
            }
        }

    ~NativeECPresentationManager()
        {
        SetInDestructor();  // Must be the first line of every ObjectWrap destructor
        // 'terminate' not called
        BeAssert(m_presentationManager == nullptr);
        }

    Napi::Value CreateReturnValue(ECPresentationResult&& result, bool serializeResponse = false)
        {
        return CreateReturnValue(Env(), result, serializeResponse);
        }

    void ResolvePromise(Napi::Promise::Deferred const& deferred, ECPresentationResult&& result)
        {
        std::shared_ptr<ECPresentationResult> resultPtr = std::make_shared<ECPresentationResult>(std::move(result));
        m_threadSafeFunc.BlockingCall([resultPtr, deferred = std::move(deferred)](Napi::Env env, Napi::Function)
            {
            deferred.Resolve(CreateReturnValue(env, *resultPtr, true));
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

        auto diagnostics = ECPresentation::Diagnostics::Scope::ResetAndCreate(requestId, ECPresentationUtils::CreateDiagnosticsOptions(params));
        try
            {
            std::shared_ptr<folly::Future<ECPresentationResult>> result = std::make_shared<folly::Future<ECPresentationResult>>(folly::makeFuture(ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("request.requestId = '%s'", requestId))));
            if (0 == strcmp("GetRootNodesCount", requestId))
                *result = ECPresentationUtils::GetRootNodesCount(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetRootNodes", requestId))
                *result = ECPresentationUtils::GetRootNodes(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetChildrenCount", requestId))
                *result = ECPresentationUtils::GetChildrenCount(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetChildren", requestId))
                *result = ECPresentationUtils::GetChildren(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetNodesDescriptor", requestId))
                *result = ECPresentationUtils::GetHierarchyLevelDescriptor(*m_presentationManager, db->GetDgnDb(), params);
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
            else if (0 == strcmp("GetContentSet", requestId))
                *result = ECPresentationUtils::GetContentSet(*m_presentationManager, db->GetDgnDb(), params);
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
                result.SetDiagnostics(std::make_unique<rapidjson::Document>(diagnostics->BuildJson()));
                ResolvePromise(deferred, std::move(result));
                ECPresentationUtils::GetLogger().debugv("Request %s completed successfully in %" PRIu64 " ms.",
                    requestGuid.c_str(), (BeTimeUtilities::GetCurrentTimeAsUnixMillis() - startTime));
                })
            .onError([this, requestGuid, startTime, diagnostics = *diagnostics, deferred = std::move(deferred)](folly::exception_wrapper e)
                {
                ECPresentationResult result = ECPresentationUtils::CreateResultFromException(e, std::make_unique<rapidjson::Document>(diagnostics->BuildJson()));
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
        catch (std::exception const& e)
            {
            ECPresentationUtils::GetLogger().errorv("Failed to queue request %s", requestGuid.c_str());
            deferred.Resolve(CreateReturnValue(ECPresentationUtils::CreateResultFromException(folly::exception_wrapper{std::current_exception(), e}, std::make_unique<rapidjson::Document>((*diagnostics)->BuildJson()))));
            }
        catch (...)
            {
            ECPresentationUtils::GetLogger().errorv("Failed to queue request %s", requestGuid.c_str());
            deferred.Resolve(CreateReturnValue(ECPresentationUtils::CreateResultFromException(folly::exception_wrapper{std::current_exception()}, std::make_unique<rapidjson::Document>((*diagnostics)->BuildJson()))));
            }

        return response;
        }

    Napi::Value ForceLoadSchemas(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db);
        if (!db->IsOpen())
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "NativeECPresentationManager::ForceLoadSchemas: iModel not open", DgnDbStatus::NotOpen);

        DgnDbWorkerPtr worker = new SchemasLoader(info.Env(), db->GetDgnDb());
        return worker->Queue();
        }

    Napi::Value SetupRulesetDirectories(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, directories);
        ECPresentationResult result = ECPresentationUtils::SetupRulesetDirectories(*m_presentationManager, directories);
        return CreateReturnValue(std::move(result));
        }

    Napi::Value SetupSupplementalRulesetDirectories(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, directories);
        ECPresentationResult result = ECPresentationUtils::SetupSupplementalRulesetDirectories(*m_presentationManager, directories);
        return CreateReturnValue(std::move(result));
        }

    Napi::Value RegisterSupplementalRuleset(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetJsonString);
        ECPresentationResult result = ECPresentationUtils::AddRuleset(*m_supplementalRulesets, rulesetJsonString);
        return CreateReturnValue(std::move(result));
        }

    Napi::Value GetRulesets(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId);
        ECPresentationResult result = ECPresentationUtils::GetRulesets(*m_primaryRulesets, rulesetId);
        return CreateReturnValue(std::move(result), true);
        }

    Napi::Value AddRuleset(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetJsonString);
        ECPresentationResult result = ECPresentationUtils::AddRuleset(*m_primaryRulesets, rulesetJsonString);
        return CreateReturnValue(std::move(result));
        }

    Napi::Value RemoveRuleset(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId);
        REQUIRE_ARGUMENT_STRING(1, hash);
        ECPresentationResult result = ECPresentationUtils::RemoveRuleset(*m_primaryRulesets, rulesetId, hash);
        return CreateReturnValue(std::move(result));
        }

    Napi::Value ClearRulesets(NapiInfoCR info)
        {
        ECPresentationResult result = ECPresentationUtils::ClearRulesets(*m_primaryRulesets);
        return CreateReturnValue(std::move(result));
        }

    Napi::Value GetRulesetVariableValue(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId);
        REQUIRE_ARGUMENT_STRING(1, variableId);
        REQUIRE_ARGUMENT_STRING(2, type);
        ECPresentationResult result = ECPresentationUtils::GetRulesetVariableValue(*m_presentationManager, rulesetId, variableId, type);
        return CreateReturnValue(std::move(result));
        }

    Napi::Value SetRulesetVariableValue(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, ruleSetId);
        REQUIRE_ARGUMENT_STRING(1, variableId);
        REQUIRE_ARGUMENT_STRING(2, variableType);
        REQUIRE_ARGUMENT_ANY_OBJ(3, value);
        ECPresentationResult result = ECPresentationUtils::SetRulesetVariableValue(*m_presentationManager, ruleSetId, variableId, variableType, value);
        return CreateReturnValue(std::move(result));
        }

    Napi::Value UnsetRulesetVariableValue(NapiInfoCR info)
        {
        REQUIRE_ARGUMENT_STRING(0, ruleSetId);
        REQUIRE_ARGUMENT_STRING(1, variableId);
        ECPresentationResult result = ECPresentationUtils::UnsetRulesetVariableValue(*m_presentationManager, ruleSetId, variableId);
        return CreateReturnValue(std::move(result));
        }

    void Terminate(NapiInfoCR info)
        {
        m_presentationManager.reset();
        m_updateCallback.Release();
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

    THROW_JS_IMODEL_NATIVE_EXCEPTION(JsInterop::Env(), Utf8PrintfString("Native Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str(), IModelJsNativeErrorKey::NativeAssertion);
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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING(0, outputFileName);
        BentleyStatus status = m_importContext->Dump(outputFileName);
        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value AddClass(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId);
        REQUIRE_ARGUMENT_STRING_ID(1, targetIdStr, CodeSpecId, targetId);
        m_importContext->AddCodeSpecId(sourceId, targetId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value AddElementId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceId);
        REQUIRE_ARGUMENT_STRING_ID(1, targetIdStr, DgnElementId, targetId);
        m_importContext->AddElementId(sourceId, targetId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value RemoveElementId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceId);
        m_importContext->AddElementId(sourceId, DgnElementId());
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value FindCodeSpecId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId);
        CodeSpecId targetId = m_importContext->FindCodeSpecId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value FindElementId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceId);
        DgnElementId targetId = m_importContext->FindElementId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value CloneElement(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceElementId);
        bool binaryGeometry = false;
        if (!ARGUMENT_IS_EMPTY(1))
            {
            Napi::Object cloneOptions = info[1].As<Napi::Object>();
            binaryGeometry = cloneOptions.Get("binaryGeometry").ToBoolean();
            }

        DgnElementCPtr sourceElement = m_importContext->GetSourceDb().Elements().GetElement(sourceElementId);
        if (!sourceElement.IsValid())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid source ElementId", IModelJsNativeErrorKey::BadArg);

        // NOTE: Elements and Models share the same mapping table. However, the root Subject can be remapped but not the RepositoryModel
        DgnModelId targetModelId;
        if (sourceElement->GetModelId() == DgnModel::RepositoryModelId())
            targetModelId = DgnModel::RepositoryModelId();
        else
            targetModelId = m_importContext->FindModelId(sourceElement->GetModelId());

        DgnModelPtr targetModel = m_importContext->GetDestinationDb().Models().GetModel(targetModelId);
        if (!targetModel.IsValid())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid target model", IModelJsNativeErrorKey::BadArg);

        DgnDbStatus cloneStatus;
        DgnElementPtr targetElement = sourceElement->CloneForImport(&cloneStatus, *targetModel, *m_importContext);
        if (cloneStatus == DgnDbStatus::WrongClass)
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "Unable to clone an element because of an invalid class. Were schemas imported?", cloneStatus);
        if (cloneStatus != DgnDbStatus::Success || !targetElement.IsValid())
            THROW_JS_DGN_DB_EXCEPTION(info.Env(), "Unable to clone element", cloneStatus);

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
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId);
        CodeSpecId targetId = m_importContext->RemapCodeSpecId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value ImportFont(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_NUMBER(0, sourceFontNumber);
        FontId sourceFontId(static_cast<uint64_t>(sourceFontNumber.Uint32Value())); // BESERVER_ISSUED_ID_CLASS can be represented as a number in TypeScript
        FontId targetFontId = m_importContext->RemapFont(sourceFontId);
        return Napi::Number::New(Env(), targetFontId.GetValue());
        }

    Napi::Value HasSubCategoryFilter(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        return Napi::Boolean::New(Env(), m_importContext->HasSubCategoryFilter());
        }

    Napi::Value IsSubCategoryFiltered(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, subCategoryIdStr, DgnSubCategoryId, subCategoryId);
        return Napi::Boolean::New(Env(), m_importContext->IsSubCategoryFiltered(subCategoryId));
        }

    Napi::Value FilterSubCategoryId(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_STRING_ID(0, subCategoryIdStr, DgnSubCategoryId, subCategoryId);
        m_importContext->FilterSubCategoryId(subCategoryId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value SaveStateToDb(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_OBJ(0, SQLiteDb, db);
        if (nullptr == db)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);
        DbResult result = m_importContext->SaveStateToDb(db->GetDb());
        if (result != DbResult::BE_SQLITE_OK)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to serialize the state", result);
        return Env().Undefined();
        }

    Napi::Value LoadStateFromDb(NapiInfoCR info)
        {
        if (nullptr == m_importContext)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);

        REQUIRE_ARGUMENT_OBJ(0, SQLiteDb, db);
        if (nullptr == db)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Invalid NativeImportContext", IModelJsNativeErrorKey::BadArg);
        DbResult result = m_importContext->LoadStateFromDb(db->GetDb());
        if (result != DbResult::BE_SQLITE_OK)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to load the state", result);
        return Env().Undefined();
        }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct NativeDevTools : BeObjectWrap<NativeDevTools>
{
    struct Finally
    {
    private:
        std::function<void()> m_finallyCallback;
    public:
        Finally(std::function<void()> finallyCallback) : m_finallyCallback(finallyCallback) {}
        ~Finally() {m_finallyCallback();}
    };

private:
static Napi::Value Signal(NapiInfoCR info)
    {
    if (info.Length() == 0 || !info[0].IsNumber())
        THROW_JS_TYPE_EXCEPTION("Must supply SignalType");
    SignalType signalType = (SignalType) info[0].As<Napi::Number>().Int32Value();
    bool status = SignalTestUtility::Signal(signalType);
    return Napi::Number::New(info.Env(), status);
    }
static void EmitLogs(NapiInfoCR info)
    {
    if (info.Length() != 5)
        THROW_JS_TYPE_EXCEPTION("Must supply 5 arguments");
    REQUIRE_ARGUMENT_UINTEGER(0, count);
    REQUIRE_ARGUMENT_STRING(1, category);
    if (!info[2].IsNumber())
        THROW_JS_TYPE_EXCEPTION("Argument 2 should be a number");
    auto severity = JsLogger::JsLevelToSeverity(info[2].As<Napi::Number>());
    REQUIRE_ARGUMENT_STRING(3, thread);
    REQUIRE_ARGUMENT_FUNCTION(4, onDone);

    auto doneCallback = Napi::ThreadSafeFunction::New(info.Env(), onDone, "Done callback", 0, 1);
    auto doLog = [count, category, severity, doneCallback]()
        {
        Finally f([doneCallback]()
            {
            doneCallback.BlockingCall([=](Napi::Env, Napi::Function jsCallback)
                {
                jsCallback.Call({});
                });
            doneCallback.Release();
            });
        NativeLogging::CategoryLogger logger(category.c_str());
        for (uint32_t i = 0; i < count; ++i)
            {
            logger.message(
                severity,
                Utf8PrintfString("[%u] ", i).append(
                    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                    "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                    ).c_str()
                );
            BeThreadUtilities::BeSleep(1);
            }
        };
    if (thread == "main")
        {
        doLog();
        }
    else if (thread == "worker")
        {
        std::thread workerThread([=](){ doLog(); });
        workerThread.detach();
        }
    else
        {
        THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Unexpected value for `thread` argument. Expecting either \"main\" or \"worker\".", IModelJsNativeErrorKey::RuntimeError);
        }
    }

public:
NativeDevTools(NapiInfoCR info) : BeObjectWrap<NativeDevTools>(info) {}
~NativeDevTools() {SetInDestructor();}

static void Init(Napi::Env env, Napi::Object exports)
    {
    Napi::HandleScope scope(env);
    Napi::Function t = DefineClass(env, "NativeDevTools", {
        StaticMethod("signal", &NativeDevTools::Signal),
        StaticMethod("emitLogs", &NativeDevTools::EmitLogs),
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
        THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), message.c_str(), IModelJsNativeErrorKey::SchemaError);

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

static Napi::Value getLogger(NapiInfoCR info) {
  return s_jsLogger.GetJsLogger();
}
static void setLogger(NapiInfoCR info) {
    s_jsLogger.SetJsLogger(info);
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
        THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Crash reporting is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
        THROW_JS_IMODEL_NATIVE_EXCEPTION(info.Env(), "Crash reporting is not initialized.", IModelJsNativeErrorKey::NotInitialized);

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
    s_jsLogger.Cleanup();
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

static Napi::Value getTrueTypeFontMetadata(NapiInfoCR info) {
    REQUIRE_ARGUMENT_STRING(0, fileName);
    auto ret = Napi::Object::New(info.Env());
    TrueTypeFile ttFile(fileName.c_str(), false);
    BeJsValue retVal(ret);
    ttFile.ExtractMetadata(retVal);
    return ret;
}

static Napi::Value isRscFontData(NapiInfoCR info) {
    REQUIRE_ARGUMENT_ANY_OBJ(0, dataObj);
    auto isRsc = dataObj.IsTypedArray() && RscFont::IsRscFontData(dataObj.As<Napi::Uint8Array>().Data());
    return Napi::Boolean::New(info.Env(), isRsc);
}

static Napi::Value imageBufferFromImageSource(NapiInfoCR info) {
    REQUIRE_ARGUMENT_UINTEGER(0, iSrcFmt);
    if (static_cast<uint32_t>(ImageSource::Format::Png) != iSrcFmt && static_cast<uint32_t>(ImageSource::Format::Jpeg) != iSrcFmt) {
        THROW_JS_TYPE_EXCEPTION("ImageSource format must be Png or Jpeg");
    }

    REQUIRE_ARGUMENT_ANY_OBJ(1, oSrcData);
    if (!oSrcData.IsTypedArray()) {
        THROW_JS_TYPE_EXCEPTION("ImageSource data must be Uint8Array");
    }

    auto srcData = oSrcData.As<Napi::Uint8Array>();
    ImageSource src(static_cast<ImageSource::Format>(iSrcFmt), ByteStream(srcData.Data(), srcData.ByteLength()));

    REQUIRE_ARGUMENT_UINTEGER(2, iImgFmt);
    Image::Format imgFmt;
    if (iImgFmt == 255) {
        // Use Rgb unless alpha channel is present.
        imgFmt = src.SupportsTransparency() ? Image::Format::Rgba : Image::Format::Rgb;
    } else {
        if (static_cast<uint32_t>(Image::Format::Rgb) != iImgFmt && static_cast<uint32_t>(Image::Format::Rgba) != iImgFmt) {
            THROW_JS_TYPE_EXCEPTION("ImageBuffer format must be Rgb or Rgba");
        }

        imgFmt = static_cast<Image::Format>(iImgFmt);
    }

    REQUIRE_ARGUMENT_BOOL(3, flipVertically);

    Image img(src, imgFmt, flipVertically ? Image::BottomUp::Yes : Image::BottomUp::No);
    if (!img.IsValid()) {
        return info.Env().Undefined();
    }

    // JPEG decoder can leave extra junk bytes past the end of the image data. Omit them.
    auto expectedImgDataSize = img.GetWidth() * img.GetHeight() * img.GetBytesPerPixel();
    auto imgDataSize = std::min(expectedImgDataSize, img.GetByteStream().GetSize());
    auto imgData = Napi::Uint8Array::New(info.Env(), imgDataSize);
    memcpy(imgData.Data(), img.GetByteStream().data(), imgDataSize);

    Napi::Object ret = Napi::Object::New(info.Env());
    ret.Set(Napi::String::New(info.Env(), "data"), imgData);
    ret.Set(Napi::String::New(info.Env(), "format"), Napi::Number::New(info.Env(), static_cast<uint32_t>(img.GetFormat())));
    ret.Set(Napi::String::New(info.Env(), "width"), Napi::Number::New(info.Env(), img.GetWidth()));

    return ret;
}

static Napi::Value imageSourceFromImageBuffer(NapiInfoCR info) {
    REQUIRE_ARGUMENT_UINTEGER(0, iImgFmt);
    if (static_cast<uint32_t>(Image::Format::Rgb) != iImgFmt && static_cast<uint32_t>(Image::Format::Rgba) != iImgFmt) {
        THROW_JS_TYPE_EXCEPTION("ImageBuffer format must be Rgb or Rgba");
    }

    REQUIRE_ARGUMENT_ANY_OBJ(1, oImgData);
    if (!oImgData.IsTypedArray()) {
        THROW_JS_TYPE_EXCEPTION("ImageBuffer data must be Uint8Array");
    }

    REQUIRE_ARGUMENT_UINTEGER(2, imgWidth);
    REQUIRE_ARGUMENT_UINTEGER(3, imgHeight);

    auto imgData = oImgData.As<Napi::Uint8Array>();
    Image img(imgWidth, imgHeight, ByteStream(imgData.Data(), imgData.ByteLength()), static_cast<Image::Format>(iImgFmt));
    if (!img.IsValid()) {
        return info.Env().Undefined();
    }

    REQUIRE_ARGUMENT_UINTEGER(4, iSrcFmt);
    ImageSource::Format srcFmt;
    if (iSrcFmt == 255) {
        // Use Jpeg unless alpha channel is present
        srcFmt = Image::Format::Rgba == img.GetFormat() ? ImageSource::Format::Png : ImageSource::Format::Jpeg;
    } else {
        if (static_cast<uint32_t>(ImageSource::Format::Png) != iSrcFmt && static_cast<uint32_t>(ImageSource::Format::Jpeg) != iSrcFmt) {
            THROW_JS_TYPE_EXCEPTION("ImageSource format must be Png or Jpeg");
        }

        srcFmt = static_cast<ImageSource::Format>(iSrcFmt);
    }

    REQUIRE_ARGUMENT_BOOL(5, flipVertically);
    REQUIRE_ARGUMENT_UINTEGER(6, jpegQuality);

    ImageSource src(img, srcFmt, jpegQuality, flipVertically ? Image::BottomUp::Yes : Image::BottomUp::No);
    if (!src.IsValid()) {
        return info.Env().Undefined();
    }

    auto data = Napi::Uint8Array::New(info.Env(), src.GetByteStream().size());
    memcpy(data.Data(), src.GetByteStream().data(), src.GetByteStream().size());

    Napi::Object ret = Napi::Object::New(info.Env());
    ret.Set(Napi::String::New(info.Env(), "data"), data);
    ret.Set(Napi::String::New(info.Env(), "format"), static_cast<uint32_t>(src.GetFormat()));

    return ret;
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
    NativeSchemaUtility::Init(env, exports);
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
        Napi::PropertyDescriptor::Function(env, exports, "getCrashReportProperties", &getCrashReportProperties),
        Napi::PropertyDescriptor::Function(env, exports, "getTileVersionInfo", &getTileVersionInfo),
        Napi::PropertyDescriptor::Function(env, exports, "queryConcurrency", &queryConcurrency),
        Napi::PropertyDescriptor::Function(env, exports, "setCrashReporting", &setCrashReporting),
        Napi::PropertyDescriptor::Function(env, exports, "setCrashReportProperty", &setCrashReportProperty),
        Napi::PropertyDescriptor::Function(env, exports, "setMaxTileCacheSize", &setMaxTileCacheSize),
        Napi::PropertyDescriptor::Function(env, exports, "getTrueTypeFontMetadata", &getTrueTypeFontMetadata),
        Napi::PropertyDescriptor::Function(env, exports, "isRscFontData", &isRscFontData),
        Napi::PropertyDescriptor::Function(env, exports, "imageBufferFromImageSource", &imageBufferFromImageSource),
        Napi::PropertyDescriptor::Function(env, exports, "imageSourceFromImageBuffer", &imageSourceFromImageBuffer),
    });

    registerCloudSqlite(env, exports);

    return exports;
}

NODE_API_MODULE(iModelJsNative, registerModule)
} // namespace IModelJsNative
