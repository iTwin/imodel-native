/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   XRefLayerResolver::ResolveEntityLayer (DwgDbObjectId layerId) const
    {
    // if the layer is in the master file, there is nothing to resolve:
    if (nullptr == m_xrefDwg)
        return  layerId;

    auto& masterDwg = m_importer.GetDwgDb ();
    if (&masterDwg == m_xrefDwg)
        return  layerId;

    DwgDbObjectId   effectiveLayerId;
    if (!layerId.IsValid())
        {
        m_importer.ReportError (IssueCategory::MissingData(), Issue::CantOpenObject(), "failed opening an xref layer!");
        effectiveLayerId = masterDwg.GetLayer0Id ();
        }
    else if (layerId == m_xrefDwg->GetLayer0Id())
        {
        // always use layer "0" in master file
        effectiveLayerId = masterDwg.GetLayer0Id ();
        }
    else if (layerId == m_xrefDwg->GetLayerDefpointsId())
        {
        // always use layer "defpoints" in master file
        effectiveLayerId = masterDwg.GetLayerDefpointsId ();
        }
    else
        {
        // keep xRef layerId
        effectiveLayerId = layerId;
        }
    return  effectiveLayerId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool    XRefLayerResolver::SearchXrefLayerByName (DwgDbObjectIdR layerId, Utf8StringCR masterLayerName)
    {
    // search all xRef files to find a layer matching the input layer from master file
    auto foundAt = masterLayerName.find ("|");
    if (foundAt == Utf8String::npos)
        return  false;

    Utf8String xblockName = masterLayerName.substr(0, foundAt);
    if (xblockName.empty())
        return  false;

    WString xlayerName;
    xlayerName.AssignUtf8 (masterLayerName.substr(foundAt+1).c_str());
    if (xlayerName.empty())
        return  false;

    auto loadedXrefs = m_importer.GetLoadedXrefs ();
    for (auto xref : loadedXrefs)
        {
        Utf8String  prefix(xref.GetPrefixInRootFile().c_str());
        if (xblockName.EqualsI(prefix))
            {
            // the layer name prefix matches the ref block name prefix in xref holder
            m_xrefDwg = xref.GetDatabaseP();
            if (m_xrefDwg == nullptr)
                continue;

            DwgDbLayerTablePtr xrefLayers(m_xrefDwg->GetLayerTableId(), DwgDbOpenMode::ForRead);
            if (xrefLayers.OpenStatus() != DwgDbStatus::Success)
                continue;

            auto xrefLayerId = xrefLayers->GetByName(xlayerName);
            if (xrefLayerId.IsValid())
                {
                // layer found in xref, update the output layer ID
                layerId = xrefLayerId;
                return  true;
                }
            }
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool    XRefLayerResolver::SearchMasterLayerFromXrefLayer (DwgDbObjectIdR masterLayerId, DwgDbObjectIdCR xrefLayerId) const
    {
    // Lookup master file's layer table from an xRef layerId
    masterLayerId = this->ResolveEntityLayer (xrefLayerId);
    if (masterLayerId != xrefLayerId)
        return  true;

    if (xrefLayerId.GetDatabase() == m_importer.GetDwgDbP())
        {
        masterLayerId = xrefLayerId;
        return  true;
        }

    DwgDbLayerTableRecordPtr    xrefLayer(xrefLayerId, DwgDbOpenMode::ForRead);
    if (xrefLayer.OpenStatus() != DwgDbStatus::Success)
        return  false;

    DwgDbLayerTablePtr  masterLayerTable(m_importer.GetDwgDb().GetLayerTableId(), DwgDbOpenMode::ForRead);
    if (masterLayerTable.OpenStatus() != DwgDbStatus::Success)
        return  false;
    auto iter = masterLayerTable->NewIterator();
    if (!iter.IsValid() || !iter->IsValid())
        return  false;

     // try match xref layer in master layer table
    Utf8String xrefLayerName(xrefLayer->GetName().c_str());
    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbLayerTableRecordPtr masterLayer(iter->GetRecordId(), DwgDbOpenMode::ForRead);
        if (masterLayer.OpenStatus() != DwgDbStatus::Success)
            continue;

        Utf8String masterLayerName(masterLayer->GetName().c_str());
        auto foundAt = masterLayerName.find ("|");
        if (foundAt == Utf8String::npos)
            continue;

        auto xlayerName = masterLayerName.substr(foundAt+1);
        if (xrefLayerName.EqualsI(xlayerName.c_str()))
            {
            masterLayerId = iter->GetRecordId();
            return  true;
            }
        }
    return  false;
    }

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

    auto layerId = layer.GetObjectId ();
    auto& db = this->GetDgnDb ();
    DgnDbStatus status;

    // use the default Unrecognized category for layer "0"
    DgnCategoryId   categoryId = this->GetUncategorizedCategory ();
    if (layerId == dwg->GetLayer0Id())
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

        // add an ExternalSourceAspect
        m_sourceAspects.AddLayerAspect(subId, layerId);
        // also cache the mapping
        CategoryEntry   entry(categoryId, subId, layerId);
        m_layersInSync.insert (T_DwgDgnLayer(layerId, entry));

#ifdef ADD_SUBCATEGORIES_WHILE_IMPORTING_LAYERS
        // add another sub-category which has an inversed on/off status of the default sub-category:
        appear.SetInvisible(!appear.IsInvisible());
        this->InsertAlternateSubCategory (subCat, appear);
#endif

        if (LOG_LAYER_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
            LOG_LAYER.tracev("merged layer %ls (%llx) -> %d", layer.GetName().c_str(), layerId.GetHandle().AsUInt64(), categoryId.GetValue());
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
            LOG_LAYER.tracev("merged layer %ls (%llx) -> %s (%d)", layer.GetName().c_str(), layerId.GetHandle().AsUInt64(), name.c_str(), categoryId.GetValue());
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
        LOG_LAYER.tracev("inserted layer (%llx) -> %s (%d)", layerId.GetHandle().AsUInt64(), name.c_str(), categoryId.GetValue());

    /*-----------------------------------------------------------------------------------
    Ready to create an ExternalSourceAspect for the layer.  If the layer depends on an xRef,
    find the layer in that xRef file.  It may sound like an expensive operation, but when 
    compared this operation to searching for a layer in master file in reverse when an entity 
    in an xRef is drawn, this is much cheaper.  Number of layers are generally less than 
    number of entities.
    -----------------------------------------------------------------------------------*/
    name.Assign (layer.GetName().c_str());
    XRefLayerResolver xrefResolver(*this, nullptr);

    auto layerIdInMasterFile = layerId;

    bool matched = xrefResolver.SearchXrefLayerByName (layerId, name);

    DgnSubCategoryId    subcategoryId = DgnCategory::GetDefaultSubCategoryId (categoryId);
    m_sourceAspects.AddLayerAspect(subcategoryId, layerId);

    // also cache the mapping for fast future retrieving
    CategoryEntry   entry(categoryId, subcategoryId, layerIdInMasterFile);
    m_layersInSync.insert (T_DwgDgnLayer(layerId, entry));

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

    DwgDbSymbolTableIteratorPtr iter = layerTable->NewIterator ();

    if (!iter.IsValid() || !iter->IsValid())
        return  BSIERROR;

    // include hidden and reconciled layers
    iter->SetSkipHiddenLayers (false);
    iter->SetSkipReconciledLayers (false);

    size_t  count = 0;
    bool    isXref = dwg != m_dwgdb.get();

    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbLayerTableRecordPtr    layer(iter->GetRecordId(), DwgDbOpenMode::ForRead);
        if (layer.IsNull())
            {
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CantOpenObject(), "LayerRecord");
            continue;
            }

        DwgString   name = layer->GetName ();
        if (this->IsUpdating())
            {
            auto layerId = layer->GetObjectId();
            auto layerIdInMasterFile = layerId;
            if (!isXref && name.Find(L"|") > 1)
                {
                // this layer is from an xRef - find the layer ID from in the xRef file:
                XRefLayerResolver xrefResolver(*this, nullptr);
                xrefResolver.SearchXrefLayerByName (layerId, Utf8String(name.c_str()));
                }

            DgnSubCategoryId subcategoryId;
            DgnCategoryId   categoryId = m_sourceAspects.FindCategory(&subcategoryId, layerId);
            if (categoryId.IsValid())
                {
                this->_OnUpdateLayer (categoryId, *layer.get());
                // cache the layer mapping
                CategoryEntry   entry(categoryId, subcategoryId, layerIdInMasterFile);
                m_layersInSync.insert (T_DwgDgnLayer(layerId, entry));
                continue;
                }
            }

        if ((count++ % 100) == 0)
            this->Progress ();

        bool        overrideName = false;
        if (name.IsEmpty())
            {
            this->ReportIssue (IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::ConfigUsingDefault(), DwgHelper::ToUtf8CP(name));
            name = L"Unnamed";
            overrideName = true;
            }
        else if (isXref)
            {
            // xref layers "0" and "Defpoints" use the same layers of the master file:
            if (name.EqualsI(L"0") || name.EqualsI(L"DefPoints"))
                continue;
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
    auto definitionModel = this->GetOptions().GetMergeDefinitions() ? &m_dgndb->GetDictionaryModel() : this->GetOrCreateJobDefinitionModel().get();
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
        }
    else
        {
        DgnCode categoryCode = DrawingCategory::CreateCode (*definitionModel, s_name);
        m_uncategorizedCategoryId = DrawingCategory::QueryCategoryId (*definitionModel, categoryCode.GetValueUtf8());
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
    Spetial layers in xRef share the same layers in master file, resolve these layers.
    The layer-category mapping should have been cached in _ImportLayer, so retrieving 
    the entry should be fast.
    -----------------------------------------------------------------------------------*/
    DwgDbObjectId   layerId = entityLayer;
    bool isXrefLayer = nullptr != xrefDwg && xrefDwg != m_dwgdb.get();
    if (isXrefLayer)
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
    auto categoryId = m_sourceAspects.FindCategory (&subCategoryId, layerId);

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

    auto masterLayerId = layerId;
    if (isXrefLayer)
        {
        // the default bridge workflow should not reach here, but just in case an app skips the default _ImportLayer
        XRefLayerResolver   resolver(*this, xrefDwg);
        if (!resolver.SearchMasterLayerFromXrefLayer(masterLayerId, layerId))
            {
            BeAssert (false && "Xref layer does not exist in master file");
            masterLayerId = m_dwgdb->GetLayer0Id ();
            }
        }

    // cache the sync'ed layer-category mapping for fast retrieving
    CategoryEntry   entry(categoryId, subCategoryId, masterLayerId);
    m_layersInSync.insert (T_DwgDgnLayer(layerId, entry));

    return  categoryId;
    }
