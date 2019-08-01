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
    m_sunId = viewportRecord.GetSunId ();
    m_isDefaultLightingOn = viewportRecord.IsDefaultLightingOn ();
    m_brightness = viewportRecord.GetBrightness ();
    m_ambientLightColor = viewportRecord.GetAmbientLightColor ();
    m_customScale = 1.0;
    m_backgroundColor = ColorDef::Black ();
    m_isPrivate = false;
    m_transform = importer.GetRootTransform ();
    m_inputViewport = DwgDbObject::Cast (&viewportRecord);
    m_viewportType = DwgSourceAspects::ViewAspect::SourceType::ModelSpaceViewport;
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
    m_sunId = viewportEntity.GetSunId ();
    m_customScale = viewportEntity.GetCustomScale ();
    m_backgroundColor = ColorDef::White ();
    m_isDefaultLightingOn = viewportEntity.IsDefaultLightingOn ();
    m_brightness = viewportEntity.GetBrightness ();
    m_ambientLightColor = viewportEntity.GetAmbientLightColor ();
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
        m_viewportType = DwgSourceAspects::ViewAspect::SourceType::PaperSpaceViewport;
        }
    else
        {
        // we are creating a spatial view for a view attachment - use view, instead of sheet, coordinates:
        m_center = DPoint3d::From (viewportEntity.GetViewCenter());

        double  aspectRatio = m_height > 1.0e-5 ? m_width / m_height : 1.0;

        m_height = viewportEntity.GetViewHeight ();
        m_width = m_height * aspectRatio;
        m_viewportType = DwgSourceAspects::ViewAspect::SourceType::ViewportEntity;
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

        // WIP - when/if Sheet::Border supports location in the future, switching code to set border origin, instead of moving geometry:
        LayoutFactory   factory (m_importer, block->GetLayoutId());
        factory.AlignSheetToPaperOrigin (trans);
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::TransformDataToBim ()
    {
    if (m_transform.IsIdentity())
        return;

    m_transform.Multiply (m_center);
    m_center.z = 0.0;

    m_transform.Multiply (m_target);
    m_transform.MultiplyMatrixOnly (m_viewDirection);

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

        /*-------------------------------------------------------------------------------
        Calculation of perspective view delta in MicroStation had 35*focalLength/lensLength,
        where focalLength was in UOR's and lensLength in unconverted DWG units. Here all of
        the view parameters have been converted to meters.
        -------------------------------------------------------------------------------*/
        if (aspectRatio > 1.0)
            {
            delta.x = 0.9 * focalLength / m_lensLength;
            delta.y = delta.x / aspectRatio;
            }
        else
            {
            delta.y = 0.9 * focalLength / m_lensLength;
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

#ifdef CLIP_THROUGH_SPATIALVIEW
    // if clipped by an entity, calculate & apply a clipping vector:
    this->ApplyViewportClipping (dgnView, frontClip, backClip);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ApplyViewportClipping (SpatialViewDefinitionR dgnView, double frontClip, double backClip)
    {
    // this method clips a SpatialView which is attached to a Sheet::ViewAttachment.
    if (m_clipEntityId.IsNull())
        return;

    RotMatrix   viewRotation = dgnView.GetRotation ();
    Transform   entityToClipper;
    if (!this->ComputeClipperTransformation(entityToClipper, viewRotation))
        return;

    // the clipper plane is the same as the view plane, but the clipper constructor expects transformation matrix from clipper to model:
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
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ApplyViewportClipping (Sheet::ViewAttachmentR viewAttachment)
    {
    // this method directly clips a Sheet::ViewAttachment.
    if (m_clipEntityId.IsNull())
        return;
    
    auto clipper = DwgHelper::CreateClipperFromEntity (m_clipEntityId, nullptr, nullptr, nullptr, &m_transform);
    if (clipper.IsValid())
        viewAttachment.SetClip (*clipper);
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
void            ViewportFactory::ComputeSheetView (ViewDefinitionR dgnView)
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

    // only when default lighting is turned off, the source lights become effective:
    if (m_isDefaultLightingOn)
        viewFlags.SetShowSourceLights (false);

    displayStyle.SetViewFlags (viewFlags);

    // set solar, scene lighting and backgrounds
    this->ComputeEnvironment (displayStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ComputeEnvironment (DisplayStyle3dR displayStyle)
    {
    auto&   environmentDisplay = displayStyle.GetEnvironmentDisplayR ();

    // default to no background & no sky
    environmentDisplay.m_groundPlane.m_enabled = false;
    environmentDisplay.m_skybox.m_enabled = false;

    ColorDef    ambientColor (m_ambientLightColor.GetRed(), m_ambientLightColor.GetGreen(), m_ambientLightColor.GetBlue());
    if (m_isDefaultLightingOn)
        displayStyle.SetBackgroundColor (m_backgroundColor);
    else
        displayStyle.SetBackgroundColor (ambientColor);
    
    bool    isPhotometric = DwgDbLightingUnits::None != m_importer.GetDwgDb().GetLightingUnits();

    Lighting::Parameters    light;
    DwgDbSunPtr sun(m_sunId, DwgDbOpenMode::ForRead);
    if (sun.OpenStatus() == DwgDbStatus::Success)
        {
        // set solar light
        light.SetType (Lighting::LightType::Solar);

        RgbFactor   photometricColor = sun->GetSunColorPhotometric (1.0);
        ColorDef    sunColor = DwgHelper::GetColorDefFromTrueColor (sun->GetSunColor());

        // NEEDSREVIEW: phisically based or calculated?
        light.SetColor (isPhotometric ? ColorDef(photometricColor.ToIntColor()) : sunColor);

        double  intensity = sun->GetIntensity ();
        light.SetIntensity (intensity);

        displayStyle.SetSolarLight (light, sun->GetSunDirection());

        if (sun->IsOn())
            {
            // turn the solar light on
            Render::ViewFlags   viewFlags = displayStyle.GetViewFlags ();
            viewFlags.SetShowSolarLight (true);
            displayStyle.SetViewFlags (viewFlags);
            }
        }
    else if (m_brightness > 0.0 && ambientColor != ColorDef::Black())
        {
        // set scene light
        light.SetType (Lighting::LightType::Ambient);
        light.SetIntensity (m_brightness);
        light.SetColor (ambientColor);

        displayStyle.SetSceneLight (light);
        }

    // set scene brightness
    if (m_brightness > 0.0)
        displayStyle.SetSceneBrightness (m_brightness);

    if (!m_backgroundId.IsValid())
        return;
    
    // get sky params from the sun:
    bool    hasSkyParams = false;
    DwgGiSkyParameters  skyParams;
    if (sun.OpenStatus() == DwgDbStatus::Success && sun->GetSkyParameters(skyParams) == DwgDbStatus::Success)
        hasSkyParams = true;

    // try Image Based Lighting first, as it might use secondary background.
    DwgDbIBLBackgroundPtr ibl(m_backgroundId, DwgDbOpenMode::ForRead);
    if (ibl.OpenStatus() == DwgDbStatus::Success)
        {
        if (!ibl->IsEnabled())
            return;

        if (ibl->IsImageDisplayed())
            {
            // an image file is used for the IBL background:
            environmentDisplay.m_skybox.m_enabled = true;

            BeFileName  imageFile(ibl->GetIBLImageName().c_str());
            auto textureId = this->FindEnvironmentImageFile (imageFile);
            if (textureId.IsValid())
                {
                environmentDisplay.m_skybox.m_image.m_type = DisplayStyle3d::EnvironmentDisplay::SkyBox::Image::Type::Spherical;
                environmentDisplay.m_skybox.m_image.m_textureId = textureId;
                }
            return;
            }

        // fall back to secondary background
        m_backgroundId = ibl->GetSecondaryBackground ();
        if (!m_backgroundId.IsValid())
            return;
        }

    DwgDbImageBackgroundPtr image(m_backgroundId, DwgDbOpenMode::ForRead);
    if (image.OpenStatus() == DwgDbStatus::Success)
        {
        // an image file is used as background
        environmentDisplay.m_skybox.m_enabled = true;

        BeFileName  imageFile(image->GetImageFileName().c_str());
        auto textureId = this->FindEnvironmentImageFile (imageFile);
        if (textureId.IsValid())
            {
            environmentDisplay.m_skybox.m_image.m_type = DisplayStyle3d::EnvironmentDisplay::SkyBox::Image::Type::Spherical;
            environmentDisplay.m_skybox.m_image.m_textureId = textureId;
            }
        return;
        }

    DwgDbGroundPlaneBackgroundPtr   groundPlane(m_backgroundId, DwgDbOpenMode::ForRead);
    if (groundPlane.OpenStatus() == DwgDbStatus::Success)
        {
        // DWG ground plane
        environmentDisplay.m_skybox.m_enabled = true;
        environmentDisplay.m_skybox.m_twoColor = true;
        environmentDisplay.m_skybox.m_zenithColor = DwgHelper::GetColorDefFromTrueColor (groundPlane->GetColorSkyZenith());
        environmentDisplay.m_skybox.m_nadirColor = DwgHelper::GetColorDefFromTrueColor (groundPlane->GetColorUndergroundHorizon());
        environmentDisplay.m_skybox.m_groundColor = DwgHelper::GetColorDefFromTrueColor (groundPlane->GetColorGroundPlaneNear());
        environmentDisplay.m_skybox.m_skyColor = DwgHelper::GetColorDefFromTrueColor (groundPlane->GetColorSkyHorizon());
        return;
        }

    DwgDbSkyBackgroundPtr   sky(m_backgroundId, DwgDbOpenMode::ForRead);
    if (sky.OpenStatus() == DwgDbStatus::Success)
        {
        // sky
        environmentDisplay.m_skybox.m_enabled = true;
        environmentDisplay.m_skybox.m_twoColor = true;

        static ColorDef s_daytimeHorizon(170, 200, 250,0);
        static ColorDef s_daytimeTop(40, 130, 250, 0);
        if (hasSkyParams)
            {
            // sun + sky
            if (sun->GetAltitude() >= skyParams.GetHorizonHeight())
                {
                // day colors
                environmentDisplay.m_skybox.m_zenithColor = s_daytimeTop;
                environmentDisplay.m_skybox.m_nadirColor = ambientColor;
                environmentDisplay.m_skybox.m_skyColor = s_daytimeHorizon;
                }
            else
                {
                // night colors
                environmentDisplay.m_skybox.m_zenithColor =
                environmentDisplay.m_skybox.m_nadirColor =
                environmentDisplay.m_skybox.m_skyColor = DwgHelper::GetColorDefFromTrueColor (skyParams.GetNightColor());
                }
            environmentDisplay.m_skybox.m_groundColor = DwgHelper::GetColorDefFromTrueColor (skyParams.GetGroundColor());
            }
        else
            {
            // no sun
            environmentDisplay.m_skybox.m_zenithColor = s_daytimeTop;
            environmentDisplay.m_skybox.m_nadirColor = ambientColor;
            environmentDisplay.m_skybox.m_skyColor = s_daytimeHorizon;
            environmentDisplay.m_skybox.m_groundColor = ambientColor;
            }
        return;
        }

    DwgDbSolidBackgroundPtr solid(m_backgroundId, DwgDbOpenMode::ForRead);
    if (solid.OpenStatus() == DwgDbStatus::Success)
        {
        // single solid color
        environmentDisplay.m_groundPlane.m_enabled = true;
        environmentDisplay.m_groundPlane.m_aboveColor = 
        environmentDisplay.m_groundPlane.m_belowColor = DwgHelper::GetColorDefFromTrueColor (solid->GetColorSolid());

        displayStyle.SetBackgroundColor (environmentDisplay.m_groundPlane.m_aboveColor);
        return;
        }

    DwgDbGradientBackgroundPtr  gradient(m_backgroundId, DwgDbOpenMode::ForRead);
    if (gradient.OpenStatus() == DwgDbStatus::Success)
        {
        //set top color as background color, middle color for above and bottom color for below:
        environmentDisplay.m_groundPlane.m_enabled = true;

        environmentDisplay.m_groundPlane.m_aboveColor = DwgHelper::GetColorDefFromTrueColor (gradient->GetColorMiddle());
        environmentDisplay.m_groundPlane.m_belowColor = DwgHelper::GetColorDefFromTrueColor (gradient->GetColorBottom());

        displayStyle.SetBackgroundColor (DwgHelper::GetColorDefFromTrueColor(gradient->GetColorTop()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId    ViewportFactory::FindEnvironmentImageFile (BeFileNameCR filenameIn) const
    {
    BeFileName  filename = filenameIn;
    if (m_importer._FindTextureFile(filename))
        {
        // warn about a none-JPEG file
        if (!filename.GetExtension().EqualsI(L"jpg"))
            m_importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::Unsupported(), Issue::ImageNotAJpeg(), filename.GetNameUtf8().c_str());

        return m_importer.GetDgnMaterialTextureFor(filename.GetNameUtf8());
        }
    
    // warn about the missing background file:
    Utf8PrintfString    context("Background \"%ls\"", filename.c_str());
    m_importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::MissingData(), Issue::FileNotFound(), filename.GetNameUtf8().c_str(), context.c_str());
    return  DgnTextureId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::ComputeSheetDisplayStyle (DisplayStyleR displayStyle)
    {
    ViewFlags   viewFlags;
    DwgHelper::SetViewFlags (viewFlags, m_isGridOn, m_isUcsIconOn, false, true, m_isFrontClipped, m_isBackClipped, m_importer.GetDwgDb());

    viewFlags.SetRenderMode (RenderMode::Wireframe);
    viewFlags.SetShowVisibleEdges (true);
    viewFlags.SetShowHiddenEdges (true);
    viewFlags.SetShowCameraLights (false);
    viewFlags.SetShowSolarLight (false);
    viewFlags.SetShowSourceLights (false);
    viewFlags.SetShowShadows (false);
    
    displayStyle.SetViewFlags (viewFlags);
    displayStyle.SetBackgroundColor (m_backgroundColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::AddModelspaceCategories (Utf8StringCR viewName)
    {
    DefinitionModelP model = m_importer.GetOrCreateJobDefinitionModel().get ();
    if (nullptr == model)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "CategorySelector");
        model = &m_importer.GetDgnDb().GetDictionaryModel ();
        }

    m_categories = new CategorySelector (*model, viewName.c_str());

    DgnCategoryIdSet&   categoryIdSet = m_categories->GetCategoriesR ();

    this->UpdateModelspaceCategories (categoryIdSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ViewportFactory::IsLayerDisplayed (DwgDbHandleCR layerHandle, DwgDbObjectIdArrayCR vpfrozenLayers, DwgDbDatabaseR dwg) const
    {
    DwgDbObjectId   layerId;
    if (layerHandle.IsNull() || !(layerId = dwg.GetObjectId(layerHandle)).IsValid())
        {
        if (layerHandle.IsNull())
            BeAssert (false && "Non-database resident layers are not supported!");
        return  false;
        }

    // layer display is per master file - find master file's layer representing the xRef layer
    bool isLayerInXref = m_importer.m_dwgdb.get() != &dwg;
    if (isLayerInXref)
        {
        auto layerMapping = m_importer.GetLayerMappingCache ();
        auto found = layerMapping.find (layerId);
        if (found == layerMapping.end())
            {
            BeAssert (false && "Xref layer has not been cached!");
            return  false;
            }
        layerId = found->second.GetLayerIdInMasterFile ();
        }

    // if this is a viewport in layout, check viewport frozen layers:
    if (!vpfrozenLayers.empty())
        {
        // erase category from the view if the layer is viewport frozen:
        auto found = std::find_if (vpfrozenLayers.begin(), vpfrozenLayers.end(), [&](DwgDbObjectId id){ return id == layerId; });
        if (found != vpfrozenLayers.end())
            return  false;
        }

    // check the layer's global on/off status and global freeze:
    DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);
    if (layer.OpenStatus() == DwgDbStatus::Success)
        return !layer->IsOff() && !layer->IsFrozen();

    // default to displaying a layer which has a valid ID.
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewportFactory::UpdateModelspaceCategories (DgnCategoryIdSet& categoryIdSet) const
    {
    // add all spatial categories to the modelspace view, but only add displayed ones to a viewport view:
    DwgDbObjectIdArray  frozenLayers;
    DwgDbViewportCP     viewportEntity = DwgDbViewport::Cast (m_inputViewport);

    // if this is for a viewport entity, get viewport frozen layer ID's:
    if (nullptr != viewportEntity)
        viewportEntity->GetFrozenLayers (frozenLayers);

    // add all displayed categories created from the layer table of the root file to the view:
    this->UpdateModelspaceCategories(categoryIdSet, frozenLayers, m_importer.GetDwgDb());

    // add all displayed categories created from xRef layers
    auto loadedXrefs = m_importer.GetLoadedXrefs ();
    for (auto xref : loadedXrefs)
        this->UpdateModelspaceCategories(categoryIdSet, frozenLayers, xref.GetDatabaseR());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewportFactory::UpdateModelspaceCategories(DgnCategoryIdSet& categoryIdSet, DwgDbObjectIdArrayCR frozenLayers, DwgDbDatabaseR dwg) const
    {
    // update modelspace categories per file/scope
    auto scopeId = m_importer.GetRepositoryLink(&dwg);
    auto& db = m_importer.GetDgnDb ();
    for (auto aspect : DwgSourceAspects::LayerAspectIterator(db, scopeId))
        {
        DgnCategoryId   categoryId = DgnSubCategory::QueryCategoryId (db, aspect.GetSubCategoryId());
        if (!categoryId.IsValid())
            {
            m_importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::Sync(), Issue::Message(), "a layer is not correctly sync'ed");
            continue;
            }
#ifdef DEBUG
        auto layerName = aspect.GetLayerName ();
#endif

        // add the category if displayed and remove it if not:
        if (this->IsLayerDisplayed(aspect.GetLayerHandle(), frozenLayers, dwg))
            categoryIdSet.insert (categoryId);
        else
            categoryIdSet.erase (categoryId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ViewportFactory::ValidateViewName (Utf8StringR viewNameInOut)
    {
    DgnDbR  dgndb = m_importer.GetDgnDb ();
    DefinitionModelP model = m_importer.GetOrCreateJobDefinitionModel().get ();
    if (nullptr == model)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "ViewDefinition");
        model = &dgndb.GetDictionaryModel ();
        }

    auto viewIdOut = ViewDefinition::QueryViewId (*model, viewNameInOut);

    if (viewIdOut.IsValid())
        {
        // deduplicate view name
        Utf8String  suffix;
        uint32_t    count = 1;
        Utf8String  fileName = Utf8String (m_importer.GetDwgDb().GetFileName().c_str());
        Utf8String  proposedName = viewNameInOut;

        do
            {
            suffix.Sprintf ("-%d", count++);
            viewNameInOut = m_importer.RemapNameString (fileName, proposedName, suffix);
            } while (ViewDefinition::QueryViewId(*model, viewNameInOut).IsValid());
        }

    // return false to create a new view with the validated name.
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ViewportFactory::UpdateViewName (ViewDefinitionR view, Utf8StringCR proposedName)
    {
    if (view.GetName().Equals(proposedName))
        return  false;

    Utf8String  newName = proposedName;
    this->ValidateViewName (newName);

    // change view name - caller shall update element
    auto code = view.GetCode ();
    auto status = view.SetCode (DgnCode::From(code.GetCodeSpecId(), code.GetScopeString(), newName));
    if (status != DgnDbStatus::Success)
        {
        m_importer.ReportIssueV (DwgImporter::IssueSeverity::Error, IssueCategory::Briefcase(), Issue::CannotUpdateName(), "View", view.GetName().c_str(), newName.c_str());
        return  false;
        }

    // change category selector name - caller shall update element
    auto userLabel = DataStrings::GetString (DataStrings::CategorySelector());
    auto& categorySelector = view.GetCategorySelector ();
    m_importer.UpdateElementName (categorySelector, newName, userLabel.c_str(), false);
    
    // change display style name - caller shall update element
    userLabel = DataStrings::GetString (DataStrings::DisplayStyle());
    auto& displayStyle = view.GetDisplayStyle ();
    m_importer.UpdateElementName (displayStyle, newName, userLabel.c_str(), false);

    // change model selector name - do not expect caller to update element
    auto spatialView = view.ToSpatialViewP ();
    if (nullptr != spatialView)
        {
        userLabel = DataStrings::GetString (DataStrings::ModelSelector());
        m_importer.UpdateElementName (spatialView->GetModelSelector(), newName, userLabel.c_str(), true);
        }

    return  status == DgnDbStatus::Success;
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
void            ViewportFactory::UpdateViewAspect (ViewDefinitionR view, Utf8StringCR viewName, bool isNew)
    {
    // insert/update viewport into syncInfo
    auto aspect = m_importer.GetSourceAspects().AddOrUpdateViewAspect (view, m_inputViewport->GetObjectId(), m_viewportType, viewName);
    BeAssert (aspect.IsValid());

    // update the change detector
    m_importer._GetChangeDetector()._OnViewSeen (m_importer, view.GetViewId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId       ViewportFactory::CreateDrawingView (DgnModelId modelId, Utf8StringCR proposedName)
    {
    DgnDbR      dgndb = m_importer.GetDgnDb ();
    Utf8String  viewName (proposedName);
    DgnViewId   viewId;

    this->ValidateViewName (viewName);

    DefinitionModelP definitionModel = m_importer.GetOrCreateJobDefinitionModel().get ();
    if (nullptr == definitionModel)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "SpatialView");
        definitionModel = &dgndb.GetDictionaryModel ();
        }

    // add all spatial categories to the view, with viewport frozen layers applied:
    this->AddModelspaceCategories (viewName);

    // create a display style:
    DisplayStyle2dPtr   displayStyle = new DisplayStyle2d (*definitionModel, viewName.c_str());
    this->ComputeSheetDisplayStyle (*displayStyle);

    // create a DrawingView and compute its params as if it's a sheet view:
    DrawingViewDefinitionPtr view = new DrawingViewDefinition (*definitionModel, viewName, modelId, *m_categories, *displayStyle->ToDisplayStyle2dP());
    this->ComputeSheetView (*view);

    view->SetIsPrivate (m_isPrivate);
    view->SetUserLabel (DataStrings::GetString(DataStrings::ModelView()).c_str());

    // insert the view to BIM
    if (!view.IsValid() || view->Insert().IsNull())
        {
        m_importer.ReportError(IssueCategory::CorruptData(), Issue::Error(), viewName.c_str());
        return viewId;
        }

    viewId = view->GetViewId ();
    this->UpdateViewAspect (*view, proposedName, true);

    return  viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ViewportFactory::UpdateDrawingView (DgnViewId viewId, Utf8StringCR proposedName)
    {
    auto drawingView = m_importer.GetDgnDb().Elements().GetForEdit<DrawingViewDefinition> (viewId);
    if (!drawingView.IsValid())
        return  static_cast<BentleyStatus>(DgnDbStatus::ViewNotFound);

    // if the view name has been changed, reset it in affected elements which will be updated as follows
    this->UpdateViewName (*drawingView, proposedName);
  
    // update categories
    auto& categorySelector = drawingView->GetCategorySelector ();
    this->UpdateModelspaceCategories (categorySelector.GetCategoriesR());
    categorySelector.Update ();

    // update display style:
    auto& displayStyle = drawingView->GetDisplayStyle ();
    this->ComputeSheetDisplayStyle (displayStyle);
    displayStyle.Update ();

    // re-compute the view, same as sheet view
    this->ComputeSheetView (*drawingView);

    if (!drawingView->Update().IsValid())
        m_importer.ReportError (IssueCategory::Briefcase(), Issue::UpdateFailure(), drawingView->GetName().c_str());

    this->UpdateViewAspect (*drawingView, proposedName, false);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId       ViewportFactory::CreateSpatialView (DgnModelId modelId, Utf8StringCR proposedName)
    {
    DgnDbR      dgndb = m_importer.GetDgnDb ();
    Utf8String  viewName (proposedName);
    DgnViewId   viewId;

    this->ValidateViewName (viewName);

    DefinitionModelP model = m_importer.GetOrCreateJobDefinitionModel().get ();
    if (nullptr == model)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "SpatialView");
        model = &dgndb.GetDictionaryModel ();
        }

    // only add the master file's modelspace at this time - other models may be added in post process:
    ModelSelectorPtr    models = new ModelSelector (*model, viewName.c_str());
    models->AddModel (modelId);

    // add all spatial categories to the view, with viewport frozen layers applied:
    this->AddModelspaceCategories (viewName);

    // create a display style:
    DisplayStyle3dPtr   displayStyle = new DisplayStyle3d (*model, viewName.c_str());
    this->ComputeSpatialDisplayStyle (*displayStyle);

    // add a default camera for now, if there is one, and will set it later in ComputerSpatialView:
    ViewDefinition3d::Camera    camera;
    ViewDefinition3d::Camera*   optionalCamera = m_hasCamera ? &camera : nullptr;

    // create a CameraView for a perspective viewport or an OrthographicView otherwise:
    SpatialViewDefinitionPtr view = new SpatialViewDefinition (*model, viewName, *m_categories, *displayStyle, *models, optionalCamera);

    // convert DWG viewport data to DgnView
    this->ComputeSpatialView (*view);

    view->SetIsPrivate (m_isPrivate);
    view->SetUserLabel (DataStrings::GetString(DataStrings::ModelView()).c_str());

    // insert the view to BIM
    if (!view.IsValid() || view->Insert().IsNull())
        {
        m_importer.ReportError(IssueCategory::CorruptData(), Issue::Error(), viewName.c_str());
        return viewId;
        }

    viewId = view->GetViewId ();
    this->UpdateViewAspect (*view, proposedName, true);

    return  viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ViewportFactory::UpdateSpatialView (DgnViewId viewId, Utf8StringCR proposedName)
    {
    auto spatialView = m_importer.GetDgnDb().Elements().GetForEdit<SpatialViewDefinition> (viewId);
    if (!spatialView.IsValid())
        return  static_cast<BentleyStatus>(DgnDbStatus::ViewNotFound);

    // if the view name has been changed, reset it in affected elements which will be updated as follows
    this->UpdateViewName (*spatialView, proposedName);
  
    // update categories
    auto& categorySelector = spatialView->GetCategorySelector ();
    this->UpdateModelspaceCategories (categorySelector.GetCategoriesR());
    categorySelector.Update ();

    // update display style:
    auto& displayStyle = spatialView->GetDisplayStyle3d ();
    this->ComputeSpatialDisplayStyle (displayStyle);
    displayStyle.Update ();

    // re-compute the view
    this->ComputeSpatialView (*spatialView);

    if (!spatialView->Update().IsValid())
        m_importer.ReportError (IssueCategory::Briefcase(), Issue::UpdateFailure(), spatialView->GetName().c_str());

    this->UpdateViewAspect (*spatialView, proposedName, false);

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

    this->ValidateViewName (viewName);

    DefinitionModelP model = m_importer.GetOrCreateJobDefinitionModel().get ();
    if (nullptr == model)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "SheetViewDefinition");
        model = &dgndb.GetDictionaryModel ();
        }

    // add an empty drawing category to the view - will update in the post process:
    CategorySelectorPtr categories = new CategorySelector (*model, viewName.c_str());

    // create a display style for the sheet
    DisplayStyle2dPtr   displayStyle = new DisplayStyle2d (*model, viewName.c_str());
    this->ComputeSheetDisplayStyle (*displayStyle);

    // create a new sheet view
    SheetViewDefinitionPtr  view = new SheetViewDefinition (*model, viewName, modelId, *categories, *displayStyle);
    if (!view.IsValid())
        {
        BeAssert (false && "failed creating SheetViewDefinition!");
        return viewId;
        }

    // convert viewport data to the sheet view:
    this->ComputeSheetView (*view.get());

    view->SetIsPrivate (m_isPrivate);
    view->SetUserLabel (DataStrings::GetString(DataStrings::LayoutView()).c_str());

    // now insert the new view into DB:
    if (view->Insert().IsNull())
        {
        m_importer.ReportError(IssueCategory::UnexpectedData(), Issue::Error(), viewName.c_str());
        return viewId;
        }

    viewId = view->GetViewId ();

    // insert viewport into syncInfo and update the change detector:
    this->UpdateViewAspect (*view, proposedName, true);

    return  viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ViewportFactory::UpdateSheetView (DgnViewId viewId, Utf8StringCR proposedName)
    {
    auto sheetView = m_importer.GetDgnDb().Elements().GetForEdit<SheetViewDefinition> (viewId);
    if (!sheetView.IsValid())
        return  static_cast<BentleyStatus>(DgnDbStatus::ViewNotFound);

    // if the view name has been changed, reset it in affected elements which will be updated as follows
    auto nameChanged = this->UpdateViewName (*sheetView, proposedName);

    // categories will be updated in post process, if name is not changed
    if (nameChanged)
        sheetView->GetCategorySelector().Update ();

    // update display style:
    auto& displayStyle = sheetView->GetDisplayStyle ();
    this->ComputeSheetDisplayStyle (displayStyle);
    displayStyle.Update ();

    // re-compute the sheet view
    this->ComputeSheetView (*sheetView.get());
    sheetView->SetIsPrivate (m_isPrivate);

    if (!sheetView->Update().IsValid())
        m_importer.ReportError (IssueCategory::Briefcase(), Issue::UpdateFailure(), sheetView->GetName().c_str());

    this->UpdateViewAspect (*sheetView, proposedName, false);

    return BSISUCCESS;
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

    // clip the view attachment
    this->ApplyViewportClipping (*viewAttachment);

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

    // clip the view attachment
    this->ApplyViewportClipping (*viewAttachment);

    return  viewAttachment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ViewportFactory::UpdateModelView (DgnModelCR targetModel, DgnViewId viewId, Utf8StringCR proposedName)
    {
    if (targetModel.IsSpatialModel())
        return  this->UpdateSpatialView (viewId, proposedName);
    else if (targetModel.IsDrawingModel())
        return  this->UpdateDrawingView (viewId, proposedName);
    else if (targetModel.IsSheetModel())
        return  this->UpdateSheetView (viewId, proposedName);
    else
        return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId       ViewportFactory::CreateModelView (DgnModelCR targetModel, Utf8StringCR proposedName)
    {
    if (targetModel.IsSpatialModel())
        return  this->CreateSpatialView (targetModel.GetModelId(), proposedName);
    else if (targetModel.IsDrawingModel())
        return  this->CreateDrawingView (targetModel.GetModelId(), proposedName);
    else if (targetModel.IsSheetModel())
        return  this->CreateSheetView (targetModel.GetModelId(), proposedName);
    else
        return  DgnViewId();
    }





/*=================================================================================**//**
* DwgViewportExt extension
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgViewportExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporter& importer)
    {
    // this is a viewport entity that displays the modelspace, not an overall viewport for a paperspace!
    DwgDbEntityPtr&     entity = context.GetEntityPtrR ();
    DwgDbViewportCP     viewport = DwgDbViewport::Cast (entity.get());
    if (nullptr == viewport)
        return  BSIERROR;

    // get the modelspace model:
    DgnModelP   rootModel = nullptr;
    ResolvedModelMapping    modelMap = importer.GetRootModel ();
    if (!modelMap.IsValid() || (rootModel = modelMap.GetModel()) == nullptr)
        {
        BeAssert(false && L"failed retrieving modelspace model!");
        return BSIERROR;
        }

    // set the view name as "LayoutName Viewport-ID":
    DgnModelR           sheetModel = context.GetModel ();
    Utf8PrintfString    viewName ("%s%s%llx", sheetModel.GetName().c_str(), ViewportFactory::GetSpatialViewNameInsert(), viewport->GetObjectId().ToUInt64());

    // this method gets called for either creating a new or updating existing element.
    if (importer.IsUpdating() && context.GetElementResults().GetExistingElement().IsValid())
        return  this->UpdateBim(context, importer, *rootModel, sheetModel, viewName);

    // create a view for the modelspace/root model:
    ViewportFactory factory (importer, *viewport);
    // use white background color for the model view:
    factory.SetBackgroundColor (ColorDef::White());
    // the spatial view is generated for a view attachment in next step, hide it from the user:
#if defined(NDEBUG) || !defined(DEBUG)
    factory.SetViewSourcePrivate (true);
#endif

    // create a spatial, drawing, or sheet view for the root model
    DgnViewId   modelViewId = factory.CreateModelView (*rootModel, viewName);
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
BentleyStatus   DwgViewportExt::UpdateBim (ProtocolExtensionContext& context, DwgImporter& importer, DgnModelCR rootModel, DgnModelCR sheetModel, Utf8StringCR viewName)
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

    // get or create a view of the modelspace model:
    BentleyStatus   status = factory.UpdateModelView (rootModel, modelViewId, viewName);
    if (BSISUCCESS != status)
        {
        // delete the view and re-import it anew:
        importer.GetDgnDb().Elements().Delete (modelViewId);

        Utf8PrintfString name("%s Viewport-%llx", sheetModel.GetName().c_str(), viewport->GetObjectId().ToUInt64());

        modelViewId = factory.CreateModelView (rootModel, name);
        if (modelViewId.IsValid())
            {
            BeAssert (false && "failed creating a spatial view for a viewport entity");
            return  BSIERROR;
            }
        }

    DwgDbObjectCP   obj = DwgDbObject::Cast(entity.get());
    if (nullptr == obj)
        return  BSIERROR;

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
    // SpatialView & SheetView are tracked separately in syncInfo.
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
    auto& viewDef = viewController.GetViewDefinitionR();

    viewDef.GetCategorySelector().Update ();
    viewDef.GetDisplayStyle().Update ();
    viewDef.Update ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId   DwgImporter::_ImportModelspaceViewport (DwgDbViewportTableRecordCR dwgVport)
    {
    DgnViewId       viewId;
    DwgDbDatabaseP  dwg = dwgVport.GetDatabase().get ();
    if (nullptr == dwg)
        {
        BeAssert(false);
        return viewId;
        }
    DgnModelP   rootModel = nullptr;
    ResolvedModelMapping    modelMap = this->FindModel (this->GetModelSpaceId(), this->GetRootTransform(), DwgSourceAspects::ModelAspect::SourceType::ModelSpace);
    if (!modelMap.IsValid() || (rootModel = modelMap.GetModel()) == nullptr)
        {
        BeAssert(false && L"root model has not been imported yet!");
        return viewId;
        }
        
    // set Model view name as the model name (also the same as the layout name):
    Utf8String  viewName = rootModel->GetName ();

    // build a view factory for either importing or updating:
    ViewportFactory factory(*this, dwgVport);

    if (this->IsUpdating())
        {
        viewId = m_sourceAspects.FindView (dwgVport.GetObjectId(), DwgSourceAspects::ViewAspect::SourceType::ModelSpaceViewport);
        if (viewId.IsValid())
            {
            factory.UpdateModelView (*rootModel, viewId, viewName);
            return  viewId;
            }
        }

    viewId = factory.CreateModelView (*rootModel, viewName);

    return viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportModelspaceViewports ()
    {
    DwgDbViewportTablePtr   viewportTable (m_dwgdb->GetViewportTableId(), DwgDbOpenMode::ForRead);
    if (viewportTable.IsNull())
        return  BSIERROR;

    DwgDbSymbolTableIteratorPtr iter = viewportTable->NewIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BSIERROR;

    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbViewportTableRecordPtr     viewport (iter->GetRecordId(), DwgDbOpenMode::ForRead);
        if (viewport.IsNull())
            {
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CantOpenObject(), "ViewportTableRecord");
            continue;
            }

        auto viewId = this->_ImportModelspaceViewport (*viewport.get());
        if (viewId.IsValid())
            {
            // do not let the default view unset; override it if active viewport exists:
            if (m_activeViewportId == viewport->GetObjectId() || !m_defaultViewId.IsValid())
                m_defaultViewId = viewId;
            }
        else
            {
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::ImportFailure(), Utf8String(viewport->GetName().c_str()).c_str(), "ViewportTableRecord");
            }
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
    if (layout.GetViewports(viewports) == 0)
        {
        /*-------------------------------------------------------------------------------
        Above call can fail if the layout is not initialized. But now we choose to activate 
        each layout before processing its viewports. So we expect the call to succeed.
        But we still keep below code as a fallback anyway just in case.
        -------------------------------------------------------------------------------*/
        DwgDbBlockTableRecordPtr    block(layout.GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
        if (block.OpenStatus() == DwgDbStatus::Success)
            viewports.push_back (LayoutFactory::FindOverallViewport(*block.get()));
        }

    Utf8String  layoutName (layout.GetName().c_str());
    if (layoutName.empty())
        layoutName.assign ("Layout");

    DwgDbViewportPtr viewport (viewports.front(), DwgDbOpenMode::ForRead);
    if (viewport.OpenStatus() != DwgDbStatus::Success || nullptr == DwgDbEntity::Cast(viewport.get()))
        {
        this->ReportIssue (IssueSeverity::Info, IssueCategory::InconsistentData(), Issue::CantOpenObject(), "the paperspace viewport", layoutName.c_str());
        return  BSIERROR;
        }
   
    // set sheet view name as the model name:
    Utf8String  viewName = model.GetName ();

    // build a view factory for either importing or updating:
    ViewportFactory factory (*this, *viewport.get(), &layout);
    DwgDbObjectId   viewportId = viewport->GetObjectId ();
    DgnViewId       sheetViewId;

    if (this->IsUpdating())
        {
        sheetViewId = m_sourceAspects.FindView (viewportId, DwgSourceAspects::ViewAspect::SourceType::PaperSpaceViewport);
        if (sheetViewId.IsValid())
            return factory.UpdateSheetView (sheetViewId, viewName);
        }

    // create a new sheet view
    sheetViewId = factory.CreateSheetView (model.GetModelId(), viewName);
    if (!sheetViewId.IsValid())
        return  BSIERROR;

    // a sheet has been created - cache the results:
    m_paperspaceViews.insert (T_PaperspaceView(sheetViewId, viewportId));

    // do not let the default view unset; override it if a valid active layout exists:
    if (viewportId == m_activeViewportId || !m_defaultViewId.IsValid())
        m_defaultViewId = sheetViewId;

    return  BSISUCCESS;
    }


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          03/18
+===============+===============+===============+===============+===============+======*/
struct ThumbnailConfig
    {
private:
    enum class ViewTypes
        {
        None        = 0,
        Physical    = 1 << 0,
        Sheet       = 1 << 1,
        };  // ViewTypes

    int m_resolution;
    Render::RenderMode m_renderModeOverride;
    bool m_isRenderModeOverridden;
    ViewTypes m_viewTypes;
    
public:
    // the constructor
    ThumbnailConfig (DwgImporter::Config& config)
        {
        // read resolution
        m_resolution = static_cast <int> (config.GetXPathInt64("/ConvertConfig/Thumbnails/@pixelResolution", 768));
        if (m_resolution < 64 || m_resolution > 1600)
            m_resolution = 768;

        // read render mode
        Utf8String renderMode = config.GetXPathString("/ConvertConfig/Thumbnails/@renderModeOverride", "");
        m_renderModeOverride = Render::RenderMode::Wireframe;
        m_isRenderModeOverridden = true;
        if (renderMode.EqualsI("Wireframe"))
            m_renderModeOverride = Render::RenderMode::Wireframe;
        else if (renderMode.EqualsI("HiddenLine"))
            m_renderModeOverride = Render::RenderMode::HiddenLine;
        else if (renderMode.EqualsI("SmoothShape"))
            m_renderModeOverride = Render::RenderMode::SmoothShade;
        else if (renderMode.EqualsI("SolidFill"))
            m_renderModeOverride = Render::RenderMode::SolidFill;
        else
            m_isRenderModeOverridden = false;

        // read view types desired to have thumbnails
        Utf8String viewTypes = config.GetXPathString("/ConvertConfig/Thumbnails/@viewTypes", "");
        size_t offset = 0;
        Utf8String nextValue;
        int intValue = static_cast<int> (ViewTypes::None);
        while ((offset = viewTypes.GetNextToken(nextValue, " ", offset)) != Utf8String::npos)
            {
            if (nextValue.Equals("Physical"))
                intValue |= static_cast<int> (ViewTypes::Physical);
            else if (nextValue.Equals("Sheet"))
                intValue |= static_cast<int> (ViewTypes::Sheet);
            }
        m_viewTypes = static_cast<ViewTypes> (intValue);
        }

    // call these to check the config's
    int GetResolution () const { return m_resolution; }
    bool IsRenderModeOverridden () const { return  m_isRenderModeOverridden; }
    Render::RenderMode GetOverriddenRenderMode () const { return m_renderModeOverride; }
    bool WantPhysicalThumbnail () const { return (int)ViewTypes::Physical == ((int)m_viewTypes & (int)ViewTypes::Physical); }
    bool WantSheetThumbnail () const { return (int)ViewTypes::Sheet == ((int)m_viewTypes & (int)ViewTypes::Sheet); }
    };  // ThumbnailConfig

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_PostProcessViewports ()
    {
    auto scopeId = this->GetRepositoryLink (m_dwgdb.get());
    if (!scopeId.IsValid())
        return;

    bool wantThumbnails = this->GetOptions().WantThumbnails ();
    BeDuration timeout = this->GetOptions().GetThumbnailTimeout ();

#if defined(TODO_IMODEL02_THUMBNAILS)
    // ensure graphics sub-system has started, if we want to generate thumbnails:
    if (wantThumbnails)
        {
        this->SetStepName(ProgressMessage::STEP_CREATE_THUMBNAILS());
        DgnViewLib::GetHost().GetViewManager().Startup();
        }
#else
    wantThumbnails = false;
#endif // TODO_IMODEL02_THUMBNAILS

    // read thumbnail options from the config file:
    ThumbnailConfig thumbnailConfig(m_config);
    Render::RenderMode mode = thumbnailConfig.GetOverriddenRenderMode ();
    Point2d size = {thumbnailConfig.GetResolution(), thumbnailConfig.GetResolution()};

    // update each view with models or categories we have added since the creation of the view:
    for (auto aspect : DwgSourceAspects::ViewAspectIterator(*m_dgndb, scopeId))
        {
        auto viewId = aspect.GetViewId ();
        auto view = ViewDefinition::Get (*m_dgndb, viewId);
        if (!view.IsValid())
            continue;

        // get an editable view
        auto viewController = view->LoadViewController ();
        if (!viewController.IsValid())
            continue;

        bool changed = false;
        bool createThumbnail = false;

        if (aspect.GetSourceType() != DwgSourceAspects::ViewAspect::SourceType::PaperSpaceViewport)
            {
            // a modelspace view, a viewport entity view, or a paperspace's xRef view:
            changed = this->UpdateModelspaceView (viewController.get());

            // should we create a thumbnail for a modelspace view?
            if (wantThumbnails && !view->GetName().Contains(ViewportFactory::GetSpatialViewNameInsert()) &&
                ((view->IsSpatialView() && thumbnailConfig.WantPhysicalThumbnail()) || (view->IsSheetView() && thumbnailConfig.WantSheetThumbnail())))
                createThumbnail = true;
            }
        else
            {
            // a paperspace sheet view - add paperspace drawing categories, except for viewport frozen layers:
            changed = this->UpdatePaperspaceView (viewController.get(), m_dwgdb->GetObjectId(aspect.GetSourceHandle()));

            // should we create a thumbnail for a paperspace view?
            if (wantThumbnails && thumbnailConfig.WantSheetThumbnail())
                createThumbnail = true;
            }

        if (createThumbnail)
            {
            this->SetTaskName (ProgressMessage::TASK_CREATING_THUMBNAIL(), view->GetName().c_str());
            this->Progress ();
#if defined(TODO_IMODEL02_THUMBNAILS)
            view->RenderAndSaveThumbnail (size, thumbnailConfig.IsRenderModeOverridden() ? &mode : nullptr, timeout);
#endif
            }

        if (changed)
            this->SaveViewDefinition (*viewController);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::UpdateModelspaceView (ViewControllerP view)
    {
    if (nullptr == view)
        return  false;

    // Update the input view for the modelspace, a viewport entity, or a paperspace's xRef:
    auto spatialView = view->ToSpatialViewP ();
    auto drawingView = view->ToDrawingViewP ();
    auto sheetView = view->ToSheetViewP ();
    if (nullptr == spatialView && nullptr == drawingView && nullptr == sheetView)
        return  false;

    /*---------------------------------------------------------------------------
    We can ignore xref model views in paperspace in the post process, as we have
    handled them through updating xref insert entities.  Adding a new DWG layer in
    mater file's layer table has no impact on xref layer status.  And we don't want
    thumbnail for xref view.
    ---------------------------------------------------------------------------*/
    auto viewName =  view->GetViewDefinition().GetName ();
    if (viewName.StartsWith(LayoutXrefFactory::GetSpatialViewNamePrefix()))
        return  false;

    // either a modelspace view or a viewport entity view as a spatial view
    if (!m_modelspaceXrefs.empty() && spatialView != nullptr)
        {
        // add xref models in modelspace into the modelspace viewport and viewport entity:
        auto& modelSelector = spatialView->GetSpatialViewDefinition().GetModelSelector ();
        modelSelector.GetModelsR().insert (m_modelspaceXrefs.begin(), m_modelspaceXrefs.end());
        modelSelector.Update ();
        }

    if (this->IsUpdating() && m_layersImported > 0)
        {
        // new layers are imported - update spatial/drawing categories from a viewport entity:
        auto handle = this->GetSourceAspects().FindViewportHandle (view->GetViewId());
        DwgDbObjectId   objId;
        if (!handle.IsNull() && (objId = m_dwgdb->GetObjectId(handle)).IsValid())
            {
            DwgDbViewportPtr    viewport(objId, DwgDbOpenMode::ForRead);
            if (viewport.OpenStatus() == DwgDbStatus::Success)
                {
                ViewportFactory factory (*this, *viewport.get());
                if (spatialView != nullptr)
                    factory.UpdateModelspaceCategories (spatialView->GetSpatialViewDefinition().GetCategorySelector().GetCategoriesR());
                else
                    factory.UpdateModelspaceCategories (view->GetViewDefinitionR().GetCategorySelector().GetCategoriesR());

                return  true;
                }
            }
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::UpdatePaperspaceView (ViewControllerP view, DwgDbObjectIdCR viewportId)
    {
    if (nullptr == view)
        return  false;

    auto definitionModel = this->GetOrCreateJobDefinitionModel();
    if (!definitionModel.IsValid())
        return  false;

    // Update a paperspace sheet view - add all drawing categories, except for viewport frozen layers:
    DwgDbObjectIdArray  frozenLayers;
    size_t              numFrozenLayers = 0;

    // check if the viewport has frozen layers:
    DwgDbViewportPtr    layoutViewport (viewportId, DwgDbOpenMode::ForRead);
    if (layoutViewport.OpenStatus() == DwgDbStatus::Success && layoutViewport->GetFrozenLayers(frozenLayers) == DwgDbStatus::Success)
        numFrozenLayers = frozenLayers.size ();

    auto rlinkId = this->GetRepositoryLink (m_dwgdb.get());
    bool isRoot3d = this->GetRootModel().GetModel()->Is3d ();
    bool changed = false;
    auto& categorySelector = view->GetViewDefinitionR().GetCategorySelector ();
    for (ElementIteratorEntry entry : DrawingCategory::MakeIterator(*m_dgndb))
        {
        DgnCategoryId   categoryId = entry.GetId <DgnCategoryId> ();
        bool isCategoryViewed = categorySelector.IsCategoryViewed (categoryId);

        /*---------------------------------------------------------------------------
        Find layer ID from syncInfo, which has categories created from the layer table only:
        when the root model is 2d, find the layer directly using current drawing category
        ID. If the root model is 3d, first find the spatial category from the code value,
        then find the layer ID from the spatial category ID.
        ---------------------------------------------------------------------------*/
        DwgDbObjectId   layerId;
        DgnCategoryId   searchId = isRoot3d ? SpatialCategory::QueryCategoryId(*definitionModel, entry.GetCodeValue()) : entry.GetId <DgnCategoryId>();
        DwgDbHandle     objHandle = m_sourceAspects.FindLayerHandle (searchId, rlinkId);
        if (!objHandle.IsNull() && (layerId = m_dwgdb->GetObjectId(objHandle)).IsValid())
            {
            // 1) check layer's global on/off status and global freeze:
            DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);
            if (layer.OpenStatus() == DwgDbStatus::Success && (layer->IsOff() || layer->IsFrozen()))
                {
                // the layer is globally turned off or frozen, if the category is viewed, drop it:
                if (isCategoryViewed)
                    {
                    categorySelector.DropCategory (categoryId);
                    changed = true;
                    }
                continue;
                }

            // 2) check viewport frozen layer for this view:
            if (numFrozenLayers > 0)
                {
                auto found = std::find_if (frozenLayers.begin(), frozenLayers.end(), [&](DwgDbObjectId id){ return id == layerId; });
                if (found != frozenLayers.end())
                    {
                    // the viewport has this layer frozen - if the category is in this view, drop it:
                    if (isCategoryViewed)
                        {
                        categorySelector.DropCategory (categoryId);
                        changed = true;
                        }
                    continue;
                    }
                }
            }

        // this category should be viewed - if not, add it:
        if (!isCategoryViewed)
            {
            categorySelector.AddCategory (categoryId);
            changed = true;
            }
        }

    return  changed;
    }
