/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/iModelBridgeFwkMain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, WCharCP argv[])
    {
    Dgn::iModelBridgeFwk app;

    if (BentleyApi::BSISUCCESS != app.ParseCommandLine(argc, argv))
        return 1;

    return app.Run(argc, argv);
    }

#ifdef __unix__
UNIX_MAIN_CALLS_WMAIN(WCharCP*)
#endif
