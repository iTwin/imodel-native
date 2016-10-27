/*--------------------------------------------------------------------------------------+
|
|     $Source: Segment.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(IntersectionElementHandler)
HANDLER_DEFINE_MEMBERS(IntersectionSegmentElementHandler)
HANDLER_DEFINE_MEMBERS(RegularSegmentElementHandler)
HANDLER_DEFINE_MEMBERS(SegmentElementHandler)
HANDLER_DEFINE_MEMBERS(TransitionSegmentElementHandler)


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
RegularSegmentElement::RegularSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TransitionSegmentElement::TransitionSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectionSegmentElement::IntersectionSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params, fromDistanceAlong, toDistanceAlong)
    {
    }
