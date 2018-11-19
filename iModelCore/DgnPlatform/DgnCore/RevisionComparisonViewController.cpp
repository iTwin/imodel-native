/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RevisionComparisonViewController.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/RevisionComparisonViewController.h>

USING_REVISION_COMPARISON_NAMESPACE
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Symbology::InitializeDefaults()
    {
    static const double s_backgroundElementTransparency = 0.8 * 255;
	// static const double s_backgroundLineTransparency = 255;
    Appearance inserted, updated, deleted;
    updated.SetRgb(ColorDef::VersionCompareModified());
    inserted.SetRgb(ColorDef::VersionCompareInserted());
    deleted.SetRgb(ColorDef::VersionCompareDeleted());
    updated.SetAlpha(0);
    inserted.SetAlpha(0);
    deleted.SetAlpha(0);
    updated.SetIgnoresMaterial(true);
    inserted.SetIgnoresMaterial(true);
    deleted.SetIgnoresMaterial(true);

    m_current.SetAppearance(DbOpcode::Insert, inserted);
    m_current.SetAppearance(DbOpcode::Update, updated);
    m_current.SetAppearance(DbOpcode::Delete, deleted);

    inserted.SetAlpha(0x80);
    deleted.SetAlpha(0x80);

    m_target.SetAppearance(DbOpcode::Insert, inserted);
    m_target.SetAppearance(DbOpcode::Update, updated);
    m_target.SetAppearance(DbOpcode::Delete, deleted);

    m_untouched.SetIgnoresMaterial(true);
    m_untouched.SetRgb(ColorDef::VersionCompareBackground());
    Byte bTransparency = (Byte) s_backgroundElementTransparency;
    m_untouched.SetAlpha(bTransparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ComparisonData::UpdateTransientDisplay(DgnCategoryIdSet const& categories, DgnModelIdSet const& models)
    {
    for (TransientState& state : m_transient)
        {
        GeometrySourceCP geomSource = state.m_element->ToGeometrySource();
        bool categoryOn = categories.find(geomSource->GetCategoryId()) != categories.end();
        bool modelOn = models.find(state.m_element->GetModelId()) != models.end();
        state.SetIsDrawn(categoryOn && modelOn);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentState ComparisonData::GetPersistentState(DgnElementId id) const
    {
    auto iter = m_persistent.find(PersistentState(id, DbOpcode::Insert));
    return m_persistent.end() != iter ? *iter : PersistentState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentState ComparisonData::GetAnyPersistentState(DgnElementId id) const
    {
    // NB: This is inefficient, but it's also not actually used.
    auto iter = std::find_if(m_persistent.begin(), m_persistent.end(), [=](PersistentState const& arg) { return arg.m_elementId == id; });
    return m_persistent.end() != iter ? *iter : PersistentState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TransientState ComparisonData::GetTransientState(DgnElementId id) const
    {
    // NB: This is inefficient, but it's also not actually used.
    auto iter = std::find_if(m_transient.begin(), m_transient.end(), [=](TransientState const& arg) { return arg.m_element->GetElementId() == id; });
    return m_transient.end() != iter ? *iter : TransientState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ComparisonData::ContainsElement(DgnElementCP element) const
    {
    auto iter = std::find_if(m_persistent.begin(), m_persistent.end(), [=](PersistentState const& arg) { return arg.m_elementId == element->GetElementId(); });
    return (m_persistent.end() != iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ComparisonData::GetDbOpcode(DgnElementId elementId, BeSQLite::DbOpcode& opcode)
    {
    // Obtain the type of Opcode associated with an element ID
    PersistentState pers = GetPersistentState(elementId);
    TransientState tran = GetTransientState(elementId);

    if (pers.IsValid())
        opcode = pers.m_opcode;
    if (tran.IsValid())
        opcode = tran.m_opcode;

    return (pers.IsValid() || tran.IsValid()) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ComparisonData::HideElement(DgnElementId id, bool transient, bool hidden)
    {
    if (transient)
        {
        for (TransientState& state : m_transient)
            {
            if (state.GetElementId() == id)
                state.SetHidden(hidden);
            }
        }
    else
        {
        for (PersistentState& state : m_persistent)
            {
            if (state.m_elementId == id)
                state.SetHidden(hidden);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Diego.Pinate    11/18
 +---------------+---------------+---------------+---------------+---------------+------*/
void ComparisonData::ClearHidden()
    {
    for (TransientState& state : m_transient)
        state.SetHidden(false);
    for (PersistentState& state : m_persistent)
        state.SetHidden(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
 RevisionComparison::Controller::Controller(SpatialViewDefinition const& view, ComparisonData & data, Show flags, SymbologyCR symb) : T_Super(view), m_symbology(symb), m_comparisonData(&data), m_show(flags)//, m_label(TextString::Create())
    {
    m_cnmHandler = [](RevisionComparison::ControllerPtr controller){ };

    // Build the opcode cache
    for (auto state : m_comparisonData->GetPersistentStates())
        m_persistentOpcodeCache[state.m_elementId] = state.m_opcode;
    for (auto state : m_comparisonData->GetTransientStates())
        m_transientOpcodeCache[state.m_element->GetElementId()] = state.m_opcode;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     01/18
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::SetItemsDisplayHandler(std::function<void(RevisionComparison::ControllerPtr)> handler)
    {
    m_cnmHandler = handler;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Diego.Pinate    11/18
 +---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparison::Controller::UpdateTransientGraphicDisplay()
    {
    DgnCategoryIdSet categories = m_definition->GetCategorySelector().GetCategories();
    DgnModelIdSet models = GetSpatialViewDefinition().GetModelSelector().GetModels();
    // Update transient state's IsDrawn flag
    m_comparisonData->UpdateTransientDisplay(categories, models);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparison::Controller::_OnCategoryChange(bool singleEnable)
    {
    T_Super::_OnCategoryChange(singleEnable);

    // TFS#798515: Provide callbacks for _OnCategoryChange so that version compare may sync view controllers
    m_cnmHandler(this);

    // Invalidate so that we redraw world decorations that may need to be hidden or shown based on categories
    if (nullptr != m_vp)
        m_vp->InvalidateDecorations();

    if (T_HOST._IsFeatureEnabled("VersionCompare.ImprovedDisplay"))
        UpdateTransientGraphicDisplay();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     01/18
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::_ChangeModelDisplay(DgnModelId modelId, bool onOff)
    {
    T_Super::_ChangeModelDisplay(modelId, onOff);

    // TFS#798515: Provide callbacks for _OnCategoryChange so that version compare may sync view controllers
    m_cnmHandler(this);

    if (T_HOST._IsFeatureEnabled("VersionCompare.ImprovedDisplay"))
        UpdateTransientGraphicDisplay();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     11/17
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::SetModelDisplay(DgnModelIdSet& modelIds, bool visible)
    {
    for (DgnModelId modelId : modelIds)
        ChangeModelDisplay(modelId, visible);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     11/17
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::SetCategoryDisplay(DgnCategoryIdSet& categories, bool visible)
    {
    for (DgnCategoryId category : categories)
        ChangeCategoryDisplay(category, visible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr  RevisionComparison::Controller::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    // Avoid letting user pick elements that are not being compared
    //if (nullptr != context.GetIPickGeom() && !m_comparisonData->ContainsElement(source.ToElement()))
    //    return nullptr;
    return T_Super::_StrokeGeometry(context, source, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Controller::_AddFeatureOverrides(Render::FeatureSymbologyOverrides& ovrs) const
    {
    // Feature overrides are applied to the persistent elements.
    // For the 'current revision' view, this renders inserted+updated elements with meaningful symbology, and everything else in a uniform symbology.
    // For the 'target revision' view, this renders every persistent element in a uniform symbology.
    // Updated+deleted elements are rendered as decorations in 'target revision' -- see Controller::_DrawDecorations()

    if (m_focusedElementId.IsValid())
        {
        // TFS#742735: If we've set the focused element, apply special symbology only to it - draw everything else in uniform ('untouched') symbology
        PersistentState lookupKey(m_focusedElementId, DbOpcode::Insert); // operator< only compares element IDs...
        auto iter = m_comparisonData->GetPersistentStates().find(lookupKey);
        if (m_comparisonData->GetPersistentStates().end() != iter)
            ovrs.OverrideElement(m_focusedElementId, m_symbology.GetCurrentRevisionOverrides(iter->m_opcode));
        }
    else if (WantShowCurrent())
        {
        for (auto const& entry : m_comparisonData->GetPersistentStates())
            ovrs.OverrideElement(entry.m_elementId, m_symbology.GetCurrentRevisionOverrides(entry.m_opcode));
        }

    ovrs.SetDefaultOverrides(m_symbology.GetUntouchedOverrides());

    if (WantShowOnlyTarget())
        {
        // In 'target' view, hide persistent state of modified and inserted elements
        for (auto const& entry : m_comparisonData->GetPersistentStates())
            if (entry.IsInsertion() || entry.IsModified())
                ovrs.NeverDraw(entry.m_elementId);
        }

    // Never draw things that have been hidden by the user in overview stage
    if (WantShowBoth())
        {
        for (auto const& entry : m_comparisonData->GetPersistentStates())
            if (entry.IsHidden())
                ovrs.NeverDraw(entry.m_elementId);
        }

    T_Super::_AddFeatureOverrides(ovrs);

#if defined(TODO_VERSION_COMPARE_MESS)
    /* ###TODO: To support this we need to allow the 'overrides' for an element to specify 'don't override anything' - otherwise it gets the default overrides
    DgnElementId elementId = el->GetElementId();

    // TFS#742735: Only colorize focused element if we have set this ViewController to do so
    if (m_focusedElementId.IsValid() && m_focusedElementId != el->GetElementId())
        {
        m_symbology.GetUntouchedOverrides(symbologyOverrides);
        //T_Super::_OverrideGraphicParams(symbologyOverrides, source);
        return;
        }
    */

    if (WantShowCurrent())
        {
        for (auto const& entry : m_comparisonData->GetPersistentStates())
            ovrs.OverrideElement(entry.m_elementId, m_symbology.GetCurrentRevisionOverrides(entry.m_opcode));
        }

    bmap<DgnElementId,DbOpcode>::const_iterator persistent = m_persistentOpcodeCache.find(elementId);
    bmap<DgnElementId,DbOpcode>::const_iterator transient = m_transientOpcodeCache.find(elementId);
    if (WantShowTarget())
    if (WantShowCurrent() && !m_visitingTransientElements && persistent != m_persistentOpcodeCache.end())
        {
        for (auto const& entry : m_comparisonData->GetTransientStates())
        m_symbology.GetCurrentRevisionOverrides(persistentOpcode, symbologyOverrides);
        }

    ovrs.SetDefaultOverrides(m_symbology.GetUntouchedOverrides());

    T_Super::_AddFeatureOverrides(ovrs);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Controller::_CreateScene(SceneContextR context)
    {
    auto status = T_Super::_CreateScene(context);
#if defined(TODO_VERSION_COMPARE_MESS)
    if (WantShowTarget())
        {
        if (transient != m_transientOpcodeCache.end() && m_visitingTransientElements)
            {
            m_symbology.GetTargetRevisionOverrides(transient->second, symbologyOverrides);
            // Joe doesn't want to show the transient/updated state of a modified element
            // if we are showing them in a single view
        if (!WantShowBoth() && persistent != m_persistentOpcodeCache.end())
                continue;

            auto geom = entry.m_element->ToGeometrySource();
            if (nullptr != geom)
                context.VisitGeometry(*geom);
            }
        }
#endif

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void getViewCorners(DPoint3dR low, DPoint3dR high, int indent, DgnViewportCR vp)
    {
    DRange3d range = vp.GetViewCorners();

    low = range.low;
    high = range.high;

    if (low.x > high.x)
        std::swap(low.x, high.x);

    if (low.y > high.y)
        std::swap(low.y, high.y);

#if defined (__APPLE__)
    //  GetViewCorners does not provide correct information for iOS.
    low.x = low.y = 4.0;
    if (high.x > high.y)
        {
        if (high.x > 2040)
            {
            high.x = 2044;
            high.y = 1475;
            }
        else
            {
            high.x = 1020;
            high.y = 700;
            }
        }
    else
        {
        if (high.y > 2000)
            {
            high.y = 2000;
            high.x = 1500;
            }
        else
            {
            high.y = 1020;
            high.x = 700;
            }
        }
#endif

    low.x += indent;
    low.y += indent;
    high.x -= indent;
    high.y -= indent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparison::Controller::SetVersionLabel(Utf8String label)
    {
#ifdef USE_LABEL
    m_labelString = label;

    TextStringStylePtr style = TextStringStyle::Create();
    style->SetFont(DgnFontManager::GetDecoratorFont());
    style->SetSize(14);

    RotMatrix textMatrix;
    textMatrix.InitFromScaleFactors(1.0, -1.0, 1.0);

    m_label = TextString::Create();
    m_label->SetStyle(*style);
    m_label->SetText(m_labelString.c_str());
    
    DPoint3d low        = DPoint3d::FromZero();
    DPoint3d position   = DPoint3d::FromZero();
    if (nullptr != m_vp)
        getViewCorners(low, position, 30, *m_vp);

    position.z = 0;
    position.x = low.x;
    m_label->SetOriginFromJustificationOrigin(position, TextString::HorizontalJustification::Left, TextString::VerticalJustification::Bottom);
    m_label->SetOrientation(textMatrix);
#endif
    }

/*---------------------------------------------------------------------------------**//**
 * Taken from QV
 * @bsimethod                                                    Diego.Pinate    11/18
 +---------------+---------------+---------------+---------------+---------------+------*/
DRange1d TransientState::GetRange(double pixelSize, AxisAlignedBox3d const& range) const
    {
    static double sizeDependentRatio = 5.0;
    static double pixelToChordRatio = 0.5;
    static double minRangeRelTol = 1.0e-4;
    static double maxRangeRelTol = 1.5e-2;
    DRange1d pixelSizeRange;
    double maxDimension = range.DiagonalDistance();
    double minChordTol = minRangeRelTol * maxDimension;
    double maxChordTol = maxRangeRelTol * maxDimension;
    double chordTol = pixelToChordRatio * pixelSize;
    bool isMin = false, isMax = false;

    if (isMin = (chordTol < minChordTol))
        chordTol = minChordTol;
    else if (isMax = (chordTol > maxChordTol))
        chordTol = maxChordTol;

    if (isMin)
        pixelSizeRange = DRange1d::FromLowHigh(0.0, chordTol * sizeDependentRatio);
    else if (isMax)
        pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, DBL_MAX);
    else
        pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, chordTol * sizeDependentRatio);

    return pixelSizeRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TransientGraphic::IsValidForSize(double metersPerPixel) const
    {
    if (0.0 == metersPerPixel || (0.0 == m_minSize && 0.0 == m_maxSize))
        return true;

    return (metersPerPixel >= m_minSize && metersPerPixel <= m_maxSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TransientGraphic TransientState::GetFromCache(double pixelSize) const
    {
    for (auto tg : m_graphicCache)
        if (tg.IsValidForSize(pixelSize))
            return tg;

    return TransientGraphic();
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Paul.Connelly   09/17
 +---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicP TransientState::GetGraphic(ViewContextR context) const
    {
    if (T_HOST._IsFeatureEnabled("VersionCompare.ImprovedDisplay"))
        {
        // This newer version caches the graphics in a map so that we may display lower quality versions
        // of the geometry when far away. It also stops displaying geometry that's too small in the view
        auto geomElem = m_element->ToGeometrySource();
        if (nullptr == geomElem)
            return nullptr;

        AxisAlignedBox3d elementRange = geomElem->CalculateRange3d();
        DPoint3d center = elementRange.GetCenter();
        double pixelSize = context.GetPixelSizeAtPoint(&center);

        // Check if based on pixel size and range if we should draw
        double numPixels = elementRange.DiagonalDistance() / pixelSize;
        if (numPixels < 1.0)
            return nullptr;

        DRange1d pixelRange = GetRange(pixelSize, elementRange);
        TransientGraphic tg = GetFromCache(pixelSize);
        if (tg.IsValid())
            return tg.m_graphic.get();

        Render::GraphicPtr graphic = geomElem->Stroke(context, pixelSize);
        if (!graphic.IsValid())
            return nullptr;

        m_graphicCache.push_back(TransientGraphic(GetElementId(), graphic, pixelRange));
        return graphic.get();
        }
    else
        {
        if (m_graphic.IsValid())
            return m_graphic.get();

        auto geomElem = m_element->ToGeometrySource();
        if (nullptr == geomElem)
            return nullptr;

        m_graphic = geomElem->Stroke(context, 0.0);
        if (!m_graphic.IsValid())
            return nullptr;

        return m_graphic.get();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicP TransientState::GetGraphic(ViewContextR context, bool wantShowBoth) const
    {
    // Check if this element was hidden by the user (only in overview stage, e.g. WantShowBoth == true)
    // Or if its category or model is off (IsDrawn will reflect this)
    if (!IsDrawn() || (IsHidden() && wantShowBoth))
        return nullptr;

    // Joe doesn't want to show the transient/updated state of a modified element
    // if we are showing them in a single view
    if (wantShowBoth && IsModified())
        return nullptr;

    // Don't want to draw if we can't see the range of the transient element in the view
    AxisAlignedBox3d elementRange = m_element->ToGeometrySource()->CalculateRange3d();
    if (!context.IsRangeVisible(elementRange))
        return nullptr;

    return GetGraphic(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparison::Controller::_DrawDecorations(DecorateContextR context)
    {
    T_Super::_DrawDecorations(context);

    if (!WantShowTarget())
        return;

    SpatialViewDefinitionP viewDef = GetViewDefinitionR().ToSpatialViewP();
    if (viewDef == nullptr)
        {
        BeAssert(false && "View definition is not spatial.");
        return;
        }

    if (T_HOST._IsFeatureEnabled("VersionCompare.ImprovedDisplay"))
        {
        for (auto& element : m_comparisonData->GetTransientStates())
            {
            Render::GraphicP graphic = element.GetGraphic(context, WantShowBoth());
            if (nullptr == graphic)
                continue;

            bool useUntouched = m_focusedElementId.IsValid() && m_focusedElementId != element.m_element->GetElementId();
            Symbology::Appearance app = useUntouched ? m_symbology.GetUntouchedOverrides() : m_symbology.GetTargetRevisionOverrides(element.m_opcode);
            OvrGraphicParams ovrs = app.ToOvrGraphicParams();
            context.AddWorldDecoration(*graphic, app.OverridesSymbology() ? &ovrs : nullptr);
            }
        }
    else
        {
        for (auto& element : m_comparisonData->GetTransientStates())
            {
            if (WantShowBoth() && element.IsModified())
                continue;

            Render::GraphicP graphic = element.GetGraphic(context);
            if (nullptr == graphic)
                continue;

            bool useUntouched = m_focusedElementId.IsValid() && m_focusedElementId != element.m_element->GetElementId();
            Symbology::Appearance app = useUntouched ? m_symbology.GetUntouchedOverrides() : m_symbology.GetTargetRevisionOverrides(element.m_opcode);
            OvrGraphicParams ovrs = app.ToOvrGraphicParams();
            context.AddWorldDecoration(*graphic, app.OverridesSymbology() ? &ovrs : nullptr);
            }
        }

#ifdef USE_LABEL
    // We only display overlay of version numbers when we have two separate views
    if (WantShowBoth() || !m_label.IsValid())
        return;

    auto graphic = context.CreateViewGraphic();

    DPoint3d low        = DPoint3d::FromZero();
    DPoint3d position   = DPoint3d::FromZero();
    if (nullptr != m_vp)
        getViewCorners(low, position, 30, *m_vp);

    position.z = 0;
    position.x = low.x;
    m_label->SetOriginFromJustificationOrigin(position, TextString::HorizontalJustification::Left, TextString::VerticalJustification::Bottom);

    graphic->AddTextString(*m_label);
    context.AddViewOverlay(*graphic->Finish());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparison::Controller::_OnViewOpened (DgnViewportR vp)
    {
    Render::ViewFlags viewFlags = GetViewFlags();
    if (viewFlags.GetRenderMode() != Render::RenderMode::SmoothShade)
        viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);

    viewFlags.SetShowVisibleEdges(false);
    viewFlags.SetShowPatterns(false);
    viewFlags.SetShowGrid(false);
    viewFlags.SetShowClipVolume(false);
    viewFlags.SetShowTransparency(true);
    viewFlags.SetShowAcsTriad(false);  // Hide the indicator because it is work-in-progress.
    vp.GetViewControllerR().GetViewDefinitionR().GetDisplayStyle().SetViewFlags(viewFlags);

    T_Super::_OnViewOpened(vp);
    }
