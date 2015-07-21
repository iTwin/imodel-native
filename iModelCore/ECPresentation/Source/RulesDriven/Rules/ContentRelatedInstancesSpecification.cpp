/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentRelatedInstancesSpecification.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "CommonTools.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRelatedInstancesSpecification::ContentRelatedInstancesSpecification () 
    : ContentSpecification (), m_skipRelatedLevel (0), m_instanceFilter (L""), 
    m_requiredDirection (RequiredRelationDirection_Both), m_relationshipClassNames (L""), m_relatedClassNames (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRelatedInstancesSpecification::ContentRelatedInstancesSpecification 
(
int                        priority,
int                        skipRelatedLevel,
WString                    instanceFilter,
RequiredRelationDirection  requiredDirection,
WString                    relationshipClassNames,
WString                    relatedClassNames
) : ContentSpecification (priority), m_skipRelatedLevel (skipRelatedLevel), 
    m_instanceFilter (instanceFilter), m_requiredDirection (requiredDirection),
    m_relationshipClassNames (relationshipClassNames), m_relatedClassNames (relatedClassNames)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ContentRelatedInstancesSpecification::_GetXmlElementName ()
    {
    return CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRelatedInstancesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_skipRelatedLevel, COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL))
        m_skipRelatedLevel = 0;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relationshipClassNames, COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES))
        m_relationshipClassNames = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relatedClassNames, COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES))
        m_relatedClassNames = L"";

    WString requiredDirectionString = L"";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION))
        requiredDirectionString = L"";
    else
        m_requiredDirection = CommonTools::ParseRequiredDirectionString (requiredDirectionString.c_str ());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeInt32Value  (COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonTools::FormatRequiredDirectionString (m_requiredDirection));
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
WStringCR ContentRelatedInstancesSpecification::GetInstanceFilter (void) const { return m_instanceFilter; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::SetInstanceFilter (WStringCR value) { m_instanceFilter = value; }

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
WStringCR ContentRelatedInstancesSpecification::GetRelationshipClassNames (void) const { return m_relationshipClassNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::SetRelationshipClassNames (WStringCR value) { m_relationshipClassNames = value; } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ContentRelatedInstancesSpecification::GetRelatedClassNames (void) const { return m_relatedClassNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::SetRelatedClassNames (WStringCR value) { m_relatedClassNames = value; }
