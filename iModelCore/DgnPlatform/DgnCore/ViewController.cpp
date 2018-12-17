/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewController.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <DgnPlatform/DgnGeoCoord.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::FromJson(JsonValueCR val)
    {
    memset(this, 0, sizeof(*this));
    m_constructions = !val[json_noConstruct()].asBool();
    m_dimensions = !val[json_noDim()].asBool();
    m_patterns = !val[json_noPattern()].asBool();
    m_weights = !val[json_noWeight()].asBool();
    m_styles = !val[json_noStyle()].asBool();
    m_transparency = !val[json_noTransp()].asBool();
    m_fill = !val[json_noFill()].asBool();
    m_grid = val[json_grid()].asBool();
    m_acsTriad = val[json_acs()].asBool();
    m_textures = !val[json_noTexture()].asBool();
    m_materials = !val[json_noMaterial()].asBool();
    m_cameraLights = !val[json_noCameraLights()].asBool();
    m_sourceLights = !val[json_noSourceLights()].asBool();
    m_solarLight = !val[json_noSolarLight()].asBool();
    m_visibleEdges = val[json_visEdges()].asBool();
    m_hiddenEdges = val[json_hidEdges()].asBool();
    m_shadows = val[json_shadows()].asBool();
    m_noClipVolume = !val[json_clipVol()].asBool();
    m_monochrome = val[json_monochrome()].asBool();
    m_edgeMask = val[json_edgeMask()].asUInt();
    m_hLineMaterialColors = val[json_hlMatColors()].asBool();
    m_animate = val[json_animate()].asBool();
    m_backgroundMap = val[json_backgroundMap()].asBool();

    // Validate render mode. V8 converter only made sure to set everything above Phong to Smooth...
    uint32_t renderModeValue = val[json_renderMode()].asUInt();

    if (renderModeValue < (uint32_t)RenderMode::HiddenLine)
        m_renderMode = RenderMode::Wireframe;
    else if (renderModeValue > (uint32_t)RenderMode::SolidFill)
        m_renderMode = RenderMode::SmoothShade;
    else
        m_renderMode = RenderMode(renderModeValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ViewFlags::ToJson() const
    {
    Json::Value val;
    if (!m_constructions) val[json_noConstruct()] = true;
    if (!m_dimensions) val[json_noDim()] = true;
    if (!m_patterns) val[json_noPattern()] = true;
    if (!m_weights) val[json_noWeight()] = true;
    if (!m_styles) val[json_noStyle()] = true;
    if (!m_transparency) val[json_noTransp()] = true;
    if (!m_fill) val[json_noFill()] = true;
    if (m_grid) val[json_grid()] = true;
    if (m_acsTriad) val[json_acs()] = true;
    if (!m_textures) val[json_noTexture()] = true;
    if (!m_materials) val[json_noMaterial()] = true;
    if (!m_cameraLights) val[json_noCameraLights()] = true;
    if (!m_sourceLights) val[json_noSourceLights()] = true;
    if (!m_solarLight) val[json_noSolarLight()] = true;
    if (m_visibleEdges) val[json_visEdges()] = true;
    if (m_hiddenEdges) val[json_hidEdges()] = true;
    if (m_shadows) val[json_shadows()] = true;
    if (!m_noClipVolume) val[json_clipVol()] = true;
    if (m_hLineMaterialColors) val[json_hlMatColors()] = true;
    if (m_monochrome) val[json_monochrome()] = true;
    if (m_backgroundMap) val[json_backgroundMap()] = true;
    if (m_animate) val[json_animate()] = true;
    if (m_edgeMask!=0) val[json_edgeMask()] = m_edgeMask;

    val[json_renderMode()] = (uint8_t) m_renderMode;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFlagsOverrides::ViewFlagsOverrides(ViewFlags base) : m_present(0xffffffff), m_values(base)
    {
    // NB: A handful of flags (grid, acs) cannot be overridden on a per-branch basis...ignore.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlagsOverrides::Apply(ViewFlags& base) const
    {
    if (!AnyOverridden())
        return;

    if (IsPresent(kDimensions)) base.SetShowDimensions(m_values.ShowDimensions());
    if (IsPresent(kPatterns)) base.SetShowPatterns(m_values.ShowPatterns());
    if (IsPresent(kWeights)) base.SetShowWeights(m_values.ShowWeights());
    if (IsPresent(kStyles)) base.SetShowStyles(m_values.ShowStyles());
    if (IsPresent(kTransparency)) base.SetShowTransparency(m_values.ShowTransparency());
    if (IsPresent(kFill)) base.SetShowFill(m_values.ShowFill());
    if (IsPresent(kTextures)) base.SetShowTextures(m_values.ShowTextures());
    if (IsPresent(kMaterials)) base.SetShowMaterials(m_values.ShowMaterials());
    if (IsPresent(kSolarLight)) base.SetShowSolarLight(m_values.ShowSolarLight());
    if (IsPresent(kCameraLights)) base.SetShowCameraLights(m_values.ShowCameraLights());
    if (IsPresent(kSourceLights)) base.SetShowSourceLights(m_values.ShowSourceLights());
    if (IsPresent(kVisibleEdges)) base.SetShowVisibleEdges(m_values.ShowVisibleEdges());
    if (IsPresent(kHiddenEdges)) base.SetShowHiddenEdges(m_values.ShowHiddenEdges());
    if (IsPresent(kShadows)) base.SetShowShadows(m_values.ShowShadows());
    if (IsPresent(kClipVolume)) base.SetShowClipVolume(m_values.ShowClipVolume());
    if (IsPresent(kBackgroundMap)) base.SetShowClipVolume(m_values.ShowBackgroundMap());
    if (IsPresent(kConstructions)) base.SetShowConstructions(m_values.ShowConstructions());
    if (IsPresent(kMonochrome)) base.SetMonochrome(m_values.IsMonochrome());
    if (IsPresent(kGeometryMap)) base.SetIgnoreGeometryMap(m_values.IgnoreGeometryMap());
    if (IsPresent(kHlineMaterialColors)) base.SetUseHlineMaterialColors(m_values.UseHlineMaterialColors());
    if (IsPresent(kEdgeMask)) base.SetEdgeMask(m_values.GetEdgeMask());
    if (IsPresent(kRenderMode)) base.SetRenderMode(m_values.GetRenderMode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ChangeCategoryDisplay(DgnCategoryId categoryId, bool onOff, bool andSubCategories)
    {
    GetViewDefinitionR().GetCategorySelector().ChangeCategoryDisplay(categoryId, onOff);
    if (andSubCategories)
        ToggleAllSubCategories(categoryId, onOff);

    SetFeatureOverridesDirty();
    _OnCategoryChange(onOff);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ToggleAllSubCategories(DgnCategoryId catId, bool onOff)
    {
    auto cat = DgnCategory::Get(GetDgnDb(), catId);
    if (cat.IsValid())
        for (ElementIteratorEntry entry : cat->MakeSubCategoryIterator())
            ChangeSubCategoryDisplay(entry.GetId<DgnSubCategoryId>(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ChangeSubCategoryDisplay(DgnSubCategoryId id, bool enable)
    {
    // Preserve existing overrides (currently, there will be none, but Revit may produce some later...)
    auto ovr = GetViewDefinitionR().GetDisplayStyle().GetSubCategoryOverride(id);
    ovr.SetInvisible(!enable);
    OverrideSubCategory(id, ovr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::SetViewedCategories(DgnCategoryIdSet const& categories, bool enableAllSubCategories)
    {
    GetViewDefinitionR().GetCategorySelector().SetCategories(categories);
    SetFeatureOverridesDirty();
    if (enableAllSubCategories)
        {
        for (auto catId : categories)
            ToggleAllSubCategories(catId, true);
        }

    _OnCategoryChange(false); // boolean indicates a single category was enabled; false means a category was disabled or multiple changes were made
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::ViewController(ViewDefinitionCR def, SkipClone skipClone)
    : m_dgndb(def.GetDgnDb()), m_definition((skipClone == SkipClone::Yes) ? const_cast<ViewDefinitionP>(&def) : def.MakeCopy<ViewDefinition>()),
    m_selectionSetDirty(true)
    {
    DgnElementId acsId = def.GetAuxiliaryCoordinateSystemId();

    if (acsId.IsValid())
        m_auxCoordSys = m_dgndb.Elements().Get<AuxCoordSystem>(acsId);

    if (!m_auxCoordSys.IsValid())
        m_auxCoordSys = AuxCoordSystem::CreateNew(def);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_LoadState()
    {
    m_activeVolume = m_definition->GetViewClip();
    m_definition->GetGridSettings(m_gridOrientation, m_gridSpacing, m_gridsPerRef);

    for (auto const& appdata : m_appData) // allow all appdata to restore from settings, if necessary
        appdata.second->_Load(*m_definition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_StoreState()
    {
    m_definition->SetViewClip(m_activeVolume);
    m_definition->SetGridSettings(m_gridOrientation, m_gridSpacing, m_gridsPerRef);

    for (auto const& appdata : m_appData)
        appdata.second->_Save(*m_definition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::InvalidateScene()
    {
    BeMutexHolder lock(m_mutex);
    m_readyScene = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/06
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr ViewController::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    return source.Stroke(context, pixelSize);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewController::_IsPointAdjustmentRequired() const {return Is3d();}
bool ViewController::_IsSnapAdjustmentRequired(bool snapLockEnabled) const {return snapLockEnabled && Is3d();}
bool ViewController::_IsContextRotationRequired(bool contextLockEnabled) const {return contextLockEnabled;}

static bool equalOne(double r1) {return DoubleOps::AlmostEqual(r1, 1.0);}
static bool equalMinusOne(double r1) {return DoubleOps::AlmostEqual(r1, -1.0);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   03/89
+---------------+---------------+---------------+---------------+---------------+------*/
StandardView ViewController::IsStandardViewRotation(RotMatrixCR rMatrix, bool check3d)
    {
    if (check3d)
        {
        // If a matrix is known apriori to be a pure rotation ....
        //   a) A one or minus one implies the remainder of the row and column are zero.
        //   b) Once ones are found, the third row and column are known by cross product rules.
        // Hence just two one or minus one entries fully identifies primary flat views.
        // Dot products with two vectors from known iso views is also complete.
        if (equalOne(rMatrix.form3d[0][0]))
            {
            if (equalOne(rMatrix.form3d[1][1]))
                return StandardView::Top;
            if (equalMinusOne(rMatrix.form3d[1][1]))
                return StandardView::Bottom;
            if (equalOne(rMatrix.form3d[1][2]))
                return StandardView::Front;
            }
        else if (equalOne(rMatrix.form3d[1][2]))
            {
            if (equalOne(rMatrix.form3d[0][1]))
                return StandardView::Right;
            if (equalMinusOne(rMatrix.form3d[0][1]))
                return StandardView::Left;
            if (equalMinusOne(rMatrix.form3d[0][0]))
                return StandardView::Back;
            }
        else                    /* Check For (left) IsoMetric */
            {
            RotMatrix  isoMatrix;
            bsiRotMatrix_getStandardRotation(&isoMatrix, static_cast<int>(StandardView::Iso));

            if (equalOne(((DVec3d*)isoMatrix.form3d[0])->DotProduct(*((DVec3d*)rMatrix.form3d[0]))) &&
                equalOne(((DVec3d*)isoMatrix.form3d[1])->DotProduct(*((DVec3d*)rMatrix.form3d[1]))))
                return StandardView::Iso;

            bsiRotMatrix_getStandardRotation(&isoMatrix, static_cast<int>(StandardView::RightIso));
            if (equalOne(((DVec3d*)isoMatrix.form3d[0])->DotProduct(*((DVec3d*)rMatrix.form3d[0]))) &&
                equalOne(((DVec3d*)isoMatrix.form3d[1])->DotProduct(*((DVec3d*)rMatrix.form3d[1]))))
                return StandardView::RightIso;
            }
        }
    else
        {
        if (equalOne(rMatrix.form3d[0][0]) && equalOne(rMatrix.form3d[1][1]))
            return StandardView::Top;
        }

    return StandardView::NotStandard;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ViewController::GetStandardViewName(StandardView viewId)
    {
    if (viewId < StandardView::Top || viewId > StandardView::RightIso)
        return "";

    L10N::StringId names[]={
        DgnCoreL10N::VIEWTITLE_MessageID_Top(),
        DgnCoreL10N::VIEWTITLE_MessageID_Bottom(),
        DgnCoreL10N::VIEWTITLE_MessageID_Left(),
        DgnCoreL10N::VIEWTITLE_MessageID_Right(),
        DgnCoreL10N::VIEWTITLE_MessageID_Front(),
        DgnCoreL10N::VIEWTITLE_MessageID_Back(),
        DgnCoreL10N::VIEWTITLE_MessageID_Iso(),
        DgnCoreL10N::VIEWTITLE_MessageID_RightIso(),
        };

    return DgnCoreL10N::GetString(*(static_cast<int>(viewId) - 1 + names));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewController::GetStandardViewByName(RotMatrix* rotP, StandardView* standardIdP, Utf8CP viewName)
    {
    for (int i = static_cast<int>(StandardView::Top); i <= (int) StandardView::RightIso; ++i)
        {
        Utf8String tname = GetStandardViewName((StandardView) i);
        if (tname.empty())
            return ERROR;

        if (tname == viewName)
            {
            if (nullptr != rotP)
                bsiRotMatrix_getStandardRotation(rotP, i);

            if (nullptr != standardIdP)
                *standardIdP =(StandardView) i;

            return SUCCESS;
            }
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewController::SetupFromFrustum(Frustum const& inFrustum)
    {
    Frustum frustum=inFrustum;
    DgnViewport::FixFrustumOrder(frustum);

    return m_definition->_SetupFromFrustum(frustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewController3d::TurnCameraOn(Angle lensAngle)
    {
    auto& cameraDef = GetViewDefinition3dR();
    if (cameraDef.IsCameraOn())
        return cameraDef.LookAtUsingLensAngle(cameraDef.GetEyePoint(), cameraDef.GetTargetPoint(), cameraDef.GetYVector(), lensAngle);

    if (nullptr == m_vp)
        return ViewportStatus::NotAttached;

    // We need to figure out a new camera    target. To do that, we need to know where the geometry is in the view.
    // We use the depth of the center of the view for that.
    double low, high;
    // m_vp->DetermineVisibleDepthNpcRange(low, high);
    low = 1.0;
    high = 0.0;
    double middle = low + ((high - low) / 2.0);

    DPoint3d corners[4];
    corners[0].Init(0.0, 0.0, middle); // lower left, at target depth
    corners[1].Init(1.0, 1.0, middle); // upper right at target depth
    corners[2].Init(0.0, 0.0, high); // lower left, at closest npc
    corners[3].Init(1.0, 1.0, high); // upper right at closest
    m_vp->NpcToWorld(corners, corners, 4);

    DPoint3d eye = DPoint3d::FromInterpolate(corners[2], 0.5, corners[3]); // middle of closest plane
    DPoint3d target = DPoint3d::FromInterpolate(corners[0], 0.5, corners[1]); // middle of halfway plane
    double backDist  = eye.Distance(target) * 2.0;
    double frontDist = cameraDef.MinimumFrontDistance();
    return cameraDef.LookAtUsingLensAngle(eye, target, cameraDef.GetYVector(), lensAngle, &frontDist, &backDist);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     12/13
//---------------------------------------------------------------------------------------
static bool convertToWorldPoint(DPoint3dR worldPoint, GeoLocationEventStatus& status, DgnGeoLocation const& units, GeoPointCR location)
    {
    if (SUCCESS != units.XyzFromLatLong(worldPoint, location))
        {
        BeAssert(false);
        status = GeoLocationEventStatus::EventIgnored;
        return false;
        }

    status = GeoLocationEventStatus::EventHandled;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool SpatialViewController::OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR location)
    {
    DPoint3d worldPoint;
    if (!convertToWorldPoint(worldPoint, status, GetDgnDb().GeoLocation(), location))
        return false;

    auto& camera = *GetViewDefinitionR().ToView3dP();
    if (!camera.IsCameraOn())
        {
        // If there's no perspective, just center the current location in the view.
        RotMatrix viewInverse;
        viewInverse.InverseOf(camera.GetRotation());

        DPoint3d delta = camera.GetExtents();
        delta.Scale(0.5);
        viewInverse.Multiply(delta);

        worldPoint.DifferenceOf(worldPoint, delta);
        camera.SetOrigin(worldPoint);
        return true;
        }

    worldPoint.z = camera.GetEyePoint().z;
    DPoint3d targetPoint = camera.GetTargetPoint();
    targetPoint.z = worldPoint.z;
    DVec3d newViewZ;
    newViewZ.DifferenceOf(targetPoint, worldPoint);
    newViewZ.Normalize();
    targetPoint.SumOf(worldPoint, newViewZ, camera.GetFocusDistance());
    camera.LookAt(worldPoint, targetPoint, DVec3d::From(0.0, 0.0, 1.0));

    return true;
    }

static DVec3d s_defaultForward, s_defaultUp;
static UiOrientation s_lastUi;

//---------------------------------------------------------------------------------------
// Gyro vector convention:
// gyrospace X,Y,Z are (respectively) DOWN, RIGHT, and TOWARDS THE EYE (when the tablet is in
// landscape mode with the home button to the left).  Y is always from home to the opposite end
// of the tablet.  Z is always towards the eye.
// (gyrospace vectors are in the absolute system of the device.  But it is not important what that is --
// just so they are to the same space and their row versus column usage is clarified by the gyroByRow parameter.
// @bsimethod                                                   Earlin.Lutz     12/2015
//---------------------------------------------------------------------------------------
static void applyGyroChangeToViewingVectors(UiOrientation ui, RotMatrixCR gyro0, RotMatrixCR gyro1, DVec3dCR forward0, DVec3dCR up0, DVec3dR forward1, DVec3dR up1)
    {
    RotMatrix gyroToBSIColumnShuffler;

    if (ui == UiOrientation::LandscapeLeft)
        {
        gyroToBSIColumnShuffler = RotMatrix::FromRowValues
            (
            0,-1,0,         //  negative X becomes Y
            1,0,0,          //  Y becomes X
            0,0,1           //  Z remains Z
            );
        }
    else if (ui == UiOrientation::LandscapeRight)
        {
        gyroToBSIColumnShuffler = RotMatrix::FromRowValues
            (
            0,1,0,          //  X becomes Y
            -1,0,0,         //  negative Y becomes X
            0,0,1           //  Z remains Z
            );
        }
    else if (ui == UiOrientation::Portrait)
        {
        gyroToBSIColumnShuffler = RotMatrix::FromRowValues
            (
            1,0,0,
            0,1,0,
            0,0,1
            );
        }
    else
        {
        BeAssert(ui == UiOrientation::PortraitUpsideDown);
        gyroToBSIColumnShuffler = RotMatrix::FromRowValues
            (
            -1,0,0,
            0,-1,0,
            0,0,1
            );
        }

    RotMatrix H0, H1;
    H0.InitProduct(gyro0, gyroToBSIColumnShuffler);
    H1.InitProduct(gyro1, gyroToBSIColumnShuffler);
    RotMatrix H1T;
    H1T.TransposeOf(H1);
    RotMatrix screenToScreenMotion;
    screenToScreenMotion.InitProduct(H1T, H0);
    DVec3d right0 = DVec3d::FromCrossProduct(up0, forward0);
    RotMatrix screenToModel = RotMatrix::FromColumnVectors(right0, up0, forward0);
    RotMatrix modelToScreen;
    modelToScreen.TransposeOf(screenToModel);
    RotMatrix modelToModel;

    screenToScreenMotion.Transpose();
    modelToModel.InitProduct(screenToModel, screenToScreenMotion, modelToScreen);
    modelToModel.Multiply(forward1, forward0);
    modelToModel.Multiply(up1, up0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool SpatialViewController::ViewVectorsFromOrientation(DVec3dR forward, DVec3dR up, RotMatrixCR orientation, OrientationMode mode, UiOrientation ui)
    {
    double azimuthCorrection = 0.0;
    DVec3d currForward = m_definition->GetZVector();

    orientation.GetColumn(forward, 2);
    switch (mode)
        {
        case OrientationMode::CompassHeading:
            {
            DgnGCS* dgnGcs = GetDgnDb().GeoLocation().GetDgnGCS();
            double azimuth = (dgnGcs != nullptr) ? dgnGcs->GetAzimuth() : 0.0;
            azimuthCorrection = msGeomConst_radiansPerDegree *(90.0 + azimuth);
            forward.RotateXY(azimuthCorrection);
            break;
            }
        case OrientationMode::IgnoreHeading:
            forward.x = currForward.x;
            forward.y = currForward.y;
            break;
        case OrientationMode::RelativeHeading:
            {
            //  orientation is arranged in columns.  The axis from the home button to other end of tablet is Y.  Z is out of the screen.  X is Y cross Z.
            //  Therefore, when the UiOrientation is Portrait, orientation Y is up and orientation X points to the right.
            applyGyroChangeToViewingVectors(ui, m_defaultDeviceOrientation, orientation, s_defaultForward, s_defaultUp, forward, up);
            break;
            }
        }
    forward.Normalize();

    // low pass filter
    if (fabs(currForward.AngleTo(forward)) < 0.025)
        return false;

    // Since roll isn't desired for any of the standard orientation modes, just ignore the device's real up vector
    // and compute a normalized one.
    up = forward;
    up.z += 0.1;
    up.Normalize();

    DVec3d right;
    right.NormalizedCrossProduct(up, forward);
    up.NormalizedCrossProduct(forward, right);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool SpatialViewController::OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui, uint32_t nEventsSinceEnabled)
    {
    auto& viewDef = GetSpatialViewDefinition();
    if (!m_defaultDeviceOrientationValid || s_lastUi != ui)
        {
        if (nEventsSinceEnabled < 2)
            {
            // Hack to make this work properly. The first rotation received from CM seems
            // to always be the reference frame - using that at best causes the camera to skip for a frame,
            // and at worst causes it to be permanently wrong (RelativeHeading).  Someone should remove and clean
            // up alongside s_defaultForward/s_defaultUp.
            return false;
            }
        m_defaultDeviceOrientation = matrix;
        m_defaultDeviceOrientationValid = true;
        s_defaultUp = viewDef.GetYVector();
        s_defaultForward = viewDef.GetZVector();
        s_lastUi = ui;
        }

    DVec3d forward, up;
    if (!ViewVectorsFromOrientation(forward, up, matrix, mode, ui))
        return false;

    if (!viewDef.IsCameraOn())
        {
        // No camera, have to manually define origin, etc.
        RotMatrix viewMatrix = viewDef.GetRotation();
        RotMatrix viewInverse;
        viewInverse.InverseOf(viewMatrix);

        DVec3d delta = viewDef.GetExtents();
        delta.Scale(0.5);
        DPoint3d worldDelta = delta;
        viewInverse.Multiply(worldDelta);

        // This is the point we want to rotate about.
        DPoint3d eyePoint;
        eyePoint.SumOf(viewDef.GetOrigin(), worldDelta);

        DVec3d xVector;
        xVector.CrossProduct(forward, up);
        viewMatrix.SetRow(xVector, 0);
        viewMatrix.SetRow(up, 1);
        viewMatrix.SetRow(forward, 2);
        viewDef.SetRotation(viewMatrix);

        // Now that we have the new rotation, we can work backward from the eye point to get the new origin.
        viewInverse.InverseOf(viewMatrix);
        worldDelta = delta;
        viewInverse.Multiply(worldDelta);
        DPoint3d newOrigin;
        newOrigin.DifferenceOf(eyePoint, worldDelta);
        viewDef.SetOrigin(newOrigin);
        return true;
        }


    DPoint3d eyePoint = viewDef.GetEyePoint(), newTarget;
    newTarget.SumOf(eyePoint, forward, -viewDef.GetFocusDistance());
    viewDef.LookAt(eyePoint, newTarget, up);

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_GetGridSpacing(DPoint2dR spacing, uint32_t& gridsPerRef) const
    {
    gridsPerRef = m_gridsPerRef;
    spacing = m_gridSpacing;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::PointToStandardGrid(DPoint3dR point, DPoint3dCR gridOrigin, RotMatrixCR gridOrientation, DPoint2dCR roundingDistance, bool isoGrid) const
    {
    // ###TODO: Remove me.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::PointToGrid(DPoint3dR point) const
    {
    // ###TODO: Remove me.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::CloseMe ViewController2d::_OnModelsDeleted(bset<DgnModelId> const& deletedIds, DgnDbR db)
    {
    if (&GetDgnDb() != &db)
        return CloseMe::No;

    // if the base model is deleted, close the view
    for (auto const& deletedId : deletedIds)
        {
        if (GetViewedModelId() == deletedId)
            return CloseMe::Yes;
        }

    return CloseMe::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController2d::_DrawView(ViewContextR context)
    {
    auto model = GetViewedModel();
    if (nullptr == model)
        return;

    context.VisitDgnModel(*model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::AddAppData(AppData::Key const& key, AppData* obj) const
    {
    auto entry = m_appData.Insert(&key, obj);
    if (entry.second)
        return;

    entry.first->second = obj;
    obj->_Load(*m_definition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TemplateViewController3d::_DrawView(ViewContextR context)
    {
    GeometricModelP model = GetViewedModel();
    if (nullptr != model)
        context.VisitDgnModel(*model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TemplateViewController3d::SetViewedModel(DgnModelId viewedModelId)
    {
    if (GetViewedModelId() == viewedModelId)
        return DgnDbStatus::Success;

    GetTemplateViewDefinition3dR().SetViewedModel(viewedModelId);
    GeometricModel3dP model = GetViewedModel();
    if (!model || !model->IsTemplate())
        {
        GetTemplateViewDefinition3dR().SetViewedModel(DgnModelId());
        return DgnDbStatus::WrongModel;
        }

    GetViewDefinitionR().LookAtVolume(model->QueryElementsRange());
    return DgnDbStatus::Success;
    }

