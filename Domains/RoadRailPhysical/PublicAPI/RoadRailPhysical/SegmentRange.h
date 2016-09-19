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

//=================================================================================
//! ElementHandler for SegmentRange Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SegmentRangeElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_SegmentRangeElement, SegmentRangeElement, SegmentRangeElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // SegmentRangeHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE