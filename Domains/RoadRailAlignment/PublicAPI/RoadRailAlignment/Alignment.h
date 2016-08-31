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
//=======================================================================================
struct Alignment : Dgn::SpatialLocationElement, LinearReferencing::ILinearElement
{
DGNELEMENT_DECLARE_MEMBERS(RRA_CLASS_Alignment, Dgn::SpatialLocationElement);

protected:
    //! @private
    explicit Alignment(CreateParams const& params) : T_Super(params) {}

    virtual double _GetLength() const override;
    virtual Dgn::DgnElementCR _ToElementLRImpl() const { return *this; }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(Alignment)
    ROADRAILALIGNMENT_EXPORT static AlignmentPtr Create(AlignmentModelR model);

    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr QueryHorizontal() const;
    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr QueryMainVertical() const;
    ROADRAILALIGNMENT_EXPORT DgnElementIdSet QueryVerticalAlignmentIds() const;
}; // Alignment

//=======================================================================================
//! Horizontal piece of an Alignment.
//=======================================================================================
struct AlignmentHorizontal : Dgn::SpatialLocationElement
{
DGNELEMENT_DECLARE_MEMBERS(RRA_CLASS_AlignmentHorizontal, Dgn::SpatialLocationElement);

private:
    CurveVectorCPtr m_geometry;

protected:
    //! @private
    explicit AlignmentHorizontal(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit AlignmentHorizontal(CreateParams const& params, CurveVectorCR geometry) : T_Super(params), m_geometry(geometry) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentHorizontal)
    ROADRAILALIGNMENT_EXPORT static AlignmentHorizontalPtr Create(AlignmentCR alignment, CurveVectorCR horizontalGeometry);

    ROADRAILALIGNMENT_EXPORT CurveVectorCR GetGeometry() const { return *m_geometry; }
}; // AlignmentHorizontal

//=======================================================================================
//! Vertical/Profile piece(s) associated to an Alignment.
//=======================================================================================
struct AlignmentVertical : Dgn::SpatialLocationElement
{
DGNELEMENT_DECLARE_MEMBERS(RRA_CLASS_AlignmentVertical, Dgn::SpatialLocationElement);

private:
    CurveVectorCPtr m_geometry;

protected:
    //! @private
    explicit AlignmentVertical(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit AlignmentVertical(CreateParams const& params, CurveVectorCR geometry) : T_Super(params), m_geometry(geometry) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentVertical)
    ROADRAILALIGNMENT_EXPORT static AlignmentVerticalPtr Create(AlignmentCR alignment, CurveVectorCR verticalGeometry);

    ROADRAILALIGNMENT_EXPORT CurveVectorCR GetGeometry() const { return *m_geometry; }
}; // AlignmentVertical

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE