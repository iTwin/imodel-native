/*--------------------------------------------------------------------------------------+
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
#include "CrashProcessor.h"
#include <Bentley/Desktop/FileSystem.h>

#define NOMINMAX
#include <map>
#include <string>
#include <vector>
#include <crashpad/client/crashpad_client.h>
#include <crashpad/client/settings.h>
#include <crashpad/client/crash_report_database.h>
#include <crashpad/client/crashpad_info.h>

USING_NAMESPACE_BENTLEY
using namespace std;
using namespace crashpad;

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*
    N.B.

    - Be careful adding assert calls. Unit tests cannot deliver the
    required handler .exe, so this code needs to fail gracefully (and not
    needlessly break tests).

    - Use fprintf(stderr, ...) instead of logging because crash reporting
    may be initialized /before/ logging.
*/
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
static base::FilePath getHandlerPath()
    {
    BeFileName handlerPathW = Desktop::FileSystem::GetLibraryDir();

#if defined(BENTLEYCONFIG_OS_WINDOWS)
    handlerPathW.AppendToPath(L"CrashpadHandler.exe");
#else
    handlerPathW.AppendToPath(L"CrashpadHandler");
#endif
    
    if (!handlerPathW.DoesPathExist())
        {
        fprintf(stderr, "%s does NOT exist, so cannot enable crash processing.\n", Utf8String(handlerPathW).c_str());
        return base::FilePath();
        }

#if defined(BENTLEYCONFIG_OS_WINDOWS)
    return base::FilePath(handlerPathW.c_str());
#else
    return base::FilePath(Utf8String(handlerPathW).c_str());
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
static base::FilePath getDbPath()
    {
#if defined(BENTLEYCONFIG_OS_WINDOWS)
    BeFileName dbPathW(_wgetenv(L"TEMP"));
#else
    BeFileName dbPathW(getenv("TEMP"), BentleyCharEncoding::Utf8);
#endif

    if (0 == dbPathW.size())
        {
        BeAssert(false);
        fprintf(stderr, "Could not determine TEMP directory, so cannot enable crash processing.\n");
        return base::FilePath();
        }

    dbPathW.AppendToPath(L"iModelBridgeFwkCrashes");

    if (!dbPathW.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(dbPathW.c_str()))
            {
            BeAssert(false);
            fprintf(stderr, "Could not create TEMP directory %s, so cannot enable crash processing.\n", Utf8String(dbPathW).c_str());
            return base::FilePath();
            }
        }
    
#if defined(BENTLEYCONFIG_OS_WINDOWS)
    return base::FilePath(dbPathW.c_str());
#else
    return base::FilePath(Utf8String(dbPathW).c_str());
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
CrashProcessor::CrashProcessor() {}
CrashProcessor::~CrashProcessor() {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
static CrashProcessor* s_instance = nullptr;
CrashProcessor* CrashProcessor::GetInstance() { return s_instance; }
CrashProcessor& CrashProcessor::CreateSentryInstance(Utf8CP appName)
    {
    if (s_instance != nullptr)
        return *s_instance;

    s_instance = new CrashProcessor();

    //.............................................................................................
    base::FilePath handlerPath = getHandlerPath();
    if (handlerPath.empty())
        return *s_instance;
    
    base::FilePath dbPath = getDbPath();
    if (dbPath.empty())
        return *s_instance;
    
    string reportingUrl = GetSentryReportingUrl(appName);
    // DO NOT early return if empty -- allow for crash processing, even if not uploading, for local diagnostics.

    map<string, string> annotations;
    // Our compliance/legal folks have requested that every crash report contain this blanket statement.
    annotations["NOTICE"] = "This information is confidential and proprietary property of Bentley Systems Inc.";

    //.............................................................................................
    vector<string> handlerArgs;
    if (reportingUrl.size() > 0)
        {
        unique_ptr<CrashReportDatabase> database = CrashReportDatabase::Initialize(dbPath);
        if (database == nullptr || database->GetSettings() == nullptr)
            { BeAssert(false); return *s_instance; }

        database->GetSettings()->SetUploadsEnabled(true);
        handlerArgs.push_back("--no-rate-limit"); // don't restrict to once per-hour
        }

    //.............................................................................................
    s_instance->m_client = make_unique<CrashpadClient>();
    if (s_instance->m_client->StartHandler(
            handlerPath,    // handler
            dbPath,         // database
            dbPath,         // metrics_dir
            reportingUrl,   // url
            annotations,    // annotations
            handlerArgs,    // arguments
            false,          // restartable
            false           // asynchronous_start
            ))
        {
        BeAssert(false);
        return *s_instance;
        }
    
    return *s_instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
BentleyStatus CrashProcessor::SetAnnotation(Utf8CP key, Utf8CP val)
    {
    CrashpadInfo* info = CrashpadInfo::GetCrashpadInfo();
    if (info == nullptr)
        return BSIERROR;
    
    SimpleStringDictionary* annotations = info->simple_annotations();
    if (annotations == nullptr)
        {
        annotations = new SimpleStringDictionary();
        info->set_simple_annotations(annotations);
        }
    
    annotations->SetKeyValue(key, val);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
BentleyStatus CrashProcessor::SetAnnotation(CommonAnnotation key, Utf8CP val)
    {
    Utf8CP keyStr = nullptr;
    switch (key)
        {
        case CommonAnnotation::JOB_Id: keyStr = "JOB_Id"; break;
        case CommonAnnotation::JOB_CorrelationId: keyStr = "JOB_CorrelationId"; break;
        case CommonAnnotation::IMH_UserName: keyStr = "IMH_UserName"; break;
        case CommonAnnotation::IMH_RpositoryName: keyStr = "IMH_RpositoryName"; break;
        case CommonAnnotation::IMH_ProjectId: keyStr = "IMH_ProjectId"; break;
        default:
            return BSIERROR;
        }

    return SetAnnotation(keyStr, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
void CrashProcessor::CreateDump(LPEXCEPTION_POINTERS exPtrs)
    {
    m_client->DumpWithoutCrash(exPtrs);
    }
