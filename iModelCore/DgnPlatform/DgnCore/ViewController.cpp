/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewController.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Geom/eigensys3d.fdf>
#include <DgnPlatform/DgnMarkupProject.h>

static Utf8CP VIEW_SETTING_Area2d                = "area2d";
static Utf8CP VIEW_SETTING_BackgroundColor       = "bgColor";
static Utf8CP VIEW_SETTING_CameraAngle           = "cameraAngle";
static Utf8CP VIEW_SETTING_CameraFocalLength     = "cameraFocalLength";
static Utf8CP VIEW_SETTING_CameraPosition        = "cameraPosition";
static Utf8CP VIEW_SETTING_Delta                 = "delta";
static Utf8CP VIEW_SETTING_Flags                 = "flags";
static Utf8CP VIEW_SETTING_IsCameraOn            = "isCameraOn";
static Utf8CP VIEW_SETTING_Categories            = "categories";
static Utf8CP VIEW_SETTING_Models                = "models";
static Utf8CP VIEW_SETTING_Origin                = "origin";
static Utf8CP VIEW_SETTING_RotAngle              = "rotAngle";
static Utf8CP VIEW_SETTING_Rotation              = "rotation";
static Utf8CP VIEW_SETTING_SubCategories         = "subCategories";
static Utf8CP VIEW_SubCategoryId                 = "subCategoryId";

static Utf8CP VIEWFLAG_construction              = "construct";
static Utf8CP VIEWFLAG_noText                    = "noText";
static Utf8CP VIEWFLAG_noDimension               = "noDim";
static Utf8CP VIEWFLAG_noPattern                 = "noPattern";
static Utf8CP VIEWFLAG_noWeight                  = "noWeight";
static Utf8CP VIEWFLAG_noStyle                   = "noStyle";
static Utf8CP VIEWFLAG_noTransparency            = "noTransp";
static Utf8CP VIEWFLAG_fill                      = "fill";
static Utf8CP VIEWFLAG_grid                      = "grid";
static Utf8CP VIEWFLAG_acs                       = "acs";
static Utf8CP VIEWFLAG_useBgImage                = "noBgImage";

static Utf8CP VIEWFLAG_noTexture                 = "noTexture";
static Utf8CP VIEWFLAG_noMaterial                = "noMaterial";
static Utf8CP VIEWFLAG_noSceneLight              = "noSceneLight";
static Utf8CP VIEWFLAG_visibleEdges              = "visEdges";
static Utf8CP VIEWFLAG_hiddenEdges               = "hidEdges";
static Utf8CP VIEWFLAG_shadows                   = "shadows";
static Utf8CP VIEWFLAG_frontClip                 = "frontClip";
static Utf8CP VIEWFLAG_backClip                  = "backClip";
static Utf8CP VIEWFLAG_noClipVolume              = "noClipVol";
static Utf8CP VIEWFLAG_renderMode                = "renderMode";
static Utf8CP VIEWFLAG_ignoreLighting            = "ignoreLighting";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewHandlerP ViewHandler::FindHandler(DgnDb const& db, DgnClassId handlerId)
    {
    // quick check for a handler already known
    DgnDomain::Handler* handler = db.Domains().LookupHandler(handlerId);
    if (nullptr != handler)
        return handler->_ToViewHandler();

    // not there, check via base classes
    handler = db.Domains().FindHandler(handlerId, db.Domains().GetClassId(GetHandler()));
    return handler ? handler->_ToViewHandler() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewHandler::_SupplyController(DgnDbR db, DgnViews::View const& view)
    {
    auto const& schemas = db.Schemas();
    ECClassCP viewClass = schemas.GetECClass(view.GetClassId().GetValue());

    if (nullptr==viewClass)
        return nullptr;

    if (viewClass->Is(schemas.GetECClass("dgn", "CameraView")))
        return new QueryViewController(db, view.GetId());

    if (viewClass->Is(schemas.GetECClass("dgn", "SheetView")))
        return new SheetViewController(db, view.GetId());

    if (viewClass->Is(schemas.GetECClass("dgn", "DrawingView")))
        return new DrawingViewController(db, view.GetId());

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr DgnViews::LoadViewController(DgnViewId viewId, FillModels fillModels) const
    {
    DgnViews::View view = QueryView(viewId);
    if (!view.IsValid())
        return nullptr;

    // make sure the class derives from Model (has a handler)
    ViewHandlerP handler = ViewHandler::FindHandler(m_dgndb, view.GetClassId());
    if (nullptr == handler)
        {
        BeAssert(false);
        return nullptr;
        }

    // if there's an "override" extension on the view handler, see if it wants to supply the view
    ViewHandlerOverride* ovr = ViewHandlerOverride::Cast(*handler);
    ViewControllerPtr controller = ovr ? ovr->_SupplyController(m_dgndb, view) : nullptr;

    if (!controller.IsValid())
        controller = handler->_SupplyController(m_dgndb, view); // use handler

    if (!controller.IsValid())
        return nullptr;

    controller->Load();
    if (fillModels == FillModels::Yes)
        controller->_FillModels();

    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::FromBaseJson(JsonValueCR val)
    {
    memset(this, 0, sizeof(*this));

    constructions = val[VIEWFLAG_construction].asBool();
    text = !val[VIEWFLAG_noText].asBool();
    dimensions = !val[VIEWFLAG_noDimension].asBool();
    patterns = !val[VIEWFLAG_noPattern].asBool();
    weights = !val[VIEWFLAG_noWeight].asBool();
    styles = !val[VIEWFLAG_noStyle].asBool();
    transparency = !val[VIEWFLAG_noTransparency].asBool();
    fill = val[VIEWFLAG_fill].asBool();
    grid = val[VIEWFLAG_grid].asBool();
    acs = val[VIEWFLAG_acs].asBool();
    bgImage = val[VIEWFLAG_useBgImage].asBool();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::From3dJson(JsonValueCR val)
    {
    textures = !val[VIEWFLAG_noTexture].asBool();
    materials = !val[VIEWFLAG_noMaterial].asBool();
    sceneLights = val[VIEWFLAG_noSceneLight].asBool();
    visibleEdges = val[VIEWFLAG_visibleEdges].asBool();
    hiddenEdges = val[VIEWFLAG_hiddenEdges].asBool();
    shadows = val[VIEWFLAG_shadows].asBool();
    noFrontClip = !val[VIEWFLAG_frontClip].asBool();
    noBackClip  = !val[VIEWFLAG_backClip].asBool();
    noClipVolume = val[VIEWFLAG_noClipVolume].asBool();
    ignoreLighting = val[VIEWFLAG_ignoreLighting].asBool();

    m_renderMode = DgnRenderMode(val[VIEWFLAG_renderMode].asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::ToBaseJson(JsonValueR val) const
    {
    if (constructions) val[VIEWFLAG_construction] = true;
    if (!text) val[VIEWFLAG_noText] = true;
    if (!dimensions) val[VIEWFLAG_noDimension] = true;
    if (!patterns) val[VIEWFLAG_noPattern] = true;
    if (!weights) val[VIEWFLAG_noWeight] = true;
    if (!styles) val[VIEWFLAG_noStyle] = true;
    if (!transparency) val[VIEWFLAG_noTransparency] = true;
    if (fill) val[VIEWFLAG_fill] = true;
    if (grid) val[VIEWFLAG_grid] = true;
    if (acs) val[VIEWFLAG_acs] = true;
    if (bgImage) val[VIEWFLAG_useBgImage] = true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::To3dJson(JsonValueR val) const
    {
    if (!textures) val[VIEWFLAG_noTexture] = true;
    if (!materials) val[VIEWFLAG_noMaterial] = true;
    if (!sceneLights) val[VIEWFLAG_noSceneLight] = true;
    if (visibleEdges) val[VIEWFLAG_visibleEdges] = true;
    if (hiddenEdges) val[VIEWFLAG_hiddenEdges] = true;
    if (shadows) val[VIEWFLAG_shadows] = true;
    if (!noFrontClip) val[VIEWFLAG_frontClip] = true;
    if (!noBackClip) val[VIEWFLAG_backClip] = true;
    if (noClipVolume) val[VIEWFLAG_noClipVolume] = true;
    if (ignoreLighting) val[VIEWFLAG_ignoreLighting] = true;

    val[VIEWFLAG_renderMode] =(uint8_t) m_renderMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::Appearance ViewController::GetSubCategoryAppearance(DgnSubCategoryId subCategoryId) const
    {
    auto const entry = m_subCategories.find(subCategoryId);
    if (entry != m_subCategories.end())
        return entry->second;

    DgnSubCategoryCPtr subCategory = DgnSubCategory::QuerySubCategory(subCategoryId, m_dgndb);
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
DgnModelP ViewController::_GetTargetModel() const {return m_dgndb.Models().GetModel(m_targetModelId).get();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::ViewController(DgnDbR dgndb, DgnViewId viewId) : m_dgndb(dgndb)
    {
    m_viewId = viewId;
    m_viewFlags.InitDefaults();
    m_defaultDeviceOrientation.InitIdentity();
    m_defaultDeviceOrientationValid = false;
    memset(&m_backgroundColor, 0, sizeof(m_backgroundColor));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::LoadCategories(JsonValueCR settings)
    {
    if (settings.isMember(VIEW_SETTING_Categories))
        m_viewedCategories.FromJson(settings[VIEW_SETTING_Categories]);

    // load all SubCategories (even for categories not currently on)
    for (auto const& id : DgnSubCategory::QuerySubCategories(m_dgndb))
        {
        DgnSubCategory::Appearance appearance;
        DgnSubCategoryCPtr subCat = DgnSubCategory::QuerySubCategory(id, m_dgndb);
        if (subCat.IsValid())
            appearance = subCat->GetAppearance();

        m_subCategories.Insert(id, appearance);
        }

    if (!settings.isMember(VIEW_SETTING_SubCategories))
        return;

    JsonValueCR facetJson = settings[VIEW_SETTING_SubCategories];
    for (Json::ArrayIndex i=0; i<facetJson.size(); ++i)
        {
        JsonValueCR val=facetJson[i];
        DgnSubCategoryId subCategoryId(val[VIEW_SubCategoryId].asUInt64());
        if (subCategoryId.IsValid())
            OverrideSubCategory(subCategoryId, DgnSubCategory::Override(val));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_RestoreFromSettings(JsonValueCR settings)
    {
    if (!settings.isMember(VIEW_SETTING_Flags))
        m_viewFlags.InitDefaults();
    else
        m_viewFlags.FromBaseJson(settings[VIEW_SETTING_Flags]);

    if (!settings.isMember(VIEW_SETTING_BackgroundColor))
        m_backgroundColor = ColorDef::Black();
    else
        m_backgroundColor = ColorDef(settings[VIEW_SETTING_BackgroundColor].asUInt());

    LoadCategories(settings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::Load()
    {
    DgnViews::View entry = m_dgndb.Views().QueryView(m_viewId);
    if (!entry.IsValid())
        {
        BeAssert(false);
        return  BE_SQLITE_ERROR;
        }

    m_viewedModels.clear();
    m_baseModelId = m_targetModelId = entry.GetBaseModelId();
    m_viewedModels.insert(m_baseModelId);

    Utf8String settingsStr;
    //  The QueryModel calls GetModel in the QueryModel thread.  produces a thread race condition if it calls QueryModelById and
    DbResult  rc = GetDgnDb().Views().QueryProperty(settingsStr, GetViewId(), DgnViewProperty::Settings());
    if (BE_SQLITE_ROW != rc)
        return rc;

    Json::Value json;
    Json::Reader::Parse(settingsStr, json);
    _RestoreFromSettings(json);

    //  The QueryModel calls GetModel in the QueryModel thread.  produces a thread race condition if it calls QueryModelById and
    //  the model is not already loaded.
    for (auto&id : GetViewedModels())
        m_dgndb.Models().GetModel(id);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_SaveToSettings(JsonValueR settings) const
    {
    m_viewFlags.ToBaseJson(settings[VIEW_SETTING_Flags]);

    // only save background color if it's not the default (black)...
    if (ColorDef::Black() != m_backgroundColor)
        settings[VIEW_SETTING_BackgroundColor] = m_backgroundColor.GetValue();

    m_viewedCategories.ToJson(settings[VIEW_SETTING_Categories]);
    if (m_subCategoryOverrides.empty())
        return;

    JsonValueR ovrJson = settings[VIEW_SETTING_SubCategories];
    int i=0;
    for (auto const& it : m_subCategoryOverrides)
        {
        ovrJson[i][VIEW_SubCategoryId] = it.first.GetValue();
        it.second.ToJson(ovrJson[i]);
        ++i;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::Save()
    {
    Json::Value settings;
    _SaveToSettings(settings);

    return m_dgndb.Views().SavePropertyString(m_viewId, DgnViewProperty::Settings(), Json::FastWriter::ToString(settings));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::SaveAs(Utf8CP newName)
    {
    DgnViews::View newRow(m_dgndb.Views().QueryView(m_viewId));
    newRow.SetName(newName);

    DbResult rc = m_dgndb.Views().Insert(newRow);
    if (BE_SQLITE_OK != rc)
        return rc;

    m_viewId = newRow.GetId();
    rc = Save();

    if (BE_SQLITE_OK == rc)
        m_dgndb.SaveSettings();

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::SaveTo(Utf8CP newName, DgnViewId& newId)
    {
    AutoRestore<DgnViewId> saveId(&m_viewId);

    DbResult rc = SaveAs(newName);
    newId =(BE_SQLITE_OK == rc) ? m_viewId : DgnViewId();
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d ViewController::_GetViewedExtents() const
    {
    return m_dgndb.Units().GetProjectExtents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawElement(ViewContextR context, GeometrySourceCR element)
    {
    element.Draw(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawElementFiltered(ViewContextR context, GeometrySourceCR element, DPoint3dCP pts, double size)
    {
    // Display nothing...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ReloadSubCategory(DgnSubCategoryId id)
    {
    auto unmodified = DgnSubCategory::QuerySubCategory(id, m_dgndb);
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

/////////////////////////////////////////////////////////////////////////////////////
///
/// Standard Views
///
/////////////////////////////////////////////////////////////////////////////////////

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

            if (equalOne(((DVec3d*)isoMatrix.form3d[0])->DotProduct (*((DVec3d*)rMatrix.form3d[0]))) &&
                equalOne(((DVec3d*)isoMatrix.form3d[1])->DotProduct (*((DVec3d*)rMatrix.form3d[1]))))
                return StandardView::Iso;

            bsiRotMatrix_getStandardRotation(&isoMatrix, static_cast<int>(StandardView::RightIso));
            if (equalOne(((DVec3d*)isoMatrix.form3d[0])->DotProduct (*((DVec3d*)rMatrix.form3d[0]))) &&
                equalOne(((DVec3d*)isoMatrix.form3d[1])->DotProduct (*((DVec3d*)rMatrix.form3d[1]))))
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

    ViewportStatus validSize = DgnViewport::ValidateWindowSize(viewDelta, false);
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
ViewportStatus CameraViewController::_SetupFromFrustum(Frustum const& frustum)
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
        SetCameraOn(false);
        return ViewportStatus::Success;
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
    SetCameraOn(true);
    CalculateLensAngle();
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

    PhysicalViewControllerP physView =(PhysicalViewControllerP) _ToPhysicalView();
    CameraViewControllerP cameraView =(CameraViewControllerP) _ToCameraView();
    DPoint3d origNewDelta = newDelta;

    bool isCameraOn = cameraView && cameraView->IsCameraOn();
    if (isCameraOn)
        {
        // If the camera is on, the only way to guarantee we can see the entire volume is to set delta at the front plane, not focus plane.
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

    if (physView && Allow3dManipulations() && !isCameraOn)
        {
        // make sure that the zDelta is large enough so that entire model will be visible from any rotation
        double diag = newDelta.MagnitudeXY ();
        if (diag > newDelta.z)
            newDelta.z = diag;
        }

    DgnViewport::ValidateWindowSize(newDelta, true);

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
        DgnModelPtr model = m_dgndb.Models().GetModel(modelId);
        if (model.IsValid())
            model->FillModel();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalViewController::PhysicalViewController(DgnDbR dgndb, DgnViewId viewId) : ViewController(dgndb, viewId)
    {
    // not valid, but better than random
    m_origin.Zero();
    m_delta.Zero();
    m_rotation.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::_OnTransform(TransformCR trans)
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
    T_Super::_OnTransform(trans);
    DPoint3d eye = GetEyePoint();
    trans.Multiply(eye);
    SetEyePoint(eye);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::TransformBy(TransformCR trans)
    {
    _OnTransform(trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PhysicalViewController::SetTargetModel(DgnModelP target)
    {
    if (!m_viewedModels.Contains(target->GetModelId()))
        return  ERROR;

    m_targetModelId = target->GetModelId();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
SectioningViewControllerPtr SectionDrawingViewController::GetSectioningViewController() const
    {
#if defined(NEEDS_WORK_DRAWINGS)
    if (m_sectionView.IsValid())
        return m_sectionView;

    SectionDrawingModel* drawing = GetSectionDrawing();
    if (drawing == nullptr)
        return nullptr;

    auto sectionViewId = GetDgnDb().GeneratedDrawings().QuerySourceView(drawing->GetModelId());
    return dynamic_cast<SectioningViewController*>(GetDgnDb().Views().LoadViewController(sectionViewId, DgnViews::FillModels::Yes).get());
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr SectionDrawingViewController::GetProjectClipVector() const
    {
    auto sectionView = GetSectioningViewController();
    if (!sectionView.IsValid())
        return ClipVector::Create();
    return sectionView->GetInsideForwardClipVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool SectionDrawingViewController::GetSectionHasDogLeg() const
    {
    auto sectionView = GetSectioningViewController();
    if (!sectionView.IsValid())
        return false;

    return sectionView->HasDogLeg();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SectionDrawingViewController::_VisitHit(HitDetailCR hit, ViewContextR context) const
    {
#if defined(NEEDS_WORK_ELEMENTS_API)
    context.PushTransform(GetFlatteningMatrixIf2D(context));
#endif
    StatusInt status = T_Super::_VisitHit(hit, context);
    context.PopTransformClip();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/14
//--------------+------------------------------------------------------------------------
void SectionDrawingViewController::_DrawView(ViewContextR context)
    {
#if defined(NEEDS_WORK_ELEMENTS_API)
    context.PushTransform(GetFlatteningMatrixIf2D(context));
#endif
    T_Super::_DrawView(context);
    context.PopTransformClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SectionDrawingViewController::_DrawElement(ViewContextR context, GeometrySourceCR element)
    {
#if defined(NEEDS_WORK_VIEW_CONTROLLER)
    if (context.GetViewport() != nullptr)
        {
        auto hyper = context.GetViewport()->GetViewControllerP()->ToHypermodelingViewController();
        if (hyper != nullptr && !hyper->ShouldDrawAnnotations() && !ProxyDisplayHandlerUtils::IsProxyDisplayHandler(elIter.GetHandler()))
            return;
        }
#endif

    T_Super::_DrawElement(context, element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
CameraViewController::CameraViewController(DgnDbR project, DgnViewId viewId) : PhysicalViewController(project, viewId)
    {
    // not valid, but better than random
    m_isCameraOn = false;
    memset(&m_camera, 0, sizeof(m_camera));
    m_camera.InvalidateFocus();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::CalculateLensAngle()
    {
    double maxDelta = std::max(m_delta.x, m_delta.y);
    double lensAngle = 2.0 * atan2(maxDelta*0.5, GetFocusDistance());
    SetLensAngle(lensAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::CenterEyePoint(double const* backDistanceIn)
    {
    BeAssert(IsCameraValid());

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
double PhysicalViewController::CalculateMaxDepth(DVec3dCR delta, DVec3dCR zVec)
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
static bool convertToWorldPointWithStatus(DPoint3dR worldPoint, GeoLocationEventStatus& status, DgnUnits const& units, GeoPointCR location)
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
    if (!IsCameraOn())
        return T_Super::_OnGeoLocationEvent(status, location);

    DPoint3d worldPoint;
    if (!convertToWorldPointWithStatus(worldPoint, status, m_dgndb.Units(), location))
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
bool PhysicalViewController::_OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR location)
    {
    DPoint3d worldPoint;
    if (!convertToWorldPointWithStatus(worldPoint, status, m_dgndb.Units(), location))
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

// Temporary hack to make this work properly. The first rotation I receive from CM seems
// to always be the reference frame - using that at best causes the camera to skip for a frame,
// and at worst causes it to be permanently wrong (RelativeHeading).  Will remove and clean
// up alongside s_defaultForward/s_defaultUp.
static bool s_isFirstMotion = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
void ViewController::ResetDeviceOrientation()
    {
    m_defaultDeviceOrientationValid = false;
    s_isFirstMotion = true;
    }

static DVec3d s_defaultForward, s_defaultUp;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool ViewController::OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui)
    {
    if (!m_defaultDeviceOrientationValid)
        {
        if (s_isFirstMotion)
            {
            s_isFirstMotion = false;
            return false;
            }
        m_defaultDeviceOrientation = matrix;
        m_defaultDeviceOrientationValid = true;
        s_defaultUp = GetYVector();
        s_defaultForward = GetZVector();
        }

    return _OnOrientationEvent(matrix, mode, ui);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool PhysicalViewController::ViewVectorsFromOrientation(DVec3dR forward, DVec3dR up, RotMatrixCR orientation, OrientationMode mode, UiOrientation ui)
    {
    double azimuthCorrection = 0.0;
    DVec3d currForward = GetZVector();

    orientation.GetColumn(forward, 2);
    switch(mode)
        {
        case OrientationMode::CompassHeading:
            azimuthCorrection = msGeomConst_radiansPerDegree *(90.0 + m_dgndb.Units().GetAzimuth());
            forward.RotateXY(azimuthCorrection);
            break;
        case OrientationMode::IgnoreHeading:
            forward.x = currForward.x;
            forward.y = currForward.y;
            break;
        case OrientationMode::RelativeHeading:
            forward.x = s_defaultForward.x +(orientation.form3d[0][2] - m_defaultDeviceOrientation.form3d[0][2]);
            forward.y = s_defaultForward.y +(orientation.form3d[1][2] - m_defaultDeviceOrientation.form3d[1][2]);
            break;
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
bool PhysicalViewController::_OnOrientationEvent(RotMatrixCR orientation, OrientationMode mode, UiOrientation ui)
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
    if (!IsCameraOn())
        return T_Super::_OnOrientationEvent(orientation, mode, ui);

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
    if (!convertToWorldPointWithStatus(worldPoint, status, m_dgndb.Units(), location))
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
    if (!m_isCameraOn)
        return;

    DVec3d eyeOrg = DVec3d::FromStartEnd(m_origin, m_camera.GetEyePoint());
    m_rotation.Multiply(eyeOrg);

    double backDist = eyeOrg.z;
    double frontDist = backDist - m_delta.z;
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
    ViewportStatus stat = DgnViewport::ValidateWindowSize(frontDelta, false); // validate window size on front (smallest) plane
    if (ViewportStatus::Success != stat)
        return  stat;

    if (delta.z > CalculateMaxDepth(delta, zVec)) // make sure we're not zoomed in too far
        return ViewportStatus::MaxDisplayDepth;

    // The origin is defined as the lower left of the view rectangle on the focus plane, projected to the back plane.
    // Start at eye point, and move to center of back plane, then move left half of width. and down half of height
    DPoint3d origin;
    origin.SumOf(eyePoint, zVec, -backDist, xVec, -0.5*delta.x, yVec, -0.5*delta.y);

    SetCameraOn(true);
    SetEyePoint(eyePoint);
    SetRotation(rotation);
    SetFocusDistance(focusDist);
    SetOrigin(origin);
    SetDelta(delta);
    CalculateLensAngle();

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
DPoint3d ViewController::GetTargetPoint() const {return _GetTargetPoint();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifer     04/07
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d CameraViewController::_GetTargetPoint() const
    {
    if (!IsCameraOn())
        return T_Super::_GetTargetPoint();

    DVec3d viewZ;
    GetRotation().GetRow(viewZ, 2);
    DPoint3d target;
    target.SumOf(GetEyePoint(), viewZ, -1.0 * GetFocusDistance());
    return  target;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
void CameraViewController::_RestoreFromSettings(JsonValueCR jsonObj)
    {
    T_Super::_RestoreFromSettings(jsonObj);

    m_isCameraOn = jsonObj[VIEW_SETTING_IsCameraOn].asBool();
    m_camera.SetLensAngle(jsonObj[VIEW_SETTING_CameraAngle].asDouble());
    m_camera.SetFocusDistance(jsonObj[VIEW_SETTING_CameraFocalLength].asDouble());

    DPoint3d eyePt;
    JsonUtils::DPoint3dFromJson(eyePt, jsonObj[VIEW_SETTING_CameraPosition]);
    m_camera.SetEyePoint(eyePt);

#ifdef WIP_PERSISTENT_CLIP_VECTOR // need to change the clip tool to recognize and use saved clip vector
    if (jsonObj.isMember(VIEW_SETTING_ClipVector))
        {
        m_clipVector = ClipVector::Create();
        JsonUtils::ClipVectorFromJson(*m_clipVector, jsonObj[VIEW_SETTING_ClipVector]);
        }
    else
        {
        m_clipVector = nullptr;
        }
#endif

    //  Anything is better than garbage
    if (m_delta.x <= DBL_EPSILON) m_delta.x = (m_delta.y + m_delta.z)/2;
    if (m_delta.y <= DBL_EPSILON) m_delta.y = (m_delta.x + m_delta.z)/2;
    if (m_delta.z <= DBL_EPSILON) m_delta.z = (m_delta.x + m_delta.y)/2;

    VerifyFocusPlane();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void PhysicalViewController::_RestoreFromSettings(JsonValueCR jsonObj)
    {
    T_Super::_RestoreFromSettings(jsonObj);

    if (jsonObj.isMember(VIEW_SETTING_Models))
        m_viewedModels.FromJson(jsonObj[VIEW_SETTING_Models]);

    m_viewFlags.From3dJson(jsonObj[VIEW_SETTING_Flags]);

    JsonUtils::DPoint3dFromJson(m_origin, jsonObj[VIEW_SETTING_Origin]);
    JsonUtils::DPoint3dFromJson(m_delta, jsonObj[VIEW_SETTING_Delta]);
    JsonUtils::RotMatrixFromJson(m_rotation, jsonObj[VIEW_SETTING_Rotation]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void CameraViewController::_SaveToSettings(JsonValueR jsonObj) const
    {
    T_Super::_SaveToSettings(jsonObj);

    jsonObj[VIEW_SETTING_IsCameraOn] = m_isCameraOn;
    jsonObj[VIEW_SETTING_CameraAngle] = m_camera.GetLensAngle();
    JsonUtils::DPoint3dToJson(jsonObj[VIEW_SETTING_CameraPosition], m_camera.GetEyePoint());
    jsonObj[VIEW_SETTING_CameraFocalLength] = m_camera.GetFocusDistance();

#ifdef WIP_PERSISTENT_CLIP_VECTOR // need to change the clip tool to recognize and use saved clip vector
    if (m_clipVector.IsValid())
        JsonUtils::ClipVectorToJson(jsonObj[VIEW_SETTING_ClipVector], *m_clipVector);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void PhysicalViewController::_SaveToSettings(JsonValueR jsonObj) const
    {
    T_Super::_SaveToSettings(jsonObj);

    m_viewFlags.To3dJson(jsonObj[VIEW_SETTING_Flags]);
    m_viewedModels.ToJson(jsonObj[VIEW_SETTING_Models]);

    JsonUtils::DPoint3dToJson(jsonObj[VIEW_SETTING_Origin], m_origin);
    JsonUtils::DPoint3dToJson(jsonObj[VIEW_SETTING_Delta], m_delta);
    JsonUtils::RotMatrixToJson(jsonObj[VIEW_SETTING_Rotation], m_rotation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysP PhysicalViewController::_GetAuxCoordinateSystem() const
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    // if we don't have an ACS when this is called, try to get one.
    if (!m_auxCoordSys.IsValid())
        IACSManager::GetManager().ReadSettings(const_cast <ViewControllerP>(this), GetDgnElement(), GetRootModelP(false));

    IAuxCoordSysP   acs = m_auxCoordSys.get();

     if (nullptr != acs && SUCCESS == acs->CompleteSetupFromViewController(this))
        return acs;
#endif

    return m_auxCoordSys.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::SetAuxCoordinateSystem(IAuxCoordSysP acs)
    {
    // if no change, return.
    if (m_auxCoordSys.get() != acs)
        m_auxCoordSys = acs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::InitDefaults()
    {
    memset(this, 0, sizeof(ViewFlags));

    text = true;
    dimensions = true;
    patterns = true;
    weights = true;
    styles = true;
    transparency = true;
    fill = true;

    textures = true;
    materials = true;
    sceneLights = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::_AdjustAspectRatio(double windowAspect, bool expandView)
    {
    windowAspect *= GetAspectRatioSkew();

    // first, make sure none of the deltas are negative
    m_delta.x = fabs(m_delta.x);
    m_delta.y = fabs(m_delta.y);
    m_delta.z = fabs(m_delta.z);

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

    DVec3d oldDelta = m_delta;

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

    DPoint3d origin;
    m_rotation.Multiply(&origin, &m_origin, 1);
    origin.x +=(oldDelta.x - m_delta.x) / 2.0;
    origin.y +=(oldDelta.y - m_delta.y) / 2.0;
    m_rotation.MultiplyTranspose(m_origin, origin);
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
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void ViewController2d::_RestoreFromSettings(JsonValueCR settings)
    {
    T_Super::_RestoreFromSettings(settings);

    JsonValueCR area2d = settings[VIEW_SETTING_Area2d];

    JsonUtils::DPoint2dFromJson(m_origin, area2d[VIEW_SETTING_Origin]);
    JsonUtils::DPoint2dFromJson(m_delta, area2d[VIEW_SETTING_Delta]);
    m_rotAngle = area2d[VIEW_SETTING_RotAngle].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void ViewController2d::_SaveToSettings(JsonValueR settings) const
    {
    T_Super::_SaveToSettings(settings);

    JsonValueR area2d = settings[VIEW_SETTING_Area2d];

    JsonUtils::DPoint2dToJson(area2d[VIEW_SETTING_Origin], m_origin);
    JsonUtils::DPoint2dToJson(area2d[VIEW_SETTING_Delta], m_delta);
    area2d[VIEW_SETTING_RotAngle] = m_rotAngle;
    }

/*---------------------------------------------------------------------------------**//**
* Show the surface normal for geometry under the cursor when snapping.
* @bsimethod                                                    Brien.Bastings  07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawLocateHitDetail(DgnViewportR vp, double aperture, HitDetailCR hit)
    {
    if (!vp.Is3dView())
        return; // Not valuable in 2d...

    if (hit.GetHitType() < HitDetailType::Snap)
        return; // Don't display unless snapped...

    if (!hit.GetGeomDetail().IsValidSurfaceHit())
        return; // AccuSnap will flash edge/segment geometry...

    IViewDrawP  output = vp.GetIViewDraw();
    ColorDef    color = ColorDef(~vp.GetHiliteColor().GetValue());// Invert hilite color for good contrast...
    DPoint3d    pt = hit.GetHitPoint();
    double      radius = (2.0 * aperture) * vp.GetPixelSizeAtPoint(&pt);
    DVec3d      normal = hit.GetGeomDetail().GetSurfaceNormal();
    RotMatrix   rMatrix = RotMatrix::From1Vector(normal, 2, true);
    DEllipse3d  ellipse = DEllipse3d::FromScaledRotMatrix(pt, rMatrix, radius, radius, 0.0, Angle::TwoPi());

    color.SetAlpha(200);
    output->SetSymbology(color, color, 1, 0);
    output->DrawArc3d(ellipse, true, true, nullptr);
    output->DrawArc3d(ellipse, false, false, nullptr);

    double      length = (0.6 * radius);
    DSegment3d  segment;

    normal.Normalize(ellipse.vector0);
    segment.point[0].SumOf(pt, normal, length);
    segment.point[1].SumOf(pt, normal, -length);
    output->DrawLineString3d(2, segment.point, nullptr);

    normal.Normalize(ellipse.vector90);
    segment.point[0].SumOf(pt, normal, length);
    segment.point[1].SumOf(pt, normal, -length);
    output->DrawLineString3d(2, segment.point, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* draw a filled and outlined circle to represent the size of the location tolerance in the current view.
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawLocateCircle(DgnViewportR vp, double aperture, DPoint3dCR pt)
    {
    IViewDrawP  output = vp.GetIViewDraw();

    output->SetToViewCoords(true);

    double      radius = (aperture / 2.0) + .5;
    DPoint3d    center;
    DEllipse3d  ellipse, ellipse2;

    vp.WorldToView(&center, &pt, 1);
    ellipse.InitFromDGNFields2d((DPoint2dCR) center, 0.0, radius, radius, 0.0, msGeomConst_2pi, 0.0);
    ellipse2.InitFromDGNFields2d((DPoint2dCR) center, 0.0, radius+1, radius+1, 0.0, msGeomConst_2pi, 0.0);

    ColorDef    white = ColorDef::White();
    ColorDef    black = ColorDef::Black();

    white.SetAlpha(165);
    output->SetSymbology(white, white, 1, 0);
    output->DrawArc2d(ellipse, true, true, 0.0, NULL);

    black.SetAlpha(100);
    output->SetSymbology(black, black, 1, 0);
    output->DrawArc2d(ellipse2, false, false, 0.0, NULL);

    white.SetAlpha(20);
    output->SetSymbology(white, white, 1, 0);
    output->DrawArc2d(ellipse, false, false, 0.0, NULL);

    output->SetToViewCoords(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawLocateCursor(DgnViewportR vp, DPoint3dCR pt, double aperture, bool isLocateCircleOn, HitDetailCP hit)
    {
    if (nullptr != hit)
        drawLocateHitDetail(vp, aperture, *hit);

    if (isLocateCircleOn)
        drawLocateCircle(vp, aperture, pt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewController::_VisitHit (HitDetailCR hit, ViewContextR context) const
    {
    DgnElementCPtr   element = hit.GetElement();
    GeometrySourceCP geom = (element.IsValid() ? element->ToGeometrySource() : nullptr);

    if (nullptr == geom)
        {
        IElemTopologyCP elemTopo = hit.GetElemTopology();
        ITransientGeometryHandlerP transientHandler = (nullptr != elemTopo ? elemTopo->_GetTransientGeometryHandler() : nullptr);

        if (nullptr == transientHandler)
            return ERROR;

        transientHandler->_DrawTransient(hit, context);
        return SUCCESS;
        }

    if (&GetDgnDb() != &element->GetDgnDb() || !IsModelViewed(element->GetModelId()))
        return SUCCESS;

    ViewContext::ContextMark mark(&context);

    // Allow element sub-class involvement for flashing sub-entities...
    if (geom->DrawHit(hit, context))
        return SUCCESS;

    return context.VisitElement(*geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawView(ViewContextR context) 
    {
    for (auto modelId : m_viewedModels)
        context.VisitDgnModel(m_dgndb.Models().GetModel(modelId).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_VisitElements(ViewContextR context)
    {
    _DrawView(context);
    }
