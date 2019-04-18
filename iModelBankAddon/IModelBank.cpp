/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "IModelBank.h"
#include <Bentley/Desktop/FileSystem.h>
#include <Napi/napi.h>
#include <imodel-bank-package-version.h>
#include <BeSQLite/ChangeSet.h>
#include <BeSQLite/RevisionChangesFile.h>
#include "DgnSqlFuncsForTriggers.h"

#if defined(_WIN32)
#include <windows.h>
#include <delayimp.h>

// On Windows, we delay load node.lib. It declares all of its exports to be from "node.exe".
// We Register a delay-load hook to redirect any node.exe references to be from the loading executable.
// This way the addon will still work even if the host executable is not named node.exe.
static FARPROC delayLoadNotify(unsigned dliNotify, PDelayLoadInfo pdli) {return (dliNotePreLoadLibrary != dliNotify || 0 != _stricmp(pdli->szDll, "node.exe")) ? nullptr : (FARPROC)::GetModuleHandle(nullptr);}
decltype(__pfnDliNotifyHook2) __pfnDliNotifyHook2 = delayLoadNotify;
#endif

#define PROPERTY_ATTRIBUTES static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)

#define OPTIONAL_ARGUMENT_STRING(i, var)                                                                         \
    Utf8String var;                                                                                              \
    if (info.Length() <= (i) || info[i].IsUndefined())                                                           \
    {                                                                                                            \
        ;                                                                                                        \
    }                                                                                                            \
    else if (info[i].IsString())                                                                                 \
    {                                                                                                            \
        var = info[i].As<Napi::String>().Utf8Value().c_str();                                                    \
    }                                                                                                            \
    else                                                                                                         \
    {                                                                                                            \
        Napi::TypeError::New(Env(), "Argument " #i " must be string or undefined").ThrowAsJavaScriptException(); \
    }

#define REQUIRE_ARGUMENT_STRING(i, var)                                                                    \
    if (info.Length() <= (i) || !info[i].IsString())                                                       \
    {                                                                                                      \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a string").ThrowAsJavaScriptException(); \
    }                                                                                                      \
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

#define REQUIRE_ARGUMENT_INTEGER(i, var)                                                                \
    if (info.Length() <= (i) || !info[i].IsNumber())                                                    \
    {                                                                                                   \
        Napi::TypeError::New(Env(), "Argument " #i " must be an integer").ThrowAsJavaScriptException(); \
    }                                                                                                   \
    int32_t var = info[i].As<Napi::Number>().Int32Value();

#define REQUIRE_ARGUMENT_BOOLEAN(i, var)                                                               \
    if (info.Length() <= (i) || !info[i].IsBoolean())                                                  \
    {                                                                                                  \
        Napi::TypeError::New(Env(), "Argument " #i " must be a boolean").ThrowAsJavaScriptException(); \
    }                                                                                                  \
    bool var = info[i].As<Napi::Boolean>().Value();

#define REQUIRE_ARGUMENT_ANY_OBJ(i, var, retval)                                     \
    if (info.Length() <= (i))                                                        \
    {                                                                                \
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an object", retval) \
    }                                                                                \
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_OBJ(i, T, var, retval)                                                  \
    if (info.Length() <= (i) || !T::InstanceOf(info[i]))                                         \
    {                                                                                            \
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an object of type " #T, retval) \
    }                                                                                            \
    T *var = T::Unwrap(info[i].As<Napi::Object>());

#define THROW_JS_TYPE_ERROR(str) Napi::TypeError::New(Env(), str).ThrowAsJavaScriptException();

#define THROW_TYPE_EXCEPTION_AND_RETURN(str, retval) \
    {                                                \
        THROW_JS_TYPE_ERROR(str)                     \
        return retval;                               \
    }

#define THROW_EXCEPTION_AND_RETURN(str, retval) \
    {                                           \
        THROW_JS_ERROR(str)                     \
        return retval;                          \
    }

#define THROW_JS_ERROR(str) Napi::Error::New(Env(), str).ThrowAsJavaScriptException();

#define RETURN_IF_HAD_EXCEPTION     \
    if (Env().IsExceptionPending()) \
        return Env().Undefined();

//=======================================================================================
// Namespace IModelBank
//=======================================================================================
namespace IModelBank
{
static BeFileName s_addonDllDir;
static intptr_t s_mainThreadId;
static Napi::Env *s_env;
static Napi::ObjectReference s_logger;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getLogger(Napi::CallbackInfo const &info)
{
    return s_logger.Value();
}

Napi::ObjectReference &jsInterop_getLogger() { return s_logger; }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void setLogger(Napi::CallbackInfo const &info)
{
    s_logger = Napi::ObjectReference::New(info[0].ToObject());
    s_logger.SuppressDestruct();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename STATUSTYPE>
static Napi::Object createErrorObject0(STATUSTYPE errCode, Utf8CP msg, Napi::Env env)
{
    Napi::Object error = Napi::Object::New(env);
    error.Set(Napi::String::New(env, "status"), Napi::Number::New(env, (int)errCode));
    if (nullptr != msg)
        error.Set(Napi::String::New(env, "message"), Napi::String::New(env, msg));
    return error;
}

//=======================================================================================
// SQLite <-> JavaScript data conversion
//! @bsiclass
//=======================================================================================
struct ConversionUtils
{
    // Get a SQLite Statement column value as a JS value
    static Napi::Value GetValue(Statement &stmt, int i, Napi::Env env)
    {
        switch (stmt.GetColumnType(i))
        {
        case DbValueType::IntegerVal:
            return Napi::Number::New(env, stmt.GetValueInt(i));
        case DbValueType::FloatVal:
            return Napi::Number::New(env, stmt.GetValueDouble(i));
        case DbValueType::TextVal:
            return Napi::String::New(env, stmt.GetValueText(i));
        case DbValueType::NullVal:
            return env.Null();
        case DbValueType::BlobVal:
        {
            auto abuf = Napi::ArrayBuffer::New(env, stmt.GetColumnBytes(i));
            memcpy(abuf.Data(), stmt.GetValueBlob(i), stmt.GetColumnBytes(i));
            return abuf;
        }

        default:
            BeAssert(false);
            return env.Undefined();
        }
    }

    // Bind a JS value to a SQLite Statement parameter
    static BentleyStatus BindValue(Statement &stmt, int i, Napi::Value value)
    {
        BentleyStatus status = BSISUCCESS;
        switch (value.Type())
        {
        case napi_undefined:
        case napi_null:
            stmt.BindNull(i);
            break;
        case napi_boolean:
            stmt.BindInt(i, value.ToBoolean().Value());
            break;
        case napi_number:
            stmt.BindDouble(i, value.ToNumber().DoubleValue());
            break;

        case napi_string:
        case napi_symbol:
            stmt.BindText(i, value.ToString().Utf8Value().c_str(), Statement::MakeCopy::Yes);
            break;
        // case napi_object :
        // case napi_function :
        // case napi_external :
        default:
            BeAssert(false);
            status = BSIERROR;
        }
        return status;
    }
};

struct ChangeSetCounts
{
    uint32_t inserts{};
    uint32_t deletes{};
    uint32_t updates{};
};

struct ChangeSetApplyStats
{
    bool containsSchemaChanges{};
    ChangeSetCounts element;
    ChangeSetCounts aspect;
    ChangeSetCounts model;
};

static bool isElementTable(Utf8CP n)
{
    return (0 == strcmp(n, "bis_Element"));
}

static bool isModelTable(Utf8CP n)
{
    return (0 == strcmp(n, "bis_Model"));
}

static bool isAspectTable(Utf8CP n)
{
    return ((0 == strcmp(n, "bis_ElementMultiAspect")) || (0 == strcmp(n, "bis_ElementUniqueAspect")));
}

static void countChanges(ChangeSetApplyStats &stats, Changes const &changes)
{
    for (auto change : changes)
    {
        Utf8CP tn;
        int nCols, indirect;
        DbOpcode opcode;
        if (BE_SQLITE_OK == change.GetOperation(&tn, &nCols, &opcode, &indirect))
        {
            ChangeSetCounts *counts = isElementTable(tn) ? &stats.element : isModelTable(tn) ? &stats.model : isAspectTable(tn) ? &stats.aspect : nullptr;
            if (nullptr == counts)
                continue;
            switch (opcode)
            {
            case DbOpcode::Delete:
                ++counts->deletes;
                break;
            case DbOpcode::Insert:
                ++counts->inserts;
                break;
            default:
                ++counts->updates;
                break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                       07/2018
//---------------------------------------------------------------------------------------
static DbResult applyChangeSet(ChangeSetApplyStats &stats, Db &db, bvector<BeFileName> const &blockFileNames)
{
    RevisionChangesFileReaderBase changesetReader(blockFileNames, db);

    // Apply DDL, if any
    DbSchemaChangeSet dbSchemaChanges;
    bool containsSchemaChanges;
    DbResult result = changesetReader.GetSchemaChanges(containsSchemaChanges, dbSchemaChanges);
    if (result != BE_SQLITE_OK)
    {
        //        BeAssert(false);
        return result;
    }

    if (containsSchemaChanges && !dbSchemaChanges.IsEmpty())
    {
        stats.containsSchemaChanges = true;

        DbResult status = db.ExecuteSql(dbSchemaChanges.ToString().c_str());
        if (BE_SQLITE_OK != status)
            return status;
    }

    countChanges(stats, changesetReader.GetChanges());

    // Apply normal/data changes (including ec_ table data changes)
    return changesetReader.ApplyChanges(db);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void throwJsExceptionOnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
{
    Utf8PrintfString formattedMessage("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
    if (BeThreadUtilities::GetCurrentThreadId() == s_mainThreadId)
        Napi::Error::New(*s_env, formattedMessage.c_str()).ThrowAsJavaScriptException();
    else
        JsInterop::LogMessage("iModelBank", NativeLogging::SEVERITY::LOG_ERROR, formattedMessage.c_str());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/18
//---------------------------------------------------------------------------------------
static void initialize(BeFileNameCR addonDllDir)
{
    s_addonDllDir = addonDllDir;

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() {
        BeFileName tempDir;
        Desktop::FileSystem::BeGetTempPath(tempDir);
        BeSQLiteLib::Initialize(tempDir);
        JsInterop::InitLogging();
        BeAssertFunctions::SetBeAssertHandler(throwJsExceptionOnAssert);
#ifdef _WIN32
        static bool s_quietAsserts = true;
        if (s_quietAsserts)
            _set_error_mode(_OUT_TO_STDERR);
        else
            _set_error_mode(_OUT_TO_MSGBOX);
#endif
    });
}

} // namespace IModelBank

using namespace IModelBank;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsInterop::IsMainThread()
{
    return (BeThreadUtilities::GetCurrentThreadId() == s_mainThreadId);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelBank::JsInterop::ThrowJsException(Utf8CP msg)
{
    Napi::Error::New(*s_env, msg).ThrowAsJavaScriptException();
}

namespace IModelBank
{
//=======================================================================================
// Projects the Db class into JS
//! @bsiclass
//=======================================================================================
struct NativeSQLiteDb : Napi::ObjectWrap<NativeSQLiteDb>
{
  private:
    static Napi::FunctionReference s_constructor;
    std::unique_ptr<Db> m_db;

  public:
    NativeSQLiteDb(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NativeSQLiteDb>(info) {}
    ~NativeSQLiteDb() {}

    // Check if val is really a NativeSQLiteDb peer object
    static bool InstanceOf(Napi::Value val)
    {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
    }
    Db &GetDb()
    {
        if (m_db == nullptr)
            m_db = std::make_unique<Db>();

        return *m_db;
    }

    Napi::Value CreateDb(const Napi::CallbackInfo &info)
    {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        REQUIRE_ARGUMENT_INTEGER(1, defaultTxn);
        RETURN_IF_HAD_EXCEPTION
        BeSQLite::Db::CreateParams params;
        params.SetStartDefaultTxn((BeSQLite::DefaultTxn)defaultTxn);
        DbResult status = GetDb().CreateNewDb(BeFileName(dbName.c_str(), true), BeGuid(), params);
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value OpenDb(const Napi::CallbackInfo &info)
    {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        REQUIRE_ARGUMENT_INTEGER(2, defaultTxn);
        RETURN_IF_HAD_EXCEPTION
        DbResult status = GetDb().OpenBeSQLiteDb(BeFileName(dbName.c_str(), true), BeSQLite::Db::OpenParams((Db::OpenMode)mode, (BeSQLite::DefaultTxn)defaultTxn));
        if ((BeSQLite::BE_SQLITE_OK == status) && GetDb().IsDbOpen())
            DgnSqlFuncsForTriggers::Register(GetDb());
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value CloseDb(const Napi::CallbackInfo &info)
    {
        if (m_db != nullptr)
        {
            m_db->CloseDb();
            m_db = nullptr;
        }
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
    }

    Napi::Value SaveChanges(const Napi::CallbackInfo &info)
    {
        OPTIONAL_ARGUMENT_STRING(0, changeSetName);
        RETURN_IF_HAD_EXCEPTION
        const DbResult status = GetDb().SaveChanges(changeSetName.empty() ? nullptr : changeSetName.c_str());
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value AbandonChanges(const Napi::CallbackInfo &info)
    {
        DbResult status = GetDb().AbandonChanges();
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value GetDbGuid(const Napi::CallbackInfo &info)
    {
        auto dbGuid = GetDb().GetDbGuid();
        return Napi::String::New(Env(), dbGuid.ToString().c_str());
    }

    Napi::Value CreateTable(const Napi::CallbackInfo &info)
    {
        REQUIRE_ARGUMENT_STRING(0, tableName);
        REQUIRE_ARGUMENT_STRING(1, ddl);
        DbResult status = GetDb().CreateTable(tableName.c_str(), ddl.c_str());
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value IsOpen(const Napi::CallbackInfo &info) { return Napi::Boolean::New(Env(), GetDb().IsDbOpen()); }

    // To catch any SQLite error when it happens, put a BP on sqlite3ErrorMsg

    /*
        Prereqs:
        Define SQLite functions: DGN_bbox, DGN_bbox_value, DGN_placement_aabb, DGN_placement, DGN_point, DGN_angles
    */

    Napi::Object countsToJs(ChangeSetCounts &counts)
    {
        auto countsJs = Napi::Object::New(Env());
        countsJs["inserts"] = Napi::Number::New(Env(), counts.inserts);
        countsJs["deletes"] = Napi::Number::New(Env(), counts.deletes);
        countsJs["updates"] = Napi::Number::New(Env(), counts.updates);
        return countsJs;
    }

    Napi::Value ApplyChangeset(const Napi::CallbackInfo &info)
    {
        if (info.Length() < 1 || !info[0].IsArray())
            Napi::TypeError::New(info.Env(), "Argument 1 must be a string[]").ThrowAsJavaScriptException();
        auto blockFileNamesJs = info[0].As<Napi::Array>();
        bvector<BeFileName> blockFileNames;
        for (uint32_t i = 0; i < blockFileNamesJs.Length(); ++i)
        {
            Napi::Value item = blockFileNamesJs[i];
            blockFileNames.push_back(BeFileName(item.ToString().Utf8Value().c_str(), true));
        }

        ChangeSetApplyStats stats;
        auto status = applyChangeSet(stats, GetDb(), blockFileNames);

        auto statsJs = Napi::Object::New(Env());
        statsJs["containsSchemaChanges"] = Napi::Boolean::New(Env(), stats.containsSchemaChanges);
        statsJs["status"] = Napi::Number::New(Env(), (int)status);
        statsJs["element"] = countsToJs(stats.element);
        statsJs["model"] = countsToJs(stats.model);
        statsJs["aspect"] = countsToJs(stats.aspect);
        return statsJs;
    }

    static void Init(Napi::Env env, Napi::Object exports)
    {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeSQLiteDb", {InstanceMethod("getDbGuid", &NativeSQLiteDb::GetDbGuid), InstanceMethod("applyChangeSet", &NativeSQLiteDb::ApplyChangeset), InstanceMethod("createTable", &NativeSQLiteDb::CreateTable), InstanceMethod("createDb", &NativeSQLiteDb::CreateDb), InstanceMethod("openDb", &NativeSQLiteDb::OpenDb), InstanceMethod("closeDb", &NativeSQLiteDb::CloseDb), InstanceMethod("saveChanges", &NativeSQLiteDb::SaveChanges), InstanceMethod("abandonChanges", &NativeSQLiteDb::AbandonChanges), InstanceMethod("isOpen", &NativeSQLiteDb::IsOpen)});

        exports.Set("NativeSQLiteDb", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
    }
};

//=======================================================================================
// Projects SQLite::Statement into JS
//! @bsiclass
//=======================================================================================
struct NativeSQLiteStatement : Napi::ObjectWrap<NativeSQLiteStatement>
{
  private:
    static Napi::FunctionReference s_constructor;
    std::unique_ptr<Statement> m_stmt;

  public:
    NativeSQLiteStatement(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NativeSQLiteStatement>(info), m_stmt(new Statement()) {}
    ~NativeSQLiteStatement() {}

    // Check if val is really a NativeSQLiteDb peer object
    static bool InstanceOf(Napi::Value val)
    {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
    }

    Napi::Value Prepare(const Napi::CallbackInfo &info)
    {
        REQUIRE_ARGUMENT_OBJ(0, NativeSQLiteDb, db, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, sql);
        auto rc = m_stmt->Prepare(db->GetDb(), sql.c_str());
        return createErrorObject0(rc, (BE_SQLITE_OK != rc) ? db->GetDb().GetLastError().c_str() : nullptr, Env());
    }

    void Reset(const Napi::CallbackInfo &info)
    {
        m_stmt->Reset();
    }

    void ClearBindings(const Napi::CallbackInfo &info)
    {
        m_stmt->ClearBindings();
    }

    void Dispose(const Napi::CallbackInfo &info)
    {
        m_stmt = nullptr;
    }

    Napi::Value Step(const Napi::CallbackInfo &info)
    {
        auto rc = m_stmt->Step();
        return Napi::Number::New(Env(), (int)rc);
    }

    Napi::Value BindValues(const Napi::CallbackInfo &info)
    {
        m_stmt->ClearBindings();

        REQUIRE_ARGUMENT_ANY_OBJ(0, values, Env().Undefined());
        if (values.IsArray())
        {
            auto array = values.As<Napi::Array>();
            for (int i = 0, n = array.Length(); i < n; ++i)
            {
                auto status = ConversionUtils::BindValue(*m_stmt, i + 1, array.Get(i)); // (Note that SQLite parameter indices are 1-based)
                if (BSISUCCESS != status)
                {
                    THROW_TYPE_EXCEPTION_AND_RETURN(Utf8PrintfString("Bad parameter type: %d", i).c_str(), Env().Undefined());
                }
            }
        }
        else
        {
            auto propNames = values.GetPropertyNames();
            for (int iPropName = 0, nPropNames = propNames.Length(); iPropName < nPropNames; ++iPropName)
            {
                auto propName = propNames.Get(iPropName).ToString();
                Utf8String paramName(":");
                paramName.append(propName.Utf8Value().c_str());
                int iCol = m_stmt->GetParameterIndex(paramName.c_str());
                if (iCol == 0)
                {
                    THROW_EXCEPTION_AND_RETURN(Utf8PrintfString("Named parameter not found in statement: %s", propName.Utf8Value().c_str()).c_str(), Env().Undefined());
                }
                auto status = ConversionUtils::BindValue(*m_stmt, iCol, values.Get(propName));
                if (BSISUCCESS != status)
                {
                    THROW_TYPE_EXCEPTION_AND_RETURN(Utf8PrintfString("Bad parameter type: %s", propName.Utf8Value().c_str()).c_str(), Env().Undefined());
                }
            }
        }
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
    }

    Napi::Value GetRow(const Napi::CallbackInfo &info)
    {
        auto row = Napi::Object::New(Env());

        for (int i = 0, n = m_stmt->GetColumnCount(); i < n; ++i)
        {
            row[m_stmt->GetColumnName(i)] = ConversionUtils::GetValue(*m_stmt, i, Env());
        }

        return row;
    }

    static void Init(Napi::Env env, Napi::Object exports)
    {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeSQLiteStatement", {InstanceMethod("bindValues", &NativeSQLiteStatement::BindValues), InstanceMethod("prepare", &NativeSQLiteStatement::Prepare), InstanceMethod("clearBindings", &NativeSQLiteStatement::ClearBindings), InstanceMethod("reset", &NativeSQLiteStatement::Reset), InstanceMethod("dispose", &NativeSQLiteStatement::Dispose), InstanceMethod("step", &NativeSQLiteStatement::Step), InstanceMethod("getRow", &NativeSQLiteStatement::GetRow)});

        exports.Set("NativeSQLiteStatement", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
    }
};

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object registerModule(Napi::Env env, Napi::Object exports)
{
    s_env = new Napi::Env(env);

    Napi::HandleScope scope(env);

    BeFileName addondir = Desktop::FileSystem::GetLibraryDir();

    s_mainThreadId = BeThreadUtilities::GetCurrentThreadId();

    initialize(addondir);

    NativeSQLiteDb::Init(env, exports);
    NativeSQLiteStatement::Init(env, exports);

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
        Napi::PropertyDescriptor::Accessor(env, exports, "logger", &getLogger, &setLogger)
        });
    return exports;
}

Napi::FunctionReference IModelBank::NativeSQLiteDb::s_constructor;
Napi::FunctionReference IModelBank::NativeSQLiteStatement::s_constructor;

NODE_API_MODULE(imodel_bank, registerModule)

} // namespace IModelBank
