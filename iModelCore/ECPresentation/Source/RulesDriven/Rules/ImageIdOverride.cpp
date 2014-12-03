/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ImageIdOverride.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverride::ImageIdOverride ()
    : PresentationRule (), m_imageIdExpression (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverride::ImageIdOverride (WStringCR condition, int priority, WStringCR imageIdExpression)
    : PresentationRule (condition, priority, false), m_imageIdExpression (imageIdExpression)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ImageIdOverride::_GetXmlElementName ()
    {
    return IMAGE_ID_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageIdOverride::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageIdExpression, IMAGE_ID_OVERRIDE_XML_ATTRIBUTE_IMAGEID))
        m_imageIdExpression = L"";

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageIdOverride::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (IMAGE_ID_OVERRIDE_XML_ATTRIBUTE_IMAGEID, m_imageIdExpression.c_str ());

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ImageIdOverride::GetImageId (void) const { return m_imageIdExpression; }
