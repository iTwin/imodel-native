/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Samples/ListProp/ListProperty.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifdef DWGTOOLKIT_RealDwg
// RealDWG uses MFC which requires afx headers to be included first!
#ifndef UNITCODE
    #define     UNICODE
#endif
#ifndef _AFXDLL
    #define     _AFXDLL
    #include    <afxwin.h>
#endif
#endif  // DWGTOOLKIT_RealDwg

#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <BeSQLite/L10N.h>
#include <Logging/BentleyLogging.h>
#include <DgnDbSync/Dwg/DwgImporter.h>
#include <DgnDbSync/Dwg/DwgHelper.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_DGNDBSYNC_DWG

// Use DwgImporter logging to trace import and to report issues
#define LOG                     (*NativeLogging::LoggingManager::GetLogger(L"DwgImporter"))
#define UNCOMPRESSED_DGNDB_EXT  L"ibim"


BEGIN_DGNDBSYNC_DWG_NAMESPACE

//=======================================================================================
//
// @bsiclass
//=======================================================================================
struct ListProperty : public DwgImporter
    {
    DEFINE_T_SUPER (DwgImporter)

private:
    // Method converts XDATA to Adhoc properties
    bool            ValidatePropertyFormat (WStringR newString, WStringR currString, WStringR previousString) const;
    BentleyStatus   ConvertMessage (T_Utf8StringVectorR utf8List, WStringR currMessage, WStringR prevMessage) const;
    BentleyStatus   ConvertMessageCollection (T_Utf8StringVectorR utf8List, T_WStringVectorR wcharList) const;
    BentleyStatus   ListEntityProperties (DgnElementR element, DwgDbEntityCR entity);

public:
    // Constructor
    ListProperty (Options& options) : T_Super(options) { }
    // Override _ImportEntity to LIST entity properties
    virtual BentleyStatus   _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs) override;
    };  // ListProperty

//=======================================================================================
// A simple progress meter class for DWG importing
// @bsiclass
//=======================================================================================
struct PrintfProgressMeter : BentleyApi::Dgn::DgnProgressMeter
    {
    DEFINE_T_SUPER(DgnProgressMeter)

protected:
    Utf8String      m_stepName;
    Utf8String      m_taskName;
    Utf8String      m_lastMessage;
    double          m_timeOfLastUpdate;
    double          m_timeOfLastSpinnerUpdate;
    uint32_t        m_fileCount;
    uint32_t        m_spinCount;

    void UpdateDisplay0 (Utf8StringCR msg);
    void UpdateDisplay ();
    void PopDescription ();
    bool HasDescription () const;
    Utf8String FmtMessage () const;
    void ForceNextUpdateToDisplay () { m_timeOfLastUpdate = m_timeOfLastSpinnerUpdate = 0; }
    virtual void _Hide () override;
    virtual Abort _ShowProgress () override;
    virtual void _SetCurrentStepName (Utf8CP stepName) override;
    virtual void _SetCurrentTaskName (Utf8CP taskName) override;

public:
    PrintfProgressMeter() : BentleyApi::Dgn::DgnProgressMeter(), m_timeOfLastUpdate(0), m_timeOfLastSpinnerUpdate(0), m_fileCount(0), m_spinCount(0) {}
    void SetFileCount (uint32_t n) { m_fileCount = n; }
    void DecrementFileCount() { if (m_fileCount) m_fileCount--; }
    };  // PrintfProgressMeter

//=======================================================================================
// A sample class that hosts DgnPlaform and instantiates DwgImporter.
// @bsiclass
//=======================================================================================
struct ListPropertySample : DgnPlatformLib::Host
    {
protected:
    virtual void                        _SupplyProductName (Utf8StringR name) override { name.assign("DwgImporter"); }
    virtual IKnownLocationsAdmin&       _SupplyIKnownLocationsAdmin() override { return *new WindowsKnownLocationsAdmin(); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;

public:
    PrintfProgressMeter     m_progressMeter;
    BeFileName              m_inputFileName;
    BeFileName              m_outputName;
    DwgImporter::Options    m_options;

    BentleyStatus   Initialize (WCharCP programName, WCharCP dwgFileName, WCharCP dgndbFolderName);
    void            InitLogging (WCharCP programName);
    void            GetImportConfiguration(BeFileNameR instanceFilePath, BeFileNameCR configurationPath, WCharCP argv0);
    BentleyStatus   GetEnv (BeFileName& fn, WCharCP envname);
    BentleyStatus   GetLogConfigurationFilename (BeFileName& configFile, WCharCP argv0);
    BentleyStatus   ImportDwgFile ();
    };  // ListPropertySample

END_DGNDBSYNC_DGNV8_NAMESPACE
