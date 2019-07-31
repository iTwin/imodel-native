/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CustomizationRuleP CustomizationRule::Create(JsonValueCR json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_RULETYPE].asCString("");
    CustomizationRuleP spec = nullptr;
    if (0 == strcmp(IMAGE_ID_OVERRIDE_JSON_TYPE, type))
        spec = new ImageIdOverride();
    else if (0 == strcmp(LABEL_OVERRIDE_JSON_TYPE, type))
        spec = new LabelOverride();
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_JSON_TYPE, type))
        spec = new InstanceLabelOverride();
    else if (0 == strcmp(STYLE_OVERRIDE_JSON_TYPE, type))
        spec = new StyleOverride();
    else if (0 == strcmp(CHECKBOX_RULE_JSON_TYPE, type))
        spec = new CheckBoxRule();
    else if (0 == strcmp(SORTING_RULE_DISABLEDSORTING_JSON_TYPE, type) || 0 == strcmp(SORTING_RULE_PROPERTYSORTING_JSON_TYPE, type))
        spec = new SortingRule();
    else if (0 == strcmp(GROUPING_RULE_JSON_TYPE, type))
        spec = new GroupingRule();
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ConditionalCustomizationRule::GetCondition() const { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalCustomizationRule::SetCondition(Utf8String value) { m_condition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConditionalCustomizationRule::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!CustomizationRule::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_condition, PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION))
        m_condition = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalCustomizationRule::_WriteXml(BeXmlNodeP xmlNode) const
    {
    CustomizationRule::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConditionalCustomizationRule::_ReadJson(JsonValueCR json)
    {
    if (!CustomizationRule::_ReadJson(json))
        return false;

    m_condition = json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION].asString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalCustomizationRule::_WriteJson(JsonValueR json) const
    {
    CustomizationRule::_WriteJson(json);
    if (!m_condition.empty())
        json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION] = m_condition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ConditionalCustomizationRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = CustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_condition.c_str(), m_condition.size());
    return md5;
    }