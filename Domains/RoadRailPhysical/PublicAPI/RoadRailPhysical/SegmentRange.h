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
struct SegmentRangeElement : Dgn::PhysicalElement, LinearReferencing::ILinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_SegmentRangeElement, Dgn::PhysicalElement);
    friend struct SegmentRangeElementHandler;

private:
    mutable RefCountedPtr<LinearReferencing::ICascadeLinearLocationChangesAlgorithm> m_cascadeAlgorithmPtr;

protected:
    //! @private
    explicit SegmentRangeElement(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }
    virtual Dgn::DgnDbStatus _OnChildUpdate(Dgn::DgnElementCR original, Dgn::DgnElementCR replacement) const override;
    virtual void _OnChildUpdated(Dgn::DgnElementCR child) const override;

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(SegmentRangeElement)

    ROADRAILPHYSICAL_EXPORT Dgn::DgnElementId QueryAlignmentId() const;
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetAlignment(SegmentRangeElementCR segmentRange, RoadRailAlignment::AlignmentCP alignment);
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

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadRange)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadRange)
    ROADRAILPHYSICAL_EXPORT static RoadRangePtr Create(Dgn::PhysicalModelR model);

    ROADRAILPHYSICAL_EXPORT RoadRangeCPtr InsertWithAlignment(RoadRailAlignment::AlignmentCR alignment, Dgn::DgnDbStatus* status = nullptr);
}; // RoadRange

//=======================================================================================
//! Physical range over a Rail that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RailRange : SegmentRangeElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RailRange, SegmentRangeElement);
    friend struct RailRangeHandler;

protected:
    //! @private
    explicit RailRange(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit RailRange(CreateParams const& params, RoadRailAlignment::AlignmentCR alignment);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RailRange)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RailRange)
    ROADRAILPHYSICAL_EXPORT static RailRangePtr Create(Dgn::PhysicalModelR model);

    ROADRAILPHYSICAL_EXPORT RailRangeCPtr InsertWithAlignment(RoadRailAlignment::AlignmentCR alignment, Dgn::DgnDbStatus* status = nullptr);
}; // RailRange


//=================================================================================
//! ElementHandler for SegmentRange Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SegmentRangeElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_SegmentRangeElement, SegmentRangeElement, SegmentRangeElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // SegmentRangeElementHandler

//=================================================================================
//! ElementHandler for RoadRange Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadRangeHandler : SegmentRangeElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadRange, RoadRange, RoadRangeHandler, SegmentRangeElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadRangeHandler

//=================================================================================
//! ElementHandler for RailRange Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RailRangeHandler : SegmentRangeElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RailRange, RailRange, RailRangeHandler, SegmentRangeElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RailRangeHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE