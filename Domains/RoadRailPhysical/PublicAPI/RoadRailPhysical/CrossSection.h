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
//! Model for CrossSection elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionDefinitionModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionDefinitionModel, Dgn::DefinitionModel);
friend struct CrossSectionDefinitionModelHandler;

protected:
    explicit CrossSectionDefinitionModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CrossSectionDefinitionModel)

    static CrossSectionDefinitionModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< CrossSectionDefinitionModel >(id); }    
    static CrossSectionDefinitionModelPtr Create(CreateParams const& params) { return new CrossSectionDefinitionModel(params); }
}; // CrossSectionDefinitionModel

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
//! Model breaking-down a CrossSection elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionBreakDownModel : Dgn::DefinitionModel
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionBreakDownModel, Dgn::DefinitionModel);
friend struct CrossSectionBreakDownModelHandler;

protected:
    explicit CrossSectionBreakDownModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CrossSectionBreakDownModel)

    static CrossSectionBreakDownModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< CrossSectionBreakDownModel >(id); }
    static CrossSectionBreakDownModelPtr Create(CreateParams const& params) { return new CrossSectionBreakDownModel(params); }
}; // CrossSectionBreakDownModel

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

    ROADRAILPHYSICAL_EXPORT RoadCrossSectionCPtr Insert(CrossSectionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // RoadCrossSection


//=======================================================================================
//! The ModelHandler for CrossSectionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionDefinitionModel, CrossSectionDefinitionModel, CrossSectionDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // CrossSectionDefinitionModelHandler

//=======================================================================================
//! The ModelHandler for CrossSectionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CrossSectionBreakDownModelHandler : Dgn::dgn_ModelHandler::Definition
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CrossSectionBreakDownModel, CrossSectionBreakDownModel, CrossSectionBreakDownModelHandler, Dgn::dgn_ModelHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // CrossSectionBreakDownModelHandler

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