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
SegmentElement::SegmentElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SegmentElement::SegmentElement(CreateParams const& params, DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition):
    T_Super(params)
    {
    m_unpersistedFromToLocationPtr = LinearlyReferencedFromToLocation::Create(fromPosition, toPosition);
    _AddLinearlyReferencedLocation(*m_unpersistedFromToLocationPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationCP SegmentElement::GetFromToLocation() const
    {
    return const_cast<SegmentElementP>(this)->GetFromToLocationP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationP SegmentElement::GetFromToLocationP()
    {
    if (!GetElementId().IsValid())
        return m_unpersistedFromToLocationPtr.get();

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationP = DgnElement::MultiAspect::GetP<LinearlyReferencedFromToLocation>(
        *this, *LinearlyReferencedFromToLocation::QueryClass(GetDgnDb()), ECInstanceId(m_fromToLocationAspectId));
    BeAssert(locationP);

    return locationP;
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
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new RoadSegment(params, fromPosition, toPosition);
    retVal->_SetLinearElementId(alignmentId);
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
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()), roadRange.GetCategoryId());
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new TransitionSegment(params, fromPosition, toPosition);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }