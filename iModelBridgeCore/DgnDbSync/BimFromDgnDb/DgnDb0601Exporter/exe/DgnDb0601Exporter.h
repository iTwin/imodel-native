/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <windows.h>
#include <BimFromDgnDb/BimFromDgnDb.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformApi.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformLib.h>
#include <DgnDb06Api/DgnPlatform/DgnProgressMeter.h>
#include <DgnDb06Api/Logging/bentleylogging.h>
#include <DgnDb06Api/BeSQLite/L10N.h>

DGNDB06_USING_NAMESPACE_BENTLEY_LOGGING
DGNDB06_USING_NAMESPACE_BENTLEY_SQLITE
DGNDB06_USING_NAMESPACE_BENTLEY_DGN

BEGIN_BIM_FROM_DGNDB_NAMESPACE

struct BimExporter0601
    {
    private:
        BeFileName m_inputFileName;
        BeFileName m_outputPath;

        BeFileName m_loggingConfigFileName;

        void PrintError(WCharCP fmt, ...);
        void PrintMessage(WCharCP fmt, ...);
        void LogMessage(WCharCP fmt, ...);
        BentleyStatus GetLogConfigurationFilename(BeFileName& configFile, WCharCP argv0);
        void InitLogging(WCharCP argv0);
        WString GetArgValueW(WCharCP arg);

        int _PrintUsage(WCharCP programName);
        BentleyStatus _ParseCommandLine(WStringR errmsg, int argc, WCharCP argv[]);
        BentleyStatus _Initialize(int argc, WCharCP argv[]);

    public:
        //! wmain should call this to run the job.
        int Run(int argc, WCharCP argv[]);

    };

END_BIM_FROM_DGNDB_NAMESPACE