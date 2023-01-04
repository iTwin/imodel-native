/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SortPropertiesByOverridePriority(PropertySpecificationsList& properties)
    {
    std::sort(properties.begin(), properties.end(),
        [&](PropertySpecificationP const lhs, PropertySpecificationP const rhs) { return lhs->GetOverridesPriority() > rhs->GetOverridesPriority(); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NoPropertiesIncluded(PropertySpecificationsList const& properties)
    {
    return properties.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AllPropertiesWithNoOverridesIncluded(PropertySpecificationsList const& properties)
    {
    static auto defaultHash = PropertySpecification(INCLUDE_ALL_PROPERTIES_SPEC).GetHash();
    return properties.size() == 1
        && properties[0]->GetHash() == defaultHash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus CreatePropertySpecFromPropertyName(PropertySpecificationP& spec, Utf8StringCR name, HashableBase* parent)
    {
    if (name.empty())
        return ERROR;

    spec = new PropertySpecification(name);
    spec->SetParent(parent);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PropertySpecificationsList CreatePropertySpecsFromPropertyNames(Utf8StringCR namesStr, HashableBase* parent)
    {
    PropertySpecificationsList list;
    if (namesStr.EqualsI(INCLUDE_NO_PROPERTIES_SPEC))
        return list;

    if (namesStr.empty() || namesStr.Equals(INCLUDE_ALL_PROPERTIES_SPEC))
        {
        PropertySpecificationP spec = nullptr;
        CreatePropertySpecFromPropertyName(spec, INCLUDE_ALL_PROPERTIES_SPEC, parent);
        list.push_back(spec);
        return list;
        }

    bvector<Utf8String> names;
    BeStringUtilities::Split(namesStr.c_str(), ",", names);
    for (Utf8StringCR name : names)
        {
        PropertySpecificationP spec = nullptr;
        CreatePropertySpecFromPropertyName(spec, name, parent);
        if (nullptr != spec)
            list.push_back(spec);
        }
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetPropertyNamesStr(PropertySpecificationsList const& properties)
    {
    if (NoPropertiesIncluded(properties))
        return INCLUDE_NO_PROPERTIES_SPEC;
    else if (AllPropertiesWithNoOverridesIncluded(properties))
        return "";

    Utf8String names;
    for (PropertySpecificationCP spec : properties)
        {
        if (!names.empty())
            names.append(",");
        names.append(spec->GetPropertyName());
        }
    return names;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddToListByOverridesPriority(PropertySpecificationsList& list, PropertySpecificationR element)
    {
    auto iter = list.rbegin();
    for (; iter != list.rend(); iter++)
        {
        if (element.GetOverridesPriority() <= (*iter)->GetOverridesPriority())
            break;
        }
    list.insert(iter.base(), &element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification()
    : m_requiredDirection(RequiredRelationDirection_Both), m_relationshipMeaning(RelationshipMeaning::RelatedInstance), m_polymorphic(false), m_autoExpand(false),
    m_propertiesSourceSpecification(nullptr), m_shouldSkipIfDuplicate(false), m_forceCreateRelationshipCategory(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection, Utf8String relationshipClassNames,
    Utf8String relatedClassNames, Utf8String propertyNames, RelationshipMeaning relationshipMeaning, bool polymorphic, bool autoExpand)
    : m_requiredDirection(requiredDirection), m_relationshipClassNames(relationshipClassNames), m_relatedClassNames(relatedClassNames),
    m_relationshipMeaning (relationshipMeaning), m_polymorphic(polymorphic), m_autoExpand(autoExpand), m_propertiesSourceSpecification(nullptr),
    m_shouldSkipIfDuplicate(false), m_forceCreateRelationshipCategory(false)
    {
    m_properties = CreatePropertySpecsFromPropertyNames(propertyNames, this);
    SortPropertiesByOverridePriority(m_properties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection, Utf8String relationshipClassNames,
    Utf8String relatedClassNames, PropertySpecificationsList properties, RelationshipMeaning relationshipMeaning, bool polymorphic, bool autoExpand)
    : m_requiredDirection(requiredDirection), m_relationshipClassNames(relationshipClassNames), m_relatedClassNames(relatedClassNames),
    m_relationshipMeaning(relationshipMeaning), m_properties(properties), m_polymorphic(polymorphic), m_autoExpand(autoExpand), m_propertiesSourceSpecification(nullptr),
    m_shouldSkipIfDuplicate(false), m_forceCreateRelationshipCategory(false)
    {
    SortPropertiesByOverridePriority(m_properties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RelationshipPathSpecification& propertiesSource, PropertySpecificationsList properties,
    RelationshipMeaning relationshipMeaning, bool polymorphic, bool autoExpand, bool skipIfDuplicate, PropertySpecificationsList relationshipProperties,
    bool forceCreateRelationshipCategory)
    : m_propertiesSourceSpecification(&propertiesSource), m_properties(properties), m_relationshipMeaning(relationshipMeaning),
    m_polymorphic(polymorphic), m_autoExpand(autoExpand), m_requiredDirection(RequiredRelationDirection_Both),
    m_shouldSkipIfDuplicate(skipIfDuplicate), m_relationshipProperties(relationshipProperties), m_forceCreateRelationshipCategory(forceCreateRelationshipCategory)
    {
    SortPropertiesByOverridePriority(m_properties);
    SortPropertiesByOverridePriority(m_relationshipProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RelatedPropertiesSpecification const& other)
    : PresentationKey(other), m_requiredDirection(other.m_requiredDirection), m_relationshipClassNames(other.m_relationshipClassNames),
    m_relatedClassNames(other.m_relatedClassNames), m_instanceFilter(other.m_instanceFilter),
    m_relationshipMeaning(other.m_relationshipMeaning), m_polymorphic(other.m_polymorphic), m_autoExpand(other.m_autoExpand),
    m_propertiesSourceSpecification(nullptr), m_shouldSkipIfDuplicate(other.m_shouldSkipIfDuplicate), m_forceCreateRelationshipCategory(other.m_forceCreateRelationshipCategory)
    {
    CommonToolsInternal::CopyRules(m_properties, other.m_properties, this);
    CommonToolsInternal::CopyRules(m_relationshipProperties, other.m_relationshipProperties, this);
    CommonToolsInternal::CopyRules(m_nestedRelatedPropertiesSpecification, other.m_nestedRelatedPropertiesSpecification, this);
    if (nullptr != other.m_propertiesSourceSpecification)
        m_propertiesSourceSpecification = new RelationshipPathSpecification(*other.m_propertiesSourceSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RelatedPropertiesSpecification&& other)
    : PresentationKey(std::move(other)), m_requiredDirection(other.m_requiredDirection), m_relationshipClassNames(std::move(other.m_relationshipClassNames)),
    m_relatedClassNames(std::move(other.m_relatedClassNames)), m_instanceFilter(std::move(other.m_instanceFilter)),
    m_relationshipMeaning(other.m_relationshipMeaning), m_polymorphic(other.m_polymorphic), m_autoExpand(other.m_autoExpand),
    m_shouldSkipIfDuplicate(other.m_shouldSkipIfDuplicate), m_forceCreateRelationshipCategory(other.m_forceCreateRelationshipCategory)
    {
    m_properties.swap(other.m_properties);
    m_relationshipProperties.swap(other.m_relationshipProperties);
    m_nestedRelatedPropertiesSpecification.swap(other.m_nestedRelatedPropertiesSpecification);
    m_propertiesSourceSpecification = other.m_propertiesSourceSpecification;
    other.m_propertiesSourceSpecification = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::~RelatedPropertiesSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_properties);
    CommonToolsInternal::FreePresentationRules(m_relationshipProperties);
    CommonToolsInternal::FreePresentationRules(m_nestedRelatedPropertiesSpecification);
    DELETE_AND_CLEAR(m_propertiesSourceSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedPropertiesSpecification::_GetXmlElementName() const {return RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPropertiesSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relationshipClassNames, COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES))
        m_relationshipClassNames = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relatedClassNames, COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES))
        m_relatedClassNames = "";

    Utf8String propertyNames = "";
    xmlNode->GetAttributeStringValue(propertyNames, COMMON_XML_ATTRIBUTE_PROPERTYNAMES);
    m_properties = CreatePropertySpecsFromPropertyNames(propertyNames, this);

    Utf8String requiredDirectionString = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION))
        requiredDirectionString = "";
    else
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString (requiredDirectionString.c_str (), _GetXmlElementName());

    Utf8String relationshipMeaningString = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue(relationshipMeaningString, COMMON_XML_ATTRIBUTE_RELATIONSHIPMEANING))
        relationshipMeaningString = "";
    else
        m_relationshipMeaning = CommonToolsInternal::ParseRelationshipMeaningString(relationshipMeaningString.c_str(), _GetXmlElementName());

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_polymorphic, COMMON_XML_ATTRIBUTE_ISPOLYMORPHIC))
        m_polymorphic = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_autoExpand, COMMON_XML_ATTRIBUTE_AUTOEXPAND))
        m_autoExpand = false;

    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_nestedRelatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    PresentationKey::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_PROPERTYNAMES, GetPropertyNamesStr(m_properties).c_str());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonToolsInternal::FormatRequiredDirectionString (m_requiredDirection));
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPMEANING, CommonToolsInternal::FormatRelationshipMeaningString(m_relationshipMeaning));
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_ISPOLYMORPHIC, m_polymorphic);
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_AUTOEXPAND, m_autoExpand);
    CommonToolsInternal::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_nestedRelatedPropertiesSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedPropertiesSpecification::_GetJsonElementType() const {return "RelatedPropertiesSpecification";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::ParsePropertiesFromPropertyJson(PropertySpecificationsList& properties, JsonValueCR propertySpecsJson, Utf8CP propertySpecJsonAttributeName)
    {
    if (propertySpecsJson.isString() && 0 == BeStringUtilities::Stricmp(propertySpecsJson.asCString(), INCLUDE_NO_PROPERTIES_SPEC))
        return;

    if (propertySpecsJson.isString() && 0 == BeStringUtilities::Stricmp(propertySpecsJson.asCString(), INCLUDE_ALL_PROPERTIES_SPEC))
        {
        PropertySpecificationP spec = nullptr;
        CreatePropertySpecFromPropertyName(spec, INCLUDE_ALL_PROPERTIES_SPEC, this);
        properties.push_back(spec);
        return;
        }

    if (propertySpecsJson.isArray() && !propertySpecsJson.isNull())
        {
        for (Json::ArrayIndex i = 0; i < propertySpecsJson.size(); ++i)
            {
            PropertySpecificationP spec = nullptr;
            if (propertySpecsJson[i].isObject() && !propertySpecsJson[i].isNull())
                spec = CommonToolsInternal::LoadRuleFromJson<PropertySpecification>(propertySpecsJson[i]);
            else if (propertySpecsJson[i].isString())
                CreatePropertySpecFromPropertyName(spec, propertySpecsJson[i].asCString(), this);

            if (nullptr != spec)
                AddToListByOverridesPriority(properties, *spec);
            else
                {
                Utf8PrintfString msg("Invalid value for `%s.%s[" PRIu64 "]`: `%s`. Expected property specification, name, '" INCLUDE_NO_PROPERTIES_SPEC "' or '" INCLUDE_ALL_PROPERTIES_SPEC "'.",
                        _GetJsonElementType(), propertySpecJsonAttributeName, (uint64_t)i, propertySpecsJson[i].ToString().c_str());
                DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, msg);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPropertiesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    bool hasIssues = false;
    if (json.isMember(RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE))
        {
        m_propertiesSourceSpecification = CommonToolsInternal::LoadRuleFromJson<RelationshipPathSpecification>(json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE]);
        hasIssues |= CommonToolsInternal::CheckRuleIssue(!m_propertiesSourceSpecification, _GetJsonElementType(), RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE, json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE], "non-empty relationship path");
        }
    else if (json.isMember(COMMON_JSON_ATTRIBUTE_RELATIONSHIPS) || json.isMember(COMMON_JSON_ATTRIBUTE_RELATEDCLASSES))
        {
        m_relationshipClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS]);
        m_relatedClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES]);
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""), _GetJsonElementType());
        if (m_relationshipClassNames.empty() && m_relatedClassNames.empty())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid `%s` specification. Expected either `%s` or `%s` to be specified, but none of them is.",
                _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPS, COMMON_JSON_ATTRIBUTE_RELATEDCLASSES));
            hasIssues = true;
            }
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_WARNING, Utf8PrintfString("Using deprecated attributes of `%s`: `%s`, `%s`, `%s`. It's recommended to switch to `%s`.",
            _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPS, COMMON_JSON_ATTRIBUTE_RELATEDCLASSES, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE));
        }
    else
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Missing required `%s` attribute: `%s`.", _GetJsonElementType(), RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE));
        hasIssues = true;
        }

    if (hasIssues)
        return false;

    // optional:
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");
    m_polymorphic = json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_HANDLETARGETCLASSPOLYMORPHICALLY].asBool(false) || json[COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC].asBool(false);
    m_autoExpand = json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND].asBool(false);
    m_shouldSkipIfDuplicate = json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_SKIPIFDUPLICATE].asBool(false);
    m_forceCreateRelationshipCategory = json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_FORCECREATERELATIONSHIPCATEGORY].asBool(false);

    if (json.isMember(COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING))
        m_relationshipMeaning = CommonToolsInternal::ParseRelationshipMeaningString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING].asCString(""), Utf8PrintfString("%s.%s", _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING).c_str());

    JsonValueCP propertyAttributeJson = nullptr;
    if (json.isMember(RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES))
        propertyAttributeJson = &json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES];
    else if (json.isMember(RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES))
        {
        propertyAttributeJson = &json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES];
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_WARNING, Utf8PrintfString("Using deprecated `%s.%s`. It's recommended to switch to `%s`.",
            _GetJsonElementType(), RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES));
        }

    if (propertyAttributeJson == nullptr)
        {
        PropertySpecificationP spec = nullptr;
        CreatePropertySpecFromPropertyName(spec, INCLUDE_ALL_PROPERTIES_SPEC, this);
        m_properties.push_back(spec);
        }
    else
        ParsePropertiesFromPropertyJson(m_properties, *propertyAttributeJson, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES);

    if (json.isMember(RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES))
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES, json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES],
            m_nestedRelatedPropertiesSpecification, CommonToolsInternal::LoadRuleFromJson<RelatedPropertiesSpecification>, this);
        }

    if (json.isMember(RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPROPERTIES))
        ParsePropertiesFromPropertyJson(m_relationshipProperties, json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPROPERTIES], RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPROPERTIES);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::_WriteJson(JsonValueR json) const
    {
    PresentationKey::_WriteJson(json);

    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    if (m_polymorphic)
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_HANDLETARGETCLASSPOLYMORPHICALLY] = m_polymorphic;
    if (m_autoExpand)
        json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND] = m_autoExpand;
    if (m_shouldSkipIfDuplicate)
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_SKIPIFDUPLICATE] = m_shouldSkipIfDuplicate;
    if (m_forceCreateRelationshipCategory)
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_FORCECREATERELATIONSHIPCATEGORY] = m_forceCreateRelationshipCategory;
    if (RelationshipMeaning::RelatedInstance != m_relationshipMeaning)
        json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING] = CommonToolsInternal::FormatRelationshipMeaningString(m_relationshipMeaning);
    if (RequiredRelationDirection_Both != m_requiredDirection)
        json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_requiredDirection);
    if (!m_relationshipClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relatedClassNames);
    if (m_propertiesSourceSpecification)
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE] = m_propertiesSourceSpecification->WriteJson();

    if (NoPropertiesIncluded(m_properties))
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES] = INCLUDE_NO_PROPERTIES_SPEC;
    else if (!AllPropertiesWithNoOverridesIncluded(m_properties))
        {
        CommonToolsInternal::WriteRulesToJson<PropertySpecification, PropertySpecificationsList>
            (json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES], m_properties);
        }

    if (AllPropertiesWithNoOverridesIncluded(m_relationshipProperties))
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPROPERTIES] = INCLUDE_ALL_PROPERTIES_SPEC;
    else if (!NoPropertiesIncluded(m_relationshipProperties))
        {
        CommonToolsInternal::WriteRulesToJson<PropertySpecification, PropertySpecificationsList>
            (json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPROPERTIES], m_relationshipProperties);
        }

    if (!m_nestedRelatedPropertiesSpecification.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>
            (json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES], m_nestedRelatedPropertiesSpecification);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::SetProperties(PropertySpecificationsList properties)
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_properties);
    m_properties = properties;
    SortPropertiesByOverridePriority(m_properties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::AddProperty(PropertySpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    AddToListByOverridesPriority(m_properties, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::SetRelationshipProperties(PropertySpecificationsList properties)
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_relationshipProperties);
    m_relationshipProperties = properties;
    SortPropertiesByOverridePriority(m_relationshipProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::AddRelationshipProperty(PropertySpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    AddToListByOverridesPriority(m_relationshipProperties, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList const& RelatedPropertiesSpecification::GetNestedRelatedProperties() const { return m_nestedRelatedPropertiesSpecification; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::AddNestedRelatedProperty(RelatedPropertiesSpecificationR specification)
    {
    ADD_HASHABLE_CHILD(m_nestedRelatedPropertiesSpecification, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedPropertiesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_instanceFilter.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter);
    if (m_requiredDirection != RequiredRelationDirection_Both)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION, m_requiredDirection);
    if (!m_relationshipClassNames.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATIONSHIPS, m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATEDCLASSES, m_relatedClassNames);
    if (m_propertiesSourceSpecification)
        ADD_STR_VALUE_TO_HASH(md5, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE, m_propertiesSourceSpecification->GetHash());
    ADD_RULES_TO_HASH(md5, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES, m_properties);
    ADD_RULES_TO_HASH(md5, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPROPERTIES, m_relationshipProperties);
    ADD_RULES_TO_HASH(md5, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES, m_nestedRelatedPropertiesSpecification);
    if (m_relationshipMeaning != RelationshipMeaning::RelatedInstance)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING, m_relationshipMeaning);
    if (m_polymorphic)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_HANDLETARGETCLASSPOLYMORPHICALLY, m_polymorphic);
    if (m_autoExpand)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_AUTOEXPAND, m_autoExpand);
    if (m_shouldSkipIfDuplicate)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_SKIPIFDUPLICATE, m_shouldSkipIfDuplicate);
    if (m_forceCreateRelationshipCategory)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_FORCECREATERELATIONSHIPCATEGORY, m_forceCreateRelationshipCategory);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPropertiesSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    RelatedPropertiesSpecification const* otherRule = dynamic_cast<RelatedPropertiesSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_requiredDirection == otherRule->m_requiredDirection
        && m_relationshipClassNames == otherRule->m_relationshipClassNames
        && m_relatedClassNames == otherRule->m_relatedClassNames
        && m_relationshipMeaning == otherRule->m_relationshipMeaning
        && m_instanceFilter == otherRule->m_instanceFilter
        && m_polymorphic == otherRule->m_polymorphic
        && m_autoExpand == otherRule->m_autoExpand
        && m_shouldSkipIfDuplicate == otherRule->m_shouldSkipIfDuplicate
        && m_forceCreateRelationshipCategory == otherRule->m_forceCreateRelationshipCategory;
    }
