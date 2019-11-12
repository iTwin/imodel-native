/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "RoadRailCategory.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! A long, narrow physical stretch that is designed for one or more modes of transportation 
//! which share a common course. It is typically defined along a main alignment. A Corridor 
//! assembles one or more Pathways with Pathway Separations in between them.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Corridor : LinearReferencing::LinearPhysicalElement, LinearReferencing::ILinearElementSource, LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(LinearReferencing::LinearPhysicalElement, Dgn::PhysicalElement)

protected:
    //! @private
    explicit Corridor(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit Corridor(Dgn::PhysicalElementR element) : T_Super(element) {}
    //! @private
    explicit Corridor(Dgn::PhysicalElementR element, CreateFromToParams const& fromToParams);
    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *get(); }
    //! @private
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *get(); }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Corridor)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(Corridor, Dgn::PhysicalElement)

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(TransportationNetworkCR scope, Utf8StringCR value);
    //! @private
    ROADRAILPHYSICAL_EXPORT static CorridorCPtr QueryByCode(TransportationNetworkCR scope, Utf8StringCR code);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(CorridorCR corridor, Dgn::GeometrySourceCR representedBy);
    //! @private
    ROADRAILPHYSICAL_EXPORT bool QueryIsRepresentedBy(Dgn::GeometrySourceCR) const;

    //! @private
    ROADRAILPHYSICAL_EXPORT static CorridorPtr Create(TransportationNetworkCR network, CreateFromToParams const& params);

    //! @private
    CorridorCPtr Update(Dgn::DgnDbStatus* status = nullptr) { return new Corridor(*getP()->GetDgnDb().Elements().Update<Dgn::PhysicalElement>(*getP(), status)); }
    ROADRAILPHYSICAL_EXPORT CorridorCPtr Insert(Dgn::DgnDbStatus* status = nullptr);
}; // Corridor

//=======================================================================================
//! Entry-point element containing Transportation-related details of a Corridor.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct TransportationSystem : GeometricElementWrapper<Dgn::PhysicalElement>
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::PhysicalElement)

protected:
    //! @private
    explicit TransportationSystem(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit TransportationSystem(Dgn::PhysicalElementR element) : T_Super(element) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TransportationSystem)
    //! Get a CorridorPortionsCPtr from the DgnElementId in the DgnDb.
    //! @param db The project database.
    //! @param id The DgnElementId of the CorridorPortions.
    //! @return The CorridorPortionsCPtr with the given id, or nullptr.
    ROADRAILPHYSICAL_EXPORT static TransportationSystemCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return new TransportationSystem(*db.Elements().Get<Dgn::PhysicalElement>(id)); }

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementId QueryId(CorridorCR corridor, Utf8StringCR codeVal);
    ROADRAILPHYSICAL_EXPORT static TransportationSystemCPtr Query(CorridorCR corridor, Utf8StringCR codeVal) { return Get(corridor.GetDgnDb(), QueryId(corridor, codeVal)); }

    //! Query for Pathways on this Corridor 
    ROADRAILPHYSICAL_EXPORT Dgn::DgnElementIdSet QueryPathwayIds() const;
    ROADRAILPHYSICAL_EXPORT Dgn::DgnElementIdSet QueryCorridorPortionIds() const;
    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILPHYSICAL_EXPORT static TransportationSystemCPtr Insert(CorridorCR corridor, Utf8StringCR codeVal);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(CorridorCR corridor, Utf8StringCR codeVal);
    //! @publicsection
    //__PUBLISH_SECTION_START__
    //! Gets the PhysicalModel that is modeling this element
    Dgn::PhysicalModelPtr GetTransportationSystemModel() const { return get()->GetSub<Dgn::PhysicalModel>(); }
}; // TransportationSystem

//=======================================================================================
//! Base class for long, narrow physical stretches of a Corridor, constructed for 
//! either a particular travel type, or a separation between them.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct CorridorPortionElement : GeometricElementWrapper<Dgn::PhysicalElement>, LinearReferencing::ILinearElementSource
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::PhysicalElement)

protected:
    //! @private
    explicit CorridorPortionElement(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit CorridorPortionElement(Dgn::PhysicalElementR element) : T_Super(element) {}
    //! @private
    virtual PathwayElementCP _ToPathway() const { return nullptr; }
    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *get(); }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CorridorPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(CorridorPortionElement, Dgn::PhysicalElement)

    Dgn::DgnElementId GetMainAlignmentId() const { return get()->GetPropertyValueId<Dgn::DgnElementId>(BRRP_PROP_CorridorPortionElement_MainAlignment); }
    ROADRAILPHYSICAL_EXPORT void SetMainAlignment(RoadRailAlignment::AlignmentCP alignment);

    //! Cast this CorridorPortionElement into a Pathway
    //! @return A Pathway or nullptr if this CorridorPortionElement is not a Pathway
    PathwayElementCP ToPathway() const { return _ToPathway(); }
}; // CorridorPortionElement

//=======================================================================================
//! A long, narrow physical stretch or a Corridor specially constructed for a particular 
//! travel type.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayElement : CorridorPortionElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(CorridorPortionElement, Dgn::PhysicalElement)

protected:
    //! @private
    explicit PathwayElement(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit PathwayElement(Dgn::PhysicalElementR element) : T_Super(element) {}
    //! @private
    virtual RailPhysical::RailwayCP _ToRailway() const { return nullptr; }
    //! @private
    virtual RoadPhysical::RoadwayCP _ToRoadway() const { return nullptr; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(PathwayElement, Dgn::PhysicalElement)

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scope, Utf8StringCR value);
    
    //! Query for a Pathway based on its Code value
    //! @return A Pathway or nullptr if no Pathway could be found with the provided Code value.
    ROADRAILPHYSICAL_EXPORT static PathwayElementCPtr QueryByCode(Dgn::PhysicalModelCR model, Utf8StringCR code);    

    //! Cast this CorridorPortionElement into a Railway
    //! @return A Railway or nullptr if this Pathway is not a Railway
    RailPhysical::RailwayCP ToRailway() const { return _ToRailway(); }

    //! Cast this CorridorPortionElement into a Railway
    //! @return A Roadway or nullptr if this Pathway is not a Roadway
    RoadPhysical::RoadwayCP ToRoadway() const { return _ToRoadway(); }
}; // PathwayElement

//=======================================================================================
//! A long, narrow physical stretch of a Corridor whose construction purpose is undetermined.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct UndeterminedCorridorPortion : CorridorPortionElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(CorridorPortionElement, Dgn::PhysicalElement)

protected:
    //! @private
    explicit UndeterminedCorridorPortion(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit UndeterminedCorridorPortion(Dgn::PhysicalElementR element) : T_Super(element) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(UndeterminedCorridorPortion)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(UndeterminedCorridorPortion, Dgn::PhysicalElement)

    //! @private
    ROADRAILPHYSICAL_EXPORT static UndeterminedCorridorPortionPtr Create(TransportationSystemCR corridorSegment, RoadRailAlignment::AlignmentCP mainAlignment);
}; // UndeterminedCorridorPortion

//=======================================================================================
//! Element representing design criteria for a Pathway.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayDesignCriteria : GeometricElementWrapper<Dgn::SpatialLocationElement>
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::SpatialLocationElement)

protected:
    //! @private
    explicit PathwayDesignCriteria(Dgn::SpatialLocationElementCR element) : T_Super(element) {}
    explicit PathwayDesignCriteria(Dgn::SpatialLocationElementR element) : T_Super(element) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayDesignCriteria)
    //! Get a PathwayDesignCriteriaCPtr from the DgnElementId in the DgnDb.
    //! @param db The project database.
    //! @param id The DgnElementId of the PathwayDesignCriteria.
    //! @return The PathwayDesignCriteriaCPtr with the given id, or nullptr.
    ROADRAILPHYSICAL_EXPORT static PathwayDesignCriteriaCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return new PathwayDesignCriteria(*db.Elements().Get<Dgn::SpatialLocationElement>(id)); }

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementId QueryId(PathwayElementCR pathway);
    ROADRAILPHYSICAL_EXPORT static PathwayDesignCriteriaCPtr Query(PathwayElementCR pathway) { return Get(pathway.GetDgnDb(), QueryId(pathway)); }

    //! Query for DesignSpeed segments associated with the Pathway owning this design criteria
    ROADRAILPHYSICAL_EXPORT bvector<Dgn::DgnElementId> QueryOrderedDesignSpeedIds() const;

    //! Get Owning Pathway element
    PathwayElementCPtr GetPathway() const { return PathwayElement::Get(GetDgnDb(), get()->GetParentId()); }
    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILPHYSICAL_EXPORT static PathwayDesignCriteriaCPtr Insert(PathwayElementCR pathway);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(PathwayElementCR pathway);
    //! @publicsection
    //__PUBLISH_SECTION_START__
    //! Gets the SpatialLocationModel that is modeling this element
    Dgn::SpatialLocationModelPtr GetDesignCriteriaModel() const { return get()->GetSub<Dgn::SpatialLocationModel>(); }
}; // PathwayDesignCriteria

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE