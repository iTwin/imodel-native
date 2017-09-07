/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadSegment.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadSegment.h>

HANDLER_DEFINE_MEMBERS(RoadIntersectionElementHandler)
HANDLER_DEFINE_MEMBERS(RoadIntersectionLegElementHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadIntersectionLegElement::RoadIntersectionLegElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
/*RoadIntersectionSegmentPtr RoadIntersectionLegElement::Create(RoadwayCR roadway, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadway.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadway.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadway.GetDgnDb(), roadway.GetModelId(), QueryClassId(roadway.GetDgnDb()), roadway.GetCategoryId());
    params.SetParentId(roadway.GetElementId(),
        DgnClassId(roadway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayAssemblesElements)));

    auto retVal = new RoadIntersectionSegment(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElement(alignmentId, Alignment::QueryClassId(roadway.GetDgnDb()));
    return retVal;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
/*RoadIntersectionPtr RoadIntersection::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetRoad(model.GetDgnDb()));

    return new RoadIntersection(createParams);
    }
*/