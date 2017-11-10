/*--------------------------------------------------------------------------------------+
|
|     $Source: Pathway.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/Pathway.h>

HANDLER_DEFINE_MEMBERS(PathwayElementHandler)
HANDLER_DEFINE_MEMBERS(RailwayHandler)
HANDLER_DEFINE_MEMBERS(RoadwayHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PathwayElement::SetMainAlignment(AlignmentCP alignment)
    {
    return SetPropertyValue("MainAlignment", (alignment) ? alignment->GetElementId() : DgnElementId(), Alignment::QueryClassId(GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayPtr Roadway::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetRoad(model.GetDgnDb()));

    return new Roadway(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailwayPtr Railway::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetTrack(model.GetDgnDb()));

    return new Railway(createParams);
    }