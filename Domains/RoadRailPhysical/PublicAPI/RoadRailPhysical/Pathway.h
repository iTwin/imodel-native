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
//! A Physical Assembly being linearly designed along an Alignment.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ILinearlyDesignedAssembly
{
protected:
    virtual Dgn::DgnElementCR _ILinearlyDesignedAssemblyToDgnElement() const = 0;

public:
    Dgn::DgnElementId GetDesignAlignmentId() const { return _ILinearlyDesignedAssemblyToDgnElement().GetPropertyValueId<Dgn::DgnElementId>(BRRP_PROP_ILinearlyDesignedAssembly_DesignAlignment); }
    ROADRAILPHYSICAL_EXPORT Dgn::DgnDbStatus SetDesignAlignment(RoadRailAlignment::AlignmentCP alignment);
}; // ILinearlyDesignedAssembly

//=======================================================================================
//! A long, narrow physical stretch that is designed for one or more modes of transportation 
//! which share a common course. It is typically defined along a main alignment. A Corridor 
//! assembles one or more Pathways with Pathway Separations in between them.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Corridor : Dgn::PhysicalElement, LinearReferencing::ILinearElementSource, ILinearlyDesignedAssembly
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Corridor, Dgn::PhysicalElement);
    friend struct CorridorHandler;

protected:
    //! @private
    explicit Corridor(CreateParams const& params) : T_Super(params) {}
    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }
    //! @private
    virtual Dgn::DgnElementCR _ILinearlyDesignedAssemblyToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Corridor)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(Corridor)

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scope, Utf8StringCR value);
    //! @private
    ROADRAILPHYSICAL_EXPORT static CorridorCPtr QueryByCode(Dgn::PhysicalModelCR model, Utf8StringCR code);    
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(CorridorCR corridor, Dgn::GeometrySourceCR representedBy);
    //! @private
    ROADRAILPHYSICAL_EXPORT bool QueryIsRepresentedBy(Dgn::GeometrySourceCR) const;

    //! Query for Pathways assembled by this Corridor in left-to-right order
    ROADRAILPHYSICAL_EXPORT bvector<Dgn::DgnElementId> QueryOrderedPathwayIds() const;

    //! @private
    ROADRAILPHYSICAL_EXPORT static CorridorPtr Create(Dgn::PhysicalModelR model);
}; // Corridor

//=======================================================================================
//! Base class for Pathways and Separations between them
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct CorridorPortionElement : Dgn::PhysicalElement, LinearReferencing::ILinearElementSource, ILinearlyDesignedAssembly
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CorridorPortionElement, Dgn::PhysicalElement);
    friend struct CorridorPortionElementHandler;

protected:
    //! @private
    explicit CorridorPortionElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }
    //! @private
    virtual Dgn::DgnElementCR _ILinearlyDesignedAssemblyToDgnElement() const override { return *this; }
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

        operator int32_t() { return m_order; }
        bool operator< (Order const& right) const { return m_order < right.m_order; }

        bool IsValid() { return m_order >= LeftMost && m_order <= RightMost; }

        static const int32_t LeftMost = 0;
        static const int32_t RightMost = 1000;
        static const int32_t Invalid = LeftMost - 1;
    };

    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(PathwayElement)

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

    ROADRAILPHYSICAL_EXPORT PathwayElementCPtr Insert(Order order, Dgn::DgnDbStatus* status = nullptr);
}; // PathwayElement

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
    virtual RoadwayCP _ToRoadway() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Roadway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(Roadway)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadwayPtr Create(CorridorCR corridor);
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
    virtual RailwayCP _ToRailway() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Railway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(Railway)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RailwayPtr Create(CorridorCR corridor);    
}; // Railway

//=======================================================================================
//! Interface providing access to pathway segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ICorridorPortionSingleFromTo : LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DEFINE_T_SUPER(LinearReferencing::ILinearlyLocatedSingleFromTo)

public:
    struct CreateFromToParams : LinearReferencing::ILinearlyLocatedSingleFromTo::CreateFromToParams
    {
        DEFINE_T_SUPER(LinearReferencing::ILinearlyLocatedSingleFromTo::CreateFromToParams)

        CorridorPortionElementCPtr m_corridorPortionCPtr;

        CreateFromToParams(CorridorPortionElementCR corridorPortion, 
            double fromDistanceFromStart, double toDistanceFromStart):
            m_corridorPortionCPtr(&corridorPortion),
            T_Super(*dynamic_cast<LinearReferencing::ILinearElementCP>(
                RoadRailAlignment::Alignment::Get(corridorPortion.GetDgnDb(), corridorPortion.GetDesignAlignmentId()).get()),
                fromDistanceFromStart, toDistanceFromStart) {}
    }; // CreateFromToParams

    Dgn::DgnElementId GetCorridorPortionId() const { return dynamic_cast<Dgn::DgnElementCP>(this)->GetParentId(); }

protected:
    ICorridorPortionSingleFromTo() {}
    ICorridorPortionSingleFromTo(CreateFromToParams const& params) : T_Super(params) {}

    static bool ValidateParams(CreateFromToParams const& params) 
        { if (!T_Super::ValidateParams(params)) return false; return params.m_corridorPortionCPtr.IsValid() && params.m_corridorPortionCPtr->GetElementId().IsValid(); }
    virtual void _OnCreate(CreateFromToParams const& params) { _SetLinearElement(params.m_linearElementCPtr->GetElementId()); }
}; // ICorridorPortionSingleFromTo


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