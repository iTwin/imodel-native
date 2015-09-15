/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/SelectedNodeInstancesSpecification.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SelectedNodeInstancesSpecification::SelectedNodeInstancesSpecification () :
ContentSpecification (),
m_onlyIfNotHandled (false),
m_acceptableSchemaName (L""),
m_acceptableClassNames (L""),
m_acceptablePolymorphically (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SelectedNodeInstancesSpecification::SelectedNodeInstancesSpecification
(
int priority,
bool onlyIfNotHandled,
WStringCR acceptableSchemaName,
WStringCR acceptableClassNames,
bool acceptablePolymorphically
) :
ContentSpecification (priority),
m_onlyIfNotHandled (onlyIfNotHandled),
m_acceptableSchemaName (acceptableSchemaName),
m_acceptableClassNames (acceptableClassNames),
m_acceptablePolymorphically (acceptablePolymorphically)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP SelectedNodeInstancesSpecification::_GetXmlElementName ()
    {
    return SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_onlyIfNotHandled, COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED))
        m_onlyIfNotHandled = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_acceptableSchemaName, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME))
        m_acceptableSchemaName = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_acceptableClassNames, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES))
        m_acceptableClassNames = L"";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_acceptablePolymorphically, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY))
        m_acceptablePolymorphically = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);
    xmlNode->AddAttributeStringValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME, m_acceptableSchemaName.c_str ());
    xmlNode->AddAttributeStringValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES, m_acceptableClassNames.c_str ());
    xmlNode->AddAttributeBooleanValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY, m_acceptablePolymorphically);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::GetOnlyIfNotHandled (void) const
    {
    return m_onlyIfNotHandled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR SelectedNodeInstancesSpecification::GetAcceptableSchemaName (void) const
    {
    return m_acceptableSchemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR SelectedNodeInstancesSpecification::GetAcceptableClassNames (void) const
    {
    return m_acceptableClassNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::GetAcceptablePolymorphically (void) const
    {
    return m_acceptablePolymorphically;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::SetOnlyIfNotHandled(bool value)
    {
    m_onlyIfNotHandled = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::SetAcceptableSchemaName(WStringCR value)
    {
    m_acceptableSchemaName = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::SetAcceptableClassNames(WStringCR value)
    {
    m_acceptableClassNames = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::SetAcceptablePolymorphically(bool value)
    {
    m_acceptablePolymorphically = value;
    }
