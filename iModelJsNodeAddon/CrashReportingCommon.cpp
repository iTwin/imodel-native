/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSchemaXmlContextUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BentleyConfig.h>
#include <Bentley/BeDirectoryIterator.h>
#include "IModelJsNative.h"
#include <imodeljs-nodeaddonapi.package.version.h>

#ifndef BENTLEYCONFIG_64BIT_HARDWARE
    #error 64-bit builds only 
#endif

using namespace IModelJsNative;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::MaintainCrashDumpDir(CrashReportingConfig const& cfg)
    {
    if (!cfg.m_crashDumpDir.DoesPathExist())
        BeFileName::CreateNewDirectory(cfg.m_crashDumpDir);

    size_t count = 0;

    BeFileName entryName;
    bool        isDir;
    for (BeDirectoryIterator dirs(cfg.m_crashDumpDir); dirs.GetCurrentEntry(entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (!isDir)
            ++count;
        }

    // Make sure there is room for one more!
    if (count <= (cfg.m_maxDumpsInDir - 1))
        return;
    
    // TODO: Sort oldest to newest and delete the oldest first.
    for (BeDirectoryIterator dirs(cfg.m_crashDumpDir); dirs.GetCurrentEntry(entryName, isDir) == SUCCESS; dirs.ToNext())
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

    props["ver"] = PACKAGE_VERSION;

    return props;
    }
