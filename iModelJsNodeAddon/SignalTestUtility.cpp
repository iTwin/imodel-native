/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SignalTestUtility.h"
#include "IModelJsNative.h"
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

#if defined (_MSC_VER)
#pragma warning (disable:4723) // divide by zero
#endif

namespace IModelJsNative {

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PUSH_MSVC_IGNORE(4717) // recursive on all control paths, function will cause runtime stack overflow
PUSH_CLANG_IGNORE(unused-value)
PUSH_CLANG_IGNORE(infinite-recursion)
void SignalTestUtility::StackOverflow(int depth)
    {
    char blockdata[10000];
    blockdata;
    printf("Overflow depth: %d\r", depth);
    StackOverflow(depth + 1);
    }
POP_CLANG_IGNORE
POP_CLANG_IGNORE
POP_MSVC_IGNORE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SignalTestUtility::ThreadDeadlock()
    {
    std::mutex m1;
    std::mutex m2;
    std::thread t1([&m1, &m2] {
        m1.lock(); // Acquiring m1
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        m2.lock(); // Acquiring m2
    });
    std::thread t2([&m1, &m2] {
        m2.lock(); // Acquiring m2
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        m1.lock(); // Acquiring m1
    });

    t1.join();
    t2.join();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PUSH_CLANG_IGNORE(unused-value)
PUSH_CLANG_IGNORE(null-dereference)
bool SignalTestUtility::Signal(SignalType signalType)
    {
    bool status = true;
    switch (signalType)
    {
    case SignalType::Abort:
        JsInterop::GetNativeLogger().error("About to crash by calling abort()");
        abort();
        break;
    case SignalType::RaiseSigSev:
        JsInterop::GetNativeLogger().error("About to crash by calling raise(SIGSEGV)");
        raise(SIGSEGV); // simulates a standard crash when access invalid memory
        break;
    case SignalType::DereferenceNull:
        JsInterop::GetNativeLogger().error("About to crash by setting *((unsigned int *)0) = 0xDEAD");
        *((unsigned int *)0) = 0xDEAD;
        break;
    case SignalType::DivideByZero:
        {
        JsInterop::GetNativeLogger().error("About to crash by evaluating 1/0");
        int a = 0;
        double b = 1 / a;
        b;
        break;
        }
    case SignalType::StackOverflow:
        JsInterop::GetNativeLogger().error("About to crash with a StackOverflow");
        StackOverflow(0);
        break;
    case SignalType::ThreadDeadlock:
        JsInterop::GetNativeLogger().error("About to crash with a ThreadDeadlock");
        ThreadDeadlock();
        break;
    case SignalType::None:
    default:
        status = false;
        break;
    }

    return status;
    }
}
POP_CLANG_IGNORE
POP_CLANG_IGNORE
