/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/BimImporter/exe/BimImporter.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <windows.h>
#include <BimFromDgnDb/BimFromDgnDb.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <Logging/bentleylogging.h>
#include <BeSQLite/L10N.h>

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BIM_FROM_DGNDB_NAMESPACE

struct KnownDesktopLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    BeFileName m_tempDirectory;
    BeFileName m_executableDirectory;
    BeFileName m_assetsDirectory;

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDirectory; }
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDirectory; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   BentleySystems
    //---------------------------------------------------------------------------------------
    KnownDesktopLocationsAdmin()
        {
        // use the standard Windows temporary directory
        wchar_t tempPathW[MAX_PATH];
        ::GetTempPathW(_countof(tempPathW), tempPathW);
        m_tempDirectory.SetName(tempPathW);
        m_tempDirectory.AppendSeparator();

        // the application directory is where the executable is located
        wchar_t moduleFileName[MAX_PATH];
        ::GetModuleFileNameW(NULL, moduleFileName, _countof(moduleFileName));
        BeFileName moduleDirectory(BeFileName::DevAndDir, moduleFileName);
        m_executableDirectory = moduleDirectory;
        m_executableDirectory.AppendSeparator();

        m_assetsDirectory = m_executableDirectory;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
    };

struct BimImporter : DgnPlatformLib::Host
{
private:
    BeFileName m_inputFileName;
    BeFileName m_outputPath;
    BeFileName m_loggingConfigFileName;

    void PrintError(WCharCP fmt, ...);
    void _PrintMessage(WCharCP fmt, ...);
    BentleyStatus GetLogConfigurationFilename(BeFileName& configFile, WCharCP argv0);
    void InitLogging(WCharCP argv0);
    WString GetArgValueW(WCharCP arg);

    virtual void                        _SupplyProductName(Utf8StringR name) override { name.assign("BimImporter"); }
    virtual IKnownLocationsAdmin&       _SupplyIKnownLocationsAdmin() override { return *new KnownDesktopLocationsAdmin(); };
    virtual L10N::SqlangFiles _SupplySqlangFiles() override;

    int _PrintUsage(WCharCP programName);
    BentleyStatus _ParseCommandLine(WStringR errmsg, int argc, WCharCP argv[]);
    BentleyStatus _Initialize(int argc, WCharCP argv[]);
    static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("BimImporter"); }
    DgnDbPtr CreateNewBim();

public:
    //! wmain should call this to run the job.
    int Run(int argc, WCharCP argv[]);

};

END_BIM_FROM_DGNDB_NAMESPACE