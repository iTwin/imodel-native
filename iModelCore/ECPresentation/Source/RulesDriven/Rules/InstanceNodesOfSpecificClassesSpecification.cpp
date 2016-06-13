/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/InstanceNodesOfSpecificClassesSpecification.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>
#include <ECPresentationRules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_EC

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
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification
(
int       priority,
bool      alwaysReturnsChildren,
bool      hideNodesInHierarchy,
bool      hideIfNoChildren,
bool      groupByClass,
bool      groupByLabel,
bool      showEmptyGroups,
Utf8StringCR instanceFilter,
Utf8StringCR classNames,
bool      arePolymorphic
) : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy, hideIfNoChildren), 
    m_groupByClass (groupByClass), m_groupByLabel (groupByLabel), m_showEmptyGroups (showEmptyGroups), 
    m_instanceFilter (instanceFilter), m_classNames (classNames), m_arePolymorphic (arePolymorphic)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP InstanceNodesOfSpecificClassesSpecification::_GetXmlElementName () const
    {
    return INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_classNames, COMMON_XML_ATTRIBUTE_CLASSNAMES))
        {
        LOG.errorv ("Invalid InstanceNodesOfSpecificClassesSpecificationXML: %s element must contain a %s attribute", INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_CLASSNAMES);
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
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_CLASSNAMES, m_classNames.c_str ());
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC, m_arePolymorphic);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS, m_showEmptyGroups);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
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
