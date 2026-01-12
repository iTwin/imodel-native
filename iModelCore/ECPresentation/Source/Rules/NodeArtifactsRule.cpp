/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NodeArtifactsRule::_GetJsonElementType() const { return NODE_ARTIFACTS_RULE_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodeArtifactsRule::_ReadJson(BeJsConst json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    BeJsConst itemsObject = json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS];
    bool isItemsObjectValid = itemsObject.isObject() && !itemsObject.isNull();
    if (CommonToolsInternal::CheckRuleIssue(!isItemsObjectValid, _GetJsonElementType(), NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS, json[NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS], "JSON object"))
        return false;

    itemsObject.ForEachProperty(
        [&](Utf8CP name, BeJsConst value)
        {
        if (CommonToolsInternal::CheckRuleIssue(!value.isString(), _GetJsonElementType(), Utf8PrintfString("%s.%s", NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS, name).c_str(), value, "string"))
            return false;
        m_items.Insert(name, value.asCString());
        return false;
        }
    );

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeArtifactsRule::_WriteJson(BeJsValue json) const
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
