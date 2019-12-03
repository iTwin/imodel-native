/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dBridge.h"

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int wmain (int argc, wchar_t const* argv[])
    {
    BentleyApi::C3D::C3dBridge  bridgeApp;
    auto status = bridgeApp.RunAsStandaloneExe (argc, argv);
    return (int)status;
    }
