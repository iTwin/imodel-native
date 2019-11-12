/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "NativeSQLiteStatement.h"
#include "NativeSQLiteDb.h"
#include "ConversionUtils.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
using namespace IModelBank;

Napi::FunctionReference NativeSQLiteStatement::s_constructor;

bool NativeSQLiteStatement::InstanceOf(Napi::Value val)
{
    if (!val.IsObject())
        return false;

    Napi::HandleScope scope(val.Env());
    return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
}

Napi::Value NativeSQLiteStatement::Prepare(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENT_OBJ(0, NativeSQLiteDb, db, Env().Undefined());
    REQUIRE_ARGUMENT_STRING(1, sql);
    auto rc = m_stmt->Prepare(db->GetDb(), sql.c_str());
    return ConversionUtils::CreateErrorObject0(rc, (BE_SQLITE_OK != rc) ? db->GetDb().GetLastError().c_str() : nullptr, Env());
}

void NativeSQLiteStatement::Reset(const Napi::CallbackInfo &info)
{
    m_stmt->Reset();
}

void NativeSQLiteStatement::ClearBindings(const Napi::CallbackInfo &info)
{
    m_stmt->ClearBindings();
}

void NativeSQLiteStatement::Dispose(const Napi::CallbackInfo &info)
{
    m_stmt = nullptr;
}

Napi::Value NativeSQLiteStatement::Step(const Napi::CallbackInfo &info)
{
    auto rc = m_stmt->Step();
    return Napi::Number::New(Env(), (int)rc);
}

Napi::Value NativeSQLiteStatement::BindValues(const Napi::CallbackInfo &info)
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

Napi::Value NativeSQLiteStatement::GetRow(const Napi::CallbackInfo &info)
{
    auto row = Napi::Object::New(Env());

    for (int i = 0, n = m_stmt->GetColumnCount(); i < n; ++i)
    {
        row[m_stmt->GetColumnName(i)] = ConversionUtils::GetValue(*m_stmt, i, Env());
    }

    return row;
}

void NativeSQLiteStatement::Init(Napi::Env env, Napi::Object exports)
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
