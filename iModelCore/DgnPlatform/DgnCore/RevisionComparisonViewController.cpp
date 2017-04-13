/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RevisionComparisonViewController.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/RevisionComparisonViewController.h>

RevisionComparisonSettings::ComparisonSymbologyOverrides* RevisionComparisonSettings::s_symbologyOverrides = NULL;
RevisionComparisonElementKeeper::ComparisonDataPtr RevisionComparisonElementKeeper::s_comparisonData;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionComparisonSettings::ComparisonSymbologyOverrides *  RevisionComparisonSettings::Overrides()
    {
    if (NULL == s_symbologyOverrides)
        {
        s_symbologyOverrides = new ComparisonSymbologyOverrides();
        s_symbologyOverrides->InitializeDefaults();
        }

    return s_symbologyOverrides;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonSettings::ComparisonSymbologyOverrides::InitializeDefaults()
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
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonSettings::ComparisonSymbologyOverrides::GetCurrentRevisionOverrides(DbOpcode const& opcode, Render::OvrGraphicParamsR overrides)
    {
    BeAssert(m_currentRevisionOverrides.find(opcode) != m_currentRevisionOverrides.end());
    overrides = m_currentRevisionOverrides[opcode];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonSettings::ComparisonSymbologyOverrides::GetTargetRevisionOverrides(DbOpcode const& opcode, Render::OvrGraphicParamsR overrides)
    {
    BeAssert(m_targetRevisionOverrides.find(opcode) != m_targetRevisionOverrides.end());
    overrides = m_targetRevisionOverrides[opcode];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonSettings::ComparisonSymbologyOverrides::GetUntouchedOverrides(Render::OvrGraphicParamsR overrides)
    {
    overrides = m_untouchedOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonElementKeeper::ClearComparisonData()
    {
    s_comparisonData = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonElementKeeper::AddElement(DgnElementPtr element, DbOpcode opcode)
    {
    if (!s_comparisonData.IsValid())
        s_comparisonData = new ComparisonData();

#ifdef TEST_IDS
    for (auto & elementPair : s_comparisonData->m_elements)
        {
        if (elementPair.m_data->GetElementId() == element->GetElementId())
            {
            BeAssert(false && "We shouldn't have two equal IDs...");
            }
        }
#endif

    PairWithState<DgnElementPtr> entry (element, opcode);
    s_comparisonData->m_elements.push_back(entry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonElementKeeper::AddElementId(DgnElementId elementId, DbOpcode opcode)
    {
    if (!s_comparisonData.IsValid())
        s_comparisonData = new ComparisonData();

    PairWithState<DgnElementId> entry (elementId, opcode);
    s_comparisonData->m_elementIds.push_back(entry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
template <>
void RevisionComparisonElementKeeper::PairWithState<DgnElementPtr>::GetOverrideGraphicParams(Render::OvrGraphicParamsR symbologyOverrides) const
    {
    RevisionComparisonSettings::Overrides()->GetTargetRevisionOverrides(m_opcode, symbologyOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
template <>
void RevisionComparisonElementKeeper::PairWithState<DgnElementId>::GetOverrideGraphicParams(Render::OvrGraphicParamsR symbologyOverrides) const
    {
    RevisionComparisonSettings::Overrides()->GetCurrentRevisionOverrides(m_opcode, symbologyOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool   RevisionComparisonElementKeeper::ContainsElementId(DgnElementId const& id, PairWithState<DgnElementId>*& outPair)
    {
    if (!s_comparisonData.IsValid())
        return false;

    for (PairWithState<DgnElementId>& current : s_comparisonData->m_elementIds)
        {
        if (id == current.m_data)
            {
            outPair = &current;
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool   RevisionComparisonElementKeeper::ContainsElement(DgnElementId const& id, PairWithState<DgnElementPtr>*& outPair)
    {
    if (!s_comparisonData.IsValid())
        return false;

    for (PairWithState<DgnElementPtr>& current : s_comparisonData->m_elements)
        {
        if (id == current.m_data->GetElementId())
            {
            outPair = &current;
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparisonElementKeeper::CollectTransientElements(bvector<DgnElementPtr> & elements)
    {
    if (!s_comparisonData.IsValid())
        return;

    for (PairWithState<DgnElementPtr>& pair : s_comparisonData->m_elements)
        elements.push_back(pair.m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparisonElementKeeper::CollectTransientElements(bvector<PairWithState<DgnElementPtr>> & elements)
    {
    if (!s_comparisonData.IsValid())
        return;

    for (PairWithState<DgnElementPtr>& pair : s_comparisonData->m_elements)
        elements.push_back(pair);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionComparisonViewController::_OverrideGraphicParams(Render::OvrGraphicParamsR symbologyOverrides, Dgn::GeometrySourceCP source)
    {
    DgnElementCP el = source->ToElement();

    RevisionComparisonElementKeeper::PairWithState<DgnElementId>* elementIdData = NULL;
    RevisionComparisonElementKeeper::PairWithState<DgnElementPtr>* elementData = NULL;

    // Get the override for element IDs
    if (((m_flags & SHOW_CURRENT) != 0) && RevisionComparisonElementKeeper::ContainsElementId(el->GetElementId(), elementIdData))
        {
        elementIdData->GetOverrideGraphicParams(symbologyOverrides);
        return;
        }

    // Get the override from the temporary elements
    if (((m_flags & SHOW_TARGET) != 0) && RevisionComparisonElementKeeper::ContainsElement(el->GetElementId(), elementData))
        {
        elementData->GetOverrideGraphicParams(symbologyOverrides);
        return;
        }

    // Provide an "untouched" override
    RevisionComparisonSettings::Overrides()->GetUntouchedOverrides(symbologyOverrides);
    T_Super::_OverrideGraphicParams(symbologyOverrides, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparisonViewController::_VisitAllElements(ViewContextR context)
    {
    T_Super::_VisitAllElements(context);

    // Visit the transient elements
    bvector<DgnElementPtr> elements;
    RevisionComparisonElementKeeper::CollectTransientElements(elements);
    for (auto& element : elements)
        {
        GeometrySourceCP geomElem = element->ToGeometrySource();
        if (nullptr == geomElem)
            continue;

        context.VisitGeometry(*geomElem);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparisonViewController::_CreateTerrain(TerrainContextR context)
    {
    T_Super::_CreateTerrain(context);

    // Visit the transient elements
    bvector<RevisionComparisonElementKeeper::PairWithState<DgnElementPtr>> elements;
    RevisionComparisonElementKeeper::CollectTransientElements(elements);
    for (auto& element : elements)
        {
        GeometrySourceCP geomElem = element.m_data->ToGeometrySource();
        if (nullptr == geomElem)
            continue;

        context.VisitGeometry(*geomElem);
        }
    }
