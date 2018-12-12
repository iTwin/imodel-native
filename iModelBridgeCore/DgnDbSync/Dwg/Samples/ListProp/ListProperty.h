/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Samples/ListProp/ListProperty.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    ListProperty (DwgImporter::Options& options) : T_Super(options) { }
    // Override _ImportEntity to LIST entity properties
    virtual BentleyStatus   _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs) override;
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
