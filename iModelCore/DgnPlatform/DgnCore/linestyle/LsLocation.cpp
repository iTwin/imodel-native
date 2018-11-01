/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsLocation.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsLocation::~LsLocation ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsLocation::SetLocation (DgnDbR project, LsComponentId componentId)
    {
    Init ();
    m_dgndb = &project;
    m_componentId = componentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsLocation::SetFrom (LsLocation const* base, LsComponentId componentId)
    {
    SetFrom (base);
    m_componentId = componentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsLocation::SetFrom (LsLocation const* base)
    {
    m_componentId     = base->m_componentId;
    m_dgndb   = base->m_dgndb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t        LsLocation::GetFileKey () const
    {
    return (intptr_t)m_dgndb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentId LsLocation::GetComponentId () const
    {
    return m_componentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentType      LsLocation::GetComponentType () const
    {
    return m_componentId.GetType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
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
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsLocation::IsValid () const
    {
    return m_componentId.IsValid() && LsComponent::IsValidComponentType(m_componentId.GetType());;
    }
