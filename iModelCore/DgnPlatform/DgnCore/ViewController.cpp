/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewController.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <DgnPlatform/DgnGeoCoord.h>

namespace ViewFlagsJson
{
    static Utf8CP Construction()      {return "construct";}
    static Utf8CP NoText()            {return "noText";}
    static Utf8CP NoDimension()       {return "noDim";}
    static Utf8CP NoPattern()         {return "noPattern";}
    static Utf8CP NoWeight()          {return "noWeight";}
    static Utf8CP NoStyle()           {return "noStyle";}
    static Utf8CP NoTransparency()    {return "noTransp";}
    static Utf8CP Fill()              {return "fill";}
    static Utf8CP Grid()              {return "grid";}
    static Utf8CP Acs()               {return "acs";}
    static Utf8CP NoTexture()         {return "noTexture";}
    static Utf8CP NoMaterial()        {return "noMaterial";}
    static Utf8CP NoSceneLight()      {return "noSceneLight";}
    static Utf8CP VisibleEdges()      {return "visEdges";}
    static Utf8CP HiddenEdges()       {return "hidEdges";}
    static Utf8CP Shadows()           {return "shadows";}
    static Utf8CP NoClipVolume()      {return "noClipVol";}
    static Utf8CP RenderMode()        {return "renderMode";}
    static Utf8CP IgnoreLighting()    {return "ignoreLighting";}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::FromBaseJson(JsonValueCR val)
    {
    memset(this, 0, sizeof(*this));

    m_constructions = val[ViewFlagsJson::Construction()].asBool();
    m_text = !val[ViewFlagsJson::NoText()].asBool();
    m_dimensions = !val[ViewFlagsJson::NoDimension()].asBool();
    m_patterns = !val[ViewFlagsJson::NoPattern()].asBool();
    m_weights = !val[ViewFlagsJson::NoWeight()].asBool();
    m_styles = !val[ViewFlagsJson::NoStyle()].asBool();
    m_transparency = !val[ViewFlagsJson::NoTransparency()].asBool();
    m_fill = val[ViewFlagsJson::Fill()].asBool();
    m_grid = val[ViewFlagsJson::Grid()].asBool();
    m_acsTriad = val[ViewFlagsJson::Acs()].asBool();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::From3dJson(JsonValueCR val)
    {
    m_textures = !val[ViewFlagsJson::NoTexture()].asBool();
    m_materials = !val[ViewFlagsJson::NoMaterial()].asBool();
    m_sceneLights = val[ViewFlagsJson::NoSceneLight()].asBool();
    m_visibleEdges = val[ViewFlagsJson::VisibleEdges()].asBool();
    m_hiddenEdges = val[ViewFlagsJson::HiddenEdges()].asBool();
    m_shadows = val[ViewFlagsJson::Shadows()].asBool();
    m_noClipVolume = val[ViewFlagsJson::NoClipVolume()].asBool();
    m_ignoreLighting = val[ViewFlagsJson::IgnoreLighting()].asBool();

    m_renderMode = RenderMode(val[ViewFlagsJson::RenderMode()].asUInt());

#if defined (TEST_FORCE_VIEW_SMOOTH_SHADE)
    static bool s_forceSmooth=true;
    if (s_forceSmooth)
        m_renderMode = RenderMode::SmoothShade;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::ToBaseJson(JsonValueR val) const
    {
    if (m_constructions) val[ViewFlagsJson::Construction()] = true;
    if (!m_text) val[ViewFlagsJson::NoText()] = true;
    if (!m_dimensions) val[ViewFlagsJson::NoDimension()] = true;
    if (!m_patterns) val[ViewFlagsJson::NoPattern()] = true;
    if (!m_weights) val[ViewFlagsJson::NoWeight()] = true;
    if (!m_styles) val[ViewFlagsJson::NoStyle()] = true;
    if (!m_transparency) val[ViewFlagsJson::NoTransparency()] = true;
    if (m_fill) val[ViewFlagsJson::Fill()] = true;
    if (m_grid) val[ViewFlagsJson::Grid()] = true;
    if (m_acsTriad) val[ViewFlagsJson::Acs()] = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::To3dJson(JsonValueR val) const
    {
    if (!m_textures) val[ViewFlagsJson::NoTexture()] = true;
    if (!m_materials) val[ViewFlagsJson::NoMaterial()] = true;
    if (!m_sceneLights) val[ViewFlagsJson::NoSceneLight()] = true;
    if (m_visibleEdges) val[ViewFlagsJson::VisibleEdges()] = true;
    if (m_hiddenEdges) val[ViewFlagsJson::HiddenEdges()] = true;
    if (m_shadows) val[ViewFlagsJson::Shadows()] = true;
    if (m_noClipVolume) val[ViewFlagsJson::NoClipVolume()] = true;
    if (m_ignoreLighting) val[ViewFlagsJson::IgnoreLighting()] = true;

    val[ViewFlagsJson::RenderMode()] =(uint8_t) m_renderMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::Appearance ViewController::GetSubCategoryAppearance(DgnSubCategoryId subCategoryId) const
    {
    auto const entry = m_subCategories.find(subCategoryId);
    if (entry != m_subCategories.end())
        return entry->second;

    DgnSubCategoryCPtr subCategory = DgnSubCategory::QuerySubCategory(subCategoryId, GetDgnDb());
    BeAssert(subCategory.IsValid());
    DgnSubCategory::Appearance appearance;
    if (subCategory.IsValid())
        appearance = subCategory->GetAppearance();

    auto out = m_subCategories.Insert(subCategoryId, appearance);
    return out.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_ChangeModelDisplay(DgnModelId modelId, bool onOff)
    {
    if (onOff)
        m_viewedModels.insert(modelId);
    else
        m_viewedModels.erase(modelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_ChangeCategoryDisplay(DgnCategoryId categoryId, bool onOff)
    {
    if (onOff)
        m_viewedCategories.insert(categoryId);
    else
        m_viewedCategories.erase(categoryId);

    _OnCategoryChange(onOff);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricModelP ViewController::_GetTargetModel() const { return GetDgnDb().Models().Get<GeometricModel>(m_targetModelId).get(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::ViewController(ViewDefinition const& def)
    : m_dgndb(def.GetDgnDb())
    {
    m_definitionElements.EditElement(def);
    m_viewFlags.InitDefaults();
    m_defaultDeviceOrientation.InitIdentity();
    m_defaultDeviceOrientationValid = false;
    memset(&m_backgroundColor, 0, sizeof(m_backgroundColor));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::LoadCategories()
    {
    m_viewedCategories = GetCategorySelector().GetCategories();

    // load all SubCategories (even for categories not currently on)
    for (auto const& id : DgnSubCategory::QuerySubCategories(GetDgnDb()))
        {           
        DgnSubCategory::Appearance appearance;
        DgnSubCategoryCPtr subCat = DgnSubCategory::QuerySubCategory(id, GetDgnDb());
        if (subCat.IsValid())
            appearance = subCat->GetAppearance();

        m_subCategories.Insert(id, appearance);
        }

#ifdef WIP_VIEW_DEFINITION // *** Get SubCategoryOverrides from CategorySelector
    if (!m_settings.isMember(ViewFlagsJson::SubCategories()))
        return;

    JsonValueCR subcatJson = m_settings[ViewFlagsJson::SubCategories()];
    for (Json::ArrayIndex i=0; i<subcatJson.size(); ++i)
        {
        JsonValueCR val=subcatJson[i];
        DgnSubCategoryId subCategoryId(val[ViewFlagsJson::SubCategoryId()].asUInt64());
        if (subCategoryId.IsValid())
            OverrideSubCategory(subCategoryId, DgnSubCategory::Override(val));
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_LoadFromDefinition()
    {
    auto& dstyle = GetDisplayStyle();
    m_viewFlags = dstyle.GetViewFlags();
    m_backgroundColor = dstyle.GetBackgroundColor();
    LoadCategories();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::LoadFromDefinition()
    {
    _LoadFromDefinition();

    // The QueryModel calls GetModel in the QueryModel thread.  produces a thread race condition if it calls QueryModelById and
    // the model is not already loaded.
    for (auto& id : GetViewedModels())
        GetDgnDb().Models().GetModel(id);

#ifdef WIP_VIEW_DEFINITION // AppData save
    for (auto const& appdata : m_appData) // allow all appdata to restore from settings, if necessary
        appdata.second->_RestoreFromSettings(m_settings);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_StoreToDefinition() const
    {
    auto& dstyle = GetDisplayStyle();
    dstyle.SetViewFlags(m_viewFlags);
    dstyle.SetBackgroundColor(m_backgroundColor);

    auto& catSel = GetCategorySelector();
    catSel.SetCategories(m_viewedCategories);

    if (m_subCategoryOverrides.empty())
        return;

#ifdef WIP_VIEW_DEFINITION // *** Set SubCategoryOverrides in CategorySelector
    BeAssert(false && "WIP_VIEW_DEFINITION");
    JsonValueR ovrJson = m_settings[ViewFlagsJson::SubCategories()];
    int i=0;
    for (auto const& it : m_subCategoryOverrides)
        {
        ovrJson[i][ViewFlagsJson::SubCategoryId()] = it.first.GetValue();
        it.second.ToJson(ovrJson[i]);
        ++i;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::StoreToDefinition()
    {
    _StoreToDefinition();

#ifdef WIP_VIEW_DEFINITION // AppData save
    for (auto const& appdata : m_appData)
        appdata.second->_SaveToSettings(m_settings);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::SaveAs(Utf8CP newName)
    {
    DgnElement::CreateParams params(GetDgnDb(), GetViewDefinition().GetModelId(), GetViewDefinition().GetElementClassId(), ViewDefinition::CreateCode(newName));
    ViewDefinitionPtr newView = dynamic_cast<ViewDefinitionP>(GetViewDefinition().Clone(nullptr, &params).get());
    BeAssert(newView.IsValid());
    if (newView.IsNull())
        return BE_SQLITE_INTERNAL;

    m_definitionElements.RemoveElement(GetViewDefinition());
    m_definitionElements.AddElement(*newView);
    newView = nullptr;

    if (DgnDbStatus::Success != m_definitionElements.Write())
        return BE_SQLITE_INTERNAL;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::SaveTo(Utf8CP newName, DgnViewId& newId)
    {
    auto wasDef = m_definitionElements;
    DbResult rc = SaveAs(newName);
    m_definitionElements = wasDef;
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* return the extents of the target model, if there is one.
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d ViewController::_GetViewedExtents(DgnViewportCR vp) const
    {
    GeometricModelP target = GetTargetModel();
    if (target && target->GetRangeIndexP(false))
        return AxisAlignedBox3d(*target->GetRangeIndexP(false)->GetExtents());

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
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ReloadSubCategory(DgnSubCategoryId id)
    {
    auto unmodified = DgnSubCategory::QuerySubCategory(id, GetDgnDb());
    BeAssert(unmodified.IsValid());
    if (unmodified.IsValid())
        {
        auto const& result = m_subCategories.Insert(id, unmodified->GetAppearance());

        if (!result.second)
            result.first->second = unmodified->GetAppearance(); // we already had this SubCategory; change it.
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::OverrideSubCategory(DgnSubCategoryId id, DgnSubCategory::Override const& ovr)
    {
    if (!id.IsValid())
        return;

    auto result = m_subCategoryOverrides.Insert(id, ovr);
    if (!result.second)
        {
        result.first->second = ovr; // we already had this override; change it.
        ReloadSubCategory(id); // To ensure none of the previous overrides are still active, we reload the original SubCategory
        }

    // now apply this override to the unmodified SubCategory appearance
    auto it = m_subCategories.find(id);
    if (it != m_subCategories.end())
        ovr.ApplyTo(it->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::DropSubCategoryOverride(DgnSubCategoryId id)
    {
    m_subCategoryOverrides.erase(id);
    ReloadSubCategory(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewController::_IsPointAdjustmentRequired(DgnViewportR vp) const {return vp.Is3dView();}
bool ViewController::_IsSnapAdjustmentRequired(DgnViewportR vp, bool snapLockEnabled) const {return snapLockEnabled && vp.Is3dView();}
bool ViewController::_IsContextRotationRequired(DgnViewportR vp, bool contextLockEnabled) const {return contextLockEnabled;}

static bool equalOne(double r1) {return BeNumerical::Compare(r1, 1.0) == 0;}
static bool equalMinusOne(double r1) {return BeNumerical::Compare(r1, -1.0) == 0;}

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
Utf8String ViewController::GetStandardViewName(StandardView viewID)
    {
    if (viewID < StandardView::Top || viewID > StandardView::RightIso)
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

    return DgnCoreL10N::GetString(*(static_cast<int>(viewID) - 1 + names));
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
* @bsimethod                                    Keith.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewController::SetStandardViewRotation(StandardView standardView)
    {
    RotMatrix rMatrix;
    if (!bsiRotMatrix_getStandardRotation(&rMatrix, static_cast<int>(standardView)))
        return  ERROR;

    SetRotation(rMatrix);
    return  SUCCESS;
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

    return _SetupFromFrustum(frustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewController::_SetupFromFrustum(Frustum const& frustum)
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

    ViewportStatus validSize = DgnViewport::ValidateViewDelta(viewDelta, false);
    if (validSize != ViewportStatus::Success)
        return validSize;

    SetOrigin(viewOrg);
    SetDelta(viewDelta);
    SetRotation(viewRot);
    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus  CameraViewController::_SetupFromFrustum(Frustum const& frustum)
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
    if (!Allow3dManipulations() ||(compression >=(1.0 - s_flatViewFractionTolerance)))
        {
#ifdef WIP_VIEW_DEFINITION // camera setup failure
        SetCameraOn(false);
        return ViewportStatus::Success;
#else
        return ViewportStatus::ViewNotInitialized;
#endif
        }

    DPoint3d viewOrg     = frustPts[NPC_000];
    DVec3d viewDelta     = GetDelta();
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
    SetDelta(viewDelta);
    SetLensAngle(CalcLensAngle());
    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::LookAtVolume(DRange3dCR volume, double const* aspect, MarginPercent const* margin, bool expandClippingPlanes)
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
void ViewController::LookAtViewAlignedVolume(DRange3dCR volume, double const* aspect, MarginPercent const* margin, bool expandClippingPlanes)
    {
    DPoint3d    oldDelta = GetDelta();
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

    SpatialViewControllerP physView = (SpatialViewControllerP) _ToSpatialView();
    CameraViewControllerP cameraView =(CameraViewControllerP) _ToCameraView();
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

    if (physView && Allow3dManipulations() && (nullptr == cameraView))
        {
        // make sure that the zDelta is large enough so that entire model will be visible from any rotation
        double diag = newDelta.MagnitudeXY ();
        if (diag > newDelta.z)
            newDelta.z = diag;
        }

    DgnViewport::ValidateViewDelta(newDelta, true);

    SetDelta(newDelta);
    if (aspect)
        AdjustAspectRatio(*aspect, true);

    newDelta = GetDelta();

    newOrigin.x -=(newDelta.x - origNewDelta.x) / 2.0;
    newOrigin.y -=(newDelta.y - origNewDelta.y) / 2.0;
    newOrigin.z -=(newDelta.z - origNewDelta.z) / 2.0;

    // if they don't want the clipping planes to change, set them back to where they were
    if (nullptr != physView && !expandClippingPlanes && Allow3dManipulations())
        {
        viewRot.Multiply(oldOrg);
        newOrigin.z = oldOrg.z;

        DVec3d delta = GetDelta();
        delta.z = oldDelta.z;
        SetDelta(delta);
        }

    DPoint3d newOrgView;
    viewRot.MultiplyTranspose(&newOrgView, &newOrigin, 1);
    SetOrigin(newOrgView);

    if (nullptr == cameraView)
        return;

    cameraView->GetControllerCameraR().ValidateLens();
    // move the camera back so the entire x,y range is visible at front plane
    double frontDist = std::max(newDelta.x, newDelta.y) /(2.0*tan(cameraView->GetLensAngle()/2.0));
    double backDist = frontDist + newDelta.z;

    cameraView->SetFocusDistance(frontDist); // do this even if the camera isn't currently on.
    cameraView->CenterEyePoint(&backDist); // do this even if the camera isn't currently on.
    cameraView->VerifyFocusPlane(); // changes delta/origin 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_FillModels()
    {
    for (DgnModelId modelId : m_viewedModels)
        {
        DgnModelPtr model = GetDgnDb().Models().GetModel(modelId);
        if (model.IsValid())
            model->FillModel();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialViewController::SpatialViewController(SpatialViewDefinition const& def) : T_Super(def)
    {
    m_auxCoordSys = IACSManager::GetManager().CreateACS(); // Should always have an ACS...
    m_auxCoordSys->SetOrigin(def.GetDgnDb().Units().GetGlobalOrigin());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
OrthographicViewController::OrthographicViewController(OrthographicViewDefinition const& def) : T_Super(def)
    {
    // not valid, but better than random
    m_origin.Zero();
    m_delta.Zero();
    m_rotation.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void OrthographicViewController::_OnTransform(TransformCR trans)
    {
    RotMatrix rMatrix;
    trans.GetMatrix(rMatrix);
    DVec3d scale;
    rMatrix.NormalizeColumnsOf(rMatrix, scale);

    trans.Multiply(m_origin);
    m_rotation.InitProductRotMatrixRotMatrixTranspose(m_rotation, rMatrix);
    m_delta.Scale(scale.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::_OnTransform(TransformCR trans)
    {
    DPoint3d eye = GetEyePoint();
    trans.Multiply(eye);
    SetEyePoint(eye);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::TransformBy(TransformCR trans)
    {
    _OnTransform(trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpatialViewController::_SetTargetModel(GeometricModelP target)
    {
    if (nullptr == target || !m_viewedModels.Contains(target->GetModelId()))
        return  ERROR;

    m_targetModelId = target->GetModelId();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (NEEDS_WORK_DRAWINGS)
SectioningViewControllerPtr SectionDrawingViewController::GetSectioningViewController() const
    {
    if (m_sectionView.IsValid())
        return m_sectionView;

    SectionDrawingModel* drawing = GetSectionDrawing();
    if (drawing == nullptr)
        return nullptr;

    auto sectionViewId = GetDgnDb().GeneratedDrawings().QuerySourceView(drawing->GetModelId());
    return dynamic_cast<SectioningViewController*>(GetDgnDb().Views().LoadViewController(sectionViewId, DgnViews::FillModels::Yes).get());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
CameraViewController::CameraViewController(CameraViewDefinition const& def) : T_Super(def)
    {
    // not valid, but better than random
    m_origin.Zero();
    m_delta.Zero();
    m_rotation.InitIdentity();
    memset(&m_camera, 0, sizeof(m_camera));
    m_camera.InvalidateFocus();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
double CameraViewController::CalcLensAngle()
    {
    double maxDelta = std::max(m_delta.x, m_delta.y);
    return 2.0 * atan2(maxDelta*0.5, GetFocusDistance());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::CenterEyePoint(double const* backDistanceIn)
    {
    DVec3d delta = GetDelta();
    DPoint3d eyePoint;
    eyePoint.Scale(delta, 0.5);
    eyePoint.z = backDistanceIn ? *backDistanceIn : GetBackDistance();

    GetRotation().MultiplyTranspose(eyePoint);
    eyePoint.Add(GetOrigin());

    SetEyePoint(eyePoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::CenterFocusDistance()
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
double SpatialViewController::CalculateMaxDepth(DVec3dCR delta, DVec3dCR zVec)
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
static bool convertToWorldPoint(DPoint3dR worldPoint, GeoLocationEventStatus& status, DgnUnits const& units, GeoPointCR location)
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
    if (!convertToWorldPoint(worldPoint, status, GetDgnDb().Units(), location))
        return false;

    worldPoint.z = GetEyePoint().z;
    DPoint3d targetPoint = GetTargetPoint();
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
    if (!convertToWorldPoint(worldPoint, status, GetDgnDb().Units(), location))
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
        s_defaultUp = GetYVector();
        s_defaultForward = GetZVector();
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
    DVec3d currForward = GetZVector();

    orientation.GetColumn(forward, 2);
    switch (mode)
        {
        case OrientationMode::CompassHeading:
            {
            DgnGCS* dgnGcs = GetDgnDb().Units().GetDgnGCS();
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
    if (!convertToWorldPoint(worldPoint, status, GetDgnDb().Units(), location))
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
void CameraViewController::VerifyFocusPlane()
    {
    DVec3d eyeOrg = DVec3d::FromStartEnd(m_origin, m_camera.GetEyePoint());
    m_rotation.Multiply(eyeOrg);

    double backDist = eyeOrg.z;
    double frontDist = backDist - m_delta.z;

    if (backDist<=0.0 || frontDist<=0.0)
        {
        // the camera location is invalid. Set it based on the view range.
        double tanangle = tan(m_camera.GetLensAngle()/2.0);
        backDist = m_delta.z / tanangle;
        m_camera.SetFocusDistance(backDist/2);
        CenterEyePoint(&backDist);
        return;
        }

    double focusDist = m_camera.GetFocusDistance();
    if (focusDist>frontDist && focusDist<backDist)
        return;

    // put it halfway between front and back planes
    m_camera.SetFocusDistance((m_delta.z / 2.0) + frontDist);

    // moving the focus plane means we have to adjust the origin and delta too (they're on the focus plane, see diagram in ViewController.h)
    double ratio = m_camera.GetFocusDistance() / focusDist;
    m_delta.x *= ratio;
    m_delta.y *= ratio;

    DVec3d xVec, yVec, zVec;
    m_rotation.GetRows(xVec, yVec, zVec);
    m_origin.SumOf(m_camera.GetEyePoint(), zVec, -backDist, xVec, -0.5*m_delta.x, yVec, -0.5*m_delta.y); // this centers the camera too
    }

/*---------------------------------------------------------------------------------**//**
* See diagram in viewController.h
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewController::LookAt(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVec,
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
    DVec3d delta     = extentsIn   ? DVec3d::From(fabs(extentsIn->x),fabs(extentsIn->y),GetDelta().z) : GetDelta();

    frontDist = std::max(frontDist, DgnUnits::OneMillimeter());
    backDist  = std::max(backDist, focusDist+DgnUnits::OneMillimeter());

    if (backDist < focusDist) // make sure focus distance is in front of back distance.
        backDist = focusDist + DgnUnits::OneMillimeter();

    BeAssert(backDist > frontDist);
    delta.z =(backDist - frontDist);

    DVec3d frontDelta = DVec3d::FromScale(delta, frontDist/focusDist);
    ViewportStatus stat = DgnViewport::ValidateViewDelta(frontDelta, false); // validate window size on front (smallest) plane
    if (ViewportStatus::Success != stat)
        return  stat;

    if (delta.z > CalculateMaxDepth(delta, zVec)) // make sure we're not zoomed in too far
        return ViewportStatus::MaxDisplayDepth;

    // The origin is defined as the lower left of the view rectangle on the focus plane, projected to the back plane.
    // Start at eye point, and move to center of back plane, then move left half of width. and down half of height
    DPoint3d origin;
    origin.SumOf(eyePoint, zVec, -backDist, xVec, -0.5*delta.x, yVec, -0.5*delta.y);

    SetEyePoint(eyePoint);
    SetRotation(rotation);
    SetFocusDistance(focusDist);
    SetOrigin(origin);
    SetDelta(delta);
    SetLensAngle(CalcLensAngle());

    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewController::LookAtUsingLensAngle(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVec,
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

    DVec2d delta  = DVec2d::From(GetDelta().x, GetDelta().y);
    double longAxis = std::max(delta.x, delta.y);
    delta.Scale(extent/longAxis);

    return LookAt(eyePoint, targetPoint, upVec, &delta, frontDist, backDist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewController::MoveCameraWorld(DVec3dCR distance)
    {
    DPoint3d newTarget, newEyePt;
    newTarget.SumOf(GetTargetPoint(), distance);
    newEyePt.SumOf(GetEyePoint(), distance);
    return LookAt(newEyePt, newTarget, GetYVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewController::MoveCameraLocal(DVec3dCR distanceLocal)
    {
    DVec3d distWorld = distanceLocal;
    GetRotation().MultiplyTranspose(distWorld);
    return MoveCameraWorld(distWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus CameraViewController::RotateCameraWorld(double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
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
ViewportStatus CameraViewController::RotateCameraLocal(double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
    {
    DVec3d axisWorld = axis;
    GetRotation().MultiplyTranspose(axisWorld);
    return RotateCameraWorld(radAngle, axisWorld, aboutPointIn);
    }

/*---------------------------------------------------------------------------------**//**
* See diagram in viewController.h
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
double CameraViewController::GetBackDistance() const
    {
    // backDist is the z component of the vector from the eyePoint to the origin.
    DPoint3d eyeOrg;
    eyeOrg.DifferenceOf(GetEyePoint(), GetOrigin());
    GetRotation().Multiply(eyeOrg); // orient to view
    return eyeOrg.z;
    }

DPoint3d ViewController2d::_GetOrigin() const {return DPoint3d::From(m_origin.x, m_origin.y);}
void     ViewController2d::_SetDelta(DVec3dCR delta) {m_delta.x = delta.x; m_delta.y = delta.y;}
void     ViewController2d::_SetOrigin(DPoint3dCR origin) {m_origin.x = origin.x; m_origin.y = origin.y;}
void     ViewController2d::_SetRotation(RotMatrixCR rot) {DVec3d xColumn; rot.GetColumn(xColumn, 0); m_rotAngle = atan2(xColumn.y, xColumn.x);}
DVec3d   ViewController2d::_GetDelta() const {return DVec3d::From(m_delta.x, m_delta.y);}
RotMatrix ViewController2d::_GetRotation() const {return RotMatrix::FromAxisAndRotationAngle(2, m_rotAngle);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ViewController::GetCenter() const
    {
    DPoint3d delta;
    GetRotation().MultiplyTranspose(delta, GetDelta());

    DPoint3d center;
    center.SumOf(GetOrigin(), delta, 0.5);
    return  center;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifer     04/07
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d CameraViewController::_GetTargetPoint() const
    {
    DVec3d viewZ;
    GetRotation().GetRow(viewZ, 2);
    DPoint3d target;
    target.SumOf(GetEyePoint(), viewZ, -1.0 * GetFocusDistance());
    return  target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
CameraViewDefinition& CameraViewController::GetCameraViewDefinition() const
    {
    return dynamic_cast<CameraViewDefinition&>(GetViewDefinition());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
OrthographicViewDefinition& OrthographicViewController::GetOrthographicViewDefinition() const
    {
    return dynamic_cast<OrthographicViewDefinition&>(GetViewDefinition());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
SpatialViewDefinition& SpatialViewController::GetSpatialViewDefinition() const
    {
    return dynamic_cast<SpatialViewDefinition&>(GetViewDefinition());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
void SpatialViewController::_LoadFromDefinition()
    {
    T_Super::_LoadFromDefinition();

    m_viewedModels = GetModelSelector().GetModelIds();

    if (m_viewedModels.begin() != m_viewedModels.end())
        m_targetModelId = *m_viewedModels.begin();

#ifdef WIP_VIEW_DEFINITION // *** TBD: ClipVolume
    m_... = GetClipVolume().Get ...
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
void SpatialViewController::_StoreToDefinition() const
    {
    T_Super::_StoreToDefinition();

    GetModelSelector().SetModelIds(m_viewedModels);

#ifdef WIP_VIEW_DEFINITION // *** TBD: ClipVolume
    GetClipVolume().Set ...
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
void CameraViewController::_LoadFromDefinition()
    {
    T_Super::_LoadFromDefinition();

    auto& cdef = GetCameraViewDefinition();

    m_origin = cdef.GetBackOrigin();
    m_delta.x = cdef.GetWidth();
    m_delta.y = cdef.GetHeight();
    m_delta.z = cdef.GetDepth();
    m_rotation = cdef.GetViewDirection().ToRotMatrix();
    m_camera.SetLensAngle(cdef.GetLensAngle());
    m_camera.SetFocusDistance(cdef.GetFocusDistance());
    m_camera.SetEyePoint(cdef.GetEyePoint());
    m_camera.ValidateLens();

    VerifyFocusPlane();

    LoadEnvironment();

#if defined (NEEDS_WORK_REALTY_DATA)
    // if the view was saved with an invalid camera lens, just turn the camera off.
    double maxDelta = std::max(m_delta.x, m_delta.y);
    double lensAngle = 2.0 * atan2(maxDelta*0.5, GetFocusDistance());
    if (!CameraInfo::IsValidLensAngle(lensAngle))
        m_isCameraOn = false; *** NEEDS WORK - we have to fix the camera lens
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
void CameraViewController::_StoreToDefinition() const
    {
    T_Super::_StoreToDefinition();

    auto& cdef = GetCameraViewDefinition();
    cdef.SetBackOrigin(m_origin);
    cdef.SetWidth(m_delta.x);
    cdef.SetHeight(m_delta.y);
    cdef.SetDepth(m_delta.z);
    YawPitchRollAngles ypr;
    YawPitchRollAngles::TryFromRotMatrix(ypr, m_rotation);
    cdef.SetViewDirection(ypr);
    cdef.SetLensAngle(m_camera.GetLensAngle());
    cdef.SetFocusDistance(m_camera.GetFocusDistance());
    cdef.SetEyePoint(m_camera.GetEyePoint());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
void OrthographicViewController::_LoadFromDefinition()
    {
    T_Super::_LoadFromDefinition();

    auto& odef = GetOrthographicViewDefinition();
    m_origin = odef.GetOrigin();
    m_delta = odef.GetExtents();
    m_rotation = odef.GetViewDirection().ToRotMatrix();

    DgnViewport::ValidateViewDelta(m_delta, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
void OrthographicViewController::_StoreToDefinition() const
    {
    T_Super::_StoreToDefinition();

    auto& def = GetOrthographicViewDefinition();
    def.SetOrigin(m_origin);
    def.SetExtents(m_delta);
    YawPitchRollAngles ypr;
    YawPitchRollAngles::TryFromRotMatrix(ypr, m_rotation);
    def.SetViewDirection(ypr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::InitDefaults()
    {
    memset(this, 0, sizeof(ViewFlags));

    m_text = true;
    m_dimensions = true;
    m_patterns = true;
    m_weights = true;
    m_styles = true;
    m_transparency = true;
    m_fill = true;
    m_textures = true;
    m_materials = true;
    m_sceneLights = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::AdjustAspectRatio(DPoint3dR origin, DVec3dR delta, RotMatrixR rotation, double windowAspect, bool expandView)
    {
    windowAspect *= GetAspectRatioSkew();

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

    if (expandView ?(viewAspect > windowAspect) :(windowAspect > 1.0))
        {
        double rtmp = delta.x / windowAspect;
        if (rtmp < DgnViewport::GetMaxViewDelta())
            delta.y = rtmp;
        else
            {
            delta.y = DgnViewport::GetMaxViewDelta();
            delta.x = DgnViewport::GetMaxViewDelta() * windowAspect;
            }
        }
    else
        {
        double rtmp = delta.y * windowAspect;
        if (rtmp < DgnViewport::GetMaxViewDelta())
            delta.x = rtmp;
        else
            {
            delta.x = DgnViewport::GetMaxViewDelta();
            delta.y = DgnViewport::GetMaxViewDelta() / windowAspect;
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
void ViewController2d::_AdjustAspectRatio(double windowAspect, bool expandView)
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

    if (expandView ?(viewAspect > windowAspect) :(windowAspect > 1.0))
        {
        double rtmp = m_delta.x / windowAspect;
        if (rtmp < DgnViewport::GetMaxViewDelta())
            m_delta.y = rtmp;
        else
            {
            m_delta.y = DgnViewport::GetMaxViewDelta();
            m_delta.x = DgnViewport::GetMaxViewDelta() * windowAspect;
            }
        }
    else
        {
        double rtmp = m_delta.y * windowAspect;
        if (rtmp < DgnViewport::GetMaxViewDelta())
            m_delta.x = rtmp;
        else
            {
            m_delta.x = DgnViewport::GetMaxViewDelta();
            m_delta.y = DgnViewport::GetMaxViewDelta() / windowAspect;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
void ViewController2d::_LoadFromDefinition()
    {
    T_Super::_LoadFromDefinition();

    ViewDefinition2d& vdef = GetViewDefinition2d();
    m_targetModelId = m_baseModelId = vdef.GetBaseModelId();
    m_origin = vdef.GetOrigin();
    m_delta = vdef.GetExtents();
    m_rotAngle = vdef.GetRotationAngle().Radians();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
void ViewController2d::_StoreToDefinition() const
    {
    T_Super::_StoreToDefinition();

    ViewDefinition2d& vdef = GetViewDefinition2d();
    vdef.SetBaseModelId(m_baseModelId);
    vdef.SetOrigin(m_origin);
    vdef.SetExtents(m_delta);
    vdef.SetRotationAngle(AngleInDegrees::FromRadians(m_rotAngle));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      08/16
//---------------------------------------------------------------------------------------
ViewDefinition2d& ViewController2d::GetViewDefinition2d() const
    {
    return dynamic_cast<ViewDefinition2d&>(GetViewDefinition());
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
    SpatialModelCP model = vp.GetViewController().GetTargetModel()->ToSpatialModel();

    // start with global origin (in world coords) and identity matrix
    rMatrix.InitIdentity();
    origin = (nullptr != model ? model->GetDgnDb().Units().GetGlobalOrigin() : DPoint3d::FromZero());

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
ViewController::CloseMe ViewController::_OnModelsDeleted(bset<DgnModelId> const& deletedIds, DgnDbR db)
    {
    if (&GetDgnDb() != &db)
        return CloseMe::No;

    // Remove deleted models from viewed models list
    for (auto const& deletedId : deletedIds)
        m_viewedModels.erase(deletedId);

    // Ensure we still have a target model - choose a new one arbitrarily if previous was deleted
    RefCountedPtr<GeometricModel> targetModel = GetTargetModel();
    if (targetModel.IsNull())
        {
        for (auto const& viewedId : m_viewedModels)
            if ((targetModel = GetDgnDb().Models().Get<GeometricModel>(viewedId)).IsValid())
                break;

        if (targetModel.IsValid())
            SetTargetModel(targetModel.get());  // NB: ViewController can reject target model...
        }

    // If no target model, no choice but to close the view
    return (nullptr == GetTargetModel()) ? CloseMe::Yes : CloseMe::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawView(ViewContextR context) 
    {
    for (auto modelId : m_viewedModels)
        context.VisitDgnModel(GetDgnDb().Models().GetModel(modelId).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::ViewFlags DisplayStyle::GetViewFlags() const
    {
    Json::Value value(Json::objectValue);
    Json::Reader::Parse(GetPropertyValueString("ViewFlags").c_str(), value);
    ViewFlags flags;
    flags.FromBaseJson(value);
    flags.From3dJson(value);
    return flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DisplayStyle::SetViewFlags(Render::ViewFlags const& flags)
    {
    Json::Value value(Json::objectValue);
    flags.ToBaseJson(value);
    flags.To3dJson(value);
    return SetPropertyValue("ViewFlags", Json::FastWriter::ToString(value).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId ViewController::GetViewId() const {return GetViewDefinition().GetViewId();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingViewController::DrawingViewController(DrawingViewDefinition const& def) : ViewController2d(def) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
SheetViewController::SheetViewController(SheetViewDefinition const& def) : ViewController2d(def) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController2d::ViewController2d(ViewDefinition2d const& def) : ViewController(def)
    {
    m_baseModelId = m_targetModelId = def.GetBaseModelId();
    m_origin.Zero();
    m_delta.Zero();
    m_rotAngle = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinitionR ViewController::GetViewDefinition() const
    {
    return *m_definitionElements.FindByClass<ViewDefinition>(*GetViewDefinitionClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
CategorySelectorR ViewController::GetCategorySelector() const
    {
    auto catSel = m_definitionElements.FindByClass<CategorySelector>(*GetCategorySelectorClass());

    if (!catSel.IsValid())
        {
        auto existingCatSel = GetDgnDb().Elements().Get<CategorySelector>(GetViewDefinition().GetCategorySelectorId());
        if (existingCatSel.IsValid())
            catSel = existingCatSel->MakeCopy<CategorySelector>();
        else
            catSel = new CategorySelector(GetDgnDb(), GetViewDefinition().GetName().c_str()); // *** WIP_VIEW_DEFINITION - auto-creation of definitions??

        m_definitionElements.AddElement(*catSel);
        }

    return *catSel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleR ViewController::GetDisplayStyle() const
    {
    auto dstyle = m_definitionElements.FindByClass<DisplayStyle>(*GetDisplayStyleClass());

    if (!dstyle.IsValid())
        {
        auto existingDstyle = GetDgnDb().Elements().Get<DisplayStyle>(GetViewDefinition().GetDisplayStyleId());
        if (existingDstyle.IsValid())
            dstyle = existingDstyle->MakeCopy<DisplayStyle>();
        else
            dstyle = new DisplayStyle(GetDgnDb(), GetViewDefinition().GetName().c_str());     // *** WIP_VIEW_DEFINITION - auto-creation of definitions??

        m_definitionElements.AddElement(*dstyle);
        }

    return *dstyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSelectorR SpatialViewController::GetModelSelector() const
    {
    auto modSel = m_definitionElements.FindByClass<ModelSelector>(*GetModelSelectorClass());

    if (!modSel.IsValid())
        {
        auto existingModSel = GetDgnDb().Elements().Get<ModelSelector>(GetSpatialViewDefinition().GetModelSelectorId());
        if (existingModSel.IsValid())
            modSel = existingModSel->MakeCopy<ModelSelector>();
        else
            modSel = new ModelSelector(GetDgnDb(), GetViewDefinition().GetName().c_str());    // *** WIP_VIEW_DEFINITION - auto-creation of definitions??

        m_definitionElements.AddElement(*modSel);
        }

    return *modSel;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_FixUpDefinitionRelationships()
    {
    auto& vdef = GetViewDefinition();
    vdef.SetDisplayStyleId(GetDisplayStyle().GetElementId());
    vdef.SetCategorySelectorId(GetCategorySelector().GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::_FixUpDefinitionRelationships()
    {
    T_Super::_FixUpDefinitionRelationships();
    auto& vdef = GetSpatialViewDefinition();
    vdef.SetModelSelectorId(GetModelSelector().GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewController::SaveDefinition()
    {
    bool anyInserts;
    DgnDbStatus status = GetDefinitionR().Write(&anyInserts);
    if (DgnDbStatus::Success != status)
        return status;

    if (anyInserts)
        {
        _FixUpDefinitionRelationships();
        GetDefinitionR().Write();
        }
    return status;
    }
