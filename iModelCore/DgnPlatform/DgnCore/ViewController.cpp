/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewController.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Geom/eigensys3d.fdf>
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>

static double const MAX_VDELTA = 1.0e20;

static Utf8CP VIEW_SETTING_IsCameraOn            = "isCameraOn";
static Utf8CP VIEW_SETTING_CameraAngle           = "cameraAngle";
static Utf8CP VIEW_SETTING_CameraFocalLength     = "cameraFocalLength";
static Utf8CP VIEW_SETTING_Origin                = "origin";
static Utf8CP VIEW_SETTING_Delta                 = "delta";
static Utf8CP VIEW_SETTING_CameraPosition        = "cameraPosition";
static Utf8CP VIEW_SETTING_Rotation              = "rotation";
static Utf8CP VIEW_SETTING_DisplayStyleId        = "displayStyleId";
static Utf8CP VIEW_SETTING_RotAngle              = "rotAngle";
static Utf8CP VIEW_SETTING_Levels                = "levels";
static Utf8CP VIEW_SETTING_Flags                 = "flags";
static Utf8CP VIEW_SETTING_SubLevels             = "sublevels";
static Utf8CP VIEW_SETTING_BackgroundColor       = "bgColor";
static Utf8CP VIEW_SETTING_Area2d                = "area2d";

static Utf8CP VIEWFLAG_noText                    = "noText";
static Utf8CP VIEWFLAG_noWeight                  = "noWeight";
static Utf8CP VIEWFLAG_noPattern                 = "noPattern";
static Utf8CP VIEWFLAG_txNodes                   = "txNodes";
static Utf8CP VIEWFLAG_noEdFields                = "noEdField";
static Utf8CP VIEWFLAG_grid                      = "grid";
static Utf8CP VIEWFLAG_noConstruction            = "noConstruct";
static Utf8CP VIEWFLAG_noDimensions              = "noDims";
static Utf8CP VIEWFLAG_fill                      = "fill";
static Utf8CP VIEWFLAG_acs                       = "acs";
static Utf8CP VIEWFLAG_renderMode                = "renderMode";
static Utf8CP VIEWFLAG_noTextureMap              = "noTxMap";
static Utf8CP VIEWFLAG_noTransparency            = "noTransp";
static Utf8CP VIEWFLAG_noLineStyle               = "noLineStyle";
static Utf8CP VIEWFLAG_renderEdges               = "edges";
static Utf8CP VIEWFLAG_hidden                    = "hidden";
static Utf8CP VIEWFLAG_frontClip                 = "frontClip";
static Utf8CP VIEWFLAG_backClip                  = "backClip";
static Utf8CP VIEWFLAG_noClipVolume              = "noClipVol";
static Utf8CP VIEWFLAG_shadows                   = "shadows";
static Utf8CP VIEWFLAG_hlStyle                   = "hlStyle";
static Utf8CP VIEWFLAG_noSceneLight              = "noSceneLight";
static Utf8CP VIEWFLAG_useBgColor                = "useBgColor";


static bvector<DgnViews::Factory*> s_factories;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViews::Register (Factory& factory)
    {
    UnRegister(factory);
    s_factories.push_back(&factory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViews::UnRegister (Factory& factory)
    {
    for (auto it=s_factories.begin(); it!=s_factories.end();)
        {
        if (*it == &factory)
            it = s_factories.erase(it);
        else
            ++it;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static ViewControllerPtr createViewController(DgnProjectR project, DgnViews::View const& view) 
    {
    for (auto it=s_factories.rbegin(); it!=s_factories.rend(); ++it)
        {
        auto controller = (*it)->_SupplyViewController(project, view);
        if (nullptr != controller)
            return controller;
        }

    auto viewId = view.GetId();
    switch (view.GetDgnViewType())
        {
        case DGNVIEW_TYPE_Physical:
            {
            if (view.GetViewSubType() == CameraViewController::GetViewSubType())
                return new CameraViewController(project, viewId);

            if (view.GetViewSubType() == SectioningViewController::GetViewSubType())
                return new SectioningViewController(project, viewId);

            return new PhysicalViewController(project, viewId);
            }

        case DGNVIEW_TYPE_Drawing:
            return new DrawingViewController(project, viewId);

        case DGNVIEW_TYPE_Sheet:
            if (view.GetViewSubType() == RedlineViewController::GetViewSubType())
                return RedlineViewController::Create (project, viewId);

            return new SheetViewController(project, viewId);
        }

    BeAssert (false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr DgnViews::LoadViewController(DgnViewId viewId, FillModels fillModels) const
    {
    auto view  = QueryViewById (viewId);
    if (!view.IsValid())
        return nullptr;

    auto controller = createViewController(m_project, view);
    if (!controller.IsValid())
        return nullptr;

    controller->Load();
    if (fillModels == FillModels::Yes)
        controller->FillModels();

    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::FromBaseJson(JsonValueCR val)
    {
    memset (this, 0, sizeof(*this));

    fast_text = val[VIEWFLAG_noText].asBool();
    line_wghts = !val[VIEWFLAG_noWeight].asBool();
    patterns = !val[VIEWFLAG_noPattern].asBool();
    text_nodes = val[VIEWFLAG_txNodes].asBool();
    ed_fields = !val[VIEWFLAG_noEdFields].asBool();
    grid = val[VIEWFLAG_grid].asBool();
    constructs = !val[VIEWFLAG_noConstruction].asBool();
    dimens = !val[VIEWFLAG_noDimensions].asBool();
    fill = val[VIEWFLAG_fill].asBool();
    transparency = !val[VIEWFLAG_noTransparency].asBool();
    overrideBackground = val[VIEWFLAG_useBgColor].asBool();
    inhibitLineStyles = val[VIEWFLAG_noLineStyle].asBool();
    auxDisplay = val[VIEWFLAG_acs].asBool();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::From3dJson(JsonValueCR val)
    {
    textureMaps = !val[VIEWFLAG_noTextureMap].asBool();
    renderDisplayEdges = val[VIEWFLAG_renderEdges].asBool();
    renderDisplayHidden = val[VIEWFLAG_hidden].asBool();
    noFrontClip = !val[VIEWFLAG_frontClip].asBool();
    noBackClip  = !val[VIEWFLAG_backClip].asBool();
    noClipVolume = val[VIEWFLAG_noClipVolume].asBool();
    renderDisplayShadows = val[VIEWFLAG_shadows].asBool();
    ignoreSceneLights = val[VIEWFLAG_noSceneLight].asBool();
    renderMode = val[VIEWFLAG_renderMode].asUInt();
    hiddenLineStyle = val[VIEWFLAG_hlStyle].asUInt();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::ToBaseJson (JsonValueR val) const
    {
    if (fast_text) val[VIEWFLAG_noText] = true;
    if (!line_wghts) val[VIEWFLAG_noWeight] = true;
    if (!patterns) val[VIEWFLAG_noPattern] = true;
    if (text_nodes) val[VIEWFLAG_txNodes] = true;
    if (!ed_fields) val[VIEWFLAG_noEdFields] = true;
    if (grid) val[VIEWFLAG_grid] = true;
    if (!constructs) val[VIEWFLAG_noConstruction] = true;
    if (!dimens) val[VIEWFLAG_noDimensions] = true;
    if (fill) val[VIEWFLAG_fill] = true;
    if (!transparency) val[VIEWFLAG_noTransparency] = true;
    if (overrideBackground) val[VIEWFLAG_useBgColor] = true;
    if (inhibitLineStyles) val[VIEWFLAG_noLineStyle] = true;
    if (auxDisplay) val[VIEWFLAG_acs] = true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFlags::To3dJson (JsonValueR val) const
    {
    if (!textureMaps) val[VIEWFLAG_noTextureMap] = true;
    if (renderDisplayEdges) val[VIEWFLAG_renderEdges] = true;
    if (renderDisplayHidden) val[VIEWFLAG_hidden] = true;
    if (!noFrontClip) val[VIEWFLAG_frontClip] = true;
    if (!noBackClip) val[VIEWFLAG_backClip] = true;
    if (noClipVolume) val[VIEWFLAG_noClipVolume] = true;
    if (renderDisplayShadows) val[VIEWFLAG_shadows] = true;
    if (ignoreSceneLights) val[VIEWFLAG_noSceneLight] = true;

    val[VIEWFLAG_renderMode] = renderMode;
    if (0 != hiddenLineStyle)
        val[VIEWFLAG_hlStyle] = hiddenLineStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLevels::SubLevel::Appearance ViewController::GetSubLevelAppearance(SubLevelId subLevelId) const
    {
    auto const entry = m_subLevels.find(subLevelId);
    if (entry != m_subLevels.end())
        return entry->second;

    auto subLevel = m_project.Levels().QuerySubLevelById(subLevelId);
    auto out = m_subLevels.Insert(subLevelId, subLevel.GetAppearance());
    return out.first->second;
    }

DgnModelIdSet const& ViewController::GetViewedModels() const {return m_viewedModels;}
DgnLevelIdSet const& ViewController::GetViewedLevels() const {return m_levels;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/12
+---------------+---------------+---------------+---------------+---------------+------*/
BitMaskCR  ViewController::_GetLevelDisplayMask () const
    {
    BeAssert (m_levels.m_mask.IsValid());
    return *m_levels.m_mask.GetBitMask();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::SetLevelDisplayMask (BitMaskCR bitMask)
    {
    auto mask = m_levels.m_mask.GetBitMask();
    if (!mask->IsEqual (&bitMask))
        {
        mask->SetFromBitMask (bitMask);
        _OnLevelChange (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_ChangeModelDisplay (DgnModelId, bool onOff)
    {
    // NEEDS_WORK
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_ChangeLevelDisplay (LevelId levelId, bool onOff)
    {
    BeAssert (m_levels.m_mask.IsValid());
    if (onOff != GetLevelDisplayMask ().Test (levelId.GetValue() - 1))
        {
        m_levels.m_mask.GetBitMask()->SetBit (levelId.GetValue() - 1, onOff);
        _OnLevelChange (onOff);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP ViewController::_GetTargetModel() const {return m_project.Models().GetModelById(m_targetModelId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::ViewController (DgnProjectR project, DgnViewId viewId) : m_project(project)
    {
    m_viewId = viewId;
    m_viewFlags.InitDefaults();
    m_defaultDeviceOrientation.InitIdentity();
    m_defaultDeviceOrientationValid = false;
    memset (&m_backgroundColor, 0, sizeof (m_backgroundColor));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::LoadLevels(JsonValueCR settings)
    {
    if (settings.isMember(VIEW_SETTING_Levels))
        {
        // by default, all bits are off. We now find out which ones should be on.
        Utf8String lmsUtf8 = settings[VIEW_SETTING_Levels].asString();
        m_levels.m_mask.GetBitMask()->SetFromString (lmsUtf8, 0, UINT_MAX);
        }

    // load all SubLevels (even for levels not currently on)
    for (auto const& it : DgnLevels::SubLevelIterator(m_project, LevelId()))
        m_subLevels.Insert(it.GetId(), it.GetAppearance());
    
    if (!settings.isMember(VIEW_SETTING_SubLevels))
        return;

    JsonValueCR facetJson = settings[VIEW_SETTING_SubLevels];
    for (Json::ArrayIndex i=0; i<facetJson.size(); ++i)
        {
        JsonValueCR val=facetJson[i];
        SubLevelId id(val);
        if (id.IsValid())
            OverrideSubLevel (id, DgnLevels::SubLevel::Override(val));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_RestoreFromSettings (JsonValueCR settings)
    {
    if (!settings.isMember(VIEW_SETTING_Flags))
        m_viewFlags.InitDefaults();
    else
        m_viewFlags.FromBaseJson(settings[VIEW_SETTING_Flags]);

    if (m_viewFlags.overrideBackground)
        {
        if (!settings.isMember(VIEW_SETTING_BackgroundColor))
            m_viewFlags.overrideBackground = false;
        else
            {
            IntColorDef bgColor(settings[VIEW_SETTING_BackgroundColor].asUInt());
            m_backgroundColor = bgColor.m_rgb;
            }
        }

    LoadLevels(settings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::Load()
    {
    DgnViews::View entry = m_project.Views().QueryViewById(m_viewId);
    if (!entry.IsValid())
        {
        BeAssert (false);
        return  BE_SQLITE_ERROR;
        }

    m_viewedModels.clear();
    m_baseModelId = m_targetModelId = entry.GetBaseModelId();
    m_viewedModels.insert (m_baseModelId);

    DgnModelSelectorId selectorId = entry.GetDgnModelSelectorId();
    if (selectorId.IsValid()) // no selector means just show base model
        {
        DgnModelSelection selections (m_project, entry.GetDgnModelSelectorId());
        DbResult rc = selections.Load();
        if (BE_SQLITE_OK != rc)
            return  rc;

        for (DgnModelId modelId : selections)
            m_viewedModels.insert(modelId);
        }

    Utf8String settingsStr;
    //  The QueryModel calls GetModelById in the QueryModel thread.  produces a thread race condition if it calls QueryModelById and 
    DbResult  rc = GetDgnProject().Views().QueryProperty (settingsStr, GetViewId(), DgnViewProperty::Settings());
    if (BE_SQLITE_ROW != rc)
        return rc;

    Json::Value json;
    Json::Reader::Parse (settingsStr, json);
    _RestoreFromSettings(json);

    //  The QueryModel calls GetModelById in the QueryModel thread.  produces a thread race condition if it calls QueryModelById and 
    //  the model is not already loaded.
    for (auto&id : GetViewedModels())
        m_project.Models().GetModelById(id);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_SaveToSettings (JsonValueR settings) const
    {
    m_viewFlags.ToBaseJson (settings[VIEW_SETTING_Flags]);

    // only save background color if viewflag is on
    if (m_viewFlags.overrideBackground)
        settings[VIEW_SETTING_BackgroundColor] = IntColorDef(m_backgroundColor).AsUInt32();

    Utf8String levelString;
    m_levels.m_mask.GetBitMask()->ToString (levelString, 0);
    settings[VIEW_SETTING_Levels] = levelString;

    if (m_subLevelOverrides.empty())
        return;

    JsonValueR ovrJson = settings[VIEW_SETTING_SubLevels];
    int i=0;
    for (auto const& it : m_subLevelOverrides)
        {
        it.first.ToJson(ovrJson[i]);
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

    return m_project.Views().SavePropertyString(m_viewId, DgnViewProperty::Settings(), Json::FastWriter::ToString(settings));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::SaveAs (Utf8CP newName)
    {
    _GetLevelDisplayMask(); // make sure its loaded

    DgnViews::View newRow(m_project.Views().QueryViewById(m_viewId));
    newRow.SetName(newName);

    DbResult rc = m_project.Views().InsertView(newRow);
    if (BE_SQLITE_OK != rc)
        return rc;

    m_viewId = newRow.GetId();
    rc = Save();

    if (BE_SQLITE_OK == rc)
        m_project.SaveSettings();

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewController::SaveTo(Utf8CP newName, DgnViewId& newId)
    {
    AutoRestore<DgnViewId> saveId (&m_viewId);

    DbResult rc = SaveAs(newName);
    newId = (BE_SQLITE_OK == rc) ? m_viewId : DgnViewId();
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d ViewController::_GetProjectExtents() const
    {
    return m_project.Units().GetProjectExtents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawElement(ViewContextR context, ElementHandleCR elIter)
    {
    context.CookElemDisplayParams (elIter);
    context.ActivateOverrideMatSymb ();

    elIter.GetDisplayHandler()->Draw (elIter, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawElementFiltered(ViewContextR context, ElementHandleCR el, DPoint3dCP pts, double size)
    {
    el.GetDisplayHandler()->DrawFiltered (el, context, pts, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::ReloadSubLevel(SubLevelId id)
    {
    auto unmodified = m_project.Levels().QuerySubLevelById(id);
    auto const& result = m_subLevels.Insert(id, unmodified.GetAppearance());

    if (!result.second)
        result.first->second = unmodified.GetAppearance(); // we already had this SubLevel; change it.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::OverrideSubLevel(SubLevelId id, DgnLevels::SubLevel::Override const& ovr)
    {
    if (!id.IsValid())
        return;

    auto result = m_subLevelOverrides.Insert(id, ovr);
    if (!result.second)
        {
        result.first->second = ovr; // we already had this override; change it.
        ReloadSubLevel(id); // To ensure none of the previous overrides are still active, we reload the original SubLevel
        }

    // now apply this override to the unmodified SubLevel appearance
    auto const& it = m_subLevels.find(id);
    if (it != m_subLevels.end())
        ovr.ApplyTo(it->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::DropSubLevelOverride(SubLevelId id)
    {
    m_subLevelOverrides.erase(id);
    ReloadSubLevel(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewController::_IsPointAdjustmentRequired (ViewportR vp) const {return vp.Is3dView ();}
bool ViewController::_IsSnapAdjustmentRequired  (ViewportR vp, bool snapLockEnabled) const {return snapLockEnabled && vp.Is3dView ();}
bool ViewController::_IsContextRotationRequired (ViewportR vp, bool contextLockEnabled) const {return contextLockEnabled;}

/////////////////////////////////////////////////////////////////////////////////////
///
/// Standard Views
///
/////////////////////////////////////////////////////////////////////////////////////

static bool equalOne (double r1) {return BeNumerical::Compare (r1, 1.0) == 0;}
static bool equalMinusOne (double r1) {return BeNumerical::Compare (r1, -1.0) == 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   03/89
+---------------+---------------+---------------+---------------+---------------+------*/
StandardView ViewController::IsStandardViewRotation (RotMatrixCR rMatrix, bool check3d)
    {
    if (check3d)
        {
        // If a matrix is known apriori to be a pure rotation ....
        //   a) A one or minus one implies the remainder of the row and column are zero.
        //   b) Once ones are found, the third row and column are known by cross product rules.
        // Hence just two one or minus one entries fully identifies primary flat views.
        // Dot products with two vectors from known iso views is also complete.
        if (equalOne (rMatrix.form3d[0][0]))
            {
            if (equalOne (rMatrix.form3d[1][1]))
                return StandardView::Top;
            if (equalMinusOne (rMatrix.form3d[1][1]))
                return StandardView::Bottom;
            if (equalOne (rMatrix.form3d[1][2]))
                return StandardView::Front;
            }
        else if (equalOne (rMatrix.form3d[1][2]))
            {
            if (equalOne (rMatrix.form3d[0][1]))
                return StandardView::Right;
            if (equalMinusOne (rMatrix.form3d[0][1]))
                return StandardView::Left;
            if (equalMinusOne (rMatrix.form3d[0][0]))
                return StandardView::Back;
            }
        else                    /* Check For (left) IsoMetric */
            {
            RotMatrix  isoMatrix;
            bsiRotMatrix_getStandardRotation(&isoMatrix, static_cast<int>(StandardView::Iso));

            if (equalOne (bsiDVec3d_dotProduct ((DVec3d*)isoMatrix.form3d[0], (DVec3d*)rMatrix.form3d[0])) &&
                equalOne (bsiDVec3d_dotProduct ((DVec3d*)isoMatrix.form3d[1], (DVec3d*)rMatrix.form3d[1])))
                return StandardView::Iso;

            bsiRotMatrix_getStandardRotation(&isoMatrix, static_cast<int>(StandardView::RightIso));
            if (equalOne (bsiDVec3d_dotProduct ((DVec3d*)isoMatrix.form3d[0], (DVec3d*)rMatrix.form3d[0])) &&
                equalOne (bsiDVec3d_dotProduct ((DVec3d*)isoMatrix.form3d[1], (DVec3d*)rMatrix.form3d[1])))
                return StandardView::RightIso;
            }
        }
    else
        {
        if (equalOne (rMatrix.form3d[0][0]) && equalOne (rMatrix.form3d[1][1]))
            return StandardView::Top;
        }

    return StandardView::NotStandard;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewController::GetStandardViewName (WStringR name, StandardView viewID)
    {
    name = L"";
    if (viewID < StandardView::Top || viewID > StandardView::RightIso)
        return ERROR;

    name = DgnCoreL10N::GetStringW((DgnCoreL10N::Number)(static_cast<int>(viewID) - 1 + DgnCoreL10N::VIEWTITLE_MessageID_Top));
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewController::GetStandardViewByName (RotMatrix* rotP, StandardView* standardIdP, WCharCP viewName)
    {
    WString tName;
    for (int i = static_cast<int>(StandardView::Top); SUCCESS == GetStandardViewName(tName, (StandardView)i); ++i)
        {
        if (0 == BeStringUtilities::Wcsicmp (viewName, tName.c_str()))
            {
            if (NULL != rotP)
                bsiRotMatrix_getStandardRotation (rotP, i);

            if (NULL != standardIdP)
                *standardIdP = (StandardView) i;

            return SUCCESS;
            }
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewController::SetStandardViewRotation (StandardView standardView)
    {
    RotMatrix rMatrix;
    if (!bsiRotMatrix_getStandardRotation(&rMatrix, static_cast<int>(standardView)))
        return  ERROR;

    SetRotation (rMatrix);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Search the (8) standard view matrices for one that is close to given matrix.
* @bsimethod                                                    EarlinLutz      05/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool findNearbyStandardViewMatrix (RotMatrixR rMatrix)
    {
    static double const s_viewMatrixTolerance = 1.0e-7;
    RotMatrix   test;

    // Standard views are numbered from 1 ....
    for (int i = 1; bsiRotMatrix_getStandardRotation (&test, i); i++)
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
ViewFrustumStatus ViewController::SetupFromFrustum (Frustum const& inFrustum) 
    {
    Frustum frustum=inFrustum;
    Viewport::FixFrustumOrder (frustum);

    return _SetupFromFrustum(frustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFrustumStatus ViewController::_SetupFromFrustum (Frustum const& frustum)
    {
    DPoint3dCP frustPts = frustum.GetPts();
    DPoint3d viewOrg = frustPts[NPC_000];

    // frustumX, frustumY, frustumZ are vectors along edges of the frustum. They are NOT unit vectors.
    // X and Y should be perpendicular, and Z should be right handed.
    DVec3d frustumX, frustumY, frustumZ;
    frustumX.DifferenceOf (frustPts[NPC_100], viewOrg);
    frustumY.DifferenceOf (frustPts[NPC_010], viewOrg);
    frustumZ.DifferenceOf (frustPts[NPC_001], viewOrg);

    RotMatrix   frustMatrix;
    frustMatrix.InitFromColumnVectors (frustumX, frustumY, frustumZ);
    if (!frustMatrix.SquareAndNormalizeColumns (frustMatrix, 0, 1))
        return VIEWFRUST_STATUS_InvalidWindow;

    findNearbyStandardViewMatrix (frustMatrix);

    DVec3d xDir, yDir, zDir;
    frustMatrix.GetColumns (xDir, yDir, zDir);

    // set up view Rotation matrix as rows of frustum matrix.
    RotMatrix viewRot;
    viewRot.InverseOf (frustMatrix);

    // Left handed frustum?
    double zSize = zDir.DotProduct (frustumZ);
    if (zSize < 0.0)
        return VIEWFRUST_STATUS_InvalidWindow;

    DPoint3d viewDiagRoot;
    viewDiagRoot.SumOf(xDir, xDir.DotProduct(frustumX), yDir, yDir.DotProduct(frustumY));  // vectors on the back plane
    viewDiagRoot.SumOf(viewDiagRoot, zDir, zSize);       // add in z vector perpendicular to x,y

    // use center of frustum and view diagonal for origin. Original frustum may not have been orgthogonal
    viewOrg.SumOf(frustum.GetCenter(), viewDiagRoot, -0.5);

    // delta is in view coordinates
    DVec3d viewDelta;
    viewRot.Multiply (viewDelta, viewDiagRoot);

    ViewFrustumStatus validSize = Viewport::ValidateWindowSize (viewDelta, false);
    if (validSize != VIEWFRUST_STATUS_SUCCESS)
        return validSize;

    SetOrigin (viewOrg);
    SetDelta (viewDelta);
    SetRotation (viewRot);
    return VIEWFRUST_STATUS_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFrustumStatus CameraViewController::_SetupFromFrustum (Frustum const& frustum)
    {
    auto stat = T_Super::_SetupFromFrustum(frustum);
    if (VIEWFRUST_STATUS_SUCCESS)
        return stat;

    DPoint3dCP frustPts = frustum.GetPts();

    // use comparison of back, front plane X sizes to indicate camera or flat view ...
    double xBack  = frustPts[NPC_000].Distance(frustPts[NPC_100]);
    double xFront = frustPts[NPC_001].Distance(frustPts[NPC_101]);

    static double const s_flatViewFractionTolerance = 1.0e-6;
    if (xFront > xBack * (1.0 + s_flatViewFractionTolerance))
        return VIEWFRUST_STATUS_InvalidWindow;

    // see if the frustum is tapered, and if so, set up camera eyepoint and adjust viewOrg and delta.
    double compression = xFront / xBack;
    if (!Allow3dManipulations() || (compression >= (1.0 - s_flatViewFractionTolerance)))
        {
        SetCameraOn(false);
        return VIEWFRUST_STATUS_SUCCESS;
        }

    DPoint3d viewOrg     = frustPts[NPC_000];
    DVec3d viewDelta     = GetDelta();
    DVec3d zDir          = GetZVector();
    DVec3d frustumZ      = DVec3d::FromStartEnd(viewOrg, frustPts[NPC_001]);
    DVec3d frustOrgToEye = DVec3d::FromScale(frustumZ, 1.0 / (1.0 - compression));
    DPoint3d eyePoint    = DPoint3d::FromSumOf(viewOrg, frustOrgToEye);

    double backDistance  = frustOrgToEye.DotProduct(zDir);         // distance from eye to back plane of frustum
    double focusDistance = backDistance - (viewDelta.z / 2.0);
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
    return VIEWFRUST_STATUS_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::LookAtVolume (DRange3dCR volume, double const* aspect, MarginPercent const* margin, bool expandClippingPlanes)
    {
    DPoint3d rangebox[8];
    volume.Get8Corners (rangebox);
    GetRotation().Multiply (rangebox, rangebox, 8);

    DRange3d viewAlignedVolume;
    viewAlignedVolume.InitFrom (rangebox, 8);

    return LookAtViewAlignedVolume (viewAlignedVolume, aspect, margin, expandClippingPlanes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::LookAtViewAlignedVolume (DRange3dCR volume, double const* aspect, MarginPercent const* margin, bool expandClippingPlanes)
    {
    DPoint3d    oldDelta = GetDelta();
    DPoint3d    oldOrg   = GetOrigin();
    RotMatrix   viewRot  = GetRotation();

    DPoint3d  newOrigin = volume.low;
    DVec3d    newDelta;
    newDelta.DifferenceOf (volume.high, volume.low);

    double minimumDepth = 1000.0 / 1.01;
    if (newDelta.z < minimumDepth)
        {
        newOrigin.z -= (minimumDepth - newDelta.z)/2.0;
        newDelta.z = minimumDepth;
        }

    PhysicalViewControllerP physView = (PhysicalViewControllerP) _ToPhysicalView();
    CameraViewControllerP cameraView = (CameraViewControllerP) _ToCameraView();
    DPoint3d origNewDelta = newDelta;

    bool isCameraOn = cameraView && cameraView->IsCameraOn();
    if (isCameraOn)
        {
        // If the camera is on, the only way to guarantee we can see the entire volume is to set delta at the front plane, not focus plane.
        // That generally causes the view to be too large (objects in it are too small), since we can't tell whether the objects are at
        // the front or back of the view. For this reason, don't attempt to add any "margin" to camera views.
        }
    else if (NULL != margin)
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
        double diag = newDelta.magnitudeXY();
        if (diag > newDelta.z)
            newDelta.z = diag;
        }

    Viewport::ValidateWindowSize (newDelta, true);

    SetDelta(newDelta);
    if (aspect)
        AdjustAspectRatio (*aspect, true);

    newDelta = GetDelta();

    newOrigin.x -= (newDelta.x - origNewDelta.x) / 2.0;
    newOrigin.y -= (newDelta.y - origNewDelta.y) / 2.0;
    newOrigin.z -= (newDelta.z - origNewDelta.z) / 2.0;

    // if they don't want the clipping planes to change, set them back to where they were
    if (NULL != physView && !expandClippingPlanes && Allow3dManipulations())
        {
        viewRot.Multiply(oldOrg);
        newOrigin.z = oldOrg.z;

        DVec3d delta = GetDelta();
        delta.z = oldDelta.z;
        SetDelta (delta);
        }

    DPoint3d newOrgView;
    viewRot.MultiplyTranspose(&newOrgView, &newOrigin, 1);
    SetOrigin (newOrgView);

    if (nullptr == cameraView)
        return;

    cameraView->GetCameraR().ValidateLens();
    // move the camera back so the entire x,y range is visible at front plane
    double frontDist = std::max(newDelta.x, newDelta.y) / (2.0*tan(cameraView->GetLensAngle()/2.0));
    double backDist = frontDist + newDelta.z;

    cameraView->SetFocusDistance(frontDist); // do this even if the camera isn't currently on.
    cameraView->CenterEyePoint(&backDist);   // "

    if (isCameraOn)
        cameraView->CenterFocusDistance();   // changes delta/origin - only do it if the camera is on
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_FillModels() 
    {
    for (auto modelId : m_viewedModels)
        {
        auto model = m_project.Models().GetModelById(modelId);
        if (model)
            model->FillModel();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalViewController::PhysicalViewController (DgnProjectR project, DgnViewId viewId) 
    : ViewController (project, viewId)
    {
    // not valid, but better than random
    m_origin.Zero();
    m_delta.Zero();
    m_rotation.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::_OnTransform (TransformCR trans)
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
void CameraViewController::_OnTransform (TransformCR trans)
    {
    T_Super::_OnTransform (trans);
    DPoint3d eye = GetEyePoint();
    trans.Multiply(eye);
    SetEyePoint(eye);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::TransformBy (TransformCR trans)
    {
    _OnTransform (trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::_SetDelta (DVec3dCR delta)     {m_delta = delta;}
void PhysicalViewController::_SetOrigin (DPoint3dCR origin) {m_origin = origin;}
void PhysicalViewController::_SetRotation (RotMatrixCR rot) {m_rotation = rot;}
DPoint3d PhysicalViewController::_GetOrigin() const {return m_origin;}
DVec3d PhysicalViewController::_GetDelta() const {return m_delta;}
RotMatrix PhysicalViewController::_GetRotation() const {return m_rotation;}

DgnStyleId PhysicalViewController::GetDisplayStyleId() const {return m_displayStyleId;}
void PhysicalViewController::SetDisplayStyle(DgnStyleId id) {m_displayStyleId = id;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PhysicalViewController::SetTargetModel (DgnModelP target)
    {
    if (!m_viewedModels.IsModelOn(target->GetModelId()))
        return  ERROR;

    m_targetModelId = target->GetModelId();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
SectioningViewControllerPtr SectionDrawingViewController::GetSectioningViewController() const
    {
    if (m_sectionView.IsValid())
        return m_sectionView;

    SectionDrawingModel* drawing = GetSectionDrawing();
    if (drawing == NULL)
        return NULL;

    auto sectionViewId = GetDgnProject().ViewGeneratedDrawings().QuerySourceView (drawing->GetModelId()); 
    return dynamic_cast<SectioningViewController*>(GetDgnProject().Views().LoadViewController(sectionViewId, DgnViews::FillModels::Yes).get());
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
Transform SectionDrawingViewController::GetFlatteningMatrix (double zdelta) const
    {
    auto drawing = GetSectionDrawing();
    if (drawing == NULL)
        return Transform::FromIdentity();

    return drawing->GetFlatteningMatrix (zdelta);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
Transform SectionDrawingViewController::GetFlatteningMatrixIf2D (ViewContextR context, double zdelta) const
    {
    if (!context.GetViewport() || context.GetViewport()->Is3dView())
        return Transform::FromIdentity();

    return GetFlatteningMatrix (zdelta);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
Transform SectionDrawingViewController::GetTransformToWorld() const
    {
    auto drawing = GetSectionDrawing();
    if (drawing == NULL)
        return Transform::FromIdentity();

    return drawing->GetTransformToWorld();
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
StatusInt SectionDrawingViewController::_VisitPath (DisplayPathCP displayPath, void* arg, ViewContextR context) const
    {
    context.PushTransform (GetFlatteningMatrixIf2D(context));
    StatusInt status = T_Super::_VisitPath (displayPath, arg, context);
    context.PopTransformClip();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/14
//--------------+------------------------------------------------------------------------
void SectionDrawingViewController::_DrawView (ViewContextR context) 
    {
    context.PushTransform (GetFlatteningMatrixIf2D(context));
    T_Super::_DrawView (context);
    context.PopTransformClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SectionDrawingViewController::_DrawElement(ViewContextR context, ElementHandleCR elIter)
    {
#if defined (NEEDS_WORK_VIEW_CONTROLLER)
    if (context.GetViewport() != NULL)
        {
        auto hyper = context.GetViewport()->GetViewControllerP()->ToHypermodelingViewController();
        if (hyper != NULL && !hyper->ShouldDrawAnnotations() && !ProxyDisplayHandlerUtils::IsProxyDisplayHandler (elIter.GetHandler()))
            return;
        }
#endif

    T_Super::_DrawElement (context, elIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
CameraViewController::CameraViewController (DgnProjectR project, DgnViewId viewId) 
    : PhysicalViewController (project, viewId) 
    {
    // not valid, but better than random
    m_isCameraOn = false;
    memset (&m_camera, 0, sizeof (m_camera));
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
    BeAssert (IsCameraValid());

    DVec3d delta = GetDelta();
    DPoint3d eyePoint; 
    eyePoint.Scale (delta, 0.5);
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
* @bsimethod                                    Ray.Bentley                     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleCP PhysicalViewController::GetDisplayStyleCP() const
    {
#if defined (NEEDS_WORK_DGNITEM)
    DgnModelP targetModel = GetTargetModel();
    if (NULL == targetModel)
        {
        BeAssert (false);
        return NULL;
        }

    DgnProjectP targetFile = &targetModel->GetDgnProject();
    if (NULL == targetFile)
        {
        BeAssert (false);
        return NULL;
        }

    return targetFile->Styles().DisplayStyles().QueryById (GetDisplayStyleId());
#endif
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
double PhysicalViewController::CalculateMaxDepth (DVec3dCR delta, DVec3dCR zVec)
    {
    // We are going to limit maximum depth to a value that will avoid subtractive cancellation
    // errors on the inverse frustum matrix. - These values will occur when the Z'th row values
    // are very small in comparison to the X-Y values.  If the X-Y values are exactly zero then
    // no error is possible and we'll arbitrarily limit to 1.0E8.
    // This change made to resolve TR# 271876.   RayBentley   04/28/2009.

    static double   s_depthRatioLimit       = 1.0E8;          // Limit for depth Ratio.
    static double   s_maxTransformRowRatio  = 1.0E5;

    double minXYComponent = std::min (fabs(zVec.x), fabs(zVec.y));
    double maxDepthRatio = (0.0 == minXYComponent) ? s_depthRatioLimit : std::min ((s_maxTransformRowRatio / minXYComponent), s_depthRatioLimit);

    return  std::max (delta.x, delta.y) * maxDepthRatio;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     12/13
//---------------------------------------------------------------------------------------
static bool convertToWorldPointWithStatus (DPoint3dR worldPoint, GeoLocationEventStatus& status, DgnUnits const& units, GeoPointCR location)
    {
    if (SUCCESS != units.ConvertToWorldPoint (worldPoint, location))
        {
        if (!units.CanConvertBetweenGeoAndWorld())
            status = GeoLocationEventStatus::NoGeoCoordinateSystem;
        else if (!units.IsGeoPointWithinGeoCoordinateSystemWorkingArea (location))
            status = GeoLocationEventStatus::PointOutsideGeoCoordinateSystem;
        else
            {
            BeAssert (false);
            status = GeoLocationEventStatus::EventIgnored;
            }

        return false;
        }

    status = GeoLocationEventStatus::EventHandled;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool CameraViewController::_OnGeoLocationEvent (GeoLocationEventStatus& status, GeoPointCR location)
    {
    if (!IsCameraOn())
        return T_Super::_OnGeoLocationEvent (status, location);

    DPoint3d worldPoint;
    if (!convertToWorldPointWithStatus (worldPoint, status, m_project.Units(), location))
        return false;

    worldPoint.z = GetEyePoint().z;
    DPoint3d targetPoint = GetTargetPoint();
    targetPoint.z = worldPoint.z;
    DVec3d newViewZ;
    newViewZ.DifferenceOf (targetPoint, worldPoint);
    newViewZ.Normalize();
    targetPoint.SumOf (worldPoint, newViewZ, GetFocusDistance());
    LookAt (worldPoint, targetPoint, DVec3d::From (0.0, 0.0, 1.0));
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool PhysicalViewController::_OnGeoLocationEvent (GeoLocationEventStatus& status, GeoPointCR location)
    {
    DPoint3d worldPoint;
    if (!convertToWorldPointWithStatus (worldPoint, status, m_project.Units(), location))
        return false;

    // If there's no perspective, just center the current location in the view.
    RotMatrix viewInverse;
    viewInverse.InverseOf (GetRotation());

    DPoint3d delta = GetDelta();
    delta.Scale (0.5);
    viewInverse.Multiply (delta);

    worldPoint.DifferenceOf (worldPoint, delta);
    SetOrigin (worldPoint);

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
bool ViewController::OnOrientationEvent (RotMatrixCR matrix, OrientationMode mode, UiOrientation ui)
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

    return _OnOrientationEvent (matrix, mode, ui);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool PhysicalViewController::ViewVectorsFromOrientation (DVec3dR forward, DVec3dR up, RotMatrixCR orientation, OrientationMode mode, UiOrientation ui)
    {
    double azimuthCorrection = 0.0;
    DVec3d currForward = GetZVector();

    orientation.GetColumn (forward, 2);
    switch (mode)
        {
        case OrientationMode::CompassHeading:
            azimuthCorrection = msGeomConst_radiansPerDegree * (90.0 + m_project.Units().GetAzimuth());
            forward.RotateXY (azimuthCorrection);
            break;
        case OrientationMode::IgnoreHeading:
            forward.x = currForward.x;
            forward.y = currForward.y;
            break;
        case OrientationMode::RelativeHeading:
            forward.x = s_defaultForward.x + (orientation.form3d[0][2] - m_defaultDeviceOrientation.form3d[0][2]);
            forward.y = s_defaultForward.y + (orientation.form3d[1][2] - m_defaultDeviceOrientation.form3d[1][2]);
            break;
        }
    forward.Normalize();

    // low pass filter
    if (fabs (currForward.AngleTo (forward)) < 0.025)
        return false;

    // Since roll isn't desired for any of the standard orientation modes, just ignore the device's real up vector
    // and compute a normalized one.
    up = forward;
    up.z += 0.1;
    up.Normalize();

    DVec3d right;
    right.NormalizedCrossProduct (up, forward);
    up.NormalizedCrossProduct (forward, right);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool PhysicalViewController::_OnOrientationEvent (RotMatrixCR orientation, OrientationMode mode, UiOrientation ui)
    {
    DVec3d forward, up;
    if (!ViewVectorsFromOrientation (forward, up, orientation, mode, ui))
        return false;

    // No camera, have to manually define origin, etc.
    RotMatrix viewMatrix = GetRotation();
    RotMatrix viewInverse;
    viewInverse.InverseOf (viewMatrix);

    DVec3d delta = GetDelta();
    delta.Scale (0.5);
    DPoint3d worldDelta = delta;
    viewInverse.Multiply (worldDelta);

    // This is the point we want to rotate about.
    DPoint3d eyePoint;
    eyePoint.SumOf (GetOrigin(), worldDelta);

    DVec3d xVector;
    xVector.CrossProduct (forward, up);
    viewMatrix.SetRow (xVector, 0);
    viewMatrix.SetRow (up, 1);
    viewMatrix.SetRow (forward, 2);
    SetRotation (viewMatrix);

    // Now that we have the new rotation, we can work backward from the eye point to get the new origin.
    viewInverse.InverseOf (viewMatrix);
    worldDelta = delta;
    viewInverse.Multiply (worldDelta);
    DPoint3d newOrigin;
    newOrigin.DifferenceOf (eyePoint, worldDelta);
    SetOrigin (newOrigin);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool CameraViewController::_OnOrientationEvent (RotMatrixCR orientation, OrientationMode mode, UiOrientation ui)
    {
    if (!IsCameraOn())
        return T_Super::_OnOrientationEvent (orientation, mode, ui);

    DVec3d forward, up;
    if (!ViewVectorsFromOrientation (forward, up, orientation, mode, ui))
        return false;

    DPoint3d eyePoint = GetEyePoint(), newTarget;
    newTarget.SumOf (eyePoint, forward, -GetFocusDistance());

    LookAt (eyePoint, newTarget, up);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
bool DrawingViewController::_OnGeoLocationEvent (GeoLocationEventStatus& status, GeoPointCR location)
    {
    DPoint3d worldPoint;
    if (!convertToWorldPointWithStatus (worldPoint, status, m_project.Units(), location))
        return false;

    RotMatrix viewInverse;
    viewInverse.InverseOf (GetRotation());

    DPoint3d delta = GetDelta();
    delta.Scale (0.5);
    viewInverse.Multiply (delta);

    worldPoint.DifferenceOf (worldPoint, delta);
    SetOrigin (worldPoint);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* See diagram in viewController.h
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFrustumStatus CameraViewController::LookAt (DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVec,
                                                  DVec2dCP extentsIn, double const* frontDistIn, double const* backDistIn)
    {
    DVec3d yVec = upVec;
    if (yVec.Normalize() <= mgds_fc_epsilon) // up vector zero length?
        return VIEWFRUST_STATUS_InvalidUpVector;

    DVec3d zVec; // z defined by direction from eye to target
    zVec.DifferenceOf (eyePoint, targetPoint);

    double focusDist = zVec.Normalize(); // set focus at target point
    if (focusDist <= 1.0)                // eye and target are too close together
        return VIEWFRUST_STATUS_InvalidTargetPoint;

    DVec3d xVec; // x is the normal to the Up-Z plane
    if (xVec.NormalizedCrossProduct(yVec, zVec) <= mgds_fc_epsilon)
        return VIEWFRUST_STATUS_InvalidUpVector;    // up is parallel to z

    if (yVec.NormalizedCrossProduct(zVec, xVec) <= mgds_fc_epsilon) // make sure up vector is perpendicular to z vector
        return VIEWFRUST_STATUS_InvalidUpVector;

    // we now have rows of the rotation matrix
    RotMatrix rotation = RotMatrix::FromRowVectors (xVec, yVec, zVec);

    double backDist  = backDistIn  ? *backDistIn  : GetBackDistance();
    double frontDist = frontDistIn ? *frontDistIn : GetFrontDistance();
    DVec3d delta     = extentsIn   ? DVec3d::From(fabs(extentsIn->x),fabs(extentsIn->y),GetDelta().z) : GetDelta();

    frontDist = std::max(frontDist, 1.0);
    backDist  = std::max(backDist, focusDist+1.0);

    BeAssert (backDist > frontDist);
    delta.z = (backDist - frontDist);

    DVec3d frontDelta = DVec3d::FromScale(delta, frontDist/focusDist);
    ViewFrustumStatus stat = Viewport::ValidateWindowSize (frontDelta, false); // validate window size on front (smallest) plane
    if (VIEWFRUST_STATUS_SUCCESS != stat)
        return  stat;

    if (delta.z > CalculateMaxDepth(delta, zVec)) // make sure we're not zoomed in too far
        return VIEWFRUST_STATUS_MaxDisplayDepth;

    // The origin is defined as the lower left of the view rectangle on the focus plane, projected to the back plane.
    // Start at eye point, and move to center of back plane, then move left half of width. and down half of height
    DPoint3d origin;
    origin.SumOf (eyePoint, zVec, -backDist, xVec, -0.5*delta.x, yVec, -0.5*delta.y);

    SetCameraOn(true);
    SetEyePoint(eyePoint);
    SetRotation(rotation);
    SetFocusDistance(focusDist);
    SetOrigin(origin);
    SetDelta(delta);
    CalculateLensAngle();

    return VIEWFRUST_STATUS_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFrustumStatus CameraViewController::LookAtUsingLensAngle (DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVec,
                                                  double lens, double const* frontDist, double const* backDist)
    {
    DVec3d zVec; // z defined by direction from eye to target
    zVec.DifferenceOf (eyePoint, targetPoint);

    double focusDist = zVec.Normalize(); // set focus at target point
    if (focusDist <= 1.0)                // eye and target are too close together
        return VIEWFRUST_STATUS_InvalidTargetPoint;

    if (lens < .0001 || lens > msGeomConst_pi)
        return VIEWFRUST_STATUS_InvalidLens;

    double extent = 2.0 * tan(lens/2.0) * focusDist;

    DVec2d delta  = DVec2d::From (GetDelta().x, GetDelta().y);
    double longAxis = std::max(delta.x, delta.y);
    delta.Scale (extent/longAxis);

    return LookAt(eyePoint, targetPoint, upVec, &delta, frontDist, backDist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFrustumStatus CameraViewController::MoveCameraWorld (DVec3dCR distance)
    {
    DPoint3d newTarget, newEyePt;
    newTarget.SumOf (GetTargetPoint(), distance);
    newEyePt.SumOf (GetEyePoint(), distance);
    return LookAt(newEyePt, newTarget, GetYVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFrustumStatus CameraViewController::MoveCameraLocal (DVec3dCR distanceLocal)
    {
    DVec3d distWorld = distanceLocal;
    GetRotation().MultiplyTranspose(distWorld);
    return MoveCameraWorld(distWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFrustumStatus CameraViewController::RotateCameraWorld (double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
    {
    DPoint3d about = aboutPointIn ? *aboutPointIn : GetEyePoint();
    RotMatrix rotation = RotMatrix::FromVectorAndRotationAngle(axis, radAngle);
    Transform trans    = Transform::FromMatrixAndFixedPoint(rotation, about);

    DPoint3d newTarget = GetTargetPoint();
    trans.Multiply (newTarget);
    DVec3d upVec = GetYVector();
    rotation.Multiply(upVec);

    return LookAt(GetEyePoint(), newTarget, upVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFrustumStatus CameraViewController::RotateCameraLocal (double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
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
    GetRotation().Multiply (eyeOrg); // orient to view
    return eyeOrg.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d PhysicalViewController::_ShowTxnSummary(TxnSummaryCR summary) 
    {
    DRange3d physicalRange = summary.GetPhysicalRange();

    // any modify that did not affect the range of the element will not have been included in the summary range
    // because there was no change to the range values in the drawing/physical element table. We need to union the range
    // of the current element (from the entry in the element data table) so its range will be healed
    for (auto el : summary.GetElementModifies())
        {
        DRange3dCP elRange = el->CheckIndexRange();
        if (NULL == elRange)
            continue;

        if (el->GetDgnModelP()->Is3d())
            physicalRange.Extend(*elRange);
        }

    return physicalRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d ViewController2d::_ShowTxnSummary(TxnSummaryCR summary) 
    {
    DRange2d drawingRange = summary.GetDrawingRange();

    for (auto el : summary.GetElementModifies())
        {
        DRange3dCP elRange = el->CheckIndexRange();
        if (NULL == elRange)
            continue;

        if (!el->GetDgnModelP()->Is3d())
            {
            drawingRange.Extend(elRange->low);
            drawingRange.Extend(elRange->high);
            }
        }

    return DRange3d::From(&drawingRange.low, 2, 0.0);
    }

double   DrawingViewController::_GetAspectRatioSkew() const {return 1.0;}
DPoint3d ViewController2d::_GetOrigin() const {return DPoint3d::From (m_origin.x, m_origin.y);}
void     ViewController2d::_SetDelta(DVec3dCR delta) {m_delta.x = delta.x; m_delta.y = delta.y;}
void     ViewController2d::_SetOrigin(DPoint3dCR origin) {m_origin.x = origin.x; m_origin.y = origin.y;}
void     ViewController2d::_SetRotation(RotMatrixCR rot) {DVec3d xColumn; rot.GetColumn (xColumn, 0); m_rotAngle = atan2 (xColumn.y, xColumn.x);}
DVec3d   ViewController2d::_GetDelta() const {return DVec3d::From (m_delta.x, m_delta.y);}
RotMatrix ViewController2d::_GetRotation() const {return RotMatrix::FromAxisAndRotationAngle(2, m_rotAngle);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ViewController::GetCenter() const
    {
    DPoint3d delta;
    GetRotation().MultiplyTranspose (delta, GetDelta());

    DPoint3d center;
    center.SumOf (GetOrigin(), delta, 0.5);
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
    GetRotation().GetRow (viewZ, 2);
    DPoint3d target;
    target.SumOf (GetEyePoint(), viewZ, -1.0 * GetFocusDistance());
    return  target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
void CameraViewController::_RestoreFromSettings (JsonValueCR jsonObj) 
    {
    T_Super::_RestoreFromSettings(jsonObj);

    m_isCameraOn = jsonObj[VIEW_SETTING_IsCameraOn].asBool();
    m_camera.SetLensAngle(jsonObj[VIEW_SETTING_CameraAngle].asDouble());
    m_camera.SetFocusDistance(jsonObj[VIEW_SETTING_CameraFocalLength].asDouble());

    DPoint3d eyePt;
    JsonUtils::DPoint3dFromJson (eyePt, jsonObj[VIEW_SETTING_CameraPosition]);
    m_camera.SetEyePoint(eyePt);

#ifdef WIP_PERSISTENT_CLIP_VECTOR // need to change the clip tool to recognize and use saved clip vector
    if (jsonObj.isMember (VIEW_SETTING_ClipVector))
        {
        m_clipVector = ClipVector::Create();
        JsonUtils::ClipVectorFromJson (*m_clipVector, jsonObj[VIEW_SETTING_ClipVector]);
        }
    else
        {
        m_clipVector = NULL;
        }
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void PhysicalViewController::_RestoreFromSettings (JsonValueCR jsonObj)
    {
    T_Super::_RestoreFromSettings(jsonObj);

    m_viewFlags.From3dJson(jsonObj[VIEW_SETTING_Flags]);

    if (jsonObj.isMember(VIEW_SETTING_DisplayStyleId))
        m_displayStyleId = DgnStyleId(jsonObj[VIEW_SETTING_DisplayStyleId].asUInt());

    JsonUtils::DPoint3dFromJson (m_origin, jsonObj[VIEW_SETTING_Origin]);
    JsonUtils::DPoint3dFromJson (m_delta, jsonObj[VIEW_SETTING_Delta]);
    JsonUtils::RotMatrixFromJson (m_rotation, jsonObj[VIEW_SETTING_Rotation]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void CameraViewController::_SaveToSettings (JsonValueR jsonObj) const
    {
    T_Super::_SaveToSettings (jsonObj);

    jsonObj[VIEW_SETTING_IsCameraOn] = m_isCameraOn;
    jsonObj[VIEW_SETTING_CameraAngle] = m_camera.GetLensAngle();
    JsonUtils::DPoint3dToJson (jsonObj[VIEW_SETTING_CameraPosition], m_camera.GetEyePoint());
    jsonObj[VIEW_SETTING_CameraFocalLength] = m_camera.GetFocusDistance();

#ifdef WIP_PERSISTENT_CLIP_VECTOR // need to change the clip tool to recognize and use saved clip vector
    if (m_clipVector.IsValid())
        JsonUtils::ClipVectorToJson (jsonObj[VIEW_SETTING_ClipVector], *m_clipVector);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void PhysicalViewController::_SaveToSettings (JsonValueR jsonObj) const
    {
    T_Super::_SaveToSettings (jsonObj);

    m_viewFlags.To3dJson(jsonObj[VIEW_SETTING_Flags]);

    if (m_displayStyleId.IsValid())
        jsonObj[VIEW_SETTING_DisplayStyleId] = m_displayStyleId.GetValue();

    JsonUtils::DPoint3dToJson (jsonObj[VIEW_SETTING_Origin], m_origin);
    JsonUtils::DPoint3dToJson (jsonObj[VIEW_SETTING_Delta], m_delta);
    JsonUtils::RotMatrixToJson (jsonObj[VIEW_SETTING_Rotation], m_rotation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/07
+---------------+---------------+---------------+---------------+---------------+------*/
ViewPortInfo::ViewPortInfo() {Clear();}
ViewPortInfo::~ViewPortInfo() {}
ViewPortInfoPtr ViewPortInfo::Create() {return new ViewPortInfo();}
ViewPortInfoPtr ViewPortInfo::CopyFrom (ViewPortInfoCR source) {return new ViewPortInfo (source);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ViewPortInfoPtr ViewPortInfo::Create (BSIRectCR windowRect, BSIRectCR screenRect, UInt32 screenNum, bool wasDefined)
    {
    ViewPortInfo*   vp = new ViewPortInfo();

    DPoint2d    screenSize;
    screenSize.x = (double) (screenRect.corner.x - screenRect.origin.x);
    screenSize.y = (double) (screenRect.corner.y - screenRect.origin.y);
    if ( (screenSize.x > 0) && (screenSize.y > 0) )
        {
        vp->m_globalRelativeRect.origin.x = (double) (windowRect.origin.x - screenRect.origin.x) / screenSize.x;
        vp->m_globalRelativeRect.origin.y = (double) (windowRect.origin.y - screenRect.origin.y) / screenSize.y;

        DPoint2d    extentFraction;
        extentFraction.x = (double) (windowRect.corner.x - windowRect.origin.x) / screenSize.x;
        extentFraction.y = (double) (windowRect.corner.y - windowRect.origin.y) / screenSize.y;

        vp->m_globalRelativeRect.corner.x = vp->m_globalRelativeRect.origin.x + extentFraction.x;
        vp->m_globalRelativeRect.corner.y = vp->m_globalRelativeRect.origin.y + extentFraction.y;
        }

    vp->m_viewPixelRect.origin.x    = (Int16)windowRect.origin.x;
    vp->m_viewPixelRect.origin.y    = (Int16)windowRect.origin.y;
    vp->m_viewPixelRect.corner.x    = (Int16)windowRect.corner.x;
    vp->m_viewPixelRect.corner.y    = (Int16)windowRect.corner.y;
    vp->m_wasDefined                = wasDefined;
    vp->m_screenNumber              = (Int16) screenNum;

    return vp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewPortInfo::Clear()
    {
    memset (&m_globalRelativeRect, 0, sizeof(m_globalRelativeRect));
    memset (&m_viewPixelRect, 0, sizeof(m_viewPixelRect));

    m_wasDefined   = false;
    m_screenNumber = 0;
    m_reserved     = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewPortInfo::From (ViewPortInfoCR other)
    {
    if (this == &other)
        return;

    m_globalRelativeRect  = other.m_globalRelativeRect;
    m_viewPixelRect       = other.m_viewPixelRect;
    m_wasDefined          = other.m_wasDefined;
    m_screenNumber        = other.m_screenNumber;
    m_reserved            = other.m_reserved;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/07
+---------------+---------------+---------------+---------------+---------------+------*/
ViewPortInfoR ViewPortInfo::operator= (ViewPortInfoCR other)
    {
    From (other);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/07
+---------------+---------------+---------------+---------------+---------------+------*/
ViewPortInfo::ViewPortInfo (ViewPortInfoCR other)
    {
    From (other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewPortInfo::IsEqual (ViewPortInfoCP other) const
    {
    if (NULL == other)
        return false;

    if (this == other)
        return true;

    if (m_screenNumber != other->m_screenNumber)
        return  false;

    if (m_wasDefined != other->m_wasDefined)
        return  false;

    if (0 != memcmp (&m_globalRelativeRect, &other->m_globalRelativeRect, sizeof(m_globalRelativeRect)))
        return  false;

    if (0 != memcmp (&m_viewPixelRect, &other->m_viewPixelRect, sizeof(m_viewPixelRect)))
        return  false;

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewPortInfo::GetAspectRatio() const
    {
    double  aspectYDelta = (double) m_viewPixelRect.corner.y - m_viewPixelRect.origin.y;

    if  (aspectYDelta > 0.0)
        return (double)(m_viewPixelRect.corner.x - m_viewPixelRect.origin.x) / aspectYDelta;

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysP PhysicalViewController::_GetAuxCoordinateSystem() const
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    // if we don't have an ACS when this is called, try to get one.
    if (!m_auxCoordSys.IsValid())
        IACSManager::GetManager().ReadSettings (const_cast <ViewControllerP>(this), GetElementRef(), GetRootModelP(false));

    IAuxCoordSysP   acs = m_auxCoordSys.get();

     if (NULL != acs && SUCCESS == acs->CompleteSetupFromViewController (this))
        return acs;

    return m_auxCoordSys.get();
#endif

    return m_auxCoordSys.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::SetAuxCoordinateSystem (IAuxCoordSysP acs)
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
    memset (this, 0, sizeof(ViewFlags));

    line_wghts  = true;
    patterns    = true;
    text_nodes  = true;
    ed_fields   = true;
    constructs  = true;
    dimens      = true;
    fill        = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalViewController::_AdjustAspectRatio (double windowAspect, bool expandView)
    {
    // first, make sure none of the deltas are negative
    m_delta.x = fabs (m_delta.x);
    m_delta.y = fabs (m_delta.y);
    m_delta.z = fabs (m_delta.z);

    double maxAbs = max (m_delta.x, m_delta.y);

    // if all deltas are zero, set to 100 (what else can we do?)
    if (0.0 == maxAbs)
        m_delta.x = m_delta.y = 100;

    // if either dimension is zero, set it to the other.
    if (m_delta.x == 0)
        m_delta.x = maxAbs;
    if (m_delta.y == 0)
        m_delta.y = maxAbs;

    double viewAspect  = m_delta.x / m_delta.y;

    if (fabs(1.0 - (viewAspect / windowAspect)) < 1.0e-9)
        return;

    DVec3d oldDelta = m_delta;

    if (!expandView)
        {
        if (viewAspect > 1.0)
            m_delta.y = m_delta.x;
        else
            m_delta.x = m_delta.y;
        }

    if (expandView ? (viewAspect > windowAspect) : (windowAspect > 1.0))
        {
        double rtmp = m_delta.x / windowAspect;
        if (rtmp < MAX_VDELTA)
            m_delta.y = rtmp;
        else
            {
            m_delta.y = MAX_VDELTA;
            m_delta.x = MAX_VDELTA * windowAspect;
            }
        }
    else
        {
        double rtmp = m_delta.y * windowAspect;
        if (rtmp < MAX_VDELTA)
            m_delta.x = rtmp;
        else
            {
            m_delta.x = MAX_VDELTA;
            m_delta.y = MAX_VDELTA / windowAspect;
            }
        }

    DPoint3d origin;
    m_rotation.Multiply (&origin, &m_origin, 1);
    origin.x += (oldDelta.x - m_delta.x) / 2.0;
    origin.y += (oldDelta.y - m_delta.y) / 2.0;
    m_rotation.MultiplyTranspose (m_origin, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController2d::_AdjustAspectRatio (double windowAspect, bool expandView)
    {
    // first, make sure none of the deltas are negative
    m_delta.x = fabs (m_delta.x);
    m_delta.y = fabs (m_delta.y);

    double maxAbs = max (m_delta.x, m_delta.y);

    // if all deltas are zero, set to 100 (what else can we do?)
    if (0.0 == maxAbs)
        m_delta.x = m_delta.y = 100;

    // if either dimension is zero, set it to the other.
    if (m_delta.x == 0)
        m_delta.x = maxAbs;
    if (m_delta.y == 0)
        m_delta.y = maxAbs;

    double viewAspect  = m_delta.x / m_delta.y;
    if (fabs(1.0 - (viewAspect / windowAspect)) < 1.0e-9)
        return;

    DVec2d oldDelta = m_delta;
    if (!expandView)
        {
        if (viewAspect > 1.0)
            m_delta.y = m_delta.x;
        else
            m_delta.x = m_delta.y;
        }

    if (expandView ? (viewAspect > windowAspect) : (windowAspect > 1.0))
        {
        double rtmp = m_delta.x / windowAspect;
        if (rtmp < MAX_VDELTA)
            m_delta.y = rtmp;
        else
            {
            m_delta.y = MAX_VDELTA;
            m_delta.x = MAX_VDELTA * windowAspect;
            }
        }
    else
        {
        double rtmp = m_delta.y * windowAspect;
        if (rtmp < MAX_VDELTA)
            m_delta.x = rtmp;
        else
            {
            m_delta.x = MAX_VDELTA;
            m_delta.y = MAX_VDELTA / windowAspect;
            }
        }

    DPoint2d origin;
    RotMatrix rMatrix = GetRotation();
    rMatrix.Multiply (&origin, &m_origin, 1);
    origin.x += (oldDelta.x - m_delta.x) / 2.0;
    origin.y += (oldDelta.y - m_delta.y) / 2.0;
    rMatrix.Transpose();
    rMatrix.Multiply (&m_origin, &origin, 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void ViewController2d::_RestoreFromSettings(JsonValueCR settings)
    {
    T_Super::_RestoreFromSettings(settings);

    JsonValueCR area2d = settings[VIEW_SETTING_Area2d];

    JsonUtils::DPoint2dFromJson (m_origin, area2d[VIEW_SETTING_Origin]);
    JsonUtils::DPoint2dFromJson (m_delta, area2d[VIEW_SETTING_Delta]);
    m_rotAngle = area2d[VIEW_SETTING_RotAngle].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void ViewController2d::_SaveToSettings (JsonValueR settings) const
    {
    T_Super::_SaveToSettings(settings);

    JsonValueR area2d = settings[VIEW_SETTING_Area2d];

    JsonUtils::DPoint2dToJson (area2d[VIEW_SETTING_Origin], m_origin);
    JsonUtils::DPoint2dToJson (area2d[VIEW_SETTING_Delta], m_delta);
    area2d[VIEW_SETTING_RotAngle] = m_rotAngle;
    }
