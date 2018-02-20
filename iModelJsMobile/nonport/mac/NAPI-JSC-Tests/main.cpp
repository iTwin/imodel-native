//
//  main.cpp
//  NAPI-JSC-Tests
//
//  Created by Satyakam Khadilkar on 2/16/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#include <iostream>
#include <iModelJs/iModelJs.h>

int main(int argc, const char * argv[]) {
    std::cout << "NAPI - JavaScriptCore Tests\n";
    
    BentleyB0200::iModelJs::Js::Runtime runtime;
    Napi::Env& env = runtime.Env();

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

    return 0;
}
