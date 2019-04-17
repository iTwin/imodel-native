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
ContentModifier::ContentModifier() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier(Utf8String schemaName, Utf8String className)
    : m_schemaName(schemaName), m_className(className)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier(ContentModifierCR other)
    : m_className(other.m_className), m_schemaName(other.m_schemaName)
    {
    CommonToolsInternal::CopyRules(m_relatedProperties, other.m_relatedProperties);
    CommonToolsInternal::CopyRules(m_propertiesDisplaySpecification, other.m_propertiesDisplaySpecification);
    CommonToolsInternal::CopyRules(m_calculatedProperties, other.m_calculatedProperties);
    CommonToolsInternal::CopyRules(m_propertyEditors, other.m_propertyEditors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::~ContentModifier()
    {
    CommonToolsInternal::FreePresentationRules(m_relatedProperties);
    CommonToolsInternal::FreePresentationRules(m_propertiesDisplaySpecification);
    CommonToolsInternal::FreePresentationRules(m_calculatedProperties);
    CommonToolsInternal::FreePresentationRules(m_propertyEditors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentModifier::_GetXmlElementName() const {return CONTENTMODIEFIER_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifier::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_schemaName, CONTENTMODIEFIER_XML_ATTRIBUTE_SCHEMANAME))
        m_schemaName = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, CONTENTMODIEFIER_XML_ATTRIBUTE_CLASSNAME))
        m_className = "";

    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>(xmlNode, m_relatedProperties, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    CommonToolsInternal::LoadSpecificationsFromXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList>(xmlNode, m_propertiesDisplaySpecification, HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    CommonToolsInternal::LoadSpecificationsFromXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList>(xmlNode, m_propertiesDisplaySpecification, DISPLAYED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    BeXmlNodeP xmlPropertyNode = xmlNode->SelectSingleNode(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonToolsInternal::LoadSpecificationsFromXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(xmlPropertyNode, m_calculatedProperties, CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME);
    xmlPropertyNode = xmlNode->SelectSingleNode(PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonToolsInternal::LoadSpecificationsFromXmlNode<PropertyEditorsSpecification, PropertyEditorsSpecificationList>(xmlPropertyNode, m_propertyEditors, PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME);
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::_WriteXml(BeXmlNodeP xmlNode) const
    {
    PresentationKey::_WriteXml(xmlNode);

    xmlNode->AddAttributeStringValue(CONTENTMODIEFIER_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());
    xmlNode->AddAttributeStringValue(CONTENTMODIEFIER_XML_ATTRIBUTE_SCHEMANAME, m_schemaName.c_str());

    CommonToolsInternal::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>(xmlNode, m_relatedProperties);
    CommonToolsInternal::WriteRulesToXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList>(xmlNode, m_propertiesDisplaySpecification);
    if (!m_calculatedProperties.empty())
        {
        BeXmlNodeP calculatedPropertiesNode = xmlNode->AddEmptyElement(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
        CommonToolsInternal::WriteRulesToXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(calculatedPropertiesNode, m_calculatedProperties);
        }
    if (!m_propertyEditors.empty())
        {
        BeXmlNodeP propertyEditorsNode = xmlNode->AddEmptyElement(PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME);
        CommonToolsInternal::WriteRulesToXmlNode<PropertyEditorsSpecification, PropertyEditorsSpecificationList>(propertyEditorsNode, m_propertyEditors);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentModifier::_GetJsonElementType() const
    {
    return CONTENTMODIEFIER_RULE_JSON_TYPE;
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
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIEFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES], 
        m_relatedProperties, CommonToolsInternal::LoadRuleFromJson<RelatedPropertiesSpecification>);
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIEFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS], 
        m_propertiesDisplaySpecification, CommonToolsInternal::LoadRuleFromJson<PropertiesDisplaySpecification>);
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIEFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES], 
        m_calculatedProperties, CommonToolsInternal::LoadRuleFromJson<CalculatedPropertiesSpecification>);
    CommonToolsInternal::LoadFromJson(json[CONTENTMODIEFIER_JSON_ATTRIBUTE_PROPERTYEDITORS], 
        m_propertyEditors, CommonToolsInternal::LoadRuleFromJson<PropertyEditorsSpecification>);
    return true;
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
    if (!m_calculatedProperties.empty())
        {
        CommonToolsInternal::WriteRulesToJson<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>
            (json[CONTENTMODIEFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES], m_calculatedProperties);
        }
    if (!m_relatedProperties.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>
            (json[CONTENTMODIEFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES], m_relatedProperties);
        }
    if (!m_propertiesDisplaySpecification.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList>
            (json[CONTENTMODIEFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS], m_propertiesDisplaySpecification);
        }
    if (!m_propertyEditors.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertyEditorsSpecification, PropertyEditorsSpecificationList>
            (json[CONTENTMODIEFIER_JSON_ATTRIBUTE_PROPERTYEDITORS], m_propertyEditors);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentModifier::GetClassName() const {return m_className;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentModifier::GetSchemaName() const {return m_schemaName;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList const& ContentModifier::GetRelatedProperties() const {return m_relatedProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::AddRelatedProperty(RelatedPropertiesSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_relatedProperties.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PropertiesDisplaySpecificationList const& ContentModifier::GetPropertiesDisplaySpecifications() const {return m_propertiesDisplaySpecification;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::AddPropertiesDisplaySpecification(PropertiesDisplaySpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_propertiesDisplaySpecification.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertiesSpecificationList const& ContentModifier::GetCalculatedProperties() const {return m_calculatedProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::AddCalculatedProperty(CalculatedPropertiesSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_calculatedProperties.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyEditorsSpecificationList const& ContentModifier::GetPropertyEditors() const {return m_propertyEditors;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::AddPropertyEditor(PropertyEditorsSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_propertyEditors.push_back(&specification);
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
    for (RelatedPropertiesSpecificationP spec : m_relatedProperties)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (PropertiesDisplaySpecificationP spec : m_propertiesDisplaySpecification)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (CalculatedPropertiesSpecificationP spec : m_calculatedProperties)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (PropertyEditorsSpecificationP spec : m_propertyEditors)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }
