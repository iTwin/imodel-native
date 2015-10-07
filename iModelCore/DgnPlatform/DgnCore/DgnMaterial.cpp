/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnMaterial.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(Material);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Material::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add("Descr");
    params.Add("Data");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) 
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        m_data.Init(stmt.GetValueText(params.GetSelectIndex("Descr")), stmt.GetValueText(params.GetSelectIndex("Data")));
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::BindParams(ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex("Descr"), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex("Data"), m_data.m_value.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;
    else
        return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMaterial::_CopyFrom(DgnElementCR el) 
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<DgnMaterialCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr != other)
        m_data = other->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_SetParentId(DgnElementId parentId) 
    {
    if (parentId.IsValid())
        {
        // parent must be another material
        auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_MaterialElement) " WHERE ECInstanceId=?");
        if (!stmt.IsValid())
            return DgnDbStatus::InvalidParent;

        stmt->BindId(1, parentId);
        if (BE_SQLITE_ROW != stmt->Step() || 1 != stmt->GetValueInt(0))
            return DgnDbStatus::InvalidParent;
        }

    // TODO? Base implementation doesn't check for cycles...
    return T_Super::_SetParentId(parentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnMaterial::CreateMaterialCode(Utf8StringCR palette, Utf8StringCR material, DgnDbR db)
    {
    auto auth = db.Authorities().Get<NamespaceAuthority>(DgnAuthority::MaterialId());
    BeAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(material, palette) : DgnAuthority::Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterial::CreateParams::CreateParams(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName, Utf8StringCR value, DgnMaterialId parentMaterialId, Utf8StringCR descr)
  : T_Super(db, DgnMaterial::QueryDgnClassId(db), CreateMaterialCode(paletteName, materialName, db), parentMaterialId),
    m_data(value, descr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId DgnMaterial::QueryMaterialId(DgnElement::Code const& code, DgnDbR db)
    {
    DgnElementId elemId = db.Elements().QueryElementIdByCode(code);
    return DgnMaterialId(elemId.GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMaterial::GetAsset(JsonValueR value, Utf8CP keyWord) const
    {
    Json::Value root;

    if (!Json::Reader::Parse(GetValue(), root))
        return ERROR;

    JsonValueCR     constValue =  root[keyWord];
    
    if (constValue.isNull())
        return ERROR;

    value = constValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void  DgnMaterial::SetAsset(JsonValueCR value, Utf8CP keyWord)
    {
    Json::Value root;

    if (!Json::Reader::Parse(GetValue(), root))
        root = Json::Value(Json::ValueType::objectValue);

    root[keyWord] = value;

    SetValue(Json::FastWriter::ToString(root).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const
    {
    DgnDbStatus status = T_Super::_OnChildImport(child, destModel, importer);
    if (DgnDbStatus::Success == status && importer.IsBetweenDbs() && !importer.FindElementId(GetElementId()).IsValid())
        {
        DgnMaterialId destParentId = DgnMaterial::QueryMaterialId(GetCode(), importer.GetDestinationDb());
        if (!destParentId.IsValid())
            Import(&status, destModel, importer);
        else
            importer.AddElementId(GetElementId(), destParentId);
        }

    return status;
    }

