/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct EntityImportPreparer
{
enum class Modifiers
    {
    None                = 0,
    RemovedGradient     = (0x0001 << 0),
    FilteredBlockRef    = (0x0001 << 1),
    ThawedLayer         = (0x0001 << 2),
    TurnedLayerOn       = (0x0001 << 3),
    };  // Modifiers

private:
    DwgDbEntityP    m_entity;
    uint32_t    m_modifiers;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
EntityImportPreparer (DrawParameters& drawParams, GeometryFactory& factory, DwgDbEntityP entity)
    {
    m_entity = entity;
    m_modifiers = static_cast<uint32_t>(Modifiers::None);
    if (m_entity == nullptr)
        return;

    // resolve the root effective ByBlock symbology:
    drawParams.ResolveRootEffectiveByBlockSymbology ();

    // prior to RealDWG 2019, gradient hatch causes the toolkit draw to create fine mesh, so we used to remove gradient.
    // since 2019, both gradient and solid fill hatches are drawn as fine mesh, so we handle that in a protocol extension, no longer need to remove gradient.

    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(entity);
    if (nullptr != insert)
        {
        // if a block reference is clipped, get the clipper and set it into the geometry factory:
        DwgDbSpatialFilterPtr   spatialFilter;
        if (DwgDbStatus::Success == insert->OpenSpatialFilter(spatialFilter, DwgDbOpenMode::ForRead))
            {
            factory.SetSpatialFilter (spatialFilter.get());
            m_modifiers |= static_cast<uint32_t>(Modifiers::FilteredBlockRef);
            }
        }

    if (entity->IsAProxy())
        {
        // turn on the layer of a proxy entity such that it will draw all sub entities - VSTS 119379
        DwgDbLayerTableRecordPtr layer(entity->GetLayerId(), DwgDbOpenMode::ForWrite);
        if (layer.OpenStatus() == DwgDbStatus::Success)
            {
            if (layer->IsFrozen() && layer->SetIsFrozen(false) == DwgDbStatus::Success)
                m_modifiers |= static_cast<uint32_t>(Modifiers::ThawedLayer);
            if (layer->IsOff() && layer->SetIsOff(false) == DwgDbStatus::Success)
                m_modifiers |= static_cast<uint32_t>(Modifiers::TurnedLayerOn);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsEntityUpdated () const
    {
    return  m_modifiers & static_cast<uint32_t>(Modifiers::RemovedGradient);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool    HasAnyChange () const
    {
    return  m_modifiers != static_cast<uint32_t>(Modifiers::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool    HasChange (Modifiers mod) const
    {
    return  m_modifiers & static_cast<uint32_t>(mod);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/19
+---------------+---------------+---------------+---------------+---------------+------*/
void Restore ()
    {
    if (!HasAnyChange() || m_entity == nullptr)
        return;
    
    if (HasChange(Modifiers::ThawedLayer) || HasChange(Modifiers::TurnedLayerOn))
        {
        DwgDbLayerTableRecordPtr layer(m_entity->GetLayerId(), DwgDbOpenMode::ForWrite);
        if (layer.OpenStatus() == DwgDbStatus::Success)
            {
            if (HasChange(Modifiers::ThawedLayer))
                layer->SetIsFrozen (true);
            if (HasChange(Modifiers::TurnedLayerOn))
                layer->SetIsOff (true);
            }
        }
    }
};  // EntityImportPreparer



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::_SkipEmptyElement (DwgDbEntityCP entity)
    {
    // create an empty host element for block attributes
    DwgDbBlockReferenceCP   blockRef = DwgDbBlockReference::Cast(entity);
    if (nullptr != blockRef)
        {
        DwgDbObjectIteratorPtr  attrIter = blockRef->GetAttributeIterator ();
        if (attrIter.IsValid() && attrIter->IsValid())
            {
            attrIter->Start ();
            if (!attrIter->Done())
                return  false;

            // no variable attributes - does the block have constant attrdef's?
            DwgDbObjectIdArray  ids;
            if (this->GetConstantAttrdefIdsFor(ids, blockRef->GetBlockTableRecordId()) && !ids.empty())
                return  false;
            }
        }

    // by default, no empty elements should be created!
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    DwgDbEntityP    entity = inputs.GetEntityP ();
    if (nullptr == entity)
        return  BSIERROR;

    uint64_t    entityId = entity->GetObjectId().ToUInt64 ();

    // set DgnDbElement creation options
    ElementCreateParams  createParams(inputs.GetTargetModelR());

    BentleyStatus   status = this->_GetElementCreateParams (createParams, inputs.GetTransform(), *entity);
    if (BSISUCCESS != status)
        return  status;

    // if the entity has a parent, use it for ByBlock symbology:
    DwgDbEntityCP   parent = inputs.GetParentEntity ();

    // set DWG entity draw options:
    GeometryOptions     geomOptions = this->_GetCurrentGeometryOptions ();
    DrawParameters      drawParams (*entity, *this, parent);
    GeometryFactory     geomFactory(createParams, drawParams, geomOptions, entity);

    // prepare for import
    EntityImportPreparer entityPreparer(drawParams, geomFactory, entity);

    // draw entity and create geometry from it in our factory:
    try
        {
        entity->Draw (geomFactory, geomOptions, drawParams);
        }
    catch (...)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::Exception(), IssueReporter::FmtEntity(*entity).c_str());
        }

    // invert entity changes as necessary
    entityPreparer.Restore ();

    auto geometryMap = geomFactory.GetOutputGeometryMap ();
    if (geometryMap.empty() && this->_SkipEmptyElement(entity))
        {
        this->ReportIssue (IssueSeverity::Info, IssueCategory::MissingData(), Issue::EmptyGeometry(), Utf8PrintfString("%ls, ID=%llx", entity->GetDwgClassName().c_str(), entityId).c_str());
        return  BSIERROR;
        }

    // create elements from collected geometries
    ElementFactory  elemFactory(results, inputs, createParams, *this);
    status = elemFactory.CreateElements (&geometryMap);

    if (BSISUCCESS == status)
        m_entitiesImported++;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::ImportNewEntity (ElementImportResults& results, ElementImportInputs& inputs, DwgDbObjectIdCR desiredOwnerId)
    {
    DwgDbEntityP    entity = inputs.GetEntityP ();
    if (nullptr == entity)
        return  BSIERROR;

    // the parent entity for ByBlock symbology, and template entity for ID dependent symbology (layer, linetype, etc):
    DwgDbEntityCP   parentEntity = inputs.GetParentEntity ();
    DwgDbEntityCP   templateEntity = (entity->GetObjectId().IsValid() || inputs.GetTemplateEntity() == nullptr) ? entity : inputs.GetTemplateEntity();

    DwgGiDrawablePtr    drawable = entity->GetDrawable ();
    if (!drawable.IsValid())
        return  BSIERROR;

    // set DgnDbElement creation options
    ElementCreateParams createParams(inputs.GetTargetModelR());

    BentleyStatus   status = this->_GetElementCreateParams (createParams, inputs.GetTransform(), *templateEntity);
    if (BSISUCCESS != status)
        return  status;

    // set DWG entity draw options:
    GeometryOptions     geomOptions = this->_GetCurrentGeometryOptions ();
    DrawParameters      drawParams (*entity, *this, parentEntity, templateEntity);
    GeometryFactory     geomFactory(createParams, drawParams, geomOptions, entity);

    // prepare for import
    EntityImportPreparer    entityPreparer(drawParams, geomFactory, entity);
    if (entityPreparer.IsEntityUpdated())
        {
        // entity is changed, make sure drawble still valid
        drawable = entity->GetDrawable ();
        if (!drawable.IsValid())
            {
            entityPreparer.Restore ();
            return  BSIERROR;
            }
        }

    // draw drawble and create geometry in our factory:
    try
        {
        drawable->Draw (geomFactory, geomOptions, drawParams);
        }
    catch (...)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::Exception(), IssueReporter::FmtEntity(*entity).c_str());
        }

    // invert entity changes as necessary
    entityPreparer.Restore ();

    auto geometryMap = geomFactory.GetOutputGeometryMap ();
    if (geometryMap.empty())
        {
        this->ReportIssue (IssueSeverity::Info, IssueCategory::MissingData(), Issue::EmptyGeometry(), Utf8PrintfString("%ls", entity->GetDwgClassName().c_str()).c_str());
        return  BSIERROR;
        }

    ElementFactory  elemFactory(results, inputs, createParams, *this);
    status = elemFactory.CreateElements (&geometryMap);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::_GetElementLabel (DwgDbEntityCR entity)
    {
    // default element label to entity name:
    Utf8String  label(entity.GetDxfName().c_str());
    if (label.empty())
        label.Assign (entity.GetDwgClassName().c_str());
    
    // for a named block reference, use block name:
    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(&entity);
    if (nullptr != insert)
        {
        DwgDbBlockTableRecordPtr block (insert->GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
        if (!block.IsNull() && !block->IsAnonymous())
            {
            // display block name as element label:
            DwgString   blockName = block->GetName ();
            if (!blockName.IsEmpty())
                {
                Utf8String  insertName(blockName.c_str());
                label.assign (insertName.c_str());
                }
            }
        }
    return  label;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_GetElementCreateParams (DwgImporter::ElementCreateParams& params, TransformCR toDgn, DwgDbEntityCR ent, Utf8CP desiredCode)
    {
    DgnModelCR      model = params.GetModel ();
    DwgDbObjectId   layerId = ent.GetLayerId ();
    DwgDbDatabaseP  xrefDwg = ent.GetDatabase().get ();

    // use entity's database if it's in an xref file; otherwise use current database:
    if (nullptr != xrefDwg && xrefDwg == m_dwgdb.get())
        xrefDwg = nullptr;

    // GeometryParts 
    params.m_geometryPartsModel = this->GetOrCreateJobDefinitionModel ();

    if (model.Is3d())
        {
        // get spatial category & subcategory from the syncInfo:
        params.m_categoryId = this->GetSpatialCategory (params.m_subCategoryId, layerId, xrefDwg);
        }
    else
        {
        // get or create a drawing category and/or a subcategory for the layer & the viewport:
        DwgDbObjectId   viewportId = this->GetCurrentViewportId ();
        params.m_categoryId = this->GetOrAddDrawingCategory (params.m_subCategoryId, layerId, viewportId, model, xrefDwg);
        }

    if (!params.m_categoryId.IsValid())
        {
        Utf8PrintfString msg("[%s] - Invalid layer %llx", IssueReporter::FmtEntity(ent).c_str(), ent.GetLayerId().ToUInt64());
        this->ReportError(IssueCategory::CorruptData(), Issue::ImportFailure(), msg.c_str());
        return BSIERROR;
        }

    // WIP - apply entity ECS?
    // Transform   ecs;
    // ent.GetEcs (ecs);
    // params.m_transform.InitProduct (ecs, toDgn);
    params.m_transform = toDgn;

    params.m_placementPoint = DwgHelper::DefaultPlacementPoint (ent);

#ifdef NEED_UNIQUE_CODE_PER_ELEMENT
    Utf8String      codeNamespace = model.GetName ();
    Utf8String      codeValue;
    if (nullptr != desiredCode)
        codeValue.assign (desiredCode);
    else
        codeValue.Sprintf ("%s:%llx", codeNamespace.c_str(), ent.GetObjectId().ToUInt64());
    params.m_elementCode = this->CreateCode (codeValue);
#else
    if (nullptr != desiredCode)
        params.m_elementCode = this->CreateCode (desiredCode);
#endif  // NEED_UNIQUE_CODE_PER_ELEMENT

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::_FilterEntity (ElementImportInputs& inputs) const
    {
    auto entity = inputs.GetEntityP ();
    if (nullptr == entity)
        return  true;

    // don't draw invisible entities:
    if (DwgDbVisibility::Invisible == entity->GetVisibility())
        return  true;

    // trivial reject clipped away entity:
    auto spatialFilter = inputs.GetSpatialFilter ();
    if (nullptr != spatialFilter && spatialFilter->IsEntityFilteredOut(*entity))
        return  true;

    // trivial reject 3D elements in a 2D model
    if (!inputs.GetTargetModel().Is3d() && DwgHelper::IsNonPlanarAsmObject(entity))
        return  true;

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportEntityByProtocolExtension (ElementImportResults& results, ElementImportInputs& inputs, DwgProtocolExtension& objExt)
    {
    ProtocolExtensionContext context(inputs, results);
    return objExt._ConvertToBim (context, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    // if the entity is extended to support ToDgnDb method, let it take over the importing:
    DwgDbEntityPtr&         entity = inputs.m_entity;
    if (entity.IsNull())
        return  BSIERROR;

#ifdef DEBUG
    uint64_t    entityId = entity->GetObjectId().ToUInt64 ();
#endif

    DwgProtocolExtension*   objExt = DwgProtocolExtension::Cast (entity->QueryX(DwgProtocolExtension::Desc()));
    if (nullptr != objExt)
        return  this->_ImportEntityByProtocolExtension (results, inputs, *objExt);

    return  this->_ImportEntity (results, inputs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::ImportOrUpdateEntity (ElementImportInputs& inputs)
    {
    DwgDbObjectP    object = DwgDbObject::Cast (inputs.GetEntityP());
    if (nullptr == object)
        return  BentleyStatus::BSIERROR;

    IDwgChangeDetector::DetectionResults    detectionResults;
    ElementImportResults    elementResults;
    BentleyStatus   status = BentleyStatus::SUCCESS;

    // consult the sync info to see if the entity has been changed, and what action to take:
    if (this->_GetChangeDetector()._IsElementChanged(detectionResults, *this, *object, inputs.GetModelMapping()))
        {
        // set existing element in elementResults
        elementResults.SetExistingElement (detectionResults.GetObjectAspect());
        // create a new non-database resident element
        status = this->ImportEntity (elementResults, inputs);
        }

    if (BentleyStatus::SUCCESS != status)
        return  status;

    // action to take based on detected results
    status = this->_ProcessDetectionResults(detectionResults, elementResults, inputs);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ProcessDetectionResults (IDwgChangeDetector::DetectionResultsR detectionResults, ElementImportResults& elementResults, ElementImportInputs& inputs)
    {
    auto& changeDetector = this->_GetChangeDetector ();
    auto objectHandle = inputs.GetEntityId().GetHandle();

    // act based on change detector results:
    switch (detectionResults.GetChangeType())
        {
        case IDwgChangeDetector::ChangeType::None:
            {
            // no change - just update input entity for output results
            elementResults.SetExistingElement (detectionResults.GetObjectAspect());
            changeDetector._OnElementSeen (*this, detectionResults.GetExistingElementId());
            // add children into the element seen list
            auto allFromSameSource = iModelExternalSourceAspect::GetSelectFromSameSource(*m_dgndb, detectionResults.GetObjectAspect(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
            while(BE_SQLITE_ROW == allFromSameSource->Step())
                changeDetector._OnElementSeen (*this, allFromSameSource->GetValueId<DgnElementId>(0));
            break;
            }

        case IDwgChangeDetector::ChangeType::Insert:
            {
            // new entity - insert results into BIM, with an ExternalSourceAspect from previously calculated prevenance
            DwgSourceAspects::ObjectAspect::SourceData source(objectHandle, inputs.GetTargetModel().GetModelId(), detectionResults.GetCurrentProvenance(), Utf8String());
            this->InsertResults (elementResults, source);
            break;
            }

        case IDwgChangeDetector::ChangeType::Update:
            {
            // existing element along with an ExternalSourceAspect needs update
            DwgSourceAspects::ObjectAspect::SourceData source(objectHandle, inputs.GetTargetModel().GetModelId(), detectionResults.GetCurrentProvenance(), Utf8String());
            auto existingElementId = detectionResults.GetExistingElementId ();
            this->UpdateResults (elementResults, existingElementId, source);
            break;
            }

        default:
            this->ReportError (IssueCategory::Unsupported(), Issue::Message(), "unknown change type ignored by the change detector");
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgImporter::OpenAndImportEntity (ElementImportInputs& inputs)
    {
    inputs.m_entity.OpenObject (inputs.m_entityId, DwgDbOpenMode::ForRead);

    if (inputs.m_entity.OpenStatus() != DwgDbStatus::Success)
        {
        Utf8PrintfString  msg("Entity: %ls, ID= %llx", inputs.m_entityId.GetDwgClassName().c_str(), inputs.m_entityId.ToUInt64());
        this->ReportError (IssueCategory::UnexpectedData(), Issue::CantOpenObject(), msg.c_str());
        return;
        }

    this->Progress ();

    if (!this->_FilterEntity(inputs))
        this->ImportOrUpdateEntity (inputs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportEntitySection ()
    {
    this->SetStepName (ProgressMessage::STEP_IMPORTING_ENTITIES());

    DwgDbBlockTableRecordPtr    modelspace(m_modelspaceId, DwgDbOpenMode::ForRead);
    if (modelspace.IsNull())
        return  BSIERROR;

    DwgDbBlockChildIteratorPtr  entityIter = modelspace->GetBlockChildIterator ();
    if (!entityIter.IsValid() || !entityIter->IsValid())
        return  BSIERROR;

    ResolvedModelMapping    modelMap = this->FindModel (m_modelspaceId, this->GetRootTransform(), DwgSourceAspects::ModelAspect::SourceType::ModelSpace);
    if (!modelMap.IsValid())
        {
        this->ReportError (IssueCategory::UnexpectedData(), Issue::Error(), "cannot find the default target model!");
        return  BSIERROR;
        }

    // first, detect changes in xRef's attached to the modelspace:
    if (this->ShouldSkipAllXrefsInModel(m_modelspaceId))
        {
        // no xRef change is detected - now detect modelspace changes, which may put the model in the skip list:
        if (this->_GetChangeDetector()._ShouldSkipModel(*this, modelMap))
            return  BSISUCCESS;
        }

    // set modelspace as current space being processed
    m_currentspaceId = m_modelspaceId;

    // set geometry options to draw all entities in the entity section
    DwgDbObjectId       activeVport = m_dwgdb->GetActiveModelspaceViewportId ();
    GeometryOptions&    currOptions = this->_GetCurrentGeometryOptions ();
    currOptions.SetDatabase (m_dwgdb.get());
    currOptions.SetViewportId (activeVport);

    size_t  numIsolines = m_dwgdb->GetISOLINES ();
    currOptions.SetNumberOfIsolines (numIsolines == 0 ? 1 : numIsolines);

    DwgDbViewportTableRecordPtr vport(activeVport, DwgDbOpenMode::ForRead);

    if (this->GetOptions().IsRenderableGeometryPrefered())
        {
        // user prefers renderable geometry - use Isometric view and regen for rendering:
        currOptions.SetRegenType (DwgGiRegenType::RenderCommand);
        currOptions.SetViewDirection (DVec3d::From(-0.577350269189626, -0.57735026918962573, 0.57735026918962573));
        currOptions.SetCameraUpDirection (DVec3d::UnitY());
        currOptions.SetCameraLocation (DPoint3d::FromZero());
        }
    else
        {
        // create geometry based on the default settings of the active viewport:
        if (DwgDbStatus::Success == vport.OpenStatus())
            {
            DVec3d  normal = vport->GetViewDirection ();
            normal.Normalize ();

            currOptions.SetViewDirection (normal);
            currOptions.SetRegenType (DwgGiRegenType::StandardDisplay);

            // Set cameraUp direction to the y-axis of the view:
            double          twist = vport->GetViewTwist ();
            if (fabs(twist) > 0.01)
                currOptions.SetCameraUpDirection (DVec3d::From(sin(twist), cos(twist), 0.0));
            // Camera is always at 0,0,0 in a viewport?
            currOptions.SetCameraLocation (DPoint3d::FromZero());

            DwgDbVisualStylePtr     vistyle(vport->GetVisualStyle(), DwgDbOpenMode::ForRead);
            if (DwgDbStatus::Success == vistyle.OpenStatus())
                {
                Render::RenderMode  renderMode = DwgHelper::GetRenderModeFromVisualStyle (*vistyle.get());
                if (renderMode != Render::RenderMode::Wireframe)
                    currOptions.SetRegenType (DwgGiRegenType::RenderCommand);
                }
            }
        }

    double  currentPDSIZE = m_dwgdb->GetPDSIZE ();
    bool    resetPDSIZE = false;

    // set currently drawing viewport range:
    if (DwgDbStatus::Success == vport.OpenStatus())
        {
        currOptions.SetViewportRange (DwgHelper::GetRangeFrom(vport->GetCenterPoint(), vport->GetWidth(), vport->GetHeight()));

        // change percentage PDSIZE to an absolute size to get a desired PDMODE geometry:
        if (m_dwgdb->GetPDMODE() != 0 && m_dwgdb->GetPDSIZE() <= 0.0)
            {
            m_dwgdb->SetPDSIZE (DwgHelper::GetAbsolutePDSIZE(m_dwgdb->GetPDSIZE(), vport->GetHeight()));
            resetPDSIZE = true;
            }
        vport.CloseObject ();
        }

    this->SetTaskName (ProgressMessage::TASK_IMPORTING_MODEL(), modelMap.GetModel()->GetName().c_str());
    this->Progress ();

    ElementImportInputs     inputs (*modelMap.GetModel());
    inputs.SetClassId (this->_GetElementType(*modelspace.get()));
    inputs.SetTransform (this->GetRootTransform());
    inputs.SetSpatialFilter (nullptr);
    inputs.SetModelMapping (modelMap);

    // SortEnts table
    DwgDbSortentsTablePtr   sortentsTable;
    if (DwgDbStatus::Success == modelspace->OpenSortentsTable(sortentsTable, DwgDbOpenMode::ForRead))
        {
        // import entities in sorted order:
        DwgDbObjectIdArray  entities;
        if (DwgDbStatus::Success == sortentsTable->GetFullDrawOrder(entities))
            {
            for (DwgDbObjectIdCR id : entities)
                {
                inputs.SetEntityId (id);
                this->OpenAndImportEntity (inputs);
                }
            }
        }
    else
        {
        // import entities in database order
        for (entityIter->Start(); !entityIter->Done(); entityIter->Step())
            {
            inputs.SetEntityId (entityIter->GetEntityId());
            this->OpenAndImportEntity (inputs);
            }
        }

    // restore original PDSIZE
    if (resetPDSIZE)
        m_dwgdb->SetPDSIZE (currentPDSIZE);
        
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::_ArePointsValid (DPoint3dCP checkPoints, size_t numPoints, DwgDbEntityCP entity)
    {
    static double   s_tooBigCoordinate = 1.0e10;
    for (DPoint3dCP lastPoint = checkPoints + numPoints; checkPoints < lastPoint; checkPoints++)
        {
        if (checkPoints->x < -s_tooBigCoordinate || checkPoints->x > s_tooBigCoordinate ||
            checkPoints->y < -s_tooBigCoordinate || checkPoints->y > s_tooBigCoordinate ||
            checkPoints->z < -s_tooBigCoordinate || checkPoints->z > s_tooBigCoordinate)
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId      DwgImporter::_GetElementType (DwgDbBlockTableRecordCR block)
    {
    BentleyApi::ECN::ECClassCP  elementClass = nullptr;

    // a physical class for a modelspace or a drawing graphic class for a paperspace entity
    if (block.IsModelspace())
        elementClass = this->GetDgnDb().Schemas().GetClass (GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject);
    else if (block.IsLayout())
        elementClass = this->GetDgnDb().Schemas().GetClass (BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic);
    else if (block.IsExternalReference())
        elementClass = this->GetDgnDb().Schemas().GetClass (GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject);
    else
        BeAssert (false && L"Unexpected element mapping!");

    if (nullptr == elementClass)
        {
        this->ReportError (IssueCategory::MissingData(), Issue::ImportFailure(), "Missing ECSchemas or element class not mapped!");
        return DgnClassId ();
        }

    return DgnClassId (elementClass->GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus     DwgImporter::InsertResults (ElementImportResults& results, DwgSourceAspects::ObjectAspect::SourceDataCR source)
    {
    if (!results.m_importedElement.IsValid())
        return DgnDbStatus::Success;

    // create & add an ObjectAspect for the new element
    results.m_existingElementMapping = m_sourceAspects.AddOrUpdateObjectAspect (*results.m_importedElement, source);
    BeAssert (results.m_existingElementMapping.IsValid() && "Failed adding a ExternalSourceAspect for a DWG object");

    // insert the primary element
    DgnDbStatus status = DgnDbStatus::Success;
    DgnCode     code = results.m_importedElement->GetCode ();
    auto        ret = GetDgnDb().Elements().Insert (*results.m_importedElement, &status);

    if (DgnDbStatus::DuplicateCode == status)
        {
        Utf8String  duplicateMessage;
        duplicateMessage.Sprintf ("Duplicate element code '%s' ignored", code.GetValueUtf8().c_str());
        ReportIssue (IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::Message(), duplicateMessage.c_str());

        DgnDbStatus setStatus = results.m_importedElement->SetCode (DgnCode::CreateEmpty());
        BeAssert(DgnDbStatus::Success == setStatus);

        ret = GetDgnDb().Elements().Insert (*results.m_importedElement, &status);
        }

    if (DgnDbStatus::Success != status)
        {
        ReportError (IssueCategory::VisualFidelity(), Issue::Message(), Utf8PrintfString("Unable to insert element [status=%d]", status).c_str());
        return status;
        }

    if (ret.IsValid())  // an Invalid element is acceptable and means it was purposefully discarded during import
        LOG_ENTITY.tracev ("Inserted %s, %s", IssueReporter::FmtElement(*ret).c_str(), ret->GetDisplayLabel().c_str());

    DgnElementId    parentId = results.m_importedElement->GetElementId ();

    // tell the change detector that we have seen this element
    this->_GetChangeDetector()._OnElementSeen (*this, parentId);

    // insert the children of the primary elements, if any
    for (ElementImportResults& child : results.m_childElements)
        {
        DgnElementP childElement = child.GetImportedElement ();
        if (childElement == nullptr)
            continue;

        childElement->SetParentId (parentId, m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));

        status = this->InsertResults (child, source);
        if (DgnDbStatus::Success != status)
            return status;
        }
        
    return status;
    }
