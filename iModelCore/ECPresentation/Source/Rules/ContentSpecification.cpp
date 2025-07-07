/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationP ContentSpecification::Create(BeJsConst json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString("");
    ContentSpecificationP spec = nullptr;
    if (0 == strcmp(SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_TYPE, type))
        spec = new SelectedNodeInstancesSpecification();
    else if (0 == strcmp(CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE, type))
        spec = new ContentInstancesOfSpecificClassesSpecification();
    else if (0 == strcmp(CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_TYPE, type))
        spec = new ContentRelatedInstancesSpecification();
    else
        {
        Utf8String msg = json.isMember(COMMON_JSON_ATTRIBUTE_SPECTYPE)
            ? Utf8PrintfString("Invalid `" COMMON_JSON_ATTRIBUTE_SPECTYPE "` attribute value: `%s`", type)
            : Utf8String("Missing required attribute: `" COMMON_JSON_ATTRIBUTE_SPECTYPE "`");
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, msg);
        }
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification() : m_showImages(false), m_onlyIfNotHandled(false) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(int priority, bool showImages, bool onlyIfNotHandled)
    : PresentationRuleSpecification(priority), m_showImages(showImages), m_onlyIfNotHandled(onlyIfNotHandled)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(ContentSpecificationCR other)
    : PresentationRuleSpecification(other), m_showImages(other.m_showImages), m_onlyIfNotHandled(other.m_onlyIfNotHandled), m_modifiers(other.m_modifiers)
    {
    CommonToolsInternal::CopyRules(m_relatedInstances, other.m_relatedInstances, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(ContentSpecification&& other)
    : PresentationRuleSpecification(std::move(other)), m_showImages(other.m_showImages), m_onlyIfNotHandled(other.m_onlyIfNotHandled), m_modifiers(std::move(other.m_modifiers))
    {
    CommonToolsInternal::SwapRules(m_relatedInstances, other.m_relatedInstances, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::~ContentSpecification ()
    {
    CommonToolsInternal::FreePresentationRules(m_relatedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecification::_ReadJson(BeJsConst json)
    {
    if (!PresentationRuleSpecification::_ReadJson(json))
        return false;

    m_showImages = json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES].asBool(false);
    m_onlyIfNotHandled = json[COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED].asBool(false);

    if (json.isMember(CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION))
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION, json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION],
            m_relatedInstances, CommonToolsInternal::LoadRuleFromJson<RelatedInstanceSpecification>, this);
        }

    return m_modifiers.ReadJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::_WriteJson(BeJsValue json) const
    {
    PresentationRuleSpecification::_WriteJson(json);
    if (m_showImages)
        json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES] = m_showImages;
    if (m_onlyIfNotHandled)
        json[COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED] = m_onlyIfNotHandled;
    if (!m_relatedInstances.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedInstanceSpecification, RelatedInstanceSpecificationList>
            (json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION], m_relatedInstances);
        }
    m_modifiers.WriteJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance) { ADD_HASHABLE_CHILD(m_relatedInstances, relatedInstance); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::ClearRelatedInstances()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_relatedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_showImages)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES, m_showImages);
    if (m_onlyIfNotHandled)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);
    ADD_RULES_TO_HASH(md5, CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION, m_relatedInstances);

    Utf8StringCR modifiersHash = m_modifiers.GetHash();
    md5.Add(modifiersHash.c_str(), modifiersHash.size());

    return md5;
    }
