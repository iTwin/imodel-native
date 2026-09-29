/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinition::LocalizationResourceKeyDefinition()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinition::LocalizationResourceKeyDefinition(int priority, Utf8StringCR id, Utf8StringCR key, Utf8StringCR defaultValue)
    : PrioritizedPresentationKey(priority), m_id(id), m_key(key), m_defaultValue(defaultValue)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LocalizationResourceKeyDefinition::GetId (void) const             { return m_id; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LocalizationResourceKeyDefinition::GetKey (void) const            { return m_key; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LocalizationResourceKeyDefinition::GetDefaultValue (void) const   { return m_defaultValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 LocalizationResourceKeyDefinition::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_id.empty())
        ADD_STR_VALUE_TO_HASH(md5, "id", m_id);
    if (!m_key.empty())
        ADD_STR_VALUE_TO_HASH(md5, "key", m_key);
    if (!m_defaultValue.empty())
        ADD_STR_VALUE_TO_HASH(md5, "defaultValue", m_defaultValue);
    return md5;
    }
