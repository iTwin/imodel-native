/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TypicalSection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

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

    static TypicalSectionPortionPtr Create(Dgn::DefinitionModelCR model, Utf8CP label);
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
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static RoadTravelwayDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT RoadTravelwayDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // RoadTravelwayDefinition

//=======================================================================================
//! EndCondition Definition Elements.
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
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static EndConditionDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT EndConditionDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // EndConditionDefinition

//=======================================================================================
//! Buffer Definition Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BufferDefinition : TypicalSectionPortionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_BufferDefinition, TypicalSectionPortionElement);
friend struct BufferDefinitionHandler;

protected:
    //! @private
    explicit BufferDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(BufferDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(BufferDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static BufferDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT BufferDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // BufferDefinition

//=======================================================================================
//! Base class for Travelway Definition Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSection : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSection, Dgn::DefinitionElement);
friend struct OverallTypicalSectionHandler;

protected:
    //! @private
    explicit OverallTypicalSection(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(OverallTypicalSection)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(OverallTypicalSection)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static OverallTypicalSectionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);
}; // OverallTypicalSection

//=======================================================================================
//! Base class for TypicalSection Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionComponentElement : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionComponentElement, Dgn::DefinitionElement);
friend struct TypicalSectionComponentElementHandler;

protected:
    //! @private
    explicit TypicalSectionComponentElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionComponentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionComponentElement)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(TypicalSectionPortionBreakDownModelCR scope, Utf8StringCR value);
}; // TypicalSectionComponentElement

//=======================================================================================
//! Buffer Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BufferComponent : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_BufferComponent, TypicalSectionComponentElement);
friend struct BufferComponentHandler;

protected:
    //! @private
    explicit BufferComponent(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(BufferComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(BufferComponent)

    ROADRAILPHYSICAL_EXPORT static BufferComponentPtr Create(TypicalSectionPortionBreakDownModelCR model, Utf8StringCR code);
}; // BufferComponent

//=======================================================================================
//! Travelway Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayComponent : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayComponent, TypicalSectionComponentElement);
friend struct TravelwayComponentHandler;

protected:
    //! @private
    explicit TravelwayComponent(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwayComponent)

    ROADRAILPHYSICAL_EXPORT static TravelwayComponentPtr Create(TypicalSectionPortionBreakDownModelCR model, Utf8StringCR code);
}; // TravelwayComponent

//=======================================================================================
//! EndCondition Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EndConditionComponent : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_EndConditionComponent, TypicalSectionComponentElement);
friend struct EndConditionComponentHandler;

protected:
    //! @private
    explicit EndConditionComponent(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(EndConditionComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(EndConditionComponent)

    ROADRAILPHYSICAL_EXPORT static EndConditionComponentPtr Create(TypicalSectionPortionBreakDownModelCR model, Utf8StringCR code);
}; // EndConditionComponent

//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSection, OverallTypicalSection, OverallTypicalSectionHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // OverallTypicalSectionHandler

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

//=================================================================================
//! ElementHandler for Travelway Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionElementHandler : TypicalSectionPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionElement, TravelwayDefinitionElement, TravelwayDefinitionElementHandler, TypicalSectionPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayDefinitionElementHandler

//=================================================================================
//! ElementHandler for Road-specific Travelway Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadTravelwayDefinitionHandler : TravelwayDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadTravelwayDefinition, RoadTravelwayDefinition, RoadTravelwayDefinitionHandler, TravelwayDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadTravelwayDefinitionHandler

//=================================================================================
//! ElementHandler for EndCondition Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EndConditionDefinitionHandler : TypicalSectionPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_EndConditionDefinition, EndConditionDefinition, EndConditionDefinitionHandler, TypicalSectionPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // EndConditionDefinitionHandler

//=================================================================================
//! ElementHandler for Buffer Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BufferDefinitionHandler : TypicalSectionPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_BufferDefinition, BufferDefinition, BufferDefinitionHandler, TypicalSectionPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // BufferDefinitionHandler

//=======================================================================================
//! The ModelHandler for TypicalSectionPortionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionBreakDownModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionBreakDownModel, TypicalSectionPortionBreakDownModel, TypicalSectionPortionBreakDownModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPortionBreakDownModelHandler

//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionComponentElementHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionComponentElement, TypicalSectionComponentElement, TypicalSectionComponentElementHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionComponentElementHandler

//=================================================================================
//! ElementHandler for EndCondition Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EndConditionComponentHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_EndConditionComponent, EndConditionComponent, EndConditionComponentHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // EndConditionComponentHandler

//=================================================================================
//! ElementHandler for Buffer Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BufferComponentHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_BufferComponent, BufferComponent, BufferComponentHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // BufferComponentHandler

//=================================================================================
//! ElementHandler for Travelway Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayComponentHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayComponent, TravelwayComponent, TravelwayComponentHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayComponentHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE