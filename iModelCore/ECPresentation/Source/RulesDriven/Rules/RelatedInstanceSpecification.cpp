/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/RelatedInstanceSpecification.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_relationshipName, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME))
        return false;
    
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_alias, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS))
        return false;
    
    Utf8String requiredDirectionString;
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION))
        requiredDirectionString = "";
    m_direction = CommonTools::ParseRequiredDirectionString(requiredDirectionString.c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP node = parentXmlNode->AddEmptyElement(RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME);
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME, m_relationshipName.c_str());
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION, CommonTools::FormatRequiredDirectionString(m_direction));
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS, m_alias.c_str());
    }