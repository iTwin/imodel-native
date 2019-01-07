/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/MaterialProfileDefinition.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\MaterialProfileDefinition.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialProfileDefinition::MaterialProfileDefinition(CreateParams const& params) : T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus MaterialProfileDefinition::_OnInsert()
    {
    DgnDbStatus status = Validate();
    if (status != DgnDbStatus::Success)
        return status;

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus MaterialProfileDefinition::_OnUpdate (DgnElement const& original)
    {
    DgnDbStatus status = Validate();
    if (status != DgnDbStatus::Success)
        return status;

    return T_Super::_OnUpdate (original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus MaterialProfileDefinition::Validate() const
    {
    return _Validate() ? DgnDbStatus::Success : DgnDbStatus::ValidationFailed;
    }

HANDLER_DEFINE_MEMBERS(MaterialProfileDefinitionHandler)

END_BENTLEY_PROFILES_NAMESPACE
