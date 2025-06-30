/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverride::ImageIdOverride() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverride::ImageIdOverride (Utf8StringCR condition, int priority, Utf8StringCR imageIdExpression)
    : ConditionalCustomizationRule (condition, priority, false), m_imageIdExpression (imageIdExpression)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ImageIdOverride::_GetJsonElementType() const
    {
    return IMAGE_ID_OVERRIDE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageIdOverride::_ReadJson(BeJsConst json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    m_imageIdExpression = json[IMAGE_ID_OVERRIDE_JSON_ATTRIBUTE_IMAGEID].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageIdOverride::_WriteJson(BeJsValue json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    if (!m_imageIdExpression.empty())
        json[IMAGE_ID_OVERRIDE_JSON_ATTRIBUTE_IMAGEID] = m_imageIdExpression;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ImageIdOverride::GetImageId (void) const { return m_imageIdExpression; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageIdOverride::SetImageId (Utf8String value) { m_imageIdExpression = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageIdOverride::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ImageIdOverride::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_imageIdExpression.empty())
        ADD_STR_VALUE_TO_HASH(md5, IMAGE_ID_OVERRIDE_JSON_ATTRIBUTE_IMAGEID, m_imageIdExpression);
    return md5;
    }
