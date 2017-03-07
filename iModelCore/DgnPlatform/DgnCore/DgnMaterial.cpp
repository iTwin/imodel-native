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

#define PROPNAME_Description "Description"

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
DgnMaterial::CreateParams::CreateParams(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName, DgnMaterialId parentMaterialId)
  : T_Super(db, DgnModel::DictionaryId(), DgnMaterial::QueryDgnClassId(db), CreateCode(db, paletteName, materialName), nullptr, parentMaterialId)
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
    Utf8String ecsql("SELECT ECInstanceId,CodeValue,CodeScope,Parent.Id,Description FROM " BIS_SCHEMA(BIS_CLASS_MaterialElement));
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

    if (importer.IsBetweenDbs())
        GetRenderingAssetR().Relocate(importer);
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
