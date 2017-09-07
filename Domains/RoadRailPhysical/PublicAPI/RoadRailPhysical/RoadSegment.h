/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadSegment.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "TravelwaySegment.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Physical segments of a road participating in an intersection.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadIntersectionLegElement : TravelwayIntersectionSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadIntersectionLegElement, TravelwayIntersectionSegmentElement);
    friend struct RoadIntersectionLegElementHandler;

protected:
    //! @private
    explicit RoadIntersectionLegElement(CreateParams const& params) : T_Super(params) {}

    explicit RoadIntersectionLegElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadIntersectionLegElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(RoadIntersectionLegElement)
}; // RoadIntersectionLegElement

//=======================================================================================
//! Physical location representing an intersection between two or more roads.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadIntersectionElement : AlignmentIntersectionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadIntersectionElement, AlignmentIntersectionElement);
    friend struct RoadIntersectionElementHandler;

protected:
    //! @private
    explicit RoadIntersectionElement(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadIntersectionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(RoadIntersectionElement)
}; // RoadIntersectionElement

//=================================================================================
//! ElementHandler for RoadIntersectionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadIntersectionLegElementHandler : TravelwayIntersectionSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadIntersectionLegElement, RoadIntersectionLegElement, RoadIntersectionLegElementHandler, TravelwayIntersectionSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadIntersectionSegmentHandler

//=================================================================================
//! ElementHandler for RoadIntersection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadIntersectionElementHandler : AlignmentIntersectionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadIntersectionElement, RoadIntersectionElement, RoadIntersectionElementHandler, AlignmentIntersectionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadIntersectionElementHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE