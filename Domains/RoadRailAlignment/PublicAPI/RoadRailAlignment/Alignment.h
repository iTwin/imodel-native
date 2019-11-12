/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
struct Alignment : GeometricElementWrapper<Dgn::SpatialLocationElement>, LinearReferencing::ISpatialLinearElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::SpatialLocationElement)
    friend struct HorizontalAlignment;

protected:
    explicit Alignment(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit Alignment(Dgn::SpatialLocationElement& element) : T_Super(element) {}

    //! ILinearElement

    //! Treat this alignment as a Dgn::DgnElement
    //! @return this Alignment cast as a Dgn::DgnElement
    virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const override final { return *get(); }

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
    //virtual Dgn::DgnDbStatus _OnDelete() const override;
    //! @private
    void _SetHorizontal(HorizontalAlignmentCR horizontal);
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
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(Alignment, Dgn::SpatialLocationElement)
    //! @publicsection

    //! Get the SpatialModel that contains this Alignment.
    //! @return this Alignment's AlignmentModel.
    Dgn::SpatialModelPtr GetAlignmentModel() const { return dynamic_cast<Dgn::SpatialModelP>(get()->GetModel().get()); }

    //! Get the DgnElementId of the horizontal.
    //! @return The DgnElementId of the Horizontal of this alignment.
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementId GetHorizontalId() const { return get()->GetPropertyValueId<Dgn::DgnElementId>(BRRA_PROP_Alignment_Horizontal); }

    //! Get the DgnElementId of the main vertical.
    //! @return The DgnElementId of the main VerticalAlignment of this alignment.  DgnElementId::IsValid() can be false if the Alignment does not have a vertical component.
    ROADRAILALIGNMENT_EXPORT Dgn::DgnElementId GetMainVerticalId() const { return get()->GetPropertyValueId<Dgn::DgnElementId>(BRRA_PROP_Alignment_MainVertical); }

    //! Get the Horizontal of this Alignment
    //! @return The Horizontal, or nullptr
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr GetHorizontal() const;

    //! Get the main VerticalAlignment of this Alignment
    //! @return The main VerticalAlignment, or nullptr
    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr GetMainVertical() const;

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
    double GetStartStation() const { return get()->GetPropertyValueDouble(BRRA_PROP_Alignment_StartStation); }

    //! Given a DgnElement, attempts to retrieve an Alignment associated with it.
    //! @param element The element to attempt to retrieve an associated alignment for.
    //! @return Either an Alignment, or nullptr if the element does not have any associated alignment.
    ROADRAILALIGNMENT_EXPORT static AlignmentCPtr GetAssociated(Dgn::DgnElementCR element);

    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILALIGNMENT_EXPORT static AlignmentPtr Create(Dgn::SpatialModelCR model);
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr InsertWithMainPair(CivilGeometry::AlignmentPairCR alignmentPair, Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT AlignmentCPtr UpdateWithMainPair(CivilGeometry::AlignmentPairCR alignmentPair, Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateAprox3dGeom(Dgn::DgnSubCategoryId subCategoryId = Dgn::DgnSubCategoryId());
    ROADRAILALIGNMENT_EXPORT bool QueryIsRepresentedBy(Dgn::GeometrySourceCR geometrySource) const;
    void SetStartStation(double station) { getP()->SetPropertyValue(BRRA_PROP_Alignment_StartStation, station); }

    ROADRAILALIGNMENT_EXPORT void SetMainVertical(VerticalAlignmentCP vertical);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(AlignmentCR alignment, Dgn::GeometrySourceCR representedBy);
    //! @publicsection
    //__PUBLISH_SECTION_START__
}; // Alignment

//=======================================================================================
//! SpatialLocationElement representing design alignments in a Physical model.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct DesignAlignments : GeometricElementWrapper<Dgn::SpatialLocationElement>
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::SpatialLocationElement)

private:
    static Dgn::DgnCode CreateCodeBasic(Dgn::SpatialModelCR model, Utf8StringCR codeVal);

protected:
    explicit DesignAlignments(Dgn::SpatialLocationElement const& element): T_Super(element) {}
    explicit DesignAlignments(Dgn::SpatialLocationElement& element) : T_Super(element) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(DesignAlignments)
    //! Get a DesignAlignmentsCPtr from the DgnElementId in the DgnDb.
    //! @param db The project database.
    //! @param id The DgnElementId of the DesignAlignments.
    //! @return The DesignAlignmentsCPtr with the given id, or nullptr.
    ROADRAILALIGNMENT_EXPORT static DesignAlignmentsCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return new DesignAlignments(*db.Elements().Get<Dgn::SpatialLocationElement>(id)); }

    //! Query for the elementId representing all of the Design Alignments for a parent Spatial Model
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnElementId QueryId(Dgn::SpatialModelCR parentSpatialModel, Utf8StringCR codeVal);

    //! Query for the element representing all of the Design Alignments for a parent Spatial Model
    static DesignAlignmentsCPtr Query(Dgn::SpatialModelCR parentSpatialModel, Utf8StringCR codeVal) { return Get(parentSpatialModel.GetDgnDb(), QueryId(parentSpatialModel, codeVal)); }
    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILALIGNMENT_EXPORT static DesignAlignmentsCPtr Insert(Dgn::SpatialModelCR model, Utf8StringCR codeVal);
    static Dgn::DgnCode CreateCode(Dgn::SpatialModelCR model, Utf8StringCR codeVal) { return CreateCodeBasic(model, codeVal); }
    //! @publicsection
    //__PUBLISH_SECTION_START__
    //! Gets the SpatialLocationModel that is modeling this element
    Dgn::SpatialLocationModelPtr GetAlignmentModel() const { return get()->GetSub<Dgn::SpatialLocationModel>(); }
}; // DesignAlignments

//=======================================================================================
//! SpatialLocationElement representing all the horizontal alignments in
//! an Alignments model.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct HorizontalAlignments : GeometricElementWrapper<Dgn::SpatialLocationElement>
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::SpatialLocationElement)

protected:
    //! @private
    explicit HorizontalAlignments(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit HorizontalAlignments(Dgn::SpatialLocationElement& element) : T_Super(element) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(HorizontalAlignments)
    //! Get a HorizontalAlignmentsCPtr from the DgnElementId in the DgnDb.
    //! @param db The project database.
    //! @param id The DgnElementId of the HorizontalAlignments.
    //! @return The HorizontalAlignmentsCPtr with the given id, or nullptr.
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentsCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return new HorizontalAlignments(*db.Elements().Get<Dgn::SpatialLocationElement>(id)); }

    //! Query for the element representing all of the HorizontalAlignments for an AlignmentModel
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnElementId QueryId(Dgn::SpatialModelCR alignmentModel);
    static HorizontalAlignmentsCPtr Query(Dgn::SpatialModelCR alignmentModel) { return Get(alignmentModel.GetDgnDb(), QueryId(alignmentModel)); }
    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILALIGNMENT_EXPORT static HorizontalAlignmentsCPtr Insert(Dgn::SpatialModelCR model);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCode CreateCode(Dgn::SpatialModelCR spatialModel);
    //! @publicsection
    //__PUBLISH_SECTION_START__
    //! Gets the SpatialLocationModel that is modeling this element
    Dgn::SpatialLocationModelPtr GetHorizontalModel() const { return get()->GetSub<Dgn::SpatialLocationModel>(); }
}; // HorizontalAlignments

//=======================================================================================
//! Horizontal piece of an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct HorizontalAlignment : GeometricElementWrapper<Dgn::SpatialLocationElement>
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::SpatialLocationElement)
    friend struct Alignment;

private:
    //! @private
    Dgn::DgnElementId m_alignmentId;
    //! @private
    mutable CurveVectorPtr m_geometry;
    AlignmentPtr m_editAlignment;

protected:
    explicit HorizontalAlignment(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit HorizontalAlignment(Dgn::SpatialLocationElement& element) : T_Super(element) {}
    explicit HorizontalAlignment(Dgn::SpatialLocationElement& element, AlignmentCR alignment, CurveVectorCR geometry);

    //ROADRAILALIGNMENT_EXPORT virtual void _CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts) override;

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(HorizontalAlignment)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_GET_METHODS(HorizontalAlignment, Dgn::SpatialLocationElement)

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
    ROADRAILALIGNMENT_EXPORT HorizontalAlignmentCPtr Update(Dgn::DgnDbStatus* stat = nullptr);
    //! @publicsection
    //__PUBLISH_SECTION_START__
}; // HorizontalAlignment

//=======================================================================================
//! Vertical/Profile piece(s) associated to an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct VerticalAlignment : GeometricElementWrapper<Dgn::GeometricElement2d>
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::GeometricElement2d)
    friend struct Alignment;

private:
    AlignmentPtr m_editAlignment;

protected:
    //! @private
    explicit VerticalAlignment(Dgn::GeometricElement2d const& element) : T_Super(element) {}
    explicit VerticalAlignment(Dgn::GeometricElement2d& element) : T_Super(element) {}
    //! @private
    explicit VerticalAlignment(Dgn::GeometricElement2d& element, CurveVectorCR geometry);
    
public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(VerticalAlignment)
    
    //! @privatesection
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(VerticalAlignment, Dgn::GeometricElement2d)
    //! @publicsection

    //! Get the Alignment holding this VerticalAlignment.
    //! @return The Alignment holding this VerticalAlignment.
    AlignmentCR GetAlignment() const { return *Alignment::Get(GetDgnDb(), get()->GetModel()->GetModeledElementId()); }

    //! Get the raw geometry of this VerticalAlignment.
    //! @return The CurveVector representing the raw geometry of this VerticalAlignment.
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr GetGeometry() const;

    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILALIGNMENT_EXPORT static VerticalAlignmentPtr Create(AlignmentCR alignment, CurveVectorCR verticalGeometry);
    ROADRAILALIGNMENT_EXPORT VerticalAlignmentCPtr InsertAsMainVertical(Dgn::DgnDbStatus* stat = nullptr);
    ROADRAILALIGNMENT_EXPORT void SetGeometry(CurveVectorCR);
    ROADRAILALIGNMENT_EXPORT Dgn::DgnDbStatus GenerateElementGeom();
    //! @publicsection
    //__PUBLISH_SECTION_START__

}; // VerticalAlignment

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

/** @endcond */