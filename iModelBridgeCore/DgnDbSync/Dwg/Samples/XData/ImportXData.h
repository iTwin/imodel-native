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
// @bsiclass
//=======================================================================================
struct ImportXData : public DwgImporter
    {
    DEFINE_T_SUPER (DwgImporter)

private:
    // Method converts XDATA to Adhoc properties
    BentleyStatus   ConvertXData (DgnElementR element, DwgDbEntityCR entity);

public:
    // Constructor
    ImportXData (Options& options) : T_Super(options) { }
    // Override _ImportEntity to convert both entity and its xdata
    virtual BentleyStatus   _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs) override;
    };  // ImportXData

//=======================================================================================
// A sample iModelBridge class that runs ImportXData bridge
// @bsiclass
//=======================================================================================
struct ImportXDataSample : public DwgBridge
    {
    DEFINE_T_SUPER (DwgBridge)
    BentleyStatus   ImportDwgFile (int argc, WCharCP argv[]);
    };  // ImportXDataSample

END_DGNDBSYNC_DWG_NAMESPACE
