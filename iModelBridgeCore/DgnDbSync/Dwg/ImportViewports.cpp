/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportViewports.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DGNDBSYNC_DWG

DWG_PROTOCALEXT_DEFINE_MEMBERS(DwgViewportExt)


/*=================================================================================**//**
* ViewFactory converts DWG viewports to Bim views
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportFactory::ViewportFactory (DwgImporter& importer, DwgDbViewportTableRecordCR viewportRecord) : m_importer(importer)
    {
    // construct a ViewportFactory from a modelspace viewport table record:
    m_center = DPoint3d::From (viewportRecord.GetCenterPoint());
    m_target = viewportRecord.GetTargetPoint ();
    m_viewDirection = viewportRecord.GetViewDirection ();
    m_hasCamera = viewportRecord.IsPerspectiveEnabled ();
    m_isFrontClipped = viewportRecord.IsFrontClipEnabled ();
    m_isBackClipped = viewportRecord.IsBackClipEnabled ();
    m_frontClipDistance = viewportRecord.GetFrontClipDistance ();
    m_backClipDistance = viewportRecord.GetBackClipDistance ();
    m_height = viewportRecord.GetHeight ();
    m_width = viewportRecord.GetWidth ();
    m_lensLength = viewportRecord.GetLensLength ();
    m_viewTwistAngle = viewportRecord.GetViewTwist ();
    m_isGridOn = viewportRecord.IsGridEnabled ();
    m_isUcsIconOn = viewportRecord.IsUcsIconEnabled ();
    m_backgroundId = viewportRecord.GetBackground ();
    m_visualStyleId = viewportRecord.GetVisualStyle ();
    m_customScale = 1.0;
    m_backgroundColor = ColorDef::Black ();
    m_isPrivate = false;
    m_transform = importer.GetRootTransform ();
    m_inputViewport = DwgDbObject::Cast (&viewportRecord);
    // modelspace viewports do not have clipping entity
    m_clipEntityId.SetNull ();

    this->TransformDataToBim ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportFactory::ViewportFactory (DwgImporter& importer, DwgDbViewportCR viewportEntity, DwgDbLayoutCP layout) : m_importer(importer)
    {
    // construct a ViewportFactory from a paperspace viewport entity
    m_center = viewportEntity.GetCenterPoint ();
    m_target = viewportEntity.GetViewTarget ();
    m_viewDirection = viewportEntity.GetViewDirection ();
    m_hasCamera = viewportEntity.IsPerspectiveEnabled ();
    m_isFrontClipped = viewportEntity.IsFrontClipEnabled ();
    m_isBackClipped = viewportEntity.IsBackClipEnabled ();
    m_frontClipDistance = viewportEntity.GetFrontClipDistance ();
    m_backClipDistance = viewportEntity.GetBackClipDistance ();
    m_height = viewportEntity.GetHeight ();
    m_width = viewportEntity.GetWidth ();
    m_lensLength = viewportEntity.GetLensLength ();
    m_viewTwistAngle = viewportEntity.GetViewTwist ();
    m_isGridOn = viewportEntity.IsGridEnabled ();
    m_isUcsIconOn = viewportEntity.IsUcsIconEnabled ();
    m_backgroundId = viewportEntity.GetBackground ();
    m_visualStyleId = viewportEntity.GetVisualStyle ();
    m_customScale = viewportEntity.GetCustomScale ();
    m_backgroundColor = ColorDef::White ();
    m_isPrivate = false;
    m_transform = importer.GetRootTransform ();
    m_inputViewport = DwgDbObject::Cast (&viewportEntity);
    // paperspaview viewports may have clipping entities
    m_clipEntityId = viewportEntity.GetClipEntity ();

    if (nullptr != layout)
        {
        // we are creating a sheet - transform view data to sheet coordinates:
        m_transform.InitIdentity ();
        this->ComposeLayoutTransform (m_transform, layout->GetBlockTableRecordId());
        }
    else
        {
        // we are creating a spatial view for a view attachment - use view, instead of sheet, coordinates:
        m_center = DPoint3d::From (viewportEntity.GetViewCenter());

        double  aspectRatio = m_height > 1.0e-5 ? m_width / m_height : 1.0;

        m_height = viewportEntity.GetViewHeight ();
        m_width = m_height * aspectRatio;
       }

    this->TransformDataToBim ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ViewportFactory::ComposeLayoutTransform (TransformR trans, DwgDbObjectIdCR blockId)
    {
    DwgDbBlockTableRecordPtr    block (blockId, DwgDbOpenMode::ForRead);
    if (block.OpenStatus() == DwgDbStatus::Success && block->IsLayout() && !block->IsModelspace())
        {
        m_importer.ScaleModelTransformBy (trans, *block.get());
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::TransformDataToBim ()
    {
    m_transform.Multiply (m_center);
    m_center.z = 0.0;

    m_transform.Multiply (m_target);
    m_transform.Multiply (m_viewDirection);

    double      toMeters = 1.0;
    m_transform.IsRigidScale (toMeters);
    
    m_frontClipDistance *= toMeters;
    m_backClipDistance *= toMeters;
    m_height *= toMeters;
    m_width *= toMeters;
    m_lensLength *= toMeters;

    if (0.0 == m_lensLength)
        m_lensLength = toMeters;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ComputeSpatialView (SpatialViewDefinitionR dgnView)
    {
    DVec3d      zAxis = DVec3d::From (m_viewDirection);
    double      focalLength = zAxis.Normalize ();
    double      frontClip = 0.0;
    double      backClip = 0.0;
    DVec3d      delta;
    double      aspectRatio = m_height > 0.0 ? (m_width / m_height) : 1.0;

    if (m_hasCamera)
        {
        double  maxFrontClip = focalLength * 0.9999;

        if (m_isFrontClipped)
            frontClip = m_frontClipDistance > maxFrontClip ? maxFrontClip : m_frontClipDistance;
        else
            frontClip = maxFrontClip;

        // do not allow too small focal length, use a 1/4 meter as minimum:
        if (focalLength < 0.25)
            focalLength = 0.25;

        if (aspectRatio > 1.0)
            {
            delta.x = 35.0 * focalLength / m_lensLength;
            delta.y = delta.x / aspectRatio;
            }
        else
            {
            delta.y = 35.0 * focalLength / m_lensLength;
            delta.x = delta.y * aspectRatio;
            }

        ViewDefinition3d::Camera&   camera = dgnView.GetCameraR ();

        camera.SetLensAngle (Angle::FromRadians(m_viewTwistAngle));
        camera.ValidateLens ();
        camera.SetFocusDistance (focalLength);
        }
    else
        {
        frontClip = m_isFrontClipped ? m_frontClipDistance : m_height * 10.0;
        delta.x = m_height * aspectRatio;
        delta.y = m_height;
        }

    // set back clip
    backClip = m_isBackClipped ? m_backClipDistance : -frontClip;

    delta.z = frontClip - backClip;

    dgnView.SetExtents (delta);

    // compose view rotation matrix
    RotMatrix   rotation;
    DwgHelper::ComputeMatrixFromArbitraryAxis (rotation, zAxis);

    RotMatrix   viewTwist;
    viewTwist.InitFromAxisAndRotationAngle (2, -m_viewTwistAngle);

    rotation.InitProduct (rotation, viewTwist);
    rotation.Transpose ();

    dgnView.SetRotation (rotation);
    
    // compute camera position
    if (m_hasCamera)
        {
        DPoint3d    cameraPosition = DPoint3d::FromSumOf (m_target, zAxis, focalLength);
        dgnView.SetEyePoint (cameraPosition);
        }
    
    // rotate target to the view plane
    rotation.Multiply (m_target);
    
    // compute origin in the view plane
    DPoint3d    origin;
    origin.x = m_target.x + m_center.x - delta.x / 2.0;
    origin.y = m_target.y + m_center.y - delta.y / 2.0;
    origin.z = m_target.z + backClip;

    // rotate origin to the world
    rotation.MultiplyTranspose (origin);
    dgnView.SetOrigin (origin);

    // if clipped by an entity, calculate & apply a clipping vector:
    this->ApplyViewportClipping (dgnView, frontClip, backClip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ApplyViewportClipping (SpatialViewDefinitionR dgnView, double frontClip, double backClip)
    {
    if (m_clipEntityId.IsNull())
        return;

    RotMatrix   viewRotation = dgnView.GetRotation ();
    Transform   entityToClipper;
    if (!this->ComputeClipperTransformation(entityToClipper, viewRotation))
        return;

    // the clipper plane as the same as the view plane, but the clipper constructor expects transformation matrix from clipper to model:
    viewRotation.InverseOf (viewRotation);
    Transform   clipperToModel = Transform::From (viewRotation, DVec3d::FromZero());

    // optional front/back clippings:
    double*     zLow = m_isFrontClipped ? &frontClip : nullptr;
    double*     zHigh = m_isBackClipped ? &backClip : nullptr;

    ClipVectorPtr   clipper = DwgHelper::CreateClipperFromEntity (m_clipEntityId, zLow, zHigh, &entityToClipper, &clipperToModel);

    if (clipper.IsValid())
        {
        // now apply units tranformation to the clipper:
        clipper->TransformInPlace (m_transform);
        dgnView.SetViewClip (clipper);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ViewportFactory::ComputeClipperTransformation (TransformR toClipper, RotMatrixCR viewRotation)
    {
    /*-----------------------------------------------------------------------------------
    Clipping entities are in layout.  We will have to transform them to the clipper plane
    which is in model/world coordinate system.  Do not do units transform - use native DWG
    coordinate readout.
    -----------------------------------------------------------------------------------*/
    DwgDbViewportCP viewport = DwgDbViewport::Cast (m_inputViewport);
    if (nullptr == viewport)
        return  false;
    
    RotMatrix   scaleMatrix = RotMatrix::FromIdentity ();
    double      viewScale = viewport->GetCustomScale ();
    if (viewScale > 0.001 && fabs(viewScale - 1.0) > 0.001)
        {
        viewScale = 1.0 / viewScale;
        scaleMatrix.ScaleColumns (viewScale, viewScale, viewScale);
        }

    // get viewport center point:
    DPoint3d    viewportCenter = viewport->GetCenterPoint ();
    viewportCenter.Negate ();

    // get the view target on view/clipper plane:
    DPoint3d    target = viewport->GetViewTarget ();
    viewRotation.Multiply (target);

    // calculate the targeted point from the viewport center:
    DPoint3d    viewCenter = DPoint3d::From (viewport->GetViewCenter());
    viewCenter.Add (target);
    viewCenter.z = 0.0;

    // compose a transformation that translates viewport center to origin, scales & rotates about the origin, then translate it to the view center in the model/world:
    toClipper = Transform::FromProduct (Transform::From(viewCenter), Transform::From(scaleMatrix), Transform::From(viewportCenter));

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ComputeSheetView (SheetViewDefinitionR dgnView)
    {
    // compute view delta:
    DVec3d      delta;
    delta.x = m_width;
    delta.y = m_height;
    delta.z = 0.0;

    if (delta.Magnitude() < mgds_fc_epsilon)
        delta.Init (DgnUnits::OneMeter(), DgnUnits::OneMeter());

    dgnView.SetExtents (delta);

    // get the view rotation matrix
    RotMatrix   rotation;
    rotation.InitFromAxisAndRotationAngle (2, m_viewTwistAngle);

    dgnView.SetRotation (rotation);
    
    // compute view origin
    DPoint3d    origin;
    origin.x = m_center.x - delta.x / 2.0;
    origin.y = m_center.y - delta.y / 2.0;
    origin.z = 0.0;

    // rotate origin
    rotation.MultiplyTranspose (origin);
    dgnView.SetOrigin (origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ComputeSpatialDisplayStyle (DisplayStyle3dR displayStyle)
    {
    ViewFlags   viewFlags;
    DwgHelper::SetViewFlags (viewFlags, m_isGridOn, m_isUcsIconOn, m_backgroundId.IsValid(), true, m_isFrontClipped, m_isBackClipped, m_importer.GetDwgDb());

    DwgHelper::UpdateViewFlagsFromVisualStyle (viewFlags, m_visualStyleId);

    displayStyle.SetViewFlags (viewFlags);

    // WIP - import DwgDbBackground to environment display
    auto&   enviromentDisplay = displayStyle.GetEnvironmentDisplayR ();
    enviromentDisplay.m_groundPlane.m_enabled = false;
    enviromentDisplay.m_skybox.m_enabled = false;

    displayStyle.SetBackgroundColor (m_backgroundColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ComputeSheetDisplayStyle (DisplayStyleR displayStyle)
    {
    ViewFlags   viewFlags;
    DwgHelper::SetViewFlags (viewFlags, m_isGridOn, m_isUcsIconOn, false, false, m_isFrontClipped, m_isBackClipped, m_importer.GetDwgDb());

    displayStyle.SetViewFlags (viewFlags);
    displayStyle.SetBackgroundColor (m_backgroundColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::AddSpatialCategories (DgnDbR dgndb, Utf8StringCR viewName)
    {
    // add all spatial categories to the modelspace view, but only add displayed ones to a viewport view:
    DwgDbObjectIdArray  frozenLayers;
    DwgDbViewportCP     viewportEntity = DwgDbViewport::Cast (m_inputViewport);

    // if this is for a viewport entity, get viewport frozen layer ID's:
    size_t  numFrozenLayers = 0;
    if (nullptr != viewportEntity && DwgDbStatus::Success == viewportEntity->GetFrozenLayers(frozenLayers))
        numFrozenLayers = frozenLayers.size ();

    // lookup current DWG file in syncInfo
    DwgDbDatabaseR          dwg = m_importer.GetDwgDb ();
    DwgSyncInfo::DwgFileId  fileId = DwgSyncInfo::DwgFileId::GetFrom (dwg);
    DwgSyncInfo&            syncInfo = m_importer.GetSyncInfo ();

    m_categories = new CategorySelector (dgndb.GetDictionaryModel(), viewName.c_str());

    DgnCategoryIdSet&   categoryIdSet = m_categories->GetCategoriesR ();

    // add all spatail categories to the view:
    for (ElementIteratorEntry entry : SpatialCategory::MakeIterator(dgndb))
        {
        DgnCategoryId   categoryId = entry.GetId <DgnCategoryId> ();

        if (nullptr != viewportEntity && numFrozenLayers > 0)
            {
            // this is a viewport entity - check viewport layer freeze:
            DwgDbObjectId   layerId;
            DwgDbHandle     objHandle = syncInfo.FindLayerHandle (categoryId, fileId);
            if (!objHandle.IsNull() && (layerId = dwg.GetObjectId(objHandle)).IsValid())
                {
                // skip viewport frozen layer for this view:
                auto found = std::find_if (frozenLayers.begin(), frozenLayers.end(), [&](DwgDbObjectId id){ return id == layerId; });
                if (found != frozenLayers.end())
                    continue;
                }
            }

        categoryIdSet.insert (categoryId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ViewportFactory::ValidateViewName (Utf8StringR viewNameInOut, DgnViewId& viewIdOut)
    {
    DgnDbR  dgndb = m_importer.GetDgnDb ();

    viewIdOut = ViewDefinition::QueryViewId (dgndb.GetDictionaryModel(), viewNameInOut);

    if (viewIdOut.IsValid())
        {
        // if we are updating Bim, return true to use existing view.
        if (m_importer.IsUpdating())
            return true;

        viewIdOut.Invalidate ();

        // deduplicate view name
        Utf8String  suffix;
        uint32_t    count = 1;
        Utf8String  fileName = Utf8String (m_importer.GetDwgDb().GetFileName().c_str());
        Utf8String  proposedName = viewNameInOut;

        do
            {
            suffix.Sprintf ("-%d", count++);
            viewNameInOut = m_importer.RemapNameString (fileName, proposedName, suffix);
            } while (ViewDefinition::QueryViewId(dgndb.GetDictionaryModel(), viewNameInOut).IsValid());
        }

    // return false to create a new view with the validated name.
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ViewportFactory::ComputeViewAttachment (Placement2dR placement)
    {
    // only a viewport entity is applicable for a view attachment
    DwgDbViewportCP viewport = DwgDbViewport::Cast (m_inputViewport);
    if (nullptr == viewport)
        return  false;

    // create model transform from layout units:
    m_transform.InitIdentity ();
    if (!this->ComposeLayoutTransform(m_transform, viewport->GetOwnerId()))
        return  false;
    
    // get the viewport size in the sheet:
    double      toMeters = m_transform.MatrixColumnMagnitude (0);
    double      viewportHeight = viewport->GetHeight() * toMeters;
    double      viewportWidth = viewport->GetWidth() * toMeters;

    // align the model view center to the viewport center, in sheet coordinates:
    DPoint3d    viewportCenter = viewport->GetCenterPoint ();
    m_transform.Multiply (viewportCenter);

    // set the origin & the bounding box of the view attachment to return:
    auto&   origin = placement.GetOriginR ();
    auto&   boundingBox = placement.GetElementBoxR ();

    origin.Init (viewportCenter.x - viewportWidth / 2.0, viewportCenter.y - viewportHeight / 2.0);

    boundingBox.low.Zero ();
    boundingBox.high.Init (viewportWidth, viewportHeight);

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId       ViewportFactory::CreateSpatialView (DgnModelId modelId, Utf8StringCR proposedName)
    {
    DgnDbR      dgndb = m_importer.GetDgnDb ();
    Utf8String  viewName (proposedName);
    DgnViewId   viewId;

    if (this->ValidateViewName(viewName, viewId) && viewId.IsValid())
        return  viewId;

    viewId.Invalidate ();

    // only add the master file's modelspace at this time - other models may be added in post process:
    ModelSelectorPtr    models = new ModelSelector (dgndb.GetDictionaryModel(), viewName.c_str());
    models->AddModel (modelId);

    // add all spatial categories to the view, with viewport frozen layers applied:
    this->AddSpatialCategories (dgndb, viewName);

    // create a display style:
    DisplayStyle3dPtr   displayStyle = new DisplayStyle3d (dgndb.GetDictionaryModel(), viewName.c_str());
    this->ComputeSpatialDisplayStyle (*displayStyle);

    // add a default camera for now, if there is one, and will set it later in ComputerSpatialView:
    ViewDefinition3d::Camera    camera;
    ViewDefinition3d::Camera*   optionalCamera = m_hasCamera ? &camera : nullptr;

    // create a CameraView for a perspective viewport or an OrthographicView otherwise:
    SpatialViewDefinitionPtr view = new SpatialViewDefinition (dgndb.GetDictionaryModel(), viewName, *m_categories, *displayStyle, *models, optionalCamera);

    // convert DWG viewport data to DgnView
    this->ComputeSpatialView (*view);

    view->SetIsPrivate (m_isPrivate);

    // insert the view to BIM
    if (!view.IsValid() || view->Insert().IsNull())
        {
        m_importer.ReportError(DwgImporter::IssueCategory::CorruptData(), DwgImporter::Issue::Error(), viewName.c_str());
        return viewId;
        }

    viewId = view->GetViewId ();

    return  viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ViewportFactory::UpdateSpatialView (DgnViewId viewId)
    {
    auto spatialView = m_importer.GetDgnDb().Elements().GetForEdit<SpatialViewDefinition> (viewId);
    if (!spatialView.IsValid())
        return  static_cast<BentleyStatus>(DgnDbStatus::ViewNotFound);

    // check view type change
    auto cameraView = dynamic_cast<SpatialViewDefinitionP> (spatialView.get());
    if (nullptr == cameraView)
        return  BSIERROR;
  
    // update display style:
    auto& displayStyle = spatialView->GetDisplayStyle3d ();
    this->ComputeSpatialDisplayStyle (displayStyle);
    displayStyle.Update ();

    // re-compute the view
    this->ComputeSpatialView (*spatialView);

    spatialView->Update ();

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId       ViewportFactory::CreateSheetView (DgnModelId modelId, Utf8StringCR proposedName)
    {
    DgnDbR      dgndb = m_importer.GetDgnDb ();
    Utf8String  viewName (proposedName);
    DgnViewId   viewId;

    if (this->ValidateViewName(viewName, viewId) && viewId.IsValid())
        return  viewId;

    viewId.Invalidate ();

    // add an empty drawing category to the view - will update in the post process:
    CategorySelectorPtr categories = new CategorySelector (dgndb.GetDictionaryModel(), viewName.c_str());

    // create a display style for the sheet
    DisplayStyle2dPtr   displayStyle = new DisplayStyle2d (dgndb.GetDictionaryModel(), viewName.c_str());
    this->ComputeSheetDisplayStyle (*displayStyle);

    // create a new sheet view
    SheetViewDefinitionPtr  view = new SheetViewDefinition (dgndb.GetDictionaryModel(), viewName, modelId, *categories, *displayStyle);
    if (!view.IsValid())
        {
        BeAssert (false && "failed creating SheetViewDefinition!");
        return viewId;
        }

    // convert viewport data to the sheet view:
    this->ComputeSheetView (*view.get());

    view->SetIsPrivate (m_isPrivate);

    // now insert the new view into DB:
    if (view->Insert().IsNull())
        {
        m_importer.ReportError(DwgImporter::IssueCategory::UnexpectedData(), DwgImporter::Issue::Error(), viewName.c_str());
        return viewId;
        }

    viewId = view->GetViewId ();

    return  viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   ViewportFactory::CreateViewAttachment (DgnModelCR sheetModel, DgnViewId viewId)
    {
    // get or create a drawing category for a view attachment:
    DwgDbEntityCP       entity = DwgDbEntity::Cast (m_inputViewport);
    if (nullptr == entity)
        return  nullptr;

    DwgDbObjectId       layerId = entity->GetLayerId ();
    // get the overall viewport ID (i.e. the layout viewport):
    DwgDbObjectId       layoutViewportId = m_importer._GetCurrentGeometryOptions().GetViewportId ();
    // get or create a drawing category from the layer of this viewport entity:
    DgnSubCategoryId    subCategoryId;
    DgnCategoryId       categoryId = m_importer.GetOrAddDrawingCategory (subCategoryId, layerId, layoutViewportId, sheetModel);

    // calculate the placement point in the sheet view for the view attachment
    Placement2d     placement;
    this->ComputeViewAttachment (placement);

    // create a view attachment in the sheet:
    auto viewAttachment = new Sheet::ViewAttachment (m_importer.GetDgnDb(), sheetModel.GetModelId(), viewId, categoryId, placement);
    if (nullptr == viewAttachment)
        {
        BeAssert (false && "failed constructing a sheet view attachment!");
        return  nullptr;
        }

    // set view attachment scale
    if (m_customScale > 1.e-5)
        viewAttachment->SetScale (1.0 / m_customScale);

    return  viewAttachment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   ViewportFactory::UpdateViewAttachment (DgnElementId attachId, DgnViewId viewId)
    {
    // get or create a view attachment:
    auto viewAttachment = m_importer.GetDgnDb().Elements().GetForEdit<Sheet::ViewAttachment>(attachId);
    if (!viewAttachment.IsValid())
        return  nullptr;

    // calculate the placement point in the sheet view for the view attachment
    Placement2d     placement;
    this->ComputeViewAttachment (placement);
    viewAttachment->SetPlacement (placement);

    // set view attachment scale
    if (m_customScale > 1.e-5)
        viewAttachment->SetScale (1.0 / m_customScale);

    return  viewAttachment;
    }





/*=================================================================================**//**
* DwgViewportExt extension
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgViewportExt::_ConvertToBim (ProtocalExtensionContext& context, DwgImporter& importer)
    {
    // this is a viewport entity that displays the modelspace, not an overall viewport for a paperspace!
    DwgDbEntityPtr&     entity = context.GetEntityPtrR ();
    DwgDbViewportCP     viewport = DwgDbViewport::Cast (entity.get());
    if (nullptr == viewport)
        return  BSIERROR;

    // get the modelspace model:
    DgnModelP   rootModel = nullptr;
    ResolvedModelMapping    modelMap = importer.FindModel (importer.GetModelSpaceId(), importer.GetRootTransform(), DwgSyncInfo::ModelSourceType::ModelOrPaperSpace);
    if (!modelMap.IsValid() || (rootModel = modelMap.GetModel()) == nullptr)
        {
        BeAssert(false && L"failed retrieving modelspace model!");
        return BSIERROR;
        }

    // set the view name as "LayoutName Viewport-ID":
    DgnModelR           sheetModel = context.GetModel ();
    Utf8PrintfString    viewName ("%s Viewport-%llx", sheetModel.GetName().c_str(), viewport->GetObjectId().ToUInt64());

    // this method gets called for either creating a new or updating existing element.
    if (importer.IsUpdating() && context.GetElementResults().GetExistingElement().IsValid())
        return  this->UpdateBim(context, importer, *rootModel, sheetModel);

    // create a spatial view from the modelspace/root model:
    ViewportFactory factory (importer, *viewport);
    // use white background color for the model view:
    factory.SetBackgroundColor (ColorDef::White());
    // the spatial view is generated for a view attachment in next step, hide it from the user:
#if defined(NDEBUG) || !defined(DEBUG)
    factory.SetViewSourcePrivate (true);
#endif

    // create a spatial view of the modelspace model:
    DgnViewId   modelViewId = factory.CreateSpatialView (rootModel->GetModelId(), viewName);
    if (!modelViewId.IsValid())
        {
        BeAssert (false && "failed creating a spatial view for a viewport entity");
        return  BSIERROR;
        }

    // create a view attachment from the viewport entity data
    DgnElementPtr   viewAttachment = factory.CreateViewAttachment (sheetModel, modelViewId);
    if (!viewAttachment.IsValid())
        return  BSIERROR;

    // the ViewAttachment is the pivotal element in syncinfo - let the caller insert it in bim as well as in syncinfo
    context.GetElementResultsR().SetImportedElement (viewAttachment.get());
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgViewportExt::UpdateBim (ProtocalExtensionContext& context, DwgImporter& importer, DgnModelCR rootModel, DgnModelCR sheetModel)
    {
    /*-----------------------------------------------------------------------------------
    The default implementation of importing an entity can only update a single output 
    element, but a viewport entity results in a DgnModel, a SpatialView, and a ViewAttachment.
    This method updates all of them. Only ViewAttachment element is saved in the syncInfo 
    for change detection.
    -----------------------------------------------------------------------------------*/
    DwgDbEntityPtr&     entity = context.GetEntityPtrR ();
    DwgDbViewportCP     viewport = DwgDbViewport::Cast (entity.get());
    if (nullptr == viewport)
        return  BSIERROR;

    // get ViewAttachment from syncinfo
    DgnElementId    oldId = context.GetElementResults().GetExistingElement().GetElementId ();
    auto oldViewAttachment = importer.GetDgnDb().Elements().Get<Sheet::ViewAttachment> (oldId);
    if (!oldViewAttachment.IsValid())
        {
        BeAssert (false && "the expected ViewAttachment does not exist for update!");
        return  BSIERROR;
        }

    DgnViewId   modelViewId = oldViewAttachment->GetAttachedViewId ();
    if (!modelViewId.IsValid())
        return  BSIERROR;

    // create a spatial view from the modelspace/root model:
    ViewportFactory factory (importer, *viewport);

    // get or create a spatial view of the modelspace model:
    BentleyStatus   status = factory.UpdateSpatialView (modelViewId);
    if (BSISUCCESS != status)
        {
        // delete the view and re-import it anew:
        importer.GetDgnDb().Elements().Delete (modelViewId);
        importer.GetSyncInfo().DeleteElement (modelViewId);

        Utf8PrintfString    viewName ("%s Viewport-%llx", sheetModel.GetName().c_str(), viewport->GetObjectId().ToUInt64());

        modelViewId = factory.CreateSpatialView (rootModel.GetModelId(), viewName);
        if (modelViewId.IsValid())
            {
            BeAssert (false && "failed creating a spatial view for a viewport entity");
            return  BSIERROR;
            }
        }

    DwgDbObjectCP   obj = DwgDbObject::Cast(entity.get());
    if (nullptr == obj)
        return  BSIERROR;

    DwgSyncInfo::DwgObjectProvenance    newProv (*obj, importer.GetSyncInfo(), importer.GetCurrentIdPolicy(), false);

    // update the view attachment and the syninfo
    DgnElementPtr   newViewAttachment = factory.UpdateViewAttachment (oldId, modelViewId);
    if (!newViewAttachment.IsValid())
        {
        // delete existing ViewAttachment and create a new one
        if (oldId.IsValid())
            importer.GetDgnDb().Elements().Delete (oldId);

        newViewAttachment = factory.CreateViewAttachment (sheetModel, modelViewId);

        if (newViewAttachment.IsValid())
            status = BSIERROR;
        }

     // the ViewAttachment is the pivotal element in syncinfo - let the caller insert it in bim as well as in syncinfo
    if (newViewAttachment.IsValid())
        context.GetElementResultsR().SetImportedElement (newViewAttachment.get());

   return  status;
    }





/*=================================================================================**//**
* common viewport API via DwgImporter
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::SaveViewDefinition (ViewControllerR viewController)
    {
    auto& viewDef = viewController.GetViewDefinition ();

    viewDef.GetCategorySelector().Update ();
    viewDef.GetDisplayStyle().Update ();
    viewDef.Update ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportModelspaceViewport (DwgDbViewportTableRecordCR dwgVport)
    {
    DwgDbDatabaseP  dwg = dwgVport.GetDatabase().get ();
    if (nullptr == dwg)
        {
        BeAssert(false);
        return BSIERROR;
        }
    DgnModelP   rootModel = nullptr;
    ResolvedModelMapping    modelMap = this->FindModel (m_modelspaceId, m_rootTransform, DwgSyncInfo::ModelSourceType::ModelOrPaperSpace);
    if (!modelMap.IsValid() || (rootModel = modelMap.GetModel()) == nullptr)
        {
        BeAssert(false && L"root model has not been imported yet!");
        return BSIERROR;
        }
        
    Utf8String                  layoutName;
    DwgDbBlockTableRecordPtr    modelspace (m_modelspaceId, DwgDbOpenMode::ForRead);
    if (!modelspace.IsNull())
        {
        DwgDbLayoutPtr  layout (modelspace->GetLayoutId(), DwgDbOpenMode::ForRead);
        if (!layout.IsNull())
            layoutName.Assign (layout->GetName().c_str());
        }
    if (layoutName.empty())
        layoutName.assign ("ModelSpace");
   
    Utf8String  vportName = Utf8String (dwgVport.GetName().c_str());
    if (vportName.empty())
        vportName.assign ("View");
    else if (vportName.StartsWith("*"))
        vportName.erase (0, 1);

    Utf8PrintfString    viewName ("%s - %s", layoutName, vportName);
    ViewportFactory     factory(*this, dwgVport);

    m_defaultViewId = factory.CreateSpatialView (rootModel->GetModelId(), viewName);

    return m_defaultViewId.IsValid() ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportModelspaceViewports ()
    {
    DwgDbViewportTablePtr   viewportTable (m_dwgdb->GetViewportTableId(), DwgDbOpenMode::ForRead);
    if (viewportTable.IsNull())
        return  BSIERROR;

    DwgDbSymbolTableIterator    iter = viewportTable->NewIterator ();

    if (!iter.IsValid())
        return  BSIERROR;

    for (iter.Start(); !iter.Done(); iter.Step())
        {
        DwgDbViewportTableRecordPtr     viewport (iter.GetRecordId(), DwgDbOpenMode::ForRead);
        if (viewport.IsNull())
            {
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CantOpenObject(), "ViewportTableRecord");
            continue;
            }

        if (BSISUCCESS != this->_ImportModelspaceViewport(*viewport.get()))
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::ImportFailure(), Utf8String(viewport->GetName().c_str()).c_str(), "ViewportTableRecord");
        }
    
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportPaperspaceViewport (DgnModelR model, TransformCR transform, DwgDbLayoutCR layout)
    {
    DwgDbObjectIdArray  viewports;
    DwgDbDatabasePtr    dwg = layout.GetDatabase ();
    if (dwg.IsNull())
        return  BSIERROR;
    
    // the first in the list is the paperspace viewport
    DwgDbEntityCP       entity = nullptr;
    DwgDbViewportPtr    viewport;
    if (layout.GetViewports(viewports) == 0)
        {
        // no viewport initialized for this layout(or initialized but still returned none, a likely RealDWG bug), create a default viewport:
        viewport.CreateObject ();
        if (!viewport.IsNull())
            {
            DRange3d    range = layout.GetExtents ();
            if (range.IsPoint())
                return  BSIERROR;

            DPoint2d    center = DPoint2d::From (0.5 * (range.low.x + range.high.x), 0.5 * (range.low.y + range.high.y));

            viewport->SetViewCenter (center);
            viewport->SetViewHeight (range.YLength());
            viewport->SetWidth (range.XLength());
            }
        }
    else
        {
        // the layout has at least one viewport - use the first one as the layout/overall viewport
        viewport.OpenObject (viewports.front(), DwgDbOpenMode::ForRead);
        }

    Utf8String  layoutName (layout.GetName().c_str());
    if (layoutName.empty())
        layoutName.assign ("Layout");
   
    if (viewport.IsNull() || nullptr == (entity = DwgDbEntity::Cast(viewport.get())))
        {
        this->ReportIssue (IssueSeverity::Info, IssueCategory::InconsistentData(), Issue::CantOpenObject(), "the paperspace viewport", layoutName.c_str());
        return  BSIERROR;
        }

    // set sheet view name as "LayoutName - View"
    Utf8PrintfString    viewName ("%s - View", layoutName);
    ViewportFactory     factory (*this, *viewport.get(), &layout);

    DgnViewId   sheetViewId = factory.CreateSheetView (model.GetModelId(), viewName);

    if (!sheetViewId.IsValid())
        return  BSIERROR;

    // a sheet has been created - cache the results:
    DwgDbObjectId       paperspaceId = layout.GetBlockTableRecordId ();
    m_paperspaceViews.insert (T_PaperspaceView(sheetViewId, paperspaceId));

    if (!m_defaultViewId.IsValid())
        m_defaultViewId = sheetViewId;

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_PostProcessViewports ()
    {
    // add the DgnModels we imported through importing entities
    for (auto loadedXref : m_loadedXrefFiles)
        {
        if (loadedXref.GetLayoutspaceIdInRootFile() == m_modelspaceId)
            {
            m_modelspaceXrefs.insert (loadedXref.GetDgnModelIds().begin(), loadedXref.GetDgnModelIds().end());
            }
        else
            {
            DwgDbObjectId   blockId = loadedXref.GetBlockIdInParentFile ();
            DwgDbObjectId   paperspaceId = loadedXref.GetLayoutspaceIdInRootFile ();
            for (auto modelId : loadedXref.GetDgnModelIds())
                m_paperspaceXrefs.push_back (DwgXRefInPaperspace(blockId, paperspaceId, modelId));
            }
        }

    // Initialize the graphics subsystem (LoadViewController will hist nullptr==s_renderQueue otherwise!):
    DgnViewLib::GetHost().GetViewManager().Startup ();
    
    DwgSyncInfo::DwgFileId  fileId = DwgSyncInfo::DwgFileId::GetFrom (*m_dwgdb);

    // update each view with models or categories we have added since the creation of the view:
    for (auto const& entry : ViewDefinition::MakeIterator(*m_dgndb))
        {
        auto viewId = entry.GetId ();
        auto view = ViewDefinition::Get (*m_dgndb, viewId);
        if (!view.IsValid())
            continue;

        // get an editable view
        auto viewController = view->LoadViewController ();
        if (!viewController.IsValid())
            continue;

        bool changed = false;
        auto sheetView = view->ToSheetView ();

        if (nullptr == sheetView)
            {
            // a modelpace view or a spatial view for a viewport entity
            auto spatialView = viewController->ToSpatialViewP ();
            if (nullptr != spatialView && !m_modelspaceXrefs.empty())
                {
                // add xref models into the modelspace viewport:
                auto&   modelSelector = spatialView->GetSpatialViewDefinition().GetModelSelector ();
                modelSelector.GetModelsR().insert (m_modelspaceXrefs.begin(), m_modelspaceXrefs.end());
                modelSelector.Update ();
                changed = true;
                }
            }
        else
            {
            // a paperspace sheet view - add all drawing categories, except for viewport frozen layers:
            DwgDbObjectIdArray  frozenLayers;
            size_t              numFrozenLayers = 0;

            // see if the viewport has frozen layers:
            auto found = m_paperspaceViews.find (viewId);
            if (found != m_paperspaceViews.end())
                {
                DwgDbViewportPtr    layoutViewport (found->second, DwgDbOpenMode::ForRead);
                if (layoutViewport.OpenStatus() == DwgDbStatus::Success && layoutViewport->GetFrozenLayers(frozenLayers) == DwgDbStatus::Success)
                    numFrozenLayers = frozenLayers.size ();
                }

            auto& categorySelector = viewController->GetViewDefinition().GetCategorySelector ();
            for (ElementIteratorEntry entry : DrawingCategory::MakeIterator(*m_dgndb))
                {
                DgnCategoryId   categoryId = entry.GetId <DgnCategoryId> ();

                if (numFrozenLayers > 0)
                    {
                    DwgDbObjectId   layerId;
                    DwgDbHandle     objHandle = m_syncInfo.FindLayerHandle (categoryId, fileId);
                    if (!objHandle.IsNull() && (layerId = m_dwgdb->GetObjectId(objHandle)).IsValid())
                        {
                        // skip viewport frozen layer for this view:
                        auto found = std::find_if (frozenLayers.begin(), frozenLayers.end(), [&](DwgDbObjectId id){ return id == layerId; });
                        if (found != frozenLayers.end())
                            continue;
                        }
                    }

                categorySelector.AddCategory (categoryId);
                changed = true;
                }

#ifdef WIP_ADD_XREFMODELS_INSHEETVIEW
            // add xref models attached to this paperspace:
            for (auto const& xref : m_paperspaceXrefs)
                {
                if (xref.m_paperspaceId == paperspaceId)
                    models.insert (xref.m_dgnModelId);
                }
#endif
            }

        if (changed)
            this->SaveViewDefinition (*viewController);
        }
    }
