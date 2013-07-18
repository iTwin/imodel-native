/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/LocalizationResourceKeyDefinition.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinition::LocalizationResourceKeyDefinition ()
    : PresentationKey (), m_id (L""), m_key (L""), m_defaultValue (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinition::LocalizationResourceKeyDefinition (int priority, WStringCR id, WStringCR key, WStringCR defaultValue)
    : PresentationKey (priority), m_id (id), m_key (key), m_defaultValue (defaultValue)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP LocalizationResourceKeyDefinition::_GetXmlElementName ()
    {
    return LOCALIZATION_DEFINITION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalizationResourceKeyDefinition::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_id, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_ID))
        m_id = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_key, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_KEY))
        m_key = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_defaultValue, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_DEFAULTVALUE))
        m_defaultValue = L"";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalizationResourceKeyDefinition::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (LOCALIZATION_DEFINITION_XML_ATTRIBUTE_ID, m_id.c_str ());
    xmlNode->AddAttributeStringValue (LOCALIZATION_DEFINITION_XML_ATTRIBUTE_KEY, m_key.c_str ());
    xmlNode->AddAttributeStringValue (LOCALIZATION_DEFINITION_XML_ATTRIBUTE_DEFAULTVALUE, m_defaultValue.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR LocalizationResourceKeyDefinition::GetId (void) const             { return m_id; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR LocalizationResourceKeyDefinition::GetKey (void) const            { return m_key; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR LocalizationResourceKeyDefinition::GetDefaultValue (void) const   { return m_defaultValue; }