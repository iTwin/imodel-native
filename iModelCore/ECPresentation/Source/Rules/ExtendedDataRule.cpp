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
Utf8CP ExtendedDataRule::_GetXmlElementName() const { return ""; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtendedDataRule::_ReadXml(BeXmlNodeP) { return false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedDataRule::_WriteXml(BeXmlNodeP) const {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ExtendedDataRule::_GetJsonElementType() const { return EXTENDED_DATA_RULE_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtendedDataRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    JsonValueCR itemsObject = json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS];
    bool isItemsObjectValid = itemsObject.isObject() && !itemsObject.isNull();
    if (CommonToolsInternal::CheckRuleIssue(!isItemsObjectValid, _GetJsonElementType(), EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS, json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS], "JSON object"))
        return false;

    auto memberNames = json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS].getMemberNames();
    for (Utf8StringCR memberName : memberNames)
        {
        JsonValueCR value = json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS][memberName];
        if (CommonToolsInternal::CheckRuleIssue(!value.isString(), _GetJsonElementType(), Utf8PrintfString("value.items.%s", memberName.c_str()).c_str(), value, "non-empty string"))
            continue;

        m_items.Insert(memberName, value.asCString());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedDataRule::_WriteJson(JsonValueR json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    for (auto entry : m_items)
        json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS][entry.first] = entry.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ExtendedDataRule::_ComputeHash() const
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
void ExtendedDataRule::SetItemsMap(bmap<Utf8String, Utf8String> map)
    {
    m_items = map;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedDataRule::AddItem(Utf8String key, Utf8String value)
    {
    m_items.Insert(key, value);
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtendedDataRule::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ConditionalCustomizationRule::_ShallowEqual(other))
        return false;

    ExtendedDataRule const* otherRule = dynamic_cast<ExtendedDataRule const*>(&other);
    if (nullptr == otherRule)
        return false;

    return true;
    }
