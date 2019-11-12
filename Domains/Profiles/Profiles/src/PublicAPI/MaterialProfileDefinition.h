/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A resource defining Material and Profile pair.
//! @ingroup GROUP_MaterialProfiles
//=======================================================================================
struct MaterialProfileDefinition : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_MaterialProfileDefinition, Dgn::DefinitionElement);
    friend struct MaterialProfileDefinitionHandler;

protected:
    MaterialProfileDefinition (CreateParams const& params);

public:
    PROFILES_EXPORT Dgn::DgnDbStatus Validate() const;

protected:
    virtual bool _Validate() const = 0;

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElement const& original) override; //!< @private
    }; // MaterialProfileDefinition

//=======================================================================================
//! Handler for MaterialProfileDefinition class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MaterialProfileDefinitionHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT (PRF_CLASS_MaterialProfileDefinition, MaterialProfileDefinition, MaterialProfileDefinitionHandler, Dgn::dgn_ElementHandler::Definition, PROFILES_EXPORT)
    
    }; // MaterialProfileDefinitionHandler

END_BENTLEY_PROFILES_NAMESPACE