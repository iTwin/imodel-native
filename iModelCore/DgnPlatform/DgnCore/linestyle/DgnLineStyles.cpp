/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/DgnLineStyles.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

using namespace std;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    07/2018
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
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Insert (DgnStyleId& newStyleId, Utf8CP name, LsComponentId componentId, uint32_t flags, double unitDefinition)
    {
    return Insert(newStyleId, DgnModel::DictionaryId(), name, nullptr, componentId, flags, unitDefinition);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus DgnLineStyles::Update (DgnStyleId styleId, Utf8CP name, LsComponentId componentId, uint32_t flags, double unitDefinition)
    {
#if defined(NEEDSWORK_LINESTYLES)
    PRECONDITION(styleId.IsValid(), ERROR);

    Json::Value jsonObj(Json::objectValue);
    LsDefinition::InitializeJsonObject(jsonObj, componentId, componentType, flags, unitDefinition);
    Utf8String data = Json::FastWriter::ToString(jsonObj);

    Statement update;
    update.Prepare(m_dgndb, "UPDATE " DGN_TABLE(BIS_CLASS_Style) " SET Name=?,Data=? WHERE Type=" DGN_STYLE_TYPE_Line " AND Id=?");
    update.BindText(1, name, Statement::MakeCopy::No);
    update.BindBlob(2, (void const*)&data[0], (int)data.size() + 1, Statement::MakeCopy::No);
    update.BindId(3, styleId);

    POSTCONDITION(BE_SQLITE_DONE == update.Step(), ERROR);

    return SUCCESS;
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    06/2009
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

