/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, Utf8StringCR propertyNames)
    : CustomizationRule(priority, onlyIfNotHandled), m_className(className)
    {
    m_properties = CommonToolsInternal::ParsePropertiesNames(propertyNames);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverride::_GetXmlElementName() const
    {
    return INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverride::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!CustomizationRule::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME))
        return false;

    Utf8String propertyNames;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(propertyNames, INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES))
        return false;

    m_properties = CommonToolsInternal::ParsePropertiesNames(propertyNames);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_WriteXml(BeXmlNodeP xmlNode) const
    {
    CustomizationRule::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());
    xmlNode->AddAttributeStringValue(INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES, BeStringUtilities::Join(m_properties, ",").c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverride::_GetJsonElementType() const
    {
    return INSTANCE_LABEL_OVERRIDE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverride::_ReadJson(JsonValueCR json)
    {
    if (!CustomizationRule::_ReadJson(json))
        return false;

    m_className = CommonToolsInternal::SchemaAndClassNameToString(json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS]);
    if (m_className.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverride", INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS);
        return false;
        }

    JsonValueCR propertyNames = json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_PROPERTYNAMES];
    for (Json::ArrayIndex i = 0; i < propertyNames.size(); i++)
        {
        Utf8CP prop = propertyNames[i].asCString(nullptr);
        if (prop != nullptr)
            m_properties.push_back(Utf8String(prop).Trim());
        }
    if (m_properties.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverride", INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_PROPERTYNAMES);
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_WriteJson(JsonValueR json) const
    {
    CustomizationRule::_WriteJson(json);
    json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_className);
    for (size_t i = 0; i < m_properties.size(); i++)
        json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_PROPERTYNAMES].append(m_properties[i]);
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
