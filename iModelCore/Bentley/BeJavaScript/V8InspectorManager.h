/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/Bentley.h>
#include <google_v8/v8.h>

BEGIN_BENTLEY_NAMESPACE 

struct V8InspectorManager
    {
    static void AttachIsolate(v8::Isolate* isolate, const char* port);
    static void DetachIsolate(v8::Isolate* isolate);

    static void AttachContext(v8::Local<v8::Context> context, const char* name);
    static void DetachContext(v8::Local<v8::Context> context);

    static void SetMainThreadDispatcher(void(*mainThreadDispatcher)(void(*callback)(void*arg), void*arg));

    static void EnableDebugging();
    static void DisableDebugging();
    };

END_BENTLEY_NAMESPACE
