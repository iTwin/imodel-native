#pragma once

/*--------------------------------------------------------------------------------------+
|
|     $Source: BimCreater/Bimcreater/DoorTools.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"      


struct DoorTools
    {
    static  Dgn::PhysicalElementPtr CreateDoor    (BuildingPhysical::BuildingPhysicalModelR model, int doorNumber );
    static  Dgn::PhysicalElementPtr CreateWindow1 (BuildingPhysical::BuildingPhysicalModelR model, int windowNumber);

    };

