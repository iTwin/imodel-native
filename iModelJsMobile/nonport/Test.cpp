//
//  Test.cpp
//  NAPI-JSC-Tests
//
//  Created by Satyakam Khadilkar on 3/19/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#include "Test.h"
#include <iostream>
#include <iModelJs/iModelJs.h>
#include <iModelJs/iModelJsServicesTier.h>

struct TestObject : Napi::ObjectWrap<TestObject>
{
    TestObject(const Napi::CallbackInfo& info) : Napi::ObjectWrap<TestObject>(info)
    {}
    
    static void Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);
        Napi::Function t = TestObject::DefineClass(env, "TestObject", {
            TestObject::InstanceMethod("TestFunc", &TestObject::TestFunc),
            StaticMethod("TestStaticFunc", &TestObject::TestStaticFunc),
        });
        
        exports.Set("TestObject", t);
        
        exports.Set("Multiply", Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value {
            Napi::Number val1 = info[0].ToNumber();
            Napi::Number val2 = info[1].ToNumber();
            return Napi::Number::New(info.Env(), val1.DoubleValue() * val2.DoubleValue());
        }));
    }
    
    Napi::Value TestFunc(const Napi::CallbackInfo& info)
    {
        return Napi::Number::New(Env(), 42.0);
    }
    
    static Napi::Value TestStaticFunc(const Napi::CallbackInfo& info)
    {
        return Napi::Number::New(info.Env(), 3.14);
    }
};

extern void imodeljs_addon_setMobileResourcesDir(Utf8CP d);
void Test (const char* resourcePath) {
    std::cout << "NAPI - JavaScriptCore Tests\n";
    BentleyApi::iModelJs::Js::Runtime runtime;
    
    imodeljs_addon_setMobileResourcesDir(resourcePath);
    
    Napi::Env& env = runtime.Env();
    BentleyApi::iModelJs::ServicesTier::UvHost host;
    while (!host.IsReady()) { ; }

    auto result = runtime.EvaluateScript ("1 + 1");
    assert (result.status == BentleyB0200::iModelJs::Js::EvaluateStatus::Success);
    assert (result.value.As<Napi::Number>().DoubleValue() == 2.0);
    
    auto u = env.Undefined();
    assert (u.IsUndefined());
    
    auto n = env.Null();
    assert (n.IsNull());
    
    auto o = Napi::Object::New(env);
    assert (o.IsObject());
    
    auto a = Napi::Array::New(env);
    assert (a.IsArray());
    
    auto b = Napi::Boolean::New(env, true);
    assert (b.IsBoolean());
    assert (b.Value() == true);
    
    auto n2 = Napi::Number::New(env, 2.0);
    assert (n2.IsNumber());
    assert (n2.DoubleValue() == 2.0);
    
    auto s = Napi::String::New(env, "hello");
    assert (s.IsString());
    assert (s.Utf8Value() == "hello");
    
    auto s2 = s;
    assert (s2.Utf8Value() == s.Utf8Value());
    
    TestObject::Init(env, env.Global());
    auto result1 = runtime.EvaluateScript ("Multiply(7.0,6.0);");
    assert (result1.status == BentleyB0200::iModelJs::Js::EvaluateStatus::Success);
    assert (result1.value.As<Napi::Number>().DoubleValue() == 42.0);
    
    auto result2 = runtime.EvaluateScript ("var a = new TestObject(1.0);");
    auto result3 = runtime.EvaluateScript ("a.TestFunc()");
    assert (result3.value.As<Napi::Number>().DoubleValue() == 42.0);
    
    auto result4 = runtime.EvaluateScript ("TestObject.TestStaticFunc()");
    assert (result4.value.As<Napi::Number>().DoubleValue() == 3.14);
}
