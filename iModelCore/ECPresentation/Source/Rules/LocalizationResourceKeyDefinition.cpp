/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinition::LocalizationResourceKeyDefinition()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinition::LocalizationResourceKeyDefinition(int priority, Utf8StringCR id, Utf8StringCR key, Utf8StringCR defaultValue)
    : PrioritizedPresentationKey(priority), m_id(id), m_key(key), m_defaultValue(defaultValue)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP LocalizationResourceKeyDefinition::_GetXmlElementName () const
    {
    return LOCALIZATION_DEFINITION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalizationResourceKeyDefinition::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PrioritizedPresentationKey::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_id, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_ID))
        m_id = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_key, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_KEY))
        m_key = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_defaultValue, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_DEFAULTVALUE))
        m_defaultValue = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalizationResourceKeyDefinition::_WriteXml (BeXmlNodeP xmlNode) const
    {
    PrioritizedPresentationKey::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(LOCALIZATION_DEFINITION_XML_ATTRIBUTE_ID, m_id.c_str());
    xmlNode->AddAttributeStringValue(LOCALIZATION_DEFINITION_XML_ATTRIBUTE_KEY, m_key.c_str());
    xmlNode->AddAttributeStringValue(LOCALIZATION_DEFINITION_XML_ATTRIBUTE_DEFAULTVALUE, m_defaultValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LocalizationResourceKeyDefinition::GetId (void) const             { return m_id; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LocalizationResourceKeyDefinition::GetKey (void) const            { return m_key; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LocalizationResourceKeyDefinition::GetDefaultValue (void) const   { return m_defaultValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 LocalizationResourceKeyDefinition::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_id.empty())
        ADD_STR_VALUE_TO_HASH(md5, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_ID, m_id);
    if (!m_key.empty())
        ADD_STR_VALUE_TO_HASH(md5, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_KEY, m_key);
    if (!m_defaultValue.empty())
        ADD_STR_VALUE_TO_HASH(md5, LOCALIZATION_DEFINITION_XML_ATTRIBUTE_DEFAULTVALUE, m_defaultValue);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalizationResourceKeyDefinition::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!PrioritizedPresentationKey::_ShallowEqual(other))
        return false;

    LocalizationResourceKeyDefinition const* otherRule = dynamic_cast<LocalizationResourceKeyDefinition const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_id == otherRule->m_id
        && m_key == otherRule->m_key
        && m_defaultValue == otherRule->m_defaultValue;
    }
