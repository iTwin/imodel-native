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
bool CalculatedPropertiesSpecification::_ReadJson(JsonValueCR json)
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

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculatedPropertiesSpecification::_WriteJson(JsonValueR json) const
    {
    PrioritizedPresentationKey::_WriteJson(json);
    json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE] = m_value;
    json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL] = m_label;
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
        && m_value == otherRule->m_value;
    }
