/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TypicalSectionPoint.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class for point constructs in a TypicalSection portion definition.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPointDefinition : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPointDefinition, Dgn::DefinitionElement);
friend struct TypicalSectionPointDefinitionHandler;

protected:
    //! @private
    explicit TypicalSectionPointDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPointDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionPointDefinition)

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);
    
    //! Query for a Typical Section Point Definition based on a Code value
    //! @return A Typical Section Point Definition or nullptr if none could be found for the given Code value
    ROADRAILPHYSICAL_EXPORT static TypicalSectionPointDefinitionCPtr QueryByCode(Dgn::DefinitionModelCR model, Utf8StringCR pointCode);
}; // TypicalSectionPointDefinition

//=======================================================================================
//! Point constructs in a Travelway portion definition.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericTypicalSectionPointDefinition : TypicalSectionPointDefinition
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_GenericTypicalSectionPointDefinition, TypicalSectionPointDefinition);
friend struct GenericTypicalSectionPointDefinitionHandler;

protected:
    //! @private
    explicit GenericTypicalSectionPointDefinition(CreateParams const& params) : T_Super(params) {}    

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(GenericTypicalSectionPointDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(GenericTypicalSectionPointDefinition)

    //! @private
    ROADRAILPHYSICAL_EXPORT static GenericTypicalSectionPointDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
    //! @private
    ROADRAILPHYSICAL_EXPORT static GenericTypicalSectionPointDefinitionCPtr CreateAndInsert(Dgn::DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel = nullptr);
}; // GenericTypicalSectionPointDefinition
//__PUBLISH_SECTION_END__


//=================================================================================
//! ElementHandler for Typical Section Point Definitions
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPointDefinitionHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPointDefinition, TypicalSectionPointDefinition, TypicalSectionPointDefinitionHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPointDefinitionHandler

//=================================================================================
//! ElementHandler for Travelway Point Definitions
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericTypicalSectionPointDefinitionHandler : TypicalSectionPointDefinitionHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_GenericTypicalSectionPointDefinition, GenericTypicalSectionPointDefinition, GenericTypicalSectionPointDefinitionHandler, TypicalSectionPointDefinitionHandler, ROADRAILPHYSICAL_EXPORT)
}; // GenericTypicalSectionPointDefinitionHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE