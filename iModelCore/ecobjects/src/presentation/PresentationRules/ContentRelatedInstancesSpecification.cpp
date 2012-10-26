/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentRelatedInstancesSpecification.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relationshipSchemaName, COMMON_XML_ATTRIBUTE_RELATIONSHIPSCHEMANAME))
        m_relationshipSchemaName = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relationshipClassNames, COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES))
        m_relationshipClassNames = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relatedSchemaName, COMMON_XML_ATTRIBUTE_RELATEDSCHEMANAME))
        m_relatedSchemaName = L"";

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
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPSCHEMANAME, m_relationshipSchemaName.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATEDSCHEMANAME, m_relatedSchemaName.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonTools::FormatRequiredDirectionString (m_requiredDirection));
    }