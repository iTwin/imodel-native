/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverridePropertyNamesJoiner : InstanceLabelOverrideValueSpecificationVisitor
    {
    private:
        Utf8String m_joinedNames;
    protected:
        void _Visit(InstanceLabelOverridePropertyValueSpecification const& spec) override
            {
            if (!m_joinedNames.empty())
                m_joinedNames.append(",");
            m_joinedNames.append(spec.GetPropertyName());
            }
    public:
        Utf8StringCR GetResult() const { return m_joinedNames; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, Utf8StringCR propertyNamesStr)
    : CustomizationRule(priority, onlyIfNotHandled), m_className(className)
    {
    bvector<Utf8String> propertyNames = CommonToolsInternal::ParsePropertiesNames(propertyNamesStr);
    for (Utf8StringCR propertyName : propertyNames)
        AddValueSpecification(*new InstanceLabelOverridePropertyValueSpecification(propertyName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::InstanceLabelOverride(InstanceLabelOverride const& other)
    : m_className(other.m_className)
    {
    CommonToolsInternal::CloneRules(m_valueSpecifications, other.m_valueSpecifications, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::InstanceLabelOverride(InstanceLabelOverride&& other)
    {
    m_className.swap(other.m_className);
    m_valueSpecifications.swap(other.m_valueSpecifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::~InstanceLabelOverride()
    {
    CommonToolsInternal::FreePresentationRules(m_valueSpecifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverride::_GetXmlElementName() const { return INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverride::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!CustomizationRule::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME))
        return false;

    Utf8String propertyNamesStr;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(propertyNamesStr, INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES))
        return false;

    bvector<Utf8String> propertyNames = CommonToolsInternal::ParsePropertiesNames(propertyNamesStr);
    for (Utf8StringCR propertyName : propertyNames)
        AddValueSpecification(*new InstanceLabelOverridePropertyValueSpecification(propertyName));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_WriteXml(BeXmlNodeP xmlNode) const
    {
    CustomizationRule::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());

    InstanceLabelOverridePropertyNamesJoiner joiner;
    for (InstanceLabelOverrideValueSpecification const* spec : m_valueSpecifications)
        spec->Accept(joiner);
    xmlNode->AddAttributeStringValue(INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES, joiner.GetResult().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverride::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_JSON_TYPE; }

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

    CommonToolsInternal::LoadFromJson(json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_VALUES], m_valueSpecifications, InstanceLabelOverrideValueSpecification::Create, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_WriteJson(JsonValueR json) const
    {
    CustomizationRule::_WriteJson(json);
    json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_className);

    Json::Value specificationsJson(Json::arrayValue);
    CommonToolsInternal::WriteRulesToJson<InstanceLabelOverrideValueSpecification, bvector<InstanceLabelOverrideValueSpecification*>>(specificationsJson, m_valueSpecifications);
    json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_VALUES] = specificationsJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverride::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = CustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_className.c_str(), m_className.size());

    Utf8String currentHash = md5.GetHashString();
    for (InstanceLabelOverrideValueSpecification* spec : m_valueSpecifications)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_Accept(CustomizationRuleVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::AddValueSpecification(InstanceLabelOverrideValueSpecification& spec)
    {
    InvalidateHash();
    spec.SetParent(this);
    m_valueSpecifications.push_back(&spec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideValueSpecification* InstanceLabelOverrideValueSpecification::Create(JsonValueCR json)
    {
    Utf8CP type = json[INSTANCE_LABEL_OVERRIDE_VALUE_SPECIFICATION_BASE_JSON_ATTRIBUTE_TYPE].asCString("");
    InstanceLabelOverrideValueSpecification* spec = nullptr;
    if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceLabelOverrideCompositeValueSpecification();
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceLabelOverridePropertyValueSpecification();
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceLabelOverrideClassNameValueSpecification();
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_CLASSLABEL_VALUE_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceLabelOverrideClassLabelValueSpecification();
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_BRIEFCASEID_VALUE_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceLabelOverrideBriefcaseIdValueSpecification();
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_LOCALID_VALUE_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceLabelOverrideLocalIdValueSpecification();
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceLabelOverrideStringValueSpecification();
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideValueSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    if (parentHash != nullptr)
        md5.Add(parentHash, strlen(parentHash));
    Utf8CP type = _GetJsonElementType();
    md5.Add(type, strlen(type));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value InstanceLabelOverrideValueSpecification::WriteJson() const
    {
    Json::Value json;
    _WriteJson(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideValueSpecification::_WriteJson(JsonValueR json) const
    {
    json[INSTANCE_LABEL_OVERRIDE_VALUE_SPECIFICATION_BASE_JSON_ATTRIBUTE_TYPE] = _GetJsonElementType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideCompositeValueSpecification::InstanceLabelOverrideCompositeValueSpecification(InstanceLabelOverrideCompositeValueSpecification const& other)
    : m_separator(other.m_separator)
    {
    CommonToolsInternal::CopyRules<Part>(m_parts, other.m_parts, this);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideCompositeValueSpecification::InstanceLabelOverrideCompositeValueSpecification(InstanceLabelOverrideCompositeValueSpecification&& other)
    : m_separator(other.m_separator)
    {
    m_parts.swap(other.m_parts);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideCompositeValueSpecification::~InstanceLabelOverrideCompositeValueSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_parts);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideCompositeValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideCompositeValueSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = InstanceLabelOverrideValueSpecification::_ComputeHash(parentHash);
    md5.Add(m_separator.c_str(), m_separator.size());
    Utf8String currentHash = md5.GetHashString();
    for (Part* part : m_parts)
        {
        Utf8StringCR partHash = part->GetHash(currentHash.c_str());
        md5.Add(partHash.c_str(), partHash.size());
        }
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static InstanceLabelOverrideCompositeValueSpecification::Part* PartsFactory(JsonValueCR partJson) { return CommonToolsInternal::LoadRuleFromJson<InstanceLabelOverrideCompositeValueSpecification::Part>(partJson); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideCompositeValueSpecification::_ReadJson(JsonValueCR json)
    {
    if (!InstanceLabelOverrideValueSpecification::_ReadJson(json))
        return false;

    if (json.isMember(INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR) && json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR].isString())
        m_separator = json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR].asCString();

    CommonToolsInternal::LoadFromJson(json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PARTS], m_parts, &PartsFactory, this);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideCompositeValueSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    if (!m_separator.Equals(" "))
        json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR] = m_separator;
    CommonToolsInternal::WriteRulesToJson<Part, bvector<Part*>>(json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PARTS], m_parts);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideCompositeValueSpecification::AddValuePart(InstanceLabelOverrideValueSpecification& partSpecification, bool isRequired)
    {
    InvalidateHash();
    Part* part = new Part(partSpecification, isRequired);
    part->SetParent(this);
    m_parts.push_back(part);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideCompositeValueSpecification::Part::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    if (parentHash != nullptr)
        md5.Add(parentHash, strlen(parentHash));
    md5.Add(&m_isRequired, sizeof(m_isRequired));

    Utf8String currentHash = md5.GetHashString();
    Utf8StringCR specHash = m_specification->GetHash(currentHash.c_str());
    md5.Add(specHash.c_str(), specHash.size());

    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideCompositeValueSpecification::Part::ReadJson(JsonValueCR json)
    {
    if (!json.isMember(INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverrideCompositeValueSpecification.Part", INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC);
        return false;
        }

    m_specification = InstanceLabelOverrideValueSpecification::Create(json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC]);
    if (!m_specification)
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverrideCompositeValueSpecification.Part", INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC);
        return false;
        }

    if (json.isMember(INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED) && json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED].isBool())
        m_isRequired = json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED].asBool();

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value InstanceLabelOverrideCompositeValueSpecification::Part::WriteJson() const
    {
    Json::Value json;
    if (m_specification)
        json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC] = m_specification->WriteJson();
    if (m_isRequired)
        json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED] = m_isRequired;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverridePropertyValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverridePropertyValueSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = InstanceLabelOverrideValueSpecification::_ComputeHash(parentHash);
    md5.Add(m_propertyName.c_str(), m_propertyName.size());
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverridePropertyValueSpecification::_ReadJson(JsonValueCR json)
    {
    if (!InstanceLabelOverrideValueSpecification::_ReadJson(json))
        return false;

    if (!json.isMember(INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverridePropertyValueSpecification", INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME);
        return false;
        }

    if (!json[INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME].isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverridePropertyValueSpecification", INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME);
        return false;
        }

    m_propertyName = Utf8String(json[INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME].asCString()).Trim();
    if (m_propertyName.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverridePropertyValueSpecification", INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME);
        return false;
        }

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverridePropertyValueSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    json[INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME] = m_propertyName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideClassNameValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideClassNameValueSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = InstanceLabelOverrideValueSpecification::_ComputeHash(parentHash);
    md5.Add(&m_full, sizeof(m_full));
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideClassNameValueSpecification::_ReadJson(JsonValueCR json)
    {
    if (!InstanceLabelOverrideValueSpecification::_ReadJson(json))
        return false;

    if (json.isMember(INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_ATTRIBUTE_FULL) && json[INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_ATTRIBUTE_FULL].isBool())
        m_full = json[INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_ATTRIBUTE_FULL].asBool();

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideClassNameValueSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    if (m_full)
        json[INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_ATTRIBUTE_FULL] = m_full;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideClassLabelValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_CLASSLABEL_VALUE_SPECIFICATION_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideBriefcaseIdValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_BRIEFCASEID_VALUE_SPECIFICATION_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideLocalIdValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_LOCALID_VALUE_SPECIFICATION_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideStringValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideStringValueSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = InstanceLabelOverrideValueSpecification::_ComputeHash(parentHash);
    md5.Add(m_value.c_str(), m_value.size());
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideStringValueSpecification::_ReadJson(JsonValueCR json)
    {
    if (!InstanceLabelOverrideValueSpecification::_ReadJson(json))
        return false;

    if (!json.isMember(INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverrideStringValueSpecification", INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE);
        return false;
        }

    if (!json[INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE].isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "InstanceLabelOverrideStringValueSpecification", INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE);
        return false;
        }

    m_value = json[INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE].asCString();
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideStringValueSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    json[INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE] = m_value;
    }
