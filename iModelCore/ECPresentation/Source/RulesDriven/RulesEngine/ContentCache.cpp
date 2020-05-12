/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ContentCache.h"
#include "ContentProviders.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey::ContentProviderKey(Utf8String connectionId, Utf8String rulesetId, Utf8String displayType, int contentFlags, Utf8String locale,
    ECPresentation::UnitSystem unitSystem, INavNodeKeysContainerCR inputNodeKeys, SelectionInfo const* selectionInfo)
    : m_connectionId(connectionId), m_rulesetId(rulesetId), m_preferredDisplayType(displayType), m_locale(locale),
    m_inputNodeKeys(&inputNodeKeys), m_selectionInfo(selectionInfo), m_contentFlags(contentFlags), m_unitSystem(unitSystem)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey::ContentProviderKey(ContentProviderKey const& other)
    : m_connectionId(other.m_connectionId), m_rulesetId(other.m_rulesetId), m_preferredDisplayType(other.m_preferredDisplayType),
    m_locale(other.m_locale), m_inputNodeKeys(other.m_inputNodeKeys), m_selectionInfo(other.m_selectionInfo), m_contentFlags(other.m_contentFlags),
    m_unitSystem(other.m_unitSystem)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey::ContentProviderKey(ContentProviderKey&& other)
    : m_connectionId(std::move(other.m_connectionId)), m_rulesetId(std::move(other.m_rulesetId)), m_preferredDisplayType(std::move(other.m_preferredDisplayType)),
    m_locale(other.m_locale), m_inputNodeKeys(std::move(other.m_inputNodeKeys)), m_selectionInfo(std::move(other.m_selectionInfo)), m_contentFlags(other.m_contentFlags),
    m_unitSystem(other.m_unitSystem)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey& ContentProviderKey::operator=(ContentProviderKey const& other)
    {
    m_connectionId = other.m_connectionId;
    m_rulesetId = other.m_rulesetId;
    m_preferredDisplayType = other.m_preferredDisplayType;
    m_contentFlags = other.m_contentFlags;
    m_locale = other.m_locale;
    m_unitSystem = other.m_unitSystem;
    m_inputNodeKeys = other.m_inputNodeKeys;
    m_selectionInfo = other.m_selectionInfo;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey& ContentProviderKey::operator=(ContentProviderKey&& other)
    {
    m_connectionId = std::move(other.m_connectionId);
    m_rulesetId = std::move(other.m_rulesetId);
    m_preferredDisplayType = std::move(other.m_preferredDisplayType);
    m_contentFlags = other.m_contentFlags;
    m_locale = std::move(other.m_locale);
    m_unitSystem = other.m_unitSystem;
    m_inputNodeKeys = std::move(other.m_inputNodeKeys);
    m_selectionInfo = std::move(other.m_selectionInfo);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentProviderKey::operator==(ContentProviderKey const& other) const
    {
    return m_connectionId == other.m_connectionId
        && m_rulesetId == other.m_rulesetId
        && m_preferredDisplayType == other.m_preferredDisplayType
        && m_contentFlags == other.m_contentFlags
        && m_unitSystem == other.m_unitSystem
        && m_locale == other.m_locale
        && m_inputNodeKeys->GetHash() == other.m_inputNodeKeys->GetHash()
        && ((m_selectionInfo.IsNull() && other.m_selectionInfo.IsNull())
            || (m_selectionInfo.IsValid() && other.m_selectionInfo.IsValid() && m_selectionInfo == other.m_selectionInfo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentProviderKey::operator<(ContentProviderKey const& other) const
    {
    int connectionIdCompareResult = m_connectionId.compare(other.m_connectionId);
    if (connectionIdCompareResult < 0)
        return true;
    if (connectionIdCompareResult > 0)
        return false;

    int rulesetIdCompareResult = m_rulesetId.compare(other.m_rulesetId);
    if (rulesetIdCompareResult < 0)
        return true;
    if (rulesetIdCompareResult > 0)
        return false;

    int displayTypeCompareResult = m_preferredDisplayType.compare(other.m_preferredDisplayType);
    if (displayTypeCompareResult < 0)
        return true;
    if (displayTypeCompareResult > 0)
        return false;

    if (m_contentFlags < other.m_contentFlags)
        return true;
    if (m_contentFlags > other.m_contentFlags)
        return false;

    if (m_unitSystem < other.m_unitSystem)
        return true;
    if (m_unitSystem > other.m_unitSystem)
        return false;

    int localeCompareResult = m_locale.compare(other.m_locale);
    if (localeCompareResult < 0)
        return true;
    if (localeCompareResult > 0)
        return false;

    int inputNodeKeysCompareResult = m_inputNodeKeys->GetHash().CompareTo(other.m_inputNodeKeys->GetHash());
    if (inputNodeKeysCompareResult < 0)
        return true;
    if (inputNodeKeysCompareResult > 0)
        return false;

    if (m_selectionInfo.IsNull() && other.m_selectionInfo.IsNull())
        return false;
    if (m_selectionInfo.IsNull() && other.m_selectionInfo.IsValid())
        return true;
    if (m_selectionInfo.IsValid() && other.m_selectionInfo.IsNull())
        return false;
    return *m_selectionInfo < *other.m_selectionInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void Erase(bvector<ContentCacheEntry>& vector, std::function<bool(ContentProviderKey const&)> const& predicate)
    {
    for (auto iter = vector.begin(); iter != vector.end();)
        {
        ContentProviderKey const& key = iter->GetProviderKey();
        if (predicate(key))
            iter = vector.erase(iter);
        else
            ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCache::ClearCache(IConnectionCR connection)
    {
    BeMutexHolder lock(m_mutex);
    auto pred = [&](ContentProviderKey const& key){return key.GetConnectionId().Equals(connection.GetId());};
    Erase(m_cacheEntries, pred);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCache::ClearCache(Utf8StringCR rulesetId)
    {
    BeMutexHolder lock(m_mutex);
    auto pred = [&](ContentProviderKey const& key){return key.GetRulesetId().Equals(rulesetId);};
    Erase(m_cacheEntries, pred);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static SpecificationContentProviderPtr GetProviderOrClone(SpecificationContentProviderR provider)
    {
    // if provider ref count > 1, the provider is currently in use, we can't share it
    if (provider.GetRefCount() <= 1)
        return &provider;
    return provider.Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderPtr ContentCache::GetProvider(ContentProviderKey const& key, RulesetVariables const& variables)
    {
    BeMutexHolder lock(m_mutex);
    for (ContentCacheEntry& entry : m_cacheEntries)
        {
        ContentProviderKey const& cacheKey = entry.GetProviderKey();
        if (cacheKey == key && variables.Contains(entry.GetRelatedVariables()))
            {
            entry.SetLastUsedTime(BeTimeUtilities::GetCurrentTimeAsUnixMillis());
            return GetProviderOrClone(entry.GetProvider());
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SpecificationContentProviderPtr> ContentCache::GetProviders(IConnectionCR connection) const
    {
    BeMutexHolder lock(m_mutex);
    bvector<SpecificationContentProviderPtr> providers;
    for (ContentCacheEntry const& entry : m_cacheEntries)
        {
        ContentProviderKey const& key = entry.GetProviderKey();
        if (key.GetConnectionId().Equals(connection.GetId()))
            providers.push_back(GetProviderOrClone(entry.GetProvider()));
        }
    return providers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SpecificationContentProviderPtr> ContentCache::GetProviders(Utf8CP rulesetId, Utf8CP variableId) const
    {
    BeMutexHolder lock(m_mutex);
    bvector<SpecificationContentProviderPtr> providers;
    for (ContentCacheEntry const& entry : m_cacheEntries)
        {
        ContentProviderKey const& key = entry.GetProviderKey();
        if (key.GetRulesetId().Equals(rulesetId) && entry.GetRelatedVariables().HasValue(variableId))
            providers.push_back(GetProviderOrClone(entry.GetProvider()));
        }
    return providers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCache::CacheProvider(ContentProviderKey key, SpecificationContentProviderR provider)
    {
    BeMutexHolder lock(m_mutex);
    RulesetVariables relatedVariables(provider.GetContext().GetRelatedRulesetVariables());
    // remove same provider from cache
    m_cacheEntries.erase(std::remove_if(m_cacheEntries.begin(), m_cacheEntries.end(), [key, relatedVariables](ContentCacheEntry const& entry)
        {
        return entry.GetProviderKey() == key && entry.GetRelatedVariables() == relatedVariables;
        }),
        m_cacheEntries.end());

    m_cacheEntries.push_back(ContentCacheEntry(key, provider));
    LimitProviderVariations(key);
    if (m_cacheEntries.size() > CONTENTCACHE_Size)
        RemoveLeastRecentlyUsedProvider();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCache::LimitProviderVariations(ContentProviderKey const& key)
    {
    int variationsCount = 0;
    auto oldestVariation = m_cacheEntries.end();
    for (auto iter = m_cacheEntries.begin(); m_cacheEntries.end() != iter; iter++)
        {
        if (iter->GetProviderKey() != key)
            continue;

        variationsCount++;
        if (oldestVariation == m_cacheEntries.end() || oldestVariation->GetLastUsedTime() > iter->GetLastUsedTime())
            oldestVariation = iter;
        }

    if (CONTENTCACHE_Provider_Variations_Limit < variationsCount)
        m_cacheEntries.erase(oldestVariation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCache::RemoveLeastRecentlyUsedProvider()
    {
    auto oldestEntry = m_cacheEntries.begin();
    for (auto iter = m_cacheEntries.begin(); m_cacheEntries.end() != iter; iter++)
        {
        if (iter->GetLastUsedTime() < oldestEntry->GetLastUsedTime())
            oldestEntry = iter;
        }

    m_cacheEntries.erase(oldestEntry);
    }
