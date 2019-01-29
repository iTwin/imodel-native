/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/MaterialProfileDefinition.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElement const& original) override;
    }; // MaterialProfileDefinition

//=======================================================================================
//! Handler for MaterialProfileDefinition class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MaterialProfileDefinitionHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT (PRF_CLASS_MaterialProfileDefinition, MaterialProfileDefinition, MaterialProfileDefinitionHandler, Dgn::dgn_ElementHandler::Definition, PROFILES_EXPORT)
    
    }; // MaterialProfileDefinitionHandler

END_BENTLEY_PROFILES_NAMESPACE