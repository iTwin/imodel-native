/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ScriptRunner.h"

#include <Bentley/Tasks/WorkerThread.h>

// Required for ExecuteCommand()
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

// Required for GetCurrentProcessId()
#include <Windows.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScriptRunner::RunScript(Utf8StringCR scriptName, Utf8String* output)
    {
    BeFileName scriptPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(scriptPath);
    scriptPath.AppendToPath(BeFileName("BeHttpIntegrationTests"));
    scriptPath.AppendToPath(BeFileName("Scripts"));
    scriptPath.AppendToPath(BeFileName("Executables"));
    scriptPath.AppendToPath(BeFileName(scriptName));
    EXPECT_TRUE(scriptPath.DoesPathExist());

    BeFileName resourcesPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(resourcesPath);
    resourcesPath.AppendToPath(BeFileName("BeHttpIntegrationTests"));
    resourcesPath.AppendToPath(BeFileName("Scripts"));
    resourcesPath.AppendToPath(BeFileName("Resources"));
    resourcesPath.AppendSeparator();
    EXPECT_TRUE(resourcesPath.DoesPathExist());

    Utf8String command = scriptPath.GetNameUtf8() + " " + resourcesPath.GetNameUtf8();

    Utf8String temp;
    if (nullptr == output)
        output = &temp;

    ExecuteCommand(command.c_str(), *output);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScriptRunner::RunScriptAsync(Utf8StringCR scriptName, Utf8String* output)
    {
    Tasks::WorkerThread::Create(("RynScriptAsync " + scriptName).c_str())->ExecuteAsync([=]
        {
        RunScript(scriptName, output);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScriptRunner::StopAllPythonScripts()
    {
    Utf8PrintfString pid("%d", GetCurrentProcessId());
    bvector<ProccessInfo> derived;
    GetDerivedProcesses(pid.c_str(), derived);

    for (ProccessInfo& info : derived)
        {
        if (info.name != "python.exe")
            continue;

        Utf8PrintfString command("taskkill /f /pid %s > NUL", info.pid.c_str());
        system(command.c_str());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScriptRunner::GetDerivedProcesses(Utf8CP pid, bvector<ProccessInfo>& derived)
    {
    // Get direct children
    Utf8PrintfString command("wmic process where (ParentProcessId=%s) get Caption,ProcessId 2> nul", pid);
    Utf8String output = ExecuteCommand(command.c_str());

    bvector<Utf8String> lines;
    BeStringUtilities::Split(output.c_str(), "\n", lines);

    lines.erase(lines.begin());

    for (auto line : lines)
        {
        bvector<Utf8String> tokens;
        BeStringUtilities::Split(line.c_str(), " ", tokens);
        if (tokens.size() != 2)
            continue;

        ProccessInfo info;
        info.name = tokens[0];
        info.pid = tokens[1];

        derived.push_back(info);

        GetDerivedProcesses(info.pid.c_str(), derived);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScriptRunner::ExecuteCommand(Utf8CP command)
    {
    Utf8String output;
    ExecuteCommand(command, output);
    return output;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScriptRunner::ExecuteCommand(Utf8CP command, Utf8String& output)
    {
    std::array<char, 128> buffer;
    std::shared_ptr<FILE> pipe(_popen(command, "r"), _pclose);
    if (!pipe)
        throw std::runtime_error("popen() failed!");

    while (!feof(pipe.get()))
        {
        if (fgets(buffer.data(), (int) buffer.size(), pipe.get()) != nullptr)
            output += buffer.data();
        }
    }
