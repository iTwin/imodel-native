#pragma once

/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"      


struct DoorTools
    {
    static  Dgn::PhysicalElementPtr CreateDoor    (BuildingPhysical::BuildingPhysicalModelR model, int doorNumber );
    static  Dgn::PhysicalElementPtr CreateWindow1 (BuildingPhysical::BuildingPhysicalModelR model, int windowNumber);

    };

