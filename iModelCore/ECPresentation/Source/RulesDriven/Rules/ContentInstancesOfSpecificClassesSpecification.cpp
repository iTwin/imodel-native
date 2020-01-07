/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification () 
    : ContentSpecification(), m_handleInstancesPolymorphically(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification (int priority, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic) 
    : ContentSpecification (priority), m_instanceFilter (instanceFilter), m_classNames (classNames), m_handleInstancesPolymorphically(arePolymorphic)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentInstancesOfSpecificClassesSpecification::_GetXmlElementName () const
    {
    return CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentInstancesOfSpecificClassesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ContentSpecification::_ReadXml(xmlNode))
        return false;

    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_classNames, COMMON_XML_ATTRIBUTE_CLASSNAMES))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_CLASSNAMES);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_handleInstancesPolymorphically, COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC))
        m_handleInstancesPolymorphically = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ContentSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_CLASSNAMES, m_classNames.c_str ());
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC, m_handleInstancesPolymorphically);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentInstancesOfSpecificClassesSpecification::_GetJsonElementType() const
    {
    return CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentInstancesOfSpecificClassesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ContentSpecification::_ReadJson(json))
        return false;

    //Required
    m_classNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_CLASSES]);
    if (m_classNames.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "ContentInstancesOfSpecificClassesSpecification", COMMON_JSON_ATTRIBUTE_CLASSES);
        return false;
        }

    //Optional
    if (json.isMember(CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEINSTANCESPOLYMORPHICALLY))
        m_handleInstancesPolymorphically = json[CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEINSTANCESPOLYMORPHICALLY].asBool(false);
    else
        m_handleInstancesPolymorphically = json[COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC].asBool(false); // deprecated
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::_WriteJson(JsonValueR json) const
    {
    ContentSpecification::_WriteJson(json);
    json[COMMON_JSON_ATTRIBUTE_CLASSES] = CommonToolsInternal::SchemaAndClassNamesToJson(m_classNames);
    if (m_handleInstancesPolymorphically)
        json[CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEINSTANCESPOLYMORPHICALLY] = m_handleInstancesPolymorphically;
    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentInstancesOfSpecificClassesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ContentSpecification::_ComputeHash(parentHash);
    md5.Add(m_instanceFilter.c_str(), m_instanceFilter.size());
    md5.Add(m_classNames.c_str(), m_classNames.size());
    md5.Add(&m_handleInstancesPolymorphically, sizeof(m_handleInstancesPolymorphically));
    return md5;
    }
