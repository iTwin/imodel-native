/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/UserSettingsGroup.cpp $
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
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::UserSettingsGroup ()
    : PresentationKey (), m_categoryLabel (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::UserSettingsGroup (WStringCR categoryLabel)
    : PresentationKey (1000), m_categoryLabel (categoryLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::~UserSettingsGroup (void)
    {
    CommonTools::FreePresentationRules (m_nestedSettings);
    CommonTools::FreePresentationRules (m_settingsItems);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP UserSettingsGroup::_GetXmlElementName ()
    {
    return USER_SETTINGS_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettingsGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_categoryLabel, USER_SETTINGS_XML_ATTRIBUTE_CATEGORY_LABEL))
        m_categoryLabel = L"";

    CommonTools::LoadRulesFromXmlNode<UserSettingsItem, UserSettingsItemList> (xmlNode, m_settingsItems, USER_SETTINGS_ITEM_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<UserSettingsGroup, UserSettingsGroupList> (xmlNode, m_nestedSettings, USER_SETTINGS_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (USER_SETTINGS_XML_ATTRIBUTE_CATEGORY_LABEL, m_categoryLabel.c_str ());

    CommonTools::WriteRulesToXmlNode<UserSettingsItem, UserSettingsItemList> (xmlNode, m_settingsItems);
    CommonTools::WriteRulesToXmlNode<UserSettingsGroup, UserSettingsGroupList> (xmlNode, m_nestedSettings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR UserSettingsGroup::GetCategoryLabel (void) const { return m_categoryLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItemList& UserSettingsGroup::GetSettingsItems (void) { return m_settingsItems; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroupList& UserSettingsGroup::GetNestedSettings (void) { return m_nestedSettings; }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItem::UserSettingsItem ()
    : m_id (L""), m_label (L""), m_options (L""), m_defaultValue (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItem::UserSettingsItem (WStringCR id, WStringCR label, WStringCR options, WStringCR defaultValue)
    : m_id (id), m_label (label), m_options (options), m_defaultValue (defaultValue)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettingsItem::ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_id, USER_SETTINGS_ITEM_XML_ATTRIBUTE_ID))
        {
        LOG.errorv (L"Invalid UserSettingsItemXML: %hs element must contain a %hs attribute", USER_SETTINGS_ITEM_XML_NODE_NAME, USER_SETTINGS_ITEM_XML_ATTRIBUTE_ID);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, USER_SETTINGS_ITEM_XML_ATTRIBUTE_LABEL))
        {
        LOG.errorv (L"Invalid UserSettingsItemXML: %hs element must contain a %hs attribute", USER_SETTINGS_ITEM_XML_NODE_NAME, USER_SETTINGS_ITEM_XML_ATTRIBUTE_LABEL);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_options, USER_SETTINGS_ITEM_XML_ATTRIBUTE_OPTIONS))
        m_options = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_defaultValue, USER_SETTINGS_ITEM_XML_ATTRIBUTE_DEFAULT_VALUE))
        m_defaultValue = L"";    

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsItem::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP xmlNode = parentXmlNode->AddEmptyElement (USER_SETTINGS_ITEM_XML_NODE_NAME);

    xmlNode->AddAttributeStringValue (USER_SETTINGS_ITEM_XML_ATTRIBUTE_ID,            m_id.c_str ());
    xmlNode->AddAttributeStringValue (USER_SETTINGS_ITEM_XML_ATTRIBUTE_LABEL,         m_label.c_str ());
    xmlNode->AddAttributeStringValue (USER_SETTINGS_ITEM_XML_ATTRIBUTE_OPTIONS,       m_options.c_str ());
    xmlNode->AddAttributeStringValue (USER_SETTINGS_ITEM_XML_ATTRIBUTE_DEFAULT_VALUE, m_defaultValue.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR UserSettingsItem::GetId (void) const               { return m_id; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR UserSettingsItem::GetLabel (void) const            { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR UserSettingsItem::GetOptions (void) const          { return m_options; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR UserSettingsItem::GetDefaultValue (void) const     { return m_defaultValue; }
