/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/Pathway.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "RoadRailCategory.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! An ILinearElementSource providing an ILinearElement considered as the main 
//! linear-referencing axis for Road/Rail purposes.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct IMainLinearElementSource : LinearReferencing::ILinearElementSource
{
    Dgn::DgnElementId GetMainLinearElementId() const { return _ILinearElementSourceToDgnElement().GetPropertyValueId<Dgn::DgnElementId>("MainLinearElement"); }

    template <class T> RefCountedCPtr<T> GetMainLinearElementAs() const { return _ILinearElementSourceToDgnElement().GetDgnDb().Elements().Get<T>(GetMainLinearElementId()); }
    ROADRAILPHYSICAL_EXPORT Dgn::DgnDbStatus SetMainLinearElement(LinearReferencing::ILinearElementCP linearElement);
}; // IMainLinearElementSource

//=======================================================================================
//! A long, narrow physical stretch that is designed for one or more modes of transportation 
//! which share a common course. It is typically defined along a main alignment. A Corridor 
//! assembles one or more Pathways with Pathway Separations in between them.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct Corridor : Dgn::PhysicalElement, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_Corridor, Dgn::PhysicalElement);
    friend struct CorridorHandler;

protected:
    //! @private
    explicit Corridor(CreateParams const& params) : T_Super(params) {}
    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }

public:
    struct PathwaySeparationInfo
    {
        Dgn::DgnElementId m_elementId, m_leftPathwayId, m_rightPathwayId;

        PathwaySeparationInfo() {}
        PathwaySeparationInfo(Dgn::DgnElementId elementId, Dgn::DgnElementId leftPathwayId, Dgn::DgnElementId rightPathwayId) : 
            m_elementId(elementId), m_leftPathwayId(leftPathwayId), m_rightPathwayId(rightPathwayId) {}

        Dgn::DgnElementId GetElementId() const { return m_elementId; }
        Dgn::DgnElementId GetLeftPathwayId() const { return m_leftPathwayId; }
        Dgn::DgnElementId GetRightPathwayId() const { return m_rightPathwayId; }

        bool operator< (PathwaySeparationInfo const& right) const { return m_elementId < right.m_elementId; }
    }; // PathwaySeparationInfo

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
    //! Query for Pathway Separations assembled by this Corridor
    ROADRAILPHYSICAL_EXPORT bset<PathwaySeparationInfo> QueryPathwaySeparationInfos() const;

    //! @private
    ROADRAILPHYSICAL_EXPORT static CorridorPtr Create(Dgn::PhysicalModelR model);
}; // Corridor

//=======================================================================================
//! Base class for Pathways and Separations between them
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct CorridorPortionElement : Dgn::PhysicalElement, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CorridorPortionElement, Dgn::PhysicalElement);
    friend struct CorridorPortionElementHandler;

protected:
    //! @private
    explicit CorridorPortionElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }
    //! @private
    virtual PathwayElementCP _ToPathway() const { return nullptr; }
    //! @private
    virtual PathwaySeparationElementCP _ToPathwaySeparation() const { return nullptr; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CorridorPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(CorridorPortionElement)

    //! Cast this CorridorPortionElement into a Pathway
    //! @return A Pathway or nullptr if this CorridorPortionElement is not a Pathway
    PathwayElementCP ToPathway() const { return _ToPathway(); }

    //! Cast this CorridorPortionElement into a PathwaySeparation
    //! @return A PathwaySeparation or nullptr if this CorridorPortionElement is not a PathwaySeparation
    PathwaySeparationElementCP ToPathwaySeparation() const { return _ToPathwaySeparation(); }
}; // CorridorPortionElement

//=======================================================================================
//! 
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
    enum class Order: int32_t { LeftMost = 0, RightMost = 1000 };

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

    ROADRAILPHYSICAL_EXPORT PathwayElementCPtr Insert(int32_t order, Dgn::DgnDbStatus* status = nullptr);
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
//! A CorridorPortion specially constructed to separate two Pathways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwaySeparationElement : CorridorPortionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwaySeparationElement, CorridorPortionElement);
    friend struct PathwaySeparationElementHandler;

protected:
    //! @private
    explicit PathwaySeparationElement(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwaySeparationElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(PathwaySeparationElement)

    //! Get the Pathway located at the left side of this Separation
    Dgn::DgnElementId GetPathwayIdLeftSide() const { return GetPropertyValueId<Dgn::DgnElementId>("PathwayLeftSide"); }
    //! Get the Pathway located at the right side of this Separation
    Dgn::DgnElementId GetPathwayIdRightSide() const { return GetPropertyValueId<Dgn::DgnElementId>("PathwayRightSide"); }

    //! @private
    void SetPathwayLeftSide(PathwayElementCR leftPathway) { SetPropertyValue("PathwayLeftSide", leftPathway.GetElementId()); }
    //! @private
    void SetPathwayRightSide(PathwayElementCR rightPathway) { SetPropertyValue("PathwayRightSide", rightPathway.GetElementId()); }
}; // PathwaySeparationElement

//=======================================================================================
//! A generic PathwaySeparationElement.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwaySeparation : PathwaySeparationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwaySeparation, PathwaySeparationElement);
    friend struct PathwaySeparationHandler;

protected:
    //! @private
    explicit PathwaySeparation(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwaySeparation)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(PathwaySeparation)

    //! @private
    ROADRAILPHYSICAL_EXPORT static PathwaySeparationPtr Create(CorridorCR corridor, LinearReferencing::ILinearElementCR linearElement, 
        PathwayElementCR leftPathway, PathwayElementCR rightPathway);
}; // PathwaySeparation

//=======================================================================================
//! Utility class facilitating some operations against ILinearElements in the 
//! context of the Road/Rail discipline.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ILinearElementUtilities : NonCopyableClass
{
private:
    ILinearElementUtilities() {}

public:
    //! Query for a CorridorPortionElement associated with a given ILinearElement. It also returns the
    //! significant point associated with such relationship.
    ROADRAILPHYSICAL_EXPORT static CorridorPortionElementCPtr QueryRelatedCorridorPortion(LinearReferencing::ILinearElementCR linearElement, 
        Dgn::DgnElementId& significantPointDefId);

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetRelatedCorridorPortion(LinearReferencing::ILinearElementCR linearElement, 
        CorridorPortionElementCR corridorPortion, SignificantPointDefinitionCR significantPointDef);
}; // ILinearElementUtilities


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

//=================================================================================
//! ElementHandler for Pathway Separation Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwaySeparationElementHandler : CorridorPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwaySeparationElement, PathwaySeparationElement, PathwaySeparationElementHandler, CorridorPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // PathwaySeparationElementHandler

//=================================================================================
//! ElementHandler for Pathway Separation Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwaySeparationHandler : PathwaySeparationElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwaySeparation, PathwaySeparation, PathwaySeparationHandler, PathwaySeparationElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // PathwaySeparationHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE