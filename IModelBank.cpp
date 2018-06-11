/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInterop.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelBank.h"
#include <Bentley/Desktop/FileSystem.h>
#include <node-addon-api/napi.h>
#include <imodel-bank-package-version.h>

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

#define RETURN_IF_HAD_EXCEPTION     \
    if (Env().IsExceptionPending()) \
        return Env().Undefined();

namespace IModelBank
{
static BeFileName s_addonDllDir;
static IModelBank::JsInterop::T_AssertHandler s_assertHandler;
static intptr_t s_mainThreadId;
static Napi::Env *s_env;
static Napi::ObjectReference s_logger;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void throwJsExceptionOnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
{
    if (JsInterop::IsMainThread())
        JsInterop::ThrowJsException(Utf8PrintfString("Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str());
}

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

} // namespace IModelBank

using namespace IModelBank;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/18
//---------------------------------------------------------------------------------------
void JsInterop::Initialize(BeFileNameCR addonDllDir, T_AssertHandler assertHandler)
{
    s_addonDllDir = addonDllDir;
    s_assertHandler = assertHandler;

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() {
        BeFileName tempDir;
        Desktop::FileSystem::BeGetTempPath(tempDir);
        BeSQLiteLib::Initialize(tempDir);
        InitLogging();
    });
}

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

//=======================================================================================
// Projects the Db class into JS
//! @bsiclass
//=======================================================================================
namespace IModelBank
{
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
        RETURN_IF_HAD_EXCEPTION
        DbResult status = GetDb().CreateNewDb(BeFileName(dbName.c_str(), true));
        return Napi::Number::New(Env(), (int)status);
    }

    Napi::Value OpenDb(const Napi::CallbackInfo &info)
    {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        RETURN_IF_HAD_EXCEPTION
        DbResult status = GetDb().OpenBeSQLiteDb(BeFileName(dbName.c_str(), true), BeSQLite::Db::OpenParams((Db::OpenMode)mode));
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

    Napi::Value IsOpen(const Napi::CallbackInfo &info) { return Napi::Boolean::New(Env(), GetDb().IsDbOpen()); }

    static void Init(Napi::Env env, Napi::Object exports)
    {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeSQLiteDb", {InstanceMethod("createDb", &NativeSQLiteDb::CreateDb), InstanceMethod("openDb", &NativeSQLiteDb::OpenDb), InstanceMethod("closeDb", &NativeSQLiteDb::CloseDb), InstanceMethod("saveChanges", &NativeSQLiteDb::SaveChanges), InstanceMethod("abandonChanges", &NativeSQLiteDb::AbandonChanges), InstanceMethod("isOpen", &NativeSQLiteDb::IsOpen)});

        exports.Set("NativeSQLiteDb", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
    }
};
} // namespace IModelBank

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object iModelBankRegisterModule(Napi::Env env, Napi::Object exports)
{
    s_env = new Napi::Env(env);

    Napi::HandleScope scope(env);

    BeFileName addondir = Desktop::FileSystem::GetLibraryDir();

    s_mainThreadId = BeThreadUtilities::GetCurrentThreadId();

    IModelBank::JsInterop::Initialize(addondir, IModelBank::throwJsExceptionOnAssert);

    IModelBank::NativeSQLiteDb::Init(env, exports);

    exports.DefineProperties(
        {
            Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
            Napi::PropertyDescriptor::Accessor("logger", &IModelBank::getLogger, &IModelBank::setLogger),
        });

    return exports;
}

Napi::FunctionReference IModelBank::NativeSQLiteDb::s_constructor;

NODE_API_MODULE(at_bentley_imodel_bank_nodeaddon, iModelBankRegisterModule)
