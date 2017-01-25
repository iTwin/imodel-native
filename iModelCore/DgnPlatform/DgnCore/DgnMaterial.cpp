/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnMaterial.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
void DgnMaterial::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);
    stmt.BindText(stmt.GetParameterIndex(PROPNAME_Descr), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindText(stmt.GetParameterIndex(PROPNAME_Data), m_data.m_value.c_str(), IECSqlBinder::MakeCopy::No);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2016
//---------------+---------------+---------------+---------------+---------------+-------
void dgn_ElementHandler::Material::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, PROPNAME_Descr,
        [] (ECValueR value, DgnElementCR elIn)
            {
            DgnMaterial& el = (DgnMaterial&) elIn;
            value.SetUtf8CP(el.GetDescr().c_str());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            DgnMaterial& el = (DgnMaterial&) elIn;
            el.SetDescr(value.GetUtf8CP());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, PROPNAME_Data,
        [] (ECValueR value, DgnElementCR elIn)
            {
            DgnMaterial& el = (DgnMaterial&) elIn;
            value.SetUtf8CP(el.GetValue().c_str());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            DgnMaterial& el = (DgnMaterial&) elIn;
            el.SetValue(value.GetUtf8CP());
            return DgnDbStatus::Success;
            });

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_SetParentId(DgnElementId parentId, DgnClassId parentRelClassId) 
    {
    if (parentId.IsValid())
        {
        // parent must be another material
        auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT count(*) FROM " BIS_SCHEMA(BIS_CLASS_MaterialElement) " WHERE ECInstanceId=?");
        if (!stmt.IsValid())
            return DgnDbStatus::InvalidParent;

        stmt->BindId(1, parentId);
        if (BE_SQLITE_ROW != stmt->Step() || 1 != stmt->GetValueInt(0))
            return DgnDbStatus::InvalidParent;
        }

    return T_Super::_SetParentId(parentId, parentRelClassId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterial::CreateParams::CreateParams(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName, Utf8StringCR value, DgnMaterialId parentMaterialId, Utf8StringCR descr)
  : T_Super(db, DgnModel::DictionaryId(), DgnMaterial::QueryDgnClassId(db), CreateCode(db, paletteName, materialName), nullptr, parentMaterialId),
    m_data(value, descr)
    {
    if (parentMaterialId.IsValid())
        m_parentRelClassId = db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_REL_MaterialOwnsChildMaterials);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId DgnMaterial::QueryMaterialId(DgnDbR db, DgnCodeCR code)
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
        DgnMaterialId destParentId = DgnMaterial::QueryMaterialId(importer.GetDestinationDb(), GetCode());
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
    Utf8String ecsql("SELECT ECInstanceId,CodeValue,CodeScope,Parent.Id,Descr FROM " BIS_SCHEMA(BIS_CLASS_MaterialElement));
    if (options.m_byPalette)
        ecsql.append(" WHERE CodeScope=?");

    if (options.m_byParent)
        ecsql.append(options.m_byPalette ? " AND " : " WHERE ").append("Parent.Id=?");

    if (options.m_ordered)
        ecsql.append(" ORDER BY CodeScope,CodeValue");

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
    DgnMaterialCPtr srcMaterial = DgnMaterial::Get(importer.GetSourceDb(), srcMaterialId);
    BeAssert(srcMaterial.IsValid());
    if (!srcMaterial.IsValid())
        {
        BeAssert(false);
        return DgnMaterialId();
        }

    DgnMaterialId dstMaterialId = QueryMaterialId(importer.GetDestinationDb(), srcMaterial->GetPaletteName(), srcMaterial->GetMaterialName());
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
DgnMaterialId DgnImportContext::_RemapMaterialId(DgnMaterialId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnMaterialId dest = FindMaterialId(source);
    return dest.IsValid() ? dest : DgnMaterial::ImportMaterial(source, *this);
    }
