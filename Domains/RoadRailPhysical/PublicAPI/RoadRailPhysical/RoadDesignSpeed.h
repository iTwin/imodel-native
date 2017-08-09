#pragma once
/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadDesignSpeed.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Model for Design-Speed Definition Table elements.
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

    static RoadwayStandardsModelPtr Create(CreateParams const& params) { return new RoadwayStandardsModel(params); }
    static RoadwayStandardsModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< RoadwayStandardsModel >(id); }
}; // RoadwayStandardsModel

//=======================================================================================
//! Represents a Design-speed table
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinitionTable : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinitionTable, Dgn::DefinitionElement);
friend struct DesignSpeedDefinitionTableHandler;

protected:
    ROADRAILPHYSICAL_EXPORT explicit DesignSpeedDefinitionTable(CreateParams const& params);

    ROADRAILPHYSICAL_EXPORT explicit DesignSpeedDefinitionTable(CreateParams const& params, Dgn::UnitSystem unitSystem);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeedDefinitionTable)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(DesignSpeedDefinitionTable)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value);
    ROADRAILPHYSICAL_EXPORT static DesignSpeedDefinitionTablePtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code, Dgn::UnitSystem unitSystem);

    Dgn::UnitSystem GetUnitSystem() const { return static_cast<Dgn::UnitSystem>(GetPropertyValueInt32("UnitSystem")); }

    ROADRAILPHYSICAL_EXPORT DesignSpeedDefinitionTableCPtr Insert(DesignSpeedDefinitionModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // DesignSpeedDefinitionTable

//=======================================================================================
//! Model for Design-Speed elements.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinitionModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinitionModel, Dgn::DefinitionModel);
friend struct DesignSpeedDefinitionModelHandler;

protected:
    explicit DesignSpeedDefinitionModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeedDefinitionModel)
    DesignSpeedDefinitionTableCPtr GetModeledDesignSpeedDefinitionTable() const { return DesignSpeedDefinitionTable::Get(GetDgnDb(), GetModeledElementId()); }

    static DesignSpeedDefinitionModelPtr Create(CreateParams const& params) { return new DesignSpeedDefinitionModel(params); }
}; // DesignSpeedDefinitionModel

//=======================================================================================
//! Base class for entries in a Design-speed table
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinitionElement : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinitionElement, Dgn::DefinitionElement);
friend struct DesignSpeedDefinitionElementHandler;

protected:
    
    ROADRAILPHYSICAL_EXPORT explicit DesignSpeedDefinitionElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeedDefinitionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(DesignSpeedDefinitionElement)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, double speed, Dgn::UnitSystem unitSystem);
    ROADRAILPHYSICAL_EXPORT static Utf8String GetDefaultUserLabel(double speed, Dgn::UnitSystem unitSystem);
}; // DesignSpeedDefinitionElement

//=======================================================================================
//! Represents an entry in a Design-speed table
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinition : DesignSpeedDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinition, DesignSpeedDefinitionElement);
friend struct RoadDesignSpeedDefinitionHandler;

protected:
    
    ROADRAILPHYSICAL_EXPORT explicit RoadDesignSpeedDefinition(CreateParams const& params);
    ROADRAILPHYSICAL_EXPORT explicit RoadDesignSpeedDefinition(CreateParams const& params, double designSpeed, double sideSlopeFrictionFactor, double maxTransitionRadius, double desirableSpiralLength,
        double type1CrestVertical, double type3SagVertical, double crestVerticalPsd, double averageRunningSpeed, double maximumRelativeGradient,
        double crestK_SSD, double sagK_SSD, double ignoreLength);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadDesignSpeedDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadDesignSpeedDefinition)

    ROADRAILPHYSICAL_EXPORT static RoadDesignSpeedDefinitionPtr Create(DesignSpeedDefinitionModelCR model, double designSpeed, double sideSlopeFrictionFactor, 
        double maxTransitionRadius, double desirableSpiralLength, double type1CrestVertical, double type3SagVertical, double crestVerticalPsd, double averageRunningSpeed, 
        double maximumRelativeGradient, double crestK_SSD, double sagK_SSD, double ignoreLength);
    
    double GetDesignSpeed() const { return GetPropertyValueDouble("DesignSpeed"); }
    double GetSideSlopeFrictionFactor() const { return GetPropertyValueDouble("SideSlopeFrictionFactor"); }
    double GetMaxTransitionRadius() const { return GetPropertyValueDouble("MaxTransitionRadius"); }
    double GetDesirableSpiralLength() const { return GetPropertyValueDouble("DesirableSpiralLength"); }
    double GetType1CrestVertical() const { return GetPropertyValueDouble("Type1CrestVertical"); }
    double GetType3SagVertical() const { return GetPropertyValueDouble("Type3SagVertical"); }
    double GetCrestVerticalPsd() const { return GetPropertyValueDouble("CrestVerticalPsd"); }
    double GetAverageRunningSpeed() const { return GetPropertyValueDouble("AverageRunningSpeed"); }
    double GetMaximumRelativeGradient() const { return GetPropertyValueDouble("MaximumRelativeGradient"); }
    double GetCrestK_SSD() const { return GetPropertyValueDouble("CrestK_SSD"); }
    double GetSagK_SSD() const { return GetPropertyValueDouble("SagK_SSD"); }
    double GetIgnoreLength() const { return GetPropertyValueDouble("IgnoreLength"); }
}; // RoadDesignSpeedDefinition

//=======================================================================================
//! Linearly-located attribution on a Roadway whose value is its design-speed.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeed : Dgn::InformationRecordElement, LinearReferencing::ILinearlyLocatedAttribution, LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeed, Dgn::InformationRecordElement);
    friend struct DesignSpeedHandler;

protected:
    //! @private
    explicit DesignSpeed(CreateParams const& params);

    //! @private
    explicit DesignSpeed(CreateParams const& params, DesignSpeedDefinitionElementCR designSpeedDef, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeed)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(DesignSpeed)

    Dgn::DgnElementId GetDesignSpeedDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>("DesignSpeedDefinition"); }
    ROADRAILPHYSICAL_EXPORT void SetDesignSpeedDefinition(DesignSpeedDefinitionElementCR designSpeedDef);

    ROADRAILPHYSICAL_EXPORT static DesignSpeedPtr Create(PathwayElementCR pathway, DesignSpeedDefinitionElementCR designSpeedDef, double fromDistanceAlong, double toDistanceAlong);
}; // RoadDesignSpeed


//=======================================================================================
//! The ModelHandler for RoadwayStandards Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayStandardsModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadwayStandardsModel, RoadwayStandardsModel, RoadwayStandardsModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadwayStandardsModelHandler

//=======================================================================================
//! ElementHandler for RoadDesignSpeedDefinitionTable Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinitionTableHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinitionTable, DesignSpeedDefinitionTable, DesignSpeedDefinitionTableHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedDefinitionTableHandler

//=======================================================================================
//! The ModelHandler for RoadDesignSpeedDefinition Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinitionModel, DesignSpeedDefinitionModel, DesignSpeedDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedDefinitionModelHandler

struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinitionElementHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinitionElement, DesignSpeedDefinitionElement, DesignSpeedDefinitionElementHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedDefinitionElementHandler

struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinitionHandler : DesignSpeedDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinition, RoadDesignSpeedDefinition, RoadDesignSpeedDefinitionHandler, DesignSpeedDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedDefinitionHandler

//=================================================================================
//! ElementHandler for DesignSpeed Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedHandler : Dgn::dgn_ElementHandler::InformationRecord
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeed, DesignSpeed, DesignSpeedHandler, Dgn::dgn_ElementHandler::InformationRecord, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE