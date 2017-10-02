/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RevisionComparisonViewController.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/RevisionComparisonViewController.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
ComparisonSymbologyOverrides::ComparisonSymbologyOverrides()
    {
    InitializeDefaults();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ComparisonSymbologyOverrides::InitializeDefaults()
    {
    static const double s_backgroundElementTransparency = 0.5 * 255;
    static const double s_backgroundLineTransparency = 255;

    Render::OvrGraphicParams update, inserted, deleted;
    update.SetFillColor(ColorDef::VersionCompareModified());
    inserted.SetFillColor(ColorDef::VersionCompareInserted());
    deleted.SetFillColor(ColorDef::VersionCompareDeleted());
    update.SetMaterial(nullptr);
    inserted.SetMaterial(nullptr);
    deleted.SetMaterial(nullptr);

    m_currentRevisionOverrides.Insert(DbOpcode::Update, update);
    m_currentRevisionOverrides.Insert(DbOpcode::Insert, inserted);
    m_currentRevisionOverrides.Insert(DbOpcode::Delete, deleted);

    inserted.SetFillTransparency(128);
    inserted.SetLineTransparency(128);
    deleted.SetFillTransparency(128);
    deleted.SetLineTransparency(128);

    m_targetRevisionOverrides.Insert(DbOpcode::Update, update);
    m_targetRevisionOverrides.Insert(DbOpcode::Insert, inserted);
    m_targetRevisionOverrides.Insert(DbOpcode::Delete, deleted);

    m_untouchedOverride.SetMaterial(nullptr);
    m_untouchedOverride.SetFillColor(ColorDef::VersionCompareBackground());
    m_untouchedOverride.SetLineColor(ColorDef::VersionCompareBackground());
    Byte bTransparency = (Byte) s_backgroundElementTransparency;
    Byte bLineTransparency = (Byte) s_backgroundLineTransparency;
    m_untouchedOverride.SetFillTransparency(bTransparency);
    m_untouchedOverride.SetLineTransparency(bLineTransparency);
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
StatusInt   ComparisonData::GetDbOpcode(DgnElementId elementId, DbOpcode& opcode)
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
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ComparisonSymbologyOverrides::GetCurrentRevisionOverrides(DbOpcode const& opcode, Render::OvrGraphicParamsR overrides)
    {
    BeAssert(m_currentRevisionOverrides.find(opcode) != m_currentRevisionOverrides.end());
    overrides = m_currentRevisionOverrides[opcode];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ComparisonSymbologyOverrides::GetTargetRevisionOverrides(DbOpcode const& opcode, Render::OvrGraphicParamsR overrides)
    {
    BeAssert(m_targetRevisionOverrides.find(opcode) != m_targetRevisionOverrides.end());
    overrides = m_targetRevisionOverrides[opcode];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ComparisonSymbologyOverrides::GetUntouchedOverrides(Render::OvrGraphicParamsR overrides)
    {
    overrides = m_untouchedOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
 RevisionComparisonViewController::RevisionComparisonViewController(SpatialViewDefinition const& view, ComparisonData const& data, unsigned int flags, ComparisonSymbologyOverrides const & symb) : T_Super(view), m_symbology(symb), m_comparisonData(&data), m_flags(flags), m_visitingTransientElements(false)//, m_label(TextString::Create())
    {
    // Build the opcode cache
    for (auto state : m_comparisonData->GetPersistentStates())
        m_persistentOpcodeCache[state.m_elementId] = state.m_opcode;
    for (auto state : m_comparisonData->GetTransientStates())
        m_transientOpcodeCache[state.m_element->GetElementId()] = state.m_opcode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr  RevisionComparisonViewController::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    // Avoid letting user pick elements that are not being compared
    //if (nullptr != context.GetIPickGeom() && !m_comparisonData->ContainsElement(source.ToElement()))
    //    return nullptr;
    return T_Super::_StrokeGeometry(context, source, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonViewController::_OverrideGraphicParams(Render::OvrGraphicParamsR symbologyOverrides, Dgn::GeometrySourceCP source)
    {
    if (nullptr == source)
        return;

    DgnElementCP el = source->ToElement();

    if (nullptr == el)
        return;

    symbologyOverrides.Clear();

    DgnElementId elementId = el->GetElementId();

    // TFS#742735: Only colorize focused element if we have set this ViewController to do so
    if (m_focusedElementId.IsValid() && m_focusedElementId != elementId)
        {
        m_symbology.GetUntouchedOverrides(symbologyOverrides);
        return;
        }

    DbOpcode persistentOpcode = m_persistentOpcodeCache[elementId];
    DbOpcode transientOpcode = m_transientOpcodeCache[elementId];

    // Get the override for element IDs
    if (WantShowCurrent() && !m_visitingTransientElements && persistentOpcode != (DbOpcode)0)
        {
        m_symbology.GetCurrentRevisionOverrides(persistentOpcode, symbologyOverrides);
        return;
        }

    // Get the override from the temporary elements
    if (WantShowTarget())
        {
        if (transientOpcode != (DbOpcode)0 && m_visitingTransientElements)
            {
            m_symbology.GetTargetRevisionOverrides(transientOpcode, symbologyOverrides);
            return;
            }

        // Elements that are modified need to be transparent if we are in "Target-only" view
        if (!WantShowBoth() && persistentOpcode != (DbOpcode)0)
            {
            symbologyOverrides.SetLineTransparency(255);
            symbologyOverrides.SetFillTransparency(255);
            return;
            }
        }

    // Provide an "untouched" override
    m_symbology.GetUntouchedOverrides(symbologyOverrides);
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
void    RevisionComparisonViewController::SetVersionLabel(Utf8String label)
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
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparisonViewController::_DrawDecorations(DecorateContextR context)
    {
    T_Super::_DrawDecorations(context);
#ifdef USE_LABEL
    // We only display overlay of version numbers when we have two separate views
    if (WantShowBoth() || !m_label.IsValid())
        return;

    auto graphic = context.CreateGraphic();

    DPoint3d low        = DPoint3d::FromZero();
    DPoint3d position   = DPoint3d::FromZero();
    if (nullptr != m_vp)
        getViewCorners(low, position, 30, *m_vp);
    position.z = 0;
    position.x = low.x;
    m_label->SetOriginFromJustificationOrigin(position, TextString::HorizontalJustification::Left, TextString::VerticalJustification::Bottom);

    graphic->AddTextString(*m_label);
    context.AddViewOverlay(*graphic);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparisonViewController::_CreateTerrain(TerrainContextR context)
    {
    T_Super::_CreateTerrain(context);

    // No need to draw transient elements if we are only showing current elements
    if (WantShowCurrent() && !WantShowTarget())
        return;

    m_visitingTransientElements = true;

    // Visit the transient elements
    for (auto& element : m_comparisonData->GetTransientStates())
        {
        // Joe doesn't want to show the transient/updated state of a modified element
        // if we are showing them in a single view
        if (WantShowBoth() && element.IsModified())
            continue;

        GeometrySourceCP geomElem = element.m_element->ToGeometrySource();
        if (nullptr == geomElem)
            continue;

        context.VisitGeometry(*geomElem);
        }

    m_visitingTransientElements = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonViewController::_OnViewOpened (DgnViewportR vp)
    {
    Render::ViewFlags viewFlags = GetViewFlags();
    if (viewFlags.GetRenderMode() != Render::RenderMode::SmoothShade)
        viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);

    viewFlags.SetShowVisibleEdges(true);
    viewFlags.SetShowPatterns(false);
    viewFlags.SetShowGrid(false);
    viewFlags.SetShowClipVolume(false);
    viewFlags.SetShowTransparency(true);
    viewFlags.SetShowAcsTriad(false);  // Hide the indicator because it is work-in-progress.
    vp.GetViewControllerR().GetViewDefinition().GetDisplayStyle().SetViewFlags(viewFlags);

    T_Super::_OnViewOpened(vp);
    }
