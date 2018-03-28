/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/Alignment.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include "RoadRailAlignment.h"
#include "AlignmentCategory.h"
#include "AlignmentModel.h"

/** @cond BENTLEY_SDK_RoadRailAlignment */

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Main Linear-Element used in Road & Rail applications.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct Alignment : Dgn::SpatialLocationElement, LinearReferencing::ISpatialLinearElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_Alignment, Dgn::SpatialLocationElement);
    friend struct AlignmentHandler;

protected:
    //! @private
    explicit Alignment(CreateParams const& params);

    // ILinearElement
    virtual double _GetLength() const override;
    virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const override final { return *this; }

    // ISpatialLinearElement
    virtual DPoint3d _ToDPoint3d(LinearReferencing::DistanceExpressionCR distanceExpression) const override;
    virtual LinearReferencing::DistanceExpression _ToDistanceExpression(DPoint3dCR point) const override;

    virtual Dgn::DgnDbStatus _OnDelete() const override;

public:
    struct DistanceAlongStationPair
        {
        double m_distanceAlong, m_station;

        DistanceAlongStationPair(double distanceAlong, double station): m_distanceAlong(distanceAlong), m_station(station) {}

        double GetDistanceAlongFromStart() const { return m_distanceAlong; }
        double GetStation() const { return m_station; }
        }; // DistanceAlongStationPair

    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(Alignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(Alignment)
    ROADRAILALIGNMENT_EXPORT static AlignmentPtr Create(AlignmentModelCR model);

    AlignmentModelPtr GetAlignmentModel() const { return dynamic_cast<AlignmentModelP>(GetModel().get()); }
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr QueryHorizontal() const;    
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementId QueryMainVerticalId() const;
    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr QueryMainVertical() const;
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementIdSet QueryVerticalAlignmentIds() const;
    ROADRAILALIGNMENT_EXPORT Dgn::DgnModelId QueryVerticalAlignmentSubModelId() const;
    ROADRAILALIGNMENT_EXPORT CivilGeometry::AlignmentPairPtr QueryMainPair() const;
    ROADRAILALIGNMENT_EXPORT bvector<DistanceAlongStationPair> QueryOrderedStations() const;
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr InsertWithMainPair(CivilGeometry::AlignmentPairCR alignmentPair, Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr UpdateWithMainPair(CivilGeometry::AlignmentPairCR alignmentPair, Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateAprox3dGeom();

    double GetStartStation() const { return GetPropertyValueDouble("StartStation"); }
    void SetStartStation(double station) { SetPropertyValue("StartStation", station); }

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetHorizontal(AlignmentCR alignment, HorizontalAlignmentCR vertical);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetMainVertical(AlignmentCR alignment, VerticalAlignmentCR vertical);

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(AlignmentCR alignment, Dgn::DgnElementCR representedBy);
    ROADRAILALIGNMENT_EXPORT static AlignmentCPtr GetAssociated(Dgn::DgnElementCR element);
}; // Alignment

//=======================================================================================
//! Information Content Element representing all the horizontal alignments in
//! an Alignments model.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct HorizontalAlignments : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignments, Dgn::SpatialLocationElement);
    friend struct HorizontalAlignmentsHandler;

protected:
    //! @private
    explicit HorizontalAlignments(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(HorizontalAlignments)
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentsCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get<HorizontalAlignments>(id); }
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentsCPtr Insert(AlignmentModelCR model);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCode CreateCode(Dgn::SpatialLocationPartitionCR alignmentPartition, Utf8StringCR name);
}; // HorizontalAlignments

//=======================================================================================
//! Horizontal piece of an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct HorizontalAlignment : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignment, Dgn::SpatialLocationElement);
    friend struct HorizontalAlignmentHandler;

private:
    Dgn::DgnElementId m_alignmentId;
    mutable CurveVectorPtr m_geometry;

protected:
    //! @private
    explicit HorizontalAlignment(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit HorizontalAlignment(CreateParams const& params, AlignmentCR alignment, CurveVectorCR geometry);

    ROADRAILALIGNMENT_EXPORT virtual void _CopyFrom(Dgn::DgnElementCR source) override;

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(HorizontalAlignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_GET_METHODS(HorizontalAlignment)
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentPtr Create(AlignmentCR alignment, CurveVectorCR horizontalGeometry);

    ROADRAILALIGNMENT_EXPORT CurveVectorCR GetGeometry() const;
    ROADRAILALIGNMENT_EXPORT void SetGeometry(CurveVectorCR);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateElementGeom();

    ROADRAILALIGNMENT_EXPORT AlignmentCPtr QueryAlignment() const;

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
    explicit VerticalAlignment(CreateParams const& params, CurveVectorCR geometry);

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(VerticalAlignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(VerticalAlignment)
    ROADRAILALIGNMENT_EXPORT static VerticalAlignmentPtr Create(VerticalAlignmentModelCR model, CurveVectorCR verticalGeometry);

    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr InsertAsMainVertical(Dgn::DgnDbStatus* stat = nullptr);

    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetGeometry() const;
    ROADRAILALIGNMENT_EXPORT void SetGeometry(CurveVectorCR);
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
//! ElementHandler for HorizontalAlignments Element
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HorizontalAlignmentsHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignments, HorizontalAlignments, HorizontalAlignmentsHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; // HorizontalAlignmentsHandler

//=================================================================================
//! ElementHandler for HorizontalAlignment Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HorizontalAlignmentHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_HorizontalAlignment, HorizontalAlignment, HorizontalAlignmentHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
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

/** @endcond */