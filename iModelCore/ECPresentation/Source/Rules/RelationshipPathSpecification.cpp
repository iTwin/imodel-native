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
Utf8CP RelationshipStepSpecification::_GetJsonElementType() const { return "RelationshipStepSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipStepSpecification::_ReadJson(JsonValueCR json)
    {
    if (!T_Super::_ReadJson(json))
        return false;

    // required:
    m_relationshipClassName = CommonToolsInternal::SchemaAndClassNameToString(json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP], Utf8PrintfString("%s.%s", _GetJsonElementType(), RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP).c_str());
    m_direction = CommonToolsInternal::ParseRequiredDirectionString(json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION].asCString(), _GetJsonElementType());

    bool hasIssues = false
        || CommonToolsInternal::CheckRuleIssue(m_relationshipClassName.empty(), _GetJsonElementType(), RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP, json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP], "relationship class specification")
        || CommonToolsInternal::CheckRuleIssue((m_direction != RequiredRelationDirection_Backward && m_direction != RequiredRelationDirection_Forward), "RelationshipStepSpecification", RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION, json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION], "'Forward' or 'Backward'");
    if (hasIssues)
        return false;

    // optional:
    if (json.isMember(RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_TARGETCLASS))
        m_targetClassName = CommonToolsInternal::SchemaAndClassNameToString(json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_TARGETCLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_TARGETCLASS).c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipStepSpecification::_WriteJson(JsonValueR json) const
    {
    T_Super::_WriteJson(json);
    json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP] = CommonToolsInternal::SchemaAndClassNameToJson(m_relationshipClassName);
    json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_direction);
    if (!m_targetClassName.empty())
        json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_TARGETCLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_targetClassName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelationshipStepSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_relationshipClassName.empty())
        ADD_STR_VALUE_TO_HASH(md5, RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP, m_relationshipClassName);
    if (m_direction != RequiredRelationDirection_Forward)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION, m_direction);
    if (!m_targetClassName.empty())
        ADD_STR_VALUE_TO_HASH(md5, RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_TARGETCLASS, m_targetClassName);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipStepSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    RelationshipStepSpecification const* otherRule = dynamic_cast<RelationshipStepSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_relationshipClassName == otherRule->m_relationshipClassName
        && m_direction == otherRule->m_direction
        && m_targetClassName == otherRule->m_targetClassName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RepeatableRelationshipStepSpecification::_GetJsonElementType() const { return "RepeatableRelationshipStepSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepeatableRelationshipStepSpecification::_ReadJson(JsonValueCR json)
    {
    if (!RelationshipStepSpecification::_ReadJson(json))
        return false;

    if (!json.isMember(RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT))
        m_count = 1;
    else if (json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT].isString() && 0 == strcmp("*", json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT].asCString()))
        m_count = 0;
    else
        m_count = json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT].asInt(1);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RepeatableRelationshipStepSpecification::_WriteJson(JsonValueR json) const
    {
    RelationshipStepSpecification::_WriteJson(json);
    if (m_count == 0)
        json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT] = "*";
    else if (m_count != 1)
        json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT] = m_count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RepeatableRelationshipStepSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_count != 1)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT, m_count);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepeatableRelationshipStepSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!RelationshipStepSpecification::_ShallowEqual(other))
        return false;

    RepeatableRelationshipStepSpecification const* otherRule = dynamic_cast<RepeatableRelationshipStepSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_count == otherRule->m_count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification::RelationshipPathSpecification(RelationshipPathSpecification const& other)
    : T_Super(other)
    {
    CommonToolsInternal::CopyRules(m_steps, other.m_steps, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification::RelationshipPathSpecification(RelationshipPathSpecification&& other)
    : T_Super(std::move(other))
    {
    m_steps.swap(other.m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification::~RelationshipPathSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification& RelationshipPathSpecification::operator=(RelationshipPathSpecification const& other)
    {
    CommonToolsInternal::FreePresentationRules(m_steps);
    CommonToolsInternal::CopyRules(m_steps, other.m_steps, this);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification& RelationshipPathSpecification::operator=(RelationshipPathSpecification&& other)
    {
    m_steps.swap(other.m_steps);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelationshipPathSpecification::_GetJsonElementType() const { return "RelationshipPathSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipPathSpecification::_ReadJson(JsonValueCR json)
    {
    // note: intentionally not calling base
    if (json.isArray())
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), nullptr, json, m_steps, CommonToolsInternal::LoadRuleFromJson<RelationshipStepSpecification>, this);
        if (m_steps.empty())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid value for `%s`: `%s`. Expected %s.",
                _GetJsonElementType(), json.ToString().c_str(), "at least one step specification"));
            return false;
            }
        return true;
        }
    if (json.isObject())
        {
        RelationshipStepSpecification* rule = CommonToolsInternal::LoadRuleFromJson<RelationshipStepSpecification>(json);
        if (nullptr != rule)
            {
            rule->SetParent(this);
            m_steps.push_back(rule);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipPathSpecification::_WriteJson(JsonValueR json) const
    {
    // note: intentionally not calling base
    if (m_steps.size() == 1)
        {
        json = m_steps[0]->WriteJson();
        return;
        }

    json = Json::Value(Json::arrayValue);
    CommonToolsInternal::WriteRulesToJson<RelationshipStepSpecification, bvector<RelationshipStepSpecification*>>(json, m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipPathSpecification::AddStep(RelationshipStepSpecification& step)
    {
    ADD_HASHABLE_CHILD(m_steps, step);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelationshipPathSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    ADD_RULES_TO_HASH(md5, "Steps", m_steps);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipPathSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    RelationshipPathSpecification const* otherRule = dynamic_cast<RelationshipPathSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification::RepeatableRelationshipPathSpecification(RepeatableRelationshipPathSpecification const& other)
    : T_Super(other)
    {
    CommonToolsInternal::CopyRules(m_steps, other.m_steps, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification::RepeatableRelationshipPathSpecification(RepeatableRelationshipPathSpecification&& other)
    : T_Super(std::move(other))
    {
    m_steps.swap(other.m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification::~RepeatableRelationshipPathSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification& RepeatableRelationshipPathSpecification::operator=(RepeatableRelationshipPathSpecification const& other)
    {
    CommonToolsInternal::FreePresentationRules(m_steps);
    CommonToolsInternal::CopyRules(m_steps, other.m_steps, this);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification& RepeatableRelationshipPathSpecification::operator=(RepeatableRelationshipPathSpecification&& other)
    {
    m_steps.swap(other.m_steps);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RepeatableRelationshipPathSpecification::_GetJsonElementType() const { return "RepeatableRelationshipPathSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepeatableRelationshipPathSpecification::_ReadJson(JsonValueCR json)
    {
    // note: intentionally not calling base
    if (json.isArray())
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), nullptr, json, m_steps, CommonToolsInternal::LoadRuleFromJson<RepeatableRelationshipStepSpecification>, this);
        if (m_steps.empty())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid value for `%s`: `%s`. Expected %s.",
                _GetJsonElementType(), json.ToString().c_str(), "at least one step specification"));
            return false;
            }
        return true;
        }
    if (json.isObject())
        {
        RepeatableRelationshipStepSpecification* rule = CommonToolsInternal::LoadRuleFromJson<RepeatableRelationshipStepSpecification>(json);
        if (nullptr != rule)
            {
            rule->SetParent(this);
            m_steps.push_back(rule);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RepeatableRelationshipPathSpecification::_WriteJson(JsonValueR json) const
    {
    // note: intentionally not calling base
    if (m_steps.size() == 1)
        {
        json = m_steps[0]->WriteJson();
        return;
        }

    json = Json::Value(Json::arrayValue);
    CommonToolsInternal::WriteRulesToJson<RepeatableRelationshipStepSpecification, bvector<RepeatableRelationshipStepSpecification*>>(json, m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RepeatableRelationshipPathSpecification::AddStep(RepeatableRelationshipStepSpecification& step)
    {
    ADD_HASHABLE_CHILD(m_steps, step);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RepeatableRelationshipPathSpecification::ClearSteps()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RepeatableRelationshipPathSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    ADD_RULES_TO_HASH(md5, "Steps", m_steps);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepeatableRelationshipPathSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    RepeatableRelationshipPathSpecification const* otherRule = dynamic_cast<RepeatableRelationshipPathSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return true;
    }
