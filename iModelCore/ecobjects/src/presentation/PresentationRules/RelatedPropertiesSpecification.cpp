/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/RelatedPropertiesSpecification.cpp $
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
RelatedPropertiesSpecification::~RelatedPropertiesSpecification ()
    {
    CommonTools::FreePresentationRules (m_nestedRelatedPropertiesSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPropertiesSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relationshipClassNames, COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES))
        m_relationshipClassNames = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relatedClassNames, COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES))
        m_relatedClassNames = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyNames, COMMON_XML_ATTRIBUTE_PROPERTYNAMES))
        m_propertyNames = L"";

    WString requiredDirectionString = L"";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION))
        requiredDirectionString = L"";
    else
        m_requiredDirection = CommonTools::ParseRequiredDirectionString (requiredDirectionString.c_str ());

    CommonTools::LoadRulesFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_nestedRelatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP relatedPropertiesNode = parentXmlNode->AddEmptyElement (RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);

    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_PROPERTYNAMES, m_propertyNames.c_str ());
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonTools::FormatRequiredDirectionString (m_requiredDirection));

    CommonTools::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (relatedPropertiesNode, m_nestedRelatedPropertiesSpecification);
    }