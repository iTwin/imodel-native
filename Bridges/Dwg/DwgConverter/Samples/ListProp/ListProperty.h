/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

#include <Dwg/DwgBridge.h>
#include <Dwg/DwgHelper.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_DWG

// Use DwgImporter logging to trace import and to report issues
#define LOG                     (*NativeLogging::LoggingManager::GetLogger(L"DwgImporter"))


BEGIN_DWG_NAMESPACE

//=======================================================================================
// This sample code shows two main features:
//  1. Apply ACAD's LIST command on non-AcDb entities and collect property strings. Parse out the strings
//      and create Adhoc properties for the imported elements. For this feature, only _ImportEntity 
//      needs overridden.
//  2. Walk through non-ACAD dictionaries, detect changes, create or update a subject converted from a
//      dictionary entry.  For this feature, we opt for overriding _MakeDefinitionChanges but it can
//      be done at a different step.
//
// General call sequenace from DwgBridge:
//  OpenDwgFile
//  InitializeJob(import) or FindJob(update)
//  _MakeSchemaChanegs  (iModelBridgeFwk holds schema locks for the bridge to make changes)
//  _BeginImport
//  _MakeDefinitionChanges (iModelBridgeFwk holds dictionary locks)
//      _ImportTextStyleSection
//      _ImportLineTypeSection
//      _ImportMaterialSection
//      _ImportLayerSection
//  _ImportSpaces
//  _ImportDwgModels    (Model dicovery phase, no data filling)
//  _ImportModelspaceViewports
//  _ImportEntitySection    (Data filling from modelspace, including xRef's attached in modelspace)
//  _ImportLayouts  (Data filling for from paperspaces, including xRef's attached paperspace)
//  _ImportGroups
//  _FinishImport
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
    // Methods converts selected dictionaries as subject elements
    SubjectPtr      ConvertDictionary (SubjectCR parentSubject, DwgDbObjectCR sourceObject, Utf8StringCR name);
    BentleyStatus   DrillInDictionary (SubjectCR parentSubject, DwgDbDictionaryP dictionary, Utf8StringCR name, Utf8StringCR prefix);

public:
    // Constructor
    ListProperty (DwgImporter::Options& options) : T_Super(options) { }
    // Override _ImportEntity to LIST entity properties
    virtual BentleyStatus   _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs) override;

    // Override import job name
    Utf8String  _ComputeImportJobName (DwgDbBlockTableRecordCR modelspaceBlock) const override;

    // Override this method to make changes in dictionary model shared by all
    BentleyStatus _MakeDefinitionChanges (SubjectCR jobSubject) override;
    };  // ListProperty

//=======================================================================================
// A sample that runs DwgBridge
// @bsiclass
//=======================================================================================
struct ListPropertySample : public DwgBridge
    {
    DEFINE_T_SUPER (DwgBridge)
    // override this method to provide DwgBridge our sample importer:
    DwgImporter*    _CreateDwgImporter () override;
    };  // ListPropertySample

END_DWG_NAMESPACE
