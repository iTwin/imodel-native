/*--------------------------------------------------------------------------------------+
|
|   Supplied under applicable software license agreement.
|
|   Copyright (c) 2016 Bentley Systems, Incorporated. All rights reserved.
|
+---------------------------------------------------------------------------------------*/
#pragma once

#include "../BeHttp/Tests.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScriptRunner
    {
    private:
        struct ProccessInfo
            {
            Utf8String name;
            Utf8String pid;
            };

    private:
        static Utf8String ExecuteCommand(Utf8CP command);
        static void ExecuteCommand(Utf8CP command, Utf8String& output);
        static void GetDerivedProcesses(Utf8CP pid, bvector<ProccessInfo>& derived);

    public:
        // Run script from "Scripts/Executables/" folder with context of "Scripts/Resources/"
        static void RunScript(Utf8StringCR scriptName, Utf8String* output = nullptr);
        // Run script from "Scripts/Executables/" folder with context of "Scripts/Resources/" in it own thread
        static void RunScriptAsync(Utf8StringCR scriptName, Utf8String* output = nullptr);
        // Stop all long-running python scripts like server/proxy/etc.
        // Windows child processes do not stop by default otherwise.
        static void StopAllPythonScripts();
    };

END_BENTLEY_HTTP_NAMESPACE
