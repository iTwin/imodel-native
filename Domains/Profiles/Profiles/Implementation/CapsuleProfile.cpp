/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CapsuleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CapsuleProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(CapsuleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CapsuleProfilePtr CapsuleProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new CapsuleProfile(params);
    }

END_BENTLEY_PROFILES_NAMESPACE
