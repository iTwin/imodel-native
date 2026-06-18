/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#define NOMINMAX

#include "IModelJsNative.h"
#include <Bentley/Desktop/FileSystem.h>

#include <crashpad/client/crash_report_database.h>
#include <crashpad/client/settings.h>
#include <crashpad/client/crashpad_client.h>
#include <crashpad/client/crashpad_info.h>

#undef MAP_TYPE // linux.h #define's this, while ECSchema.h uses it as a template parameter name

using namespace std;
using namespace crashpad;
using namespace IModelJsNative;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void InitFilePath(base::FilePath& filePath, BeFileNameCR fileName)
    {
#ifdef BENTLEY_WIN32
    filePath = base::FilePath(fileName.c_str());
#else
    filePath = base::FilePath(Utf8String(fileName).c_str());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetEnvironmentVariable(std::string& value, const char* variableName)
    {
    value.clear();
#ifdef BENTLEY_WIN32
    char* buf = nullptr;
    size_t len = 0;
    if (_dupenv_s(&buf, &len, variableName) == 0 && buf != nullptr)
        {
        value.assign(buf);
        free(buf);
        return true;
        }
    return false;
#else
    const char* env = getenv(variableName);
    if (env != nullptr)
        {
        value.assign(env);
        return true;
        }
    return false;
#endif
    }

// WIP: See comments about MaintainCrashDumpDir below.
// static int s_nextNativeCrashTxtFileNo;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

    for (auto const& annotation : GetCrashReportPropertiesFromConfig(cfg))
        SetCrashReportProperty(annotation.first.c_str(), annotation.second.c_str());
    
    //.............................................................................................
    // WIP: Check whether the user wants dumps at all via CrashReportingConfig.
    // if (!cfg.m_enableCrashDumps)
    //     return;
#if defined(BENTLEYCONFIG_OS_LINUX)
    // The old environment variable for enabling crash dumps on Linux was "LINUX_MINIDUMP_ENABLED". We only want to
    // support that on Linux. However, even on Linux we want to allow users to switch to the new
    // "IMODEL_ADDON_MINIDUMP_ENABLED" variable, so we check for both and require at least one of them to be set. On
    // other platforms, only check for "IMODEL_ADDON_MINIDUMP_ENABLED". 
    if (NULL == getenv("LINUX_MINIDUMP_ENABLED"))
        return;
#endif
    
    std::string minidumpEnabledStr;
    if (!GetEnvironmentVariable(minidumpEnabledStr, "IMODEL_ADDON_MINIDUMP_ENABLED"))
        return;
    
    //.............................................................................................
    // WIP: solely rely on configuration for crash path.
    BeFileName dbPathW = cfg.m_crashDir;
    if (0 == dbPathW.size())
        {
#ifdef BENTLEY_WIN32
        wchar_t tempPath[MAX_PATH];
        DWORD len = GetTempPath2W(MAX_PATH, tempPath);
        if (len == 0 || len >= MAX_PATH)
            dbPathW.assign(L"C:\\temp\\crash");
        else
            {
            dbPathW.assign(tempPath);
            dbPathW.AppendToPath(L"crash");
            }
#else
        dbPathW.assign(L"/tmp/crash");
#endif
        }

    if (!dbPathW.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(dbPathW.c_str()))
            {
            // BeAssert(false);
            printf("JsInterop::InitializeCrashReporting: Failed to create crashpad database directory %s\n", Utf8String(dbPathW).c_str());
            return;
            }
        }
    
    base::FilePath dbPath;
    InitFilePath(dbPath, dbPathW);
    unique_ptr<CrashReportDatabase> database = CrashReportDatabase::Initialize(dbPath);
    if (nullptr == database || nullptr == database->GetSettings())
        {
        // BeAssert(false);
        printf("JsInterop::InitializeCrashReporting: Failed to initialize crashpad database in path %s\n", Utf8String(dbPathW).c_str());
        return;
        }

    // WIP: Check for presence of upload URL and/or config option to determine whether to attempt uploads.
    std::string reportingUrlEnv;
    bool hasReportingUrlEnv = GetEnvironmentVariable(reportingUrlEnv, "MINIDUMP_UPLOAD_URL");
    
    database->GetSettings()->SetUploadsEnabled(hasReportingUrlEnv);

    //.............................................................................................
    BeFileName handlerPathW = Desktop::FileSystem::GetLibraryDir();
#ifdef BENTLEY_WIN32
    handlerPathW.AppendToPath(L"CrashpadHandler.exe");
#else
    handlerPathW.AppendToPath(L"CrashpadHandler");
#endif
    if (!handlerPathW.DoesPathExist())
        {
        // BeAssert(false);
        printf("JsInterop::InitializeCrashReporting: 'CrashpadHandler' application not found: %s\n", Utf8String(handlerPathW).c_str());
        return;
        }

    base::FilePath handlerPath;
    InitFilePath(handlerPath, handlerPathW);

    // WIP: Get the URL from configuration.
    string reportingUrl;
    if (hasReportingUrlEnv)
        reportingUrl.assign(reportingUrlEnv);
    
    // backtrace.io requires some manual annotations.
    std::map<std::string, std::string> processAnnotations;
    std::string backtraceToken;
    if (GetEnvironmentVariable(backtraceToken, "MINIDUMP_UPLOAD_BACKTRACE_TOKEN"))
        {
        processAnnotations["format"] = "minidump";
        processAnnotations["token"] = backtraceToken;
        }

    vector<string> args;
    args.push_back("--no-rate-limit"); // don't restrict to once per-hour
    // args.push_back("--no-upload-gzip"); // don't compress HTTP request (for debugging purposes)

    static CrashpadClient client;
    // @todo: Decide if we want to pass true for restartable and asynchronous_start on macOS and maybe Windows.
    if (!client.StartHandler(
            handlerPath,        // handler
            dbPath,             // database
            dbPath,             // metrics_dir
            reportingUrl,       // url
            processAnnotations, // annotations
            args,               // arguments
#if defined(BENTLEYCONFIG_OS_LINUX)
            false,  // restartable (not supported on Linux)
            false   // asynchronous_start (not supported on Linux)
#else
            true,   // restartable
            true    // asynchronous_start
#endif
        ))
        {
        // BeAssert(false);
        printf("JsInterop::InitializeCrashReporting: Failed to start the crashpad handler.\n");
        return;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::SetCrashReportProperty(Utf8CP key, Utf8CP val)
    {
    CrashpadInfo* info = CrashpadInfo::GetCrashpadInfo();
    if (info == nullptr)
        return;

    SimpleStringDictionary* annotations = info->simple_annotations();
    if (annotations == nullptr)
        {
        annotations = new SimpleStringDictionary();
        info->set_simple_annotations(annotations);
        }

    annotations->SetKeyValue(key, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::map<std::string, std::string> JsInterop::GetCrashReportProperties()
    {
    std::map<std::string, std::string> props;

    CrashpadInfo* info = CrashpadInfo::GetCrashpadInfo();
    if (info == nullptr)
        return props;

    SimpleStringDictionary* annotations = info->simple_annotations();
    if (annotations == nullptr)
        return props;

    SimpleStringDictionary::Iterator dictionaryIter(*annotations);
    for (auto dictionaryEntry = dictionaryIter.Next(); dictionaryEntry != nullptr; dictionaryEntry = dictionaryIter.Next())
        props[dictionaryEntry->key] = dictionaryEntry->value;

    return props;
    }
