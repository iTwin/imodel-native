/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadSegment.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(ElevatedRoadIntersectionHandler)
HANDLER_DEFINE_MEMBERS(ElevatedRoadIntersectionSegmentHandler)
HANDLER_DEFINE_MEMBERS(ElevatedRoadSegmentHandler)
HANDLER_DEFINE_MEMBERS(RoadIntersectionHandler)
HANDLER_DEFINE_MEMBERS(RoadIntersectionSegmentHandler)
HANDLER_DEFINE_MEMBERS(RoadSegmentHandler)
HANDLER_DEFINE_MEMBERS(RoadTransitionSegmentHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadSegment::RoadSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadSegmentPtr RoadSegment::Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new RoadSegment(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadTransitionSegment::RoadTransitionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadTransitionSegmentPtr RoadTransitionSegment::Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new RoadTransitionSegment(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ElevatedRoadSegment::ElevatedRoadSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ElevatedRoadSegmentPtr ElevatedRoadSegment::Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new ElevatedRoadSegment(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadIntersectionSegment::RoadIntersectionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadIntersectionSegmentPtr RoadIntersectionSegment::Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new RoadIntersectionSegment(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ElevatedRoadIntersectionSegment::ElevatedRoadIntersectionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ElevatedRoadIntersectionSegmentPtr ElevatedRoadIntersectionSegment::Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new ElevatedRoadIntersectionSegment(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadIntersectionPtr RoadIntersection::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailPhysicalDomain::QueryRoadCategoryId(model.GetDgnDb()));

    return new RoadIntersection(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ElevatedRoadIntersectionPtr ElevatedRoadIntersection::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailPhysicalDomain::QueryRoadCategoryId(model.GetDgnDb()));

    return new ElevatedRoadIntersection(createParams);
    }