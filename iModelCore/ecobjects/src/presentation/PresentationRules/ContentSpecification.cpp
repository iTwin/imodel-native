/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentSpecification.cpp $
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
ContentSpecification::~ContentSpecification ()
    {
    CommonTools::FreePresentationRules (m_relatedPropertiesSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_onlyIfNotHandled, COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED))
        m_onlyIfNotHandled = false;

    CommonTools::LoadRulesFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_relatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);

    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    specificationNode->AddAttributeInt32Value   (COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);
    specificationNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);

    CommonTools::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (specificationNode, m_relatedPropertiesSpecification);

    //Make sure we call protected override
    _WriteXml (specificationNode);
    }