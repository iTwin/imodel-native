/*--------------------------------------------------------------------------------------+
|
|     $Source: TravelwayStructureSegment.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/TravelwayStructureSegment.h>
#include <RoadRailPhysical/Pathway.h>

HANDLER_DEFINE_MEMBERS(RegularTravelwayStructureSegmentHandler)
HANDLER_DEFINE_MEMBERS(TravelwayStructureSegmentElementHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayStructureSegmentElement::TravelwayStructureSegmentElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayStructureSegmentElement::TravelwayStructureSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params), ILinearlyLocatedSingleFromTo(fromDistanceAlong, toDistanceAlong)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RegularTravelwayStructureSegment::RegularTravelwayStructureSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong, TravelwayStructureDefinitionCR travelwayStructureDef):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    SetTravelwayStructureDefinition(travelwayStructureDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RegularTravelwayStructureSegmentPtr RegularTravelwayStructureSegment::Create(PathwayElementCR pathway, double fromDistanceAlong, double toDistanceAlong, 
    TravelwayStructureDefinitionCR travelwayDef, AlignmentCP alignment)
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

    auto retVal = new RegularTravelwayStructureSegment(params, fromDistanceAlong, toDistanceAlong, travelwayDef);
    retVal->_SetLinearElement(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RegularTravelwayStructureSegment::SetTravelwayStructureDefinition(TravelwayStructureDefinitionCR travelwayDef)
    { 
    SetPropertyValue("Definition", travelwayDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_RegularSegmentRefersToTravelwayStructureDefinition));
    }