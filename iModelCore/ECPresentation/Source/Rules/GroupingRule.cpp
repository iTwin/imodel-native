/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::GroupingRule() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::GroupingRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR contextMenuCondition, Utf8StringCR contextMenuLabel, Utf8StringCR settingsId)
    : ConditionalCustomizationRule(condition, priority, onlyIfNotHandled),
      m_schemaName (schemaName), m_className (className), m_contextMenuCondition (contextMenuCondition), m_contextMenuLabel (contextMenuLabel), m_settingsId (settingsId)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::GroupingRule(GroupingRuleCR other)
    : ConditionalCustomizationRule(other), m_schemaName(other.m_schemaName), m_className(other.m_className), m_contextMenuCondition(other.m_contextMenuCondition),
    m_contextMenuLabel(other.m_contextMenuLabel), m_settingsId(other.m_settingsId)
    {
    CommonToolsInternal::CloneRules(m_groups, other.m_groups, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::~GroupingRule (void)
    {
    CommonToolsInternal::FreePresentationRules(m_groups);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GroupingRule::_GetXmlElementName () const
    {
    return GROUPING_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupingRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ConditionalCustomizationRule::_ReadXml(xmlNode))
        return false;

    // required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_schemaName, GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, GROUPING_RULE_XML_NODE_NAME, GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME));
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_className, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, GROUPING_RULE_XML_NODE_NAME, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME));
        return false;
        }

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuCondition, GROUPING_RULE_XML_ATTRIBUTE_MENUCONDITION))
        m_contextMenuCondition = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuLabel, GROUPING_RULE_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_settingsId, GROUPING_RULE_XML_ATTRIBUTE_SETTINGSID))
        m_settingsId = "";

    // load Class and Property Groups
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GroupingRule::_GetJsonElementType() const
    {
    return GROUPING_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupingRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    // required:
    CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_className, json[COMMON_JSON_ATTRIBUTE_CLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_CLASS).c_str());
    if (CommonToolsInternal::CheckRuleIssue(m_schemaName.empty() || m_className.empty(), _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_CLASS, json[COMMON_JSON_ATTRIBUTE_CLASS], "at least one class"))
        return false;

    // optional:
    if (json.isMember(GROUPING_RULE_JSON_ATTRIBUTE_GROUPS))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), GROUPING_RULE_JSON_ATTRIBUTE_GROUPS, json[GROUPING_RULE_JSON_ATTRIBUTE_GROUPS], m_groups, GroupSpecification::Create, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetSchemaName (void) const              { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetClassName (void) const               { return m_className; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetContextMenuCondition (void) const    { return m_contextMenuCondition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::SetContextMenuCondition (Utf8String value)      { m_contextMenuCondition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetContextMenuLabel (void) const        { return m_contextMenuLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupingRule::GetSettingsId (void) const              { return m_settingsId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupList const& GroupingRule::GetGroups (void) const            { return m_groups; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::AddGroup(GroupSpecificationR group)
    {
    ADD_HASHABLE_CHILD(m_groups, group);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 GroupingRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_schemaName.empty())
        ADD_STR_VALUE_TO_HASH(md5, GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME, m_schemaName);
    if (!m_className.empty())
        ADD_STR_VALUE_TO_HASH(md5, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME, m_className);
    if (!m_contextMenuCondition.empty())
        ADD_STR_VALUE_TO_HASH(md5, GROUPING_RULE_XML_ATTRIBUTE_MENUCONDITION, m_contextMenuCondition);
    if (!m_contextMenuLabel.empty())
        ADD_STR_VALUE_TO_HASH(md5, GROUPING_RULE_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel);
    if (!m_settingsId.empty())
        ADD_STR_VALUE_TO_HASH(md5, GROUPING_RULE_XML_ATTRIBUTE_SETTINGSID, m_settingsId);
    ADD_RULES_TO_HASH(md5, GROUPING_RULE_JSON_ATTRIBUTE_GROUPS, m_groups);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupingRule::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ConditionalCustomizationRule::_ShallowEqual(other))
        return false;

    GroupingRule const* otherRule = dynamic_cast<GroupingRule const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_schemaName == otherRule->m_schemaName
        && m_className == otherRule->m_className
        && m_contextMenuCondition == otherRule->m_contextMenuCondition
        && m_contextMenuLabel == otherRule->m_contextMenuLabel
        && m_settingsId == otherRule->m_settingsId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    else
        {
        Utf8String msg = json.isMember(COMMON_JSON_ATTRIBUTE_SPECTYPE)
            ? Utf8PrintfString("Invalid `" COMMON_JSON_ATTRIBUTE_SPECTYPE "` attribute value: `%s`", type)
            : Utf8String("Missing required attribute: `" COMMON_JSON_ATTRIBUTE_SPECTYPE "`");
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, msg);
        }
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecification::GroupSpecification () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecification::GroupSpecification (Utf8StringCR contextMenuLabel, Utf8CP defaultLabel)
    : m_contextMenuLabel (contextMenuLabel), m_defaultLabel (defaultLabel)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupSpecification::Accept(GroupingRuleSpecificationVisitor& visitor) const {_Accept(visitor);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_contextMenuLabel, GROUP_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_defaultLabel, GROUP_XML_ATTRIBUTE_DEFAULTLABEL))
        m_defaultLabel.clear();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupSpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    PresentationKey::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(GROUP_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel.c_str());
    xmlNode->AddAttributeStringValue(GROUP_XML_ATTRIBUTE_DEFAULTLABEL, m_defaultLabel.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP GroupSpecification::_GetJsonElementTypeAttributeName() const { return COMMON_JSON_ATTRIBUTE_SPECTYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupSpecification::_ReadJson(JsonValueCR json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    m_defaultLabel = json[GROUP_JSON_ATTRIBUTE_DEFAULTLABEL].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupSpecification::_WriteJson(JsonValueR json) const
    {
    PresentationKey::_WriteJson(json);
    if (!m_defaultLabel.empty())
        json[GROUP_JSON_ATTRIBUTE_DEFAULTLABEL] = m_defaultLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR GroupSpecification::GetContextMenuLabel (void) const  { return m_contextMenuLabel; }
Utf8StringCR GroupSpecification::GetDefaultLabel (void) const      { return m_defaultLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 GroupSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_contextMenuLabel.empty())
        ADD_STR_VALUE_TO_HASH(md5, GROUP_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel);
    if (!m_defaultLabel.empty())
        ADD_STR_VALUE_TO_HASH(md5, GROUP_XML_ATTRIBUTE_DEFAULTLABEL, m_defaultLabel);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    GroupSpecification const* otherRule = dynamic_cast<GroupSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_contextMenuLabel == otherRule->m_contextMenuLabel
        && m_defaultLabel == otherRule->m_defaultLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static SameLabelInstanceGroupApplicationStage GetSameLabelInstanceGroupApplicationStageFromString(Utf8StringCR str)
    {
    if (str.Equals(SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE_VALUES_QUERY))
        return SameLabelInstanceGroupApplicationStage::Query;
    else if (str.Equals(SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE_VALUES_POSTPROCESS))
        return SameLabelInstanceGroupApplicationStage::PostProcess;

    DIAGNOSTICS_EDITOR_LOG(DiagnosticsCategory::Rules, LOG_ERROR, Utf8PrintfString("Failed to parse same-label instance group application stage: '%s'. Defaulting to 'Query'.", str.c_str()));
    return SameLabelInstanceGroupApplicationStage::Query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetSameLabelInstanceGroupApplicationStageAsString(SameLabelInstanceGroupApplicationStage value)
    {
    switch (value)
        {
        case SameLabelInstanceGroupApplicationStage::Query: return SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE_VALUES_QUERY;
        case SameLabelInstanceGroupApplicationStage::PostProcess: return SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE_VALUES_POSTPROCESS;
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Serialization, LOG_ERROR, Utf8PrintfString("Failed to serialize same-label instance group application stage: %d. Defaulting to 'Query'.", (int)value));
    return SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE_VALUES_QUERY;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SameLabelInstanceGroup::_Accept(GroupingRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SameLabelInstanceGroup::_GetXmlElementName () const
    {
    return SAME_LABEL_INSTANCE_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SameLabelInstanceGroup::_GetJsonElementType() const
    {
    return SAME_LABEL_INSTANCE_GROUP_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SameLabelInstanceGroup::_ReadJson(JsonValueCR json)
    {
    if (!GroupSpecification::_ReadJson(json))
        return false;

    m_applicationStage = GetSameLabelInstanceGroupApplicationStageFromString(json[SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE].asCString(SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE_VALUES_QUERY));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SameLabelInstanceGroup::_WriteJson(JsonValueR json) const
    {
    GroupSpecification::_WriteJson(json);
    if (m_applicationStage != SameLabelInstanceGroupApplicationStage::Query)
        json[SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE] = GetSameLabelInstanceGroupApplicationStageAsString(m_applicationStage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SameLabelInstanceGroup::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_applicationStage != SameLabelInstanceGroupApplicationStage::Query)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE, m_applicationStage);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SameLabelInstanceGroup::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!GroupSpecification::_ShallowEqual(other))
        return false;

    SameLabelInstanceGroup const* otherRule = dynamic_cast<SameLabelInstanceGroup const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_applicationStage == otherRule->m_applicationStage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassGroup::ClassGroup ()
    : GroupSpecification (), m_createGroupForSingleItem (false), m_schemaName (""), m_baseClassName ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassGroup::ClassGroup (Utf8StringCR contextMenuLabel, bool createGroupForSingleItem, Utf8StringCR schemaName, Utf8StringCR baseClassName)
    : GroupSpecification (contextMenuLabel), m_createGroupForSingleItem (createGroupForSingleItem), m_schemaName (schemaName), m_baseClassName (baseClassName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassGroup::_Accept(GroupingRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ClassGroup::_GetXmlElementName () const
    {
    return CLASS_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!GroupSpecification::_ReadXml(xmlNode))
        return false;

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_createGroupForSingleItem, GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM))
        m_createGroupForSingleItem = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_schemaName, CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME))
        m_schemaName = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_baseClassName, CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME))
        m_baseClassName = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassGroup::_WriteXml (BeXmlNodeP xmlNode) const
    {
    GroupSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue (GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM, m_createGroupForSingleItem);
    xmlNode->AddAttributeStringValue  (CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME,         m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue  (CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME,      m_baseClassName.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ClassGroup::_GetJsonElementType() const
    {
    return CLASS_GROUP_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::_ReadJson (JsonValueCR json)
    {
    if (!GroupSpecification::_ReadJson(json))
        return false;

    m_createGroupForSingleItem = json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM].asBool(false);
    if (json.isMember(CLASS_GROUP_JSON_ATTRIBUTE_BASECLASS))
        CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_baseClassName, json[CLASS_GROUP_JSON_ATTRIBUTE_BASECLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), CLASS_GROUP_JSON_ATTRIBUTE_BASECLASS).c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::GetCreateGroupForSingleItem (void) const       { return m_createGroupForSingleItem; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClassGroup::GetSchemaName (void) const                { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClassGroup::GetBaseClassName (void) const             { return m_baseClassName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ClassGroup::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_createGroupForSingleItem)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM, m_createGroupForSingleItem);
    if (!m_schemaName.empty())
        ADD_STR_VALUE_TO_HASH(md5, CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME, m_schemaName);
    if (!m_baseClassName.empty())
        ADD_STR_VALUE_TO_HASH(md5, CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME, m_baseClassName);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!GroupSpecification::_ShallowEqual(other))
        return false;

    ClassGroup const* otherRule = dynamic_cast<ClassGroup const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_createGroupForSingleItem == otherRule->m_createGroupForSingleItem
        && m_schemaName == otherRule->m_schemaName
        && m_baseClassName == otherRule->m_baseClassName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::PropertyGroup ()
    : GroupSpecification(), m_createGroupForSingleItem(false), m_createGroupForUnspecifiedValues(true),
    m_groupingValue(PropertyGroupingValue::DisplayLabel), m_sortingValue(PropertyGroupingValue::DisplayLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::PropertyGroup (Utf8StringCR contextMenuLabel, Utf8StringCR imageId, bool createGroupForSingleItem, Utf8StringCR propertyName, Utf8CP defaultLabel)
    : GroupSpecification (contextMenuLabel, defaultLabel), m_imageId (imageId),
    m_createGroupForSingleItem (createGroupForSingleItem), m_createGroupForUnspecifiedValues(true),
    m_propertyName (propertyName), m_groupingValue(PropertyGroupingValue::DisplayLabel), m_sortingValue(PropertyGroupingValue::DisplayLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::PropertyGroup(PropertyGroupCR other)
    : GroupSpecification(other), m_imageId(other.m_imageId), m_createGroupForSingleItem(other.m_createGroupForSingleItem),
    m_createGroupForUnspecifiedValues(other.m_createGroupForUnspecifiedValues), m_propertyName(other.m_propertyName),
    m_groupingValue(other.m_groupingValue), m_sortingValue(other.m_sortingValue)
    {
    CommonToolsInternal::CopyRules(m_ranges, other.m_ranges, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::~PropertyGroup (void)
    {
    CommonToolsInternal::FreePresentationRules(m_ranges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::_Accept(GroupingRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyGroup::_GetXmlElementName () const
    {
    return PROPERTY_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PropertyGroupingValue GetPropertyGroupingValueFromString(Utf8StringCR str)
    {
    if (str.Equals(PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL))
        return PropertyGroupingValue::DisplayLabel;
    else if (str.Equals(PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_PROPERTYVALUE))
        return PropertyGroupingValue::PropertyValue;

    DIAGNOSTICS_EDITOR_LOG(DiagnosticsCategory::Rules, LOG_ERROR, Utf8PrintfString("Failed to parse property grouping value: '%s'. Defaulting to 'DisplayLabel'.", str.c_str()));
    return PropertyGroupingValue::DisplayLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetPropertyGroupingValueAsString(PropertyGroupingValue value)
    {
    switch (value)
        {
        case PropertyGroupingValue::PropertyValue:  return PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_PROPERTYVALUE;
        case PropertyGroupingValue::DisplayLabel:   return PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL;
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Serialization, LOG_ERROR, Utf8PrintfString("Failed to serialize property grouping value: %d. Defaulting to 'DisplayLabel'.", (int)value));
    return PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!GroupSpecification::_ReadXml(xmlNode))
        return false;

    // required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyName, COMMON_XML_ATTRIBUTE_PROPERTYNAME))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, PROPERTY_GROUP_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_PROPERTYNAME));
        return false;
        }

    // optional:
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

    // load Ranges
    CommonToolsInternal::LoadSpecificationsFromXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (xmlNode, m_ranges, PROPERTY_RANGE_GROUP_XML_NODE_NAME, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyGroup::_GetJsonElementType() const
    {
    return PROPERTY_GROUP_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::_ReadJson (JsonValueCR json)
    {
    if (!GroupSpecification::_ReadJson(json))
        return false;

    // required:
    m_propertyName = json[COMMON_JSON_ATTRIBUTE_PROPERTYNAME].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_propertyName.empty(), _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_PROPERTYNAME, json[COMMON_JSON_ATTRIBUTE_PROPERTYNAME], "non-empty string"))
        return false;

    // optional:
    m_imageId = json[GROUP_JSON_ATTRIBUTE_IMAGEID].asCString("");
    m_createGroupForSingleItem = json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM].asBool(false);
    m_createGroupForUnspecifiedValues = json[GROUP_JSON_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES].asBool(true);
    if (json.isMember(PROPERTY_GROUP_JSON_ATTRIBUTE_GROUPINGVALUE))
        m_groupingValue = GetPropertyGroupingValueFromString(json[PROPERTY_GROUP_JSON_ATTRIBUTE_GROUPINGVALUE].asCString());
    if (json.isMember(PROPERTY_GROUP_JSON_ATTRIBUTE_SORTINGVALUE))
        m_sortingValue = GetPropertyGroupingValueFromString(json[PROPERTY_GROUP_JSON_ATTRIBUTE_SORTINGVALUE].asCString());
    if (json.isMember(PROPERTY_GROUP_JSON_ATTRIBUTE_RANGES))
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), PROPERTY_GROUP_JSON_ATTRIBUTE_RANGES, json[PROPERTY_GROUP_JSON_ATTRIBUTE_RANGES],
            m_ranges, CommonToolsInternal::LoadRuleFromJson<PropertyRangeGroupSpecification>, this);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyGroup::GetImageId (void) const             { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::GetCreateGroupForSingleItem (void) const    { return m_createGroupForSingleItem; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::GetCreateGroupForUnspecifiedValues() const {return m_createGroupForUnspecifiedValues;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::SetCreateGroupForUnspecifiedValues(bool value) {m_createGroupForUnspecifiedValues = value;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyGroup::GetPropertyName (void) const        { return m_propertyName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupList const& PropertyGroup::GetRanges (void) const { return m_ranges;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::AddRange (PropertyRangeGroupSpecificationR range)
    {
    ADD_HASHABLE_CHILD(m_ranges, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyGroup::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_imageId.empty())
        ADD_STR_VALUE_TO_HASH(md5, GROUP_JSON_ATTRIBUTE_IMAGEID, m_imageId);
    if (m_createGroupForSingleItem)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM, m_createGroupForSingleItem);
    if (!m_createGroupForUnspecifiedValues)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, GROUP_JSON_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES, m_createGroupForUnspecifiedValues);
    if (m_groupingValue != PropertyGroupingValue::DisplayLabel)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_GROUP_JSON_ATTRIBUTE_GROUPINGVALUE, m_groupingValue);
    if (m_sortingValue != PropertyGroupingValue::DisplayLabel)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_GROUP_JSON_ATTRIBUTE_SORTINGVALUE, m_sortingValue);
    if (!m_propertyName.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_PROPERTYNAME, m_propertyName);
    ADD_RULES_TO_HASH(md5, PROPERTY_GROUP_JSON_ATTRIBUTE_RANGES, m_ranges);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!GroupSpecification::_ShallowEqual(other))
        return false;

    PropertyGroup const* otherRule = dynamic_cast<PropertyGroup const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_imageId == otherRule->m_imageId
        && m_createGroupForSingleItem == otherRule->m_createGroupForSingleItem
        && m_createGroupForUnspecifiedValues == otherRule->m_createGroupForUnspecifiedValues
        && m_groupingValue == otherRule->m_groupingValue
        && m_sortingValue == otherRule->m_sortingValue
        && m_propertyName == otherRule->m_propertyName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupSpecification::PropertyRangeGroupSpecification()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupSpecification::PropertyRangeGroupSpecification (Utf8StringCR label, Utf8StringCR imageId, Utf8StringCR fromValue, Utf8StringCR toValue)
    : m_label (label), m_imageId (imageId), m_fromValue (fromValue), m_toValue (toValue)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyRangeGroupSpecification::_GetXmlElementName() const {return PROPERTY_RANGE_GROUP_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyRangeGroupSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    // required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_fromValue, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, PROPERTY_RANGE_GROUP_XML_NODE_NAME, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE));
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_toValue, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, PROPERTY_RANGE_GROUP_XML_NODE_NAME, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE));
        return false;
        }

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_LABEL))
        m_label = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_IMAGEID))
        m_imageId = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyRangeGroupSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE, m_fromValue.c_str ());
    xmlNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE, m_toValue.c_str ());
    xmlNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_LABEL, m_label.c_str ());
    xmlNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_IMAGEID, m_imageId.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyRangeGroupSpecification::_GetJsonElementType() const { return "PropertyRangeGroupSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyRangeGroupSpecification::_ReadJson(JsonValueCR json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    m_fromValue = json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_fromValue.empty(), _GetXmlElementName(), PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE, json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE], "non-empty string"))
        return false;

    m_toValue = json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_toValue.empty(), _GetXmlElementName(), PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE, json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE], "non-empty string"))
        return false;

    // optional:
    m_imageId = json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_IMAGEID].asCString("");
    m_label = json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_LABEL].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyRangeGroupSpecification::_WriteJson(JsonValueR json) const
    {
    json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE] = m_fromValue;
    json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE] = m_toValue;
    if (!m_imageId.empty())
        json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_IMAGEID] = m_imageId;
    if (!m_label.empty())
        json[PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_LABEL] = m_label;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyRangeGroupSpecification::GetLabel (void) const     { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyRangeGroupSpecification::GetImageId (void) const   { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyRangeGroupSpecification::GetFromValue (void) const { return m_fromValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PropertyRangeGroupSpecification::GetToValue (void) const   { return m_toValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyRangeGroupSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_label.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_LABEL, m_label);
    if (!m_imageId.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_IMAGEID, m_imageId);
    if (!m_fromValue.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE, m_fromValue);
    if (!m_toValue.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE, m_toValue);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyRangeGroupSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    PropertyRangeGroupSpecification const* otherRule = dynamic_cast<PropertyRangeGroupSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_label == otherRule->m_label
        && m_imageId == otherRule->m_imageId
        && m_fromValue == otherRule->m_fromValue
        && m_toValue == otherRule->m_toValue;
    }
