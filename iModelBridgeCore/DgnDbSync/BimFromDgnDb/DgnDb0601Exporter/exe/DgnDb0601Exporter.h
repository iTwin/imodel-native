/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/DgnDb0601Exporter/exe/DgnDb0601Exporter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

#if defined (__BIMTELEPORTER_BUILD__)
#   define BIMTELEPORTER_EXPORT      EXPORT_ATTRIBUTE
#else
#   define BIMTELEPORTER_EXPORT      IMPORT_ATTRIBUTE
#endif

#define BIM_EXPORTER_NAMESPACE_NAME BimTeleporter
#define BEGIN_BIM_EXPORTER_NAMESPACE namespace BentleyG0601 { namespace Dgn { namespace BIM_EXPORTER_NAMESPACE_NAME {
#define END_BIM_EXPORTER_NAMESPACE   } } }

BEGIN_BIM_EXPORTER_NAMESPACE

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
        BIMTELEPORTER_EXPORT int Run(int argc, WCharCP argv[]);

    };

END_BIM_EXPORTER_NAMESPACE