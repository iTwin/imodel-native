/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyCategorySpecification::ReadJson(JsonValueCR json)
    {
    // required
    m_id = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID].asCString("");
    if (m_id.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyCategorySpecification", PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID);
        return false;
        }

    m_label = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL].asCString("");
    if (m_label.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyCategorySpecification", PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL);
        return false;
        }

    // optional
    m_description = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION].asCString("");
    m_priority = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY].asInt(1000);
    m_autoExpand = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_AUTOEXPAND].asBool();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PropertyCategorySpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID] = m_id;
    json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL] = m_label;
    if (!m_description.empty())
        json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION] = m_description;
    if (m_priority != 1000)
        json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY] = m_priority;
    if (m_autoExpand)
        json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_AUTOEXPAND] = m_autoExpand;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyCategorySpecification::_ComputeHash() const
    {
    MD5 md5;
    md5.Add(m_id.c_str(), m_id.size());
    md5.Add(m_label.c_str(), m_label.size());
    md5.Add(m_description.c_str(), m_description.size());
    md5.Add(&m_priority, sizeof(m_priority));
    md5.Add(&m_autoExpand, sizeof(m_autoExpand));
    return md5;
    }
