/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* note: this is only needed to support deprecated PropertyEditorsSpecification
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertySpecification::ReadEditorSpecificationJson(JsonValueCR json)
    {
    //Required
    JsonValueCR propertyNameJson = json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME];
    if (propertyNameJson.isNull() || !propertyNameJson.isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyEditorsSpecification", PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME);
        return false;
        }
    m_propertyName = propertyNameJson.asCString();

    m_editorOverride = CommonToolsInternal::LoadRuleFromJson<PropertyEditorSpecification>(json);
    if (nullptr == m_editorOverride)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertySpecification::ReadJson(JsonValueCR json)
    {
    //Required
    JsonValueCR propertyNameJson = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME];
    if (propertyNameJson.isNull() || !propertyNameJson.isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertySpecification", PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME);
        return false;
        }
    m_propertyName = propertyNameJson.asCString();

    //Optional
    m_overridesPriority = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_OVERRIDESPRIORITY].asInt(1000);
    m_labelOverride = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_LABELOVERRIDE].asCString("");
    m_categoryId = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID].asCString("");

    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED))
        m_isDisplayed = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED].asBool();
    else
        m_isDisplayed = nullptr;

    DELETE_AND_CLEAR(m_editorOverride);
    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_EDITOR))
        m_editorOverride = CommonToolsInternal::LoadRuleFromJson<PropertyEditorSpecification>(json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_EDITOR]);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PropertySpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME] = m_propertyName;
    if (m_overridesPriority != 1000)
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_OVERRIDESPRIORITY] = m_overridesPriority;
    if (!m_labelOverride.empty())
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_LABELOVERRIDE] = m_labelOverride;
    if (m_isDisplayed.IsValid())
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED] = m_isDisplayed.Value();
    if (nullptr != m_editorOverride)
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_EDITOR] = m_editorOverride->WriteJson();
    if (!m_categoryId.empty())
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID] = m_categoryId;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertySpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_propertyName.c_str(), m_propertyName.size());
    md5.Add(&m_overridesPriority, sizeof(m_overridesPriority));
    md5.Add(m_labelOverride.c_str(), m_labelOverride.size());
    md5.Add(m_categoryId.c_str(), m_categoryId.size());
    if (m_isDisplayed.IsValid())
        md5.Add(&m_isDisplayed.Value(), sizeof(m_isDisplayed.Value()));
    if (nullptr != m_editorOverride)
        {
        Utf8String editorOverrideHash = m_editorOverride->GetHash();
        md5.Add(editorOverrideHash.c_str(), editorOverrideHash.size());
        }
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }
