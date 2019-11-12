/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/ContentFieldEditors.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorJsonParams::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int FieldEditorJsonParams::_CompareTo(Params const& other) const
    {
    int baseCompare = Params::_CompareTo(other);
    if (0 != baseCompare)
        return baseCompare;

    FieldEditorJsonParams const* jsonParams = dynamic_cast<FieldEditorJsonParams const*>(&other);
    if (nullptr == jsonParams)
        {
        BeAssert(false);
        return -1;
        }
    Utf8String lhs = BeRapidJsonUtilities::ToString(m_json);
    Utf8String rhs = BeRapidJsonUtilities::ToString(jsonParams->m_json);
    return strcmp(lhs.c_str(), rhs.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorMultilineParams::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int FieldEditorMultilineParams::_CompareTo(Params const& other) const
    {
    int baseCompare = Params::_CompareTo(other);
    if (0 != baseCompare)
        return baseCompare;

    FieldEditorMultilineParams const* multilineParams = dynamic_cast<FieldEditorMultilineParams const*>(&other);
    if (nullptr == multilineParams)
        {
        BeAssert(false);
        return -1;
        }
    if (m_spec.GetHeightInRows() < multilineParams->m_spec.GetHeightInRows())
        return -1;
    if (m_spec.GetHeightInRows() > multilineParams->m_spec.GetHeightInRows())
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorRangeParams::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int FieldEditorRangeParams::_CompareTo(Params const& other) const
    {
    int baseCompare = Params::_CompareTo(other);
    if (0 != baseCompare)
        return baseCompare;

    FieldEditorRangeParams const* rangeParams = dynamic_cast<FieldEditorRangeParams const*>(&other);
    if (nullptr == rangeParams)
        {
        BeAssert(false);
        return -1;
        }

    if (nullptr == m_spec.GetMinimumValue() && nullptr != rangeParams->m_spec.GetMinimumValue())
        return -1;
    if (nullptr != m_spec.GetMinimumValue() && nullptr == rangeParams->m_spec.GetMinimumValue())
        return 1;
    if (nullptr != m_spec.GetMinimumValue() && nullptr != rangeParams->m_spec.GetMinimumValue()
        && 0 != BeNumerical::Compare(*m_spec.GetMinimumValue(), *rangeParams->m_spec.GetMinimumValue()))
        {
        return BeNumerical::Compare(*m_spec.GetMinimumValue(), *rangeParams->m_spec.GetMinimumValue());
        }

    if (nullptr == m_spec.GetMaximumValue() && nullptr != rangeParams->m_spec.GetMaximumValue())
        return -1;
    if (nullptr != m_spec.GetMaximumValue() && nullptr == rangeParams->m_spec.GetMaximumValue())
        return 1;
    if (nullptr != m_spec.GetMaximumValue() && nullptr != rangeParams->m_spec.GetMaximumValue()
        && 0 != BeNumerical::Compare(*m_spec.GetMaximumValue(), *rangeParams->m_spec.GetMaximumValue()))
        {
        return BeNumerical::Compare(*m_spec.GetMaximumValue(), *rangeParams->m_spec.GetMaximumValue());
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorSliderParams::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int FieldEditorSliderParams::_CompareTo(Params const& other) const
    {
    int baseCompare = Params::_CompareTo(other);
    if (0 != baseCompare)
        return baseCompare;

    FieldEditorSliderParams const* sliderParams = dynamic_cast<FieldEditorSliderParams const*>(&other);
    if (nullptr == sliderParams)
        {
        BeAssert(false);
        return -1;
        }

    if (BeNumerical::Compare(m_spec.GetMinimumValue(), sliderParams->m_spec.GetMinimumValue()) < 0)
        return -1;
    if (BeNumerical::Compare(m_spec.GetMinimumValue(), sliderParams->m_spec.GetMinimumValue()) > 0)
        return 1;
    if (BeNumerical::Compare(m_spec.GetMaximumValue(), sliderParams->m_spec.GetMaximumValue()) < 0)
        return -1;
    if (BeNumerical::Compare(m_spec.GetMaximumValue(), sliderParams->m_spec.GetMaximumValue()) > 0)
        return 1;
    if (m_spec.GetIntervalsCount() < sliderParams->m_spec.GetIntervalsCount())
        return -1;
    if (m_spec.GetIntervalsCount() > sliderParams->m_spec.GetIntervalsCount())
        return 1;
    if (m_spec.GetValueFactor() < sliderParams->m_spec.GetValueFactor())
        return -1;
    if (m_spec.GetValueFactor() > sliderParams->m_spec.GetValueFactor())
        return 1;
    if (m_spec.IsVertical() < sliderParams->m_spec.IsVertical())
        return -1;
    if (m_spec.IsVertical() > sliderParams->m_spec.IsVertical())
        return 1;
    return 0;
    }
