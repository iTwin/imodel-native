/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TravelwayStructureSegment.h $
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
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureSegmentElement : Dgn::PhysicalElement, LinearReferencing::ILinearlyLocatedElement, LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureSegmentElement, Dgn::PhysicalElement);
    friend struct TravelwayStructureSegmentElementHandler;

protected:
    //! @private
    explicit TravelwayStructureSegmentElement(CreateParams const& params);

    //! @private
    explicit TravelwayStructureSegmentElement(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayStructureSegmentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwayStructureSegmentElement)
}; // TravelwayStructureSegmentElement

//=======================================================================================
//! Base class for physical Road segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularTravelwayStructureSegment : TravelwayStructureSegmentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RegularTravelwayStructureSegment, TravelwayStructureSegmentElement);
    friend struct RegularTravelwayStructureSegmentHandler;
    
protected:
    //! @private
    explicit RegularTravelwayStructureSegment(CreateParams const& params) : T_Super(params) {}
            
    //! @private
    explicit RegularTravelwayStructureSegment(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong, TravelwayStructureDefinitionCR travelwayStructureDef);
            
public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RegularTravelwayStructureSegment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RegularTravelwayStructureSegment)

    Dgn::DgnElementId GetTravelwayStructureDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>("Definition"); }
    ROADRAILPHYSICAL_EXPORT void SetTravelwayStructureDefinition(TravelwayStructureDefinitionCR travelwayDef);

    ROADRAILPHYSICAL_EXPORT static RegularTravelwayStructureSegmentPtr Create(PathwayElementCR pathway, double fromDistanceAlong, double toDistanceAlong, 
        TravelwayStructureDefinitionCR travelwayDef, RoadRailAlignment::AlignmentCP alignment = nullptr);
}; // RegularTravelwayStructureSegment


//=================================================================================
//! ElementHandler for TravelwayStructureSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureSegmentElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureSegmentElement, TravelwayStructureSegmentElement, TravelwayStructureSegmentElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayStructureSegmentElementHandler

//=================================================================================
//! ElementHandler for Regular StructureSegment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularTravelwayStructureSegmentHandler : TravelwayStructureSegmentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RegularTravelwayStructureSegment, RegularTravelwayStructureSegment, RegularTravelwayStructureSegmentHandler, TravelwayStructureSegmentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RegularTravelwayStructureSegmentHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE