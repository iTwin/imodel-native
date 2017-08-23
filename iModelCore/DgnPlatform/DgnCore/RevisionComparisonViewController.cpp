/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RevisionComparisonViewController.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    Appearance inserted, updated, deleted;
    updated.SetRgb(ColorDef::Blue());
    inserted.SetRgb(ColorDef::Green());
    deleted.SetRgb(ColorDef::Red());

    m_current.SetAppearance(DbOpcode::Insert, inserted);
    m_current.SetAppearance(DbOpcode::Update, updated);
    m_current.SetAppearance(DbOpcode::Delete, deleted);

    updated.SetRgb(ColorDef::Cyan());
    updated.SetAlpha(0x80);
    inserted.SetAlpha(0x80);
    deleted.SetAlpha(0x80);

    m_target.SetAppearance(DbOpcode::Insert, inserted);
    m_target.SetAppearance(DbOpcode::Update, updated);
    m_target.SetAppearance(DbOpcode::Delete, deleted);

    m_untouched.SetRgb(ColorDef::MediumGrey());
    m_untouched.SetAlpha(200);
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
* @bsimethod                                                    Diego.Pinate    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr  RevisionComparison::Controller::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    // Avoid letting user pick elements that are not being compared
    if (nullptr != context.GetIPickGeom() && !m_comparisonData->ContainsElement(source.ToElement()))
        return nullptr;

    return T_Super::_StrokeGeometry(context, source, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Controller::_AddFeatureOverrides(Render::FeatureSymbologyOverrides& ovrs) const
    {
    if (WantShowCurrent())
        {
        for (auto const& entry : m_comparisonData->GetPersistentStates())
            ovrs.OverrideElement(entry.m_elementId, m_symbology.GetCurrentRevisionOverrides(entry.m_opcode));
        }

    if (WantShowTarget())
        {
        for (auto const& entry : m_comparisonData->GetTransientStates())
            ovrs.OverrideElement(entry.m_element->GetElementId(), m_symbology.GetTargetRevisionOverrides(entry.m_opcode));
        }

    ovrs.SetDefaultOverrides(m_symbology.GetUntouchedOverrides());

    T_Super::_AddFeatureOverrides(ovrs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Controller::_CreateScene(SceneContextR context)
    {
    auto status = T_Super::_CreateScene(context);
    if (WantShowTarget())
        {
        for (auto const& entry : m_comparisonData->GetTransientStates())
            {
            auto geom = entry.m_element->ToGeometrySource();
            if (nullptr != geom)
                context.VisitGeometry(*geom);
            }
        }

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparison::Controller::_DrawDecorations(DecorateContextR context)
    {
    T_Super::_DrawDecorations(context);

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
    }

