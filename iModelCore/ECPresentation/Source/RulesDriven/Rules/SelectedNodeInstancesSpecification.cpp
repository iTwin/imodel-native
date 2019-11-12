/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SelectedNodeInstancesSpecification::SelectedNodeInstancesSpecification()
    : ContentSpecification(), m_onlyIfNotHandled(false), m_acceptablePolymorphically(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SelectedNodeInstancesSpecification::SelectedNodeInstancesSpecification
(
int priority,
bool onlyIfNotHandled,
Utf8StringCR acceptableSchemaName,
Utf8StringCR acceptableClassNames,
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
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SelectedNodeInstancesSpecification::_GetXmlElementName () const
    {
    return SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ContentSpecification::_ReadXml(xmlNode))
        return false;

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_onlyIfNotHandled, COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED))
        m_onlyIfNotHandled = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_acceptableSchemaName, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME))
        m_acceptableSchemaName = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_acceptableClassNames, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES))
        m_acceptableClassNames = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_acceptablePolymorphically, SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY))
        m_acceptablePolymorphically = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ContentSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);
    xmlNode->AddAttributeStringValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME, m_acceptableSchemaName.c_str ());
    xmlNode->AddAttributeStringValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES, m_acceptableClassNames.c_str ());
    xmlNode->AddAttributeBooleanValue (SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY, m_acceptablePolymorphically);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SelectedNodeInstancesSpecification::_GetJsonElementType() const
    {
    return SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ContentSpecification::_ReadJson(json))
        return false;

    m_onlyIfNotHandled = json[COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED].asBool(false);
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
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::_WriteJson(JsonValueR json) const
    {
    ContentSpecification::_WriteJson(json);
    if (m_onlyIfNotHandled)
        json[COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED] = m_onlyIfNotHandled;
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
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::GetOnlyIfNotHandled (void) const
    {
    return m_onlyIfNotHandled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SelectedNodeInstancesSpecification::GetAcceptableSchemaName (void) const
    {
    return m_acceptableSchemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SelectedNodeInstancesSpecification::GetAcceptableClassNames (void) const
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
void SelectedNodeInstancesSpecification::SetAcceptableSchemaName(Utf8StringCR value)
    {
    m_acceptableSchemaName = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectedNodeInstancesSpecification::SetAcceptableClassNames(Utf8StringCR value)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SelectedNodeInstancesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ContentSpecification::_ComputeHash(parentHash);
    md5.Add(m_acceptableClassNames.c_str(), m_acceptableClassNames.size());
    md5.Add(m_acceptableSchemaName.c_str(), m_acceptableSchemaName.size());
    md5.Add(&m_acceptablePolymorphically, sizeof(m_acceptablePolymorphically));
    md5.Add(&m_onlyIfNotHandled, sizeof(m_onlyIfNotHandled));
    return md5;
    }
