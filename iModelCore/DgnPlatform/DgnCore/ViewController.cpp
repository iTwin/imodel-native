/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewController.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/ElementTileTree.h>

namespace StyleJson
{
    static Utf8CP str_Acs()            {return "acs";}
    static Utf8CP str_Grid()           {return "grid";}
    static Utf8CP str_HiddenEdges()    {return "hidEdges";}
    static Utf8CP str_ClipVolume()     {return "clipVol";}
    static Utf8CP str_NoConstruction() {return "noConstruct";}
    static Utf8CP str_NoDimension()    {return "noDim";}
    static Utf8CP str_NoFill()         {return "noFill";}
    static Utf8CP str_NoLighting()     {return "noLighting";}
    static Utf8CP str_NoMaterial()     {return "noMaterial";}
    static Utf8CP str_NoPattern()      {return "noPattern";}
    static Utf8CP str_NoSceneLight()   {return "noSceneLight";}
    static Utf8CP str_NoStyle()        {return "noStyle";}
    static Utf8CP str_NoText()         {return "noText";}
    static Utf8CP str_NoTexture()      {return "noTexture";}
    static Utf8CP str_NoTransparency() {return "noTransp";}
    static Utf8CP str_NoWeight()       {return "noWeight";}
    static Utf8CP str_RenderMode()     {return "renderMode";}
    static Utf8CP str_Shadows()        {return "shadows";}
    static Utf8CP str_VisibleEdges()   {return "visEdges";}
};

using namespace StyleJson;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::FromJson(JsonValueCR val)
    {
    memset(this, 0, sizeof(*this));
    m_constructions = !val[str_NoConstruction()].asBool();
    m_text = !val[str_NoText()].asBool();
    m_dimensions = !val[str_NoDimension()].asBool();
    m_patterns = !val[str_NoPattern()].asBool();
    m_weights = !val[str_NoWeight()].asBool();
    m_styles = !val[str_NoStyle()].asBool();
    m_transparency = !val[str_NoTransparency()].asBool();
    m_fill = !val[str_NoFill()].asBool();
    m_grid = val[str_Grid()].asBool();
    m_acsTriad = val[str_Acs()].asBool();
    m_textures = !val[str_NoTexture()].asBool();
    m_materials = !val[str_NoMaterial()].asBool();
    m_sceneLights = !val[str_NoSceneLight()].asBool();
    m_visibleEdges = val[str_VisibleEdges()].asBool();
    m_hiddenEdges = val[str_HiddenEdges()].asBool();
    m_shadows = val[str_Shadows()].asBool();
    m_noClipVolume = !val[str_ClipVolume()].asBool();
    m_ignoreLighting = val[str_NoLighting()].asBool();

    // Validate render mode. V8 converter only made sure to set everything above Phong to Smooth...
    uint32_t renderModeValue = val[str_RenderMode()].asUInt();

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
    if (!m_constructions) val[str_NoConstruction()] = true;
    if (!m_text) val[str_NoText()] = true;
    if (!m_dimensions) val[str_NoDimension()] = true;
    if (!m_patterns) val[str_NoPattern()] = true;
    if (!m_weights) val[str_NoWeight()] = true;
    if (!m_styles) val[str_NoStyle()] = true;
    if (!m_transparency) val[str_NoTransparency()] = true;
    if (!m_fill) val[str_NoFill()] = true;
    if (m_grid) val[str_Grid()] = true;
    if (m_acsTriad) val[str_Acs()] = true;
    if (!m_textures) val[str_NoTexture()] = true;
    if (!m_materials) val[str_NoMaterial()] = true;
    if (!m_sceneLights) val[str_NoSceneLight()] = true;
    if (m_visibleEdges) val[str_VisibleEdges()] = true;
    if (m_hiddenEdges) val[str_HiddenEdges()] = true;
    if (m_shadows) val[str_Shadows()] = true;
    if (!m_noClipVolume) val[str_ClipVolume()] = true;
    if (m_ignoreLighting) val[str_NoLighting()] = true;
    val[str_RenderMode()] = (uint8_t) m_renderMode;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ChangeCategoryDisplay(DgnCategoryId categoryId, bool onOff)
    {
    GetViewDefinition().GetCategorySelector().ChangeCategoryDisplay(categoryId, onOff);
    _OnCategoryChange(onOff);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::ViewController(ViewDefinitionCR def) : m_dgndb(def.GetDgnDb()), m_definition(def.MakeCopy<ViewDefinition>())
    {
    m_defaultDeviceOrientation.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_LoadState()
    {
    m_activeVolume = m_definition->GetViewClip();

    for (auto const& appdata : m_appData) // allow all appdata to restore from settings, if necessary
        appdata.second->_Load(*m_definition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_StoreState()
    {
    m_definition->SetViewClip(m_activeVolume);
    for (auto const& appdata : m_appData)
        appdata.second->_Save(*m_definition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewController::SaveAs(Utf8CP newName)
    {
    DgnElement::CreateParams params(GetDgnDb(), m_definition->GetModelId(), m_definition->GetElementClassId(), ViewDefinition::CreateCode(GetDgnDb(), newName));

    ViewDefinitionPtr newView = dynamic_cast<ViewDefinitionP>(m_definition->Clone(nullptr, &params).get());
    BeAssert(newView.IsValid());
    if (newView.IsNull())
        return DgnDbStatus::BadElement;

    m_definition = newView;

    DgnDbStatus stat;
    m_definition->Insert(&stat);
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewController::SaveTo(Utf8CP newName, DgnViewId& newId)
    {
    auto wasDef = m_definition;
    auto rc = SaveAs(newName);
    m_definition = wasDef;
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::InvalidateScene()
    {
    RequestAbort(false); 
    BeMutexHolder lock(m_mutex);
    m_readyScene = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* return the extents of the target model, if there is one.
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d ViewController2d::_GetViewedExtents(DgnViewportCR vp) const
    {
    GeometricModelP target = GetViewedModel();
    if (target && target->GetRangeIndex())
        return AxisAlignedBox3d(target->GetRangeIndex()->GetExtents().ToRange3d());

    return AxisAlignedBox3d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/06
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr ViewController::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    return source.Stroke(context, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr ViewController::_StrokeHit(ViewContextR context, GeometrySourceCR source, HitDetailCR hit)
    {
    return source.StrokeHit(context, hit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewController::_IsPointAdjustmentRequired(DgnViewportR vp) const {return vp.Is3dView();}
bool ViewController::_IsSnapAdjustmentRequired(DgnViewportR vp, bool snapLockEnabled) const {return snapLockEnabled && vp.Is3dView();}
bool ViewController::_IsContextRotationRequired(DgnViewportR vp, bool contextLockEnabled) const {return contextLockEnabled;}

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
* Search the (8) standard view matrices for one that is close to given matrix.
* @bsimethod                                                    EarlinLutz      05/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool findNearbyStandardViewMatrix(RotMatrixR rMatrix)
    {
    static double const s_viewMatrixTolerance = 1.0e-7;
    RotMatrix   test;

    // Standard views are numbered from 1 ....
    for (int i = 1; bsiRotMatrix_getStandardRotation(&test, i); i++)
        {
        double a = test.MaxDiff(rMatrix);
        if (a < s_viewMatrixTolerance)
            {
            rMatrix = test;
            return true;
            }
        }

    return false;
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
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition::_SetupFromFrustum(Frustum const& frustum)
    {
    DPoint3dCP frustPts = frustum.GetPts();
    DPoint3d viewOrg = frustPts[NPC_000];

    // frustumX, frustumY, frustumZ are vectors along edges of the frustum. They are NOT unit vectors.
    // X and Y should be perpendicular, and Z should be right handed.
    DVec3d frustumX, frustumY, frustumZ;
    frustumX.DifferenceOf(frustPts[NPC_100], viewOrg);
    frustumY.DifferenceOf(frustPts[NPC_010], viewOrg);
    frustumZ.DifferenceOf(frustPts[NPC_001], viewOrg);

    RotMatrix   frustMatrix;
    frustMatrix.InitFromColumnVectors(frustumX, frustumY, frustumZ);
    if (!frustMatrix.SquareAndNormalizeColumns(frustMatrix, 0, 1))
        return ViewportStatus::InvalidWindow;

    findNearbyStandardViewMatrix(frustMatrix);

    DVec3d xDir, yDir, zDir;
    frustMatrix.GetColumns(xDir, yDir, zDir);

    // set up view Rotation matrix as rows of frustum matrix.
    RotMatrix viewRot;
    viewRot.InverseOf(frustMatrix);

    // Left handed frustum?
    double zSize = zDir.DotProduct(frustumZ);
    if (zSize < 0.0)
        return ViewportStatus::InvalidWindow;

    DPoint3d viewDiagRoot;
    viewDiagRoot.SumOf(xDir, xDir.DotProduct(frustumX), yDir, yDir.DotProduct(frustumY));  // vectors on the back plane
    viewDiagRoot.SumOf(viewDiagRoot, zDir, zSize);       // add in z vector perpendicular to x,y

    // use center of frustum and view diagonal for origin. Original frustum may not have been orgthogonal
    viewOrg.SumOf(frustum.GetCenter(), viewDiagRoot, -0.5);

    // delta is in view coordinates
    DVec3d viewDelta;
    viewRot.Multiply(viewDelta, viewDiagRoot);

    ViewportStatus validSize = ValidateViewDelta(viewDelta, false);
    if (validSize != ViewportStatus::Success)
        return validSize;

    SetOrigin(viewOrg);
    SetExtents(viewDelta);
    SetRotation(viewRot);
    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewDefinition::_SetupFromFrustum(Frustum const& frustum)
    {
    auto stat = T_Super::_SetupFromFrustum(frustum);
    if (ViewportStatus::Success != stat)
        return stat;

    DPoint3dCP frustPts = frustum.GetPts();

    // use comparison of back, front plane X sizes to indicate camera or flat view ...
    double xBack  = frustPts[NPC_000].Distance(frustPts[NPC_100]);
    double xFront = frustPts[NPC_001].Distance(frustPts[NPC_101]);

    static double const s_flatViewFractionTolerance = 1.0e-6;
    if (xFront > xBack *(1.0 + s_flatViewFractionTolerance))
        return ViewportStatus::InvalidWindow;

    // see if the frustum is tapered, and if so, set up camera eyepoint and adjust viewOrg and delta.
    double compression = xFront / xBack;
#if defined (BENTLEY_CHANGE)
    if (compression >=(1.0 - s_flatViewFractionTolerance))
        {
        SetCameraOn(false);
        return ViewportStatus::Success;
        }
#endif

    DPoint3d viewOrg     = frustPts[NPC_000];
    DVec3d viewDelta     = GetExtents();
    DVec3d zDir          = GetZVector();
    DVec3d frustumZ      = DVec3d::FromStartEnd(viewOrg, frustPts[NPC_001]);
    DVec3d frustOrgToEye = DVec3d::FromScale(frustumZ, 1.0 /(1.0 - compression));
    DPoint3d eyePoint    = DPoint3d::FromSumOf(viewOrg, frustOrgToEye);

    double backDistance  = frustOrgToEye.DotProduct(zDir);         // distance from eye to back plane of frustum
    double focusDistance = backDistance -(viewDelta.z / 2.0);
    double focalFraction = focusDistance / backDistance;           // ratio of focus plane distance to back plane distance

    viewOrg.SumOf(eyePoint, frustOrgToEye, -focalFraction);        // project point along org-to-eye vector onto focus plane
    viewOrg.SumOf(viewOrg, zDir, focusDistance - backDistance);    // now project that point onto back plane

    viewDelta.x *= focalFraction;                                  // adjust view delta for x and y so they are also at focus plane
    viewDelta.y *= focalFraction;

    SetEyePoint(eyePoint);
    SetFocusDistance(focusDistance);
    SetOrigin(viewOrg);
    SetExtents(viewDelta);
    SetLensAngle(CalcLensAngle());
    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::LookAtVolume(DRange3dCR volume, double const* aspect, MarginPercent const* margin, bool expandClippingPlanes)
    {
    DPoint3d rangebox[8];
    volume.Get8Corners(rangebox);
    GetRotation().Multiply(rangebox, rangebox, 8);

    DRange3d viewAlignedVolume;
    viewAlignedVolume.InitFrom(rangebox, 8);

    return LookAtViewAlignedVolume(viewAlignedVolume, aspect, margin, expandClippingPlanes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::LookAtViewAlignedVolume(DRange3dCR volume, double const* aspect, MarginPercent const* margin, bool expandClippingPlanes)
    {
    DPoint3d    oldDelta = GetExtents();
    DPoint3d    oldOrg   = GetOrigin();
    RotMatrix   viewRot  = GetRotation();

    DPoint3d  newOrigin = volume.low;
    DVec3d    newDelta;
    newDelta.DifferenceOf(volume.high, volume.low);

    double minimumDepth = DgnUnits::OneMillimeter();
    if (newDelta.z < minimumDepth)
        {
        newOrigin.z -=(minimumDepth - newDelta.z)/2.0;
        newDelta.z = minimumDepth;
        }

    auto physView   = _ToSpatialView();
    auto cameraView = ToCameraViewP();
    DPoint3d origNewDelta = newDelta;

    if (nullptr != cameraView)
        {
        // In a camera view, the only way to guarantee we can see the entire volume is to set delta at the front plane, not focus plane.
        // That generally causes the view to be too large (objects in it are too small), since we can't tell whether the objects are at
        // the front or back of the view. For this reason, don't attempt to add any "margin" to camera views.
        }
    else if (nullptr != margin)
        {
        // compute how much space we'll need for both of X and Y margins in root coordinates
        double wPercent = margin->Left() + margin->Right();
        double hPercent = margin->Top()  + margin->Bottom();

        double marginHoriz = wPercent/(1-wPercent) * newDelta.x;
        double marginVert  = hPercent/(1-hPercent) * newDelta.y;

        // compute left and bottom margins in root coordinates
        double marginLeft   = margin->Left()/(1-wPercent) *   newDelta.x;
        double marginBottom = margin->Bottom()/(1-hPercent) * newDelta.y;

        // add the margins to the range
        newOrigin.x -= marginLeft;
        newOrigin.y -= marginBottom;
        newDelta.x  += marginHoriz;
        newDelta.y  += marginVert;

        // don't fix the origin due to changes in delta here
        origNewDelta = newDelta;
        }
    else
        {
        newDelta.Scale(1.04); // default "dilation"
        }

    if (physView /* && Allow3dManipulations() */ && (nullptr == cameraView))
        {
        // make sure that the zDelta is large enough so that entire model will be visible from any rotation
        double diag = newDelta.MagnitudeXY ();
        if (diag > newDelta.z)
            newDelta.z = diag;
        }

    ValidateViewDelta(newDelta, true);

    SetExtents(newDelta);
    if (aspect)
        _AdjustAspectRatio(*aspect, true);

    newDelta = GetExtents();

    newOrigin.x -=(newDelta.x - origNewDelta.x) / 2.0;
    newOrigin.y -=(newDelta.y - origNewDelta.y) / 2.0;
    newOrigin.z -=(newDelta.z - origNewDelta.z) / 2.0;

    // if they don't want the clipping planes to change, set them back to where they were
    if (nullptr != physView && !expandClippingPlanes)
        {
        viewRot.Multiply(oldOrg);
        newOrigin.z = oldOrg.z;

        DVec3d delta = GetExtents();
        delta.z = oldDelta.z;
        SetExtents(delta);
        }

    DPoint3d newOrgView;
    viewRot.MultiplyTranspose(&newOrgView, &newOrigin, 1);
    SetOrigin(newOrgView);

    if (nullptr == cameraView)
        return;

    auto& cameraDef = cameraView->GetCameraR();
    cameraDef.ValidateLens();
    // move the camera back so the entire x,y range is visible at front plane
    double frontDist = std::max(newDelta.x, newDelta.y) /(2.0*tan(cameraDef.GetLensAngle()/2.0));
    double backDist = frontDist + newDelta.z;

    cameraDef.SetFocusDistance(frontDist); // do this even if the camera isn't currently on.
    cameraView->CenterEyePoint(&backDist); // do this even if the camera isn't currently on.
    cameraView->VerifyFocusPlane(); // changes delta/origin
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void OrthographicViewDefinition::_OnTransform(TransformCR trans)
    {
    RotMatrix rMatrix;
    trans.GetMatrix(rMatrix);
    DVec3d scale;
    rMatrix.NormalizeColumnsOf(rMatrix, scale);

    trans.Multiply(m_origin);
    m_rotation.InitProductRotMatrixRotMatrixTranspose(m_rotation, rMatrix);
    m_extents.Scale(scale.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewDefinition::_OnTransform(TransformCR trans)
    {
    DPoint3d eye = m_camera.GetEyePoint();
    trans.Multiply(eye);
    m_camera.SetEyePoint(eye);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::TransformBy(TransformCR trans)
    {
    GetSpatialViewDefinition()._OnTransform(trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
double CameraViewDefinition::CalcLensAngle() const
    {
    double maxDelta = std::max(m_extents.x, m_extents.y);
    return 2.0 * atan2(maxDelta*0.5, m_camera.GetFocusDistance());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewDefinition::CenterEyePoint(double const* backDistanceIn)
    {
    DVec3d delta = GetExtents();
    DPoint3d eyePoint;
    eyePoint.Scale(delta, 0.5);
    eyePoint.z = backDistanceIn ? *backDistanceIn : GetBackDistance();

    GetRotation().MultiplyTranspose(eyePoint);
    eyePoint.Add(GetOrigin());

    m_camera.SetEyePoint(eyePoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewDefinition::CenterFocusDistance()
    {
    double backDist  = GetBackDistance();
    double frontDist = GetFrontDistance();
    DPoint3d eye     = GetEyePoint();
    DPoint3d target  = DPoint3d::FromSumOf(eye, GetZVector(), frontDist-backDist);
    LookAtUsingLensAngle(eye, target, GetYVector(), GetLensAngle(), &frontDist, &backDist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
double SpatialViewDefinition::CalculateMaxDepth(DVec3dCR delta, DVec3dCR zVec)
    {
    // We are going to limit maximum depth to a value that will avoid subtractive cancellation
    // errors on the inverse frustum matrix. - These values will occur when the Z'th row values
    // are very small in comparison to the X-Y values.  If the X-Y values are exactly zero then
    // no error is possible and we'll arbitrarily limit to 1.0E8.
    // This change made to resolve TR# 271876.   RayBentley   04/28/2009.

    static double s_depthRatioLimit       = 1.0E8;          // Limit for depth Ratio.
    static double s_maxTransformRowRatio  = 1.0E5;

    double minXYComponent = std::min(fabs(zVec.x), fabs(zVec.y));
    double maxDepthRatio =(0.0 == minXYComponent) ? s_depthRatioLimit : std::min((s_maxTransformRowRatio / minXYComponent), s_depthRatioLimit);

    return  std::max(delta.x, delta.y) * maxDepthRatio;
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
bool CameraViewController::_OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR location)
    {
    DPoint3d worldPoint;
    if (!convertToWorldPoint(worldPoint, status, GetDgnDb().GeoLocation(), location))
        return false;

    worldPoint.z = GetEyePoint().z;
    DPoint3d targetPoint = GetCameraViewDefinition().GetTargetPoint();
    targetPoint.z = worldPoint.z;
    DVec3d newViewZ;
    newViewZ.DifferenceOf(targetPoint, worldPoint);
    newViewZ.Normalize();
    targetPoint.SumOf(worldPoint, newViewZ, GetFocusDistance());
    LookAt(worldPoint, targetPoint, DVec3d::From(0.0, 0.0, 1.0));

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool OrthographicViewController::_OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR location)
    {
    DPoint3d worldPoint;
    if (!convertToWorldPoint(worldPoint, status, GetDgnDb().GeoLocation(), location))
        return false;

    // If there's no perspective, just center the current location in the view.
    RotMatrix viewInverse;
    viewInverse.InverseOf(GetRotation());

    DPoint3d delta = GetDelta();
    delta.Scale(0.5);
    viewInverse.Multiply(delta);

    worldPoint.DifferenceOf(worldPoint, delta);
    SetOrigin(worldPoint);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
void ViewController::ResetDeviceOrientation()
    {
    m_defaultDeviceOrientationValid = false;
    }

static DVec3d s_defaultForward, s_defaultUp;
static UiOrientation s_lastUi;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool ViewController::OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui, uint32_t nEventsSinceEnabled)
    {
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
        s_defaultUp = m_definition->GetYVector();
        s_defaultForward = m_definition->GetZVector();
        s_lastUi = ui;
        }

    return _OnOrientationEvent(matrix, mode, ui);
    }

//---------------------------------------------------------------------------------------
// Gyro vector convention:
// gyrospace X,Y,Z are (respectively) DOWN, RIGHT, and TOWARDS THE EYE.
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
bool OrthographicViewController::_OnOrientationEvent(RotMatrixCR orientation, OrientationMode mode, UiOrientation ui)
    {
    DVec3d forward, up;
    if (!ViewVectorsFromOrientation(forward, up, orientation, mode, ui))
        return false;

    // No camera, have to manually define origin, etc.
    RotMatrix viewMatrix = GetRotation();
    RotMatrix viewInverse;
    viewInverse.InverseOf(viewMatrix);

    DVec3d delta = GetDelta();
    delta.Scale(0.5);
    DPoint3d worldDelta = delta;
    viewInverse.Multiply(worldDelta);

    // This is the point we want to rotate about.
    DPoint3d eyePoint;
    eyePoint.SumOf(GetOrigin(), worldDelta);

    DVec3d xVector;
    xVector.CrossProduct(forward, up);
    viewMatrix.SetRow(xVector, 0);
    viewMatrix.SetRow(up, 1);
    viewMatrix.SetRow(forward, 2);
    SetRotation(viewMatrix);

    // Now that we have the new rotation, we can work backward from the eye point to get the new origin.
    viewInverse.InverseOf(viewMatrix);
    worldDelta = delta;
    viewInverse.Multiply(worldDelta);
    DPoint3d newOrigin;
    newOrigin.DifferenceOf(eyePoint, worldDelta);
    SetOrigin(newOrigin);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool CameraViewController::_OnOrientationEvent(RotMatrixCR orientation, OrientationMode mode, UiOrientation ui)
    {
    DVec3d forward, up;
    if (!ViewVectorsFromOrientation(forward, up, orientation, mode, ui))
        return false;

    DPoint3d eyePoint = GetEyePoint(), newTarget;
    newTarget.SumOf(eyePoint, forward, -GetFocusDistance());

    LookAt(eyePoint, newTarget, up);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool DrawingViewController::_OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR location)
    {
    DPoint3d worldPoint;
    if (!convertToWorldPoint(worldPoint, status, GetDgnDb().GeoLocation(), location))
        return false;

    RotMatrix viewInverse;
    viewInverse.InverseOf(GetRotation());

    DPoint3d delta = GetDelta();
    delta.Scale(0.5);
    viewInverse.Multiply(delta);

    worldPoint.DifferenceOf(worldPoint, delta);
    SetOrigin(worldPoint);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Ensure the focus plane lies between the front and back planes. If not, center it.
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewDefinition::VerifyFocusPlane()
    {
    DVec3d eyeOrg = DVec3d::FromStartEnd(m_origin, m_camera.GetEyePoint());
    m_rotation.Multiply(eyeOrg);

    double backDist = eyeOrg.z;
    double frontDist = backDist - m_extents.z;

    if (backDist<=0.0 || frontDist<=0.0)
        {
        // the camera location is invalid. Set it based on the view range.
        double tanangle = tan(m_camera.GetLensAngle()/2.0);
        backDist = m_extents.z / tanangle;
        m_camera.SetFocusDistance(backDist/2);
        CenterEyePoint(&backDist);
        return;
        }

    double focusDist = m_camera.GetFocusDistance();
    if (focusDist>frontDist && focusDist<backDist)
        return;

    // put it halfway between front and back planes
    m_camera.SetFocusDistance((m_extents.z / 2.0) + frontDist);

    // moving the focus plane means we have to adjust the origin and delta too (they're on the focus plane, see diagram in ViewController.h)
    double ratio = m_camera.GetFocusDistance() / focusDist;
    m_extents.x *= ratio;
    m_extents.y *= ratio;

    DVec3d xVec, yVec, zVec;
    m_rotation.GetRows(xVec, yVec, zVec);
    m_origin.SumOf(m_camera.GetEyePoint(), zVec, -backDist, xVec, -0.5*m_extents.x, yVec, -0.5*m_extents.y); // this centers the camera too
    }

/*---------------------------------------------------------------------------------**//**
* See diagram in DgnView.h
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewDefinition::LookAt(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVec,
                                            DVec2dCP extentsIn, double const* frontDistIn, double const* backDistIn)
    {
    DVec3d yVec = upVec;
    if (yVec.Normalize() <= mgds_fc_epsilon) // up vector zero length?
        return ViewportStatus::InvalidUpVector;

    DVec3d zVec; // z defined by direction from eye to target
    zVec.DifferenceOf(eyePoint, targetPoint);

    double focusDist = zVec.Normalize(); // set focus at target point
    if (focusDist <= DgnUnits::OneMillimeter())      // eye and target are too close together
        return ViewportStatus::InvalidTargetPoint;

    DVec3d xVec; // x is the normal to the Up-Z plane
    if (xVec.NormalizedCrossProduct(yVec, zVec) <= mgds_fc_epsilon)
        return ViewportStatus::InvalidUpVector;    // up is parallel to z

    if (yVec.NormalizedCrossProduct(zVec, xVec) <= mgds_fc_epsilon) // make sure up vector is perpendicular to z vector
        return ViewportStatus::InvalidUpVector;

    // we now have rows of the rotation matrix
    RotMatrix rotation = RotMatrix::FromRowVectors(xVec, yVec, zVec);

    double backDist  = backDistIn  ? *backDistIn  : GetBackDistance();
    double frontDist = frontDistIn ? *frontDistIn : GetFrontDistance();
    DVec3d delta     = extentsIn   ? DVec3d::From(fabs(extentsIn->x),fabs(extentsIn->y),GetExtents().z) : GetExtents();

    frontDist = std::max(frontDist, DgnUnits::OneMillimeter());
    backDist  = std::max(backDist, focusDist+DgnUnits::OneMillimeter());

    if (backDist < focusDist) // make sure focus distance is in front of back distance.
        backDist = focusDist + DgnUnits::OneMillimeter();

    BeAssert(backDist > frontDist);
    delta.z =(backDist - frontDist);

    DVec3d frontDelta = DVec3d::FromScale(delta, frontDist/focusDist);
    ViewportStatus stat = ValidateViewDelta(frontDelta, false); // validate window size on front (smallest) plane
    if (ViewportStatus::Success != stat)
        return  stat;

    if (delta.z > CalculateMaxDepth(delta, zVec)) // make sure we're not zoomed out too far
        return ViewportStatus::MaxDisplayDepth;

    // The origin is defined as the lower left of the view rectangle on the focus plane, projected to the back plane.
    // Start at eye point, and move to center of back plane, then move left half of width. and down half of height
    DPoint3d origin;
    origin.SumOf(eyePoint, zVec, -backDist, xVec, -0.5*delta.x, yVec, -0.5*delta.y);

    SetEyePoint(eyePoint);
    SetRotation(rotation);
    SetFocusDistance(focusDist);
    SetOrigin(origin);
    SetExtents(delta);
    SetLensAngle(CalcLensAngle());

    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewDefinition::LookAtUsingLensAngle(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVec,
                                                  double lens, double const* frontDist, double const* backDist)
    {
    DVec3d zVec; // z defined by direction from eye to target
    zVec.DifferenceOf(eyePoint, targetPoint);

    double focusDist = zVec.Normalize();  // set focus at target point
    if (focusDist <= DgnUnits::OneMillimeter())       // eye and target are too close together
        return ViewportStatus::InvalidTargetPoint;

    if (lens < .0001 || lens > msGeomConst_pi)
        return ViewportStatus::InvalidLens;

    double extent = 2.0 * tan(lens/2.0) * focusDist;

    DVec2d delta  = DVec2d::From(GetExtents().x, GetExtents().y);
    double longAxis = std::max(delta.x, delta.y);
    delta.Scale(extent/longAxis);

    return LookAt(eyePoint, targetPoint, upVec, &delta, frontDist, backDist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewDefinition::MoveCameraWorld(DVec3dCR distance)
    {
    DPoint3d newTarget, newEyePt;
    newTarget.SumOf(GetTargetPoint(), distance);
    newEyePt.SumOf(GetEyePoint(), distance);
    return LookAt(newEyePt, newTarget, GetYVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewDefinition::MoveCameraLocal(DVec3dCR distanceLocal)
    {
    DVec3d distWorld = distanceLocal;
    GetRotation().MultiplyTranspose(distWorld);
    return MoveCameraWorld(distWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewDefinition::RotateCameraWorld(double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
    {
    DPoint3d about = aboutPointIn ? *aboutPointIn : GetEyePoint();
    RotMatrix rotation = RotMatrix::FromVectorAndRotationAngle(axis, radAngle);
    Transform trans    = Transform::FromMatrixAndFixedPoint(rotation, about);

    DPoint3d newTarget = GetTargetPoint();
    trans.Multiply(newTarget);
    DVec3d upVec = GetYVector();
    rotation.Multiply(upVec);

    return LookAt(GetEyePoint(), newTarget, upVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewDefinition::RotateCameraLocal(double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
    {
    DVec3d axisWorld = axis;
    GetRotation().MultiplyTranspose(axisWorld);
    return RotateCameraWorld(radAngle, axisWorld, aboutPointIn);
    }

/*---------------------------------------------------------------------------------**//**
* See diagram in viewController.h
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
double CameraViewDefinition::GetBackDistance() const
    {
    // backDist is the z component of the vector from the eyePoint to the origin.
    DPoint3d eyeOrg;
    eyeOrg.DifferenceOf(GetEyePoint(), GetOrigin());
    GetRotation().Multiply(eyeOrg); // orient to view
    return eyeOrg.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ViewDefinition::GetCenter() const
    {
    DPoint3d delta;
    GetRotation().MultiplyTranspose(delta, GetExtents());

    DPoint3d center;
    center.SumOf(GetOrigin(), delta, 0.5);
    return  center;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifer     04/07
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d CameraViewDefinition::_GetTargetPoint() const
    {
    DVec3d viewZ;
    GetRotation().GetRow(viewZ, 2);
    DPoint3d target;
    target.SumOf(GetEyePoint(), viewZ, -1.0 * GetFocusDistance());
    return  target;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_AdjustAspectRatio(double windowAspect, bool expandView)
    {
    DPoint3dR origin = m_origin;
    DVec3dR   delta = m_extents;
    RotMatrixR rotation = m_rotation; 

    // first, make sure none of the deltas are negative
    delta.x = fabs(delta.x);
    delta.y = fabs(delta.y);
    delta.z = fabs(delta.z);

    double maxAbs = max(delta.x, delta.y);

    // if all deltas are zero, set to 100 (what else can we do?)
    if (0.0 == maxAbs)
        delta.x = delta.y = 100;

    // if either dimension is zero, set it to the other.
    if (delta.x == 0)
        delta.x = maxAbs;
    if (delta.y == 0)
        delta.y = maxAbs;

    double viewAspect  = delta.x / delta.y;

    if (fabs(1.0 -(viewAspect / windowAspect)) < 1.0e-9)
        return;

    DVec3d oldDelta = delta;

    if (!expandView)
        {
        if (viewAspect > 1.0)
            delta.y = delta.x;
        else
            delta.x = delta.y;
        }

    double maxExtent, minExtent;
    _GetExtentLimits(minExtent, maxExtent);
    if (expandView ?(viewAspect > windowAspect) :(windowAspect > 1.0))
        {
        double rtmp = delta.x / windowAspect;
        if (rtmp < maxExtent)
            delta.y = rtmp;
        else
            {
            delta.y = maxExtent;
            delta.x = maxExtent * windowAspect;
            }
        }
    else
        {
        double rtmp = delta.y * windowAspect;
        if (rtmp < maxExtent)
            delta.x = rtmp;
        else
            {
            delta.x = maxExtent;
            delta.y = maxExtent / windowAspect;
            }
        }

    DPoint3d newOrigin;
    rotation.Multiply(&newOrigin, &origin, 1);
    newOrigin.x +=(oldDelta.x - delta.x) / 2.0;
    newOrigin.y +=(oldDelta.y - delta.y) / 2.0;
    rotation.MultiplyTranspose(origin, newOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_AdjustAspectRatio(double windowAspect, bool expandView)
    {
    // first, make sure none of the deltas are negative
    m_delta.x = fabs(m_delta.x);
    m_delta.y = fabs(m_delta.y);

    double maxAbs = max(m_delta.x, m_delta.y);

    // if all deltas are zero, set to 100 (what else can we do?)
    if (0.0 == maxAbs)
        m_delta.x = m_delta.y = 100;

    // if either dimension is zero, set it to the other.
    if (m_delta.x == 0)
        m_delta.x = maxAbs;
    if (m_delta.y == 0)
        m_delta.y = maxAbs;

    double viewAspect  = m_delta.x / m_delta.y;
    if (fabs(1.0 -(viewAspect / windowAspect)) < 1.0e-9)
        return;

    DVec2d oldDelta = m_delta;
    if (!expandView)
        {
        if (viewAspect > 1.0)
            m_delta.y = m_delta.x;
        else
            m_delta.x = m_delta.y;
        }


    double maxExtent, minExtent;
    _GetExtentLimits(minExtent, maxExtent);
    if (expandView ?(viewAspect > windowAspect) :(windowAspect > 1.0))
        {
        double rtmp = m_delta.x / windowAspect;
        if (rtmp < maxExtent)
            m_delta.y = rtmp;
        else
            {
            m_delta.y = maxExtent;
            m_delta.x = maxExtent * windowAspect;
            }
        }
    else
        {
        double rtmp = m_delta.y * windowAspect;
        if (rtmp < maxExtent)
            m_delta.x = rtmp;
        else
            {
            m_delta.x = maxExtent;
            m_delta.y = maxExtent / windowAspect;
            }
        }

    DPoint2d origin;
    RotMatrix rMatrix = GetRotation();
    rMatrix.Multiply(&origin, &m_origin, 1);
    origin.x +=(oldDelta.x - m_delta.x) / 2.0;
    origin.y +=(oldDelta.y - m_delta.y) / 2.0;
    rMatrix.Transpose();
    rMatrix.Multiply(&m_origin, &origin, 1);
    }

/*---------------------------------------------------------------------------------**//**
* Show the surface normal for geometry under the cursor when snapping.
* @bsimethod                                                    Brien.Bastings  07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawLocateHitDetail(DecorateContextR context, double aperture, HitDetailCR hit)
    {
    DgnViewportR vp = *context.GetViewport();
    if (!vp.Is3dView())
        return; // Not valuable in 2d...

    if (hit.GetHitType() < HitDetailType::Snap)
        return; // Don't display unless snapped...

    if (!hit.GetGeomDetail().IsValidSurfaceHit())
        return; // AccuSnap will flash edge/segment geometry...

    if (!(static_cast<SnapDetailCR>(hit)).IsHot())
        return; // Only display if snap is hot...otherwise it's confusing as it shows the surface information for a location that won't be used...

    ColorDef    color = ColorDef(~vp.GetHiliteColor().GetValue()); // Invert hilite color for good contrast...
    ColorDef    colorFill = color;
    DPoint3d    pt = hit.GetHitPoint();
    double      radius = (2.5 * aperture) * vp.GetPixelSizeAtPoint(&pt);
    DVec3d      normal = hit.GetGeomDetail().GetSurfaceNormal();
    RotMatrix   rMatrix = RotMatrix::From1Vector(normal, 2, true);
    DEllipse3d  ellipse = DEllipse3d::FromScaledRotMatrix(pt, rMatrix, radius, radius, 0.0, Angle::TwoPi());

    GraphicBuilderPtr graphic = context.CreateGraphic();

    colorFill.SetAlpha(150);
    graphic->SetSymbology(color, colorFill, 1);
    graphic->AddArc(ellipse, true, true);
    graphic->AddArc(ellipse, false, false);

    double      length = (0.6 * radius);
    DSegment3d  segment;

    normal.Normalize(ellipse.vector0);
    segment.point[0].SumOf(pt, normal, length);
    segment.point[1].SumOf(pt, normal, -length);
    graphic->AddLineString(2, segment.point);

    normal.Normalize(ellipse.vector90);
    segment.point[0].SumOf(pt, normal, length);
    segment.point[1].SumOf(pt, normal, -length);
    graphic->AddLineString(2, segment.point);
    context.AddWorldOverlay(*graphic);
    }

/*---------------------------------------------------------------------------------**//**
* draw a filled and outlined circle to represent the size of the location tolerance in the current view.
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawLocateCircle(DecorateContextR context, double aperture, DPoint3dCR pt)
    {
    double      radius = (aperture / 2.0) + .5;
    DPoint3d    center;
    DEllipse3d  ellipse, ellipse2;

    context.GetViewport()->WorldToView(&center, &pt, 1);
    ellipse.InitFromDGNFields2d((DPoint2dCR) center, 0.0, radius, radius, 0.0, msGeomConst_2pi, 0.0);
    ellipse2.InitFromDGNFields2d((DPoint2dCR) center, 0.0, radius+1, radius+1, 0.0, msGeomConst_2pi, 0.0);

    GraphicBuilderPtr graphic = context.CreateGraphic();
    ColorDef    white = ColorDef::White();
    ColorDef    black = ColorDef::Black();

    white.SetAlpha(165);
    graphic->SetSymbology(white, white, 1);
    graphic->AddArc2d(ellipse, true, true, 0.0);

    black.SetAlpha(100);
    graphic->SetSymbology(black, black, 1);
    graphic->AddArc2d(ellipse2, false, false, 0.0);

    white.SetAlpha(20);
    graphic->SetSymbology(white, white, 1);
    graphic->AddArc2d(ellipse, false, false, 0.0);
    context.AddViewOverlay(*graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawLocateCursor(DecorateContextR context, DPoint3dCR pt, double aperture, bool isLocateCircleOn, HitDetailCP hit)
    {
    if (nullptr != hit)
        drawLocateHitDetail(context, aperture, *hit);

    if (isLocateCircleOn)
        drawLocateCircle(context, aperture, pt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/86
+---------------+---------------+---------------+---------------+---------------+------*/
static void roundGrid(double& num, double units)
    {
    double sign = ((num * units) < 0.0) ? -1.0 : 1.0;

    num = (num * sign) / units + 0.5;
    num = units * sign * floor(num);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void getGridOrientation(DgnViewportR vp, DPoint3dR origin, RotMatrixR rMatrix, GridOrientationType orientation)
    {
    DgnDbR db = vp.GetViewController().GetDgnDb();

    // start with global origin (in world coords) and identity matrix
    rMatrix.InitIdentity();
    origin = db.GeoLocation().GetGlobalOrigin();

    DVec3d xVec, yVec, zVec;

    switch (orientation)
        {
        case GridOrientationType::View:
            {
            DPoint3d    viewOrigin;
            DPoint3d    centerWorld = vp.NpcToWorld(DPoint3d::From(0.5,0.5,0.5));

            rMatrix = vp.GetRotMatrix();
            rMatrix.Multiply(viewOrigin, *(vp.GetViewOrigin()));
            rMatrix.Multiply(origin);
            origin.z = viewOrigin.z + centerWorld.z;
            rMatrix.MultiplyTranspose(origin, origin);
            break;
            }

        case GridOrientationType::WorldXY:
            {
            break;
            }

        case GridOrientationType::WorldYZ:
            {
            rMatrix.GetRows(xVec, yVec, zVec);
            rMatrix.InitFromRowVectors(yVec, zVec, xVec);
            break;
            }

        case GridOrientationType::WorldXZ:
            {
            rMatrix.GetRows(xVec, yVec, zVec);
            rMatrix.InitFromRowVectors(xVec, zVec, yVec);
            break;
            }
        }

#ifdef DGNV10FORMAT_CHANGES_WIP
    double      angle = 0.0;
    dgnModel_getGridParams(model, NULL, NULL, NULL, NULL, &angle);

    if (0.0 != angle)
        rMatrix->InitFromPrincipleAxisRotations(*rMatrix, 0.0, 0.0, -angle);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void gridFix(DgnViewportR vp, DPoint3dR point, RotMatrixCR rMatrix, DPoint3dCR origin, DPoint2dCR roundingDistance, bool isoGrid)
    {
    DVec3d planeNormal, eyeVec;

    rMatrix.GetRow(planeNormal, 2);

    if (vp.IsCameraOn())
        eyeVec.NormalizedDifference(point, vp.GetCamera().GetEyePoint());
    else
        vp.GetRotMatrix().GetRow(eyeVec, 2);

    LegacyMath::Vec::LinePlaneIntersect(&point, &point, &eyeVec, &origin, &planeNormal, false);

    // get origin and point in view coordinate system
    DPoint3d pointView, originView;

    rMatrix.Multiply(pointView, point);
    rMatrix.Multiply(originView, origin);

    // see whether we need to adjust the origin for iso-grid
    if (isoGrid)
        {
        long ltmp = (long) (pointView.y / roundingDistance.y);

        if (ltmp & 0x0001)
            originView.x += (roundingDistance.x / 2.0);
        }

    // subtract off the origin
    pointView.y -= originView.y;
    pointView.x -= originView.x;

    // round off the remainder to the grid distances
    roundGrid(pointView.y, roundingDistance.y);
    roundGrid(pointView.x, roundingDistance.x);

    // add the origin back in
    pointView.x += originView.x;
    pointView.y += originView.y;

    // go back to root coordinate system
    rMatrix.MultiplyTranspose(point, pointView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewController::_GetGridScaleFactor(DgnViewportR vp) const
    {
    double  scaleFactor = 1.0;

    // Apply ACS scale to grid if ACS Context Lock active...
#ifdef DGNV10FORMAT_CHANGES_WIP
    if (TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_ACS_LOCK)))
#else
    if (false)
#endif
        {
        IAuxCoordSysP acs = IACSManager::GetManager().GetActive(vp);

        if (nullptr != acs)
            scaleFactor *= acs->GetScale();
        }

    return scaleFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_GetGridSpacing(DgnViewportR vp, DPoint2dR spacing, uint32_t& gridsPerRef) const
    {
#if defined DGNV10FORMAT_CHANGES_WIP
    DgnModelRefP targetModelRef = GetTargetModel();

    if (NULL == targetModelRef)
        return ERROR;

    double      uorPerGrid, gridRatio;
    double      scaleFactor = GetGridScaleFactor();

    if (SUCCESS != dgnModel_getGridParams(targetModelRef->GetDgnModelP (), &uorPerGrid, &gridsPerRef, &gridRatio, NULL, NULL) || 0.0 >= uorPerGrid)
        return ERROR;

    uorPerGrid *= scaleFactor;

    spacing.x = uorPerGrid;
    spacing.y = spacing.x * gridRatio;

    double      refScale = (0 == gridsPerRef) ? 1.0 : (double) gridsPerRef;

    spacing.scale(&spacing, refScale);
#else
    gridsPerRef = 10;
    spacing.x = spacing.y = 10.0;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_GetGridRoundingDistance(DgnViewportR vp, DPoint2dR roundingDistance) const
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    ModelInfoCR modelInfo = m_rootModel->GetModelInfo();

    double uorPerGrid     = modelInfo.GetUorPerGrid();
    double gridRatio      = modelInfo.GetGridRatio();
    double roundUnit      = modelInfo.GetRoundoffUnit();
    double roundUnitRatio = modelInfo.GetRoundoffRatio();

    if (TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_GRID_LOCK)))
        roundingDistance.x = uorPerGrid;
    else
        roundingDistance.x = roundUnit;

    if (TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_UNIT_LOCK)))
        {
        if (roundUnit > roundingDistance.x)
            roundingDistance.x = roundUnit;

        if (0.0 != roundUnitRatio)
            gridRatio = roundUnitRatio;
        }

    roundingDistance.y = roundingDistance.x * gridRatio;
    roundingDistance.Scale(roundingDistance, _GetGridScaleFactor());
#else
    roundingDistance.Init(1.0, 1.0);
    roundingDistance.Scale(roundingDistance, _GetGridScaleFactor(vp));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::PointToStandardGrid(DgnViewportR vp, DPoint3dR point, DPoint3dCR gridOrigin, RotMatrixCR gridOrientation) const
    {
#if defined DGNV10FORMAT_CHANGES_WIP
    bool     isoGrid = TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_ISO_GRID));
#else
    bool     isoGrid = false;
#endif
    DPoint2d roundingDistance;

    _GetGridRoundingDistance(vp, roundingDistance);
    gridFix(vp, point, gridOrientation, gridOrigin, roundingDistance, isoGrid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::PointToGrid(DgnViewportR vp, DPoint3dR point) const
    {
    GridOrientationType orientation = _GetGridOrientationType();

    if (GridOrientationType::ACS == orientation)
        {
        IAuxCoordSysP acs = IACSManager::GetManager().GetActive(vp);

        if (NULL != acs)
            acs->PointToGrid(vp, point);

        return;
        }

    DPoint3d    origin;
    RotMatrix   rMatrix;

    getGridOrientation(vp, origin, rMatrix, orientation);
    PointToStandardGrid(vp, point, origin, rMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawGrid(DecorateContextR context)
    {
    DgnViewportR vp = *context.GetViewport();

    if (!vp.IsGridOn())
        return;

    GridOrientationType orientation = _GetGridOrientationType();

    if (GridOrientationType::ACS == orientation)
        {
        IAuxCoordSysP   acs;

        if (NULL != (acs = IACSManager::GetManager().GetActive(vp)))
            acs->DrawGrid(context);
        }
    else
        {
        // if grid units disabled or not set up, give up
#if defined DGNV10FORMAT_CHANGES_WIP
        bool     isoGrid = TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_ISO_GRID));
#else
        bool     isoGrid = false;
#endif
        uint32_t gridsPerRef;
        DPoint2d spacing;

        _GetGridSpacing(vp, spacing, gridsPerRef);

        DPoint3d    origin;
        RotMatrix   rMatrix;

        getGridOrientation(vp, origin, rMatrix, orientation);
        context.DrawStandardGrid(origin, rMatrix, spacing, gridsPerRef, isoGrid, nullptr);
        }
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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewController2d::_CreateScene(RenderContextR context)
    {
    if (m_root.IsNull())
        {
        auto model = GetViewedModel();
        if (nullptr == model)
            return ERROR;

        m_root = model->CreateTileTree(context, *this);
        //BeAssert(m_root.IsValid());
        if (m_root.IsNull())
            return ERROR;
        }

    m_root->DrawInView(context);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController2d::_OnCategoryChange(bool singleEnable)
    {
    // Category stuff is baked into the tiles (for now) - throw them away
    m_root = nullptr;
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
