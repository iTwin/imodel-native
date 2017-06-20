/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Apps/DwgImportApp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if defined(_MSC_VER) && defined(DWGTOOLKIT_RealDwg)
// RealDWG uses MFC which requires afx headers to be included first!
#ifndef UNITCODE
    #define     UNICODE
#endif
#ifndef _AFXDLL
    #define     _AFXDLL
    #include    <afxwin.h>
#endif
#endif

#include <Bentley/BeFileListIterator.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <DgnPlatform/DgnIModel.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <BeSQLite/L10N.h>
#include <Logging/bentleylogging.h>
#include <DgnDbSync/Dwg/DwgImporter.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_DGNDBSYNC_DWG
USING_NAMESPACE_BENTLEY_SQLITE

#define UNCOMPRESSED_DGNDB_EXT      L"ibim"
#define LOG                         (*NativeLogging::LoggingManager::GetLogger(L"DwgImporter"))


BEGIN_DGNDBSYNC_DWG_NAMESPACE

//=======================================================================================
// A quick and dirty progress meter.
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
struct PrintfProgressMeter : BentleyApi::Dgn::DgnProgressMeter
{
    DEFINE_T_SUPER(DgnProgressMeter)

protected:
    Utf8String m_stepName;
    Utf8String m_taskName;
    Utf8String m_lastMessage;
    double m_timeOfLastUpdate;
    double m_timeOfLastSpinnerUpdate;
    uint32_t m_fileCount;
    uint32_t m_spinCount;

    void UpdateDisplay0 (Utf8StringCR msg);
    void UpdateDisplay();
    void PopDescription();
    bool HasDescription() const;
    Utf8String FmtMessage() const;
    void ForceNextUpdateToDisplay() {m_timeOfLastUpdate=m_timeOfLastSpinnerUpdate=0;}
    virtual void _Hide() override;
    virtual Abort _ShowProgress() override;
    virtual void _SetCurrentStepName (Utf8CP stepName) override;
    virtual void _SetCurrentTaskName (Utf8CP taskName) override;

public:
    PrintfProgressMeter() : BentleyApi::Dgn::DgnProgressMeter(), m_timeOfLastUpdate(0), m_timeOfLastSpinnerUpdate(0), m_fileCount(0), m_spinCount(0) {}
    void SetFileCount (uint32_t n) {m_fileCount=n;}
    void DecrementFileCount() {if (m_fileCount) m_fileCount--;}
};

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgImportApp : DgnPlatformLib::Host
{
protected:
    virtual void                        _SupplyProductName (Utf8StringR name) override {name.assign("DwgImporter");}
    virtual IKnownLocationsAdmin&       _SupplyIKnownLocationsAdmin() override {return *new WindowsKnownLocationsAdmin();}
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;

public:
    enum class ParseStatus
        {
        Success,
        Error,
        NotRecognized
        };

    PrintfProgressMeter     m_meter;
    BeFileName              m_inputFileName;
    BeFileName              m_outputName;
    DwgImporter::Options    m_options;
    bool                    m_preventAssertDialogs;
    bool                    m_tryUpdate;
    bool                    m_wasUpdateEmpty;
    bool                    m_shouldCompress;
    uint32_t                m_compressChunkSize;

    DwgImportApp() {m_tryUpdate=false; m_wasUpdateEmpty=false; m_preventAssertDialogs=false; m_compressChunkSize=0; m_shouldCompress=true;}

    void            PrintMessage (WCharCP fmt, ...);
    void            PrintError (WCharCP fmt, ...);
    int             PrintUsage (WCharCP programName);
    WString         GetCommonCommandLineOptions ();
    ParseStatus     ParseCommonCommandLineArgument (WCharP argv[], int& iArg);
    int             ParseCommandLine (int argc, WCharP argv[]);

    static void     AssertHandler (WCharCP message, WCharCP file, uint32_t line, BeAssertFunctions::AssertType atype);
    void            GetImportConfiguration(BeFileNameR instanceFilePath, BeFileNameCR configurationPath, WCharCP argv0);
    BentleyStatus   GetEnv (BeFileName& fn, WCharCP envname);
    BentleyStatus   MustBeFile (BeFileName& fn);
    BentleyStatus   GetLogConfigurationFilename (BeFileName& configFile, WCharCP argv0);
    void            InitLogging (WCharCP argv0);
    WString         GetArgValueW (WCharCP arg);
    Utf8String      GetArgValue (WCharCP arg);
    void            OnFailedConversion ();
    void            CreateIModel ();
    BentleyStatus   ExtractFromIModel (BeFileName& outFile, BeFileNameCR imodelFile);
    BentleyStatus   Initialize (int argc, WCharP argv[]);
    BentleyStatus   Process ();
    void            PostProcess ();
};

END_DGNDBSYNC_DGNV8_NAMESPACE
