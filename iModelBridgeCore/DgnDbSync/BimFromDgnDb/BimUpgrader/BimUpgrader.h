/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <windows.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <BimFromDgnDb/BimFromDgnDb.h>
#include <Logging/bentleylogging.h>

namespace BentleyB0200
    {
    namespace Dgn
        {
        struct DgnProgressMeter;
        }
    }

BEGIN_BIM_FROM_DGNDB_NAMESPACE

struct BimUpgraderHost;

struct BimUpgrader
{
private:
    BeFileName m_inputFileName;
    BeFileName m_outputPath;
    BeFileName m_loggingConfigFileName;
    BimUpgraderHost* m_host;
    DgnProgressMeter* m_meter;
    bool m_compress;

    void PrintError(WCharCP fmt, ...);
    void _PrintMessage(WCharCP fmt, ...);
    BentleyStatus GetLogConfigurationFilename(BeFileName& configFile, WCharCP argv0);
    void InitLogging(WCharCP argv0);
    WString GetArgValueW(WCharCP arg);

    int _PrintUsage(WCharCP programName);
    BentleyStatus _ParseCommandLine(WStringR errmsg, int argc, WCharCP argv[]);
    BentleyStatus _Initialize(int argc, WCharCP argv[]);
    static BentleyApi::NativeLogging::ILogger& GetLogger() { return *BentleyApi::NativeLogging::LoggingManager::GetLogger("BimTeleporter"); }

public:
    //! wmain should call this to run the job.
    int Run(int argc, WCharCP argv[]);

};


END_BIM_FROM_DGNDB_NAMESPACE