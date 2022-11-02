/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/ContentFieldEditors.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorJsonParams::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int FieldEditorJsonParams::_CompareTo(Params const& other) const
    {
    int baseCompare = Params::_CompareTo(other);
    if (0 != baseCompare)
        return baseCompare;

    FieldEditorJsonParams const* jsonParams = dynamic_cast<FieldEditorJsonParams const*>(&other);
    if (nullptr == jsonParams)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Trying to compare field editor JSON params with other type of params, although base compare succeeded");

    Utf8String lhs = BeRapidJsonUtilities::ToString(m_json);
    Utf8String rhs = BeRapidJsonUtilities::ToString(jsonParams->m_json);
    return strcmp(lhs.c_str(), rhs.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorMultilineParams::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int FieldEditorMultilineParams::_CompareTo(Params const& other) const
    {
    int baseCompare = Params::_CompareTo(other);
    if (0 != baseCompare)
        return baseCompare;

    FieldEditorMultilineParams const* multilineParams = dynamic_cast<FieldEditorMultilineParams const*>(&other);
    if (nullptr == multilineParams)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Trying to compare field editor multiline params with other type of params, although base compare succeeded");

    if (m_spec.GetHeightInRows() < multilineParams->m_spec.GetHeightInRows())
        return -1;
    if (m_spec.GetHeightInRows() > multilineParams->m_spec.GetHeightInRows())
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorRangeParams::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int FieldEditorRangeParams::_CompareTo(Params const& other) const
    {
    int baseCompare = Params::_CompareTo(other);
    if (0 != baseCompare)
        return baseCompare;

    FieldEditorRangeParams const* rangeParams = dynamic_cast<FieldEditorRangeParams const*>(&other);
    if (nullptr == rangeParams)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Trying to compare field editor range params with other type of params, although base compare succeeded");

    if (m_spec.GetMinimumValue().IsNull() && !rangeParams->m_spec.GetMinimumValue().IsNull())
        return -1;
    if (!m_spec.GetMinimumValue().IsNull() && rangeParams->m_spec.GetMinimumValue().IsNull())
        return 1;
    if (!m_spec.GetMinimumValue().IsNull() && !rangeParams->m_spec.GetMinimumValue().IsNull())
        {
        int comp = BeNumerical::Compare(m_spec.GetMinimumValue().Value(), rangeParams->m_spec.GetMinimumValue().Value());
        if (0 != comp)
            return comp;
        }

    if (m_spec.GetMaximumValue().IsNull() && !rangeParams->m_spec.GetMaximumValue().IsNull())
        return -1;
    if (!m_spec.GetMaximumValue().IsNull() && rangeParams->m_spec.GetMaximumValue().IsNull())
        return 1;
    if (!m_spec.GetMaximumValue().IsNull() && !rangeParams->m_spec.GetMaximumValue().IsNull())
        {
        int comp = BeNumerical::Compare(m_spec.GetMaximumValue().Value(), rangeParams->m_spec.GetMaximumValue().Value());
        if (0 != comp)
            return comp;
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document FieldEditorSliderParams::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int FieldEditorSliderParams::_CompareTo(Params const& other) const
    {
    int baseCompare = Params::_CompareTo(other);
    if (0 != baseCompare)
        return baseCompare;

    FieldEditorSliderParams const* sliderParams = dynamic_cast<FieldEditorSliderParams const*>(&other);
    if (nullptr == sliderParams)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Trying to compare field editor slider params with other type of params, although base compare succeeded");

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
