/*--------------------------------------------------------------------------------------+
|
|     $Source: TravelwaySideSegment.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/TravelwaySideSegment.h>
#include <RoadRailPhysical/Pathway.h>

HANDLER_DEFINE_MEMBERS(RegularTravelwaySideSegmentHandler)
HANDLER_DEFINE_MEMBERS(TravelwaySideSegmentElementHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideSegmentElement::TravelwaySideSegmentElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideSegmentElement::TravelwaySideSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params), ILinearlyLocatedSingleFromTo(fromDistanceAlong, toDistanceAlong)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RegularTravelwaySideSegment::RegularTravelwaySideSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong, TravelwaySideDefinitionCR travelwaySideDef):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    SetTravelwaySideDefinition(travelwaySideDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RegularTravelwaySideSegmentPtr RegularTravelwaySideSegment::Create(PathwayElementCR pathway, double fromDistanceAlong, double toDistanceAlong, 
    TravelwaySideDefinitionCR travelwayDef, AlignmentCP alignment)
    {
    if (!pathway.GetElementId().IsValid())
        return nullptr;

    DgnElementId alignmentId;

    if (alignment)
        {
        if (pathway.GetMainAlignmentId() != alignment->GetParentId())
            return nullptr;

        alignmentId = alignment->GetElementId();
        }
    else
        {
        alignmentId = pathway.GetMainAlignmentId();
        if (!alignmentId.IsValid())
            return nullptr;
        }

    CreateParams params(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()), pathway.GetCategoryId());
    params.SetParentId(pathway.GetElementId(),
        DgnClassId(pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayAssemblesElements)));

    auto retVal = new RegularTravelwaySideSegment(params, fromDistanceAlong, toDistanceAlong, travelwayDef);
    retVal->_SetLinearElement(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RegularTravelwaySideSegment::SetTravelwaySideDefinition(TravelwaySideDefinitionCR travelwayDef)
    { 
    SetPropertyValue("Definition", travelwayDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_RegularSegmentRefersToTravelwaySideDefinition));
    }