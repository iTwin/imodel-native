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

protected:
    explicit RoadwayStandardsModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadwayStandardsModel)

    static RoadwayStandardsModelPtr Create(CreateParams const& params) { return new RoadwayStandardsModel(params); }
}; // RoadwayStandardsModel

//=======================================================================================
//! Represents a standardized set of Design-speed tables 
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedStandards : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedStandards, Dgn::DefinitionElement);
friend struct RoadDesignSpeedStandardsHandler;

protected:
    ROADRAILPHYSICAL_EXPORT explicit RoadDesignSpeedStandards(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadDesignSpeedStandards)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(RoadDesignSpeedStandards)

    ROADRAILPHYSICAL_EXPORT static RoadDesignSpeedStandardsPtr Create(RoadwayStandardsModelCR model);

    ROADRAILPHYSICAL_EXPORT RoadDesignSpeedStandardsCPtr Insert(RoadDesignSpeedDefinitionTableModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // RoadDesignSpeedStandards

//=======================================================================================
//! Model for Design-Speed Definition Table elements.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinitionTableModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinitionTableModel, Dgn::DefinitionModel);
friend struct RoadDesignSpeedDefinitionTableModelHandler;

protected:
    explicit RoadDesignSpeedDefinitionTableModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadDesignSpeedDefinitionTableModel)

    static RoadDesignSpeedDefinitionTableModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< RoadDesignSpeedDefinitionTableModel >(id); }
    static RoadDesignSpeedDefinitionTableModelPtr Create(CreateParams const& params) { return new RoadDesignSpeedDefinitionTableModel(params); }
}; // RoadDesignSpeedDefinitionTableModel

//=======================================================================================
//! Represents a Design-speed table
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinitionTable : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinitionTable, Dgn::DefinitionElement);
friend struct RoadDesignSpeedDefinitionTableHandler;

protected:
    ROADRAILPHYSICAL_EXPORT explicit RoadDesignSpeedDefinitionTable(CreateParams const& params);

    ROADRAILPHYSICAL_EXPORT explicit RoadDesignSpeedDefinitionTable(CreateParams const& params, Dgn::UnitSystem unitSystem);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadDesignSpeedDefinitionTable)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(RoadDesignSpeedDefinitionTable)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value);
    ROADRAILPHYSICAL_EXPORT static RoadDesignSpeedDefinitionTablePtr Create(RoadDesignSpeedDefinitionTableModelCR model, Utf8StringCR code, Dgn::UnitSystem unitSystem);

    Dgn::UnitSystem GetUnitSystem() const { return static_cast<Dgn::UnitSystem>(GetPropertyValueInt32("UnitSystem")); }

    ROADRAILPHYSICAL_EXPORT RoadDesignSpeedDefinitionTableCPtr Insert(RoadDesignSpeedDefinitionModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // RoadDesignSpeedDefinitionTable

//=======================================================================================
//! Model for Design-Speed elements.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinitionModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinitionModel, Dgn::DefinitionModel);
friend struct RoadDesignSpeedDefinitionModelHandler;

protected:
    explicit RoadDesignSpeedDefinitionModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadDesignSpeedDefinitionModel)
    RoadDesignSpeedDefinitionTableCPtr GetModeledDesignSpeedDefinitionTable() const { return RoadDesignSpeedDefinitionTable::Get(GetDgnDb(), GetModeledElementId()); }

    static RoadDesignSpeedDefinitionModelPtr Create(CreateParams const& params) { return new RoadDesignSpeedDefinitionModel(params); }
}; // RoadDesignSpeedDefinitionModel

//=======================================================================================
//! Represents an entry in a Design-speed table
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinition : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinition, Dgn::DefinitionElement);
friend struct RoadDesignSpeedDefinitionHandler;

protected:
    
    ROADRAILPHYSICAL_EXPORT explicit RoadDesignSpeedDefinition(CreateParams const& params);
    ROADRAILPHYSICAL_EXPORT explicit RoadDesignSpeedDefinition(CreateParams const& params, double designSpeed, double sideSlopeFrictionFactor, double maxTransitionRadius, double desirableSpiralLength,
        double type1CrestVertical, double type3SagVertical, double crestVerticalPsd, double averageRunningSpeed, double maximumRelativeGradient,
        double crestK_SSD, double sagK_SSD, double ignoreLength);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadDesignSpeedDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadDesignSpeedDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, double speed, Dgn::UnitSystem unitSystem);
    ROADRAILPHYSICAL_EXPORT static Utf8String GetDefaultUserLabel(double speed, Dgn::UnitSystem unitSystem);
    ROADRAILPHYSICAL_EXPORT static RoadDesignSpeedDefinitionPtr Create(RoadDesignSpeedDefinitionModelCR model, double designSpeed, double sideSlopeFrictionFactor, 
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
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeed : Dgn::InformationRecordElement, LinearReferencing::ILinearlyLocatedAttribution, LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeed, Dgn::InformationRecordElement);
    friend struct RoadDesignSpeedHandler;

protected:
    //! @private
    explicit RoadDesignSpeed(CreateParams const& params);

    //! @private
    explicit RoadDesignSpeed(CreateParams const& params, RoadDesignSpeedDefinitionCR designSpeedDef, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadDesignSpeed)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadDesignSpeed)

    Dgn::DgnElementId GetDesignSpeedDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>("RoadDesignSpeedDefinition"); }
    ROADRAILPHYSICAL_EXPORT void SetDesignSpeedDefinition(RoadDesignSpeedDefinitionCR designSpeedDef);

    ROADRAILPHYSICAL_EXPORT static RoadDesignSpeedPtr Create(RoadwayCR roadway, RoadDesignSpeedDefinitionCR designSpeedDef, double fromDistanceAlong, double toDistanceAlong);
}; // RoadDesignSpeed


//=======================================================================================
//! The ModelHandler for RoadwayStandards Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadwayStandardsModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadwayStandardsModel, RoadwayStandardsModel, RoadwayStandardsModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadwayStandardsModelHandler

//=======================================================================================
//! ElementHandler for RoadDesignSpeedStandards Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedStandardsHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedStandards, RoadDesignSpeedStandards, RoadDesignSpeedStandardsHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedStandardsHandler

//=======================================================================================
//! The ModelHandler for RoadDesignSpeedStandards Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinitionTableModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinitionTableModel, RoadDesignSpeedDefinitionTableModel, RoadDesignSpeedDefinitionTableModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedDefinitionTableModelHandler

//=======================================================================================
//! ElementHandler for RoadDesignSpeedDefinitionTable Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinitionTableHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinitionTable, RoadDesignSpeedDefinitionTable, RoadDesignSpeedDefinitionTableHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedDefinitionTableHandler

//=======================================================================================
//! The ModelHandler for RoadDesignSpeedDefinition Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinitionModel, RoadDesignSpeedDefinitionModel, RoadDesignSpeedDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedDefinitionModelHandler

struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedDefinitionHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeedDefinition, RoadDesignSpeedDefinition, RoadDesignSpeedDefinitionHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedDefinitionHandler

//=================================================================================
//! ElementHandler for RoadDesignSpeed Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedHandler : Dgn::dgn_ElementHandler::InformationRecord
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeed, RoadDesignSpeed, RoadDesignSpeedHandler, Dgn::dgn_ElementHandler::InformationRecord, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE