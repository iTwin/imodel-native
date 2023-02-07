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
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification()
    : ChildNodeSpecification(), m_groupByClass(true), m_groupByLabel(true), m_showEmptyGroups(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, bool showEmptyGroups, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic)
    : InstanceNodesOfSpecificClassesSpecification(priority, alwaysReturnsChildren ? ChildrenHint::Always : ChildrenHint::Unknown, hideNodesInHierarchy,
        hideIfNoChildren, groupByClass, groupByLabel, instanceFilter, classNames, arePolymorphic)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic)
    : InstanceNodesOfSpecificClassesSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren, groupByClass, groupByLabel, instanceFilter,
        bvector<MultiSchemaClass*>(), bvector<MultiSchemaClass*>())
    {
    CommonToolsInternal::ParseMultiSchemaClassesFromClassNamesString(classNames, arePolymorphic, m_classes, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
        bool hideIfNoChildren, bool groupByClass, bool groupByLabel, Utf8StringCR instanceFilter, bvector<MultiSchemaClass*> classes,
        bvector<MultiSchemaClass*> excludedClasses)
    : ChildNodeSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren),
    m_groupByClass(groupByClass), m_groupByLabel(groupByLabel), m_showEmptyGroups(false),
    m_instanceFilter(instanceFilter), m_classes(classes), m_excludedClasses(excludedClasses)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification(InstanceNodesOfSpecificClassesSpecification const& other)
    : ChildNodeSpecification(other), m_groupByClass(other.m_groupByClass), m_groupByLabel(other.m_groupByLabel), m_showEmptyGroups(other.m_showEmptyGroups),
    m_instanceFilter(other.m_instanceFilter)
    {
    CommonToolsInternal::CopyRules(m_classes, other.m_classes, this);
    CommonToolsInternal::CopyRules(m_excludedClasses, other.m_excludedClasses, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::InstanceNodesOfSpecificClassesSpecification(InstanceNodesOfSpecificClassesSpecification&& other)
    : ChildNodeSpecification(std::move(other)), m_groupByClass(other.m_groupByClass), m_groupByLabel(other.m_groupByLabel), m_showEmptyGroups(other.m_showEmptyGroups),
    m_instanceFilter(std::move(other.m_instanceFilter))
    {
    CommonToolsInternal::SwapRules(m_classes, other.m_classes, this);
    CommonToolsInternal::SwapRules(m_excludedClasses, other.m_excludedClasses, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceNodesOfSpecificClassesSpecification::~InstanceNodesOfSpecificClassesSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_classes);
    CommonToolsInternal::FreePresentationRules(m_excludedClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ChildNodeSpecification::_ShallowEqual(other))
        return false;

    InstanceNodesOfSpecificClassesSpecificationCP otherRule = dynamic_cast<InstanceNodesOfSpecificClassesSpecificationCP>(&other);
    if (nullptr == otherRule)
        return false;

    return m_groupByClass == otherRule->m_groupByClass
        && m_groupByLabel == otherRule->m_groupByLabel
        && m_showEmptyGroups == otherRule->m_showEmptyGroups
        && m_classes == otherRule->m_classes
        && m_excludedClasses == otherRule->m_excludedClasses
        && m_instanceFilter == otherRule->m_instanceFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceNodesOfSpecificClassesSpecification::_GetXmlElementName() const
    {
    return INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!ChildNodeSpecification::_ReadXml(xmlNode))
        return false;

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_groupByClass, COMMON_XML_ATTRIBUTE_GROUPBYCLASS))
        m_groupByClass = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_groupByLabel, COMMON_XML_ATTRIBUTE_GROUPBYLABEL))
        m_groupByLabel = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_showEmptyGroups, COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS))
        m_showEmptyGroups = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
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
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_CLASSNAMES));
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    ChildNodeSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(COMMON_XML_ATTRIBUTE_CLASSNAMES, CommonToolsInternal::SchemaAndClassNamesToString(CommonToolsInternal::MultiSchemaClassesToJson(m_classes)).c_str());
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS, m_showEmptyGroups);
    xmlNode->AddAttributeStringValue(COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str());

    bool arePolymorphic = m_classes.empty() ? false : m_classes.front()->GetArePolymorphic();
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC, arePolymorphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP InstanceNodesOfSpecificClassesSpecification::_GetJsonElementType() const
    {
    return INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;

    // optional:
    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");

    if (json.isMember(COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_WARNING, Utf8PrintfString("Using deprecated `%s.%s`. It's recommended to switch to `%s.%s`.",
            _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC, COMMON_JSON_ATTRIBUTE_CLASSES, COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC));
        }
    bool defaultPolymorphism = json[COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC].asBool(false);

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
void InstanceNodesOfSpecificClassesSpecification::_WriteJson(JsonValueR json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    if (!m_classes.empty())
        json[COMMON_JSON_ATTRIBUTE_CLASSES] = CommonToolsInternal::MultiSchemaClassesToJson(m_classes);
    if (!m_excludedClasses.empty())
        json[COMMON_JSON_ATTRIBUTE_EXCLUDEDCLASSES] = CommonToolsInternal::MultiSchemaClassesToJson(m_excludedClasses);
    if (!m_groupByClass)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS] = m_groupByClass;
    if (!m_groupByLabel)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL] = m_groupByLabel;
    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::GetGroupByClass(void) const { return m_groupByClass; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetGroupByClass(bool value) { m_groupByClass = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::GetGroupByLabel(void) const { return m_groupByLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetGroupByLabel(bool value) { m_groupByLabel = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceNodesOfSpecificClassesSpecification::GetShowEmptyGroups(void) const { return m_showEmptyGroups; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetShowEmptyGroups(bool value) { m_showEmptyGroups = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetClasses(bvector<MultiSchemaClass*> value)
    {
    CommonToolsInternal::FreePresentationRules(m_classes);
    m_classes = value;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetExcludedClasses(bvector<MultiSchemaClass*> value)
    {
    CommonToolsInternal::FreePresentationRules(m_excludedClasses);
    m_excludedClasses = value;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR InstanceNodesOfSpecificClassesSpecification::GetInstanceFilter(void) const { return m_instanceFilter; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceNodesOfSpecificClassesSpecification::SetInstanceFilter(Utf8String value) { m_instanceFilter = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 InstanceNodesOfSpecificClassesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();

    if (!m_groupByClass)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    if (!m_groupByLabel)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    if (m_showEmptyGroups)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS, m_showEmptyGroups);
    if (!m_instanceFilter.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter);
    if (!m_classes.empty())
        ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_CLASSES, m_classes);
    if (!m_excludedClasses.empty())
        ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_EXCLUDEDCLASSES, m_excludedClasses);
    return md5;
    }
