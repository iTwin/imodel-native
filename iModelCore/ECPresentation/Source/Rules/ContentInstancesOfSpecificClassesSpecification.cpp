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
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification()
    : m_handlePropertiesPolymorphically(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification(int priority, Utf8StringCR instanceFilter,
    Utf8StringCR classNames, bool handleInstancesPolymorphically, bool handlePropertiesPolymorphically)
    : ContentInstancesOfSpecificClassesSpecification(priority, false, instanceFilter, bvector<MultiSchemaClass*>(), bvector<MultiSchemaClass*>(), handlePropertiesPolymorphically)
    {
    CommonToolsInternal::ParseMultiSchemaClassesFromClassNamesString(classNames, handleInstancesPolymorphically, m_classes, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification(int priority, bool onlyIfNotHandled, Utf8StringCR instanceFilter,
    bvector<MultiSchemaClass*> classes, bvector<MultiSchemaClass*> excludedClasses, bool handlePropertiesPolymorphically)
    : ContentSpecification(priority, false, onlyIfNotHandled), m_instanceFilter(instanceFilter), m_classes(classes), m_excludedClasses(excludedClasses),
    m_handlePropertiesPolymorphically(handlePropertiesPolymorphically)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification(ContentInstancesOfSpecificClassesSpecification const& other)
    : ContentSpecification(other), m_instanceFilter(other.m_instanceFilter), m_handlePropertiesPolymorphically(other.m_handlePropertiesPolymorphically)
    {
    CommonToolsInternal::CopyRules(m_classes, other.m_classes, this);
    CommonToolsInternal::CopyRules(m_excludedClasses, other.m_excludedClasses, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification(ContentInstancesOfSpecificClassesSpecification&& other)
    : ContentSpecification(std::move(other)), m_instanceFilter(std::move(other.m_instanceFilter)), m_handlePropertiesPolymorphically(other.m_handlePropertiesPolymorphically)
    {
    CommonToolsInternal::SwapRules(m_classes, other.m_classes, this);
    CommonToolsInternal::SwapRules(m_excludedClasses, other.m_excludedClasses, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentInstancesOfSpecificClassesSpecification::~ContentInstancesOfSpecificClassesSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_classes);
    CommonToolsInternal::FreePresentationRules(m_excludedClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentInstancesOfSpecificClassesSpecification::_GetXmlElementName () const
    {
    return CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::SetClasses(bvector<MultiSchemaClass*> value)
    {
    CommonToolsInternal::FreePresentationRules(m_classes);
    m_classes = value;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::SetExcludedClasses(bvector<MultiSchemaClass*> value)
    {
    CommonToolsInternal::FreePresentationRules(m_excludedClasses);
    m_excludedClasses = value;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentInstancesOfSpecificClassesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ContentSpecification::_ReadXml(xmlNode))
        return false;

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = "";

    bool defaultPolymorphism;
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(defaultPolymorphism, COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC))
        defaultPolymorphism = false;

    // required:
    Utf8String classNames;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(classNames, COMMON_XML_ATTRIBUTE_CLASSNAMES) ||
        !CommonToolsInternal::ParseMultiSchemaClassesFromClassNamesString(classNames, defaultPolymorphism, m_classes, this) ||
        m_classes.empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_CLASSNAMES));
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ContentSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(COMMON_XML_ATTRIBUTE_CLASSNAMES, CommonToolsInternal::SchemaAndClassNamesToString(CommonToolsInternal::MultiSchemaClassesToJson(m_classes)).c_str());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());

    bool handlePolymorphically = m_classes.empty() ? false : m_classes.front()->GetArePolymorphic();
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC, handlePolymorphically);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentInstancesOfSpecificClassesSpecification::_GetJsonElementType() const
    {
    return CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentInstancesOfSpecificClassesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ContentSpecification::_ReadJson(json))
        return false;

    // optional:
    m_handlePropertiesPolymorphically = json[CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEPROPERTIESPOLYMORPHICALLY].asBool(false);
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");

    bool defaultPolymorphism = false;
    if (json.isMember(CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEINSTANCESPOLYMORPHICALLY))
        {
        defaultPolymorphism = json[CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEINSTANCESPOLYMORPHICALLY].asBool(false);

        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_WARNING, Utf8PrintfString("Using deprecated `%s.%s`. It's recommended to switch to `%s.%s.%s`.",
            _GetJsonElementType(), CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEINSTANCESPOLYMORPHICALLY,
            _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_CLASSES, COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC));
        }
    else if (json.isMember(COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC))
        {
        defaultPolymorphism = json[COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC].asBool(false);

        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_WARNING, Utf8PrintfString("Using deprecated `%s.%s`. It's recommended to switch to `%s.%s`.",
            _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC,
            _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_CLASSES, COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC));
        }
    CommonToolsInternal::ParseMultiSchemaClassesFromJson(json[COMMON_JSON_ATTRIBUTE_EXCLUDEDCLASSES], defaultPolymorphism, m_excludedClasses, this);

    // required:
    if (!CommonToolsInternal::ParseMultiSchemaClassesFromJson(json[COMMON_JSON_ATTRIBUTE_CLASSES], defaultPolymorphism, m_classes, this) || m_classes.empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid value for `%s`: `%s`. Expected %s.",
            _GetJsonElementType(), json.ToString().c_str(), "at least one class"));
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::_WriteJson(JsonValueR json) const
    {
    ContentSpecification::_WriteJson(json);
    json[COMMON_JSON_ATTRIBUTE_CLASSES] = CommonToolsInternal::MultiSchemaClassesToJson(m_classes);
    if (!m_excludedClasses.empty())
        json[COMMON_JSON_ATTRIBUTE_EXCLUDEDCLASSES] = CommonToolsInternal::MultiSchemaClassesToJson(m_excludedClasses);
    if (m_handlePropertiesPolymorphically)
        json[CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEPROPERTIESPOLYMORPHICALLY] = true;
    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentInstancesOfSpecificClassesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_instanceFilter.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter);
    if (!m_classes.empty())
        ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_CLASSES, m_classes);
    if (!m_excludedClasses.empty())
        ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_EXCLUDEDCLASSES, m_excludedClasses);
    if (m_handlePropertiesPolymorphically)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEPROPERTIESPOLYMORPHICALLY, m_handlePropertiesPolymorphically);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentInstancesOfSpecificClassesSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ContentSpecification::_ShallowEqual(other))
        return false;

    ContentInstancesOfSpecificClassesSpecification const* otherRule = dynamic_cast<ContentInstancesOfSpecificClassesSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_instanceFilter == otherRule->m_instanceFilter
        && m_classes == otherRule->m_classes
        && m_excludedClasses == otherRule->m_excludedClasses
        && m_handlePropertiesPolymorphically == otherRule->m_handlePropertiesPolymorphically;
    }
