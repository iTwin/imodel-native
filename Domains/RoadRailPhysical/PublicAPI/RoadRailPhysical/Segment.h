/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/Segment.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class for physical Road and Rail segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SegmentElement : Dgn::PhysicalElement, LinearReferencing::ILinearlyLocatedElement, ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_SegmentElement, Dgn::PhysicalElement);
    friend struct SegmentElementHandler;

protected:
    //! @private
    explicit SegmentElement(CreateParams const& params);

    //! @private
    explicit SegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(SegmentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(SegmentElement)
}; // SegmentElement

//=======================================================================================
//! Base class for physical Road segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularSegmentElement : SegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RegularSegmentElement, SegmentElement);
    friend struct RegularSegmentElementHandler;

protected:
    //! @private
    explicit RegularSegmentElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit RegularSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RegularSegmentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(RegularSegmentElement)
}; // RegularSegmentElement

//=======================================================================================
//! Physical segment of a road, with a variable cross-section.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TransitionSegmentElement : SegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TransitionSegmentElement, SegmentElement);
    friend struct TransitionSegmentElementHandler;

protected:
    //! @private
    explicit TransitionSegmentElement(CreateParams const& params) : T_Super(params) {}

    explicit TransitionSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TransitionSegmentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TransitionSegmentElement)
}; // TransitionSegmentElement

//=======================================================================================
//! Base class for physical segments of a road participating in an intersection.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionSegmentElement : SegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_IntersectionSegmentElement, SegmentElement);
    friend struct IntersectionSegmentElementHandler;

protected:
    //! @private
    explicit IntersectionSegmentElement(CreateParams const& params) : T_Super(params) {}

    explicit IntersectionSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(IntersectionSegmentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(IntersectionSegmentElement)
}; // IntersectionSegmentElement

//=======================================================================================
//! Base class for a physical location representing an intersection between 
//! two or more roads.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionElement : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_IntersectionElement, Dgn::PhysicalElement);
    friend struct IntersectionElementHandler;

protected:
    //! @private
    explicit IntersectionElement(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(IntersectionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(IntersectionElement)
}; // IntersectionElement


//=================================================================================
//! ElementHandler for Segment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SegmentElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_SegmentElement, SegmentElement, SegmentElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // SegmentElementHandler

//=================================================================================
//! ElementHandler for base RoadSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularSegmentElementHandler : SegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RegularSegmentElement, RegularSegmentElement, RegularSegmentElementHandler, SegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RegularSegmentElementHandler

//=================================================================================
//! ElementHandler for TransitionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TransitionSegmentElementHandler : SegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TransitionSegmentElement, TransitionSegmentElement, TransitionSegmentElementHandler, SegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TransitionSegmentElementHandler

//=================================================================================
//! ElementHandler for IntersectionSegmentElement Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionSegmentElementHandler : SegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_IntersectionSegmentElement, IntersectionSegmentElement, IntersectionSegmentElementHandler, SegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // IntersectionSegmentElementHandler

//=================================================================================
//! ElementHandler for IntersectionElement Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_IntersectionElement, IntersectionElement, IntersectionElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // IntersectionSegmentHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE