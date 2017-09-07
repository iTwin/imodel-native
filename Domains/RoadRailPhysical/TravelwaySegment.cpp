/*--------------------------------------------------------------------------------------+
|
|     $Source: TravelwaySegment.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/TravelwaySegment.h>
#include <RoadRailPhysical/Pathway.h>

HANDLER_DEFINE_MEMBERS(AlignmentIntersectionElementHandler)
HANDLER_DEFINE_MEMBERS(RegularTravelwaySegmentHandler)
HANDLER_DEFINE_MEMBERS(TravelwayIntersectionSegmentElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwaySegmentElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwayTransitionHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySegmentElement::TravelwaySegmentElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySegmentElement::TravelwaySegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params), ILinearlyLocatedSingleFromTo(fromDistanceAlong, toDistanceAlong)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RegularTravelwaySegment::RegularTravelwaySegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong, TravelwayDefinitionElementCR travelwayDef):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    SetTravelwayDefinition(travelwayDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RegularTravelwaySegmentPtr RegularTravelwaySegment::Create(PathwayElementCR pathway, double fromDistanceAlong, double toDistanceAlong, TravelwayDefinitionElementCR travelwayDef)
    {
    if (!pathway.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = pathway.GetAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()), pathway.GetCategoryId());
    params.SetParentId(pathway.GetElementId(),
        DgnClassId(pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayAssemblesElements)));

    auto retVal = new RegularTravelwaySegment(params, fromDistanceAlong, toDistanceAlong, travelwayDef);
    retVal->_SetLinearElement(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RegularTravelwaySegment::SetTravelwayDefinition(TravelwayDefinitionElementCR travelwayDef)
    { 
    SetPropertyValue("TravelwayDefinition", travelwayDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_RegularSegmentRefersToTravelwayDefinition));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayTransition::TravelwayTransition(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayTransitionPtr TravelwayTransition::Create(PathwayElementCR pathway, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!pathway.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = pathway.GetAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()), pathway.GetCategoryId());
    params.SetParentId(pathway.GetElementId(),
        DgnClassId(pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayAssemblesElements)));

    auto retVal = new TravelwayTransition(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElement(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayIntersectionSegmentElement::TravelwayIntersectionSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong) :
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    }