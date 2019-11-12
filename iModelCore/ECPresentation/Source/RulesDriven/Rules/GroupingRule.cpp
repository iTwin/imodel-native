/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::GroupingRule() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::GroupingRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR contextMenuCondition, Utf8StringCR contextMenuLabel, Utf8StringCR settingsId)
    : ConditionalCustomizationRule(condition, priority, onlyIfNotHandled),
      m_schemaName (schemaName), m_className (className), m_contextMenuCondition (contextMenuCondition), m_contextMenuLabel (contextMenuLabel), m_settingsId (settingsId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::GroupingRule(GroupingRuleCR other)
    : ConditionalCustomizationRule(other), m_schemaName(other.m_schemaName), m_className(other.m_className), m_contextMenuCondition(other.m_contextMenuCondition),
    m_contextMenuLabel(other.m_contextMenuLabel), m_settingsId(other.m_settingsId)
    {
    CommonToolsInternal::CloneRules(m_groups, other.m_groups, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::~GroupingRule (void)
    {
    CommonToolsInternal::FreePresentationRules(m_groups);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GroupingRule::_GetXmlElementName () const
    {
    return GROUPING_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupingRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ConditionalCustomizationRule::_ReadXml(xmlNode))
        return false;

    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_schemaName, GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, GROUPING_RULE_XML_NODE_NAME, GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_className, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, GROUPING_RULE_XML_NODE_NAME, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuCondition, GROUPING_RULE_XML_ATTRIBUTE_MENUCONDITION))
        m_contextMenuCondition = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuLabel, GROUPING_RULE_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_settingsId, GROUPING_RULE_XML_ATTRIBUTE_SETTINGSID))
        m_settingsId = "";

    //Load Class and Property Groups
    for (BeXmlNodeP child = xmlNode->GetFirstChild (BEXMLNODE_Element); NULL != child; child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), CLASS_GROUP_XML_NODE_NAME))
            CommonToolsInternal::LoadSpecificationFromXmlNode<ClassGroup, GroupList> (child, m_groups, this);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), PROPERTY_GROUP_XML_NODE_NAME))
            CommonToolsInternal::LoadSpecificationFromXmlNode<PropertyGroup, GroupList> (child, m_groups, this);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), SAME_LABEL_INSTANCE_GROUP_XML_NODE_NAME))
            CommonToolsInternal::LoadSpecificationFromXmlNode<SameLabelInstanceGroup, GroupList> (child, m_groups, this);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ConditionalCustomizationRule::_WriteXml (xmlNode);
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME, m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME, m_className.c_str ());
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_MENUCONDITION, m_contextMenuCondition.c_str ());
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel.c_str ());
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_SETTINGSID, m_settingsId.c_str ());
    CommonToolsInternal::WriteRulesToXmlNode<GroupSpecification, GroupList> (xmlNode, m_groups);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GroupingRule::_GetJsonElementType() const
    {
    return GROUPING_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupingRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    //Required
    CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_className, json[COMMON_JSON_ATTRIBUTE_CLASS]);
    if (m_schemaName.empty() || m_className.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "GroupingRule", COMMON_JSON_ATTRIBUTE_CLASS);
        return false;
        }

    //Optional
    CommonToolsInternal::LoadFromJson(json[GROUPING_RULE_JSON_ATTRIBUTE_GROUPS], m_groups, GroupSpecification::Create, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::_WriteJson(JsonValueR json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    if (!m_schemaName.empty() && !m_className.empty())
        json[COMMON_JSON_ATTRIBUTE_CLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_schemaName, m_className);
    if (!m_groups.empty())
        CommonToolsInternal::WriteRulesToJson<GroupSpecification, GroupList>(json[GROUPING_RULE_JSON_ATTRIBUTE_GROUPS], m_groups);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetSchemaName (void) const              { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetClassName (void) const               { return m_className; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetContextMenuCondition (void) const    { return m_contextMenuCondition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::SetContextMenuCondition (Utf8String value)      { m_contextMenuCondition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetContextMenuLabel (void) const        { return m_contextMenuLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetSettingsId (void) const              { return m_settingsId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupList const& GroupingRule::GetGroups (void) const            { return m_groups; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::AddGroup(GroupSpecificationR group)
    {
    InvalidateHash();
    group.SetParent(this);
    m_groups.push_back(&group);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 GroupingRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ConditionalCustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_schemaName.c_str(), m_schemaName.size());
    md5.Add(m_className.c_str(), m_className.size());
    md5.Add(m_contextMenuCondition.c_str(), m_contextMenuCondition.size());
    md5.Add(m_contextMenuLabel.c_str(), m_contextMenuLabel.size());
    md5.Add(m_settingsId.c_str(), m_settingsId.size());

    Utf8String currentHash = md5.GetHashString();
    for (GroupSpecificationP spec : m_groups)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecification* GroupSpecification::Create(JsonValueCR json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString("");
    GroupSpecification* spec = nullptr;
    if (0 == strcmp(CLASS_GROUP_JSON_TYPE, type))
        spec = new ClassGroup();
    else if (0 == strcmp(SAME_LABEL_INSTANCE_GROUP_JSON_TYPE, type))
        spec = new SameLabelInstanceGroup();
    else if (0 == strcmp(PROPERTY_GROUP_JSON_TYPE, type))
        spec = new PropertyGroup();
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecification::GroupSpecification () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecification::GroupSpecification (Utf8StringCR contextMenuLabel, Utf8CP defaultLabel) 
    : m_contextMenuLabel (contextMenuLabel), m_defaultLabel (defaultLabel)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupSpecification::Accept(GroupingRuleSpecificationVisitor& visitor) const {_Accept(visitor);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuLabel, GROUP_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_defaultLabel, GROUP_XML_ATTRIBUTE_DEFAULTLABEL))
        m_defaultLabel.clear();

    //Make sure we call protected override
    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupSpecification::WriteXml (BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());
    specificationNode->AddAttributeStringValue  (GROUP_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel.c_str ());
    specificationNode->AddAttributeStringValue (GROUP_XML_ATTRIBUTE_DEFAULTLABEL, m_defaultLabel.c_str());

    //Make sure we call protected override
    _WriteXml (specificationNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupSpecification::ReadJson (JsonValueCR json)
    {
    if (!json.isMember(COMMON_JSON_ATTRIBUTE_SPECTYPE) || !json[COMMON_JSON_ATTRIBUTE_SPECTYPE].isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "GroupSpecification", COMMON_JSON_ATTRIBUTE_SPECTYPE);
        return false;
        }
    if (0 != strcmp(json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString(), _GetJsonElementType()))
        return false;

    //Optional:
    m_defaultLabel = json[GROUP_JSON_ATTRIBUTE_DEFAULTLABEL].asCString("");

    //Make sure we call protected override
    return _ReadJson (json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value GroupSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[COMMON_JSON_ATTRIBUTE_SPECTYPE] = _GetJsonElementType();
    if (!m_defaultLabel.empty())
        json[GROUP_JSON_ATTRIBUTE_DEFAULTLABEL] = m_defaultLabel;
    _WriteJson(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupSpecification::GetContextMenuLabel (void) const  { return m_contextMenuLabel; }
Utf8StringCR GroupSpecification::GetDefaultLabel (void) const      { return m_defaultLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 GroupSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_contextMenuLabel.c_str(), m_contextMenuLabel.size());
    md5.Add(m_defaultLabel.c_str(), m_defaultLabel.size());
    Utf8CP name = _GetXmlElementName();
    md5.Add(name, strlen(name));
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SameLabelInstanceGroup::SameLabelInstanceGroup () : GroupSpecification ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SameLabelInstanceGroup::SameLabelInstanceGroup (Utf8StringCR contextMenuLabel) : GroupSpecification (contextMenuLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SameLabelInstanceGroup::_Accept(GroupingRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SameLabelInstanceGroup::_GetXmlElementName () const
    {
    return SAME_LABEL_INSTANCE_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SameLabelInstanceGroup::_GetJsonElementType() const
    {
    return SAME_LABEL_INSTANCE_GROUP_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SameLabelInstanceGroup::_ComputeHash(Utf8CP parentHash) const
    {
    return GroupSpecification::_ComputeHash(parentHash);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ClassGroup::ClassGroup ()
    : GroupSpecification (), m_createGroupForSingleItem (false), m_schemaName (""), m_baseClassName ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ClassGroup::ClassGroup (Utf8StringCR contextMenuLabel, bool createGroupForSingleItem, Utf8StringCR schemaName, Utf8StringCR baseClassName)
    : GroupSpecification (contextMenuLabel), m_createGroupForSingleItem (createGroupForSingleItem), m_schemaName (schemaName), m_baseClassName (baseClassName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassGroup::_Accept(GroupingRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ClassGroup::_GetXmlElementName () const
    {
    return CLASS_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!GroupSpecification::_ReadXml(xmlNode))
        return false;

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_createGroupForSingleItem, GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM))
        m_createGroupForSingleItem = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_schemaName, CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME))
        m_schemaName = "";
    
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_baseClassName, CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME))
        m_baseClassName = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassGroup::_WriteXml (BeXmlNodeP xmlNode) const
    {
    GroupSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue (GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM, m_createGroupForSingleItem);
    xmlNode->AddAttributeStringValue  (CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME,         m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue  (CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME,      m_baseClassName.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ClassGroup::_GetJsonElementType() const
    {
    return CLASS_GROUP_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::_ReadJson (JsonValueCR json) 
    {
    if (!GroupSpecification::_ReadJson(json))
        return false;

    m_createGroupForSingleItem = json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM].asBool(false);
    CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_baseClassName, json[CLASS_GROUP_JSON_ATTRIBUTE_BASECLASS]);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassGroup::_WriteJson(JsonValueR json) const
    {
    GroupSpecification::_WriteJson(json);
    if (m_createGroupForSingleItem)
        json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM] = m_createGroupForSingleItem;
    if (!m_schemaName.empty() && !m_baseClassName.empty())
        json[CLASS_GROUP_JSON_ATTRIBUTE_BASECLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_schemaName, m_baseClassName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::GetCreateGroupForSingleItem (void) const       { return m_createGroupForSingleItem; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClassGroup::GetSchemaName (void) const                { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClassGroup::GetBaseClassName (void) const             { return m_baseClassName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ClassGroup::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = GroupSpecification::_ComputeHash(parentHash);
    md5.Add(&m_createGroupForSingleItem, sizeof(m_createGroupForSingleItem));
    md5.Add(m_schemaName.c_str(), m_schemaName.size());
    md5.Add(m_baseClassName.c_str(), m_baseClassName.size());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::PropertyGroup ()
    : GroupSpecification(), m_createGroupForSingleItem(false), m_createGroupForUnspecifiedValues(true), 
    m_groupingValue(PropertyGroupingValue::DisplayLabel), m_sortingValue(PropertyGroupingValue::DisplayLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::PropertyGroup (Utf8StringCR contextMenuLabel, Utf8StringCR imageId, bool createGroupForSingleItem, Utf8StringCR propertyName, Utf8CP defaultLabel)
    : GroupSpecification (contextMenuLabel, defaultLabel), m_imageId (imageId), 
    m_createGroupForSingleItem (createGroupForSingleItem), m_createGroupForUnspecifiedValues(true), 
    m_propertyName (propertyName), m_groupingValue(PropertyGroupingValue::DisplayLabel), m_sortingValue(PropertyGroupingValue::DisplayLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::PropertyGroup(PropertyGroupCR other)
    : GroupSpecification(other), m_imageId(other.m_imageId), m_createGroupForSingleItem(other.m_createGroupForSingleItem),
    m_createGroupForUnspecifiedValues(other.m_createGroupForUnspecifiedValues), m_propertyName(other.m_propertyName), 
    m_groupingValue(other.m_groupingValue), m_sortingValue(other.m_sortingValue)
    {
    CommonToolsInternal::CopyRules(m_ranges, other.m_ranges, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::~PropertyGroup (void)
    {
    CommonToolsInternal::FreePresentationRules(m_ranges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::_Accept(GroupingRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyGroup::_GetXmlElementName () const
    {
    return PROPERTY_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static PropertyGroupingValue GetPropertyGroupingValueFromString(Utf8StringCR str)
    {
    if (str.Equals(PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL))
        return PropertyGroupingValue::DisplayLabel;
    else if (str.Equals(PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_PROPERTYVALUE))
        return PropertyGroupingValue::PropertyValue;

    BeAssert(false);
    return PropertyGroupingValue::DisplayLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetPropertyGroupingValueAsString(PropertyGroupingValue value)
    {
    switch (value)
        {
        case PropertyGroupingValue::PropertyValue:  return PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_PROPERTYVALUE;
        case PropertyGroupingValue::DisplayLabel:   return PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL;
        }
    BeAssert(false);
    return PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!GroupSpecification::_ReadXml(xmlNode))
        return false;

    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyName, COMMON_XML_ATTRIBUTE_PROPERTYNAME))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, PROPERTY_GROUP_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_PROPERTYNAME);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, GROUP_XML_ATTRIBUTE_IMAGEID))
        m_imageId = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_createGroupForSingleItem, GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM))
        m_createGroupForSingleItem = false;
    
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_createGroupForUnspecifiedValues, GROUP_XML_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES))
        m_createGroupForUnspecifiedValues = true;
    
    Utf8String groupingValueStr;
    if (BEXML_Success == xmlNode->GetAttributeStringValue(groupingValueStr, PROPERTY_GROUP_XML_ATTRIBUTE_GROUPINGVALUE))
        m_groupingValue = GetPropertyGroupingValueFromString(groupingValueStr);
    
    Utf8String sortingValueStr;
    if (BEXML_Success == xmlNode->GetAttributeStringValue(sortingValueStr, PROPERTY_GROUP_XML_ATTRIBUTE_SORTINGVALUE))
        m_sortingValue = GetPropertyGroupingValueFromString(sortingValueStr);

    //Load Ranges
    CommonToolsInternal::LoadSpecificationsFromXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (xmlNode, m_ranges, PROPERTY_RANGE_GROUP_XML_NODE_NAME, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::_WriteXml (BeXmlNodeP xmlNode) const
    {
    GroupSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue  (GROUP_XML_ATTRIBUTE_IMAGEID,                         m_imageId.c_str ());
    xmlNode->AddAttributeBooleanValue (GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM,        m_createGroupForSingleItem);
    xmlNode->AddAttributeBooleanValue (GROUP_XML_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES, m_createGroupForUnspecifiedValues);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_PROPERTYNAME,                   m_propertyName.c_str ());
    xmlNode->AddAttributeStringValue(PROPERTY_GROUP_XML_ATTRIBUTE_GROUPINGVALUE,            GetPropertyGroupingValueAsString(m_groupingValue));
    xmlNode->AddAttributeStringValue(PROPERTY_GROUP_XML_ATTRIBUTE_SORTINGVALUE,             GetPropertyGroupingValueAsString(m_sortingValue));
    CommonToolsInternal::WriteRulesToXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (xmlNode, m_ranges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyGroup::_GetJsonElementType() const
    {
    return PROPERTY_GROUP_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::_ReadJson (JsonValueCR json) 
    {
    if (!GroupSpecification::_ReadJson(json))
        return false;

    //Required
    m_propertyName = json[COMMON_JSON_ATTRIBUTE_PROPERTYNAME].asCString("");
    if (m_propertyName.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyGroup", COMMON_JSON_ATTRIBUTE_PROPERTYNAME);
        return false;
        }

    //Optional
    m_imageId = json[GROUP_JSON_ATTRIBUTE_IMAGEID].asCString("");
    m_createGroupForSingleItem = json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM].asBool(false);
    m_createGroupForUnspecifiedValues = json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES].asBool(true);
    if (json.isMember(PROPERTY_GROUP_JSON_ATTRIBUTE_GROUPINGVALUE))
        m_groupingValue = GetPropertyGroupingValueFromString(json[PROPERTY_GROUP_JSON_ATTRIBUTE_GROUPINGVALUE].asCString());
    if (json.isMember(PROPERTY_GROUP_JSON_ATTRIBUTE_SORTINGVALUE))
        m_sortingValue = GetPropertyGroupingValueFromString(json[PROPERTY_GROUP_JSON_ATTRIBUTE_SORTINGVALUE].asCString());
    CommonToolsInternal::LoadFromJson(json[PROPERTY_GROUP_JSON_ATTRIBUTE_RANGES], m_ranges, CommonToolsInternal::LoadRuleFromJson<PropertyRangeGroupSpecification>, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::_WriteJson(JsonValueR json) const
    {
    GroupSpecification::_WriteJson(json);
    json[COMMON_JSON_ATTRIBUTE_PROPERTYNAME] = m_propertyName;
    if (!m_imageId.empty())
        json[GROUP_JSON_ATTRIBUTE_IMAGEID] = m_imageId;
    if (m_createGroupForSingleItem)
        json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM] = m_createGroupForSingleItem;
    if (!m_createGroupForUnspecifiedValues)
        json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES] = m_createGroupForUnspecifiedValues;
    if (PropertyGroupingValue::DisplayLabel != m_groupingValue)
        json[PROPERTY_GROUP_JSON_ATTRIBUTE_GROUPINGVALUE] = GetPropertyGroupingValueAsString(m_groupingValue);
    if (PropertyGroupingValue::DisplayLabel != m_sortingValue)
        json[PROPERTY_GROUP_JSON_ATTRIBUTE_SORTINGVALUE] = GetPropertyGroupingValueAsString(m_sortingValue);
    if (!m_ranges.empty())
        CommonToolsInternal::WriteRulesToJson<PropertyRangeGroupSpecification, PropertyRangeGroupList>(json[PROPERTY_GROUP_JSON_ATTRIBUTE_RANGES], m_ranges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyGroup::GetImageId (void) const             { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::GetCreateGroupForSingleItem (void) const    { return m_createGroupForSingleItem; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::GetCreateGroupForUnspecifiedValues() const {return m_createGroupForUnspecifiedValues;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::SetCreateGroupForUnspecifiedValues(bool value) {m_createGroupForUnspecifiedValues = value;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyGroup::GetPropertyName (void) const        { return m_propertyName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupList const& PropertyGroup::GetRanges (void) const { return m_ranges;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::AddRange (PropertyRangeGroupSpecificationR range) 
    {
    InvalidateHash();
    range.SetParent(this);
    m_ranges.push_back(&range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyGroup::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = GroupSpecification::_ComputeHash(parentHash);
    md5.Add(m_imageId.c_str(), m_imageId.size());
    md5.Add(&m_createGroupForSingleItem, sizeof(m_createGroupForSingleItem));
    md5.Add(&m_createGroupForUnspecifiedValues, sizeof(m_createGroupForUnspecifiedValues));
    md5.Add(&m_groupingValue, sizeof(m_groupingValue));
    md5.Add(&m_sortingValue, sizeof(m_sortingValue));
    md5.Add(m_propertyName.c_str(), m_propertyName.size());

    Utf8String currentHash = md5.GetHashString();
    for (PropertyRangeGroupSpecificationP spec : m_ranges)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupSpecification::PropertyRangeGroupSpecification()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupSpecification::PropertyRangeGroupSpecification (Utf8StringCR label, Utf8StringCR imageId, Utf8StringCR fromValue, Utf8StringCR toValue)
    : m_label (label), m_imageId (imageId), m_fromValue (fromValue), m_toValue (toValue)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyRangeGroupSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_fromValue, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, PROPERTY_RANGE_GROUP_XML_NODE_NAME, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_toValue, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, PROPERTY_RANGE_GROUP_XML_NODE_NAME, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_LABEL))
        m_label = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_IMAGEID))
        m_imageId = "";    

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyRangeGroupSpecification::WriteXml (BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (PROPERTY_RANGE_GROUP_XML_NODE_NAME);

    specificationNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE, m_fromValue.c_str ());
    specificationNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE, m_toValue.c_str ());
    specificationNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_LABEL, m_label.c_str ());
    specificationNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_IMAGEID, m_imageId.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyRangeGroupSpecification::ReadJson(JsonValueCR json)
    {
    //Required
    m_fromValue = json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE].asCString("");
    if (m_fromValue.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyRangeGroupSpecification", PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE);
        return false;
        }

    m_toValue = json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE].asCString("");
    if (m_toValue.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyRangeGroupSpecification", PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE);
        return false;
        }

    //Optional
    m_imageId = json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_IMAGEID].asCString("");
    m_label = json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_LABEL].asCString("");

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PropertyRangeGroupSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE] = m_fromValue;
    json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE] = m_toValue;
    if (!m_imageId.empty())
        json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_IMAGEID] = m_imageId;
    if (!m_label.empty())
        json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_LABEL] = m_label;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyRangeGroupSpecification::GetLabel (void) const     { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyRangeGroupSpecification::GetImageId (void) const   { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyRangeGroupSpecification::GetFromValue (void) const { return m_fromValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyRangeGroupSpecification::GetToValue (void) const   { return m_toValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyRangeGroupSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_label.c_str(), m_label.size());
    md5.Add(m_imageId.c_str(), m_imageId.size());
    md5.Add(m_fromValue.c_str(), m_fromValue.size());
    md5.Add(m_toValue.c_str(), m_toValue.size());
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }
