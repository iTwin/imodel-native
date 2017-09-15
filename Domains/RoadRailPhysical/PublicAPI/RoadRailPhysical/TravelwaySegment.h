/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TravelwaySegment.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "TypicalSection.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class for physical Road and Rail segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySegmentElement : Dgn::SpatialLocationElement, LinearReferencing::ILinearlyLocatedElement, LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySegmentElement, Dgn::SpatialLocationElement);
    friend struct TravelwaySegmentElementHandler;

protected:
    //! @private
    explicit TravelwaySegmentElement(CreateParams const& params);

    //! @private
    explicit TravelwaySegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySegmentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwaySegmentElement)
}; // TravelwaySegmentElement

//=======================================================================================
//! Base class for physical Road segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularTravelwaySegment : TravelwaySegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RegularTravelwaySegment, TravelwaySegmentElement);
    friend struct RegularTravelwaySegmentHandler;
    
protected:
    //! @private
    explicit RegularTravelwaySegment(CreateParams const& params) : T_Super(params) {}
            
    //! @private
    explicit RegularTravelwaySegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong, TravelwayDefinitionElementCR travelwayDef);
            
public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RegularTravelwaySegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RegularTravelwaySegment)

    Dgn::DgnElementId GetTravelwayDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>("Definition"); }
    ROADRAILPHYSICAL_EXPORT void SetTravelwayDefinition(TravelwayDefinitionElementCR travelwayDef);

    ROADRAILPHYSICAL_EXPORT static RegularTravelwaySegmentPtr Create(PathwayElementCR pathway, double fromDistanceAlong, double toDistanceAlong, TravelwayDefinitionElementCR travelwayDef);
}; // RegularTravelwaySegment

//=======================================================================================
//! Physical segment of a road, with a variable cross-section.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayTransition : TravelwaySegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayTransition, TravelwaySegmentElement);
    friend struct TravelwayTransitionHandler;
    
protected:
    //! @private
    explicit TravelwayTransition(CreateParams const& params) : T_Super(params) {}
            
    explicit TravelwayTransition(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);
            
public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayTransition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwayTransition)

    ROADRAILPHYSICAL_EXPORT static TravelwayTransitionPtr Create(PathwayElementCR roadway, double fromDistanceAlong, double toDistanceAlong);
}; // TravelwayTransition

//=======================================================================================
//! Physical segment of a road, with a variable cross-section.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayIntersectionSegmentElement : TravelwaySegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayIntersectionSegmentElement, TravelwaySegmentElement);
    friend struct TravelwayIntersectionSegmentElementHandler;
    
protected:
    //! @private
    explicit TravelwayIntersectionSegmentElement(CreateParams const& params) : T_Super(params) {}
            
    explicit TravelwayIntersectionSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);
            
public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayIntersectionSegmentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwayIntersectionSegmentElement)
}; // TravelwayIntersectionSegmentElement

//=======================================================================================
//! Base class for a physical location representing an intersection between 
//! two or more roads.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentIntersectionElement : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_AlignmentIntersectionElement, Dgn::PhysicalElement);
    friend struct AlignmentIntersectionElementHandler;

protected:
    //! @private
    explicit AlignmentIntersectionElement(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(AlignmentIntersectionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(AlignmentIntersectionElement)
}; // AlignmentIntersectionElement


//=================================================================================
//! ElementHandler for TravelwaySegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySegmentElementHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySegmentElement, TravelwaySegmentElement, TravelwaySegmentElementHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILPHYSICAL_EXPORT)
}; // TravelwaySegmentElementHandler

//=================================================================================
//! ElementHandler for base RoadSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularTravelwaySegmentHandler : TravelwaySegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RegularTravelwaySegment, RegularTravelwaySegment, RegularTravelwaySegmentHandler, TravelwaySegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RegularTravelwaySegmentHandler

//=================================================================================
//! ElementHandler for TransitionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayTransitionHandler : TravelwaySegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayTransition, TravelwayTransition, TravelwayTransitionHandler, TravelwaySegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayTransitionHandler

//=================================================================================
//! ElementHandler for TravelwayIntersectionSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayIntersectionSegmentElementHandler : TravelwaySegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayIntersectionSegmentElement, TravelwayIntersectionSegmentElement, TravelwayIntersectionSegmentElementHandler, TravelwaySegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayIntersectionSegmentElementHandler

//=================================================================================
//! ElementHandler for IntersectionElement Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentIntersectionElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_AlignmentIntersectionElement, AlignmentIntersectionElement, AlignmentIntersectionElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // AlignmentIntersectionSegmentHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE