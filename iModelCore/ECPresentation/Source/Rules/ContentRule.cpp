/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled)
    : ConditionalPresentationRule (condition, priority, onlyIfNotHandled)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::ContentRule(ContentRuleCR other)
    : ConditionalPresentationRule(other)
    {
    CommonToolsInternal::CloneRules(m_specifications, other.m_specifications, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRule::~ContentRule ()
    {
    CommonToolsInternal::FreePresentationRules(m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRule::_GetJsonElementType() const
    {
    return CONTENT_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRule::_ReadJson(BeJsConst json)
    {
    if (!ConditionalPresentationRule::_ReadJson(json))
        return false;

    if (json.isMember(CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS))
        CommonToolsInternal::LoadFromJsonByPriority(_GetJsonElementType(), CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS, json[CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications, ContentSpecification::Create, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::_WriteJson(BeJsValue json) const
    {
    ConditionalPresentationRule::_WriteJson(json);
    CommonToolsInternal::WriteRulesToJson<ContentSpecification, ContentSpecificationList>(json[CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationList const& ContentRule::GetSpecifications (void) const { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRule::AddSpecification(ContentSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    CommonTools::AddToListByPriority(m_specifications, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    ADD_RULES_TO_HASH(md5, CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS, m_specifications);
    return md5;
    }
