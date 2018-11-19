/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/TrapeziumProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TrapeziumProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (TrapeziumProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TrapeziumProfile::GetTopWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TrapeziumProfile_TopWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetTopWidth (double val)
    {
    SetPropertyValue (PRF_PROP_TrapeziumProfile_TopWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TrapeziumProfile::GetBottomWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TrapeziumProfile_BottomWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetBottomWidth (double val)
    {
    SetPropertyValue (PRF_PROP_TrapeziumProfile_BottomWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TrapeziumProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TrapeziumProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_TrapeziumProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TrapeziumProfile::GetTopOffset() const
    {
    return GetPropertyValueDouble (PRF_PROP_TrapeziumProfile_TopOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetTopOffset (double val)
    {
    SetPropertyValue (PRF_PROP_TrapeziumProfile_TopOffset, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
