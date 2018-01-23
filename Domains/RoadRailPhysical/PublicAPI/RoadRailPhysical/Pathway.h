/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/Pathway.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "RoadRailCategory.h"
#include "TravelwaySegment.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class for Road and Rail range of physical segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayElement : Dgn::PhysicalElement, LinearReferencing::ILinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, Dgn::PhysicalElement);
    friend struct PathwayElementHandler;

private:
    mutable RefCountedPtr<LinearReferencing::ICascadeLinearLocationChangesAlgorithm> m_cascadeAlgorithmPtr;

protected:
    //! @private
    explicit PathwayElement(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(PathwayElement)

    Dgn::DgnElementId GetMainAlignmentId() const { return GetPropertyValueId<Dgn::DgnElementId>("MainAlignment"); }
    RoadRailAlignment::AlignmentCPtr GetMainAlignment() const { return RoadRailAlignment::Alignment::Get(GetDgnDb(), GetMainAlignmentId()); }
    ROADRAILPHYSICAL_EXPORT Dgn::DgnDbStatus SetMainAlignment(RoadRailAlignment::AlignmentCP alignment);

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(PathwayElementCR pathway, Dgn::DgnElementCR representedBy);
}; // PathwayElement

//=======================================================================================
//! Physical range over a Road that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Roadway : PathwayElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Roadway, PathwayElement);
    friend struct RoadwayHandler;

protected:
    //! @private
    explicit Roadway(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Roadway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(Roadway)
    ROADRAILPHYSICAL_EXPORT static RoadwayPtr Create(Dgn::PhysicalModelR model);
}; // Roadway

//=======================================================================================
//! Physical range over a Rail that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Railway : PathwayElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Railway, PathwayElement);
    friend struct RailwayHandler;

protected:
    //! @private
    explicit Railway(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit Railway(CreateParams const& params, RoadRailAlignment::AlignmentCR alignment);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Railway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(Railway)
    ROADRAILPHYSICAL_EXPORT static RailwayPtr Create(Dgn::PhysicalModelR model);
}; // Railway


//=================================================================================
//! ElementHandler for SegmentRange Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwayElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, PathwayElement, PathwayElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // SegmentRangeElementHandler

//=================================================================================
//! ElementHandler for Roadway Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayHandler : PathwayElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_Roadway, Roadway, RoadwayHandler, PathwayElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadwayHandler

//=================================================================================
//! ElementHandler for RailRange Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RailwayHandler : PathwayElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_Railway, Railway, RailwayHandler, PathwayElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RailRangeHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE