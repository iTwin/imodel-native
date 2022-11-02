/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LsLocation::~LsLocation ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsLocation::SetLocation (DgnDbR project, LsComponentId componentId)
    {
    Init ();
    m_dgndb = &project;
    m_componentId = componentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LsLocation::SetFrom (LsLocation const* base, LsComponentId componentId)
    {
    SetFrom (base);
    m_componentId = componentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsLocation::SetFrom (LsLocation const* base)
    {
    m_componentId     = base->m_componentId;
    m_dgndb   = base->m_dgndb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t        LsLocation::GetFileKey () const
    {
    return (intptr_t)m_dgndb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentId LsLocation::GetComponentId () const
    {
    return m_componentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentType      LsLocation::GetComponentType () const
    {
    return m_componentId.GetType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool LsComponent::IsValidComponentType(LsComponentType value)
    {
    switch(value)
        {
        case LsComponentType::Compound:
        case LsComponentType::Internal:
        case LsComponentType::LineCode:
        case LsComponentType::LinePoint:
        case LsComponentType::PointSymbol:
        case LsComponentType::RasterImage:
            return true;
        }
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsLocation::IsValid () const
    {
    return m_componentId.IsValid() && LsComponent::IsValidComponentType(m_componentId.GetType());;
    }
