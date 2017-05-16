#pragma once

/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchPhysCreater/ArchPhysCreater/DoorTools.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"      


struct DoorTools
    {
    static  ArchitecturalPhysical::DoorPtr CreateDoor( BuildingPhysical::BuildingPhysicalModelR model, int doorNumber );
    static  ArchitecturalPhysical::WindowPtr CreateWindow1 (BuildingPhysical::BuildingPhysicalModelR model, int windowNumber);

    };

