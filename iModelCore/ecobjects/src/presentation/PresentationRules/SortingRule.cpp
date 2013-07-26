/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/SortingRule.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
    : PresentationRule (), m_schemaName (L""), m_className (L""), m_propertyName (L""), m_sortAscending (true), m_doNotSort (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRule::SortingRule (WStringCR condition, int priority, WStringCR schemaName, WStringCR className, WStringCR propertyName, bool sortAscending, bool doNotSort)
    : PresentationRule (condition, priority, false), 
      m_schemaName (schemaName), m_className (className), m_propertyName (propertyName), m_sortAscending (sortAscending), m_doNotSort (doNotSort)
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
        m_schemaName = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_className,       COMMON_XML_ATTRIBUTE_CLASSNAME))
        m_className = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyName,    COMMON_XML_ATTRIBUTE_PROPERTYNAME))
        m_propertyName = L"";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_sortAscending,  SORTING_RULE_XML_ATTRIBUTE_SORTASCENDING))
        m_sortAscending = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_doNotSort,      SORTING_RULE_XML_ATTRIBUTE_DONOTSORT))
        m_doNotSort = false;

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


    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR SortingRule::GetSchemaName (void) const { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR SortingRule::GetClassName (void) const { return m_className; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR SortingRule::GetPropertyName (void) const { return m_propertyName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::GetSortAscending (void) const { return m_sortAscending; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::GetDoNotSort (void) const { return m_doNotSort; }
