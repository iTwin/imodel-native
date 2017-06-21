/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/iModelBridgeFwkMain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    Dgn::iModelBridgeFwk app;

    if (BentleyApi::BSISUCCESS != app.ParseCommandLine(argc, argv))
        return 1;

    return app.Run(argc, argv);
    }
