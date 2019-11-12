#pragma once

/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"      


struct DoorTools
    {
    static  Dgn::PhysicalElementPtr CreateDoor    (BuildingPhysical::BuildingPhysicalModelR model, int doorNumber );
    static  Dgn::PhysicalElementPtr CreateWindow1 (BuildingPhysical::BuildingPhysicalModelR model, int windowNumber);

    };

