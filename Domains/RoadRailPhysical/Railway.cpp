/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/Corridor.h>
#include <RoadRailPhysical/Railway.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailwayPtr Railway::Create(TransportationSystemCR transportationSystem, RoadRailAlignment::AlignmentCP mainAlignment)
    {
    if (!transportationSystem.GetElementId().IsValid() || !transportationSystem.GetSubModelId().IsValid())
        return nullptr;

    PhysicalElement::CreateParams createParams(transportationSystem.GetDgnDb(), transportationSystem.GetSubModelId(), 
                                               QueryClassId(transportationSystem.GetDgnDb()), RoadRailCategory::GetRailway(transportationSystem.GetDgnDb()));

    RailwayPtr ptr = new Railway(*Create(transportationSystem.GetDgnDb(), createParams));
    ptr->SetMainAlignment(mainAlignment);
    return ptr;
    }