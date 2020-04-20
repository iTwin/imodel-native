/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipStepSpecification::ReadJson(JsonValueCR json)
    {
    m_relationshipClassName = CommonToolsInternal::SchemaAndClassNameToString(json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP]);
    if (m_relationshipClassName.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelationshipStepSpecification", RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP);
        return false;
        }

    m_direction = CommonToolsInternal::ParseRequiredDirectionString(json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION].asCString());
    if (m_direction != RequiredRelationDirection_Backward && m_direction != RequiredRelationDirection_Forward)
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelationshipStepSpecification", RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION);
        return false;
        }

    m_targetClassName = CommonToolsInternal::SchemaAndClassNameToString(json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_TARGETCLASS]);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value RelationshipStepSpecification::WriteJson() const
    {
    Json::Value json;
    json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP] = CommonToolsInternal::SchemaAndClassNameToJson(m_relationshipClassName);
    json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_direction);
    if (!m_targetClassName.empty())
        json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_TARGETCLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_targetClassName);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelationshipStepSpecification::_ComputeHash() const
    {
    MD5 md5;
    md5.Add(m_relationshipClassName.c_str(), m_relationshipClassName.size());
    md5.Add(&m_direction, sizeof(m_direction));
    md5.Add(m_targetClassName.c_str(), m_targetClassName.size());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepeatableRelationshipStepSpecification::ReadJson(JsonValueCR json)
    {
    if (!RelationshipStepSpecification::ReadJson(json))
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
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value RepeatableRelationshipStepSpecification::WriteJson() const
    {
    Json::Value json = RelationshipStepSpecification::WriteJson();
    if (m_count == 0)
        json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT] = "*";
    else if (m_count != 1)
        json[RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT] = m_count;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RepeatableRelationshipStepSpecification::_ComputeHash() const
    {
    MD5 md5 = RelationshipStepSpecification::_ComputeHash();
    md5.Add(&m_count, sizeof(m_count));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification::RelationshipPathSpecification(RelationshipPathSpecification const& other)
    {
    CommonToolsInternal::CopyRules(m_steps, other.m_steps, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification::RelationshipPathSpecification(RelationshipPathSpecification&& other)
    {
    m_steps.swap(other.m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification::~RelationshipPathSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification& RelationshipPathSpecification::operator=(RelationshipPathSpecification const& other)
    {
    CommonToolsInternal::FreePresentationRules(m_steps);
    CommonToolsInternal::CopyRules(m_steps, other.m_steps, this);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipPathSpecification& RelationshipPathSpecification::operator=(RelationshipPathSpecification&& other)
    {
    m_steps.swap(other.m_steps);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipPathSpecification::ReadJson(JsonValueCR json)
    {
    if (json.isArray())
        {
        CommonToolsInternal::LoadFromJson(json, m_steps, CommonToolsInternal::LoadRuleFromJson<RelationshipStepSpecification>, this);
        return true;
        }
    if (json.isObject())
        {
        RelationshipStepSpecification* rule = CommonToolsInternal::LoadRuleFromJson<RelationshipStepSpecification>(json);
        if (nullptr == rule)
            return false;
        rule->SetParent(this);
        m_steps.push_back(rule);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value RelationshipPathSpecification::WriteJson() const
    {
    if (m_steps.size() == 1)
        return m_steps[0]->WriteJson();

    Json::Value json(Json::arrayValue);
    CommonToolsInternal::WriteRulesToJson<RelationshipStepSpecification, bvector<RelationshipStepSpecification*>>(json, m_steps);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipPathSpecification::AddStep(RelationshipStepSpecification& step)
    {
    ADD_HASHABLE_CHILD(m_steps, step);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelationshipPathSpecification::_ComputeHash() const
    {
    MD5 md5;
    ADD_RULES_TO_HASH(md5, m_steps);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification::RepeatableRelationshipPathSpecification(RepeatableRelationshipPathSpecification const& other)
    {
    CommonToolsInternal::CopyRules(m_steps, other.m_steps, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification::RepeatableRelationshipPathSpecification(RepeatableRelationshipPathSpecification&& other)
    {
    m_steps.swap(other.m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification::~RepeatableRelationshipPathSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification& RepeatableRelationshipPathSpecification::operator=(RepeatableRelationshipPathSpecification const& other)
    {
    CommonToolsInternal::FreePresentationRules(m_steps);
    CommonToolsInternal::CopyRules(m_steps, other.m_steps, this);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RepeatableRelationshipPathSpecification& RepeatableRelationshipPathSpecification::operator=(RepeatableRelationshipPathSpecification&& other)
    {
    m_steps.swap(other.m_steps);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepeatableRelationshipPathSpecification::ReadJson(JsonValueCR json)
    {
    if (json.isArray())
        {
        CommonToolsInternal::LoadFromJson(json, m_steps, CommonToolsInternal::LoadRuleFromJson<RepeatableRelationshipStepSpecification>, this);
        return true;
        }
    if (json.isObject())
        {
        RepeatableRelationshipStepSpecification* rule = CommonToolsInternal::LoadRuleFromJson<RepeatableRelationshipStepSpecification>(json);
        if (nullptr == rule)
            return false;
        rule->SetParent(this);
        m_steps.push_back(rule);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value RepeatableRelationshipPathSpecification::WriteJson() const
    {
    if (m_steps.size() == 1)
        return m_steps[0]->WriteJson();

    Json::Value json(Json::arrayValue);
    CommonToolsInternal::WriteRulesToJson<RepeatableRelationshipStepSpecification, bvector<RepeatableRelationshipStepSpecification*>>(json, m_steps);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void RepeatableRelationshipPathSpecification::AddStep(RepeatableRelationshipStepSpecification& step)
    {
    ADD_HASHABLE_CHILD(m_steps, step);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void RepeatableRelationshipPathSpecification::ClearSteps()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RepeatableRelationshipPathSpecification::_ComputeHash() const
    {
    MD5 md5;
    ADD_RULES_TO_HASH(md5, m_steps);
    return md5;
    }
