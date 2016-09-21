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
HANDLER_DEFINE_MEMBERS(SegmentElementHandler)
HANDLER_DEFINE_MEMBERS(TransitionSegmentHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SegmentElement::SegmentElement(CreateParams const& params, DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition):
    T_Super(params)
    {
    _AddLinearlyReferencedLocation(*LinearlyReferencedFromToLocation::Create(fromPosition, toPosition));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadSegmentElement::RoadSegmentElement(CreateParams const& params, DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition):
    T_Super(params, fromPosition, toPosition)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadSegment::RoadSegment(CreateParams const& params, DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition):
    T_Super(params, fromPosition, toPosition)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadSegmentPtr RoadSegment::Create(RoadRangeCR roadRange, DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition)
    {
    if (!roadRange.GetElementId().IsValid() || !roadRange.GetAlignmentId().IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new RoadSegment(params, fromPosition, toPosition);
    retVal->_SetLinearElementId(roadRange.GetAlignmentId());
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TransitionSegment::TransitionSegment(CreateParams const& params, DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition):
    T_Super(params, fromPosition, toPosition)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TransitionSegmentPtr TransitionSegment::Create(RoadRangeCR roadRange, DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition)
    {
    if (!roadRange.GetElementId().IsValid() || !roadRange.GetAlignmentId().IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new TransitionSegment(params, fromPosition, toPosition);
    retVal->_SetLinearElementId(roadRange.GetAlignmentId());
    return retVal;
    }