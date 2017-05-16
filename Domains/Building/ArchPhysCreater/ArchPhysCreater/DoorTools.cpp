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
ArchitecturalPhysical::DoorPtr DoorTools::CreateDoor(BuildingPhysical::BuildingPhysicalModelR model, int doorNumber)
    {

    ArchitecturalPhysical::DoorPtr door = ArchitecturalPhysical::Door::Create(model);
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

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Finish", "Polished"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FireRating", 2.0))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameDepth", 0.075))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameExteriorThickness", 0.005))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameInteriorThickness", 0.005))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameFinish", "Polished"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameHeadDetail", "HD-001"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameJambDetail", "JD-001"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameMaterial", "Steel"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameNotes", "Notes"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameSillDetail", "SD-001"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameThickness", 0.005))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameType", "Wrapping" ))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameWidth", 0.150 ))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Function", "Interior"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("GlazingMaterial", "Glass"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Height", 2.500))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Width", 0.900))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Keynote", "Keynote"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Material", "Wood"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Phase", "Design"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("PanelThickness", 0.035))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("PanelWidth", 0.900))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("RoughOpeningHeight", 2.500))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("RoughOpeningWidth", 0.900))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("TypeID", "Standard Door"))
        return nullptr;

    return door;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
ArchitecturalPhysical::WindowPtr DoorTools::CreateWindow1(BuildingPhysical::BuildingPhysicalModelR model, int windowNumber)
    {

    ArchitecturalPhysical::WindowPtr window = ArchitecturalPhysical::Window::Create(model);
    if (!window.IsValid())
        return nullptr;


    char doorID[100];
    sprintf_s(doorID, "W%.3d", windowNumber);

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, model, doorID);
    if (Dgn::DgnDbStatus::Success != window->SetCode(code))
        return nullptr;

/*    if (Dgn::DgnDbStatus::Success != window->SetPropertyValue("OverallWidth", 900.0))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("OverallHeight", 2500.0))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Description", "This is a door"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Finish", "Polished"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FireRating", 2.0))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameDepth", 0.075))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameExteriorThickness", 0.005))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameInteriorThickness", 0.005))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameFinish", "Polished"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameHeadDetail", "HD-001"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameJambDetail", "JD-001"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameMaterial", "Steel"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameNotes", "Notes"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameSillDetail", "SD-001"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameThickness", 0.005))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameType", "Wrapping"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("FrameWidth", 0.150))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Function", "Interior"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("GlazingMaterial", "Glass"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Height", 2.500))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Width", 0.900))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Keynote", "Keynote"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Material", "Wood"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("Phase", "Design"))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("PanelThickness", 0.035))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("PanelWidth", 0.900))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("RoughOpeningHeight", 2.500))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("RoughOpeningWidth", 0.900))
        return nullptr;

    if (Dgn::DgnDbStatus::Success != door->SetPropertyValue("TypeID", "Standard Door"))
        return nullptr;                                */

    return window;
    }
