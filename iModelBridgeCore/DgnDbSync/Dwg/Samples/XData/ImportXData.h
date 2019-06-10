/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifdef DWGTOOLKIT_RealDwg
// RealDWG uses MFC which requires afx headers to be included first!
#ifndef UNICODE
    #define     UNICODE
#endif
#ifndef _AFXDLL
    #define     _AFXDLL
    #include    <afxwin.h>
#endif
#endif  // DWGTOOLKIT_RealDwg

#include <Dwg/DwgImporter.h>
#include <Dwg/DwgHelper.h>
#include <Dwg/DwgBridge.h>
#include <Dwg/ProtocalExtensions.h>

// _ImportGroup will demonstrate how to create a GenericGroup element.
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_DWG

// Use DwgImporter logging to trace import and to report issues
#define LOG                     (*NativeLogging::LoggingManager::GetLogger(L"DwgImporter"))

BEGIN_DWG_NAMESPACE

//=======================================================================================
// This is the core class of the sample code: it lets DwgImporter convert DWG data by 
// default, parses the XDATA found on each and every entity, and converts the data as
// Adhoc properties of the DgnDb element.
//
// Group is implemented via _ImportGroup and _UpdateGroup. See method comments for details.
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
    // Find all elements from the SyncInfo for the input DWG object
    DgnElementIdSet FindElementsMappedFrom (DwgDbObjectIdCR objectId);

public:
    // Constructor
    ImportXData (DwgImporter::Options& options, SampleOptions const& sample) : T_Super(options), m_sampleOptions(sample), m_dumpCount(0) { }
    //
    // Override _BeginImport when we may optionally dump Xrecord's in dictionaries
    virtual void    _BeginImport () override;
    //
    // Override _ImportEntity to convert both entity and its xdata
    //
    // This method is called on entities that are seen in both the modelspace and paperpsaces, and that are not
    // implemented by DwgProtocolExtension.  A protocol extended entity is routed to _ImportEntityByProtocolExtension.
    // The mehod is not called on entities in a block definition as there is no such a concept of cell exists in DgnDb.
    // It is called in both initial import as well as incremental job run.  The caller handles the change detection,
    // element insertion and element updates.  It is important that you do not insert DgnDb element in this
    // call.  Fill ElementImportResults with new elements you have created to represent the input DWG entity,
    // but do not insert them - leave the insertion to the caller.
    //
    virtual BentleyStatus   _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs) override;
    //
    // Override _ImportEntityByProtocolExtension for entities not routed through _ImportEntity
    //
    // This method supplements above _ImportEntity, i.e. if an entity implemented by DwgProtocolExtension, this method
    // will called first.  The protocol extension may optionally fallback to call _ImportEntity.
    //
    virtual BentleyStatus   _ImportEntityByProtocolExtension (ElementImportResults& results, ElementImportInputs& inputs, DwgProtocolExtension& entityExt) override;
    //
    // Override _ImportGroup to create & insert a DgnDb group element from a DWG group.
    //
    // This method is overridden to serve as a sample code that demonstrates how a GenericGroup 
    // can be created from a DWG group.  The super method has the same implementation.  The base class
    // of DwgImporter works this way: after _BeginImport() is called, the models are discovered from
    // the root DWG file's block section.  Each xRef attachment is converted as a DgnModel.  The importer 
    // moves on to _ImportEntitySection, followed by _ImportLayouts for modelspapce and
    // paperspaces respectively, to import their entities and add converted elements in their respective models.
    // After the importer has finished creating models and elements, it then calls _ImportGroups(void) to start 
    // processing DWG groups, by iterating through the root DWG file and xRef files.  For each DWG file, it 
    // calls _ImportGroups and passes in DwgDbDatabase of that file.  That method in turn calls _ImportGroup
    // for each group object found in the group dictionary section of that DWG file.
    //
    virtual DgnElementPtr   _ImportGroup (DwgDbGroupCR dwgGroup) override;
    //
    // Override _UpdateGroup to update existing DgnDb group element from a DWG group
    //
    // This method goes parallel with _ImportGroup. A difference is that it only gets called during
    // an updating job run.  During such an incremental process, _ImportGroups consults DwgSyncInfo
    // about the incoming DWG groups.  When a previously imported group is found, _OnUpdateGroup 
    // is called.  If a change of the DWG group has been detected, _UpdateGroup gets called and both
    // the DgnDb and DWG groups are passed in.  This method is expected to update the contents of 
    // the existing DgnDb group element from the DWG group object.  If no change has been detected 
    // by _OnUpdateGroup, _ImportGroup gets called, and is expected to create a new element.
    //
    virtual BentleyStatus   _UpdateGroup (DgnElementR dgnGroup, DwgDbGroupCR dwgGroup) override;
    //
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
    // Override this method to set CONNECTION Client infomation to track product usage etc
    void           _SetClientInfo () override;
    // Override this method to parse command arguments specific for this sample app:
    iModelBridge::CmdLineArgStatus _ParseCommandLineArg (int iArg, int argc, WCharCP argv[]) override;
    void           _PrintUsage () override;
    // override this method to provide DwgBridge our sample importer:
    DwgImporter*    _CreateDwgImporter () override;
    };  // ImportXDataSample

END_DWG_NAMESPACE
