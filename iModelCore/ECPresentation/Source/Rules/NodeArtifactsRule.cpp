/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NodeArtifactsRule::_GetXmlElementName() const { return ""; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodeArtifactsRule::_ReadXml(BeXmlNodeP) { return false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::_WriteXml(BeXmlNodeP) const {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NodeArtifactsRule::_GetJsonElementType() const { return NODE_ARTIFACTS_RULE_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodeArtifactsRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    JsonValueCR itemsObject = json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS];
    bool isItemsObjectValid = itemsObject.isObject() && !itemsObject.isNull();
    if (CommonToolsInternal::CheckRuleIssue(!isItemsObjectValid, _GetJsonElementType(), NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS, json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS], "JSON object"))
        return false;

    auto memberNames = itemsObject.getMemberNames();
    for (Utf8StringCR memberName : memberNames)
        {
        JsonValueCR value = itemsObject[memberName];
        if (CommonToolsInternal::CheckRuleIssue(!value.isString(), _GetJsonElementType(), Utf8PrintfString("%s.%s", NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS, memberName.c_str()).c_str(), value, "string"))
            continue;

        m_items.Insert(memberName, value.asCString());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::_WriteJson(JsonValueR json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    for (auto entry : m_items)
        json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS][entry.first] = entry.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 NodeArtifactsRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    for (auto entry : m_items)
        {
        md5.Add(entry.first.c_str(), entry.first.size());
        md5.Add(entry.second.c_str(), entry.second.size());
        }
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodeArtifactsRule::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ConditionalCustomizationRule::_ShallowEqual(other))
        return false;

    NodeArtifactsRule const* otherRule = dynamic_cast<NodeArtifactsRule const*>(&other);
    if (nullptr == otherRule)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::SetItemsMap(bmap<Utf8String, Utf8String> map)
    {
    m_items = map;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::AddItem(Utf8String key, Utf8String value)
    {
    m_items.Insert(key, value);
    InvalidateHash();
    }
