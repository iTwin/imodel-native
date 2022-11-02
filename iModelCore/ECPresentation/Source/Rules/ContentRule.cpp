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
ContentRule::ContentRule() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled)
    : ConditionalPresentationRule (condition, priority, onlyIfNotHandled), m_customControl ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule(ContentRuleCR other)
    : ConditionalPresentationRule(other), m_customControl(other.m_customControl)
    {
    CommonToolsInternal::CloneRules(m_specifications, other.m_specifications, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::~ContentRule ()
    {
    CommonToolsInternal::FreePresentationRules(m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRule::_GetXmlElementName () const
    {
    return CONTENT_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_customControl, CONTENT_RULE_XML_ATTRIBUTE_CUSTOMCONTROL))
        m_customControl = "";

    for (BeXmlNodeP child = xmlNode->GetFirstChild (BEXMLNODE_Element); NULL != child; child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp(child->GetName(), CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<ContentInstancesOfSpecificClassesSpecification, ContentSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<ContentRelatedInstancesSpecification, ContentSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<SelectedNodeInstancesSpecification, ContentSpecificationList>(child, m_specifications, this);
        }

    return ConditionalPresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ConditionalPresentationRule::_WriteXml (xmlNode);
    xmlNode->AddAttributeStringValue (CONTENT_RULE_XML_ATTRIBUTE_CUSTOMCONTROL, m_customControl.c_str ());
    CommonToolsInternal::WriteRulesToXmlNode<ContentSpecification, ContentSpecificationList> (xmlNode, m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRule::_GetJsonElementType() const
    {
    return CONTENT_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalPresentationRule::_ReadJson(json))
        return false;

    if (json.isMember(CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS))
        CommonToolsInternal::LoadFromJsonByPriority(_GetJsonElementType(), CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS, json[CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications, ContentSpecification::Create, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::_WriteJson(JsonValueR json) const
    {
    ConditionalPresentationRule::_WriteJson(json);
    Json::Value specificationsJson(Json::arrayValue);
    CommonToolsInternal::WriteRulesToJson<ContentSpecification, ContentSpecificationList>(specificationsJson, m_specifications);
    json[CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS] = specificationsJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationList const& ContentRule::GetSpecifications (void) const { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::AddSpecification(ContentSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    CommonTools::AddToListByPriority(m_specifications, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentRule::GetCustomControl (void)                  { return m_customControl;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::SetCustomControl (Utf8StringCR customControl)    { m_customControl = customControl; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_customControl.empty())
        ADD_STR_VALUE_TO_HASH(md5, CONTENT_RULE_XML_ATTRIBUTE_CUSTOMCONTROL, m_customControl);
    ADD_RULES_TO_HASH(md5, CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS, m_specifications);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRule::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ConditionalPresentationRule::_ShallowEqual(other))
        return false;

    ContentRule const* otherRule = dynamic_cast<ContentRule const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_customControl == otherRule->m_customControl;
    }
