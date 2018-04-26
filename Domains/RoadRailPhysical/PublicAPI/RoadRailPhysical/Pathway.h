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

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scope, Utf8StringCR value);
    ROADRAILPHYSICAL_EXPORT static CorridorCPtr QueryByCode(Dgn::PhysicalModelCR model, Utf8StringCR code);    
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(CorridorCR corridor, Dgn::GeometrySourceCR representedBy);

    ROADRAILPHYSICAL_EXPORT bool QueryIsRepresentedBy(Dgn::GeometrySourceCR) const;
    ROADRAILPHYSICAL_EXPORT bvector<Dgn::DgnElementId> QueryOrderedPathwayIds() const;
    ROADRAILPHYSICAL_EXPORT bset<PathwaySeparationInfo> QueryPathwaySeparationInfos() const;

    ROADRAILPHYSICAL_EXPORT static CorridorPtr Create(Dgn::PhysicalModelR model);
}; // Corridor

//=======================================================================================
//! Base class for Pathways and their Separations
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct CorridorPortionElement : Dgn::PhysicalElement, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CorridorPortionElement, Dgn::PhysicalElement);
    friend struct CorridorPortionElementHandler;

protected:
    //! @private
    explicit CorridorPortionElement(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }
    virtual PathwayElementCP _ToPathway() const { return nullptr; }
    virtual PathwaySeparationElementCP _ToPathwaySeparation() const { return nullptr; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CorridorPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(CorridorPortionElement)

    PathwayElementCP ToPathway() const { return _ToPathway(); }
    PathwaySeparationElementCP ToPathwaySeparation() const { return _ToPathwaySeparation(); }
}; // CorridorPortionElement

//=======================================================================================
//! Base class for Road and Rail range of physical segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayElement : CorridorPortionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, CorridorPortionElement);
    friend struct PathwayElementHandler;

protected:
    //! @private
    explicit PathwayElement(CreateParams const& params) : T_Super(params) {}    

    virtual RailwayCP _ToRailway() const { return nullptr; }
    virtual RoadwayCP _ToRoadway() const { return nullptr; }

public:
    enum class Order: int32_t { LeftMost = 0, RightMost = 100 };

    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(PathwayElement)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scope, Utf8StringCR value);
    ROADRAILPHYSICAL_EXPORT static PathwayElementCPtr QueryByCode(Dgn::PhysicalModelCR model, Utf8StringCR code);    

    RailwayCP ToRailway() const { return _ToRailway(); }
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

    virtual RoadwayCP _ToRoadway() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Roadway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(Roadway)
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

    virtual RailwayCP _ToRailway() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Railway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(Railway)

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

    Dgn::DgnElementId GetPathwayIdLeftSide() const { return GetPropertyValueId<Dgn::DgnElementId>("PathwayLeftSide"); }
    Dgn::DgnElementId GetPathwayIdRightSide() const { return GetPropertyValueId<Dgn::DgnElementId>("PathwayRightSide"); }

    void SetPathwayLeftSide(PathwayElementCR leftPathway) { SetPropertyValue("PathwayLeftSide", leftPathway.GetElementId()); }
    void SetPathwayRightSide(PathwayElementCR rightPathway) { SetPropertyValue("PathwayRightSide", rightPathway.GetElementId()); }
}; // PathwaySeparationElement

//=======================================================================================
//! A TravelSeparationPortionElement specified using linear-referencing methods.
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
public:
    ROADRAILPHYSICAL_EXPORT static CorridorPortionElementCPtr QueryRelatedCorridorPortion(LinearReferencing::ILinearElementCR linearElement, 
        Dgn::DgnElementId& significantPointDefId);

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetRelatedCorridorPortion(LinearReferencing::ILinearElementCR linearElement, 
        CorridorPortionElementCR corridorPortion, SignificantPointDefinitionCR significantPointDef);
}; // ILinearElementUtilities

//=======================================================================================
//! Model to contain and manage Road&Rail physical elements
//=======================================================================================
struct RoadRailPhysicalModel : Dgn::PhysicalModel
{
    DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_RoadRailPhysicalModel, Dgn::PhysicalModel);
    friend struct RoadRailPhysicalModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(RoadRailPhysicalModel::T_Super::CreateParams);

    //! Parameters to create a new instance of a RoadRailPhysicalModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, RoadRailPhysicalModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit RoadRailPhysicalModel(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadRailPhysicalModel)

    //! Query for the Parent Subject of this model.
    //! @param[in] model The PhysicalModel who's parent is being queried for.
    //! @return The Subject of the \p model
    ROADRAILPHYSICAL_EXPORT Dgn::SubjectCPtr GetParentSubject() const;

    //! Query for the physical model
    //! @param[in] parentSubject The parent subject of the physical model with \p modelName
    //! @return The PhysicalModel belonging to the \p parentSubject
    ROADRAILPHYSICAL_EXPORT static RoadRailPhysicalModelPtr Query(Dgn::SubjectCR parentSubject);

    //! @private
    static RoadRailPhysicalModelPtr Create(CreateParams const& params) { return new RoadRailPhysicalModel(params); }
    
    static RoadRailPhysicalModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< RoadRailPhysicalModel >(id); }
}; // RoadRailPhysicalModel


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

//=======================================================================================
//! The ModelHandler for RoadRailPhysicalModel
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadRailPhysicalModelHandler : Dgn::dgn_ModelHandler::Physical
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadRailPhysicalModel, RoadRailPhysicalModel, RoadRailPhysicalModelHandler, Dgn::dgn_ModelHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // RoadRailPhysicalModelHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE