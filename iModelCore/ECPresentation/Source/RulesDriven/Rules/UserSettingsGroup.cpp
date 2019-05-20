/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::UserSettingsGroup() 
    : PresentationKey()
    {}

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
    CommonToolsInternal::CopyRules(m_nestedSettings, other.m_nestedSettings, this);
    CommonToolsInternal::CopyRules(m_settingsItems, other.m_settingsItems, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::~UserSettingsGroup (void)
    {
    CommonToolsInternal::FreePresentationRules (m_nestedSettings);
    CommonToolsInternal::FreePresentationRules (m_settingsItems);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP UserSettingsGroup::_GetXmlElementName () const
    {
    return USER_SETTINGS_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettingsGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_categoryLabel, USER_SETTINGS_XML_ATTRIBUTE_CATEGORY_LABEL))
        m_categoryLabel = "";

    CommonToolsInternal::LoadSpecificationsFromXmlNode<UserSettingsItem, UserSettingsItemList> (xmlNode, m_settingsItems, USER_SETTINGS_ITEM_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode<UserSettingsGroup, UserSettingsGroupList> (xmlNode, m_nestedSettings, USER_SETTINGS_XML_NODE_NAME, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::_WriteXml (BeXmlNodeP xmlNode) const
    {
    PresentationKey::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue (USER_SETTINGS_XML_ATTRIBUTE_CATEGORY_LABEL, m_categoryLabel.c_str ());
    CommonToolsInternal::WriteRulesToXmlNode<UserSettingsItem, UserSettingsItemList> (xmlNode, m_settingsItems);
    CommonToolsInternal::WriteRulesToXmlNode<UserSettingsGroup, UserSettingsGroupList> (xmlNode, m_nestedSettings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP UserSettingsGroup::_GetJsonElementType() const
    {
    return USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettingsGroup::_ReadJson(JsonValueCR json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    m_categoryLabel = json[USER_SETTINGS_JSON_ATTRIBUTE_CATEGORY_LABEL].asCString("");
    CommonToolsInternal::LoadFromJson(json[USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS], m_settingsItems, CommonToolsInternal::LoadRuleFromJson<UserSettingsItem>, this);
    CommonToolsInternal::LoadFromJsonByPriority(json[USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS], m_nestedSettings, CommonToolsInternal::LoadRuleFromJson<UserSettingsGroup>, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::_WriteJson(JsonValueR json) const
    {
    PresentationKey::_WriteJson(json);
    json[USER_SETTINGS_JSON_ATTRIBUTE_CATEGORY_LABEL] = m_categoryLabel;
    if (!m_settingsItems.empty())
        CommonToolsInternal::WriteRulesToJson<UserSettingsItem, UserSettingsItemList>(json[USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS], m_settingsItems);
    if (!m_nestedSettings.empty())
        CommonToolsInternal::WriteRulesToJson<UserSettingsGroup, UserSettingsGroupList>(json[USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS], m_nestedSettings);
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
UserSettingsItem::UserSettingsItem() {}

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
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, USER_SETTINGS_ITEM_XML_NODE_NAME, USER_SETTINGS_ITEM_XML_ATTRIBUTE_ID);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, USER_SETTINGS_ITEM_XML_ATTRIBUTE_LABEL))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, USER_SETTINGS_ITEM_XML_NODE_NAME, USER_SETTINGS_ITEM_XML_ATTRIBUTE_LABEL);
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
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettingsItem::ReadJson (JsonValueCR json)
    {
    //Required
    JsonValueCR idJson = json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID];
    if (idJson.isNull() || !idJson.isConvertibleTo(Json::ValueType::stringValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "UserSettingsItem", USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID);
        return false;
        }
    m_id = idJson.asCString("");

    JsonValueCR labelJson = json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL];
    if (labelJson.isNull() || !labelJson.isConvertibleTo(Json::ValueType::stringValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "UserSettingsItem", USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL);
        return false;
        }
    m_label = labelJson.asCString("");

    //Optional
    m_options = json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_OPTIONS].asCString("");
    m_defaultValue = json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_DEFAULT_VALUE].asCString("");

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UserSettingsItem::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID] = m_id;
    json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL] = m_label;
    json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_OPTIONS] = m_options;
    json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_DEFAULT_VALUE] = m_defaultValue;
    return json;
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
