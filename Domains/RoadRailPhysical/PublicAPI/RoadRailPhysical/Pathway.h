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
//! Base class for Road and Rail range of physical segments.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayElement : Dgn::PhysicalElement, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, Dgn::PhysicalElement);
    friend struct PathwayElementHandler;

protected:
    //! @private
    explicit PathwayElement(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }

public:
    enum class TravelSide { Single = 0, Left = 1, Right = 2 };

    struct TravelPortionInfo
    {
        Dgn::DgnElementId m_elementId;
        TravelSide m_travelSide;

        TravelPortionInfo(): m_travelSide(TravelSide::Single) {}
        TravelPortionInfo(Dgn::DgnElementId elementId, TravelSide travelSide): m_elementId(elementId), m_travelSide(travelSide) {}

        Dgn::DgnElementId GetElementId() const { return m_elementId; }
        TravelSide GetTravelSide() const { return m_travelSide; }

        bool operator< (TravelPortionInfo const& right) const { return m_elementId < right.m_elementId; }
    }; // TravelPortionInfo

    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(PathwayElement)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scope, Utf8StringCR value);
    ROADRAILPHYSICAL_EXPORT static PathwayElementCPtr QueryByCode(Dgn::PhysicalModelCR model, Utf8StringCR code);    
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus AddRepresentedBy(PathwayElementCR pathway, Dgn::GeometrySourceCR representedBy);

    ROADRAILPHYSICAL_EXPORT bool QueryIsRepresentedBy(Dgn::GeometrySourceCR) const;
    ROADRAILPHYSICAL_EXPORT Dgn::DgnElementIdSet QueryPortionIds() const;
    ROADRAILPHYSICAL_EXPORT bset<TravelPortionInfo> QueryTravelPortionInfos() const;
    ROADRAILPHYSICAL_EXPORT Dgn::DgnElementId QueryTravelPortionId(TravelSide side) const;
    ROADRAILPHYSICAL_EXPORT Dgn::DgnElementId QueryTravelSeparationId() const;
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

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Roadway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(Roadway)
    ROADRAILPHYSICAL_EXPORT static RoadwayPtr Create(Dgn::PhysicalModelR model);
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

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(Railway)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(Railway)
    ROADRAILPHYSICAL_EXPORT static RailwayPtr Create(Dgn::PhysicalModelR model);
}; // Railway

//=======================================================================================
//! Physical Portion of a Pathway.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PathwayPortionElement : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PathwayPortionElement, Dgn::PhysicalElement);
    friend struct PathwayPortionElementHandler;

protected:
    //! @private
    explicit PathwayPortionElement(CreateParams const& params) : T_Super(params) {}

    virtual TravelPortionElementCP _ToTravelPortionElement() const { return nullptr; }
    virtual TravelSeparationPortionElementCP _ToTravelSeparationPortionElement() const { return nullptr; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PathwayPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(PathwayPortionElement)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scope, Utf8StringCR value);
    ROADRAILPHYSICAL_EXPORT static PathwayPortionElementCPtr QueryByCode(Dgn::PhysicalModelCR model, Utf8StringCR code);

    TravelPortionElementCP ToTravelPortionElement() const { return _ToTravelPortionElement(); }
    TravelSeparationPortionElementCP ToTravelSeparationPortionElement() const { return _ToTravelSeparationPortionElement(); }
}; // PathwayPortionElement

//=======================================================================================
//! A PathwayPortion specially constructed for continuous travel of a particular type.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct TravelPortionElement : PathwayPortionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelPortionElement, PathwayPortionElement);
    friend struct TravelPortionElementHandler;

protected:
    //! @private
    explicit TravelPortionElement(CreateParams const& params) : T_Super(params) {}

    virtual TravelPortionElementCP _ToTravelPortionElement() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelPortionElement)

    ROADRAILPHYSICAL_EXPORT PathwayElement::TravelSide QueryTravelSide() const;
}; // TravelPortionElement

//=======================================================================================
//! A PathwayPortion specially constructed to separate two portions of continuous travel.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct TravelSeparationPortionElement : PathwayPortionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelSeparationPortionElement, PathwayPortionElement);
    friend struct TravelSeparationPortionElementHandler;

protected:
    //! @private
    explicit TravelSeparationPortionElement(CreateParams const& params) : T_Super(params) {}

    virtual TravelSeparationPortionElementCP _ToTravelSeparationPortionElement() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelSeparationPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelSeparationPortionElement)
}; // TravelSeparationPortionElement

//=======================================================================================
//! A TravelPortionElement specified using linear-referencing methods.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ThruwayPortion : TravelPortionElement, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ThruwayPortion, TravelPortionElement);
    friend struct ThruwayPortionHandler;

protected:
    //! @private
    explicit ThruwayPortion(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ThruwayPortion)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(ThruwayPortion)

    ROADRAILPHYSICAL_EXPORT static ThruwayPortionPtr Create(PathwayElementCR pathway, LinearReferencing::ILinearElementCR linearElement);

    ROADRAILPHYSICAL_EXPORT ThruwayPortionCPtr Insert(PathwayElement::TravelSide side, Dgn::DgnDbStatus* status = nullptr);
}; // ThruwayPortion

//=======================================================================================
//! A TravelSeparationPortionElement specified using linear-referencing methods.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ThruwaySeparationPortion : TravelSeparationPortionElement, IMainLinearElementSource
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_ThruwaySeparationPortion, TravelSeparationPortionElement);
    friend struct ThruwaySeparationPortionHandler;

protected:
    //! @private
    explicit ThruwaySeparationPortion(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _ILinearElementSourceToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(ThruwaySeparationPortion)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(ThruwaySeparationPortion)

    ROADRAILPHYSICAL_EXPORT static ThruwaySeparationPortionPtr Create(PathwayElementCR pathway, LinearReferencing::ILinearElementCR linearElement);
}; // ThruwaySeparationPortion

//=======================================================================================
//! Utility class facilitating some operations against ILinearElements in the 
//! context of the Road/Rail discipline.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ILinearElementUtilities : NonCopyableClass
{
public:
    ROADRAILPHYSICAL_EXPORT static PathwayPortionElementCPtr QueryRelatedPathwayPortion(LinearReferencing::ILinearElementCR linearElement, 
        Dgn::DgnElementId& significantPointDefId);

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetRelatedPathwayPortion(LinearReferencing::ILinearElementCR linearElement, 
        PathwayPortionElementCR pathwayPortion, SignificantPointDefinitionCR significantPointDef);
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
//! ElementHandler for Pathway Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwayElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwayElement, PathwayElement, PathwayElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
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
//! ElementHandler for Pathway Portion Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PathwayPortionElementHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PathwayPortionElement, PathwayPortionElement, PathwayPortionElementHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // PathwayPortionElementHandler

//=================================================================================
//! ElementHandler for TravelPortionElement Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelPortionElementHandler : PathwayPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelPortionElement, TravelPortionElement, TravelPortionElementHandler, PathwayPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelPortionElementHandler

//=================================================================================
//! ElementHandler for TravelPortionElement Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelSeparationPortionElementHandler : PathwayPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelSeparationPortionElement, TravelSeparationPortionElement, TravelSeparationPortionElementHandler, PathwayPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelSeparationPortionElementHandler

//=================================================================================
//! ElementHandler for ThruwayPortion Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThruwayPortionHandler : TravelPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ThruwayPortion, ThruwayPortion, ThruwayPortionHandler, TravelPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ThruwayPortionHandler

//=================================================================================
//! ElementHandler for ThruwayPortion Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThruwaySeparationPortionHandler : TravelSeparationPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_ThruwaySeparationPortion, ThruwaySeparationPortion, ThruwaySeparationPortionHandler, TravelSeparationPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // ThruwayPortionHandler

//=======================================================================================
//! The ModelHandler for RoadRailPhysicalModel
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadRailPhysicalModelHandler : Dgn::dgn_ModelHandler::Physical
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadRailPhysicalModel, RoadRailPhysicalModel, RoadRailPhysicalModelHandler, Dgn::dgn_ModelHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // RoadRailPhysicalModelHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE