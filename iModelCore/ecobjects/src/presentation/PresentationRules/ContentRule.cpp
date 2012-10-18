/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentRule.cpp $
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
ContentRule::~ContentRule ()
    {
    CommonTools::FreePresentationRules (m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    CommonTools::LoadRulesFromXmlNode<ContentInstancesOfSpecificClassesSpecification, ContentSpecificationList> (xmlNode, m_specifications, CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<ContentRelatedInstancesSpecification, ContentSpecificationList> (xmlNode, m_specifications, CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<SelectedNodeInstancesSpecification, ContentSpecificationList> (xmlNode, m_specifications, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME);

    return PresentationRule::_ReadXml (xmlNode);
    }