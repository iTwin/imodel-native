/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include "IModelBank.h"
#include "macros.h"
#include <BeSQLite/BeSQLite.h>

namespace IModelBank
{
struct NativeSQLiteDb : Napi::ObjectWrap<NativeSQLiteDb>
{
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

private:
    static Napi::FunctionReference s_constructor;
    std::unique_ptr<BeSQLite::Db> m_db;

public:
    NativeSQLiteDb(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NativeSQLiteDb>(info) {}
    ~NativeSQLiteDb() { OBJECT_WRAP_DTOR_DISABLE_JS_CALLS }

    // Check if val is really a NativeSQLiteDb peer object
    static bool InstanceOf(Napi::Value val);
    BeSQLite::Db &GetDb();
    Napi::Value CreateDb(const Napi::CallbackInfo &info);
    Napi::Value OpenDb(const Napi::CallbackInfo &info);
    Napi::Value CloseDb(const Napi::CallbackInfo &info);
    Napi::Value SaveChanges(const Napi::CallbackInfo &info);
    Napi::Value AbandonChanges(const Napi::CallbackInfo &info);
    Napi::Value GetDbGuid(const Napi::CallbackInfo &info);
    Napi::Value CreateTable(const Napi::CallbackInfo &info);
    Napi::Value IsOpen(const Napi::CallbackInfo &info);
    Napi::Object countsToJs(ChangeSetCounts &counts);
    Napi::Value ApplyChangeset(const Napi::CallbackInfo &info);
    static void Init(Napi::Env env, Napi::Object exports);
};

} // namespace IModelBank
