/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/StructPhysCreaterApp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
int wmain(int argc, WCharP argv[])
    {
    StructPhysCreator app;
    Dgn::DgnPlatformLib::Initialize(app, false);

    app.ParseCommandLine(argc, argv);

    return app.DoCreate();
    }
