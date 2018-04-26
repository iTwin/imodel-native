/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJavaScript/V8InspectorManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
