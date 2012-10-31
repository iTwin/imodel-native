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

    //Load Class and Property Groups
    for (BeXmlNodeP child = xmlNode->GetFirstChild (BEXMLNODE_Element); NULL != child; child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), CLASS_GROUP_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<ClassGroup, GroupList> (child, m_groups);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), PROPERTY_GROUP_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<PropertyGroup, GroupList> (child, m_groups);
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

    CommonTools::WriteRulesToXmlNode<GroupSpecification, GroupList> (xmlNode, m_groups);

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, GROUP_XML_ATTRIBUTE_IMAGEID))
        m_imageId = L"";
    
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_contextMenuLabel, GROUP_XML_ATTRIBUTE_MENULABEL))
        m_contextMenuLabel = L"";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_createGroupForSingleItem, GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM))
        m_createGroupForSingleItem = false;

    //Make sure we call protected override
    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupSpecification::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    specificationNode->AddAttributeStringValue  (GROUP_XML_ATTRIBUTE_IMAGEID, m_imageId.c_str ());
    specificationNode->AddAttributeStringValue  (GROUP_XML_ATTRIBUTE_MENULABEL, m_contextMenuLabel.c_str ());
    specificationNode->AddAttributeBooleanValue (GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM, m_createGroupForSingleItem);

    //Make sure we call protected override
    _WriteXml (specificationNode);
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
    xmlNode->AddAttributeStringValue  (CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME, m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue  (CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME, m_baseClassName.c_str ());
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
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyName, PROPERTY_GROUP_XML_ATTRIBUTE_PROPERTYNAME))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid PropertyGroupXML: %hs element must contain a %hs attribute", PROPERTY_GROUP_XML_NODE_NAME, PROPERTY_GROUP_XML_ATTRIBUTE_PROPERTYNAME);
        return false;
        }

    //Optional:
    //Load Ranges
    CommonTools::LoadRulesFromXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (xmlNode, m_ranges, PROPERTY_RANGE_GROUP_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyGroup::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue  (PROPERTY_GROUP_XML_ATTRIBUTE_PROPERTYNAME, m_propertyName.c_str ());
    CommonTools::WriteRulesToXmlNode<PropertyRangeGroupSpecification, PropertyRangeGroupList> (xmlNode, m_ranges);
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