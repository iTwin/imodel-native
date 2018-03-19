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

void Test () {
    std::cout << "NAPI - JavaScriptCore Tests\n";
    BentleyApi::iModelJs::Js::Runtime runtime;
    
//    Napi::Env& env = runtime.Env();
    
    auto result = runtime.EvaluateScript ("1 + 1");
    assert (result.status == BentleyB0200::iModelJs::Js::EvaluateStatus::Success);
    assert (result.value.As<Napi::Number>().DoubleValue() == 2.0);
}
