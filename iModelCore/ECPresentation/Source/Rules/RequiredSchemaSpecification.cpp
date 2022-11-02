/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RequiredSchemaSpecification::_GetJsonElementType() const { return "RequiredSchemaSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RequiredSchemaSpecification::_ReadJson(JsonValueCR json)
    {
    if (!T_Super::_ReadJson(json))
        return false;

    // required:
    m_name = json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_NAME].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_name.empty(), _GetJsonElementType(), REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_NAME, json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_NAME], "non-empty string"))
        return false;

    // optional:
    if (json.isMember(REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MINVERSION))
        {
        Version temp;
        if (SUCCESS == Version::FromString(temp, json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MINVERSION].asCString()))
            m_minVersion = temp;
        else
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid value for `%s.%s`: `%s`. Expected %s.",
                _GetJsonElementType(), REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MINVERSION, json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MINVERSION].ToString().c_str(),
                "version string in format {read version}.{write version}.{minor version}"));
            }
        }
    if (json.isMember(REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MAXVERSION))
        {
        Version temp;
        if (SUCCESS == Version::FromString(temp, json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MAXVERSION].asCString()))
            m_maxVersion = temp;
        else
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid value for `%s.%s`: `%s`. Expected %s.",
                _GetJsonElementType(), REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MAXVERSION, json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MAXVERSION].ToString().c_str(),
                "version string in format {read version}.{write version}.{minor version}"));
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RequiredSchemaSpecification::_WriteJson(JsonValueR json) const
    {
    T_Super::_WriteJson(json);
    json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_NAME] = m_name;
    if (m_minVersion != nullptr)
        json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MINVERSION] = m_minVersion.Value().ToString();
    if (m_maxVersion != nullptr)
        json[REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MAXVERSION] = m_maxVersion.Value().ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RequiredSchemaSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_name.empty())
        ADD_STR_VALUE_TO_HASH(md5, REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_NAME, m_name);
    if (m_minVersion != nullptr)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MINVERSION, m_minVersion.Value());
    if (m_maxVersion != nullptr)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MAXVERSION, m_maxVersion.Value());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RequiredSchemaSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!T_Super::_ShallowEqual(other))
        return false;

    RequiredSchemaSpecification const* otherRule = dynamic_cast<RequiredSchemaSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_name == otherRule->m_name
        && m_minVersion == otherRule->m_minVersion
        && m_maxVersion == otherRule->m_maxVersion;
    }
