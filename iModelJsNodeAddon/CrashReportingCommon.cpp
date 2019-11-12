/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BentleyConfig.h>
#include <Bentley/BeDirectoryIterator.h>
#include "IModelJsNative.h"
#include <imodeljs-nodeaddonapi.package.version.h>
#include <ctime>

#ifndef BENTLEYCONFIG_64BIT_HARDWARE
    #error 64-bit builds only 
#endif

using namespace IModelJsNative;

bmap<Utf8String, Utf8String> JsInterop::s_crashReportProperties;
bmap<Dgn::DgnDb*, BeFileName> JsInterop::s_openDgnDbFileNames;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  04/19
//---------------------------------------------------------------------------------------
void JsInterop::AddCrashReportDgnDb(Dgn::DgnDbR db)
    {
    s_openDgnDbFileNames[&db] = db.GetFileName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  04/19
//---------------------------------------------------------------------------------------
void JsInterop::RemoveCrashReportDgnDb(Dgn::DgnDbR db)
    {
    s_openDgnDbFileNames.erase(&db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  04/19
//---------------------------------------------------------------------------------------
void JsInterop::SetCrashReportProperty(Utf8StringCR key, Utf8StringCR value)
    {
    s_crashReportProperties[key] = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  04/19
//---------------------------------------------------------------------------------------
void JsInterop::RemoveCrashReportProperty(Utf8StringCR key)
    {
    s_crashReportProperties.erase(key);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/19
//---------------------------------------------------------------------------------------
void JsInterop::FormatCurrentTime(char* buf, size_t maxbuf)
    {
    std::time_t now = std::time(nullptr);
    std::tm *gmtm = std::gmtime(&now);
    std::strftime(buf, maxbuf, "%FT%TZ", gmtm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::MaintainCrashDumpDir(int& maxNativeCrashTxtFileNo, CrashReportingConfig const& cfg)
    {
    if (!cfg.m_crashDir.DoesPathExist())
        BeFileName::CreateNewDirectory(cfg.m_crashDir);

    size_t count = 0;
    maxNativeCrashTxtFileNo = 0;

    BeFileName entryName;
    bool        isDir;
    for (BeDirectoryIterator dirs(cfg.m_crashDir); dirs.GetCurrentEntry(entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (isDir)
            continue;
            
        ++count;

        int i;
        if ((1 == swscanf(entryName.GetBaseName().c_str(), L"iModelJsNativeCrash-%d.txt", &i)) && (i > maxNativeCrashTxtFileNo))
            maxNativeCrashTxtFileNo = i;
        }

    // Make sure there is room for one more!
    if (count <= (cfg.m_maxDumpsInDir - 1))
        return;
    
    // TODO: Sort oldest to newest and delete the oldest first.
    for (BeDirectoryIterator dirs(cfg.m_crashDir); dirs.GetCurrentEntry(entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (isDir)
            continue;

        BeFileName::BeDeleteFile(entryName);
        if (--count <= cfg.m_maxDumpsInDir/2)
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<Utf8String,Utf8String> JsInterop::GetCrashReportCustomProperties(CrashReportingConfig const& cfg)
    {
    bmap<Utf8String,Utf8String> props = cfg.m_params;

    props["iModelJsNativeVersion"] = PACKAGE_VERSION;

    char buf[128];
    JsInterop::FormatCurrentTime(buf, sizeof(buf));
    props["LoadTime"] = buf;

    return props;
    }
