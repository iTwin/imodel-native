/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodeSpecification::CustomNodeSpecification()
    : ChildNodeSpecification()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodeSpecification::CustomNodeSpecification (int priority, bool hideIfNoChildren, Utf8StringCR type, Utf8StringCR label, Utf8StringCR description, Utf8StringCR imageId)
    : ChildNodeSpecification (priority, ChildrenHint::Always, false, hideIfNoChildren),
    m_type(type), m_label(label), m_description(description), m_imageId(imageId)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodeSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ChildNodeSpecification::_ShallowEqual(other))
        return false;

    CustomNodeSpecificationCP otherSpec = dynamic_cast<CustomNodeSpecificationCP>(&other);
    if (nullptr == otherSpec)
        return false;

    return m_type == otherSpec->m_type
        && m_label == otherSpec->m_label
        && m_imageId == otherSpec->m_imageId
        && m_description == otherSpec->m_description;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CustomNodeSpecification::_GetXmlElementName () const
    {
    return CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodeSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ChildNodeSpecification::_ReadXml(xmlNode))
        return false;

    // required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_type, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_TYPE))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_label, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_LABEL))
        return false;

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_description, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_DESCRIPTION))
        m_description = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_IMAGEID))
        m_imageId = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ChildNodeSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_TYPE, m_type.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_LABEL, m_label.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_DESCRIPTION, m_description.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_IMAGEID, m_imageId.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CustomNodeSpecification::_GetJsonElementType() const
    {
    return CUSTOM_NODE_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodeSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;

    // required:
    m_type = json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE].asCString("");
    m_label = json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL].asCString("");

    bool hasIssues = false
        || CommonToolsInternal::CheckRuleIssue(m_type.empty(), _GetJsonElementType(), CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE, json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE], "non-empty string")
        || CommonToolsInternal::CheckRuleIssue(m_label.empty(), _GetJsonElementType(), CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL, json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL], "non-empty string");
    if (hasIssues)
        return false;

    // optional:
    m_description = json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION].asCString("");
    m_imageId = json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_IMAGEID].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::_WriteJson(JsonValueR json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE] = m_type;
    json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL] = m_label;
    if (!m_description.empty())
        json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION] = m_description;
    if (!m_imageId.empty())
        json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_IMAGEID] = m_imageId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CustomNodeSpecification::GetNodeType (void) const { return m_type; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetNodeType(Utf8StringCR value) { m_type = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CustomNodeSpecification::GetLabel (void) const { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetLabel (Utf8StringCR value) { m_label = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CustomNodeSpecification::GetDescription (void) const { return m_description; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetDescription (Utf8StringCR value) { m_description = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CustomNodeSpecification::GetImageId (void) const { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetImageId (Utf8StringCR value) { m_imageId = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 CustomNodeSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_type.empty())
        ADD_STR_VALUE_TO_HASH(md5, CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE, m_type);
    if (!m_label.empty())
        ADD_STR_VALUE_TO_HASH(md5, CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL, m_label);
    if (!m_description.empty())
        ADD_STR_VALUE_TO_HASH(md5, CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION, m_description);
    if (!m_imageId.empty())
        ADD_STR_VALUE_TO_HASH(md5, CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_IMAGEID, m_imageId);
    return md5;
    }
