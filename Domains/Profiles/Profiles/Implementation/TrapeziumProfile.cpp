/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/TrapeziumProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TrapeziumProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(TrapeziumProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TrapeziumProfilePtr TrapeziumProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetTopWidth(double val)
    {
    SetPropertyValue(PRF_PROP_TrapeziumProfile_TopWidth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetBottomWidth(double val)
    {
    SetPropertyValue(PRF_PROP_TrapeziumProfile_BottomWidth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_TrapeziumProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetTopOffset(double val)
    {
    SetPropertyValue(PRF_PROP_TrapeziumProfile_TopOffset, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
