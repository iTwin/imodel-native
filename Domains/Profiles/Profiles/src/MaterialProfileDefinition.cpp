/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\MaterialProfileDefinition.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(MaterialProfileDefinitionHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialProfileDefinition::MaterialProfileDefinition (CreateParams const& params) : T_Super (params)
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

END_BENTLEY_PROFILES_NAMESPACE
