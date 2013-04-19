/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/SelectedNodeInstancesSpecification.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP SelectedNodeInstancesSpecification::_GetXmlElementName ()
    {
    return SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_onlyIfNotHandled, COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED))
        m_onlyIfNotHandled = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_acceptableSchemaName, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME))
        m_acceptableSchemaName = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_acceptableClassNames, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES))
        m_acceptableClassNames = L"";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_acceptablePolymorphically, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY))
        m_acceptablePolymorphically = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);
    xmlNode->AddAttributeStringValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME, m_acceptableSchemaName.c_str ());
    xmlNode->AddAttributeStringValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES, m_acceptableClassNames.c_str ());
    xmlNode->AddAttributeBooleanValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY, m_acceptablePolymorphically);
    }