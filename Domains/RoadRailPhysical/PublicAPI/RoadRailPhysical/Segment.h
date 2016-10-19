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
}; // SegmentElement

//=======================================================================================
//! Base class for physical Road segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadSegmentElement : SegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadSegmentElement, SegmentElement);
    friend struct RoadSegmentElementHandler;

protected:
    //! @private
    explicit RoadSegmentElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit RoadSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadSegmentElement)
}; // RoadSegmentElement

//=======================================================================================
//! Physical segment of a road, with a constant cross-section.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadSegment : RoadSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadSegment, RoadSegmentElement);
    friend struct RoadSegmentHandler;

protected:
    //! @private
    explicit RoadSegment(CreateParams const& params) : T_Super(params) {}

    explicit RoadSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadSegment)

    ROADRAILPHYSICAL_EXPORT static RoadSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // RoadSegment

//=======================================================================================
//! Physical segment of a road, with a variable cross-section.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TransitionSegment : RoadSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TransitionSegment, RoadSegmentElement);
    friend struct TransitionSegmentHandler;

protected:
    //! @private
    explicit TransitionSegment(CreateParams const& params) : T_Super(params) {}

    explicit TransitionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TransitionSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TransitionSegment)

    ROADRAILPHYSICAL_EXPORT static TransitionSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // TransitionSegment

//=======================================================================================
//! Physical segment of a road supported by a bridge.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedRoadSegment : RoadSegmentElement, BridgePhysical::IElevatedPhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ElevatedRoadSegment, RoadSegmentElement);
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
//! Base class for physical segments of a road participating in an intersection.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionSegmentElement : RoadSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_IntersectionSegmentElement, RoadSegmentElement);
    friend struct IntersectionSegmentElementHandler;

protected:
    //! @private
    explicit IntersectionSegmentElement(CreateParams const& params) : T_Super(params) {}

    explicit IntersectionSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(IntersectionSegmentElement)
}; // IntersectionSegmentElement

//=======================================================================================
//! Physical segments of a road participating in an intersection.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionSegment : IntersectionSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_IntersectionSegment, IntersectionSegmentElement);
    friend struct IntersectionSegmentHandler;

protected:
    //! @private
    explicit IntersectionSegment(CreateParams const& params) : T_Super(params) {}

    explicit IntersectionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(IntersectionSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(IntersectionSegment)

    ROADRAILPHYSICAL_EXPORT static IntersectionSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // IntersectionSegment

//=======================================================================================
//! Physical segment of a road participating in an intersection, supported by a bridge.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedIntersectionSegment : IntersectionSegmentElement, BridgePhysical::IElevatedPhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ElevatedIntersectionSegment, IntersectionSegmentElement);
    friend struct ElevatedIntersectionSegmentHandler;

protected:
    //! @private
    explicit ElevatedIntersectionSegment(CreateParams const& params) : T_Super(params) {}

    explicit ElevatedIntersectionSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::PhysicalElementCR _IElevatedPhysicalElementToPhysicalElement() const override { return *this; }
    virtual LinearReferencing::ILinearlyLocatedElementCR _IElevatedPhysicalElementToLinearlyLocated() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ElevatedIntersectionSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(ElevatedIntersectionSegment)

    ROADRAILPHYSICAL_EXPORT static ElevatedIntersectionSegmentPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // ElevatedIntersectionSegment

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
}; // IntersectionElement

//=======================================================================================
//! Physical location representing an intersection between two or more roads.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Intersection : IntersectionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Intersection, IntersectionElement);
    friend struct IntersectionHandler;

protected:
    //! @private
    explicit Intersection(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Intersection)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(Intersection)

    ROADRAILPHYSICAL_EXPORT static IntersectionPtr Create(Dgn::PhysicalModelR model);
}; // Intersection

//=======================================================================================
//! Physical location representing an intersection between two or more roads, 
//! supported by a bridge.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedIntersection : IntersectionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ElevatedIntersection, IntersectionElement);
    friend struct ElevatedIntersectionHandler;

protected:
    //! @private
    explicit ElevatedIntersection(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ElevatedIntersection)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(ElevatedIntersection)

    ROADRAILPHYSICAL_EXPORT static ElevatedIntersectionPtr Create(Dgn::PhysicalModelR model);
}; // ElevatedIntersection


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
struct EXPORT_VTABLE_ATTRIBUTE RoadSegmentElementHandler : SegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadSegmentElement, RoadSegmentElement, RoadSegmentElementHandler, SegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadSegmentElementHandler

//=================================================================================
//! ElementHandler for RoadSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadSegmentHandler : RoadSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadSegment, RoadSegment, RoadSegmentHandler, RoadSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadSegmentHandler

//=================================================================================
//! ElementHandler for TransitionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TransitionSegmentHandler : RoadSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TransitionSegment, TransitionSegment, TransitionSegmentHandler, RoadSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TransitionSegmentHandler

//=================================================================================
//! ElementHandler for ElevatedRoadSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedRoadSegmentHandler : RoadSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ElevatedRoadSegment, ElevatedRoadSegment, ElevatedRoadSegmentHandler, RoadSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ElevatedRoadSegmentHandler

//=================================================================================
//! ElementHandler for IntersectionSegmentElement Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionSegmentElementHandler : RoadSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_IntersectionSegmentElement, IntersectionSegmentElement, IntersectionSegmentElementHandler, RoadSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // IntersectionSegmentElementHandler

//=================================================================================
//! ElementHandler for IntersectionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionSegmentHandler : IntersectionSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_IntersectionSegment, IntersectionSegment, IntersectionSegmentHandler, IntersectionSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // IntersectionSegmentHandler

//=================================================================================
//! ElementHandler for ElevatedIntersectionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedIntersectionSegmentHandler : IntersectionSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ElevatedIntersectionSegment, ElevatedIntersectionSegment, ElevatedIntersectionSegmentHandler, IntersectionSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ElevatedIntersectionSegmentHandler

//=================================================================================
//! ElementHandler for IntersectionElement Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_IntersectionElement, IntersectionElement, IntersectionElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // IntersectionSegmentHandler

//=================================================================================
//! ElementHandler for Intersection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionHandler : IntersectionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_Intersection, Intersection, IntersectionHandler, IntersectionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // IntersectionHandler

//=================================================================================
//! ElementHandler for ElevatedIntersection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevatedIntersectionHandler : IntersectionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ElevatedIntersection, ElevatedIntersection, ElevatedIntersectionHandler, IntersectionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ElevatedIntersectionHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE