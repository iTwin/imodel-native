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

    LinePointRsc*   lpRsc = (LinePointRsc*) reader->GetRsc();
    m_componentType = (LsComponentType)lpRsc->lcType;
    m_componentId = (LsComponentId)lpRsc->lcID;

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

    LinePointRsc*           lpRsc = (LinePointRsc*) reader->GetRsc();

    m_componentType = (LsComponentType)lpRsc->symbol[symbolNumber].symType;
    m_componentId = LsComponentId(lpRsc->symbol[symbolNumber].symID);

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

    m_componentType = (LsComponentType)reader->GetRsc()->component[componentNumber].type;
    m_componentId   = LsComponentId(reader->GetRsc()->component[componentNumber].id);

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
