/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

BEGIN_BENTLEY_RAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Physical range over a Rail that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Railway : RoadRailPhysical::PathwayElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(RoadRailPhysical::PathwayElement, Dgn::PhysicalElement)

protected:
    //! @private
    explicit Railway(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit Railway(Dgn::PhysicalElementR element) : T_Super(element) {}

    //! @private
    virtual RailwayCP _ToRailway() const override { return this; }

public:
    DECLARE_RAILPHYSICAL_QUERYCLASS_METHODS(Railway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(Railway, Dgn::PhysicalElement)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RailwayPtr Create(RoadRailPhysical::TransportationSystemCR transportationSystem, RoadRailAlignment::AlignmentCP mainAlignment);
}; // Railway

END_BENTLEY_RAILPHYSICAL_NAMESPACE