/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchPhysCreater/ArchPhysCreater/DoorTools.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
ArchitecturalPhysical::DoorPtr DoorTools::CreateDoor(BuildingPhysical::BuildingPhysicalModelR model)
    {

    ArchitecturalPhysical::DoorPtr door = ArchitecturalPhysical::Door::Create(model);
    if (!door.IsValid())
       return nullptr;

    return door;
    }
