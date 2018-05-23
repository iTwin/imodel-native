/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/src/BimTeleporterInternal.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <windows.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <BimTeleporter/BimTeleporter.h>
#include <Logging/bentleylogging.h>

#if defined (__BIMTELEPORTER_BUILD__)
#   define BIMTELEPORTER_EXPORT      EXPORT_ATTRIBUTE
#else
#   define BIMTELEPORTER_EXPORT      IMPORT_ATTRIBUTE
#endif

namespace BentleyB0200
    {
    namespace Dgn
        {
        struct DgnProgressMeter;
        }
    }

BEGIN_BIM_TELEPORTER_NAMESPACE

struct BimTeleporterHost;

struct BimTeleporter
{
private:
    BeFileName m_inputFileName;
    BeFileName m_outputPath;
    BeFileName m_loggingConfigFileName;
    BimTeleporterHost* m_host;
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
    BIMTELEPORTER_EXPORT int Run(int argc, WCharCP argv[]);

};


END_BIM_TELEPORTER_NAMESPACE