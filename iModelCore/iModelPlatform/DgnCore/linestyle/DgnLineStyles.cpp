/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

using namespace std;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Insert(DgnStyleId& newStyleId, DgnModelId modelId, Utf8CP name, Utf8CP description, LsComponentId componentId, uint32_t flags, double unitDefinition)
    {
    DefinitionModelPtr model = m_dgndb.Models().Get<DefinitionModel>(modelId);
    if (!model.IsValid())
        {
        newStyleId = DgnStyleId();
        return ERROR;
        }

    Json::Value jsonObj(Json::objectValue);
    LsDefinition::InitializeJsonObject(jsonObj, componentId, flags, unitDefinition);
    Utf8String data = Json::FastWriter::ToString(jsonObj);

    LineStyleElementPtr lsElement = LineStyleElement::Create(*model);
    lsElement->SetName(name);
    lsElement->SetDescription(description);
    lsElement->SetData(data.c_str());
    LineStyleElementCPtr constLs = lsElement->Insert();
    if (!constLs.IsValid())
        {
        newStyleId = DgnStyleId();
        return ERROR;
        }
    
    newStyleId = DgnStyleId(constLs->GetElementId().GetValue());

    LsDefinition* lsDef = new LsDefinition(name, m_dgndb, jsonObj, newStyleId);
    GetCache().AddIdEntry(lsDef);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//--------------+------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Insert (DgnStyleId& newStyleId, Utf8CP name, LsComponentId componentId, uint32_t flags, double unitDefinition)
    {
    return Insert(newStyleId, DgnModel::DictionaryId(), name, nullptr, componentId, flags, unitDefinition);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//--------------+------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Update (DgnStyleId styleId, Utf8CP name, LsComponentId componentId, uint32_t flags, double unitDefinition)
    {
    PRECONDITION(styleId.IsValid(), ERROR);

    Json::Value jsonObj(Json::objectValue);
    LsDefinition::InitializeJsonObject(jsonObj, componentId, flags, unitDefinition);
    Utf8String data = Json::FastWriter::ToString(jsonObj);

    LineStyleElementPtr lsElement = m_dgndb.Elements().GetForEdit<LineStyleElement>(styleId);
    if (!lsElement.IsValid())
        return ERROR;

    lsElement->SetName(name);
    lsElement->SetData(data.c_str());
    auto stat = lsElement->Update();
    return stat == DgnDbStatus::Success ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LsCacheR DgnLineStyles::GetCache()
    {
    if (m_lineStyleMap.IsNull())
        {
        BeMutexHolder lock(m_mutex);
        if (m_lineStyleMap.IsNull())
            m_lineStyleMap = LsCache::Create (m_dgndb);
        }

    return *m_lineStyleMap;
    }

