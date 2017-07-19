/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/iModelBridgeAssignMain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, WCharCP argv[])
    {
    return iModelBridgeFwk::AssignMain(argc, argv);
    }

#ifdef __unix__
UNIX_MAIN_CALLS_WMAIN(WCharCP*)
#endif
