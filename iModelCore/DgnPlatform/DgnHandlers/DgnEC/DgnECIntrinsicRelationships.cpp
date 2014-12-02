/*--------------------------------------------------------------------------------------+ 
|
|     $Source: DgnHandlers/DgnEC/DgnECIntrinsicRelationships.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    "DgnECIntrinsicRelationships.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
 WString DgnECIntrinsicRelationshipInstance::_GetInstanceId() const
    {
    WString id = m_source->GetInstanceId();
    id.append (L":::");
    id.append (m_target->GetInstanceId());
    id.append (L":::");
    id.append (m_enabler->GetClass().GetSchema().GetFullSchemaName());
    id.append (L":::");
    id.append (m_enabler->GetClass().GetName());
    return id;
    }
