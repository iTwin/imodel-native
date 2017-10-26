/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TravelwaySideSegment.h $
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
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideSegmentElement : Dgn::PhysicalElement, LinearReferencing::ILinearlyLocatedElement, LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideSegmentElement, Dgn::PhysicalElement);
    friend struct TravelwaySideSegmentElementHandler;

protected:
    //! @private
    explicit TravelwaySideSegmentElement(CreateParams const& params);

    //! @private
    explicit TravelwaySideSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySideSegmentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwaySideSegmentElement)
}; // TravelwaySideSegmentElement

//=======================================================================================
//! Base class for physical Road segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularTravelwaySideSegment : TravelwaySideSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RegularTravelwaySideSegment, TravelwaySideSegmentElement);
    friend struct RegularTravelwaySideSegmentHandler;
    
protected:
    //! @private
    explicit RegularTravelwaySideSegment(CreateParams const& params) : T_Super(params) {}
            
    //! @private
    explicit RegularTravelwaySideSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong, TravelwaySideDefinitionCR travelwaySideDef);
            
public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RegularTravelwaySideSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RegularTravelwaySideSegment)

    Dgn::DgnElementId GetTravelwaySideDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>("Definition"); }
    ROADRAILPHYSICAL_EXPORT void SetTravelwaySideDefinition(TravelwaySideDefinitionCR travelwayDef);

    ROADRAILPHYSICAL_EXPORT static RegularTravelwaySideSegmentPtr Create(PathwayElementCR pathway, double fromDistanceAlong, double toDistanceAlong, 
        TravelwaySideDefinitionCR travelwayDef, RoadRailAlignment::AlignmentCP alignment = nullptr);
}; // RegularTravelwaySideSegment


//=================================================================================
//! ElementHandler for TravelwaySideSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideSegmentElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideSegmentElement, TravelwaySideSegmentElement, TravelwaySideSegmentElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // TravelwaySideSegmentElementHandler

//=================================================================================
//! ElementHandler for Regular SideSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularTravelwaySideSegmentHandler : TravelwaySideSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RegularTravelwaySideSegment, RegularTravelwaySideSegment, RegularTravelwaySideSegmentHandler, TravelwaySideSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RegularTravelwaySideSegmentHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE