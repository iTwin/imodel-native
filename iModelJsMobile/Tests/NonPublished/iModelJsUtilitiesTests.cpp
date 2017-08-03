/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/iModelJsUtilitiesTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
static Js::Value JsCallback (Js::CallbackInfoCR info)
    {
    BeAssert (&info.GetRuntime() == s_jsRuntime);
    BeAssert (info.GetThis() == s_jsRuntime->GetGlobal());
    BeAssert (!info.IsConstructCall());
    BeAssert (info.GetArgumentCount() == 0);

    return info.GetRuntime().EvaluateScript ("1 + 1").value;
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

    {
        Js::Runtime runtime;
        s_jsRuntime = &runtime;

        {
            Js::Scope scope (runtime);
            BeAssert (&scope.GetRuntime() == &runtime);

            BeAssert (runtime.GetGlobal() == runtime.GetGlobal());

            auto result = runtime.EvaluateScript ("1 + 1");
            BeAssert (result.status == Js::EvaluateStatus::Success);
            BeAssert (result.value.AsNumber().GetValue() == 2.0);

            auto u = scope.CreateUndefined();
            BeAssert (u.IsUndefined());

            auto n = scope.CreateNull();
            BeAssert (n.IsNull());

            auto o = scope.CreateObject();
            BeAssert (o.IsObject());

            auto a = scope.CreateArray();
            BeAssert (a.IsArray());

            auto ab = scope.CreateArrayBuffer (this, sizeof (this));
            BeAssert (ab.IsArrayBuffer());
            BeAssert (ab.GetValue() == this);
            BeAssert (ab.GetLength() == sizeof (this));

            auto b = scope.CreateBoolean (true);
            BeAssert (b.IsBoolean());
            BeAssert (b.GetValue() == true);

            auto n2 = scope.CreateNumber (2.0);
            BeAssert (n2.IsNumber());
            BeAssert (n2.GetValue() == 2.0);

            auto s = scope.CreateString ("hello");
            BeAssert (s.IsString());
            BeAssert (s.GetValue() == "hello");

            auto s2 = s;
            BeAssert (s2.GetValue() == s.GetValue());

            auto c = scope.CreateCallback (&JsCallback);
            BeAssert (c.IsFunction());
            BeAssert (c (runtime.GetGlobal()).AsNumber().GetValue() == 2.0);

            auto e = scope.CreateExternal (this);
            BeAssert (e.IsExternal() && e.IsObject());
            BeAssert (e.GetValue() == this);

            auto p = scope.CreatePointer (this);
            BeAssert (p.IsPointer());
            BeAssert (p.GetValue() == this);

            auto c2 = scope.CreateCallback ([](Js::CallbackInfoCR info) -> Js::Value
                {
                BeAssert (info.GetArgumentCount() == 11);
    
                info.GetArgument (0).AsUndefined();
                info.GetArgument (1).AsNull();
                info.GetArgument (2).AsObject();
                info.GetArgument (3).AsArray();
                BeAssert (info.GetArgument (4).AsArrayBuffer().GetValue() == s_jsExternal && info.GetArgument (4).AsArrayBuffer().GetLength() == s_jsExternalSize);
                BeAssert (info.GetArgument (5).AsBoolean().GetValue() == true);
                BeAssert (info.GetArgument (6).AsNumber().GetValue() == 2.0);
                BeAssert (info.GetArgument (7).AsString().GetValue() == "hello");
                BeAssert (info.GetArgument (8).AsCallback() (info.GetRuntime().GetGlobal()).AsNumber().GetValue() == 2.0);
                BeAssert (info.GetArgument (9).AsExternal().GetValue() == s_jsExternal);
                BeAssert (info.GetArgument (10).AsPointer().GetValue() == s_jsExternal);

                return info.GetScope().CreateUndefined();
                });
                
            c2 (runtime.GetGlobal(), u, n, o, a, ab, b, n2, s, c, e, p);

            BeAssert (a.GetLength() == 0);
            BeAssert (a.Set (0, n2));
            BeAssert (a.GetLength() == 1);
            BeAssert (a.Get (0).AsNumber().GetValue() == 2.0);

            BeAssert (o.Set ("n2", n2));
            BeAssert (o.Has ("n2") && o.HasOwn ("n2"));
            BeAssert (o.Get ("n2").AsNumber().GetValue() == 2.0);

            auto objectWithPrototype = scope.CreateObject();
            auto prototypeForObject = scope.CreateObject();
            BeAssert (objectWithPrototype.SetPrototype (prototypeForObject));
            BeAssert (objectWithPrototype.GetPrototype() == prototypeForObject);
        }

        {
            Js::Reference u (runtime);
            Js::Reference n (runtime);
            Js::Reference o (runtime);
            Js::Reference a (runtime);
            Js::Reference ab (runtime);
            Js::Reference b (runtime);
            Js::Reference n2 (runtime);
            Js::Reference s (runtime);
            Js::Reference c (runtime);
            Js::Reference e (runtime);
            Js::Reference p (runtime);

            {
                Js::Scope scope (runtime);

                u.Assign (scope.CreateUndefined());
                n.Assign (scope.CreateNull());
                o.Assign (scope.CreateObject());
                a.Assign (scope.CreateArray());
                ab.Assign (scope.CreateArrayBuffer (this, sizeof (this)));
                b.Assign (scope.CreateBoolean (true));
                n2.Assign (scope.CreateNumber (2.0));
                s.Assign (scope.CreateString ("hello"));
                c.Assign (scope.CreateCallback (&JsCallback));
                e.Assign (scope.CreateExternal (this));
                p.Assign (scope.CreatePointer (this));

                u.Get().AsUndefined();
                n.Get().AsNull();
                o.Get().AsObject();
                a.Get().AsArray();

                BeAssert (ab.Get().AsArrayBuffer().GetValue() == this && ab.Get().AsArrayBuffer().GetLength() == sizeof (this));
                BeAssert (b.Get().AsBoolean().GetValue() == true);
                BeAssert (n2.Get().AsNumber().GetValue() == 2.0);
                BeAssert (s.Get().AsString().GetValue() == "hello");
                BeAssert (c.Get().AsCallback() (runtime.GetGlobal()).AsNumber().GetValue() == 2.0);
                BeAssert (e.Get().AsExternal().GetValue() == this);
                BeAssert (p.Get().AsPointer().GetValue() == this);
            }

            Js::Scope scope (runtime);
            u.Get().AsUndefined();
            n.Get().AsNull();
            o.Get().AsObject();
            a.Get().AsArray();
            BeAssert (ab.Get().AsArrayBuffer().GetValue() == this && ab.Get().AsArrayBuffer().GetLength() == sizeof (this));
            BeAssert (b.Get().AsBoolean().GetValue() == true);
            BeAssert (n2.Get().AsNumber().GetValue() == 2.0);
            BeAssert (s.Get().AsString().GetValue() == "hello");
            BeAssert (c.Get().AsCallback() (runtime.GetGlobal()).AsNumber().GetValue() == 2.0);
            BeAssert (e.Get().AsExternal().GetValue() == this);
            BeAssert (p.Get().AsPointer().GetValue() == this);
        }
    }

    }
