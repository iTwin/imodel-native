/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Samples/XData/ImportXData.h $
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

#include <DgnDbSync/Dwg/DwgImporter.h>
#include <DgnDbSync/Dwg/DwgHelper.h>
#include <DgnDbSync/Dwg/DwgBridge.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_DGNDBSYNC_DWG

// Use DwgImporter logging to trace import and to report issues
#define LOG                     (*NativeLogging::LoggingManager::GetLogger(L"DwgImporter"))

BEGIN_DGNDBSYNC_DWG_NAMESPACE

//=======================================================================================
// This is the core class of the sample code: it lets DwgImporter convert DWG data by 
// default, parses the XDATA found on each and every entity, and converts the data as
// Adhoc properties of the DgnDb element.
//
// It also optionally dumps dictionary xRecords into the importer's issues file. The option
// is part of this sample bridge and it is passed on to the sample importer. When the option
// is on, at the beginning of the import process we open the main named dictionary of the input
// DWG file and recursively search for and dump xRecord entries.
//
// @bsiclass
//=======================================================================================
struct ImportXData : public DwgImporter
    {
    DEFINE_T_SUPER (DwgImporter)

    struct SampleOptions
        {
    private:
        Utf8String  m_regappName;
        bool        m_dumpXrecords;

    public:
        SampleOptions () : m_dumpXrecords(false) {}
        // Desired RegApp name to dump entity XData
        Utf8StringCR    GetRegAppName () const { return m_regappName; }
        void    SetRegAppName (Utf8StringCR newName) { m_regappName = newName; }
        // Should also dump xRecords in the dictionary section?
        bool    ShouldDumpXrecords () const { return m_dumpXrecords; }
        void    SetShouldDumpXrecords (bool shouldDump) { m_dumpXrecords = shouldDump; }
        };  // SampleOptions

private:
    SampleOptions   m_sampleOptions;
    size_t      m_dumpCount;
    Utf8String  m_xRecordName;

    // Method converts XDATA to Adhoc properties
    BentleyStatus   ConvertXData (DgnElementR element, DwgDbEntityCR entity);
    // Print an xRecord
    BentleyStatus   DumpXrecord (DwgDbXrecordCR xRecord);
    // Walk through dictionary entries and print out xRecords
    BentleyStatus   DumpDictionaryXrecords (DwgDbDictionaryCR dictionary);

public:
    // Constructor
    ImportXData (DwgImporter::Options const& options, SampleOptions const& sample) : T_Super(options), m_sampleOptions(sample), m_dumpCount(0) { }
    // override _BeginImport when we may optionally dump Xrecord's in dictionaries
    virtual void    _BeginImport () override;
    // Override _ImportEntity to convert both entity and its xdata
    virtual BentleyStatus   _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs) override;
    };  // ImportXData

//=======================================================================================
// A sample iModelBridge class that runs ImportXData and can also be registered as a 
// separate iModelBridge by supplying itself an independent instance and affinity to the
// framework(see the .cpp file for implementation).
// @bsiclass
//=======================================================================================
struct ImportXDataSample : public DwgBridge
    {
    DEFINE_T_SUPER (DwgBridge)

private:
    // The effective instance of a DwgImporter:
    std::unique_ptr<ImportXData>    m_importer;
    // Options for this sample:
    ImportXData::SampleOptions      m_sampleOptions;

public:
    // The constructor
    ImportXDataSample () : T_Super() {}
    // Override this method to parse command arguments specific for this sample app:
    iModelBridge::CmdLineArgStatus _ParseCommandLineArg (int iArg, int argc, WCharCP argv[]) override;
    void           _PrintUsage () override;
    // override this method to provide DwgBridge our sample importer:
    DwgImporter*    _CreateDwgImporter () override;
    };  // ImportXDataSample

END_DGNDBSYNC_DWG_NAMESPACE
