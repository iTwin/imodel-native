/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/LabelOverride.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverride::LabelOverride ()
    : PresentationRule (), m_label (L""), m_description (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverride::LabelOverride (WStringCR condition, int priority, WStringCR label, WStringCR description)
    : PresentationRule (condition, priority, false), m_label (label), m_description (description)
    {
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR LabelOverride::GetLabel (void) const { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::SetLabel (WString value) { m_label = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR LabelOverride::GetDescription (void) const { return m_description; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::SetDescription (WString value) { m_description = value; }
