/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/SegmentRange.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class for Road and Rail range of physical segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct SegmentRangeElement : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_SegmentRangeElement, Dgn::PhysicalElement);
    friend struct SegmentRangeElementHandler;

protected:
    //! @private
    explicit SegmentRangeElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit SegmentRangeElement(CreateParams const& params, RoadRailAlignment::AlignmentCR alignment) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(SegmentRangeElement)

}; // SegmentRangeElement

//=======================================================================================
//! Physical range over a Road that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadRange : SegmentRangeElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadRange, SegmentRangeElement);
    friend struct RoadRangeHandler;

protected:
    //! @private
    explicit RoadRange(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit RoadRange(CreateParams const& params, RoadRailAlignment::AlignmentCR alignment) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadRange)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadRange)
    ROADRAILPHYSICAL_EXPORT static RoadRangePtr Create(Dgn::SpatialModelR model, RoadRailAlignment::AlignmentCR alignment);
}; // RoadRange

//=================================================================================
//! ElementHandler for SegmentRange Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SegmentRangeElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_SegmentRangeElement, SegmentRangeElement, SegmentRangeElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // SegmentRangeHandler

//=================================================================================
//! ElementHandler for RoadRange Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadRangeHandler : SegmentRangeElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadRange, RoadRange, RoadRangeHandler, SegmentRangeElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadRangeHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE