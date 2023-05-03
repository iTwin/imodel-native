/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/CommonTools.h>
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertiesSpecification::CalculatedPropertiesSpecification(CalculatedPropertiesSpecification const& other)
    : T_Super(other), m_label(other.m_label), m_value(other.m_value),
    m_renderer(other.m_renderer ? new CustomRendererSpecification(*other.m_renderer) : nullptr),
    m_editor(other.m_editor ? new PropertyEditorSpecification(*other.m_editor) : nullptr)
    {
    if (other.m_categoryId)
        m_categoryId = other.m_categoryId->Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertiesSpecification::CalculatedPropertiesSpecification(CalculatedPropertiesSpecification&& other)
    : T_Super(std::move(other)), m_label(std::move(other.m_label)), m_value(std::move(other.m_value)), m_categoryId(std::move(other.m_categoryId)),
    m_renderer(other.m_renderer), m_editor(other.m_editor)
    {
    other.m_renderer = nullptr;
    other.m_editor = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CalculatedPropertiesSpecification::_GetXmlElementName() const {return CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CalculatedPropertiesSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PrioritizedPresentationKey::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_label, CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_LABEL))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME, CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_LABEL));
        return false;
        }

    if (BEXML_Success != xmlNode->GetContent(m_value) || m_value.empty())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculatedPropertiesSpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    PrioritizedPresentationKey::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_LABEL, m_label.c_str());
    xmlNode->SetContentFast(m_value.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CalculatedPropertiesSpecification::_GetJsonElementType() const {return "CalculatedPropertiesSpecification";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CalculatedPropertiesSpecification::_ReadJson(BeJsConst json)
    {
    if (!PrioritizedPresentationKey::_ReadJson(json))
        return false;

    // required:
    m_value = json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE].asCString("");
    m_label = json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL].asCString("");

    bool hasIssues = false
        || CommonToolsInternal::CheckRuleIssue(m_value.empty(), _GetJsonElementType(), CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE, json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE], "non-empty string")
        || CommonToolsInternal::CheckRuleIssue(m_label.empty(), _GetJsonElementType(), CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL, json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL], "non-empty string");
    if (hasIssues)
        return false;

    if (json.hasMember(CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID))
        m_categoryId = PropertyCategoryIdentifier::Create(json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID]);
    if (json.hasMember(CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RENDERER))
        m_renderer = CommonToolsInternal::LoadRuleFromJson<CustomRendererSpecification>(json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RENDERER]);
    if (json.hasMember(CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_EDITOR))
        m_editor = CommonToolsInternal::LoadRuleFromJson<PropertyEditorSpecification>(json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_EDITOR]);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculatedPropertiesSpecification::_WriteJson(BeJsValue json) const
    {
    PrioritizedPresentationKey::_WriteJson(json);
    json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE] = m_value;
    json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL] = m_label;

    if (nullptr != m_renderer)
        m_renderer->WriteJson(json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RENDERER]);
    if (nullptr != m_editor)
        m_editor->WriteJson(json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_EDITOR]);
    if (nullptr != m_categoryId)
        m_categoryId->WriteJson(json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculatedPropertiesSpecification::SetRenderer(CustomRendererSpecificationP renderer)
    {
    if (renderer == m_renderer)
        return;

    InvalidateHash();
    DELETE_AND_CLEAR(m_renderer);
    if (renderer)
        renderer->SetParent(this);
    m_renderer = renderer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculatedPropertiesSpecification::SetEditor(PropertyEditorSpecificationP editor)
    {
    if (editor == m_editor)
        return;

    InvalidateHash();
    DELETE_AND_CLEAR(m_editor);
    if (editor)
        editor->SetParent(this);
    m_editor = editor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 CalculatedPropertiesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_label.empty())
        ADD_STR_VALUE_TO_HASH(md5, CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL, m_label);
    if (!m_value.empty())
        ADD_STR_VALUE_TO_HASH(md5, CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE, m_value);
    if (nullptr != m_renderer)
        ADD_STR_VALUE_TO_HASH(md5, CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RENDERER, m_renderer->GetHash());
    if (nullptr != m_editor)
        ADD_STR_VALUE_TO_HASH(md5, CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_EDITOR, m_editor->GetHash());
    if (nullptr != m_categoryId)
        ADD_STR_VALUE_TO_HASH(md5, CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID, m_categoryId->GetHash());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CalculatedPropertiesSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!PrioritizedPresentationKey::_ShallowEqual(other))
        return false;

    CalculatedPropertiesSpecification const* otherRule = dynamic_cast<CalculatedPropertiesSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_label == otherRule->m_label
        && m_value == otherRule->m_value
        && m_categoryId == otherRule->m_categoryId
        && m_renderer == otherRule->m_renderer
        && m_editor == otherRule->m_editor;
    }
