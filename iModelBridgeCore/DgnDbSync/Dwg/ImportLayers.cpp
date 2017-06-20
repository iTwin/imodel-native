/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportLayers.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DGNDBSYNC_DWG

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/16
+===============+===============+===============+===============+===============+======*/
struct    XRefLayerResolver
{
private:
    DwgImporter&        m_importer;
    DwgDbDatabaseP      m_xrefDwg;

public:
    XRefLayerResolver (DwgImporter& importer, DwgDbDatabaseP xref) : m_importer(importer), m_xrefDwg(xref) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   ResolveEntityLayer (DwgDbObjectId layerId)
    {
    // if the layer is in the master file, there is nothing to resolve:
    if (nullptr == m_xrefDwg)
        return  layerId;

    DwgDbObjectId               effectiveLayerId;
    DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);

    if (layer.IsNull())
        {
        m_importer.ReportError (DwgImporter::IssueCategory::MissingData(), DwgImporter::Issue::CantOpenObject(), "failed opening an xref layer!");
        }
    else if (layerId == m_xrefDwg->GetLayer0Id())
        {
        // always use layer "0" in mater file
        effectiveLayerId = m_importer.GetDwgDb().GetLayer0Id ();
        }
    else if (layerId == m_xrefDwg->GetLayerDefpointsId())
        {
        // always use layer "defpoints" in mater file
        effectiveLayerId = m_importer.GetDwgDb().GetLayerDefpointsId ();
        }
    else
        {
        WString layerName = m_importer.GetCurrentXRefHolder().GetPrefixInRootFile() + WString(L"|") + layer->GetName().c_str();
        DwgDbSymbolTablePtr masterFileLayers(m_importer.GetDwgDb().GetLayerTableId(), DwgDbOpenMode::ForRead);
        if (masterFileLayers.IsNull() || DwgDbStatus::Success != masterFileLayers->GetByName(layerId, layerName.c_str()))
            {
            m_importer.ReportError (DwgImporter::IssueCategory::UnexpectedData(), DwgImporter::Issue::Error(), Utf8PrintfString("can't find xref layer %ls", layerName.c_str()).c_str());
            layerId = m_importer.GetDwgDb().GetLayer0Id ();
            }
        }
    return  effectiveLayerId;
    }
}; // XRefLayerResolver

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::GetLayerAppearance (DgnSubCategory::Appearance& appearance, DwgDbLayerTableRecordCR layer, DwgDbObjectIdCP viewportId)
    {
    bool    applyOverrides = nullptr != viewportId && viewportId->IsValid() && layer.HasOverrides(*viewportId);
    bool    isOverridden = false;

    appearance.SetInvisible (layer.IsOff() || layer.IsFrozen());
    if (applyOverrides && appearance.IsVisible())
        {
        // apply viewport freez
        DwgDbViewportPtr    viewport(*viewportId, DwgDbOpenMode::ForRead);
        if (viewport.OpenStatus() == DwgDbStatus::Success && viewport->IsLayerFrozen(layer.GetObjectId()))
            appearance.SetInvisible (true);
        }

    DwgTransparency transparency = applyOverrides ? layer.GetTransparency(isOverridden, *viewportId) : layer.GetTransparency();
    appearance.SetTransparency (DwgHelper::GetTransparencyFromDwg(transparency));

    DwgCmEntityColor    color = applyOverrides ? layer.GetColor(isOverridden, *viewportId) : layer.GetColor();
    ColorDef            dgnColor;
    if (color.IsByACI())
        dgnColor = DwgHelper::GetColorDefFromACI(color.GetIndex());
    else
        dgnColor.SetColors(color.GetRed(), color.GetGreen(), color.GetBlue(), 0);
    appearance.SetColor (dgnColor);

    DwgDbLineWeight     lineweight = applyOverrides ? layer.GetLineweight(isOverridden, *viewportId) : layer.GetLineweight();
    appearance.SetWeight (this->GetOptions().GetDgnLineWeight(lineweight));

    DgnStyleId      styleId = this->GetDgnLineStyleFor (applyOverrides ? layer.GetLinetypeId(isOverridden, *viewportId) : layer.GetLinetypeId());
    if (styleId.IsValid())
        appearance.SetStyle (styleId);

    DgnMaterialId   material = this->GetDgnMaterialFor (layer.GetMaterialId());
    if (material.IsValid())
        appearance.SetMaterial(material);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId    DwgImporter::InsertAlternateSubCategory (DgnSubCategoryCPtr subcategory, DgnSubCategory::Appearance const& appearance, Utf8CP desiredName)
    {
    // create a new sub-category next to the input sub-category with a different appearance:
    DgnSubCategoryId    newSubcatId;
    if (!subcategory.IsValid() || subcategory->GetAppearance().IsEqual(appearance))
        return  newSubcatId;

    Utf8String  name;
    if (nullptr == desiredName)
        name.Sprintf ("%s-%s", subcategory->GetSubCategoryName().c_str(), appearance.IsVisible() ? "on" : "off");
    else
        name.assign (desiredName);

    DgnCategoryId   categoryId = subcategory->GetCategoryId ();
    DgnSubCategory  newSubcat (DgnSubCategory::CreateParams(*m_dgndb, categoryId, name, appearance, subcategory->GetDescription()));

    auto    inserted = newSubcat.Insert ();
    if (inserted.IsValid())
        newSubcatId = inserted->GetSubCategoryId ();

    if (!newSubcatId.IsValid())
        BeAssert (false && "failed inserting a new sub-category!");

    return  newSubcatId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportLayer (DwgDbLayerTableRecordCR layer)
    {
    // import a DWG layer as a SpatialCategory
    DwgDbDatabaseP  dwg = layer.GetDatabase().get ();
    if (nullptr == dwg)
        {
        BeAssert(false);
        return BSIERROR;
        }
        
    DgnSubCategory::Appearance  appear;
    this->GetLayerAppearance (appear, layer);

    DgnDbR      db = this->GetDgnDb ();
    DgnDbStatus status;
    // use dictionary model(0) for layers in the syncInfo
    DwgSyncInfo::DwgModelSource     modelSource (DwgSyncInfo::GetDwgFileId(*dwg));

    // use the default Unrecognized level for layer "0"
    DgnCategoryId   categoryId = this->GetUncategorizedCategory ();
    if (layer.GetObjectId() == dwg->GetLayer0Id())
        {
        DgnSubCategoryId    subId = DgnCategory::GetDefaultSubCategoryId (categoryId);
        DgnSubCategoryCPtr  subCat = DgnSubCategory::Get (db, subId);
        if (!subCat.IsValid())
            {
            BeAssert (false && L"Can't open default sub-category of Unrecognized category!");
            return  BSIERROR;
            }

        DgnSubCategoryPtr   writeSubcat = subCat->MakeCopy<DgnSubCategory> ();
        if (writeSubcat.IsValid())
            {
            writeSubcat->GetAppearanceR() = appear;
            if (!writeSubcat->Update(&status).IsValid())
                this->ReportError (IssueCategory::VisualFidelity(), Issue::Error(), "failed setting layer 0's appearance!");
            }

        m_syncInfo.InsertLayer (subId, modelSource, layer);

#ifdef ADD_SUBCATEGORIES_WHILE_IMPORTING_LAYERS
        // add another sub-category which has an inversed on/off status of the default sub-category:
        appear.SetInvisible(!appear.IsInvisible());
        this->InsertAlternateSubCategory (subCat, appear);
#endif

        if (LOG_LAYER_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
            LOG_LAYER.tracev("merged layer %ls (%llx) -> %d", layer.GetName().c_str(), layer.GetObjectId().GetHandle().AsUInt64(), categoryId.GetValue());
        return  BSISUCCESS;
        }

    Utf8String      name(DwgHelper::ToUtf8CP(layer.GetName(), true));
    DgnDbTable::ReplaceInvalidCharacters(name, DgnCategory::GetIllegalCharacters(), '_');
    name.Trim ();

    DgnCode         categoryCode = SpatialCategory::CreateCode (db, name.c_str());

    categoryId = DgnCategory::QueryCategoryId (db, categoryCode);
    if (categoryId.IsValid())
        {
        if (LOG_LAYER_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LAYER.tracev("merged layer %ls (%llx) -> %s (%d)", layer.GetName().c_str(), layer.GetObjectId().GetHandle().AsUInt64(), name.c_str(), categoryId.GetValue());
        }
    else
        {
        SpatialCategory newCategory (db, name, DgnCategory::Rank::User, Utf8String(layer.GetDescription().c_str()));

        newCategory.Insert (appear, &status);

        if (DgnDbStatus::Success != status || !newCategory.GetCategoryId().IsValid())
            {
            BeAssert(DgnDbStatus::DuplicateCode != status && "Caller is responsible for providing a unique layer name");

            if (LOG_LAYER_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
                LOG_LAYER.tracev("failed to insert layer %ls (%llx) as %s", layer.GetName().c_str(), layer.GetObjectId().GetHandle().AsUInt64(), name.c_str());

            this->ReportIssueV (IssueSeverity::Warning, IssueCategory::CorruptData(), Issue::InvalidLayer(), name.c_str());
            BeAssert(false);
            return BSIERROR;
            }
        
        categoryId = newCategory.GetCategoryId ();

#ifdef ADD_SUBCATEGORIES_WHILE_IMPORTING_LAYERS
        // add another sub-category which has an inversed on/off status of the default sub-category:
        appear.SetInvisible(!appear.IsInvisible());
        this->InsertAlternateSubCategory (DgnSubCategory::Get(db, newCategory.GetDefaultSubCategoryId()), appear);
#endif

        if (LOG_LAYER_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LAYER.tracev("inserted layer (%llx) -> %s (%d)", layer.GetObjectId().GetHandle().AsUInt64(), name.c_str(), categoryId.GetValue());
        }

    m_syncInfo.InsertLayer (DgnCategory::GetDefaultSubCategoryId(categoryId), modelSource, layer);
    
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportLayerSection ()
    {
    DwgDbLayerTablePtr  layerTable(m_dwgdb->GetLayerTableId(), DwgDbOpenMode::ForRead);
    if (layerTable.IsNull())
        return  BSIERROR;

    DwgSyncInfo::DwgFileId      dwgfileId = DwgSyncInfo::GetDwgFileId (*m_dwgdb.get());
    DwgDbSymbolTableIterator    iter = layerTable->NewIterator ();

    if (!iter.IsValid())
        return  BSIERROR;

    this->SetStepName (ProgressMessage::STEP_IMPORTING_LAYERS());

    uint32_t    count = 0;

    for (iter.Start(); !iter.Done(); iter.Step())
        {
        DwgDbLayerTableRecordPtr    layer(iter.GetRecordId(), DwgDbOpenMode::ForRead);
        if (layer.IsNull())
            {
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CantOpenObject(), "LayerRecord");
            continue;
            }

        if (this->_IsUpdating())
            {
            DgnCategoryId   categoryId = m_syncInfo.FindCategory (layer->GetObjectId(), dwgfileId);
            if (categoryId.IsValid())
                {
                this->_OnUpdateLayer (categoryId, *layer.get());
                continue;
                }
            }

        if ((count++ % 100) == 0)
            this->Progress ();

        DwgString       name = layer->GetName ();
        if (name.IsEmpty())
            {
            this->ReportIssue (IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::ConfigUsingDefault(), DwgHelper::ToUtf8CP(name));
            name = L"Unnamed";
            }

        this->_ImportLayer (*layer.get());
        }
    
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::InitUncategorizedCategory ()
    {
    static Utf8CP name = "0";
    DgnCode categoryCode = SpatialCategory::CreateCode (*m_dgndb, name, "");
    m_uncategorizedCategoryId = DgnCategory::QueryCategoryId (*m_dgndb, categoryCode);

    if (m_uncategorizedCategoryId.IsValid())
        return;

    SpatialCategory category (*m_dgndb, categoryCode, DgnCategory::Rank::Application);
    if (!category.Insert(DgnSubCategory::Appearance()).IsValid())
        {
        BeAssert(false);
        }

    m_uncategorizedCategoryId = category.GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId   DwgImporter::GetOrAddDrawingCategory (DgnSubCategoryId& subCategoryId, DwgDbObjectIdCR layerId, DwgDbObjectIdCR viewportId, DgnModelCR model, DwgDbDatabaseP xrefDwg)
    {
    // resolve effective layer
    Utf8String          name, description;
    XRefLayerResolver   xresolver (*this, xrefDwg);

    // get layer info
    DgnSubCategory::Appearance  appear;
    DwgDbLayerTableRecordPtr    layer (xresolver.ResolveEntityLayer(layerId), DwgDbOpenMode::ForRead);
    if (layer.OpenStatus() == DwgDbStatus::Success)
        {
        name.Assign (layer->GetName().c_str());
        this->GetLayerAppearance (appear, *layer, &viewportId);
        description.Assign (layer->GetDescription());
        }
    else
        {
        // default to layer "0":
        name.assign ("0");
        }
    
    DgnDbTable::ReplaceInvalidCharacters (name, DgnCategory::GetIllegalCharacters(), '_');

    // create a draw category code & retrieve the category from the db:
    DgnDbR          db = model.GetDgnDb ();
    DgnCode         code = DrawingCategory::CreateCode (db.GetDictionaryModel(), name.c_str());
    DgnCategoryId   categoryId = DgnCategory::QueryCategoryId (db, code);
    if (categoryId.IsValid())
        {
        // a drawing category found - check sub-category:
        subCategoryId.Invalidate ();

        for (auto entry : DgnSubCategory::MakeIterator(db, categoryId))
            {
            DgnSubCategoryCPtr  subcategory = DgnSubCategory::Get (db, entry.GetId<DgnSubCategoryId>());
            if (subcategory.IsValid() && subcategory->GetAppearance().IsEqual(appear))
                {
                // a matching sub-category found - use it:
                subCategoryId = subcategory->GetSubCategoryId ();
                break;
                }
            }

        if (!subCategoryId.IsValid())
            {
            // no matching sub-category found - add a new one for the desired appearance:
            DgnSubCategory  newSubcat (DgnSubCategory::CreateParams(db, categoryId, name, appear));
            if (newSubcat.Insert().IsValid())
                subCategoryId = newSubcat.GetSubCategoryId ();
            else
                BeAssert (false && "failed inserting a new sub-category to db!");
            }

        return categoryId;
        }

    // create a new drawing category:
    DgnDbStatus     status = DgnDbStatus::Success;
    DrawingCategory category(db.GetDictionaryModel(), name, DgnCategory::Rank::Application, description);
    if (category.Insert(appear, &status).IsValid() && DgnDbStatus::Success == status)
        {
        categoryId = category.GetCategoryId ();
        subCategoryId = DgnCategory::GetDefaultSubCategoryId (categoryId);
        }
    else
        {
        BeAssert (false && "failed inserting a new DrawingCategory to bim!");

        if (LOG_LAYER_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LAYER.tracev("failed to insert layer %ls (%llx) as a DrawingCategory", name.c_str(), layerId.ToUInt64());

        this->ReportIssueV (IssueSeverity::Warning, IssueCategory::CorruptData(), Issue::InvalidLayer(), name.c_str());
        }

    return  categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId   DwgImporter::FindCategoryFromSyncInfo (DwgDbObjectIdCR entityLayerId, DwgDbDatabaseP xrefDwg)
    {
    // find spatial category from syncInfo for the input layer
    auto&           syncInfo = this->GetSyncInfo ();
    DwgDbObjectId   layerId = entityLayerId;
    DgnCategoryId   categoryId;

    if (nullptr != xrefDwg && xrefDwg != m_dwgdb.get())
        {
        // entity is in an xref file - find category by layer name
        XRefLayerResolver xresolver (*this, xrefDwg);
        layerId = xresolver.ResolveEntityLayer (layerId);

        if (layerId.IsValid())
            categoryId = syncInfo.FindCategory (layerId, DwgSyncInfo::DwgFileId::GetFrom(*m_dwgdb));
        else
            categoryId = this->GetUncategorizedCategory ();
        }
    else
        {
        // entity is in the master file
        categoryId = syncInfo.GetCategory (layerId, m_dwgdb.get());
        }

    return  categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId    DwgImporter::FindSubCategoryFromSyncInfo (DwgDbObjectIdCR layerId, DwgDbDatabaseP xrefDwg)
    {
    // find spatial sub-category from syncInfo for the input layer
    DwgDbDatabaseP  dwg = nullptr == xrefDwg ? layerId.GetDatabase() : xrefDwg;
    if (nullptr == dwg)
        {
        BeAssert (false && "layer querying sub-category from syncInfo must be a database resident!");
        dwg = m_dwgdb.get ();
        }

    auto&   syncInfo = this->GetSyncInfo ();

    // query the default sub-category from the syncInfo
    DwgSyncInfo::DwgModelSource     modelSource (DwgSyncInfo::GetDwgFileId(*dwg));
    return  modelSource.IsValid() ? syncInfo.GetSubCategory(layerId, modelSource) : DgnSubCategoryId();
    }

