/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TypicalSection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class for TypicalSection Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionElement : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionElement, Dgn::DefinitionElement);
friend struct TypicalSectionPortionElementHandler;

protected:
    //! @private
    explicit TypicalSectionPortionElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPortionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionPortionElement)
}; // TypicalSectionPortionElement

//=======================================================================================
//! Model for TypicalSection elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionModel, Dgn::DefinitionModel);
friend struct TypicalSectionModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(TypicalSectionModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an TypicalSectionModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, TypicalSectionModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit TypicalSectionModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionModel)

    static TypicalSectionModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< TypicalSectionModel >(id); }    
    static TypicalSectionModelPtr Create(CreateParams const& params) { return new TypicalSectionModel(params); }
}; // TypicalSectionModel

//=======================================================================================
//! TypicalSection Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortion : TypicalSectionPortionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortion, TypicalSectionPortionElement);
friend struct TypicalSectionPortionHandler;

protected:
    //! @private
    explicit TypicalSectionPortion(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPortion)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TypicalSectionPortion)

    static TypicalSectionPortionPtr Create(TypicalSectionModelCR model, Utf8CP label);
}; // TypicalSectionPortion

//=======================================================================================
//! Model breaking-down a TypicalSectionPortion elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionBreakDownModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionBreakDownModel, Dgn::DefinitionModel);
friend struct TypicalSectionPortionBreakDownModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(TypicalSectionPortionBreakDownModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an TypicalSectionPortionBreakDownModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, TypicalSectionPortionBreakDownModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit TypicalSectionPortionBreakDownModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPortionBreakDownModel)

    static TypicalSectionPortionBreakDownModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< TypicalSectionPortionBreakDownModel >(id); }
    static TypicalSectionPortionBreakDownModelPtr Create(CreateParams const& params) { return new TypicalSectionPortionBreakDownModel(params); }
}; // TypicalSectionPortionBreakDownModel

//=======================================================================================
//! Model for Travelway Definition elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionModel, Dgn::DefinitionModel);
friend struct TravelwayDefinitionModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(TravelwayDefinitionModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an TravelwayDefinitionModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, TravelwayDefinitionModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit TravelwayDefinitionModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayDefinitionModel)

    ROADRAILPHYSICAL_EXPORT static TravelwayDefinitionModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< TravelwayDefinitionModel >(id); }
    ROADRAILPHYSICAL_EXPORT static TravelwayDefinitionModelPtr Create(CreateParams const& params) { return new TravelwayDefinitionModel(params); }
}; // TravelwayDefinitionModel

//=======================================================================================
//! Base class for Travelway Definition Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionElement : TypicalSectionPortionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionElement, TypicalSectionPortionElement);
friend struct TravelwayDefinitionElementHandler;

protected:
    //! @private
    explicit TravelwayDefinitionElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayDefinitionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwayDefinitionElement)
}; // TravelwayDefinitionElement

//=======================================================================================
//! Base class for Travelway Definition Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadTravelwayDefinition : TravelwayDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadTravelwayDefinition, TravelwayDefinitionElement);
friend struct RoadTravelwayDefinitionHandler;

protected:
    //! @private
    explicit RoadTravelwayDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadTravelwayDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(RoadTravelwayDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static RoadTravelwayDefinitionPtr Create(TravelwayDefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT RoadTravelwayDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // RoadTravelwayDefinition

//=======================================================================================
//! Model for Travelway Definition elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EndConditionDefinitionModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_EndConditionDefinitionModel, Dgn::DefinitionModel);
friend struct EndConditionDefinitionModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(EndConditionDefinitionModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an EndConditionDefinitionModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, EndConditionDefinitionModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit EndConditionDefinitionModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(EndConditionDefinitionModel)

    ROADRAILPHYSICAL_EXPORT static EndConditionDefinitionModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< EndConditionDefinitionModel >(id); }
    ROADRAILPHYSICAL_EXPORT static EndConditionDefinitionModelPtr Create(CreateParams const& params) { return new EndConditionDefinitionModel(params); }
}; // EndConditionDefinitionModel

//=======================================================================================
//! Base class for Travelway Definition Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EndConditionDefinition : TypicalSectionPortionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_EndConditionDefinition, TypicalSectionPortionElement);
friend struct EndConditionDefinitionHandler;

protected:
    //! @private
    explicit EndConditionDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(EndConditionDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(EndConditionDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static EndConditionDefinitionPtr Create(EndConditionDefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT EndConditionDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // EndConditionDefinition


//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionElementHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionElement, TypicalSectionPortionElement, TypicalSectionPortionElementHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPortionElementHandler

//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionHandler : TypicalSectionPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortion, TypicalSectionPortion, TypicalSectionPortionHandler, TypicalSectionPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPortionElementHandler

//=======================================================================================
//! The ModelHandler for TravelwayDefinitionModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionModel, TravelwayDefinitionModel, TravelwayDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayDefinitionModelHandler

//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionElementHandler : TypicalSectionPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionElement, TravelwayDefinitionElement, TravelwayDefinitionElementHandler, TypicalSectionPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayDefinitionElementHandler

//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadTravelwayDefinitionHandler : TravelwayDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadTravelwayDefinition, RoadTravelwayDefinition, RoadTravelwayDefinitionHandler, TravelwayDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadTravelwayDefinitionHandler

//=======================================================================================
//! The ModelHandler for EndConditionDefinitionModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EndConditionDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_EndConditionDefinitionModel, EndConditionDefinitionModel, EndConditionDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // EndConditionDefinitionModelHandler

//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EndConditionDefinitionHandler : TypicalSectionPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_EndConditionDefinition, EndConditionDefinition, EndConditionDefinitionHandler, TypicalSectionPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // EndConditionDefinitionHandler

//=======================================================================================
//! The ModelHandler for TypicalSectionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionModel, TypicalSectionModel, TypicalSectionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionModelHandler

//=======================================================================================
//! The ModelHandler for TypicalSectionPortionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionBreakDownModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionBreakDownModel, TypicalSectionPortionBreakDownModel, TypicalSectionPortionBreakDownModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPortionBreakDownModelHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE