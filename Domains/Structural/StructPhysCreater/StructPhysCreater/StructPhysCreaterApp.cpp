/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
