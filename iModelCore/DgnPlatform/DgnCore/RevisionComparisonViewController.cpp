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
    Render::OvrGraphicParams update, inserted, deleted;
    update.SetFillColor(ColorDef::Blue());
    inserted.SetFillColor(ColorDef::Green());
    deleted.SetFillColor(ColorDef::Red());

    m_currentRevisionOverrides.Insert(DbOpcode::Update, update);
    m_currentRevisionOverrides.Insert(DbOpcode::Insert, inserted);
    m_currentRevisionOverrides.Insert(DbOpcode::Delete, deleted);

    update.SetFillColor(ColorDef::Cyan());
    update.SetFillTransparency(128);
    update.SetLineTransparency(128);
    inserted.SetFillTransparency(128);
    inserted.SetLineTransparency(128);
    deleted.SetFillTransparency(128);
    deleted.SetLineTransparency(128);

    m_targetRevisionOverrides.Insert(DbOpcode::Update, update);
    m_targetRevisionOverrides.Insert(DbOpcode::Insert, inserted);
    m_targetRevisionOverrides.Insert(DbOpcode::Delete, deleted);

    m_untouchedOverride.SetFillColor(ColorDef::MediumGrey());
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
* @bsimethod                                                    Diego.Pinate    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr  RevisionComparisonViewController::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    if (!WantShowCurrent() && nullptr != context.GetIPickGeom())
        {
        DgnElementCP element = source.ToElement();

        // Avoid letting user pick elements that are not being compared
        if (!m_comparisonData->ContainsElement(element))
            return nullptr;

        // If we are only showing target version on a view, avoid modified persistent elements to be highlighted by mouse
        if (m_comparisonData->ContainsElement(element) && !(m_comparisonData->GetPersistentState(element->GetElementId())).IsModified())
            return nullptr;

        // Let user hover/select transient elements
        TransientState state = m_comparisonData->GetTransientState(element->GetElementId());
        if (WantShowTarget() && state.IsValid() && nullptr != state.m_element->ToGeometrySource())
            return state.m_element->ToGeometrySource()->Stroke(context, pixelSize);
        }

    return source.Stroke(context, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonViewController::_OverrideGraphicParams(Render::OvrGraphicParamsR symbologyOverrides, Dgn::GeometrySourceCP source)
    {
    DgnElementCP el = source->ToElement();

    if (nullptr == el)
        return;

    PersistentState elementIdData = m_comparisonData->GetPersistentState(el->GetElementId());
    TransientState elementData = m_comparisonData->GetTransientState(el->GetElementId());

    // Get the override for element IDs
    if (WantShowCurrent() && !m_visitingTransientElements && elementIdData.IsValid())
        {
        m_symbology.GetCurrentRevisionOverrides(elementIdData.m_opcode, symbologyOverrides);
        return;
        }

    // Get the override from the temporary elements
    if (WantShowTarget())
        {
        if (elementData.IsValid() && m_visitingTransientElements)
            {
            m_symbology.GetTargetRevisionOverrides(elementData.m_opcode, symbologyOverrides);
            return;
            }

        // Elements that are modified need to be transparent if we are in "Target-only" view
        if (!WantShowBoth() && elementIdData.IsValid())
            {
            symbologyOverrides.SetLineTransparency(255);
            symbologyOverrides.SetFillTransparency(255);
            return;
            }
        }

    // Provide an "untouched" override
    m_symbology.GetUntouchedOverrides(symbologyOverrides);
    T_Super::_OverrideGraphicParams(symbologyOverrides, source);
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
        GeometrySourceCP geomElem = element.m_element->ToGeometrySource();
        if (nullptr == geomElem)
            continue;

        context.VisitGeometry(*geomElem);
        }

    m_visitingTransientElements = false;
    }
