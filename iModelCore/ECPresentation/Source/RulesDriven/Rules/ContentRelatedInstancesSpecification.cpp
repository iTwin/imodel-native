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
ContentRelatedInstancesSpecification::ContentRelatedInstancesSpecification () 
    : ContentSpecification(), m_skipRelatedLevel(0), m_isRecursive(false), m_requiredDirection(RequiredRelationDirection_Both)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRelatedInstancesSpecification::ContentRelatedInstancesSpecification 
(
int                        priority,
int                        skipRelatedLevel,
bool                       isRecursive,
Utf8String                 instanceFilter,
RequiredRelationDirection  requiredDirection,
Utf8String                 relationshipClassNames,
Utf8String                 relatedClassNames
) : ContentSpecification (priority), m_skipRelatedLevel (skipRelatedLevel), m_isRecursive(isRecursive),
    m_instanceFilter (instanceFilter), m_requiredDirection (requiredDirection),
    m_relationshipClassNames (relationshipClassNames), m_relatedClassNames (relatedClassNames)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRelatedInstancesSpecification::_GetXmlElementName () const
    {
    return CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRelatedInstancesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ContentSpecification::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_skipRelatedLevel, COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL))
        m_skipRelatedLevel = 0;
    
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_isRecursive, CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ISRECURSIVE))
        m_isRecursive = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relationshipClassNames, COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES))
        m_relationshipClassNames = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relatedClassNames, COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES))
        m_relatedClassNames = "";

    Utf8String requiredDirectionString = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION))
        requiredDirectionString = "";
    else
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString (requiredDirectionString.c_str ());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ContentSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeInt32Value  (COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    xmlNode->AddAttributeBooleanValue (CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ISRECURSIVE, m_isRecursive);
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonToolsInternal::FormatRequiredDirectionString (m_requiredDirection));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRelatedInstancesSpecification::_GetJsonElementType() const
    {
    return CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRelatedInstancesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ContentSpecification::_ReadJson(json))
        return false;
    m_skipRelatedLevel = json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL].asInt(0);
    m_isRecursive = json[CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE].asBool(false);
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");
    m_relationshipClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS]);
    m_relatedClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES]);
    m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::_WriteJson(JsonValueR json) const
    {
    ContentSpecification::_WriteJson(json);
    if (0 != m_skipRelatedLevel)
        json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL] = m_skipRelatedLevel;
    if (m_isRecursive)
        json[CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE] = m_isRecursive;
    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    if (!m_relationshipClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relatedClassNames);
    if (RequiredRelationDirection_Both != m_requiredDirection)
        json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_requiredDirection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentRelatedInstancesSpecification::GetSkipRelatedLevel (void) const { return m_skipRelatedLevel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::SetSkipRelatedLevel (int value) { m_skipRelatedLevel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentRelatedInstancesSpecification::GetInstanceFilter (void) const { return m_instanceFilter; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::SetInstanceFilter (Utf8StringCR value) { m_instanceFilter = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection ContentRelatedInstancesSpecification::GetRequiredRelationDirection (void) const { return m_requiredDirection; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::SetRequiredRelationDirection (RequiredRelationDirection value) { m_requiredDirection = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentRelatedInstancesSpecification::GetRelationshipClassNames (void) const { return m_relationshipClassNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::SetRelationshipClassNames (Utf8StringCR value) { m_relationshipClassNames = value; } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentRelatedInstancesSpecification::GetRelatedClassNames (void) const { return m_relatedClassNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::SetRelatedClassNames (Utf8StringCR value) { m_relatedClassNames = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentRelatedInstancesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ContentSpecification::_ComputeHash(parentHash);
    md5.Add(&m_skipRelatedLevel, sizeof(m_skipRelatedLevel));
    md5.Add(&m_isRecursive, sizeof(m_isRecursive));
    md5.Add(m_instanceFilter.c_str(), m_instanceFilter.size());
    md5.Add(&m_requiredDirection, sizeof(m_requiredDirection));
    md5.Add(m_relationshipClassNames.c_str(), m_relationshipClassNames.size());
    md5.Add(m_relatedClassNames.c_str(), m_relatedClassNames.size());
    return md5;
    }
