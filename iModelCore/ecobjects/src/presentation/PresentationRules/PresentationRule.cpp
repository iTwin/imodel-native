/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/PresentationRule.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationKey::PresentationKey () : m_priority (1000)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationKey::PresentationKey (int priority) : m_priority (priority)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationKey::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    //Make sure we call protected override
    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationKey::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP ruleNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    ruleNode->AddAttributeInt32Value (COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);

    //Make sure we call protected override
    _WriteXml (ruleNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int PresentationKey::GetPriority (void) const { return m_priority; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule ()
    : PresentationKey (), m_condition (""), m_onlyIfNotHandled (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled)
    : PresentationKey (priority), m_condition (condition), m_onlyIfNotHandled (onlyIfNotHandled)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_condition, PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION))
        m_condition = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_onlyIfNotHandled, COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED))
        m_onlyIfNotHandled = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str ());
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PresentationRule::GetCondition (void) const       { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::SetCondition (Utf8String value)         { m_condition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRule::GetOnlyIfNotHandled (void) const     { return m_onlyIfNotHandled; }
