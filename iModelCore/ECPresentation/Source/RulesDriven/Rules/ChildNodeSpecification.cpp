/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/ChildNodeSpecification.cpp $
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
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification ()
: m_priority (1000), m_alwaysReturnsChildren (false), m_hideNodesInHierarchy (false), m_hideIfNoChildren (false), m_extendedData (L""), m_doNotSort (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren)
: m_priority (priority), m_alwaysReturnsChildren (alwaysReturnsChildren), m_hideNodesInHierarchy (hideNodesInHierarchy), m_hideIfNoChildren (hideIfNoChildren), m_extendedData (L""), m_doNotSort (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification(ChildNodeSpecificationCR other)
    : m_priority(other.m_priority), m_alwaysReturnsChildren(other.m_alwaysReturnsChildren),
    m_hideNodesInHierarchy(other.m_hideNodesInHierarchy), m_hideIfNoChildren(other.m_hideIfNoChildren),
    m_doNotSort(other.m_doNotSort), m_extendedData(other.m_extendedData)
    {
    CommonTools::CopyRules(m_relatedInstances, other.m_relatedInstances);
    CommonTools::CopyRules(m_nestedRules, other.m_nestedRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::~ChildNodeSpecification ()
    {
    CommonTools::FreePresentationRules(m_relatedInstances);
    CommonTools::FreePresentationRules(m_nestedRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_alwaysReturnsChildren, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_ALWAYSRETURNSCHILDREN))
        m_alwaysReturnsChildren = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_hideNodesInHierarchy, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDENODESINHIERARCHY))
        m_hideNodesInHierarchy = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_hideIfNoChildren, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDEIFNOCHILDREN))
        m_hideIfNoChildren = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_extendedData, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_EXTENDEDDATA))
        m_extendedData = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_doNotSort, SORTING_RULE_XML_ATTRIBUTE_DONOTSORT))
        m_doNotSort = false;

    CommonTools::LoadSpecificationsFromXmlNode<RelatedInstanceSpecification, RelatedInstanceSpecificationList> (xmlNode, m_relatedInstances, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<ChildNodeRule, ChildNodeRuleList>(xmlNode, m_nestedRules, CHILD_NODE_RULE_XML_NODE_NAME);

    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::WriteXml (BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    specificationNode->AddAttributeInt32Value   (COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);
    specificationNode->AddAttributeBooleanValue (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_ALWAYSRETURNSCHILDREN, m_alwaysReturnsChildren);
    specificationNode->AddAttributeBooleanValue (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDENODESINHIERARCHY, m_hideNodesInHierarchy);
    specificationNode->AddAttributeBooleanValue (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDEIFNOCHILDREN, m_hideIfNoChildren);
    specificationNode->AddAttributeStringValue  (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_EXTENDEDDATA, m_extendedData.c_str ());
    specificationNode->AddAttributeBooleanValue (SORTING_RULE_XML_ATTRIBUTE_DONOTSORT, m_doNotSort);

    CommonTools::WriteRulesToXmlNode<RelatedInstanceSpecification, RelatedInstanceSpecificationList>(specificationNode, m_relatedInstances);
    CommonTools::WriteRulesToXmlNode<ChildNodeRule, ChildNodeRuleList>(specificationNode, m_nestedRules);

    //Make sure we call protected override
    _WriteXml (specificationNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ChildNodeSpecification::GetPriority (void) const
    {
    return m_priority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetPriority (int value) { m_priority = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetAlwaysReturnsChildren (void) const
    {
    return m_alwaysReturnsChildren;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetAlwaysReturnsChildren (bool value) { m_alwaysReturnsChildren = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetHideNodesInHierarchy (void) const
    {
    return m_hideNodesInHierarchy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetHideNodesInHierarchy (bool value) { m_hideNodesInHierarchy = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetHideIfNoChildren (void) const
    {
    return m_hideIfNoChildren;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetHideIfNoChildren (bool value) { m_hideIfNoChildren = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ChildNodeSpecification::GetExtendedData (void) const
    {
    return m_extendedData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetExtendedData (Utf8StringCR extendedData)
    {
    m_extendedData = extendedData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetDoNotSort (void) const
    {
    return m_doNotSort;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetDoNotSort (bool doNotSort)
    {
    m_doNotSort = doNotSort;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRuleList const& ChildNodeSpecification::GetNestedRules (void)
    {
    return m_nestedRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::AddNestedRule(ChildNodeRuleR rule)
    {
    InvalidateHash();
    rule.SetParent(this);
    m_nestedRules.push_back(&rule);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance)
    {
    InvalidateHash();
    relatedInstance.SetParent(this);
    m_relatedInstances.push_back(&relatedInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ChildNodeSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = PresentationRuleSpecification::_ComputeHash(parentHash);
    md5.Add(&m_priority, sizeof(m_priority));
    md5.Add(&m_alwaysReturnsChildren, sizeof(m_alwaysReturnsChildren));
    md5.Add(&m_hideNodesInHierarchy, sizeof(m_hideNodesInHierarchy));
    md5.Add(&m_hideIfNoChildren, sizeof(m_hideIfNoChildren));
    md5.Add(&m_doNotSort, sizeof(m_doNotSort));
    md5.Add(m_extendedData.c_str(), m_extendedData.size());
    CharCP name = _GetXmlElementName();
    md5.Add(name, strlen(name));

    Utf8String currentHash = md5.GetHashString();
    for (RelatedInstanceSpecificationP spec : m_relatedInstances)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    for (ChildNodeRuleP rule : m_nestedRules)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    return md5;
    }
