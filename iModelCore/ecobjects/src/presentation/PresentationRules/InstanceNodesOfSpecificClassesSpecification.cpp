/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/InstanceNodesOfSpecificClassesSpecification.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP InstanceNodesOfSpecificClassesSpecification::_GetXmlElementName ()
    {
    return INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_schemaName, COMMON_XML_ATTRIBUTE_SCHEMANAME))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid InstanceNodesOfSpecificClassesSpecificationXML: %hs element must contain a %hs attribute", INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_SCHEMANAME);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_classNames, COMMON_XML_ATTRIBUTE_CLASSNAMES))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid InstanceNodesOfSpecificClassesSpecificationXML: %hs element must contain a %hs attribute", INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_CLASSNAMES);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByClass, COMMON_XML_ATTRIBUTE_GROUPBYCLASS))
        m_groupByClass = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByLabel, COMMON_XML_ATTRIBUTE_GROUPBYLABEL))
        m_groupByLabel = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_showEmptyGroups, COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS))
        m_showEmptyGroups = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = L"";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SCHEMANAME, m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_CLASSNAMES, m_classNames.c_str ());
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS, m_showEmptyGroups);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    }