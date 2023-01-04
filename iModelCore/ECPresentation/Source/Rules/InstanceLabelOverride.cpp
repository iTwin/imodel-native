/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRule.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsiclass
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, Utf8StringCR propertyNamesStr)
    : CustomizationRule(priority, onlyIfNotHandled), m_className(className)
    {
    bvector<Utf8String> propertyNames = CommonToolsInternal::ParsePropertiesNames(propertyNamesStr);
    for (Utf8StringCR propertyName : propertyNames)
        AddValueSpecification(*new InstanceLabelOverridePropertyValueSpecification(propertyName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::InstanceLabelOverride(InstanceLabelOverride const& other)
    : CustomizationRule(other), m_className(other.m_className)
    {
    CommonToolsInternal::CloneRules(m_valueSpecifications, other.m_valueSpecifications, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::InstanceLabelOverride(InstanceLabelOverride&& other)
    : CustomizationRule(std::move(other))
    {
    m_className.swap(other.m_className);
    m_valueSpecifications.swap(other.m_valueSpecifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverride::~InstanceLabelOverride()
    {
    CommonToolsInternal::FreePresentationRules(m_valueSpecifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverride::_GetXmlElementName() const { return INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverride::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverride::_ReadJson(JsonValueCR json)
    {
    if (!CustomizationRule::_ReadJson(json))
        return false;

    // required:
    m_className = CommonToolsInternal::SchemaAndClassNameToString(json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS).c_str());
    if (CommonToolsInternal::CheckRuleIssue(m_className.empty(), _GetJsonElementType(), INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS, json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS], "valid class object"))
        return false;

    // optional:
    if (json.isMember(INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_VALUES))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_VALUES, json[INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_VALUES], m_valueSpecifications, InstanceLabelOverrideValueSpecification::Create, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverride::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_className.empty())
        ADD_STR_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS, m_className);
    ADD_RULES_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_VALUES, m_valueSpecifications);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverride::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!CustomizationRule::_ShallowEqual(other))
        return false;

    InstanceLabelOverride const* otherRule = dynamic_cast<InstanceLabelOverride const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_className == otherRule->m_className;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::_Accept(CustomizationRuleVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverride::AddValueSpecification(InstanceLabelOverrideValueSpecification& spec)
    {
    ADD_HASHABLE_CHILD(m_valueSpecifications, spec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceLabelOverrideRelatedInstanceLabelSpecification();
    else
        {
        Utf8String msg = json.isMember(INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_TYPE)
            ? Utf8PrintfString("Invalid `" INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_TYPE "` attribute value: `%s`", type)
            : Utf8String("Missing required attribute: `" INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_TYPE "`");
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, msg);
        }
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideValueSpecification::_GetJsonElementTypeAttributeName() const {return INSTANCE_LABEL_OVERRIDE_VALUE_SPECIFICATION_BASE_JSON_ATTRIBUTE_TYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideCompositeValueSpecification::InstanceLabelOverrideCompositeValueSpecification(InstanceLabelOverrideCompositeValueSpecification const& other)
    : T_Super(other), m_separator(other.m_separator)
    {
    CommonToolsInternal::CopyRules<Part>(m_parts, other.m_parts, this);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideCompositeValueSpecification::InstanceLabelOverrideCompositeValueSpecification(InstanceLabelOverrideCompositeValueSpecification&& other)
    : T_Super(std::move(other)), m_separator(other.m_separator)
    {
    m_parts.swap(other.m_parts);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideCompositeValueSpecification::~InstanceLabelOverrideCompositeValueSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_parts);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideCompositeValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideCompositeValueSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_separator.empty())
        ADD_STR_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR, m_separator);
    ADD_RULES_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PARTS, m_parts);
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideCompositeValueSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!InstanceLabelOverrideValueSpecification::_ShallowEqual(other))
        return false;

    InstanceLabelOverrideCompositeValueSpecification const* otherRule = dynamic_cast<InstanceLabelOverrideCompositeValueSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_separator == otherRule->m_separator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static InstanceLabelOverrideCompositeValueSpecification::Part* PartsFactory(JsonValueCR partJson) { return CommonToolsInternal::LoadRuleFromJson<InstanceLabelOverrideCompositeValueSpecification::Part>(partJson); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideCompositeValueSpecification::_ReadJson(JsonValueCR json)
    {
    if (!InstanceLabelOverrideValueSpecification::_ReadJson(json))
        return false;

    if (json.isMember(INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR) && json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR].isString())
        m_separator = json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR].asCString();

    if (json.isMember(INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PARTS))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PARTS, json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PARTS], m_parts, &PartsFactory, this);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideCompositeValueSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    if (!m_separator.Equals(" "))
        json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR] = m_separator;
    CommonToolsInternal::WriteRulesToJson<Part, bvector<Part*>>(json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PARTS], m_parts);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideCompositeValueSpecification::AddValuePart(InstanceLabelOverrideValueSpecification& partSpecification, bool isRequired)
    {
    Part* part = new Part(partSpecification, isRequired);
    ADD_HASHABLE_CHILD(m_parts, *part);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideCompositeValueSpecification::Part::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_isRequired)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED, m_isRequired);
    if (m_specification)
        {
        Utf8StringCR specHash = m_specification->GetHash();
        ADD_STR_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC, specHash);
        }
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideCompositeValueSpecification::Part::_ShallowEqual(PresentationKeyCR other) const
    {
    InstanceLabelOverrideCompositeValueSpecification::Part const* otherRule = dynamic_cast<InstanceLabelOverrideCompositeValueSpecification::Part const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_isRequired == otherRule->m_isRequired;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideCompositeValueSpecification::Part::_ReadJson(JsonValueCR json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    if (CommonToolsInternal::CheckRuleIssue(!json.isMember(INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC), INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC ".Part", INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC, Json::Value(), "valid JSON object"))
        return false;

    m_specification = InstanceLabelOverrideValueSpecification::Create(json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC]);
    if (CommonToolsInternal::CheckRuleIssue(!m_specification, INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC ".Part", INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC, json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC], "valid specification"))
        return false;

    m_specification->SetParent(this);

    // optional:
    if (json.isMember(INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED) && json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED].isBool())
        m_isRequired = json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED].asBool();

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideCompositeValueSpecification::Part::_WriteJson(JsonValueR json) const
    {
    PresentationKey::_WriteJson(json);
    if (m_specification)
        json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC] = m_specification->WriteJson();
    if (m_isRequired)
        json[INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED] = m_isRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverridePropertyValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverridePropertyValueSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_propertyName.empty())
        ADD_STR_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME, m_propertyName);
    if (!m_pathToRelatedInstanceSpec.GetSteps().empty())
        ADD_STR_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYSOURCE, m_pathToRelatedInstanceSpec.GetHash());
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverridePropertyValueSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!InstanceLabelOverrideValueSpecification::_ShallowEqual(other))
        return false;

    InstanceLabelOverridePropertyValueSpecification const* otherRule = dynamic_cast<InstanceLabelOverridePropertyValueSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_propertyName == otherRule->m_propertyName
        && m_pathToRelatedInstanceSpec == otherRule->m_pathToRelatedInstanceSpec;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverridePropertyValueSpecification::_ReadJson(JsonValueCR json)
    {
    if (!InstanceLabelOverrideValueSpecification::_ReadJson(json))
        return false;

    // required:
    m_propertyName = Utf8String(json[INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME].asCString()).Trim();
    if (CommonToolsInternal::CheckRuleIssue(m_propertyName.empty(), _GetJsonElementType(), INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME, json[INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME], "non-empty string"))
        return false;

    // optional:
    if (json.isMember(INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYSOURCE))
        m_pathToRelatedInstanceSpec.ReadJson(json[INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYSOURCE]);

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverridePropertyValueSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    json[INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME] = m_propertyName;
    if (m_pathToRelatedInstanceSpec.GetSteps().size() > 0)
        json[INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYSOURCE] = m_pathToRelatedInstanceSpec.WriteJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideClassNameValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideClassNameValueSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_full)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_ATTRIBUTE_FULL, m_full);
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideClassNameValueSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!InstanceLabelOverrideValueSpecification::_ShallowEqual(other))
        return false;

    InstanceLabelOverrideClassNameValueSpecification const* otherRule = dynamic_cast<InstanceLabelOverrideClassNameValueSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_full == otherRule->m_full;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideClassNameValueSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    if (m_full)
        json[INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_ATTRIBUTE_FULL] = m_full;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideClassLabelValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_CLASSLABEL_VALUE_SPECIFICATION_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideBriefcaseIdValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_BRIEFCASEID_VALUE_SPECIFICATION_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideLocalIdValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_LOCALID_VALUE_SPECIFICATION_JSON_TYPE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideStringValueSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideStringValueSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_value.empty())
        ADD_STR_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE, m_value);
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideStringValueSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!InstanceLabelOverrideValueSpecification::_ShallowEqual(other))
        return false;

    InstanceLabelOverrideStringValueSpecification const* otherRule = dynamic_cast<InstanceLabelOverrideStringValueSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_value == otherRule->m_value;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideStringValueSpecification::_ReadJson(JsonValueCR json)
    {
    if (!InstanceLabelOverrideValueSpecification::_ReadJson(json))
        return false;

    // required:
    m_value = json[INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE].asCString();
    if (CommonToolsInternal::CheckRuleIssue(m_value.empty(), _GetJsonElementType(), INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE, json[INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE], "non-empty string"))
        return false;

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideStringValueSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    json[INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE] = m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceLabelOverrideRelatedInstanceLabelSpecification::_GetJsonElementType() const { return INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_TYPE; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceLabelOverrideRelatedInstanceLabelSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_pathToRelatedInstanceSpec.GetSteps().empty())
        ADD_STR_VALUE_TO_HASH(md5, INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_ATTRIBUTE_PATHTORELATEDINSTANCE, m_pathToRelatedInstanceSpec.GetHash());
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideRelatedInstanceLabelSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!InstanceLabelOverrideValueSpecification::_ShallowEqual(other))
        return false;

    InstanceLabelOverrideRelatedInstanceLabelSpecification const* otherRule = dynamic_cast<InstanceLabelOverrideRelatedInstanceLabelSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_pathToRelatedInstanceSpec == otherRule->m_pathToRelatedInstanceSpec;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceLabelOverrideRelatedInstanceLabelSpecification::_ReadJson(JsonValueCR json)
    {
    if (!InstanceLabelOverrideValueSpecification::_ReadJson(json))
        return false;

    // required:
    m_pathToRelatedInstanceSpec.ReadJson(json[INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_ATTRIBUTE_PATHTORELATEDINSTANCE]);
    if (CommonToolsInternal::CheckRuleIssue(m_pathToRelatedInstanceSpec.GetSteps().empty(), _GetJsonElementType(), INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_ATTRIBUTE_PATHTORELATEDINSTANCE, json[INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_ATTRIBUTE_PATHTORELATEDINSTANCE], "non-empty relationship path specification"))
        return false;

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceLabelOverrideRelatedInstanceLabelSpecification::_WriteJson(JsonValueR json) const
    {
    InstanceLabelOverrideValueSpecification::_WriteJson(json);
    json[INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_ATTRIBUTE_PATHTORELATEDINSTANCE] = m_pathToRelatedInstanceSpec.WriteJson();
    }
