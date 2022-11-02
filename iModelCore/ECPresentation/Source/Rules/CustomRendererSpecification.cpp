/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/CustomRendererSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 CustomRendererSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_rendererName.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_RENDERERS_SPECIFICATION_JSON_ATTRIBUTE_RENDERERNAME, m_rendererName);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomRendererSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    CustomRendererSpecification const* otherRule = dynamic_cast<CustomRendererSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_rendererName == otherRule->m_rendererName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CustomRendererSpecification::_GetJsonElementType() const {return "CustomRendererSpecification";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomRendererSpecification::_ReadJson(JsonValueCR json)
    {
    if (!T_Super::_ReadJson(json))
        return false;

    m_rendererName = json[PROPERTY_RENDERERS_SPECIFICATION_JSON_ATTRIBUTE_RENDERERNAME].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_rendererName.empty(), _GetJsonElementType(), PROPERTY_RENDERERS_SPECIFICATION_JSON_ATTRIBUTE_RENDERERNAME, json[PROPERTY_RENDERERS_SPECIFICATION_JSON_ATTRIBUTE_RENDERERNAME], "non-empty string"))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomRendererSpecification::_WriteJson(JsonValueR json) const
    {
    T_Super::_WriteJson(json);
    json[PROPERTY_RENDERERS_SPECIFICATION_JSON_ATTRIBUTE_RENDERERNAME] = m_rendererName;
    }
