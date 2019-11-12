/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Licensing/Utils/FeatureUserDataMap.h>
#include "../../Licensing/Logging.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FeatureUserDataMap::operator==(const FeatureUserDataMap &other) const
    {
    return m_featureUserDataMap.size() == other.m_featureUserDataMap.size() &&
        std::equal(m_featureUserDataMap.begin(), m_featureUserDataMap.end(), other.m_featureUserDataMap.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureUserDataMap::CopyMap(const FeatureUserDataMap* other)
    {
    m_featureUserDataMap.insert(other->m_featureUserDataMap.begin(), other->m_featureUserDataMap.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t FeatureUserDataMap::GetKeys(Utf8StringVector &vector)
    {
    for (auto const& kvp : m_featureUserDataMap)
        vector.push_back(kvp.first);

    return static_cast<uint64_t>(m_featureUserDataMap.size());
    }

/*---------------------------------------------------------------------------------**//**
* Gets a complete list of the map entries in the form: "key=val"
* returns the size of the list.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t FeatureUserDataMap::GetMapEntries(Utf8StringVector &vector)
    {
    for (auto const& kvp : m_featureUserDataMap)
        {
        Utf8String entry = kvp.first;
        entry.append(1, '=');
        entry.append(kvp.second);
        vector.push_back(entry);
        }

    return static_cast<uint64_t>(m_featureUserDataMap.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FeatureUserDataMap::AddAttribute(Utf8String key, Utf8String value)
    {    
    m_featureUserDataMap[key] = value;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FeatureUserDataMap::GetValue(Utf8CP key, Utf8StringR value)
    {
    auto found = m_featureUserDataMap.find(key);
    if (m_featureUserDataMap.end() != found)
        value.assign(found->second);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureUserDataMap::Dump() const
    {
    LOG.debug("AttributeMap: ==============================================");
    bool first = true;
    for (auto const& kvp : m_featureUserDataMap)
        {
        if (!first)
            LOG.debug("\n");

        first = false;
        LOG.debugv("Key:   %ls", kvp.first.c_str());
        LOG.debugv("Value: %ls", kvp.second.c_str());
        }

    LOG.debug("AttributeMap: ==============================================");
    }
