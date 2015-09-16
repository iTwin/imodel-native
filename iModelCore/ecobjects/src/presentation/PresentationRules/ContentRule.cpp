/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentRule.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "CommonTools.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule () : PresentationRule (), m_customControl ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled)
    : PresentationRule (condition, priority, onlyIfNotHandled), m_customControl ("")
    {
    }

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
CharCP ContentRule::_GetXmlElementName ()
    {
    return CONTENT_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_customControl, CONTENT_RULE_XML_ATTRIBUTE_CUSTOMCONTROL))
        m_customControl = "";

    for (BeXmlNodeP child = xmlNode->GetFirstChild (BEXMLNODE_Element); NULL != child; child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<ContentInstancesOfSpecificClassesSpecification, ContentSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<ContentRelatedInstancesSpecification, ContentSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<SelectedNodeInstancesSpecification, ContentSpecificationList> (child, m_specifications);
        }

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (CONTENT_RULE_XML_ATTRIBUTE_CUSTOMCONTROL, m_customControl.c_str ());

    CommonTools::WriteRulesToXmlNode<ContentSpecification, ContentSpecificationList> (xmlNode, m_specifications);

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationList& ContentRule::GetSpecifications (void) { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentRule::GetCustomControl (void)                  { return m_customControl;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::SetCustomControl (Utf8StringCR customControl)    { m_customControl = customControl; }