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
Utf8CP RelatedInstanceSpecification::_GetJsonElementType() const { return "RelatedInstanceSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::_ReadJson(BeJsConst json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    bool hasIssues = false;
    if (json.isMember(RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH))
        {
        m_relationshipPath.ReadJson(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH]);
        hasIssues |= CommonToolsInternal::CheckRuleIssue(m_relationshipPath.GetSteps().empty(), _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH], "non-empty relationship path");
        }
    else if (json.isMember(RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS) && json.isMember(RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP))
        {
        Utf8String className = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS).c_str());
        Utf8String relationshipName = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP], Utf8PrintfString("%s.%s", _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP).c_str());
        RequiredRelationDirection direction = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""), _GetJsonElementType());
        hasIssues |= false
            || CommonToolsInternal::CheckRuleIssue(className.empty(), _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS], "class specification")
            || CommonToolsInternal::CheckRuleIssue(relationshipName.empty(), _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP], "relationship specification");
        if (!hasIssues)
            m_relationshipPath.AddStep(*new RelationshipStepSpecification(relationshipName, direction, className));
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_WARNING, Utf8PrintfString("Using deprecated attributes of `%s`: `%s`, `%s`, `%s`. It's recommended to switch to `%s`.",
            _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION,
            RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH));
        }
    else if (json.isMember(RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_TARGETINSTANCES))
        {
        auto targetInstancesSpec = std::make_unique<RelatedInstanceTargetInstancesSpecification>();
        if (targetInstancesSpec->ReadJson(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_TARGETINSTANCES]))
            m_targetInstancesSpecification = std::move(targetInstancesSpec);
        else
            hasIssues |= CommonToolsInternal::CheckRuleIssue(false, _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_TARGETINSTANCES, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_TARGETINSTANCES], "target instances specification");
        }
    else
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Missing required `%s` attribute: `%s` or `%s`.", _GetJsonElementType(),
            RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_TARGETINSTANCES));
        hasIssues = true;
        }

    m_alias = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS].asCString("");
    hasIssues |= CommonToolsInternal::CheckRuleIssue(m_alias.empty(), _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS], "non-empty string");

    if (hasIssues)
        return false;

    // optional:
    m_isRequired = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceSpecification::_WriteJson(BeJsValue json) const
    {
    PresentationKey::_WriteJson(json);
    if (nullptr != m_targetInstancesSpecification)
        m_targetInstancesSpecification->WriteJson(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_TARGETINSTANCES]);
    else
        m_relationshipPath.WriteJson(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH]);
    json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS] = m_alias;
    if (m_isRequired)
        json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED] = m_isRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedInstanceSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_relationshipPath.GetSteps().empty())
        ADD_STR_VALUE_TO_HASH(md5, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH, m_relationshipPath.GetHash());
    if (nullptr != m_targetInstancesSpecification)
        ADD_STR_VALUE_TO_HASH(md5, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_TARGETINSTANCES, m_targetInstancesSpecification->GetHash());
    if (!m_alias.empty())
        ADD_STR_VALUE_TO_HASH(md5, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS, m_alias);
    if (m_isRequired)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED, m_isRequired);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedInstanceTargetInstancesSpecification::_GetJsonElementType() const { return "RelatedInstanceTargetInstancesSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceTargetInstancesSpecification::_ReadJson(BeJsConst json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    bool hasIssues = false;

    m_className = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_CLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_CLASS).c_str());
    hasIssues = CommonToolsInternal::CheckRuleIssue(m_className.empty(), _GetJsonElementType(), RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_CLASS, json[RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_CLASS], "target class specification");

    if (json.isMember(RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_INSTANCEIDS) && json[RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_INSTANCEIDS].isArray())
        {
        BeJsConst instanceIDsJson = json[RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_INSTANCEIDS];
        for (BeJsConst::ArrayIndex i = 0; i < instanceIDsJson.size(); i++)
            {
            BentleyStatus status;
            auto instanceId = (ECInstanceId)BeInt64Id::FromString(instanceIDsJson[i].asCString(), &status);
            if (status == SUCCESS)
                m_instanceIds.push_back(instanceId);
            }
        }
    else
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Required `%s` attribute `%s` is missing of wrong type. Expecting an array of instance IDs.", _GetJsonElementType(), RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_INSTANCEIDS));
        hasIssues = true;
        }

    if (hasIssues)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceTargetInstancesSpecification::_WriteJson(BeJsValue json) const
    {
    PresentationKey::_WriteJson(json);
    CommonToolsInternal::WriteSchemaAndClassNameToJson(json[RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_CLASS], m_className);
    BeJsValue idsArrayJson = json[RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_INSTANCEIDS];
    for (auto const& instanceId : m_instanceIds)
        idsArrayJson[idsArrayJson.size()] = instanceId.ToHexStr();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedInstanceTargetInstancesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_className.empty())
        ADD_STR_VALUE_TO_HASH(md5, RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_CLASS, m_className);
    if (!m_instanceIds.empty())
        {
        md5.Add(RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_INSTANCEIDS, strlen(RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_INSTANCEIDS));
        for (auto const& instanceId : m_instanceIds)
            md5.Add(&instanceId, sizeof(instanceId));
        }
    return md5;
    }