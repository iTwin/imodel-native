/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/GroupingRule.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "CommonTools.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::GroupingRule ()
    : PresentationRule (), m_schemaName (L""), m_className (L""), m_contextMenuCondition (L""), m_contextMenuLabel (L""), m_settingsId (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::GroupingRule (WStringCR condition, int priority, bool onlyIfNotHandled, WStringCR schemaName, WStringCR className, WStringCR contextMenuCondition, WStringCR contextMenuLabel, WStringCR settingsId)
    : PresentationRule (condition, priority, onlyIfNotHandled), 
      m_schemaName (schemaName), m_className (className), m_contextMenuCondition (contextMenuCondition), m_contextMenuLabel (contextMenuLabel), m_settingsId (settingsId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRule::~GroupingRule (void)
    {
    CommonTools::FreePresentationRules (m_groups);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP GroupingRule::_GetXmlElementName ()
    {
    return GROUPING_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupingRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_schemaName, GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME))
        {
        LOG.errorv (L"Invalid GroupingRuleXML: %hs element must contain a %hs attribute", GROUPING_RULE_XML_NODE_NAME, GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_className, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME))
        {
        LOG.errorv (L"Invalid GroupingRuleXML: %hs element must contain a %hs attribute", GROUPING_RULE_XML_NODE_NAME, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuCondition, GROUPING_RULE_XML_ATTRIBUTE_MENUCONDITION))
        m_contextMenuCondition = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuLabel, GROUPING_RULE_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_settingsId, GROUPING_RULE_XML_ATTRIBUTE_SETTINGSID))
        m_settingsId = L"";

    //Load Class and Property Groups
    for (BeXmlNodeP child = xmlNode->GetFirstChild (BEXMLNODE_Element); NULL != child; child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), CLASS_GROUP_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<ClassGroup, GroupList> (child, m_groups);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), PROPERTY_GROUP_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<PropertyGroup, GroupList> (child, m_groups);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), SAMEL_LABEL_INSTANCE_GROUP_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<SameLabelInstanceGroup, GroupList> (child, m_groups);
        }

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME, m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME, m_className.c_str ());
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_MENUCONDITION, m_contextMenuCondition.c_str ());
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel.c_str ());
    xmlNode->AddAttributeStringValue (GROUPING_RULE_XML_ATTRIBUTE_SETTINGSID, m_settingsId.c_str ());

    CommonTools::WriteRulesToXmlNode<GroupSpecification, GroupList> (xmlNode, m_groups);

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR GroupingRule::GetSchemaName (void) const              { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR GroupingRule::GetClassName (void) const               { return m_className; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR GroupingRule::GetContextMenuCondition (void) const    { return m_contextMenuCondition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupingRule::SetContextMenuCondition (WString value)      { m_contextMenuCondition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR GroupingRule::GetContextMenuLabel (void) const        { return m_contextMenuLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR GroupingRule::GetSettingsId (void) const              { return m_settingsId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupList& GroupingRule::GetGroups (void)                       { return m_groups; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecification::GroupSpecification () : m_contextMenuLabel (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecification::GroupSpecification (WStringCR contextMenuLabel, WCharCP defaultLabel) : m_contextMenuLabel (contextMenuLabel), m_defaultLabel (defaultLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuLabel, GROUP_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = L"";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_defaultLabel, GROUP_XML_ATTRIBUTE_DEFAULTLABEL))
        m_defaultLabel.clear();

    //Make sure we call protected override
    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupSpecification::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    specificationNode->AddAttributeStringValue  (GROUP_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel.c_str ());
    specificationNode->AddAttributeStringValue (GROUP_XML_ATTRIBUTE_DEFAULTLABEL, m_defaultLabel.c_str());

    //Make sure we call protected override
    _WriteXml (specificationNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR GroupSpecification::GetContextMenuLabel (void) const  { return m_contextMenuLabel; }
WStringCR GroupSpecification::GetDefaultLabel (void) const      { return m_defaultLabel; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SameLabelInstanceGroup::SameLabelInstanceGroup () : GroupSpecification ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SameLabelInstanceGroup::SameLabelInstanceGroup (WStringCR contextMenuLabel) : GroupSpecification (contextMenuLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP SameLabelInstanceGroup::_GetXmlElementName ()
    {
    return SAMEL_LABEL_INSTANCE_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SameLabelInstanceGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SameLabelInstanceGroup::_WriteXml (BeXmlNodeP xmlNode)
    {
    //there are no additioanl options
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ClassGroup::ClassGroup ()
    : GroupSpecification (), m_createGroupForSingleItem (false), m_schemaName (L""), m_baseClassName (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ClassGroup::ClassGroup (WStringCR contextMenuLabel, bool createGroupForSingleItem, WStringCR schemaName, WStringCR baseClassName)
    : GroupSpecification (contextMenuLabel), m_createGroupForSingleItem (createGroupForSingleItem), m_schemaName (schemaName), m_baseClassName (baseClassName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ClassGroup::_GetXmlElementName ()
    {
    return CLASS_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_createGroupForSingleItem, GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM))
        m_createGroupForSingleItem = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_schemaName, CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME))
        m_schemaName = L"";
    
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_baseClassName, CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME))
        m_baseClassName = L"";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassGroup::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeBooleanValue (GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM, m_createGroupForSingleItem);
    xmlNode->AddAttributeStringValue  (CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME,         m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue  (CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME,      m_baseClassName.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassGroup::GetCreateGroupForSingleItem (void) const       { return m_createGroupForSingleItem; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ClassGroup::GetSchemaName (void) const                { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ClassGroup::GetBaseClassName (void) const             { return m_baseClassName; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::PropertyGroup ()
    : GroupSpecification (), m_imageId (L""), m_createGroupForSingleItem (false), m_propertyName (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::PropertyGroup (WStringCR contextMenuLabel, WStringCR imageId, bool createGroupForSingleItem, WStringCR propertyName, WCharCP defaultLabel)
    : GroupSpecification (contextMenuLabel, defaultLabel), m_imageId (imageId), m_createGroupForSingleItem (createGroupForSingleItem), m_propertyName (propertyName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::~PropertyGroup (void)
    {
    CommonTools::FreePresentationRules (m_ranges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP PropertyGroup::_GetXmlElementName ()
    {
    return PROPERTY_GROUP_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyName, COMMON_XML_ATTRIBUTE_PROPERTYNAME))
        {
        LOG.errorv (L"Invalid PropertyGroupXML: %hs element must contain a %hs attribute", PROPERTY_GROUP_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_PROPERTYNAME);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, GROUP_XML_ATTRIBUTE_IMAGEID))
        m_imageId = L"";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_createGroupForSingleItem, GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM))
        m_createGroupForSingleItem = false;

    //Load Ranges
    CommonTools::LoadRulesFromXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (xmlNode, m_ranges, PROPERTY_RANGE_GROUP_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue  (GROUP_XML_ATTRIBUTE_IMAGEID,                  m_imageId.c_str ());
    xmlNode->AddAttributeBooleanValue (GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM, m_createGroupForSingleItem);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_PROPERTYNAME,            m_propertyName.c_str ());
    CommonTools::WriteRulesToXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (xmlNode, m_ranges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PropertyGroup::GetImageId (void) const                { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::GetCreateGroupForSingleItem (void) const    { return m_createGroupForSingleItem; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PropertyGroup::GetPropertyName (void) const           { return m_propertyName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupList& PropertyGroup::GetRanges (void)         { return m_ranges;    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupSpecification::PropertyRangeGroupSpecification ()
    : m_label (L""), m_imageId (L""), m_fromValue (L""), m_toValue (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyRangeGroupSpecification::PropertyRangeGroupSpecification (WStringCR label, WStringCR imageId, WStringCR fromValue, WStringCR toValue)
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
        LOG.errorv (L"Invalid PropertyRangeGroupSpecificationXML: %hs element must contain a %hs attribute", PROPERTY_RANGE_GROUP_XML_NODE_NAME, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_toValue, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE))
        {
        LOG.errorv (L"Invalid PropertyRangeGroupSpecificationXML: %hs element must contain a %hs attribute", PROPERTY_RANGE_GROUP_XML_NODE_NAME, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_LABEL))
        m_label = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_IMAGEID))
        m_imageId = L"";    

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyRangeGroupSpecification::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (PROPERTY_RANGE_GROUP_XML_NODE_NAME);

    specificationNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE, m_fromValue.c_str ());
    specificationNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE, m_toValue.c_str ());
    specificationNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_LABEL, m_label.c_str ());
    specificationNode->AddAttributeStringValue (PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_IMAGEID, m_imageId.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PropertyRangeGroupSpecification::GetLabel (void) const     { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PropertyRangeGroupSpecification::GetImageId (void) const   { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PropertyRangeGroupSpecification::GetFromValue (void) const { return m_fromValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PropertyRangeGroupSpecification::GetToValue (void) const   { return m_toValue; }
