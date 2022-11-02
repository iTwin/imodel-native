/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
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

PUSH_DISABLE_DEPRECATION_WARNINGS
using namespace IModelJsNative;

bmap<Dgn::DgnDb*, BeFileName> JsInterop::s_openDgnDbFileNames;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::AddCrashReportDgnDb(Dgn::DgnDbR db)
    {
    s_openDgnDbFileNames[&db] = db.GetFileName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::RemoveCrashReportDgnDb(Dgn::DgnDbR db)
    {
    s_openDgnDbFileNames.erase(&db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::FormatCurrentTime(char* buf, size_t maxbuf)
    {
    std::time_t now = std::time(nullptr);
    std::tm *gmtm = std::gmtime(&now);
    std::strftime(buf, maxbuf, "%FT%TZ", gmtm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        if ((1 == WString::Swscanf_safe(entryName.GetBaseName().c_str(), L"iModelJsNativeCrash-%d.txt", &i)) && (i > maxNativeCrashTxtFileNo))
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::map<std::string,std::string> JsInterop::GetCrashReportPropertiesFromConfig(CrashReportingConfig const& cfg)
    {
    std::map<std::string,std::string> props = cfg.m_params;

    props["iModelJsNativeVersion"] = PACKAGE_VERSION;

    char buf[128];
    JsInterop::FormatCurrentTime(buf, sizeof(buf));
    props["LoadTime"] = buf;

    return props;
    }
POP_DISABLE_DEPRECATION_WARNINGS
