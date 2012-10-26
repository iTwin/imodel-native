/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/GroupingRule.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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
        ECObjectsLogger::Log()->errorv (L"Invalid GroupingRuleXML: %hs element must contain a %hs attribute", GROUPING_RULE_XML_NODE_NAME, GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_className, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid GroupingRuleXML: %hs element must contain a %hs attribute", GROUPING_RULE_XML_NODE_NAME, GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuCondition, GROUPING_RULE_XML_ATTRIBUTE_MENUCONDITION))
        m_contextMenuCondition = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuLabel, GROUPING_RULE_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = L"";

    //Load PropertyGroups
    CommonTools::LoadRulesFromXmlNode<PropertyGroup, PropertyGroupList> (xmlNode, m_groups, PROPERTY_GROUP_XML_NODE_NAME);

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

    CommonTools::WriteRulesToXmlNode<PropertyGroup, PropertyGroupList> (xmlNode, m_groups);

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyGroup::~PropertyGroup (void)
    {
    CommonTools::FreePresentationRules (m_ranges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyGroup::ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyName, PROPERTY_GROUP_XML_ATTRIBUTE_PROPERTYNAME))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid PropertyGroupXML: %hs element must contain a %hs attribute", PROPERTY_GROUP_XML_NODE_NAME, PROPERTY_GROUP_XML_ATTRIBUTE_PROPERTYNAME);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, PROPERTY_GROUP_XML_ATTRIBUTE_IMAGEID))
        m_imageId = L"";
    
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuLabel, PROPERTY_GROUP_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = L"";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_createGroupForSingleItem, PROPERTY_GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM))
        m_createGroupForSingleItem = false;

    //Load Ranges
    CommonTools::LoadRulesFromXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (xmlNode, m_ranges, PROPERTY_RANGE_GROUP_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (PROPERTY_GROUP_XML_NODE_NAME);

    specificationNode->AddAttributeStringValue  (PROPERTY_GROUP_XML_ATTRIBUTE_PROPERTYNAME, m_propertyName.c_str ());
    specificationNode->AddAttributeStringValue  (PROPERTY_GROUP_XML_ATTRIBUTE_IMAGEID, m_imageId.c_str ());
    specificationNode->AddAttributeStringValue  (PROPERTY_GROUP_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel.c_str ());
    specificationNode->AddAttributeBooleanValue (PROPERTY_GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM, m_createGroupForSingleItem);

    CommonTools::WriteRulesToXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (specificationNode, m_ranges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyRangeGroupSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_fromValue, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid PropertyRangeGroupSpecificationXML: %hs element must contain a %hs attribute", PROPERTY_RANGE_GROUP_XML_NODE_NAME, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_toValue, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid PropertyRangeGroupSpecificationXML: %hs element must contain a %hs attribute", PROPERTY_RANGE_GROUP_XML_NODE_NAME, PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE);
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