/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/InstanceLabelOverride.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP InstanceLabelOverride::_GetXmlElementName () const
    {
    return INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverride::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_className, INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME))
        m_className = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyNames, INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES))
        m_propertyNames = "";

    return CustomizationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME, m_className.c_str ());
    xmlNode->AddAttributeStringValue (INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES, m_propertyNames.c_str ());

    CustomizationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverride::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = CustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_className.c_str(), m_className.size());
    md5.Add(m_propertyNames.c_str(), m_propertyNames.size());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR InstanceLabelOverride::GetClassName() const { return m_className; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR InstanceLabelOverride::GetPropertyNames() const { return m_propertyNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }