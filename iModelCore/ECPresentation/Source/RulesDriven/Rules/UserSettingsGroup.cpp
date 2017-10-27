/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/UserSettingsGroup.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::UserSettingsGroup ()
    : PresentationKey (), m_categoryLabel ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::UserSettingsGroup (Utf8StringCR categoryLabel)
    : PresentationKey (1000), m_categoryLabel (categoryLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::UserSettingsGroup(UserSettingsGroupCR other)
    : PresentationKey(other)
    {
    CommonTools::CopyRules(m_nestedSettings, other.m_nestedSettings);
    CommonTools::CopyRules(m_settingsItems, other.m_settingsItems);
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
CharCP UserSettingsGroup::_GetXmlElementName () const
    {
    return USER_SETTINGS_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettingsGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_categoryLabel, USER_SETTINGS_XML_ATTRIBUTE_CATEGORY_LABEL))
        m_categoryLabel = "";

    CommonTools::LoadSpecificationsFromXmlNode<UserSettingsItem, UserSettingsItemList> (xmlNode, m_settingsItems, USER_SETTINGS_ITEM_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<UserSettingsGroup, UserSettingsGroupList> (xmlNode, m_nestedSettings, USER_SETTINGS_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (USER_SETTINGS_XML_ATTRIBUTE_CATEGORY_LABEL, m_categoryLabel.c_str ());

    CommonTools::WriteRulesToXmlNode<UserSettingsItem, UserSettingsItemList> (xmlNode, m_settingsItems);
    CommonTools::WriteRulesToXmlNode<UserSettingsGroup, UserSettingsGroupList> (xmlNode, m_nestedSettings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsGroup::GetCategoryLabel (void) const { return m_categoryLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::AddSettingsItem(UserSettingsItemR item) 
    {
    InvalidateHash();
    item.SetParent(this);
    m_settingsItems.push_back(&item);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::AddNestedSettings(UserSettingsGroupR group)
    {
    InvalidateHash();
    group.SetParent(this);
    m_nestedSettings.push_back(&group);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItemList const& UserSettingsGroup::GetSettingsItems (void) const { return m_settingsItems; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroupList const& UserSettingsGroup::GetNestedSettings (void) const { return m_nestedSettings; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 UserSettingsGroup::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = PresentationKey::_ComputeHash(parentHash);
    md5.Add(m_categoryLabel.c_str(), m_categoryLabel.size());
    Utf8String currentHash = md5.GetHashString();

    for (UserSettingsGroupP group : m_nestedSettings)
        {
        Utf8StringCR groupHash = group->GetHash(currentHash.c_str());
        md5.Add(groupHash.c_str(), groupHash.size());
        }
    for (UserSettingsItemP item : m_settingsItems)
        {
        Utf8StringCR itemHash = item->GetHash(currentHash.c_str());
        md5.Add(itemHash.c_str(), itemHash.size());
        }
    return md5;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItem::UserSettingsItem ()
    : m_id (""), m_label (""), m_options (""), m_defaultValue ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItem::UserSettingsItem (Utf8StringCR id, Utf8StringCR label, Utf8StringCR options, Utf8StringCR defaultValue)
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
        ECPRENSETATION_RULES_LOG.errorv ("Invalid UserSettingsItemXML: %hs element must contain a %hs attribute", USER_SETTINGS_ITEM_XML_NODE_NAME, USER_SETTINGS_ITEM_XML_ATTRIBUTE_ID);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, USER_SETTINGS_ITEM_XML_ATTRIBUTE_LABEL))
        {
        ECPRENSETATION_RULES_LOG.errorv ("Invalid UserSettingsItemXML: %hs element must contain a %hs attribute", USER_SETTINGS_ITEM_XML_NODE_NAME, USER_SETTINGS_ITEM_XML_ATTRIBUTE_LABEL);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_options, USER_SETTINGS_ITEM_XML_ATTRIBUTE_OPTIONS))
        m_options = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_defaultValue, USER_SETTINGS_ITEM_XML_ATTRIBUTE_DEFAULT_VALUE))
        m_defaultValue = "";    

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsItem::WriteXml (BeXmlNodeP parentXmlNode) const
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
Utf8StringCR UserSettingsItem::GetId (void) const               { return m_id; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsItem::GetLabel (void) const            { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsItem::GetOptions (void) const          { return m_options; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsItem::GetDefaultValue (void) const     { return m_defaultValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 UserSettingsItem::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_id.c_str(), m_id.size());
    md5.Add(m_label.c_str(), m_label.size());
    md5.Add(m_options.c_str(), m_options.size());
    md5.Add(m_defaultValue.c_str(), m_defaultValue.size());
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }
