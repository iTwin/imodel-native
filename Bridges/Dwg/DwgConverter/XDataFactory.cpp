/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWG


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus XDataFactory::AddProperty (Utf8StringCR name, T_Utf8StringVectorCR strings)
    {
    // add a string array property to current ECInstance
    uint32_t    propertyIndex = 0;
    ECObjectsStatus status = ECObjectsStatus::EnablerNotFound;
    if (m_ecInstance.IsValid())
        {
        status = m_ecInstance->GetEnabler().GetPropertyIndex (propertyIndex, name.c_str());
        if (ECObjectsStatus::Success == status)
            {
            status = m_ecInstance->AddArrayElements (propertyIndex, strings.size());
            if (ECObjectsStatus::Success == status)
                {
                uint32_t    arrayIndex = 0;
                for (auto string : strings)
                    {
                    status = m_ecInstance->SetValue (propertyIndex, ECValue(string.c_str()), arrayIndex);
                    arrayIndex++;
                    }
                }
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataFactory::CreateECInstance (Utf8StringCR ecClassName)
    {
    if (m_sourceSchema == nullptr)
        return  BentleyStatus::BSIERROR;
        
    ECClassCP   ecClass = m_sourceSchema->GetClassCP (ecClassName.c_str());
    if (nullptr == ecClass)
        return  BentleyStatus::BSIERROR;

    m_ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance ();

    return  m_ecInstance.IsValid() ? BentleyStatus::BSISUCCESS : BentleyStatus::BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataFactory::ExtractBreadcrumbsFromBlock (T_Utf8StringVectorR strings, DwgDbBlockReferenceCR insert)
    {
    DwgDbBlockTableRecordPtr    block(insert.GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
    if (block.OpenStatus() != DwgDbStatus::Success)
        return  BentleyStatus::BSIERROR;
        
    auto xdata = block->GetXData (REGAPPName_BentleyCCBlockInfo);
    if (!xdata.IsValid())
        return  BentleyStatus::BSISUCCESS;

    for (DwgResBufP curr = xdata->Start(); curr != xdata->End(); curr = curr->Next())
        {
        // skip DXF group code 1001's
        if (curr->IsRegappName())
            continue;

        // collect DXF group code 1000's
        Utf8String  string;
        if (curr->GetDataType() == DwgResBuf::DataType::Text && !string.Assign(curr->GetString().c_str()).empty())
            strings.push_back (string);
        }

    return  strings.empty() ? BentleyStatus::BSIERROR : BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataFactory::CreateBreadcrumbs (DgnElementR targetElement, ECSchemaCP sourceSchema, DwgDbBlockReferenceCR insert)
    {
    /*-----------------------------------------------------------------------------------
    Support Bentley.CCBlockInfo: extract xdata from block definition, convert GUIDS to strings,
    set strings as an array of Breadscrums properties on the target element as multi-aspects
    -----------------------------------------------------------------------------------*/
    if (sourceSchema == nullptr)
        return  BentleyStatus::BSIERROR;

    m_sourceSchema = sourceSchema;
        
    T_Utf8StringVector  strings;
    auto status = this->ExtractBreadcrumbsFromBlock (strings, insert);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    status = this->CreateECInstance (ECCLASSName_BentleyCCBlockInfo);
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    if (this->AddProperty(PROPERTYName_Breadcrumbs, strings) != ECObjectsStatus::Success)
        return  BentleyStatus::BSIERROR;

    // schedule a GenericMultiAspect to be inserted or updated along with the host element:
    if (DgnElement::GenericMultiAspect::AddAspect(targetElement, *m_ecInstance.get()) == DgnDbStatus::Success)
        status = m_importer._AddPresentationRuleContent (targetElement, m_ecInstance->GetClass());
    else
        status = BentleyStatus::BSIERROR;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus XDataFactory::InitializeTargetSchema (ECSchemaPtr& targetSchema)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    if (targetSchema.IsNull())
        {
        auto status = ECSchema::CreateSchema (targetSchema, SCHEMAName_DwgAppData, SCHEMAAlias_DwgAppData, 1, 0, 0);
        if (ECObjectsStatus::Success == status)
            {
            status = targetSchema->SetDisplayLabel (SCHEMALabel_DwgAppData);
            status = targetSchema->SetDescription (SCHEMADescription_DwgAppData);
            status = DwgHelper::MakeSchemaDynamicForDwg (m_importer.GetDgnDb(), *targetSchema);
            }
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus XDataFactory::CreateDwgAppDataSchema (ECSchemaPtr& targetSchema, DwgDbBlockTableRecordCR block)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    if (targetSchema.IsValid())
        return  status;

    // support regapp Bentley.CCBlockInfo
    auto xdata = block.GetXData (REGAPPName_BentleyCCBlockInfo);
    if (xdata.IsValid())
        {
        auto status = this->InitializeTargetSchema (targetSchema);
        if (status != ECObjectsStatus::Success)
            return  status;

        // add a Bentley.CCBlockInfo class
        ECEntityClassP  ecClass = nullptr;
        status = targetSchema->CreateEntityClass (ecClass, ECCLASSName_BentleyCCBlockInfo);
        if (ECObjectsStatus::Success == status)
            {
            ecClass->SetDisplayLabel (ECCLASSName_BentleyCCBlockInfo);
            ecClass->SetDescription (ECCLASSName_BentleyCCBlockInfo);

            // add GenericMultiAspect as a base ECClass:
            ECClassCP   multiAspect = m_importer.GetDgnDb().Schemas().GetClass (BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
            if (nullptr != multiAspect)
                ecClass->AddBaseClass (*multiAspect);
            else
                return  ECObjectsStatus::ClassNotFound;

            PrimitiveArrayECPropertyP   ecProperty = nullptr;
            status = ecClass->CreatePrimitiveArrayProperty (ecProperty, PROPERTYName_Breadcrumbs);
            if (status == ECObjectsStatus::Success)
                {
                ecProperty->SetPrimitiveElementType (PRIMITIVETYPE_String);
                ecProperty->SetIsReadOnly (true);
                }
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
XDataFactory::XDataFactory (DwgImporterR importer) : m_importer(importer)
    {
    m_ecInstance = nullptr;
    m_sourceSchema = nullptr;
    }
