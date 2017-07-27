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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr  RevisionComparisonViewController::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    if (nullptr != context.GetIPickGeom())
        {
        DgnElementCP element = source.ToElement();

        // Avoid letting user pick elements that are not being compared
        if (!m_comparisonData->ContainsElement(element))
            return nullptr;

        // If we are only showing target version on a view, avoid modified persistent elements to be highlighted by mouse
        if (!WantShowCurrent() && m_comparisonData->ContainsElement(element) && !(m_comparisonData->GetPersistentState(element->GetElementId())).IsModified())
            return nullptr;

        // Let user hover/select transient elements
        TransientState state = m_comparisonData->GetTransientState(element->GetElementId());
        if (WantShowTarget() && state.IsValid() && nullptr != state.m_element->ToGeometrySource())
            return state.m_element->ToGeometrySource()->Stroke(context, pixelSize);
        }

    return source.Stroke(context, pixelSize);
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

