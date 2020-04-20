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
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ExtendedDataRule::_GetXmlElementName() const { return ""; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtendedDataRule::_ReadXml(BeXmlNodeP) { return false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedDataRule::_WriteXml(BeXmlNodeP) const {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ExtendedDataRule::_GetJsonElementType() const { return EXTENDED_DATA_RULE_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtendedDataRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    if (!json.isMember(EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS) || !json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS].isObject())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "ExtendedDataRule", EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS);
        return false;
        }

    auto memberNames = json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS].getMemberNames();
    for (Utf8StringCR memberName : memberNames)
        {
        JsonValueCR value = json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS][memberName];
        if (!value.isString())
            {
            ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "ExtendedDataRule", Utf8PrintfString("%s.%s", EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS, memberName.c_str()).c_str());
            continue;
            }
        m_items.Insert(memberName, value.asCString());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedDataRule::_WriteJson(JsonValueR json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    for (auto entry : m_items)
        json[EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS][entry.first] = entry.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ExtendedDataRule::_ComputeHash() const
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
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedDataRule::SetItemsMap(bmap<Utf8String, Utf8String> map)
    {
    m_items = map;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedDataRule::AddItem(Utf8String key, Utf8String value)
    {
    m_items.Insert(key, value);
    InvalidateHash();
    }
