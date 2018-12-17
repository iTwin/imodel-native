/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/ContentRule.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled)
    : ConditionalPresentationRule (condition, priority, onlyIfNotHandled), m_customControl ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule(ContentRuleCR other)
    : ConditionalPresentationRule(other), m_customControl(other.m_customControl)
    {
    CommonToolsInternal::CloneRules(m_specifications, other.m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::~ContentRule ()
    {
    CommonToolsInternal::FreePresentationRules (m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRule::_GetXmlElementName () const
    {
    return CONTENT_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_customControl, CONTENT_RULE_XML_ATTRIBUTE_CUSTOMCONTROL))
        m_customControl = "";

    for (BeXmlNodeP child = xmlNode->GetFirstChild (BEXMLNODE_Element); NULL != child; child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp(child->GetName(), CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<ContentInstancesOfSpecificClassesSpecification, ContentSpecificationList>(child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<ContentRelatedInstancesSpecification, ContentSpecificationList>(child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<SelectedNodeInstancesSpecification, ContentSpecificationList>(child, m_specifications);
        }

    return ConditionalPresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (CONTENT_RULE_XML_ATTRIBUTE_CUSTOMCONTROL, m_customControl.c_str ());

    CommonToolsInternal::WriteRulesToXmlNode<ContentSpecification, ContentSpecificationList> (xmlNode, m_specifications);

    ConditionalPresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRule::_GetJsonElementType() const
    {
    return CONTENT_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalPresentationRule::_ReadJson(json))
        return false;

    CommonToolsInternal::LoadFromJsonByPriority(json[CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications, ContentSpecification::Create);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::_WriteJson(JsonValueR json) const
    {
    ConditionalPresentationRule::_WriteJson(json);
    Json::Value specificationsJson(Json::arrayValue);
    CommonToolsInternal::WriteRulesToJson<ContentSpecification, ContentSpecificationList>(specificationsJson, m_specifications);
    json[CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS] = specificationsJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationList const& ContentRule::GetSpecifications (void) const { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::AddSpecification(ContentSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_specifications.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentRule::GetCustomControl (void)                  { return m_customControl;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::SetCustomControl (Utf8StringCR customControl)    { m_customControl = customControl; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ConditionalPresentationRule::_ComputeHash(parentHash);
    md5.Add(m_customControl.c_str(), m_customControl.size());

    Utf8String currentHash = md5.GetHashString();
    for (ContentSpecificationP spec : m_specifications)
        {
        Utf8StringCR specHash = spec->GetHash(parentHash);
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }
