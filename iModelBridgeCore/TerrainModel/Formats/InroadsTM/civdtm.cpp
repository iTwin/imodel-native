//---------------------------------------------------------------------------+
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//---------------------------------------------------------------------------+
// civdtm.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"

struct DllMain
    {
    DllMain ()
        {
        aecDTM_projectSurfaceInitialize ();
        }
    };

DllMain dllMain;