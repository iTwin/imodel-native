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
void            LsLocation::SetLocation (DgnDbR project, LsComponentType componentType, LsComponentId componentId)
    {
    Init ();
    m_dgndb = &project;
    m_componentType = componentType;
    m_componentId = componentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsLocation::SetFrom (LsLocation const* base, LsComponentType componentType)
    {
    SetFrom (base);
    m_componentType   = componentType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsLocation::SetFrom (LsLocation const* base)
    {
    m_componentType   = base->m_componentType;
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
    return m_componentType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsLocation::GetLineCodeLocation (LsComponentReader* reader)
    {
    SetFrom (reader->GetSource());

    V10LinePoint*   lpData = (V10LinePoint*)reader->GetRsc();
    m_componentType = (LsComponentType)lpData->m_lcType;
    m_componentId = (LsComponentId)lpData->m_lcID;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsLocation::GetPointSymbolLocation
(
LsComponentReader*    reader,
int             symbolNumber
)
    {
    SetFrom (reader->GetSource());

    V10LinePoint*   lpData = (V10LinePoint*)reader->GetRsc();

    m_componentType = (LsComponentType)lpData->m_symbol[symbolNumber].m_symType;
    m_componentId = LsComponentId(lpData->m_symbol[symbolNumber].m_symID);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsLocation::GetCompoundComponentLocation
(
LsComponentReader*    reader,
int             componentNumber
)
    {
    SetFrom (reader->GetSource());
    V10Compound* v10Data = (V10Compound*)reader->GetRsc();
    m_componentType = (LsComponentType)v10Data->m_component[componentNumber].m_type;
    m_componentId   = LsComponentId(v10Data->m_component[componentNumber].m_id);

    return SUCCESS;
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
            return true;
        }
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsLocation::IsValid () const
    {
    return LsComponent::IsValidComponentType(m_componentType);
    }
