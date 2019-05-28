/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "RoadRailCategory.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! A Physical Element being linearly designed along an Alignment.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ILinearlyDesignedElement
{
protected:
    virtual Dgn::DgnElementCR _ILinearlyDesignedElementToDgnElement() const = 0;

public:
    Dgn::DgnElementId GetDesignAlignmentId() const { return _ILinearlyDesignedElementToDgnElement().GetPropertyValueId<Dgn::DgnElementId>(BRRP_PROP_ILinearlyDesignedElement_DesignAlignment); }
    ROADRAILPHYSICAL_EXPORT Dgn::DgnDbStatus SetDesignAlignment(RoadRailAlignment::AlignmentCP alignment);
}; // ILinearlyDesignedElement

//=======================================================================================
//! A long, narrow physical stretch that is designed for one or more modes of transportation 
//! which share a common course. It is typically defined along a main alignment. A Corridor 
//! assembles one or more Pathways with Pathway Separations in between them.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Corridor : Dgn::PhysicalElement, LinearReferencing::ILinearElementSource, ILinearlyDesignedElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Corridor, Dgn::PhysicalElement);
    friend struct CorridorHandler;

protected:
    //! @private
    explicit Corridor(CreateParams const& params) : T_Super(params) {}
    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }
    //! @private
    virtual Dgn::DgnElementCR _ILinearlyDesignedElementToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Corridor)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(Corridor)

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(RoadRailNetworkCR scope, Utf8StringCR value);
    //! @private
    ROADRAILPHYSICAL_EXPORT static CorridorCPtr QueryByCode(RoadRailNetworkCR scope, Utf8StringCR code);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(CorridorCR corridor, Dgn::GeometrySourceCR representedBy);
    //! @private
    ROADRAILPHYSICAL_EXPORT bool QueryIsRepresentedBy(Dgn::GeometrySourceCR) const;

    //! @private
    ROADRAILPHYSICAL_EXPORT static CorridorPtr Create(RoadRailNetworkCR network);

    //! @private
    CorridorCPtr Update(Dgn::DgnDbStatus* status = nullptr) { return GetDgnDb().Elements().Update<Corridor>(*this, status); }
    ROADRAILPHYSICAL_EXPORT CorridorCPtr Insert(Dgn::DgnDbStatus* status = nullptr);
}; // Corridor

//=======================================================================================
//! Physical element representing a range of a corridor in a Corridor model.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct CorridorSegment : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CorridorSegment, Dgn::PhysicalElement);
    friend struct CorridorSegmentHandler;

protected:
    //! @private
    explicit CorridorSegment(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CorridorSegment)
    //! Get a CorridorPortionsCPtr from the DgnElementId in the DgnDb.
    //! @param db The project database.
    //! @param id The DgnElementId of the CorridorPortions.
    //! @return The CorridorPortionsCPtr with the given id, or nullptr.
    ROADRAILPHYSICAL_EXPORT static CorridorSegmentCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get<CorridorSegment>(id); }

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementId QueryId(CorridorCR corridor, Utf8StringCR codeVal);
    ROADRAILPHYSICAL_EXPORT static CorridorSegmentCPtr Query(CorridorCR corridor, Utf8StringCR codeVal) { return Get(corridor.GetDgnDb(), QueryId(corridor, codeVal)); }

    //! Query for Pathways on this Corridor in left-to-right order
    ROADRAILPHYSICAL_EXPORT bvector<Dgn::DgnElementId> QueryOrderedPathwayIds() const;
    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILPHYSICAL_EXPORT static CorridorSegmentCPtr Insert(CorridorCR corridor, Utf8StringCR codeVal);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(CorridorCR corridor, Utf8StringCR codeVal);
    //! @publicsection
    //__PUBLISH_SECTION_START__
    //! Gets the PhysicalModel that is modeling this element
    Dgn::PhysicalModelPtr GetCorridorSegmentModel() const { return GetSub<Dgn::PhysicalModel>(); }
}; // CorridorSegment

//=======================================================================================
//! Base class for Pathways and Separations between them
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct CorridorPortionElement : Dgn::PhysicalElement, LinearReferencing::ILinearElementSource, ILinearlyDesignedElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CorridorPortionElement, Dgn::PhysicalElement);
    friend struct CorridorPortionElementHandler;

protected:
    //! @private
    explicit CorridorPortionElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }
    //! @private
    virtual Dgn::DgnElementCR _ILinearlyDesignedElementToDgnElement() const override { return *this; }
    //! @private
    virtual PathwayElementCP _ToPathway() const { return nullptr; }
    //! @private

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CorridorPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(CorridorPortionElement)

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
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, CorridorPortionElement);
    friend struct PathwayElementHandler;

protected:
    //! @private
    explicit PathwayElement(CreateParams const& params) : T_Super(params) {}    

    //! @private
    virtual RailwayCP _ToRailway() const { return nullptr; }
    //! @private
    virtual RoadwayCP _ToRoadway() const { return nullptr; }

public:
    //=======================================================================================
    //! Order of a Pathway in its Corridor (left to right)
    //! @ingroup GROUP_RoadRailPhysical
    //=======================================================================================
    struct Order
    { 
    private:
        int32_t m_order;

    public:
        Order() : m_order(Invalid) {}
        Order(int32_t order) : m_order(order) {}

        operator int32_t() const { return m_order; }
        bool operator< (Order const& right) const { return m_order < right.m_order; }

        bool IsValid() { return m_order >= LeftMost && m_order <= RightMost; }

        static const int32_t LeftMost = 0;
        static const int32_t RightMost = 1000;
        static const int32_t Invalid = LeftMost - 1;
    }; // Order

    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(PathwayElement)

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scope, Utf8StringCR value);
    
    //! Query for a Pathway based on its Code value
    //! @return A Pathway or nullptr if no Pathway could be found with the provided Code value.
    ROADRAILPHYSICAL_EXPORT static PathwayElementCPtr QueryByCode(Dgn::PhysicalModelCR model, Utf8StringCR code);    

    //! Cast this CorridorPortionElement into a Railway
    //! @return A Railway or nullptr if this Pathway is not a Railway
    RailwayCP ToRailway() const { return _ToRailway(); }

    //! Cast this CorridorPortionElement into a Railway
    //! @return A Roadway or nullptr if this Pathway is not a Roadway
    RoadwayCP ToRoadway() const { return _ToRoadway(); }

    //! @private
    Order GetOrder() const { return GetPropertyValueInt32(BRRP_PROP_PathwayElement_Order); }
    //! @private
    void SetOrder(Order const& order) { SetPropertyValue(BRRP_PROP_PathwayElement_Order, order); }
}; // PathwayElement

//=======================================================================================
//! Element representing design criteria for a Pathway.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayDesignCriteria : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwayDesignCriteria, Dgn::SpatialLocationElement);
    friend struct PathwayDesignCriteriaHandler;

protected:
    //! @private
    explicit PathwayDesignCriteria(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayDesignCriteria)
    //! Get a PathwayDesignCriteriaCPtr from the DgnElementId in the DgnDb.
    //! @param db The project database.
    //! @param id The DgnElementId of the PathwayDesignCriteria.
    //! @return The PathwayDesignCriteriaCPtr with the given id, or nullptr.
    ROADRAILPHYSICAL_EXPORT static PathwayDesignCriteriaCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get<PathwayDesignCriteria>(id); }

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementId QueryId(PathwayElementCR pathway);
    ROADRAILPHYSICAL_EXPORT static PathwayDesignCriteriaCPtr Query(PathwayElementCR pathway) { return Get(pathway.GetDgnDb(), QueryId(pathway)); }

    //! Query for DesignSpeed segments associated with the Pathway owning this design criteria
    ROADRAILPHYSICAL_EXPORT bvector<Dgn::DgnElementId> QueryOrderedDesignSpeedIds() const;

    //! Get Owning Pathway element
    PathwayElementCPtr GetPathway() const { return PathwayElement::Get(GetDgnDb(), GetParentId()); }
    //__PUBLISH_SECTION_END__
    //! @privatesection
    ROADRAILPHYSICAL_EXPORT static PathwayDesignCriteriaCPtr Insert(PathwayElementCR pathway);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(PathwayElementCR pathway);
    //! @publicsection
    //__PUBLISH_SECTION_START__
    //! Gets the SpatialLocationModel that is modeling this element
    Dgn::SpatialLocationModelPtr GetDesignCriteriaModel() const { return GetSub<Dgn::SpatialLocationModel>(); }
}; // PathwayDesignCriteria

//=======================================================================================
//! Physical range over a Road that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Roadway : PathwayElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Roadway, PathwayElement);
    friend struct RoadwayHandler;

protected:
    //! @private
    explicit Roadway(CreateParams const& params) : T_Super(params) {}
    //! @private
    explicit Roadway(CreateParams const& params, Order const& order);
    //! @private
    virtual RoadwayCP _ToRoadway() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Roadway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(Roadway)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadwayPtr Create(CorridorSegmentCR corridorSegment, PathwayElement::Order const& order);
}; // Roadway

//=======================================================================================
//! Physical range over a Rail that can be segmented.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Railway : PathwayElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Railway, PathwayElement);
    friend struct RailwayHandler;

protected:
    //! @private
    explicit Railway(CreateParams const& params) : T_Super(params) {}
    //! @private
    explicit Railway(CreateParams const& params, Order const& order);

    //! @private
    virtual RailwayCP _ToRailway() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Railway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(Railway)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RailwayPtr Create(CorridorSegmentCR corridorSegment, PathwayElement::Order const& order);
}; // Railway


//__PUBLISH_SECTION_END__
//=================================================================================
//! ElementHandler for Corridor Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CorridorHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_Corridor, Corridor, CorridorHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // CorridorHandler

//=================================================================================
//! ElementHandler for CorridorPortions Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CorridorSegmentHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CorridorSegment, CorridorSegment, CorridorSegmentHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // CorridorSegmentHandler

//=================================================================================
//! ElementHandler for Corridor Portion Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CorridorPortionElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CorridorPortionElement, CorridorPortionElement, CorridorPortionElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // CorridorPortionElementHandler

//=================================================================================
//! ElementHandler for Pathway Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwayElementHandler : CorridorPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, PathwayElement, PathwayElementHandler, CorridorPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // PathwayElementHandler

//=================================================================================
//! ElementHandler for Corridor Portion Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwayDesignCriteriaHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwayDesignCriteria, PathwayDesignCriteria, PathwayDesignCriteriaHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILPHYSICAL_EXPORT)
}; // PathwayDesignCriteriaHandler

//=================================================================================
//! ElementHandler for Roadway Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayHandler : PathwayElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_Roadway, Roadway, RoadwayHandler, PathwayElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadwayHandler

//=================================================================================
//! ElementHandler for Railway Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RailwayHandler : PathwayElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_Railway, Railway, RailwayHandler, PathwayElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RailwayHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE