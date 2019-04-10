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

    //! ILinearElement

    //! Returns the length of the main horizontal
    //! @return The length of the main horizontal, in meters
    virtual double _GetLength() const override;

    //! Treat this alignment as a Dgn::DgnElement
    //! @return this Alignment cast as a Dgn::DgnElement
    virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const override final { return *this; }

    //! ISpatialLinearElement

    //! Converts a LinearReferencing::DistanceExpression to a DPoint3d.
    //! If the DistanceExpression cannot be converted, returns an invalid point.  ValidatedDPoint3d
    //! @param distanceExpression The LinearReferencing::DistanceExpression to convert to a point
    //! @return The DPoint3d calculated from LinearReferencing::DistanceExpression
    virtual DPoint3d _ToDPoint3d(LinearReferencing::DistanceExpressionCR distanceExpression) const override;

    //! Converts a DPoint3d to a LinearReferencing::DistanceExpression
    //! @param point The point to convert
    //! @return LinearReferencing::DistanceExpression calculated from @param point
    virtual LinearReferencing::DistanceExpression _ToDistanceExpression(DPoint3dCR point) const override;

    //__PUBLISH_SECTION_END__
    //! @private
    virtual Dgn::DgnDbStatus _OnDelete() const override;
    //__PUBLISH_SECTION_START__

public:
    //! A DistanceAlongStationPair wraps together the distance along an alignment with the distance's corresponding station.
    //! The station value should take into account any station equations present along the alignment.
    struct DistanceAlongStationPair
        {
    private:
        //! @private
        double m_distanceAlong, m_station;
      
    public:
        //! Instantiate a new DistanceAlongStationPair
        //! @param distanceAlong The linear distance along the Alignment
        //! @param station The station corresponding to the distanceAlong
        DistanceAlongStationPair(double distanceAlong, double station): m_distanceAlong(distanceAlong), m_station(station) {}

        //! @return The distance along the alignment
        double GetDistanceAlongFromStart() const { return m_distanceAlong; }
        
        //! @return The station along the alignment
        double GetStation() const { return m_station; }
        }; // DistanceAlongStationPair


    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(Alignment)

    //! @privatesection
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(Alignment)
    //! @publicsection

    //! Get the AlignmentModel that contains this Alignment.
    //! @return this Alignment's AlignmentModel.
    AlignmentModelPtr GetAlignmentModel() const { return dynamic_cast<AlignmentModelP>(GetModel().get()); }

    //! Access the HorizontalAlignemnt of this Alignment.
    //! @return This Alignment's HorizontalAlignment, or nullptr.
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr QueryHorizontal() const;    

    //! Query for the DgnElementId of the main vertical.
    //! @return The DgnElementId of the main VerticalAlignment of this alignment.  DgnElementId::IsValid() can be false if the Alignment does not have a vertical component.
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementId QueryMainVerticalId() const;

    //! Query for the main VerticalAlignment of this Alignment
    //! @return The main VerticalAlignment, or nullptr
    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr QueryMainVertical() const;

    //! Query for the DgnElementIds of any/all VerticalAlignments that are included in this Alignment.
    //! \ref QueryMainVertical()
    //! @return A DgnElementIdSet containing the DgnElementIds of each VerticalAlignment in this Alignment
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementIdSet QueryVerticalAlignmentIds() const;
    
    //! Query for the ModelId containing VerticalAlignments for this Alignment.
    //! @return The ModelId of the VerticalAlignmentModel associated with this Alignment. DgnModelId::IsValid() can be false if the Alignment does not have a vertical component.
    ROADRAILALIGNMENT_EXPORT Dgn::DgnModelId QueryVerticalAlignmentSubModelId() const;

    //! Query for the main Vertical/Horizontal alignments corresponding to this Alignment.
    //! @return An AlignmentPair holding this Alignment's main Vertical and Horizontal parts, if they exist.
    ROADRAILALIGNMENT_EXPORT CivilGeometry::AlignmentPairPtr QueryMainPair() const;

    //! Access the station points and their horizontal distance along this Alignment.
    //! @return Vector of DistanceAlongStationPair, in ascending order from the start station
    ROADRAILALIGNMENT_EXPORT bvector<DistanceAlongStationPair> QueryOrderedStations() const;

    //! Get the start station of this Alignment.
    //! This is set by the design application and may be non-zero.
    //! @return Alignment's start station
    double GetStartStation() const { return GetPropertyValueDouble("StartStation"); }

    //! Given a DgnElement, attempts to retrieve an Alignment associated with it.
    //! @param element The element to attempt to retrieve an associated alignment for.
    //! @return Either an Alignment, or nullptr if the element does not have any associated alignment.
    ROADRAILALIGNMENT_EXPORT static AlignmentCPtr GetAssociated(Dgn::DgnElementCR element);

    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILALIGNMENT_EXPORT static AlignmentPtr Create(AlignmentModelCR model);
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr InsertWithMainPair(CivilGeometry::AlignmentPairCR alignmentPair, Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr UpdateWithMainPair(CivilGeometry::AlignmentPairCR alignmentPair, Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateAprox3dGeom();
    ROADRAILALIGNMENT_EXPORT bool QueryIsRepresentedBy(Dgn::GeometrySourceCR geometrySource) const;
    void SetStartStation(double station) { SetPropertyValue("StartStation", station); }    

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetHorizontal(AlignmentCR alignment, HorizontalAlignmentCR vertical);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetMainVertical(AlignmentCR alignment, VerticalAlignmentCR vertical);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(AlignmentCR alignment, Dgn::GeometrySourceCR representedBy);
    //! @publicsection
    //__PUBLISH_SECTION_START__
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
    //! Get a HorizontalAlignmentsCPtr from the DgnElementId in the DgnDb.
    //! @param db The project database.
    //! @param id The DgnElementId of the HorizontalAlignments.
    //! @return The HorizontalAlignmentsCPtr with the given id, or nullptr.
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentsCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get<HorizontalAlignments>(id); }
    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentsCPtr Insert(AlignmentModelCR model);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCode CreateCode(Dgn::SpatialLocationPartitionCR alignmentPartition, Utf8StringCR name);
    //! @publicsection
    //__PUBLISH_SECTION_START__
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
    //! @private
    Dgn::DgnElementId m_alignmentId;
    //! @private
    mutable CurveVectorPtr m_geometry;

protected:
    //! @private
    explicit HorizontalAlignment(CreateParams const& params) : T_Super(params) {}
    //! @private
    explicit HorizontalAlignment(CreateParams const& params, AlignmentCR alignment, CurveVectorCR geometry);

    ROADRAILALIGNMENT_EXPORT virtual void _CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts) override;

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(HorizontalAlignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_GET_METHODS(HorizontalAlignment)

    //! Get the raw geometry of this HorizontalAlignment.
    //! @return CurveVector representing the raw geometry of this HorizontalAlignment.
    ROADRAILALIGNMENT_EXPORT CurveVectorCR GetGeometry() const;

    //! Get the Alignment holding this HorizontalAlignment.
    //! @return This HorizontalAlignment's parent Alignment, or nullptr.
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr QueryAlignment() const;

    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateElementGeom();
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentPtr Create(AlignmentCR alignment, CurveVectorCR horizontalGeometry);
    ROADRAILALIGNMENT_EXPORT void SetGeometry(CurveVectorCR);
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr Insert(Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr Update(Dgn::DgnDbStatus* stat = nullptr) { return GetDgnDb().Elements().Update<HorizontalAlignment>(*this, stat); }
    //! @publicsection
    //__PUBLISH_SECTION_START__
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
    
    //! @privatesection
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(VerticalAlignment)
    //! @publicsection

    //! Get the Alignment holding this VerticalAlignment.
    //! @return The Alignment holding this VerticalAlignment.
    AlignmentCR GetAlignment() const { return *Alignment::Get(GetDgnDb(), GetModel()->GetModeledElementId()); }

    //! Get the raw geometry of this VerticalAlignment.
    //! @return The CurveVector representing the raw geometry of this VerticalAlignment.
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetGeometry() const;

    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILALIGNMENT_EXPORT static VerticalAlignmentPtr Create(VerticalAlignmentModelCR model, CurveVectorCR verticalGeometry);
    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr InsertAsMainVertical(Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT void SetGeometry(CurveVectorCR);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateElementGeom();
    //! @publicsection
    //__PUBLISH_SECTION_START__

}; // VerticalAlignment


//__PUBLISH_SECTION_END__
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

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

/** @endcond */