/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
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
Utf8CP SelectedNodeInstancesSpecification::_GetJsonElementType() const
    {
    return SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectedNodeInstancesSpecification::_ReadJson(BeJsConst json)
    {
    if (!ContentSpecification::_ReadJson(json))
        return false;

    m_acceptableSchemaName = json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLESCHEMANAME].asCString("");
    m_acceptablePolymorphically = json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY].asBool(false);

    if (json.isMember(SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES))
        {
        BeJsConst acceptableClassNamesJson = json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES];
        for (BeJsConst::ArrayIndex i = 0; i < acceptableClassNamesJson.size(); ++i)
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
void SelectedNodeInstancesSpecification::_WriteJson(BeJsValue json) const
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
        BeJsValue acceptableClassNames = json[SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES];
        for (Utf8StringCR name : names)
            {
            acceptableClassNames[acceptableClassNames.size()] = name;
            }
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
