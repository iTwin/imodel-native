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
InstanceLabelOverride::InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, Utf8StringCR propertyNames)
    : CustomizationRule(priority, onlyIfNotHandled), m_className(className)
    {
    m_properties = CommonTools::ParsePropertiesNames(propertyNames);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP InstanceLabelOverride::_GetXmlElementName() const
    {
    return INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverride::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME))
        m_className = "";

    Utf8String propertyNames;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(propertyNames, INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES))
        propertyNames = "";
    m_properties = CommonTools::ParsePropertiesNames(propertyNames);
    return CustomizationRule::_ReadXml(xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue(INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());
    xmlNode->AddAttributeStringValue(INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES, BeStringUtilities::Join(m_properties, ",").c_str());

    CustomizationRule::_WriteXml(xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverride::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = CustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_className.c_str(), m_className.size());

    Utf8String currentHash = md5.GetHashString();
    for (Utf8StringCR propertyName : m_properties)
        md5.Add(propertyName.c_str(), propertyName.size());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR InstanceLabelOverride::GetClassName() const {return m_className;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> const& InstanceLabelOverride::GetPropertyNames() const {return m_properties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_Accept(CustomizationRuleVisitor& visitor) const {visitor._Visit(*this);}
