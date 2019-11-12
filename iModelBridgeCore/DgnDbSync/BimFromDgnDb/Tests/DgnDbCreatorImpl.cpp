/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "DgnDbCreatorImpl.h"

DGNDB06_USING_NAMESPACE_BENTLEY_EC
DGNDB06_USING_NAMESPACE_BENTLEY_DGN
DGNDB06_USING_NAMESPACE_BENTLEY_SQLITE
DGNDB06_USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_DGNDB0601_TO_JSON_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbCreatorImpl::DgnDbCreatorImpl(const char* fileName) : m_dgndbFilename(fileName)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
bool DgnDbCreatorImpl::CreateDgnDb()
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(true);

    DbResult createStatus;
    m_dgndb = DgnDb::CreateDgnDb(&createStatus, m_dgndbFilename, createProjectParams);

    return m_dgndb.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
bool DgnDbCreatorImpl::ImportSchema(const char* schemaXml)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_dgndb->GetSchemaLocater());

    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, (Utf8CP) schemaXml, *schemaReadContext))
        return false;

    return SUCCESS == m_dgndb->Schemas().ImportECSchemas(schemaReadContext->GetCache());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
bool DgnDbCreatorImpl::AddElement(const char* schemaName, const char* instanceXml)
    {

    ECSchemaCP schema = m_dgndb->Schemas().GetECSchema(schemaName);
    if (nullptr == schema)
        return false;

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schema);
    IECInstancePtr dgnDbECInstance = nullptr;
    const InstanceReadStatus readStat = IECInstance::ReadFromXmlString(dgnDbECInstance, (Utf8CP) instanceXml, *instanceContext);
    if (readStat != InstanceReadStatus::Success)
        return false;

    DgnDbStatus status;
    auto ele = m_dgndb->Elements().CreateElement(&status, *dgnDbECInstance);
    if (!ele.IsValid())
        return false;
    return DgnDbStatus::Success == status;

    }
END_DGNDB0601_TO_JSON_NAMESPACE
