/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "IModelBank.h"
#include "macros.h"
#include <BeSQLite/BeSQLite.h>

namespace IModelBank
{
struct NativeSQLiteStatement : Napi::ObjectWrap<NativeSQLiteStatement>
{
  private:
    static Napi::FunctionReference s_constructor;
    std::unique_ptr<BeSQLite::Statement> m_stmt;

  public:
    NativeSQLiteStatement(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NativeSQLiteStatement>(info), m_stmt(new BeSQLite::Statement()) {}
    ~NativeSQLiteStatement() {OBJECT_WRAP_DTOR_DISABLE_JS_CALLS}

    // Check if val is really a NativeSQLiteDb peer object
    static bool InstanceOf(Napi::Value val);
    Napi::Value Prepare(const Napi::CallbackInfo &info);
    void Reset(const Napi::CallbackInfo &info);
    void ClearBindings(const Napi::CallbackInfo &info);
    void Dispose(const Napi::CallbackInfo &info);
    Napi::Value Step(const Napi::CallbackInfo &info);
    Napi::Value BindValues(const Napi::CallbackInfo &info);
    Napi::Value GetRow(const Napi::CallbackInfo &info);
    static void Init(Napi::Env env, Napi::Object exports);
};
} // namespace IModelBank
