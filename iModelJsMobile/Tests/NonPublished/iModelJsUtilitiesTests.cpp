/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../Environment/PublicAPI/TestEnvironment.h"

USING_IMODELJS_NAMESPACE
USING_IMODELJS_UNIT_TESTS_NAMESPACE

static Js::RuntimeP s_jsRuntime = nullptr;
static void* s_jsExternal = nullptr;
static size_t s_jsExternalSize = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static Napi::Value JsCallback (Napi::CallbackInfo const& info)
    {
    BeAssert (info.This() == s_jsRuntime->Env().Global());
    BeAssert (!info.IsConstructCall());
    BeAssert (info.Length() == 0);

    Js::RuntimeR runtime = Js::Runtime::GetRuntime(info.Env());
    return runtime.EvaluateScript ("1 + 1").value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
TEST_F (iModelJsTestFixture, JavaScriptUtilities)
    {
    if (IsStandalone())
        return;

    s_jsExternal = this;
    s_jsExternalSize = sizeof (this);

    if (true)
        {
        Js::Runtime runtime;
        s_jsRuntime = &runtime;
        Napi::Env& env = runtime.Env();

        if (true)
            {
            Napi::HandleScope scope (env);

            try {
                auto result = runtime.EvaluateScript ("1 + 1");
                BeAssert (result.status == Js::EvaluateStatus::Success);
                BeAssert (result.value.As<Napi::Number>().DoubleValue() == 2.0);

                auto u = env.Undefined();
                BeAssert (u.IsUndefined());

                auto n = env.Null();
                BeAssert (n.IsNull());

                auto o = Napi::Object::New(env);
                BeAssert (o.IsObject());

                auto a = Napi::Array::New(env);
                BeAssert (a.IsArray());

                auto ab = Napi::ArrayBuffer::New(env, this, sizeof (this));
                BeAssert (ab.IsArrayBuffer());
                BeAssert (ab.Data() == this);
                BeAssert (ab.ByteLength() == sizeof (this));

                auto b = Napi::Boolean::New(env, true);
                BeAssert (b.IsBoolean());
                BeAssert (b.Value() == true);

                auto n2 = Napi::Number::New(env, 2.0);
                BeAssert (n2.IsNumber());
                BeAssert (n2.DoubleValue() == 2.0);

                auto s = Napi::String::New(env, "hello");
                BeAssert (s.IsString());
                BeAssert (s.Utf8Value() == "hello");

                auto s2 = s;
                BeAssert (s2.Utf8Value() == s.Utf8Value());

                auto c = Napi::Function::New(env, &JsCallback);
                BeAssert (c.IsFunction());
                BeAssert (c ({}).As<Napi::Number>().DoubleValue() == 2.0);

                void* pThis = this;
                auto e = Napi::External<void*>::New(env, &pThis);
                // TBD: BeAssert (e.IsExternal() && e.IsObject());
                BeAssert(e.Data() == &pThis);

                auto c2 = Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
                    {
                    BeAssert (info.Length() == 10);
    
                    BeAssert(info[0].IsUndefined());
                    BeAssert(info[1].IsNull());
                    BeAssert(info[2].IsObject());
                    BeAssert(info[3].IsArray());
                    BeAssert(info[4].As<Napi::ArrayBuffer>().Data() == s_jsExternal);
                    BeAssert(info[4].As<Napi::ArrayBuffer>().ByteLength() == s_jsExternalSize);
                    BeAssert(info[5].As<Napi::Boolean>().Value() == true);
                    BeAssert(info[6].As<Napi::Number>().DoubleValue() == 2.0);
                    BeAssert(info[7].As<Napi::String>().Utf8Value() == "hello");
                    BeAssert(info[8].As<Napi::Function>() ({}).As<Napi::Number>().DoubleValue() == 2.0);
                    // *** TBD: BeAssert(info[9].As<Napi::External>().Data() == s_jsExternal);

                    return info.Env().Undefined();
                    });
                
                c2 ({u, n, o, a, ab, b, n2, s, c, e});

                BeAssert (a.Length() == 0);
                a[(uint32_t)0] = n2;
                BeAssert (a.Length() == 1);
                BeAssert (a.Get((uint32_t)0).As<Napi::Number>().DoubleValue() == 2.0);

                o["n2"] = n2;
                BeAssert (o.Has("n2") && o.HasOwnProperty("n2"));
                BeAssert (o.Get("n2").As<Napi::Number>().DoubleValue() == 2.0);

    /* WIP prototype in NAPI?
                auto objectWithPrototype = Napi::Object::New(env);
                auto prototypeForObject = Napi::Object::New(env);
                BeAssert (objectWithPrototype.SetPrototype (prototypeForObject));
                BeAssert (objectWithPrototype.GetPrototype() == prototypeForObject);
                */
                } 
            catch (Napi::Error err)
                {
                fprintf(stderr, "%s\n", err.what());
                BeAssert(false);
                }
            }

        if (true)
            {
            Napi::HandleScope scope (env);
            try
                {
                // The following are not objects and therefore cannot be the target of references: undefined, null, boolean, number, and string.
                auto u = env.Undefined();
                auto n = env.Null();
                auto o = Napi::ObjectReference::New(Napi::Object::New(env));
                auto a = Napi::ObjectReference::New(Napi::Array::New(env));
                auto ab = Napi::ObjectReference::New(Napi::ArrayBuffer::New(env, this, sizeof (this)));
                auto b = Napi::Boolean::New(env, true); // looks like true and false are not objects, either
                auto n2 = Napi::Number::New(env, 2.0);
                auto s = Napi::String::New(env, "hello");
                auto c = Napi::Reference<Napi::Function>::New(Napi::Function::New(env, &JsCallback));
                struct Foo { int i; } foo;
                auto e = Napi::Reference<Napi::External<Foo>>::New(Napi::External<Foo>::New(env, &foo));


                BeAssert (u.IsUndefined());
                BeAssert (!n.IsUndefined());
                BeAssert (n.IsNull());
                BeAssert (!ab.Value().IsUndefined());
                BeAssert (!ab.Value().IsNull());
                BeAssert (ab.Value().As<Napi::ArrayBuffer>().Data() == this && ab.Value().As<Napi::ArrayBuffer>().ByteLength() == sizeof (this));
                BeAssert ( b.As<Napi::Boolean>().Value() == true);
                BeAssert (n2.As<Napi::Number>().DoubleValue() == 2.0);
                BeAssert ( s.As<Napi::String>().Utf8Value() == "hello");
                BeAssert ( c.Value().As<Napi::Function>() ({}).As<Napi::Number>().DoubleValue() == 2.0);
                BeAssert ( e.Value().As<Napi::External<Foo>>().Data() == &foo);
                }
            catch (Napi::Error err)
                {
                fprintf(stderr, "%s\n", err.what());
                BeAssert(false);
                }
            }
        }
    }

    
