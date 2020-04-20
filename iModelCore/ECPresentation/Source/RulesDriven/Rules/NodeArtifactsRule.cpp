/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NodeArtifactsRule::_GetXmlElementName() const { return ""; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodeArtifactsRule::_ReadXml(BeXmlNodeP) { return false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::_WriteXml(BeXmlNodeP) const {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NodeArtifactsRule::_GetJsonElementType() const { return NODE_ARTIFACTS_RULE_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodeArtifactsRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    if (!json.isMember(NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS) || !json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS].isObject())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "NodeArtifactsRule", NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS);
        return false;
        }

    auto memberNames = json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS].getMemberNames();
    for (Utf8StringCR memberName : memberNames)
        {
        JsonValueCR value = json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS][memberName];
        if (!value.isString())
            {
            ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "NodeArtifactsRule", Utf8PrintfString("%s.%s", NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS, memberName.c_str()).c_str());
            continue;
            }
        m_items.Insert(memberName, value.asCString());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::_WriteJson(JsonValueR json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    for (auto entry : m_items)
        json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS][entry.first] = entry.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 NodeArtifactsRule::_ComputeHash() const
    {
    MD5 md5 = ConditionalCustomizationRule::_ComputeHash();
    for (auto entry : m_items)
        {
        md5.Add(entry.first.c_str(), entry.first.size());
        md5.Add(entry.second.c_str(), entry.second.size());
        }
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::SetItemsMap(bmap<Utf8String, Utf8String> map)
    {
    m_items = map;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::AddItem(Utf8String key, Utf8String value)
    {
    m_items.Insert(key, value);
    InvalidateHash();
    }
