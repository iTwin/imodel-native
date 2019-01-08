/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportLayers.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

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

    auto& masterDwg = m_importer.GetDwgDb ();
    if (&masterDwg == m_xrefDwg)
        return  layerId;

    DwgDbObjectId               effectiveLayerId;
    DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);

    if (layer.IsNull())
        {
        m_importer.ReportError (IssueCategory::MissingData(), Issue::CantOpenObject(), "failed opening an xref layer!");
        effectiveLayerId = masterDwg.GetLayer0Id ();
        }
    else if (layerId == m_xrefDwg->GetLayer0Id())
        {
        // always use layer "0" in mater file
        effectiveLayerId = masterDwg.GetLayer0Id ();
        }
    else if (layerId == m_xrefDwg->GetLayerDefpointsId())
        {
        // always use layer "defpoints" in mater file
        effectiveLayerId = masterDwg.GetLayerDefpointsId ();
        }
    else if (!masterDwg.GetVISRETAIN())
        {
        // xref layers are not saved in the master file - use the layerId of the xRef:
        if (layerId.GetDatabase() != m_xrefDwg)
            {
            m_importer.ReportError (IssueCategory::UnexpectedData(), Issue::Error(), Utf8PrintfString("Invalid xref layer %ls", layer->GetName().c_str()).c_str());
            effectiveLayerId = masterDwg.GetLayer0Id ();
            }
        else
            {
            effectiveLayerId = layerId;
            }
        }
    else
        {
        // xref layers are saved in the master file - find the layer in the master file by name:
        WString layerName = m_importer.GetCurrentXRefHolder().GetPrefixInRootFile() + WString(L"|") + layer->GetName().c_str();
        DwgDbSymbolTablePtr masterFileLayers(masterDwg.GetLayerTableId(), DwgDbOpenMode::ForRead);
        if (masterFileLayers.IsNull() || !(layerId = masterFileLayers->GetByName(layerName.c_str())).IsValid())
            {
            m_importer.ReportError (IssueCategory::UnexpectedData(), Issue::Error(), Utf8PrintfString("can't find xref layer %ls", layerName.c_str()).c_str());
            effectiveLayerId = masterDwg.GetLayer0Id ();
            }
        else
            {
            effectiveLayerId = layerId;
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

    // set category invisible if the layer is hidden from GUI (not about displaying elements in the category)
    appearance.SetInvisible (layer.IsHidden());

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

    RenderMaterialId   material = this->GetDgnMaterialFor (layer.GetMaterialId());
    if (material.IsValid())
        appearance.SetRenderMaterial(material);

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
BentleyStatus   DwgImporter::_ImportLayer (DwgDbLayerTableRecordCR layer, DwgStringP layerName)
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

    // use the default Unrecognized category for layer "0"
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

    Utf8String      name(nullptr == layerName ? layer.GetName().c_str() : layerName->c_str());
    DgnDbTable::ReplaceInvalidCharacters(name, DgnCategory::GetIllegalCharacters(), '_');
    name.Trim ();

    auto definitionModel = this->GetOptions().GetMergeDefinitions() ? &db.GetDictionaryModel() : this->GetOrCreateJobDefinitionModel().get();
    if (nullptr == definitionModel)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "SpartialCategory");
        return  BSIERROR;
        }

    bool isRoot3d = this->GetRootModel().GetModel()->Is3d ();
    DgnCode categoryCode = isRoot3d ? SpatialCategory::CreateCode(*definitionModel, name.c_str()) : DrawingCategory::CreateCode(*definitionModel, name.c_str());;

    categoryId = isRoot3d ? SpatialCategory::QueryCategoryId(*definitionModel, categoryCode.GetValueUtf8()) : DrawingCategory::QueryCategoryId(*definitionModel, categoryCode.GetValueUtf8());;
    if (categoryId.IsValid())
        {
        if (LOG_LAYER_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LAYER.tracev("merged layer %ls (%llx) -> %s (%d)", layer.GetName().c_str(), layer.GetObjectId().GetHandle().AsUInt64(), name.c_str(), categoryId.GetValue());
        }
    else if (isRoot3d)
        {
        SpatialCategory newCategory (*definitionModel, name, DgnCategory::Rank::User, Utf8String(layer.GetDescription().c_str()));

        newCategory.Insert (appear, &status);

        if (DgnDbStatus::Success != status || !newCategory.GetCategoryId().IsValid())
            {
            BeAssert(DgnDbStatus::DuplicateCode != status && "Caller is responsible for providing a unique layer name");

            this->ReportIssueV (IssueSeverity::Warning, IssueCategory::CorruptData(), Issue::InvalidLayer(), "Spatial Category", name.c_str());
            BeAssert(false);
            return BSIERROR;
            }
        categoryId = newCategory.GetCategoryId ();
        }
    else
        {
        DrawingCategory newCategory (*definitionModel, name, DgnCategory::Rank::User, Utf8String(layer.GetDescription().c_str()));

        newCategory.Insert (appear, &status);

        if (DgnDbStatus::Success != status || !newCategory.GetCategoryId().IsValid())
            {
            this->ReportIssueV (IssueSeverity::Warning, IssueCategory::CorruptData(), Issue::InvalidLayer(), "Drawing Category", name.c_str());
            return BSIERROR;
            }
        categoryId = newCategory.GetCategoryId ();
        }

    if (!categoryId.IsValid())
        return  BSIERROR;

#ifdef ADD_SUBCATEGORIES_WHILE_IMPORTING_LAYERS
    // add another sub-category which has an inversed on/off status of the default sub-category:
    appear.SetInvisible(!appear.IsInvisible());
    this->InsertAlternateSubCategory (DgnSubCategory::Get(db, newCategory.GetDefaultSubCategoryId()), appear);
#endif

    if (LOG_LAYER_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
        LOG_LAYER.tracev("inserted layer (%llx) -> %s (%d)", layer.GetObjectId().GetHandle().AsUInt64(), name.c_str(), categoryId.GetValue());

    m_syncInfo.InsertLayer (DgnCategory::GetDefaultSubCategoryId(categoryId), modelSource, layer);

    m_layersImported++;
    
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  DwgImporter::_ImportLayersByFile (DwgDbDatabaseP dwg)
    {
    if (nullptr == dwg)
        {
        BeAssert (false && "nullptr input DwgDbDatabase!");
        return  0;
        }

    DwgDbLayerTablePtr  layerTable(dwg->GetLayerTableId(), DwgDbOpenMode::ForRead);
    if (layerTable.IsNull())
        return  BSIERROR;

    DwgSyncInfo::DwgFileId      dwgfileId = DwgSyncInfo::GetDwgFileId (*dwg);
    DwgDbSymbolTableIteratorPtr iter = layerTable->NewIterator ();

    if (!iter.IsValid() || !iter->IsValid())
        return  BSIERROR;

    // include hidden and reconciled layers
    iter->SetSkipHiddenLayers (false);
    iter->SetSkipReconciledLayers (false);

    size_t      count = 0;
    DwgString   xrefName;
    if (dwg != m_dwgdb.get())
        xrefName.Assign (BeFileName::GetFileNameWithoutExtension(dwg->GetFileName().c_str()).c_str());

    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbLayerTableRecordPtr    layer(iter->GetRecordId(), DwgDbOpenMode::ForRead);
        if (layer.IsNull())
            {
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CantOpenObject(), "LayerRecord");
            continue;
            }

        if (this->IsUpdating())
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

        bool        overrideName = false;
        DwgString   name = layer->GetName ();
        if (name.IsEmpty())
            {
            this->ReportIssue (IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::ConfigUsingDefault(), DwgHelper::ToUtf8CP(name));
            name = L"Unnamed";
            overrideName = true;
            }
        else if (!xrefName.IsEmpty())
            {
            // xref layers "0" and "Defpoints" use the same layers of the master file:
            if (name.EqualsI(L"0") || name.EqualsI(L"DefPoints"))
                continue;

            // build an xref layer name, "xrefName|layerName"
            name.Insert (0, L"|");
            name.Insert (0, xrefName.c_str());
            overrideName = true;
            }

        this->_ImportLayer (*layer.get(), overrideName ? &name : nullptr);
        }

    return  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportLayerSection ()
    {
    this->SetStepName (ProgressMessage::STEP_IMPORTING_LAYERS());

    // import layers from the master file:
    size_t  numLayers = this->_ImportLayersByFile (m_dwgdb.get());

    // when VISRETAIN=0, we will also have to import xRef layers:
    if (m_dwgdb->GetVISRETAIN())
        return  BSISUCCESS;

    // use xref files cached from prior model discovery phase:
    for (auto xrefHolder : m_loadedXrefFiles)
        numLayers += this->_ImportLayersByFile (xrefHolder.GetDatabaseP());
    
    if (LOG_LAYER_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
        LOG_LAYER.tracev("Total layers imported => %d", numLayers);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::InitUncategorizedCategory ()
    {
    static Utf8CP s_name = "0";
    DefinitionModelP    definitionModel = this->GetOrCreateJobDefinitionModel().get ();
    if (nullptr == definitionModel)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "SpartialCategory");
        BeAssert (false && "Invalid dictionary model for uncategorized category");
        return;
        }

    // allow an app to create a 2d model representing the modelspace
    bool isRoot3d = true;
    DwgDbBlockTableRecordPtr    modelspace(m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (modelspace.OpenStatus() == DwgDbStatus::Success)
        isRoot3d = m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME,BIS_CLASS_PhysicalModel) == this->_GetModelType(*modelspace);

    if (isRoot3d)
        {
        DgnCode categoryCode = SpatialCategory::CreateCode (*definitionModel, s_name);
        m_uncategorizedCategoryId = SpatialCategory::QueryCategoryId (*definitionModel, categoryCode.GetValueUtf8());
        if (!m_uncategorizedCategoryId.IsValid() && this->GetOptions().GetMergeDefinitions())
            {
            // try BisCore.Dictionary model for "0" which might be created from DgnV8Bridge:
            categoryCode = SpatialCategory::CreateCode (m_dgndb->GetDictionaryModel(), s_name);
            m_uncategorizedCategoryId = SpatialCategory::QueryCategoryId (m_dgndb->GetDictionaryModel(), categoryCode.GetValueUtf8());
            }
        }
    else
        {
        DgnCode categoryCode = DrawingCategory::CreateCode (*definitionModel, s_name);
        m_uncategorizedCategoryId = DrawingCategory::QueryCategoryId (*definitionModel, categoryCode.GetValueUtf8());
        if (!m_uncategorizedCategoryId.IsValid() && this->GetOptions().GetMergeDefinitions())
            {
            // try BisCore.Dictionary model for "0" which might be created from DgnV8Bridge:
            categoryCode = DrawingCategory::CreateCode (m_dgndb->GetDictionaryModel(), s_name);
            m_uncategorizedCategoryId = DrawingCategory::QueryCategoryId (m_dgndb->GetDictionaryModel(), categoryCode.GetValueUtf8());
            }
        }

    if (m_uncategorizedCategoryId.IsValid())
        return;

    if (isRoot3d)
        {
        SpatialCategory category (*definitionModel, s_name, DgnCategory::Rank::Application);
        if (!category.Insert(DgnSubCategory::Appearance()).IsValid())
            BeAssert(false);
        m_uncategorizedCategoryId = category.GetCategoryId();
        }
    else
        {
        DrawingCategory category (*definitionModel, s_name, DgnCategory::Rank::Application);
        if (!category.Insert(DgnSubCategory::Appearance()).IsValid())
            BeAssert(false);
        m_uncategorizedCategoryId = category.GetCategoryId();
        }
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
    auto& db = model.GetDgnDb ();
    auto definitionModel = this->GetOptions().GetMergeDefinitions() ? &db.GetDictionaryModel() : this->GetOrCreateJobDefinitionModel().get();
    if (nullptr == definitionModel)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "DrawingCategory");
        return DgnCategoryId();
        }

    DgnCode         code = DrawingCategory::CreateCode (*definitionModel, name.c_str());
    DgnCategoryId   categoryId = DrawingCategory::QueryCategoryId (*definitionModel, code.GetValueUtf8());
    if (categoryId.IsValid())
        {
        // a drawing category found - check sub-category:
        DgnSubCategory::Appearance lastAppearance;
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
            lastAppearance = subcategory->GetAppearance ();
            }

        if (!subCategoryId.IsValid())
            {
            // no matching sub-category found - add a new one for the desired appearance:
            Utf8String suffix = DwgHelper::CompareSubcatAppearance (lastAppearance, appear);
            if (!suffix.empty())
                name += suffix;
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
    DrawingCategory category(*definitionModel, name, DgnCategory::Rank::Application, description);
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

        this->ReportIssueV (IssueSeverity::Warning, IssueCategory::CorruptData(), Issue::InvalidLayer(), nullptr, name.c_str());
        }

    return  categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId   DwgImporter::GetSpatialCategory (DgnSubCategoryId& subCategoryId, DwgDbObjectIdCR entityLayer, DwgDbDatabaseP xrefDwg)
    {
    /*-----------------------------------------------------------------------------------
    This method finds category and sub-categeory from the syncInfo for a layer.  If found,
    it caches the entry so next time when the same layer is requested it will be retrieved
    from the cache.  This is done for the sake of performance.
    -----------------------------------------------------------------------------------*/
    DwgDbObjectId   layerId = entityLayer;
    if (nullptr != xrefDwg && xrefDwg != m_dwgdb.get())
        {
        // entity is in an xRef file - find the layer in master file by layer name:
        XRefLayerResolver xresolver (*this, xrefDwg);
        layerId = xresolver.ResolveEntityLayer (entityLayer);
        }

    // retieve layer from the cache
    auto found = m_layersInSync.find (layerId);
    if (found != m_layersInSync.end())
        {
        // a sync layer found in cache
        subCategoryId = found->second.GetSubCategoryId ();
        return  found->second.GetCategoryId ();
        }

    // not cached yet - querying them from the syncInfo for the resolved master file layer:
    auto categoryId = this->FindCategoryFromSyncInfo (layerId);
    subCategoryId = this->FindSubCategoryFromSyncInfo (layerId);

    if (!categoryId.IsValid() || !subCategoryId.IsValid())
        {
        // should not reach here - error out!
        Utf8String  layerName("???");
        DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);
        if (layer.OpenStatus() == DwgDbStatus::Success)
            layerName.Assign (layer->GetName().c_str());
        this->ReportIssueV (IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::MissingCategory(), nullptr, layerName.c_str());

        // fallback to the default category:
        categoryId = this->GetUncategorizedCategory ();
        subCategoryId = DgnCategory::GetDefaultSubCategoryId (this->GetUncategorizedCategory());
        }

    // cache the sync'ed layer-category mapping for fast retrieving
    CategoryEntry   entry(categoryId, subCategoryId);
    m_layersInSync.insert (T_DwgDgnLayer(layerId, entry));

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

        bool isLayerInXref = layerId.GetDatabase() == xrefDwg;
        auto dwgfileId = DwgSyncInfo::DwgFileId::GetFrom (isLayerInXref ? *xrefDwg : *m_dwgdb);

        if (layerId.IsValid())
            categoryId = syncInfo.FindCategory (layerId, dwgfileId);
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

