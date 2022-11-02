/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SelectedNodeInstancesSpecification::SelectedNodeInstancesSpecification()
    : ContentSpecification(), m_acceptablePolymorphically(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SelectedNodeInstancesSpecification::SelectedNodeInstancesSpecification
(
int priority,
bool onlyIfNotHandled,
Utf8StringCR acceptableSchemaName,
Utf8StringCR acceptableClassNames,
bool acceptablePolymorphically
) :
ContentSpecification (priority, false, onlyIfNotHandled),
m_acceptableSchemaName (acceptableSchemaName),
m_acceptableClassNames (acceptableClassNames),
m_acceptablePolymorphically (acceptablePolymorphically)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SelectedNodeInstancesSpecification::_GetXmlElementName () const
    {
    return SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ContentSpecification::_ReadXml(xmlNode))
        return false;

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_acceptableSchemaName, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME))
        m_acceptableSchemaName = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_acceptableClassNames, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES))
        m_acceptableClassNames = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_acceptablePolymorphically, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY))
        m_acceptablePolymorphically = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ContentSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME, m_acceptableSchemaName.c_str ());
    xmlNode->AddAttributeStringValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES, m_acceptableClassNames.c_str ());
    xmlNode->AddAttributeBooleanValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY, m_acceptablePolymorphically);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SelectedNodeInstancesSpecification::_GetJsonElementType() const
    {
    return SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ContentSpecification::_ReadJson(json))
        return false;

    m_acceptableSchemaName = json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLESCHEMANAME].asCString("");
    m_acceptablePolymorphically = json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY].asBool(false);

    if (json.isMember(SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES))
        {
        JsonValueCR acceptableClassNamesJson = json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES];
        for (Json::ArrayIndex i = 0; i < acceptableClassNamesJson.size(); ++i)
            {
            if (!m_acceptableClassNames.empty())
                m_acceptableClassNames.append(",");
            m_acceptableClassNames.append(acceptableClassNamesJson[i].asCString());
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::_WriteJson(JsonValueR json) const
    {
    ContentSpecification::_WriteJson(json);
    if (!m_acceptableSchemaName.empty())
        json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLESCHEMANAME] = m_acceptableSchemaName;
    if (m_acceptablePolymorphically)
        json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY] = m_acceptablePolymorphically;
    if (!m_acceptableClassNames.empty())
        {
        bvector<Utf8String> names;
        BeStringUtilities::Split(m_acceptableClassNames.c_str(), ",", names);
        for (Utf8StringCR name : names)
            json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES].append(name);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SelectedNodeInstancesSpecification::GetAcceptableSchemaName (void) const
    {
    return m_acceptableSchemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SelectedNodeInstancesSpecification::GetAcceptableClassNames (void) const
    {
    return m_acceptableClassNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::GetAcceptablePolymorphically (void) const
    {
    return m_acceptablePolymorphically;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::SetAcceptableSchemaName(Utf8StringCR value)
    {
    InvalidateHash();
    m_acceptableSchemaName = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::SetAcceptableClassNames(Utf8StringCR value)
    {
    InvalidateHash();
    m_acceptableClassNames = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::SetAcceptablePolymorphically(bool value)
    {
    InvalidateHash();
    m_acceptablePolymorphically = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SelectedNodeInstancesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_acceptableClassNames.empty())
        ADD_STR_VALUE_TO_HASH(md5, SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES, m_acceptableClassNames);
    if (!m_acceptableSchemaName.empty())
        ADD_STR_VALUE_TO_HASH(md5, SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLESCHEMANAME, m_acceptableSchemaName);
    if (m_acceptablePolymorphically)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY, m_acceptablePolymorphically);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ContentSpecification::_ShallowEqual(other))
        return false;

    SelectedNodeInstancesSpecification const* otherRule = dynamic_cast<SelectedNodeInstancesSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_acceptableSchemaName == otherRule->m_acceptableSchemaName
        && m_acceptableClassNames == otherRule->m_acceptableClassNames
        && m_acceptablePolymorphically == otherRule->m_acceptablePolymorphically;
    }
