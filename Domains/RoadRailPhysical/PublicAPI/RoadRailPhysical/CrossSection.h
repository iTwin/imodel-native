/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/CrossSection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class for CrossSection Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionPortionDefinitionElement : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionPortionDefinitionElement, Dgn::DefinitionElement);
friend struct CrossSectionPortionDefinitionElementHandler;

protected:
    //! @private
    explicit CrossSectionPortionDefinitionElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CrossSectionPortionDefinitionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(CrossSectionPortionDefinitionElement)
}; // CrossSectionPortionDefinitionElement

//=======================================================================================
//! Model for CrossSection elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionDefinitionModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionDefinitionModel, Dgn::DefinitionModel);
friend struct CrossSectionDefinitionModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(CrossSectionDefinitionModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an CrossSectionDefinitionModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, CrossSectionDefinitionModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit CrossSectionDefinitionModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CrossSectionDefinitionModel)

    static CrossSectionDefinitionModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< CrossSectionDefinitionModel >(id); }    
    static CrossSectionDefinitionModelPtr Create(CreateParams const& params) { return new CrossSectionDefinitionModel(params); }
}; // CrossSectionDefinitionModel

//=======================================================================================
//! CrossSection Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionPortionDefinition : CrossSectionPortionDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionPortionDefinition, CrossSectionPortionDefinitionElement);
friend struct CrossSectionPortionDefinitionHandler;

protected:
    //! @private
    explicit CrossSectionPortionDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CrossSectionPortionDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(CrossSectionPortionDefinition)

    static CrossSectionPortionDefinitionPtr Create(CrossSectionDefinitionModelCR model, Utf8CP label);
}; // CrossSectionPortionDefinition

//=======================================================================================
//! Model breaking-down a CrossSectionPortion elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionPortionBreakDownModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionPortionBreakDownModel, Dgn::DefinitionModel);
friend struct CrossSectionPortionBreakDownModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(CrossSectionPortionBreakDownModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an CrossSectionPortionBreakDownModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, CrossSectionPortionBreakDownModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit CrossSectionPortionBreakDownModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CrossSectionPortionBreakDownModel)

    static CrossSectionPortionBreakDownModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< CrossSectionPortionBreakDownModel >(id); }
    static CrossSectionPortionBreakDownModelPtr Create(CreateParams const& params) { return new CrossSectionPortionBreakDownModel(params); }
}; // CrossSectionPortionBreakDownModel

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
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionElement : CrossSectionPortionDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionElement, CrossSectionPortionDefinitionElement);
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

    ROADRAILPHYSICAL_EXPORT RoadTravelwayDefinitionCPtr Insert(CrossSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
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
struct EXPORT_VTABLE_ATTRIBUTE EndConditionDefinition : CrossSectionPortionDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_EndConditionDefinition, CrossSectionPortionDefinitionElement);
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

    ROADRAILPHYSICAL_EXPORT EndConditionDefinitionCPtr Insert(CrossSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // EndConditionDefinition

//=======================================================================================
//! Base class for CrossSection Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionElement : Dgn::DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionElement, Dgn::DefinitionElement);
    friend struct CrossSectionElementHandler;

protected:
    //! @private
    explicit CrossSectionElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CrossSectionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(CrossSectionElement)
}; // CrossSectionElement

//=======================================================================================
//! Definition class for Road CrossSection Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadCrossSection : CrossSectionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadCrossSection, CrossSectionElement);
    friend struct RoadCrossSectionHandler;

protected:
    //! @private
    explicit RoadCrossSection(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadCrossSection)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(RoadCrossSection)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static RoadCrossSectionPtr Create(CrossSectionDefinitionModelCR model, Utf8StringCR code);

    //ROADRAILPHYSICAL_EXPORT RoadCrossSectionCPtr Insert(CrossSectionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // RoadCrossSection


//=================================================================================
//! ElementHandler for CrossSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionPortionDefinitionElementHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionPortionDefinitionElement, CrossSectionPortionDefinitionElement, CrossSectionPortionDefinitionElementHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // CrossSectionPortionDefinitionElementHandler

//=================================================================================
//! ElementHandler for CrossSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionPortionDefinitionHandler : CrossSectionPortionDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionPortionDefinition, CrossSectionPortionDefinition, CrossSectionPortionDefinitionHandler, CrossSectionPortionDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // CrossSectionPortionDefinitionElementHandler

//=======================================================================================
//! The ModelHandler for TravelwayDefinitionModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionModel, TravelwayDefinitionModel, TravelwayDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayDefinitionModelHandler

//=================================================================================
//! ElementHandler for CrossSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionElementHandler : CrossSectionPortionDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionElement, TravelwayDefinitionElement, TravelwayDefinitionElementHandler, CrossSectionPortionDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayDefinitionElementHandler

//=================================================================================
//! ElementHandler for CrossSection Elements
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
//! ElementHandler for CrossSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EndConditionDefinitionHandler : CrossSectionPortionDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_EndConditionDefinition, EndConditionDefinition, EndConditionDefinitionHandler, CrossSectionPortionDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // EndConditionDefinitionHandler

//=======================================================================================
//! The ModelHandler for CrossSectionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionDefinitionModel, CrossSectionDefinitionModel, CrossSectionDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // CrossSectionDefinitionModelHandler

//=======================================================================================
//! The ModelHandler for CrossSectionPortionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionPortionBreakDownModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionPortionBreakDownModel, CrossSectionPortionBreakDownModel, CrossSectionPortionBreakDownModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // CrossSectionPortionBreakDownModelHandler

//=================================================================================
//! ElementHandler for CrossSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionElementHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionElement, CrossSectionElement, CrossSectionElementHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // CrossSectionElementHandler

//=================================================================================
//! ElementHandler for Road CrossSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadCrossSectionHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadCrossSection, RoadCrossSection, RoadCrossSectionHandler, CrossSectionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadCrossSectionHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE