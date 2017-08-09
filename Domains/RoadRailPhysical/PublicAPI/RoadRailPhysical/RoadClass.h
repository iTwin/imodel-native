/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadClass.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Represents a Road-Class table
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadClassDefinitionTable : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadClassDefinitionTable, Dgn::DefinitionElement);
friend struct RoadClassDefinitionTableHandler;

protected:
    ROADRAILPHYSICAL_EXPORT explicit RoadClassDefinitionTable(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadClassDefinitionTable)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(RoadClassDefinitionTable)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value);
    ROADRAILPHYSICAL_EXPORT static RoadClassDefinitionTablePtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT RoadClassDefinitionTableCPtr Insert(RoadClassDefinitionModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // RoadClassDefinitionTable

//=======================================================================================
//! Model for Road-Class elements.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadClassDefinitionModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_RoadClassDefinitionModel, Dgn::DefinitionModel);
friend struct RoadClassDefinitionModelHandler;

protected:
    explicit RoadClassDefinitionModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadClassDefinitionModel)
    RoadClassDefinitionTableCPtr GetModeledClassDefinitionTable() const { return RoadClassDefinitionTable::Get(GetDgnDb(), GetModeledElementId()); }

    static RoadClassDefinitionModelPtr Create(CreateParams const& params) { return new RoadClassDefinitionModel(params); }
}; // RoadClassDefinitionModel

//=======================================================================================
//! Represents an entry in a Road-Class table
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadClassDefinition : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadClassDefinition, Dgn::DefinitionElement);
friend struct RoadClassDefinitionHandler;

protected:
    
    ROADRAILPHYSICAL_EXPORT explicit RoadClassDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadClassDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadClassDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR roadClassCode);
    ROADRAILPHYSICAL_EXPORT static RoadClassDefinitionPtr Create(RoadClassDefinitionModelCR model, Utf8StringCR roadClassCode, Utf8CP roadClassLabel);
}; // RoadClassDefinition

//=======================================================================================
//! Linearly-located attribution on a Roadway whose value is its Road-Class.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadClass : Dgn::InformationRecordElement, LinearReferencing::ILinearlyLocatedAttribution, LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadClass, Dgn::InformationRecordElement);
    friend struct RoadClassHandler;

protected:
    //! @private
    explicit RoadClass(CreateParams const& params);

    //! @private
    explicit RoadClass(CreateParams const& params, RoadClassDefinitionCR classDef, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadClass)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadClass)

    Dgn::DgnElementId GetClassDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>("RoadClassDefinition"); }
    ROADRAILPHYSICAL_EXPORT void SetClassDefinition(RoadClassDefinitionCR ClassDef);

    ROADRAILPHYSICAL_EXPORT static RoadClassPtr Create(RoadwayCR roadway, RoadClassDefinitionCR ClassDef, double fromDistanceAlong, double toDistanceAlong);
}; // RoadClass


//=======================================================================================
//! ElementHandler for RoadClassDefinitionTable Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadClassDefinitionTableHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadClassDefinitionTable, RoadClassDefinitionTable, RoadClassDefinitionTableHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadClassDefinitionTableHandler

//=======================================================================================
//! The ModelHandler for RoadClassDefinition Models
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadClassDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadClassDefinitionModel, RoadClassDefinitionModel, RoadClassDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadClassDefinitionModelHandler

struct EXPORT_VTABLE_ATTRIBUTE RoadClassDefinitionHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadClassDefinition, RoadClassDefinition, RoadClassDefinitionHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // RoadClassDefinitionHandler

//=================================================================================
//! ElementHandler for RoadClass Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadClassHandler : Dgn::dgn_ElementHandler::InformationRecord
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadClass, RoadClass, RoadClassHandler, Dgn::dgn_ElementHandler::InformationRecord, ROADRAILPHYSICAL_EXPORT)
}; // RoadClassHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE