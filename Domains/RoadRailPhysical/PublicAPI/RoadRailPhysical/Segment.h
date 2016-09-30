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
struct EXPORT_VTABLE_ATTRIBUTE SegmentElement : Dgn::PhysicalElement, LinearReferencing::ILinearlyLocatedElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_SegmentElement, Dgn::PhysicalElement);
    friend struct SegmentElementHandler;

private:
    mutable LinearReferencing::LinearlyReferencedLocationId m_fromToLocationAspectId;
    LinearReferencing::LinearlyReferencedFromToLocationPtr m_unpersistedFromToLocationPtr;

protected:
    //! @private
    explicit SegmentElement(CreateParams const& params);

    //! @private
    explicit SegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(SegmentElement)

    ROADRAILPHYSICAL_EXPORT double GetFromDistanceAlong() const;
    ROADRAILPHYSICAL_EXPORT void SetFromDistanceAlong(double newFrom);

    ROADRAILPHYSICAL_EXPORT double GetToDistanceAlong() const;
    ROADRAILPHYSICAL_EXPORT void SetToDistanceAlong(double newFrom);
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
struct EXPORT_VTABLE_ATTRIBUTE RoadSegmentOnBridge : RoadSegmentElement, BridgePhysical::IPhysicalElementOnBridge
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadSegmentOnBridge, RoadSegmentElement);
    friend struct RoadSegmentOnBridgeHandler;

protected:
    //! @private
    explicit RoadSegmentOnBridge(CreateParams const& params) : T_Super(params) {}

    explicit RoadSegmentOnBridge(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::PhysicalElementCR _IPhysicalElementOnBridgeToPhysicalElement() const override { return *this; }
    virtual LinearReferencing::ILinearlyLocatedElementCR _IPhysicalElementOnBridgeToLinearlyLocated() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadSegmentOnBridge)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadSegmentOnBridge)

    ROADRAILPHYSICAL_EXPORT static RoadSegmentOnBridgePtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // RoadSegmentOnBridge


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
//! ElementHandler for RoadSegmentOnBridge Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadSegmentOnBridgeHandler : RoadSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadSegmentOnBridge, RoadSegmentOnBridge, RoadSegmentOnBridgeHandler, RoadSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadSegmentOnBridgeHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE