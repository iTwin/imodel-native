/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

BEGIN_BENTLEY_ROADPHYSICAL_NAMESPACE

//=======================================================================================
//! Physical range over a Road that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Roadway : RoadRailPhysical::PathwayElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(RoadRailPhysical::PathwayElement, Dgn::PhysicalElement)

protected:
    //! @private
    explicit Roadway(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit Roadway(Dgn::PhysicalElementR element) : T_Super(element) {}
    //! @private
    virtual RoadwayCP _ToRoadway() const override { return this; }

public:
    DECLARE_ROADPHYSICAL_QUERYCLASS_METHODS(Roadway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(Roadway, Dgn::PhysicalElement)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadwayPtr Create(RoadRailPhysical::TransportationSystemCR transportationSystem, RoadRailAlignment::AlignmentCP mainAlignment);
}; // Roadway

END_BENTLEY_ROADPHYSICAL_NAMESPACE