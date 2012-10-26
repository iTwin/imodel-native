/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/LabelOverride.cpp $
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
CharCP LabelOverride::_GetXmlElementName ()
    {
    return LABEL_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelOverride::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, LABEL_OVERRIDE_XML_ATTRIBUTE_LABEL))
        m_label = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_description, LABEL_OVERRIDE_XML_ATTRIBUTE_DESCRIPTION))
        m_description = L"";

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (LABEL_OVERRIDE_XML_ATTRIBUTE_LABEL, m_label.c_str ());
    xmlNode->AddAttributeStringValue (LABEL_OVERRIDE_XML_ATTRIBUTE_DESCRIPTION, m_description.c_str ());

    PresentationRule::_WriteXml (xmlNode);
    }