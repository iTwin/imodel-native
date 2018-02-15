/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/SortingRule.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRule::SortingRule ()
    : m_schemaName (""), m_className (""), m_propertyName (""), m_sortAscending (true), m_doNotSort (false), m_isPolymorphic (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRule::SortingRule (Utf8StringCR condition, int priority, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR propertyName, bool sortAscending, bool doNotSort, bool isPolymorphic)
    : ConditionalCustomizationRule(condition, priority, false),
      m_schemaName (schemaName), m_className (className), m_propertyName (propertyName), m_sortAscending (sortAscending), m_doNotSort (doNotSort), m_isPolymorphic (isPolymorphic)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP SortingRule::_GetXmlElementName () const
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

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_isPolymorphic,  COMMON_XML_ATTRIBUTE_ISPOLYMORPHIC))
        m_isPolymorphic = false;

    return ConditionalCustomizationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SortingRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_SCHEMANAME,            m_schemaName.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_CLASSNAME,             m_className.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_PROPERTYNAME,          m_propertyName.c_str ());
    xmlNode->AddAttributeBooleanValue (SORTING_RULE_XML_ATTRIBUTE_SORTASCENDING,  m_sortAscending);
    xmlNode->AddAttributeBooleanValue (SORTING_RULE_XML_ATTRIBUTE_DONOTSORT,      m_doNotSort);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_ISPOLYMORPHIC,  m_isPolymorphic);

    ConditionalCustomizationRule::_WriteXml (xmlNode);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SortingRule::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SortingRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ConditionalCustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_schemaName.c_str(), m_schemaName.size());
    md5.Add(m_className.c_str(), m_className.size());
    md5.Add(m_propertyName.c_str(), m_propertyName.size());
    md5.Add(&m_sortAscending, sizeof(m_sortAscending));
    md5.Add(&m_doNotSort, sizeof(m_doNotSort));
    md5.Add(&m_isPolymorphic, sizeof(m_isPolymorphic));
    return md5;
    }
