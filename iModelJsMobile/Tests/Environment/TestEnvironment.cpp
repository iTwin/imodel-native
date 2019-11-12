/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/TestEnvironment.h"

#if defined (_WIN32) && !defined (BENTLEY_WINRT)
#include <windows.h>
#endif

static std::atomic<size_t> s_failedJsAssertCount;

BEGIN_IMODELJS_UNIT_TESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void iModelJsTestFixture::SetUpTestCase()
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void iModelJsTestFixture::TearDownTestCase()
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
iModelJsTestFixture::iModelJsTestFixture()
    : m_standalone (false)
    {
#ifdef BENTLEYCONFIG_OS_WINDOWS
    WString argv (::GetCommandLineW());
    if (argv.find (L"--standalone") != std::string::npos)
        m_standalone = true;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
iModelJsTestFixture::~iModelJsTestFixture()
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void iModelJsTestFixture::SetUp()
    {
    Test::SetUp();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void iModelJsTestFixture::TearDown()
    {
    Test::TearDown();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void iModelJsTestFixture::RunSystemMessageLoop()
    {
#if defined (_WIN32) && !defined (BENTLEY_WINRT)
    MSG msg;
    BOOL ret;

    for (;;)
        {
        ret = ::GetMessage (&msg, nullptr, 0, 0);
        if (ret > 0)
            {
            ::TranslateMessage (&msg);
            ::DispatchMessage (&msg);
            }
        else if (ret < 0)
            {
            BeAssert (false);
            }
        else
            {
            break;
            }
        }
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void iModelJsTestFixture::InstallTestingUtilities (Js::RuntimeR runtime)
    {
    auto& config = ServicesTier::Host::GetConfig();
    Napi::Env env = runtime.Env();
    if (config.enableJsDebugger && config.waitForJsDebugger)
        {
        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (function() {
                return function (condition, message) {
                    if (arguments.length === 1)
                        var message = "Assertion Failed";

                    if (!condition)
                        {
                        if (message instanceof Error)
                            throw message;
                        else
                            throw new Error (message);
                        }
                    };
            })();
        )");

        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);
        env.Global().Set ("BeAssert", evaluateResult.value);
        }
    else
        {
        env.Global().Set ("BeAssert", Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
            {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

            auto condition = JS_CALLBACK_GET_BOOLEAN (0);

            if (!condition.Value())
                {
                ++s_failedJsAssertCount;
                std::string message("BeAssert");
                if (info.Length() > 1)
                    {
                    message = info[1].As<Napi::String>().Utf8Value();
                    }
                Napi::Error::New(info.Env(), message.c_str()).ThrowAsJavaScriptException();
                }

            return JS_CALLBACK_UNDEFINED;
            }));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void iModelJsTestFixture::ResetFailedJsAssertCount()
    {
    s_failedJsAssertCount = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
size_t iModelJsTestFixture::GetFailedJsAssertCount()
    {
    return s_failedJsAssertCount;
    }

END_IMODELJS_UNIT_TESTS_NAMESPACE
