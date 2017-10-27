/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentFieldEditors.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ContentFieldEditors.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool FieldEditorJsonParams::_Equals(Params const& other) const
    {
    if (!Params::_Equals(other))
        return false;

    FieldEditorJsonParams const* jsonParams = dynamic_cast<FieldEditorJsonParams const*>(&other);
    if (nullptr == jsonParams)
        {
        BeAssert(false);
        return false;
        }
    return m_json == jsonParams->m_json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorJsonParams::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.CopyFrom(m_json, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool FieldEditorMultilineParams::_Equals(Params const& other) const
    {
    if (!Params::_Equals(other))
        return false;

    FieldEditorMultilineParams const* multilineParams = dynamic_cast<FieldEditorMultilineParams const*>(&other);
    if (nullptr == multilineParams)
        {
        BeAssert(false);
        return false;
        }
    return m_spec.GetHeightInRows() == multilineParams->m_spec.GetHeightInRows();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorMultilineParams::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("HeightInRows", m_spec.GetHeightInRows(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool FieldEditorRangeParams::_Equals(Params const& other) const
    {
    if (!Params::_Equals(other))
        return false;

    FieldEditorRangeParams const* rangeParams = dynamic_cast<FieldEditorRangeParams const*>(&other);
    if (nullptr == rangeParams)
        {
        BeAssert(false);
        return false;
        }

    if (nullptr == m_spec.GetMinimumValue() && nullptr != rangeParams->m_spec.GetMinimumValue())
        return false;
    if (nullptr != m_spec.GetMinimumValue() && nullptr == rangeParams->m_spec.GetMinimumValue())
        return false;
    if (nullptr != m_spec.GetMinimumValue() && nullptr != rangeParams->m_spec.GetMinimumValue()
        && 0 != BeNumerical::Compare(*m_spec.GetMinimumValue(), *rangeParams->m_spec.GetMinimumValue()))
        {
        return false;
        }

    if (nullptr == m_spec.GetMaximumValue() && nullptr != rangeParams->m_spec.GetMaximumValue())
        return false;
    if (nullptr != m_spec.GetMaximumValue() && nullptr == rangeParams->m_spec.GetMaximumValue())
        return false;
    if (nullptr != m_spec.GetMaximumValue() && nullptr != rangeParams->m_spec.GetMaximumValue()
        && 0 != BeNumerical::Compare(*m_spec.GetMaximumValue(), *rangeParams->m_spec.GetMaximumValue()))
        {
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorRangeParams::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    if (nullptr != m_spec.GetMinimumValue())
        json.AddMember("Minimum", *m_spec.GetMinimumValue(), json.GetAllocator());
    else
        json.AddMember("Minimum", rapidjson::Value(), json.GetAllocator());
    
    if (nullptr != m_spec.GetMaximumValue())
        json.AddMember("Maximum", *m_spec.GetMaximumValue(), json.GetAllocator());
    else
        json.AddMember("Maximum", rapidjson::Value(), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool FieldEditorSliderParams::_Equals(Params const& other) const
    {
    if (!Params::_Equals(other))
        return false;

    FieldEditorSliderParams const* sliderParams = dynamic_cast<FieldEditorSliderParams const*>(&other);
    if (nullptr == sliderParams)
        {
        BeAssert(false);
        return false;
        }

    return 0 == BeNumerical::Compare(m_spec.GetMinimumValue(), sliderParams->m_spec.GetMinimumValue())
        && 0 == BeNumerical::Compare(m_spec.GetMaximumValue(), sliderParams->m_spec.GetMaximumValue())
        && m_spec.GetIntervalsCount() == sliderParams->m_spec.GetIntervalsCount()
        && m_spec.GetValueFactor() == sliderParams->m_spec.GetValueFactor()
        && m_spec.IsVertical() == sliderParams->m_spec.IsVertical();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorSliderParams::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Minimum", m_spec.GetMinimumValue(), json.GetAllocator());
    json.AddMember("Maximum", m_spec.GetMaximumValue(), json.GetAllocator());
    json.AddMember("IntervalsCount", m_spec.GetIntervalsCount(), json.GetAllocator());
    json.AddMember("ValueFactor", m_spec.GetValueFactor(), json.GetAllocator());
    json.AddMember("IsVertical", m_spec.IsVertical(), json.GetAllocator());
    return json;
    }