/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/Alignment.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignmentApi.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Main Linear-Element used in Road & Rail applications.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct Alignment : Dgn::SpatialLocationElement, LinearReferencing::ILinearElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_Alignment, Dgn::SpatialLocationElement);
    friend struct AlignmentHandler;

protected:
    //! @private
    explicit Alignment(CreateParams const& params) : T_Super(params) {}

    virtual double _GetLength() const override;
    virtual Dgn::DgnElementCR _ToElementLRImpl() const { return *this; }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(Alignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(Alignment)
    ROADRAILALIGNMENT_EXPORT static AlignmentPtr Create(AlignmentModelR model);

    ROADRAILALIGNMENT_EXPORT AlignmentHorizontalCPtr QueryHorizontal() const;
    ROADRAILALIGNMENT_EXPORT AlignmentVerticalCPtr QueryMainVertical() const;
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementIdSet QueryAlignmentVerticalIds() const;
}; // Alignment

//=======================================================================================
//! Horizontal piece of an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentHorizontal : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentHorizontal, Dgn::SpatialLocationElement);
    friend struct AlignmentHorizontalHandler;

private:
    CurveVectorCPtr m_geometry;

protected:
    //! @private
    explicit AlignmentHorizontal(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit AlignmentHorizontal(CreateParams const& params, CurveVectorCR geometry) : T_Super(params), m_geometry(&geometry) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentHorizontal)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentHorizontal)
    ROADRAILALIGNMENT_EXPORT static AlignmentHorizontalPtr Create(AlignmentCR alignment, CurveVectorCR horizontalGeometry);

    ROADRAILALIGNMENT_EXPORT CurveVectorCR GetGeometry() const { return *m_geometry; }
}; // AlignmentHorizontal

//=======================================================================================
//! Vertical/Profile piece(s) associated to an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentVertical : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentVertical, Dgn::SpatialLocationElement);
    friend struct AlignmentVerticalHandler;

private:
    CurveVectorCPtr m_geometry;

protected:
    //! @private
    explicit AlignmentVertical(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit AlignmentVertical(CreateParams const& params, CurveVectorCR geometry) : T_Super(params), m_geometry(&geometry) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentVertical)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentVertical)
    ROADRAILALIGNMENT_EXPORT static AlignmentVerticalPtr Create(AlignmentCR alignment, CurveVectorCR verticalGeometry);

    ROADRAILALIGNMENT_EXPORT CurveVectorCR GetGeometry() const { return *m_geometry; }
}; // AlignmentVertical


//=================================================================================
//! ElementHandler for Alignment Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_Alignment, Alignment, AlignmentHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; //AlignmentHandler

//=================================================================================
//! ElementHandler for AlignmentHorizontal Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentHorizontalHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentHorizontal, AlignmentHorizontal, AlignmentHorizontalHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; // AlignmentHorizontalHandler

//=================================================================================
//! ElementHandler for AlignmentVertical Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentVerticalHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentVertical, AlignmentVertical, AlignmentVerticalHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; // AlignmentVerticalHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE