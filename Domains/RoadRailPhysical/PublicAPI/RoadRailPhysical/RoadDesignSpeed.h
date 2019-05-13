/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "Pathway.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Model for Roadway Standards definition elements.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayStandardsModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_RoadwayStandardsModel, Dgn::DefinitionModel);
friend struct RoadwayStandardsModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(RoadwayStandardsModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an RoadwayStandardsModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, RoadwayStandardsModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit RoadwayStandardsModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadwayStandardsModel)

    //! @private
    static RoadwayStandardsModelPtr Create(CreateParams const& params) { return new RoadwayStandardsModel(params); }
    //! @private
    static RoadwayStandardsModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< RoadwayStandardsModel >(id); }
    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadwayStandardsModelPtr Query(Dgn::SubjectCR parentSubject);
}; // RoadwayStandardsModel

//=======================================================================================
//! Model for Railway Standards definition elements.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RailwayStandardsModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_RailwayStandardsModel, Dgn::DefinitionModel);
friend struct RailwayStandardsModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(RailwayStandardsModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an RailwayStandardsModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, RailwayStandardsModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit RailwayStandardsModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RailwayStandardsModel)

    //! @private
    static RailwayStandardsModelPtr Create(CreateParams const& params) { return new RailwayStandardsModel(params); }
    //! @private
    static RailwayStandardsModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< RailwayStandardsModel >(id); }
    //! @private
    ROADRAILPHYSICAL_EXPORT static RailwayStandardsModelPtr Query(Dgn::SubjectCR parentSubject);
}; // RailwayStandardsModel

//=======================================================================================
//! Standardized design-speed definition in the context of a Subject.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinition : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinition, Dgn::DefinitionElement);
friend struct DesignSpeedDefinitionHandler;

public:
    enum class UnitSystem : int16_t { SI = 0, Imperial = 1 };

protected:
    //! @private
    ROADRAILPHYSICAL_EXPORT explicit DesignSpeedDefinition(CreateParams const& params);
    //! @private
    ROADRAILPHYSICAL_EXPORT explicit DesignSpeedDefinition(CreateParams const& params, double designSpeed, UnitSystem unitSystem);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeedDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(DesignSpeedDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, double speed, UnitSystem unitSystem);
    ROADRAILPHYSICAL_EXPORT static DesignSpeedDefinitionCPtr QueryByCode(Dgn::DefinitionModelCR model, double speed, UnitSystem unitSystem);

    double GetDesignSpeed() const { return GetPropertyValueDouble("DesignSpeed"); }
    ROADRAILPHYSICAL_EXPORT UnitSystem GetUnitSystem() const;

    ROADRAILPHYSICAL_EXPORT static DesignSpeedDefinitionPtr Create(Dgn::DefinitionModelCR model, double designSpeed, UnitSystem unitSystem);
}; // DesignSpeedDefinition

//=======================================================================================
//! Linearly-located attribution on a Pathway whose value is its design-speed.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedElement : Dgn::InformationRecordElement, LinearReferencing::ILinearlyLocatedAttribution, ICorridorPortionSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedElement, Dgn::InformationRecordElement);
    friend struct DesignSpeedElementHandler;

protected:
    //! @private
    explicit DesignSpeedElement(CreateParams const& params);

    //! @private
    explicit DesignSpeedElement(CreateParams const& params, CreateFromToParams const& fromToParams);    

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }    

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeedElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(DesignSpeedElement)
}; // DesignSpeedElement

//=======================================================================================
//! Linearly-located attribution on a Pathway whose value is its design-speed.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeed : DesignSpeedElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeed, DesignSpeedElement);
    friend struct DesignSpeedHandler;

public:
    struct CreateFromToParams : ICorridorPortionSingleFromTo::CreateFromToParams
    {
    DEFINE_T_SUPER(ICorridorPortionSingleFromTo::CreateFromToParams)

    DesignSpeedDefinitionCPtr m_designSpeedDefCPtr;

    CreateFromToParams(PathwayElementCR pathway, DesignSpeedDefinitionCR designSpeedDef,
        double fromDistanceFromStart, double toDistanceFromStart) :
        m_designSpeedDefCPtr(&designSpeedDef), T_Super(pathway, fromDistanceFromStart, toDistanceFromStart)
        {
        }
    }; // CreateFromToParams

protected:
    //! @private
    explicit DesignSpeed(CreateParams const& params);

    //! @private
    explicit DesignSpeed(CreateParams const& params, CreateFromToParams const& fromToParams);    

    static bool ValidateParams(CreateFromToParams const& params)
        {
        if (!T_Super::ValidateParams(params)) return false; return params.m_designSpeedDefCPtr.IsValid() && params.m_designSpeedDefCPtr->GetElementId().IsValid();
        }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeed)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(DesignSpeed)

    Dgn::DgnElementId GetDesignSpeedDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>("Definition"); }
    //! @private
    ROADRAILPHYSICAL_EXPORT void SetDesignSpeedDefinition(DesignSpeedDefinitionCR designSpeedDef);

    //! @private
    ROADRAILPHYSICAL_EXPORT static DesignSpeedPtr Create(CreateFromToParams const& params);
}; // DesignSpeed

//=======================================================================================
//! Linearly-located attribution on a Pathway whose value is its design-speed.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedTransition : DesignSpeedElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedTransition, DesignSpeedElement);
    friend struct DesignSpeedTransitionHandler;

protected:
    //! @private
    explicit DesignSpeedTransition(CreateParams const& params);

    //! @private
    explicit DesignSpeedTransition(CreateParams const& params, CreateFromToParams const& fromToParams);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeedTransition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(DesignSpeedTransition)

    //! @private
    ROADRAILPHYSICAL_EXPORT static DesignSpeedTransitionPtr Create(CreateFromToParams const& params);
}; // DesignSpeedTransition


//__PUBLISH_SECTION_END__
//=======================================================================================
//! The ModelHandler for RoadwayStandards Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayStandardsModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadwayStandardsModel, RoadwayStandardsModel, RoadwayStandardsModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadwayStandardsModelHandler

//=======================================================================================
//! The ModelHandler for RailwayStandards Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RailwayStandardsModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RailwayStandardsModel, RailwayStandardsModel, RailwayStandardsModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RailwayStandardsModelHandler

//=======================================================================================
//! Handler for DesignSpeedDefinition Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinitionHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinition, DesignSpeedDefinition, DesignSpeedDefinitionHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedDefinitionHandler

//=======================================================================================
//! Handler for base DesignSpeed Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedElementHandler : Dgn::dgn_ElementHandler::InformationRecord
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedElement, DesignSpeedElement, DesignSpeedElementHandler, Dgn::dgn_ElementHandler::InformationRecord, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedElementHandler

//=======================================================================================
//! Handler for DesignSpeed Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedHandler : DesignSpeedElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeed, DesignSpeed, DesignSpeedHandler, DesignSpeedElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedElementHandler

//=======================================================================================
//! Handler for DesignSpeedTransition Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedTransitionHandler : DesignSpeedElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedTransition, DesignSpeedTransition, DesignSpeedTransitionHandler, DesignSpeedElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedTransitionHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE