/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/ImageIdOverride.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverride::ImageIdOverride ()
    : m_imageIdExpression ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverride::ImageIdOverride (Utf8StringCR condition, int priority, Utf8StringCR imageIdExpression)
    : CustomizationRule (condition, priority, false), m_imageIdExpression (imageIdExpression)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ImageIdOverride::_GetXmlElementName () const
    {
    return IMAGE_ID_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageIdOverride::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageIdExpression, IMAGE_ID_OVERRIDE_XML_ATTRIBUTE_IMAGEID))
        m_imageIdExpression = "";

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageIdOverride::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (IMAGE_ID_OVERRIDE_XML_ATTRIBUTE_IMAGEID, m_imageIdExpression.c_str ());

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ImageIdOverride::GetImageId (void) const { return m_imageIdExpression; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageIdOverride::SetImageId (Utf8String value) { m_imageIdExpression = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageIdOverride::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ImageIdOverride::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = CustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_imageIdExpression.c_str(), m_imageIdExpression.size());
    return md5;
    }
