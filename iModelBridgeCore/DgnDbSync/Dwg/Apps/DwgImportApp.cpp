/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Apps/DwgImportApp.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Dwg/DwgBridge.h>

struct DwgBridgeApp : DwgBridge
{
DEFINE_T_SUPER (DwgBridge)

public:
//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus   RunCmdline (int argc, WCharCP argv[])
    {
    return T_Super::RunAsStandaloneExe(argc, argv);
    }
};  // DwgBridgeApp

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
int wmain (int argc, wchar_t const* argv[])
    {
    // Create a DWG bridge and run it as a standalone console program
    DwgBridgeApp    bridgeApp;
    return  (int)bridgeApp.RunAsStandaloneExe(argc, argv);
    }

