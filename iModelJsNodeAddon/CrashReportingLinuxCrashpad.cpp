/*--------------------------------------------------------------------------------------+
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BentleyConfig.h>
#include <Bentley/Desktop/FileSystem.h>

#ifndef BENTLEYCONFIG_OS_LINUX
    #error This file is for Linux only
#endif

#include <crashpad/client/crash_report_database.h>
#include <crashpad/client/settings.h>
#include <crashpad/client/crashpad_client.h>

#undef MAP_TYPE // linux.h #define's this, while ECSchema.h uses it as a template parameter name
#include "IModelJsNative.h"

using namespace std;
using namespace crashpad;
using namespace IModelJsNative;

// WIP: See comments about MaintainCrashDumpDir below.
// static int s_nextNativeCrashTxtFileNo;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::InitializeCrashReporting(CrashReportingConfig const& cfg)
    {
    // https://docs.sentry.io/platforms/minidump/
    // https://docs.sentry.io/platforms/minidump/crashpad/
    // https://crashpad.chromium.org/
    // https://crashpad.chromium.org/doxygen/index.html

    //.............................................................................................
    // WIP: Update or replace MaintainCrashDumpDir to be able to prune crashpad's dump directory layout.
    // MaintainCrashDumpDir(s_nextNativeCrashTxtFileNo, cfg);
    
    //.............................................................................................
    // WIP: Check whether the user wants dumps at all via CrashReportingConfig.
    // if (!cfg.m_enableCrashDumps)
    //     return;
    if (NULL == getenv("LINUX_MINIDUMP_ENABLED"))
        return;
    
    //.............................................................................................
    // WIP: solely rely on configuration for crash path.
    BeFileName dbPathW = cfg.m_crashDir;
    if (0 == dbPathW.size())
        dbPathW.assign(L"/tmp/crash");

    if (!dbPathW.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(dbPathW.c_str()))
            {
            // BeAssert(false);
            printf("JsInterop::InitializeCrashReporting: Failed to create crashpad database directory %s\n", Utf8String(dbPathW).c_str());
            return;
            }
        }
    
    base::FilePath dbPath(Utf8String(dbPathW).c_str());
    unique_ptr<CrashReportDatabase> database = CrashReportDatabase::Initialize(dbPath);
    if (nullptr == database || nullptr == database->GetSettings())
        {
        // BeAssert(false);
        printf("JsInterop::InitializeCrashReporting: Failed to initialize crashpad database in path %s\n", Utf8String(dbPathW).c_str());
        return;
        }

    // WIP: Check for presence of upload URL and/or config option to determine whether to attempt uploads.
    Utf8CP reportingUrlEnv = getenv("MINIDUMP_UPLOAD_URL");
    bool hasReportingUrlEnv = !Utf8String::IsNullOrEmpty(reportingUrlEnv);
    
    database->GetSettings()->SetUploadsEnabled(hasReportingUrlEnv);

    //.............................................................................................
    BeFileName handlerPathW = Desktop::FileSystem::GetLibraryDir();
    handlerPathW.AppendToPath(L"CrashpadHandler");
    if (!handlerPathW.DoesPathExist())
        {
        // BeAssert(false);
        printf("JsInterop::InitializeCrashReporting: 'CrashpadHandler' application not found: %s\n", Utf8String(handlerPathW).c_str());
        return;
        }

    base::FilePath handlerPath(Utf8String(handlerPathW).c_str());
    
    // WIP: Get the URL from configuration.
    string reportingUrl;
    if (hasReportingUrlEnv)
        reportingUrl.assign(reportingUrlEnv);

    auto customProperties = GetCrashReportCustomProperties(cfg);
    map<string, string> processAnnotations;
    for (auto const& prop : customProperties)
        processAnnotations[string(prop.first.c_str())] = string(prop.second.c_str());
    
    // backtrace.io requires some manual annotations.
    Utf8CP backtraceToken = getenv("MINIDUMP_UPLOAD_BACKTRACE_TOKEN");
    if (!Utf8String::IsNullOrEmpty(backtraceToken))
        {
        processAnnotations["format"] = "minidump";
        processAnnotations["token"] = backtraceToken;
        }

    vector<string> args;
    args.push_back("--no-rate-limit"); // don't restrict to once per-hour
    // args.push_back("--no-upload-gzip"); // don't compress HTTP request (for debugging purposes)

    CrashpadClient client;
    if (!client.StartHandler(
            handlerPath,        // handler
            dbPath,             // database
            dbPath,             // metrics_dir
            reportingUrl,       // url
            processAnnotations, // annotations
            args,               // arguments
            false,              // restartable (not supported on Linux)
            false               // asynchronous_start (not supported on Linux)
            ))
        {
        // BeAssert(false);
        printf("JsInterop::InitializeCrashReporting: Failed to start the crashpad handler.\n");
        return;
        }
    }
