/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/SortingRule.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRule::SortingRule ()
    : PresentationRule (), m_schemaName (""), m_className (""), m_propertyName (""), m_sortAscending (true), m_doNotSort (false), m_isPolymorphic (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRule::SortingRule (Utf8StringCR condition, int priority, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR propertyName, bool sortAscending, bool doNotSort, bool isPolymorphic)
    : PresentationRule (condition, priority, false), 
      m_schemaName (schemaName), m_className (className), m_propertyName (propertyName), m_sortAscending (sortAscending), m_doNotSort (doNotSort), m_isPolymorphic (isPolymorphic)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP SortingRule::_GetXmlElementName ()
    {
    return SORTING_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_schemaName,      COMMON_XML_ATTRIBUTE_SCHEMANAME))
        m_schemaName = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_className,       COMMON_XML_ATTRIBUTE_CLASSNAME))
        m_className = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyName,    COMMON_XML_ATTRIBUTE_PROPERTYNAME))
        m_propertyName = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_sortAscending,  SORTING_RULE_XML_ATTRIBUTE_SORTASCENDING))
        m_sortAscending = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_doNotSort,      SORTING_RULE_XML_ATTRIBUTE_DONOTSORT))
        m_doNotSort = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_isPolymorphic,  SORTING_RULE_XML_ATTRIBUTE_ISPOLYMORPHIC))
        m_isPolymorphic = false;

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SortingRule::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_SCHEMANAME,            m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_CLASSNAME,             m_className.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_PROPERTYNAME,          m_propertyName.c_str ());
    xmlNode->AddAttributeBooleanValue (SORTING_RULE_XML_ATTRIBUTE_SORTASCENDING,  m_sortAscending);
    xmlNode->AddAttributeBooleanValue (SORTING_RULE_XML_ATTRIBUTE_DONOTSORT,      m_doNotSort);
    xmlNode->AddAttributeBooleanValue (SORTING_RULE_XML_ATTRIBUTE_ISPOLYMORPHIC,  m_isPolymorphic);

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SortingRule::GetSchemaName (void) const { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SortingRule::GetClassName (void) const { return m_className; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SortingRule::GetPropertyName (void) const { return m_propertyName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::GetSortAscending (void) const { return m_sortAscending; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::GetDoNotSort (void) const { return m_doNotSort; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::GetIsPolymorphic (void) const { return m_isPolymorphic; }
