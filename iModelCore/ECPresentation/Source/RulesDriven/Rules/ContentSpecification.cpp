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
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationP ContentSpecification::Create(JsonValueCR json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString("");
    ContentSpecificationP spec = nullptr;
    if (0 == strcmp(SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_TYPE, type))
        spec = new SelectedNodeInstancesSpecification();
    else if (0 == strcmp(CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE, type))
        spec = new ContentInstancesOfSpecificClassesSpecification();
    else if (0 == strcmp(CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_TYPE, type))
        spec = new ContentRelatedInstancesSpecification();
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification() : m_priority(1000), m_showImages(false) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(int priority, bool showImages)
    : m_priority(priority), m_showImages(showImages)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(ContentSpecificationCR other)
    : m_priority(other.m_priority), m_showImages(other.m_showImages)
    {
    CommonToolsInternal::CopyRules(m_relatedPropertiesSpecification, other.m_relatedPropertiesSpecification, this);
    CommonToolsInternal::CopyRules(m_propertiesDisplaySpecification, other.m_propertiesDisplaySpecification, this);
    CommonToolsInternal::CopyRules(m_calculatedPropertiesSpecification, other.m_calculatedPropertiesSpecification, this);
    CommonToolsInternal::CopyRules(m_propertyEditorsSpecification, other.m_propertyEditorsSpecification, this);
    CommonToolsInternal::CopyRules(m_relatedInstances, other.m_relatedInstances, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::~ContentSpecification ()
    {
    CommonToolsInternal::FreePresentationRules(m_relatedPropertiesSpecification);
    CommonToolsInternal::FreePresentationRules(m_propertiesDisplaySpecification);
    CommonToolsInternal::FreePresentationRules(m_calculatedPropertiesSpecification);
    CommonToolsInternal::FreePresentationRules(m_propertyEditorsSpecification);
    CommonToolsInternal::FreePresentationRules(m_relatedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationRuleSpecification::_ReadXml(xmlNode))
        return false;

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value(m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_showImages, CONTENT_SPECIFICATION_XML_ATTRIBUTE_SHOWIMAGES))
        m_showImages = false;

    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_relatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME, this);
    CommonToolsInternal::LoadSpecificationsFromXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList> (xmlNode, m_propertiesDisplaySpecification, HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME, this);
    CommonToolsInternal::LoadSpecificationsFromXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList> (xmlNode, m_propertiesDisplaySpecification, DISPLAYED_PROPERTIES_SPECIFICATION_XML_NODE_NAME, this);
    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedInstanceSpecification, RelatedInstanceSpecificationList> (xmlNode, m_relatedInstances, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, this);
    BeXmlNodeP xmlPropertyNode = xmlNode->SelectSingleNode(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonToolsInternal::LoadSpecificationsFromXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList> (xmlPropertyNode, m_calculatedPropertiesSpecification, CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME, this);
    xmlPropertyNode = xmlNode->SelectSingleNode(PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonToolsInternal::LoadSpecificationsFromXmlNode<PropertyEditorsSpecification, PropertyEditorsSpecificationList>(xmlPropertyNode, m_propertyEditorsSpecification, PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::_WriteXml (BeXmlNodeP specificationNode) const
    {
    PresentationRuleSpecification::_WriteXml(specificationNode);

    specificationNode->AddAttributeInt32Value(COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);
    specificationNode->AddAttributeBooleanValue(CONTENT_SPECIFICATION_XML_ATTRIBUTE_SHOWIMAGES, m_showImages);

    CommonToolsInternal::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (specificationNode, m_relatedPropertiesSpecification);
    CommonToolsInternal::WriteRulesToXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList> (specificationNode, m_propertiesDisplaySpecification);
    CommonToolsInternal::WriteRulesToXmlNode<RelatedInstanceSpecification, RelatedInstanceSpecificationList> (specificationNode, m_relatedInstances);
    if (!m_calculatedPropertiesSpecification.empty())
        {
        BeXmlNodeP calculatedPropertiesNode = specificationNode->AddEmptyElement(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
        CommonToolsInternal::WriteRulesToXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(calculatedPropertiesNode, m_calculatedPropertiesSpecification);
        }
    if (!m_propertyEditorsSpecification.empty())
        {
        BeXmlNodeP propertyEditorsNode = specificationNode->AddEmptyElement(PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME);
        CommonToolsInternal::WriteRulesToXmlNode<PropertyEditorsSpecification, PropertyEditorsSpecificationList>(propertyEditorsNode, m_propertyEditorsSpecification);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecification::_ReadJson(JsonValueCR json)
    {
    if (!PresentationRuleSpecification::_ReadJson(json))
        return false;

    m_priority = json[COMMON_JSON_ATTRIBUTE_PRIORITY].asInt(1000);
    m_showImages = json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES].asBool(false);
    CommonToolsInternal::LoadFromJson(json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDPROPERTIESSPECIFICATION], 
        m_relatedPropertiesSpecification, CommonToolsInternal::LoadRuleFromJson<RelatedPropertiesSpecification>, this);
    CommonToolsInternal::LoadFromJson(json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESDISPLAYSPECIFICATION], 
        m_propertiesDisplaySpecification, CommonToolsInternal::LoadRuleFromJson<PropertiesDisplaySpecification>, this);
    CommonToolsInternal::LoadFromJson(json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_CALCULATEDPROPERTIESSPECIFICATION], 
        m_calculatedPropertiesSpecification, CommonToolsInternal::LoadRuleFromJson<CalculatedPropertiesSpecification>, this);
    CommonToolsInternal::LoadFromJson(json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYEDITORSSPECIFICATION], 
        m_propertyEditorsSpecification, CommonToolsInternal::LoadRuleFromJson<PropertyEditorsSpecification>, this);
    CommonToolsInternal::LoadFromJson(json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION], 
        m_relatedInstances, CommonToolsInternal::LoadRuleFromJson<RelatedInstanceSpecification>, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::_WriteJson(JsonValueR json) const
    {
    PresentationRuleSpecification::_WriteJson(json);
    if (1000 != m_priority)
        json[COMMON_JSON_ATTRIBUTE_PRIORITY] = m_priority;
    if (m_showImages)
        json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES] = m_showImages;
    if (!m_relatedPropertiesSpecification.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>
            (json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDPROPERTIESSPECIFICATION], m_relatedPropertiesSpecification);
        }
    if (!m_propertiesDisplaySpecification.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList>
            (json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESDISPLAYSPECIFICATION], m_propertiesDisplaySpecification);
        }
    if (!m_calculatedPropertiesSpecification.empty())
        {
        CommonToolsInternal::WriteRulesToJson<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>
            (json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_CALCULATEDPROPERTIESSPECIFICATION], m_calculatedPropertiesSpecification);
        }
    if (!m_propertyEditorsSpecification.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertyEditorsSpecification, PropertyEditorsSpecificationList>
            (json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYEDITORSSPECIFICATION], m_propertyEditorsSpecification);
        }
    if (!m_relatedInstances.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedInstanceSpecification, RelatedInstanceSpecificationList>
            (json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION], m_relatedInstances);
        }
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentSpecification::GetPriority (void) const { return m_priority; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::SetPriority (int value) { m_priority = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::AddRelatedProperty(RelatedPropertiesSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_relatedPropertiesSpecification.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList const& ContentSpecification::GetRelatedProperties() const {return m_relatedPropertiesSpecification;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::AddPropertiesDisplaySpecification(PropertiesDisplaySpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_propertiesDisplaySpecification.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::AddCalculatedProperty(CalculatedPropertiesSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_calculatedPropertiesSpecification.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::AddPropertyEditor(PropertyEditorsSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_propertyEditorsSpecification.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance)
    {
    InvalidateHash();
    relatedInstance.SetParent(this);
    m_relatedInstances.push_back(&relatedInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = PresentationRuleSpecification::_ComputeHash(parentHash);
    md5.Add(&m_priority, sizeof(m_priority));
    md5.Add(&m_showImages, sizeof(m_showImages));
    Utf8CP name = _GetXmlElementName();
    md5.Add(name, strlen(name));

    Utf8String currentHash = md5.GetHashString();
    for (RelatedPropertiesSpecificationP spec : m_relatedPropertiesSpecification)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (PropertiesDisplaySpecificationP spec : m_propertiesDisplaySpecification)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (CalculatedPropertiesSpecificationP spec : m_calculatedPropertiesSpecification)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (PropertyEditorsSpecificationP spec : m_propertyEditorsSpecification)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (RelatedInstanceSpecificationP spec : m_relatedInstances)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }
