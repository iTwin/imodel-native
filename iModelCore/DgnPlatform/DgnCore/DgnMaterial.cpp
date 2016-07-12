/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnMaterial.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(Material);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

#define PROPNAME_Data "Data"
#define PROPNAME_Descr "Descr"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_OnDelete() const
    {
    return DgnDbStatus::DeletionProhibited; // can only purge, not delete
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Material::_TEMPORARY_GetHandlingCustomAttributes(ECSqlClassParams::HandlingCustomAttributes& params) // *** WIP_AUTO_HANDLED_PROPERTIES
    {
    T_Super::_TEMPORARY_GetHandlingCustomAttributes(params);
    params.Add(PROPNAME_Descr);
    params.Add(PROPNAME_Data);
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
DgnDbStatus DgnMaterial::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        m_data.Init(stmt.GetValueText(params.GetSelectIndex(PROPNAME_Data)), stmt.GetValueText(params.GetSelectIndex(PROPNAME_Descr)));
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::BindParams(ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Descr), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Data), m_data.m_value.c_str(), IECSqlBinder::MakeCopy::No))
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

    return T_Super::_SetParentId(parentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterial::CreateParams::CreateParams(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName, Utf8StringCR value, DgnMaterialId parentMaterialId, Utf8StringCR descr)
  : T_Super(db, DgnModel::DictionaryId(), DgnMaterial::QueryDgnClassId(db), CreateMaterialCode(paletteName, materialName), nullptr, parentMaterialId),
    m_data(value, descr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId DgnMaterial::QueryMaterialId(DgnCode const& code, DgnDbR db)
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

    JsonValueCR  constValue =  root[keyWord];
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterial::Iterator DgnMaterial::Iterator::Create(DgnDbR db, Options const& options)
    {
    Utf8String ecsql("SELECT ECInstanceId,Code.[Value],Code.Namespace,ParentId,Descr FROM " DGN_SCHEMA(DGN_CLASSNAME_MaterialElement));
    if (options.m_byPalette)
        ecsql.append(" WHERE Code.[Namespace]=?");

    if (options.m_byParent)
        ecsql.append(options.m_byPalette ? " AND " : " WHERE ").append("ParentId=?");

    if (options.m_ordered)
        ecsql.append(" ORDER BY Code.Namespace,Code.[Value]");

    Iterator iter;
    ECSqlStatement* stmt = iter.Prepare(db, ecsql.c_str(), 0);
    if (nullptr != stmt)
        {
        if (options.m_byPalette)
            stmt->BindText(1, options.m_palette.c_str(), IECSqlBinder::MakeCopy::Yes);

        if (options.m_byParent)
            stmt->BindId(options.m_byPalette ? 2 : 1, options.m_parent);
        }

    return iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterial::Iterator DgnMaterial::MakeIterator(DgnDbR db, Iterator::Options options)
    {
    return Iterator::Create(db, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId DgnMaterial::ImportMaterial(DgnMaterialId srcMaterialId, DgnImportContext& importer)
    {
    //  See if we already have a material with the same palette and code in the destination Db.
    //  If so, we'll map the source material to it.
    DgnMaterialCPtr srcMaterial = DgnMaterial::QueryMaterial(srcMaterialId, importer.GetSourceDb());
    BeAssert(srcMaterial.IsValid());
    if (!srcMaterial.IsValid())
        {
        BeAssert(false);
        return DgnMaterialId();
        }

    DgnMaterialId dstMaterialId = QueryMaterialId(srcMaterial->GetPaletteName(), srcMaterial->GetMaterialName(), importer.GetDestinationDb());
    if (dstMaterialId.IsValid())
        {
        //  *** TBD: Check if the material definitions match. If not, rename and remap
        //  *** TBD: Make sure that child materials are also remapped? Or, wait for someone to ask for them one by one?
        importer.AddMaterialId(srcMaterialId, dstMaterialId);
        return dstMaterialId;
        }

    //  No such material in the destination Db. Ask the source Material to import itself.
    auto importedElem = srcMaterial->Import(nullptr, importer.GetDestinationDb().GetDictionaryModel(), importer);

    return importedElem.IsValid()? DgnMaterialId(importedElem->GetElementId().GetValue()): DgnMaterialId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMaterial::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);

    if (!importer.IsBetweenDbs())
        return;

    JsonRenderMaterial material;
    if (SUCCESS==GetRenderingAsset(material.GetValueR()) && SUCCESS==material.Relocate(importer))
        SetRenderingAsset(material.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId DgnImportContext::RemapMaterialId(DgnMaterialId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnMaterialId dest = FindMaterialId(source);
    return dest.IsValid() ? dest : DgnMaterial::ImportMaterial(source, *this);
    }
