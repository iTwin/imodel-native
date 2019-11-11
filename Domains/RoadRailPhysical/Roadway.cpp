/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/Corridor.h>
#include <RoadRailPhysical/Roadway.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayPtr Roadway::Create(RoadRailPhysical::TransportationSystemCR transportationSystem, AlignmentCP mainAlignment)
    {
    if (!transportationSystem.GetElementId().IsValid() || !transportationSystem.GetSubModelId().IsValid())
        return nullptr;

    PhysicalElement::CreateParams createParams(transportationSystem.GetDgnDb(), transportationSystem.GetSubModelId(), 
                                               QueryClassId(transportationSystem.GetDgnDb()), RoadRailCategory::GetRoadway(transportationSystem.GetDgnDb()));

    RoadwayPtr ptr = new Roadway(*Create(transportationSystem.GetDgnDb(), createParams));
    ptr->SetMainAlignment(mainAlignment);
    return ptr;
    }