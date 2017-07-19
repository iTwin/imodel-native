/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/Alignment.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct Alignment : Dgn::SpatialLocationElement, LinearReferencing::ISegmentableLinearElement, LinearReferencing::ISpatialLinearElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_Alignment, Dgn::SpatialLocationElement);
    friend struct AlignmentHandler;

protected:
    //! @private
    explicit Alignment(CreateParams const& params) : T_Super(params) {}

    // ILinearElement
    virtual double _GetLength() const override;
    virtual double _GetStartValue() const override { return 0.0; }
    virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const override final { return *this; }

    // ISpatialLinearElement
    virtual DPoint3d _ToDPoint3d(LinearReferencing::DistanceExpressionCR distanceExpression) const override;
    virtual LinearReferencing::DistanceExpression _ToDistanceExpression(DPoint3dCR point) const override;

    virtual Dgn::DgnDbStatus _OnDelete() const override;

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(Alignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(Alignment)
    ROADRAILALIGNMENT_EXPORT static AlignmentPtr Create(AlignmentModelCR model);

    AlignmentModelPtr GetAlignmentModel() const { return dynamic_cast<AlignmentModelP>(GetModel().get()); }
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr QueryHorizontal() const;    
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementId QueryMainVerticalId() const;
    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr QueryMainVertical() const;
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementIdSet QueryVerticalAlignmentIds() const;
    ROADRAILALIGNMENT_EXPORT Dgn::DgnModelId QueryVerticalAlignmentSubModelId() const;
    ROADRAILALIGNMENT_EXPORT AlignmentPairPtr QueryMainPair() const;
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr InsertWithMainPair(AlignmentPairCR alignmentPair, Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr UpdateWithMainPair(AlignmentPairCR alignmentPair, Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateAprox3dGeom();

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetHorizontal(AlignmentCR alignment, HorizontalAlignmentCR vertical);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetMainVertical(AlignmentCR alignment, VerticalAlignmentCR vertical);
}; // Alignment

//=======================================================================================
//! Information Content Element representing all the horizontal alignments in
//! an Alignments portion.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct HorizontalAlignmentsPortion : Dgn::SpatialLocationPortion
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignmentsPortion, Dgn::SpatialLocationPortion);
    friend struct HorizontalAlignmentsPortionHandler;

protected:
    //! @private
    explicit HorizontalAlignmentsPortion(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(HorizontalAlignmentsPortion)
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentsPortionCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get<HorizontalAlignmentsPortion>(id); }
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentsPortionCPtr InsertPortion(AlignmentModelCR model);
}; // HorizontalAlignmentsPortion

//=======================================================================================
//! Horizontal piece of an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct HorizontalAlignment : Dgn::GeometricElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignment, Dgn::GeometricElement2d);
    friend struct HorizontalAlignmentHandler;

private:
    Dgn::DgnElementId m_alignmentId;
    mutable CurveVectorPtr m_geometry;

protected:
    //! @private
    explicit HorizontalAlignment(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit HorizontalAlignment(CreateParams const& params, AlignmentCR alignment, CurveVectorR geometry);

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(HorizontalAlignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_GET_METHODS(HorizontalAlignment)
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentPtr Create(AlignmentCR alignment, CurveVectorR horizontalGeometry);

    ROADRAILALIGNMENT_EXPORT CurveVectorCR GetGeometry() const;
    ROADRAILALIGNMENT_EXPORT void SetGeometry(CurveVectorR);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateElementGeom();

    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr Insert(Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr Update(Dgn::DgnDbStatus* stat = nullptr) { return GetDgnDb().Elements().Update<HorizontalAlignment>(*this, stat); }
}; // HorizontalAlignment

//=======================================================================================
//! Vertical/Profile piece(s) associated to an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct VerticalAlignment : Dgn::GeometricElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_VerticalAlignment, Dgn::GeometricElement2d);
    friend struct VerticalAlignmentHandler;

protected:
    //! @private
    explicit VerticalAlignment(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit VerticalAlignment(CreateParams const& params, CurveVectorR geometry);

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(VerticalAlignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(VerticalAlignment)
    ROADRAILALIGNMENT_EXPORT static VerticalAlignmentPtr Create(VerticalAlignmentModelCR model, CurveVectorR verticalGeometry);

    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr InsertAsMainVertical(Dgn::DgnDbStatus* stat = nullptr);

    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetGeometry() const;
    ROADRAILALIGNMENT_EXPORT void SetGeometry(CurveVectorR);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateElementGeom();

    AlignmentCR GetAlignment() const { return *Alignment::Get(GetDgnDb(), GetModel()->GetModeledElementId()); }
}; // VerticalAlignment


//=================================================================================
//! ElementHandler for Alignment Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_Alignment, Alignment, AlignmentHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; //AlignmentHandler

//=================================================================================
//! ElementHandler for HorizontalAlignmentsPortion Element
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HorizontalAlignmentsPortionHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignmentsPortion, HorizontalAlignmentsPortion, HorizontalAlignmentsPortionHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; // HorizontalAlignmentsHandler

//=================================================================================
//! ElementHandler for HorizontalAlignment Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HorizontalAlignmentHandler : Dgn::dgn_ElementHandler::Geometric2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignment, HorizontalAlignment, HorizontalAlignmentHandler, Dgn::dgn_ElementHandler::Geometric2d, ROADRAILALIGNMENT_EXPORT)
}; // HorizontalAlignmentHandler

//=================================================================================
//! ElementHandler for VerticalAlignment Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE VerticalAlignmentHandler : Dgn::dgn_ElementHandler::Geometric2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_VerticalAlignment, VerticalAlignment, VerticalAlignmentHandler, Dgn::dgn_ElementHandler::Geometric2d, ROADRAILALIGNMENT_EXPORT)
}; // VerticalAlignmentHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE