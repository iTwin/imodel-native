/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::PhysicalElementPtr DoorTools::CreateDoor(BuildingPhysical::BuildingPhysicalModelR model, int doorNumber)
    {

	Dgn::PhysicalElementPtr door =  BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement (BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_CLASS_Door, model);
    if (!door.IsValid())
       return nullptr;

    char doorID[100];
    sprintf_s( doorID, "D%.3d", doorNumber);

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode (BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, model, doorID );
    if (Dgn::DgnDbStatus::Success != door->SetCode(code) )
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("OverallWidth", 900.0))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("OverallHeight", 2500.0))          
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Description", "This is a door"))
        return nullptr;

    return door;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::PhysicalElementPtr DoorTools::CreateWindow1(BuildingPhysical::BuildingPhysicalModelR model, int windowNumber)
    {

	Dgn::PhysicalElementPtr window = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_CLASS_Window, model);
    if (!window.IsValid())
        return nullptr;


    char windowID[100];
    sprintf_s(windowID, "W%.3d", windowNumber);

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, model, windowID);
    if (Dgn::DgnDbStatus::Success != window->SetCode(code))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != window->SetPropertyValue("OverallWidth", 900.0))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != window->SetPropertyValue("OverallHeight", 2500.0))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != window->SetPropertyValue("Description", "This is a window"))
        return nullptr;


    return window;
    }
