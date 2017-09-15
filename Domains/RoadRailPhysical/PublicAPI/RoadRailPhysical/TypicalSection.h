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
//! Base class representing the definition of a portion of overall Typical-sections of a Pathway.
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
//! Model breaking-down a TypicalSectionPortion elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionBreakDownModel : Dgn::GeometricModel2d
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionBreakDownModel, Dgn::GeometricModel2d);
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
//! Base class for definitions of Travelways.
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
//! Travelway definition for Roadways.
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
//! Definition for TravelwaySide elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideDefinition : TypicalSectionPortionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideDefinition, TypicalSectionPortionElement);
friend struct TravelwaySideDefinitionHandler;

protected:
    //! @private
    explicit TravelwaySideDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySideDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(TravelwaySideDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static TravelwaySideDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT TravelwaySideDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // EndConditionDefinition

//=======================================================================================
//! Definition for TravelwayStructure Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureDefinition : TypicalSectionPortionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureDefinition, TypicalSectionPortionElement);
friend struct TravelwayStructureDefinitionHandler;

protected:
    //! @private
    explicit TravelwayStructureDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayStructureDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(TravelwayStructureDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static TravelwayStructureDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT TravelwayStructureDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // BufferDefinition

//=======================================================================================
//! Base class for Travelway Definition Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSection : Dgn::TemplateRecipe2d
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSection, Dgn::TemplateRecipe2d);
friend struct OverallTypicalSectionHandler;

protected:
    //! @private
    explicit OverallTypicalSection(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(OverallTypicalSection)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(OverallTypicalSection)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static OverallTypicalSectionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT OverallTypicalSectionCPtr Insert(OverallTypicalSectionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // OverallTypicalSection

//=======================================================================================
//! Model breaking-down a TypicalSectionPortion elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionBreakDownModel : Dgn::GeometricModel2d
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSectionBreakDownModel, Dgn::GeometricModel2d);
friend struct OverallTypicalSectionBreakDownModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(OverallTypicalSectionBreakDownModel::T_Super::CreateParams);

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
    explicit OverallTypicalSectionBreakDownModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPortionBreakDownModel)

    static OverallTypicalSectionBreakDownModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< OverallTypicalSectionBreakDownModel >(id); }
    static OverallTypicalSectionBreakDownModelPtr Create(CreateParams const& params) { return new OverallTypicalSectionBreakDownModel(params); }
}; // OverallTypicalSectionBreakDownModel

//=======================================================================================
//! Base class for TypicalSection Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionComponentElement : Dgn::GeometricElement2d
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionComponentElement, Dgn::GeometricElement2d);
friend struct TypicalSectionComponentElementHandler;

protected:
    //! @private
    explicit TypicalSectionComponentElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionComponentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionComponentElement)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetPoints(TypicalSectionComponentElementCR component, bvector<TypicalSectionPointCP> const& points);

    ROADRAILPHYSICAL_EXPORT bvector<Dgn::DgnElementId> QueryPointIds() const;
}; // TypicalSectionComponentElement

//=======================================================================================
//! Buffer Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureComponent : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureComponent, TypicalSectionComponentElement);
friend struct TravelwayStructureComponentHandler;

protected:
    //! @private
    explicit TravelwayStructureComponent(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayStructureComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwayStructureComponent)

    ROADRAILPHYSICAL_EXPORT static TravelwayStructureComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
}; // TravelwayStructureComponent

//=======================================================================================
//! Base class for Travelway Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayComponentElement : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayComponentElement, TypicalSectionComponentElement);
friend struct TravelwayComponentElementHandler;

protected:
    //! @private
    explicit TravelwayComponentElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayComponentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwayComponentElement)    
}; // TravelwayComponentElement

//=======================================================================================
//! Lane Components for Roadways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadLaneComponent : TravelwayComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadLaneComponent, TravelwayComponentElement);
friend struct RoadLaneComponentHandler;

protected:
    //! @private
    explicit RoadLaneComponent(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadLaneComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadLaneComponent)

    ROADRAILPHYSICAL_EXPORT static RoadLaneComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static RoadLaneComponentCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<TypicalSectionPointCP> const& points);
}; // RoadLaneComponent

//=======================================================================================
//! EndCondition Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideComponent : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideComponent, TypicalSectionComponentElement);
friend struct TravelwaySideComponentHandler;

protected:
    //! @private
    explicit TravelwaySideComponent(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySideComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TravelwaySideComponent)

    ROADRAILPHYSICAL_EXPORT static TravelwaySideComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
}; // TravelwaySideComponent



//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionHandler : Dgn::dgn_ElementHandler::TemplateRecipe2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSection, OverallTypicalSection, OverallTypicalSectionHandler, Dgn::dgn_ElementHandler::TemplateRecipe2d, ROADRAILPHYSICAL_EXPORT)
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
//! ElementHandler for TravelwaySide Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideDefinitionHandler : TypicalSectionPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideDefinition, TravelwaySideDefinition, TravelwaySideDefinitionHandler, TypicalSectionPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwaySideDefinitionHandler

//=================================================================================
//! ElementHandler for TravelwayStructure Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureDefinitionHandler : TypicalSectionPortionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureDefinition, TravelwayStructureDefinition, TravelwayStructureDefinitionHandler, TypicalSectionPortionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayStructureDefinitionHandler

//=======================================================================================
//! The ModelHandler for TypicalSectionPortionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionBreakDownModelHandler : Dgn::dgn_ModelHandler::Geometric2d
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSectionBreakDownModel, OverallTypicalSectionBreakDownModel, OverallTypicalSectionBreakDownModelHandler, Dgn::dgn_ModelHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // OverallTypicalSectionBreakDownModelHandler

//=======================================================================================
//! The ModelHandler for TypicalSectionPortionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionBreakDownModelHandler : Dgn::dgn_ModelHandler::Geometric2d
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionBreakDownModel, TypicalSectionPortionBreakDownModel, TypicalSectionPortionBreakDownModelHandler, Dgn::dgn_ModelHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPortionBreakDownModelHandler

//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionComponentElementHandler : Dgn::dgn_ElementHandler::Geometric2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionComponentElement, TypicalSectionComponentElement, TypicalSectionComponentElementHandler, Dgn::dgn_ElementHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionComponentElementHandler

//=================================================================================
//! ElementHandler for EndCondition Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideComponentHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideComponent, TravelwaySideComponent, TravelwaySideComponentHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwaySideComponentHandler

//=================================================================================
//! ElementHandler for Buffer Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureComponentHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureComponent, TravelwayStructureComponent, TravelwayStructureComponentHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayStructureComponentHandler

//=================================================================================
//! ElementHandler for Travelway Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayComponentElementHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayComponentElement, TravelwayComponentElement, TravelwayComponentElementHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayComponentElementHandler

//=================================================================================
//! ElementHandler for RoadTravelway Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadLaneComponentHandler : TravelwayComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadLaneComponent, RoadLaneComponent, RoadLaneComponentHandler, TravelwayComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadTravelwayComponentHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE