/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsLocation.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/LsLocal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsLocation::~LsLocation ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsLocation::SetLocation (DgnDbR project, uint32_t componentType, uint32_t componentId)
    {
    Init ();
    m_dgndb   = &project;
    m_rscType   = static_cast <LsResourceType>(remapElmTypeToRscType ((LsElementType)componentType));
    m_rscID = componentId;
    m_fileType  = LsLocationType::DgnDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsLocation::SetFrom (LsLocation const* base, uint32_t rscType)
    {
    SetFrom (base);
    m_rscType   = static_cast <LsResourceType> (rscType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsLocation::SetFrom (LsLocation const* base)
    {
    m_rscType   = base->m_rscType;
    m_rscID     = base->m_rscID;
    m_dgndb   = base->m_dgndb;
    m_fileType  = base->m_fileType;
    m_idDefined = base->m_idDefined;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t        LsLocation::GetFileKey () const
    {
    switch (m_fileType)
        {
        case    LsLocationType::DgnDb:
            return (intptr_t)m_dgndb;
        }

    return (intptr_t)LsLocationType::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t LsLocation::GetIdentKey () const
    {
    switch (m_fileType)
        {
        case    LsLocationType::DgnDb:
            return m_rscID;
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
LsElementType      LsLocation::GetElementType () const
    {
    return (LsElementType)remapRscTypeToElmType ((uint32_t)m_rscType);
    }

/*---------------------------------------------------------------------------------**//**
* Returns appropriate name map for this location
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
struct          LsMap *LsLocation::GetMapPtr () const
    {
    if (LsLocationType::DgnDb == m_fileType)
        return LsMap::GetMapPtr (*m_dgndb);

    return LsSystemMap::GetSystemMapPtr ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsLocation::GetLineCodeLocation (LsRscReader* reader)
    {
    SetFrom (reader->GetSource());

    LinePointRsc*        lpRsc = (LinePointRsc*) reader->GetRsc();
    m_rscType = static_cast <LsResourceType> (remapElmTypeToRscType((LsElementType)lpRsc->lcType));
    m_rscID = lpRsc->lcID;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsLocation::GetPointSymbolLocation
(
LsRscReader*    reader,
int             symbolNumber
)
    {
    SetFrom (reader->GetSource());

    LinePointRsc*           lpRsc = (LinePointRsc*) reader->GetRsc();

    m_rscType = static_cast <LsResourceType> (lpRsc->symbol[symbolNumber].symType);
    m_rscID = lpRsc->symbol[symbolNumber].symID;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsLocation::GetCompoundComponentLocation
(
LsRscReader*    reader,
int             componentNumber
)
    {
    SetFrom (reader->GetSource());

    m_rscType = static_cast <LsResourceType> (reader->GetRsc()->component[componentNumber].type);
    m_rscID   = reader->GetRsc()->component[componentNumber].id;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman     02/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsLocation::IsValid () const
    {
    if (LsLocationType::Unknown == m_fileType)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/93
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t LsLocation::GetSeedID
(
) const
    {
    LsElementType type = (LsElementType)m_rscType;
    switch (type)
        {
        case LsElementType::LineCode:
        case LsElementType::LinePoint:
        case LsElementType::Compound:
            return  (long) (GetIdentKey() & 0xFFFFFFFF);
        }

    return -1L;
    }

