/*--------------------------------------------------------------------------------------+
|
|     $Source: Segment.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(RoadSegmentHandler)
HANDLER_DEFINE_MEMBERS(RoadSegmentElementHandler)
HANDLER_DEFINE_MEMBERS(RoadSegmentOnBridgeHandler)
HANDLER_DEFINE_MEMBERS(SegmentElementHandler)
HANDLER_DEFINE_MEMBERS(TransitionSegmentHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SegmentElement::SegmentElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SegmentElement::SegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params), ILinearlyLocatedSingleFromTo(fromDistanceAlong, toDistanceAlong)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadSegmentElement::RoadSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    }

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
TransitionSegment::TransitionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TransitionSegmentPtr TransitionSegment::Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new TransitionSegment(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadSegmentOnBridge::RoadSegmentOnBridge(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadSegmentOnBridgePtr RoadSegmentOnBridge::Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new RoadSegmentOnBridge(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }