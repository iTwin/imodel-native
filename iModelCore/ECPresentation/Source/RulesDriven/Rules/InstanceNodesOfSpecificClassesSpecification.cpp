/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification ()
    : ChildNodeSpecification (), m_groupByClass (true), m_groupByLabel (true), m_showEmptyGroups (false),
    m_instanceFilter (""), m_classNames (""), m_arePolymorphic (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, bool showEmptyGroups, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic)
    : InstanceNodesOfSpecificClassesSpecification(priority, alwaysReturnsChildren ? ChildrenHint::Always : ChildrenHint::Unknown, hideNodesInHierarchy,
        hideIfNoChildren, groupByClass, groupByLabel, instanceFilter, classNames, arePolymorphic)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic)
    : ChildNodeSpecification (priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren),
    m_groupByClass(groupByClass), m_groupByLabel(groupByLabel), m_showEmptyGroups(false),
    m_instanceFilter(instanceFilter), m_classNames(classNames), m_arePolymorphic(arePolymorphic)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::_ShallowEqual(PresentationRuleSpecification const& other) const
    {
    if (!ChildNodeSpecification::_ShallowEqual(other))
        return false;

    InstanceNodesOfSpecificClassesSpecificationCP otherRule = dynamic_cast<InstanceNodesOfSpecificClassesSpecificationCP>(&other);
    if (nullptr == otherRule)
        return false;

    return m_arePolymorphic == otherRule->m_arePolymorphic
        && m_groupByClass == otherRule->m_groupByClass
        && m_groupByLabel == otherRule->m_groupByLabel
        && m_showEmptyGroups == otherRule->m_showEmptyGroups
        && m_classNames == otherRule->m_classNames
        && m_instanceFilter == otherRule->m_instanceFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceNodesOfSpecificClassesSpecification::_GetXmlElementName () const
    {
    return INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ChildNodeSpecification::_ReadXml(xmlNode))
        return false;

    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_classNames, COMMON_XML_ATTRIBUTE_CLASSNAMES))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_CLASSNAMES);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_arePolymorphic, COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC))
        m_arePolymorphic = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByClass, COMMON_XML_ATTRIBUTE_GROUPBYCLASS))
        m_groupByClass = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByLabel, COMMON_XML_ATTRIBUTE_GROUPBYLABEL))
        m_groupByLabel = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_showEmptyGroups, COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS))
        m_showEmptyGroups = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ChildNodeSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_CLASSNAMES, m_classNames.c_str ());
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC, m_arePolymorphic);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS, m_showEmptyGroups);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceNodesOfSpecificClassesSpecification::_GetJsonElementType() const
    {
    return INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;

    //Required:
    m_classNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_CLASSES]);
    if (m_classNames.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceNodesOfSpecificClassesSpecification", COMMON_JSON_ATTRIBUTE_CLASSES);
        return false;
        }

    //Optional:
    m_arePolymorphic = json[COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC].asBool(false);
    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::_WriteJson(JsonValueR json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    if (!m_classNames.empty())
        json[COMMON_JSON_ATTRIBUTE_CLASSES] = CommonToolsInternal::SchemaAndClassNamesToJson(m_classNames);
    if (m_arePolymorphic)
        json[COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC] = m_arePolymorphic;
    if (!m_groupByClass)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS] = m_groupByClass;
    if (!m_groupByLabel)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL] = m_groupByLabel;
    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::GetGroupByClass (void) const { return m_groupByClass; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetGroupByClass (bool value) { m_groupByClass = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::GetGroupByLabel (void) const { return m_groupByLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetGroupByLabel (bool value) { m_groupByLabel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::GetShowEmptyGroups (void) const { return m_showEmptyGroups; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetShowEmptyGroups (bool value) { m_showEmptyGroups = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR InstanceNodesOfSpecificClassesSpecification::GetClassNames (void) const { return m_classNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetClassNames (Utf8String value) { m_classNames = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::GetArePolymorphic (void) const { return m_arePolymorphic; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetArePolymorphic (bool value) { m_arePolymorphic = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR InstanceNodesOfSpecificClassesSpecification::GetInstanceFilter (void) const { return m_instanceFilter; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceNodesOfSpecificClassesSpecification::_ComputeHash() const
    {
    MD5 md5 = ChildNodeSpecification::_ComputeHash();
    md5.Add(&m_groupByClass, sizeof(m_groupByClass));
    md5.Add(&m_groupByLabel, sizeof(m_groupByLabel));
    md5.Add(&m_showEmptyGroups, sizeof(m_showEmptyGroups));
    md5.Add(&m_arePolymorphic, sizeof(m_arePolymorphic));
    md5.Add(m_instanceFilter.c_str(), m_instanceFilter.size());
    md5.Add(m_classNames.c_str(), m_classNames.size());
    return md5;
    }
