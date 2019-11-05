/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifiersList::ContentModifiersList(ContentModifiersList const& other)
    {
    CommonToolsInternal::CopyRules(m_relatedProperties, other.m_relatedProperties, this);
    CommonToolsInternal::CopyRules(m_calculatedProperties, other.m_calculatedProperties, this);
    CommonToolsInternal::CopyRules(m_propertyOverrides, other.m_propertyOverrides, this);
    CommonToolsInternal::CopyRules(m_propertyCategories, other.m_propertyCategories, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifiersList::ContentModifiersList(ContentModifiersList&& other)
    {
    CommonToolsInternal::SwapRules(m_relatedProperties, other.m_relatedProperties, this);
    CommonToolsInternal::SwapRules(m_calculatedProperties, other.m_calculatedProperties, this);
    CommonToolsInternal::SwapRules(m_propertyOverrides, other.m_propertyOverrides, this);
    CommonToolsInternal::SwapRules(m_propertyCategories, other.m_propertyCategories, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifiersList::~ContentModifiersList()
    {
    CommonToolsInternal::FreePresentationRules(m_relatedProperties);
    CommonToolsInternal::FreePresentationRules(m_calculatedProperties);
    CommonToolsInternal::FreePresentationRules(m_propertyOverrides);
    CommonToolsInternal::FreePresentationRules(m_propertyCategories);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::AddRelatedProperty(RelatedPropertiesSpecificationR specification) { ADD_HASHABLE_CHILD(m_relatedProperties, specification); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::AddCalculatedProperty(CalculatedPropertiesSpecificationR specification) { ADD_HASHABLE_CHILD(m_calculatedProperties, specification); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::AddPropertyOverride(PropertySpecificationR specification) { ADD_HASHABLE_CHILD(m_propertyOverrides, specification); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::AddPropertyCategory(PropertyCategorySpecificationR specification) { ADD_HASHABLE_CHILD(m_propertyCategories, specification); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifiersList::ReadXml(BeXmlNodeP xmlNode)
    {
    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>(xmlNode, m_relatedProperties, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME, this);
    BeXmlNodeP xmlPropertyNode = xmlNode->SelectSingleNode(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonToolsInternal::LoadSpecificationsFromXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(xmlPropertyNode, m_calculatedProperties, CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::WriteXml(BeXmlNodeP xmlNode) const
    {
    CommonToolsInternal::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>(xmlNode, m_relatedProperties);
    if (!m_calculatedProperties.empty())
        {
        BeXmlNodeP calculatedPropertiesNode = xmlNode->AddEmptyElement(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
        CommonToolsInternal::WriteRulesToXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(calculatedPropertiesNode, m_calculatedProperties);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static PropertySpecificationP CreatePropertyEditorSpecification(JsonValueCR json)
    {
    return CommonToolsInternal::LoadRuleFromJson(json, &PropertySpecification::ReadEditorSpecificationJson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void LoadPropertyDisplaySpecifications(JsonValueCR json, bvector<PropertySpecificationP>& specs, HashableBase const* parentSpec)
    {
    for (Json::ArrayIndex i = 0; i < json.size(); ++i)
        {
        JsonValueCR propertyNamesJson = json[i][PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES];
        if (propertyNamesJson.isNull() || !propertyNamesJson.isArray() || 0 == propertyNamesJson.size())
            continue;

        int priority = json[i][PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY].asInt(1000);
        bool isDisplayed = json[i][PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED].asBool(true);
        for (Json::ArrayIndex j = 0; j < propertyNamesJson.size(); ++j)
            {
            Utf8CP propertyName = propertyNamesJson[j].asCString();
            specs.push_back(new PropertySpecification(propertyName, priority, "", "", isDisplayed, nullptr));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifiersList::ReadJson(JsonValueCR json)
    {
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES],
        m_relatedProperties, CommonToolsInternal::LoadRuleFromJson<RelatedPropertiesSpecification>, this);
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES],
        m_calculatedProperties, CommonToolsInternal::LoadRuleFromJson<CalculatedPropertiesSpecification>, this);
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYCATEGORIES],
        m_propertyCategories, CommonToolsInternal::LoadRuleFromJson<PropertyCategorySpecification>, this);
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES],
        m_propertyOverrides, CommonToolsInternal::LoadRuleFromJson<PropertySpecification>, this);
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYEDITORS],
        m_propertyOverrides, &CreatePropertyEditorSpecification, this);
    LoadPropertyDisplaySpecifications(json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS],
        m_propertyOverrides, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::WriteJson(JsonValueR json) const
    {
    if (!m_calculatedProperties.empty())
        {
        CommonToolsInternal::WriteRulesToJson<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>
            (json[CONTENTMODIFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES], m_calculatedProperties);
        }
    if (!m_relatedProperties.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>
            (json[CONTENTMODIFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES], m_relatedProperties);
        }
    if (!m_propertyCategories.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertyCategorySpecification, PropertyCategorySpecificationsList>
            (json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYCATEGORIES], m_propertyCategories);
        }
    if (!m_propertyOverrides.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertySpecification, PropertySpecificationsList>
            (json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES], m_propertyOverrides);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentModifiersList::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    for (RelatedPropertiesSpecificationP spec : m_relatedProperties)
        {
        Utf8StringCR specHash = spec->GetHash(parentHash);
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (CalculatedPropertiesSpecificationP spec : m_calculatedProperties)
        {
        Utf8StringCR specHash = spec->GetHash(parentHash);
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (PropertySpecificationP spec : m_propertyOverrides)
        {
        Utf8StringCR specHash = spec->GetHash(parentHash);
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (PropertyCategorySpecificationP spec : m_propertyCategories)
        {
        Utf8StringCR specHash = spec->GetHash(parentHash);
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentModifier::_GetXmlElementName() const {return CONTENTMODIFIER_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifier::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_schemaName, CONTENTMODIFIER_XML_ATTRIBUTE_SCHEMANAME))
        m_schemaName = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, CONTENTMODIFIER_XML_ATTRIBUTE_CLASSNAME))
        m_className = "";

    return m_modifiers.ReadXml(xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::_WriteXml(BeXmlNodeP xmlNode) const
    {
    PresentationKey::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(CONTENTMODIFIER_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());
    xmlNode->AddAttributeStringValue(CONTENTMODIFIER_XML_ATTRIBUTE_SCHEMANAME, m_schemaName.c_str());
    m_modifiers.WriteXml(xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentModifier::_GetJsonElementType() const
    {
    return CONTENTMODIFIER_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifier::_ReadJson(JsonValueCR json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    if (!json.isMember(COMMON_JSON_ATTRIBUTE_RULETYPE) || !json[COMMON_JSON_ATTRIBUTE_RULETYPE].isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "ContentModifier", COMMON_JSON_ATTRIBUTE_RULETYPE);
        return false;
        }
    if (0 != strcmp(json[COMMON_JSON_ATTRIBUTE_RULETYPE].asCString(), _GetJsonElementType()))
        return false;

    CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_className, json[COMMON_JSON_ATTRIBUTE_CLASS]);
    return m_modifiers.ReadJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::_WriteJson(JsonValueR json) const
    {
    PresentationKey::_WriteJson(json);
    json[COMMON_JSON_ATTRIBUTE_RULETYPE] = _GetJsonElementType();
    if (!m_schemaName.empty() && !m_className.empty())
        json[COMMON_JSON_ATTRIBUTE_CLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_schemaName, m_className);
    m_modifiers.WriteJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentModifier::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = PresentationKey::_ComputeHash(parentHash);
    md5.Add(m_schemaName.c_str(), m_schemaName.size());
    md5.Add(m_className.c_str(), m_className.size());

    Utf8String currentHash = md5.GetHashString();
    Utf8StringCR modifiersHash = m_modifiers.GetHash(currentHash.c_str());
    md5.Add(modifiersHash.c_str(), modifiersHash.size());
    return md5;
    }
