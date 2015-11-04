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
void dgn_ElementHandler::Material::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
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
DgnDbStatus DgnMaterial::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
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

    // TODO? Base implementation doesn't check for cycles...
    return T_Super::_SetParentId(parentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterial::CreateParams::CreateParams(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName, Utf8StringCR value, DgnMaterialId parentMaterialId, Utf8StringCR descr)
  : T_Super(db, DgnMaterial::QueryDgnClassId(db), CreateMaterialCode(paletteName, materialName), parentMaterialId),
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterial::Iterator DgnMaterial::Iterator::Create(DgnDbR db, Options const& options)
    {
    Utf8String ecsql("SELECT ECInstanceId,Code,CodeNameSpace,ParentId,Descr FROM " DGN_SCHEMA(DGN_CLASSNAME_MaterialElement));
    if (options.m_byPalette)
        ecsql.append(" WHERE CodeNameSpace=?");

    if (options.m_byParent)
        ecsql.append(options.m_byPalette ? " AND " : " WHERE ").append("ParentId=?");

    if (options.m_ordered)
        ecsql.append(" ORDER BY CodeNameSpace,Code");

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
        BeAssert(false && "invalid source material ID");
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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_MERGE_YII
DgnMaterialId DgnMaterials::ImportMaterial(DgnImportContext& context, DgnDbR sourceDb, DgnMaterialId source)
    {
    Material sourceMaterial = sourceDb.Materials().Query(source);
    if (!sourceMaterial.IsValid())
        {
        BeAssert(!source.IsValid() && "look up should fail only for an invalid materialid");
        return DgnMaterialId();
        }

    // If the destination Db already contains a material by this name, then remap to it. Don't create another copy.
    DgnMaterialId destMaterialId = context.GetDestinationDb().Materials().QueryMaterialId(sourceMaterial.GetName(), sourceMaterial.GetPalette());
    if (destMaterialId.IsValid())
        return destMaterialId;

    //  Must copy and remap the source material.
    Material destMaterial(sourceMaterial);
    if (sourceMaterial.GetParentId().IsValid())
        destMaterial.SetParentId(context.RemapMaterialId(sourceMaterial.GetParentId()));

    Json::Value renderingAsset;

    if (SUCCESS == destMaterial.GetRenderingAsset (renderingAsset))
        {
        RenderMaterialPtr       renderMaterial = JsonRenderMaterial::Create (renderingAsset, source);
        JsonRenderMaterial*     jsonRenderMaterial = dynamic_cast <JsonRenderMaterial*> (renderMaterial.get());

        if (nullptr != jsonRenderMaterial &&
            SUCCESS == jsonRenderMaterial->DoImport (context, sourceDb))
           destMaterial.SetRenderingAsset (jsonRenderMaterial->GetValue());
        }

    Insert (destMaterial);

    return context.AddMaterialId(source, destMaterial.GetId());
    }
#endif
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId DgnImportContext::RemapMaterialId(DgnMaterialId source)
    {
    if (!IsBetweenDbs())
        return source;
    DgnMaterialId dest = FindMaterialId(source);
    if (dest.IsValid())
        return dest;
    return DgnMaterial::ImportMaterial(source, *this);
    }
