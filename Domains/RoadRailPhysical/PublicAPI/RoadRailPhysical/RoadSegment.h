/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadSegment.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Physical segment of a road, with a constant cross-section.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadSegment : RegularSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadSegment, RegularSegmentElement);
    friend struct RoadSegmentHandler;

protected:
    //! @private
    explicit RoadSegment(CreateParams const& params) : T_Super(params) {}

    explicit RoadSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong, RoadCrossSectionCR crossSection);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadSegment)

    Dgn::DgnElementId GetRoadCrossSectionId() const { return GetPropertyValueId<Dgn::DgnElementId>("RoadCrossSection"); }
    void SetRoadCrossSection(RoadCrossSectionCR crossSection) { SetPropertyValue("RoadCrossSection", crossSection.GetElementId()); }

    ROADRAILPHYSICAL_EXPORT static RoadSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong, RoadCrossSectionCR crossSection);
}; // RoadSegment

//=======================================================================================
//! Physical segment of a road supported by a bridge.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedRoadSegment : RegularSegmentElement, BridgePhysical::IElevatedPhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ElevatedRoadSegment, RegularSegmentElement);
    friend struct ElevatedRoadSegmentHandler;

protected:
    //! @private
    explicit ElevatedRoadSegment(CreateParams const& params) : T_Super(params) {}

    explicit ElevatedRoadSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::PhysicalElementCR _IElevatedPhysicalElementToPhysicalElement() const override { return *this; }
    virtual LinearReferencing::ILinearlyLocatedElementCR _IElevatedPhysicalElementToLinearlyLocated() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ElevatedRoadSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(ElevatedRoadSegment)

    ROADRAILPHYSICAL_EXPORT static ElevatedRoadSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // ElevatedRoadSegment

//=======================================================================================
//! Physical segment of a road, with a variable cross-section.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadTransitionSegment : TransitionSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadTransitionSegment, TransitionSegmentElement);
    friend struct RoadTransitionSegmentHandler;

protected:
    //! @private
    explicit RoadTransitionSegment(CreateParams const& params) : T_Super(params) {}

    explicit RoadTransitionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadTransitionSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadTransitionSegment)

    ROADRAILPHYSICAL_EXPORT static RoadTransitionSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // RoadTransitionSegmentElement

//=======================================================================================
//! Physical segments of a road participating in an intersection.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadIntersectionSegment : IntersectionSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadIntersectionSegment, IntersectionSegmentElement);
    friend struct RoadIntersectionSegmentHandler;

protected:
    //! @private
    explicit RoadIntersectionSegment(CreateParams const& params) : T_Super(params) {}

    explicit RoadIntersectionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadIntersectionSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadIntersectionSegment)

    ROADRAILPHYSICAL_EXPORT static RoadIntersectionSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // RoadIntersectionSegment

//=======================================================================================
//! Physical segment of a road participating in an intersection, supported by a bridge.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedRoadIntersectionSegment : IntersectionSegmentElement, BridgePhysical::IElevatedPhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ElevatedRoadIntersectionSegment, IntersectionSegmentElement);
    friend struct ElevatedRoadIntersectionSegmentHandler;

protected:
    //! @private
    explicit ElevatedRoadIntersectionSegment(CreateParams const& params) : T_Super(params) {}

    explicit ElevatedRoadIntersectionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::PhysicalElementCR _IElevatedPhysicalElementToPhysicalElement() const override { return *this; }
    virtual LinearReferencing::ILinearlyLocatedElementCR _IElevatedPhysicalElementToLinearlyLocated() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ElevatedRoadIntersectionSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(ElevatedRoadIntersectionSegment)

    ROADRAILPHYSICAL_EXPORT static ElevatedRoadIntersectionSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // ElevatedRoadIntersectionSegment

//=======================================================================================
//! Physical location representing an intersection between two or more roads.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadIntersection : IntersectionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadIntersection, IntersectionElement);
    friend struct RoadIntersectionHandler;

protected:
    //! @private
    explicit RoadIntersection(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadIntersection)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadIntersection)

    ROADRAILPHYSICAL_EXPORT static RoadIntersectionPtr Create(Dgn::PhysicalModelR model);
}; // RoadIntersection

//=======================================================================================
//! Physical location representing an intersection between two or more roads, 
//! supported by a bridge.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedRoadIntersection : IntersectionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ElevatedRoadIntersection, IntersectionElement);
    friend struct ElevatedRoadIntersectionHandler;

protected:
    //! @private
    explicit ElevatedRoadIntersection(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ElevatedRoadIntersection)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(ElevatedRoadIntersection)

    ROADRAILPHYSICAL_EXPORT static ElevatedRoadIntersectionPtr Create(Dgn::PhysicalModelR model);
}; // ElevatedRoadIntersection


//=================================================================================
//! ElementHandler for RoadSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadSegmentHandler : RegularSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadSegment, RoadSegment, RoadSegmentHandler, RegularSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadSegmentHandler

//=================================================================================
//! ElementHandler for ElevatedRoadSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedRoadSegmentHandler : RegularSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ElevatedRoadSegment, ElevatedRoadSegment, ElevatedRoadSegmentHandler, RegularSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ElevatedRoadSegmentHandler

//=================================================================================
//! ElementHandler for RoadTransitionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadTransitionSegmentHandler : TransitionSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadTransitionSegment, RoadTransitionSegment, RoadTransitionSegmentHandler, TransitionSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadTransitionSegmentHandler

//=================================================================================
//! ElementHandler for RoadIntersectionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadIntersectionSegmentHandler : IntersectionSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadIntersectionSegment, RoadIntersectionSegment, RoadIntersectionSegmentHandler, IntersectionSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadIntersectionSegmentHandler

//=================================================================================
//! ElementHandler for RoadIntersectionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedRoadIntersectionSegmentHandler : IntersectionSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ElevatedRoadIntersectionSegment, ElevatedRoadIntersectionSegment, ElevatedRoadIntersectionSegmentHandler, IntersectionSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ElevatedRoadIntersectionSegmentHandler

//=================================================================================
//! ElementHandler for RoadIntersection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadIntersectionHandler : IntersectionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadIntersection, RoadIntersection, RoadIntersectionHandler, IntersectionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadIntersectionHandler

//=================================================================================
//! ElementHandler for ElevatedIntersection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedRoadIntersectionHandler : IntersectionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ElevatedRoadIntersection, ElevatedRoadIntersection, ElevatedRoadIntersectionHandler, IntersectionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ElevatedRoadIntersectionHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE