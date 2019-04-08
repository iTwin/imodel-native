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
    HANDLER_DEFINE_MEMBERS(RenderMaterial);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RenderMaterial::_OnDelete() const
    {
    return DgnDbStatus::DeletionProhibited; // can only purge, not delete
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RenderMaterial::_SetParentId(DgnElementId parentId, DgnClassId parentRelClassId) 
    {
    if (parentId.IsValid())
        {
        // parent must be another material
        auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT count(*) FROM " BIS_SCHEMA(BIS_CLASS_RenderMaterial) " WHERE ECInstanceId=?");
        if (!stmt.IsValid())
            return DgnDbStatus::InvalidParent;

        stmt->BindId(1, parentId);
        if (BE_SQLITE_ROW != stmt->Step() || 1 != stmt->GetValueInt(0))
            return DgnDbStatus::InvalidParent;
        }

    return T_Super::_SetParentId(parentId, parentRelClassId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RenderMaterial::_OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const
    {
    DgnDbStatus status = T_Super::_OnChildImport(child, destModel, importer);
    if (DgnDbStatus::Success == status && importer.IsBetweenDbs() && !importer.FindElementId(GetElementId()).IsValid())
        {
        RenderMaterialId destParentId = RenderMaterial::QueryMaterialId(importer.GetDestinationDb(), GetCode());
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
RenderMaterial::Iterator RenderMaterial::Iterator::Create(DgnDbR db, Options const& options)
    {
    Utf8String ecsql("SELECT ECInstanceId,CodeValue,PaletteName,Parent.Id,Description FROM " BIS_SCHEMA(BIS_CLASS_RenderMaterial));
    if (options.m_byPalette)
        ecsql.append(" WHERE PaletteName=?");

    if (options.m_byParent)
        ecsql.append(options.m_byPalette ? " AND " : " WHERE ").append("Parent.Id=?");

    if (options.m_ordered)
        ecsql.append(" ORDER BY PaletteName,CodeValue");

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
RenderMaterial::Iterator RenderMaterial::MakeIterator(DgnDbR db, Iterator::Options options)
    {
    return Iterator::Create(db, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialId RenderMaterial::ImportMaterial(RenderMaterialId srcMaterialId, DgnImportContext& importer)
    {
    RenderMaterialCPtr srcMaterial = RenderMaterial::Get(importer.GetSourceDb(), srcMaterialId);
    BeAssert(srcMaterial.IsValid());
    if (!srcMaterial.IsValid())
        return RenderMaterialId();

    DgnModelId destModelId = importer.FindModelId(srcMaterial->GetModelId());
    if (!destModelId.IsValid())
        destModelId = DgnModel::DictionaryId(); // Use the DictionaryModel as a fallback

    DefinitionModelPtr destModel = importer.GetDestinationDb().Models().Get<DefinitionModel>(destModelId);
    if (!destModel.IsValid())
        return RenderMaterialId(); // ERROR: didn't find the destination model

    //  See if we already have a material with the same name in the destination DefinitionModel. If so, we'll map the source material to it.
    RenderMaterialId destMaterialId = QueryMaterialId(*destModel, srcMaterial->GetMaterialName());
    if (destMaterialId.IsValid())
        {
        //  *** TBD: Check if the material definitions match. If not, rename and remap
        //  *** TBD: Make sure that child materials are also remapped? Or, wait for someone to ask for them one by one?
        importer.AddMaterialId(srcMaterialId, destMaterialId);
        return destMaterialId; // SUCCESS: material found by name
        }

    //  No such material in the destination DefinitionModel. Ask the source Material to import itself.
    DgnElementCPtr destMaterial = srcMaterial->Import(nullptr, *destModel, importer);
    if (!destMaterial.IsValid())
        return RenderMaterialId(); // ERROR: Import failed

    destMaterialId = RenderMaterialId(destMaterial->GetElementId().GetValue());
    importer.AddMaterialId(srcMaterialId, destMaterialId);
    return destMaterialId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RenderMaterial::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);

    if (importer.IsBetweenDbs())
        GetRenderingAssetR().Relocate(importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialId DgnImportContext::_RemapRenderMaterialId(RenderMaterialId source)
    {
    if (!IsBetweenDbs())
        return source;

    RenderMaterialId dest = FindRenderMaterialId(source);
    return dest.IsValid() ? dest : RenderMaterial::ImportMaterial(source, *this);
    }
