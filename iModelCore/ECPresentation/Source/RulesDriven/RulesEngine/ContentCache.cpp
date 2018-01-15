/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentCache.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ContentCache.h"
#include "ContentProviders.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey::ContentProviderKey(Utf8String connectionId, Utf8String rulesetId, Utf8String displayType, INavNodeKeysContainerCR inputNodeKeys,
    SelectionInfo const* selectionInfo)
    : m_connectionId(connectionId), m_rulesetId(rulesetId), m_preferredDisplayType(displayType), m_inputNodeKeys(&inputNodeKeys), m_selectionInfo(selectionInfo)
    {
    if (nullptr != selectionInfo)
        m_selectionInfo = new SelectionInfo(*selectionInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey::ContentProviderKey(ContentProviderKey const& other)
    : m_connectionId(other.m_connectionId), m_rulesetId(other.m_rulesetId), m_preferredDisplayType(other.m_preferredDisplayType), m_inputNodeKeys(other.m_inputNodeKeys),
    m_selectionInfo(other.m_selectionInfo)
    {
    if (nullptr != other.m_selectionInfo)
        m_selectionInfo = new SelectionInfo(*other.m_selectionInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey::ContentProviderKey(ContentProviderKey&& other)
    : m_connectionId(std::move(other.m_connectionId)), m_rulesetId(std::move(other.m_rulesetId)), m_preferredDisplayType(std::move(other.m_preferredDisplayType)),
    m_inputNodeKeys(std::move(other.m_inputNodeKeys))
    {
    m_selectionInfo = other.m_selectionInfo;
    other.m_selectionInfo = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderKey& ContentProviderKey::operator=(ContentProviderKey const& other)
    {
    m_connectionId = other.m_connectionId;
    m_rulesetId = other.m_rulesetId;
    m_preferredDisplayType = other.m_preferredDisplayType;
    m_inputNodeKeys = other.m_inputNodeKeys;
    m_selectionInfo = (nullptr != other.m_selectionInfo) ? new SelectionInfo(*other.m_selectionInfo) : nullptr;
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
    m_inputNodeKeys = std::move(other.m_inputNodeKeys);
    m_selectionInfo = other.m_selectionInfo;
    other.m_selectionInfo = nullptr;
    return *this;
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

    int inputNodeKeysCompareResult = m_inputNodeKeys->GetHash().CompareTo(other.m_inputNodeKeys->GetHash());
    if (inputNodeKeysCompareResult < 0)
        return true;
    if (inputNodeKeysCompareResult > 0)
        return false;
    
    if (nullptr == m_selectionInfo && nullptr == other.m_selectionInfo)
        return false;
    if (nullptr == m_selectionInfo && nullptr != other.m_selectionInfo)
        return true;
    if (nullptr != m_selectionInfo && nullptr == other.m_selectionInfo)
        return false;
    return *m_selectionInfo < *other.m_selectionInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename ValueType> static void Erase(bmap<ContentProviderKey, ValueType>& map, std::function<bool(ContentProviderKey const&)> const& predicate)
    {
    for (auto iter = map.begin(); iter != map.end();)
        {
        ContentProviderKey const& key = iter->first;
        if (predicate(key))
            iter = map.erase(iter);
        else
            ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCache::ClearCache(IConnectionCR connection)
    {
    auto pred = [&](ContentProviderKey const& key){return key.GetConnectionId().Equals(connection.GetId());};
    Erase(m_providers, pred);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCache::ClearCache(Utf8StringCR rulesetId)
    {
    auto pred = [&](ContentProviderKey const& key){return key.GetRulesetId().Equals(rulesetId);};
    Erase(m_providers, pred);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderPtr ContentCache::GetProvider(ContentProviderKey const& key) const
    {
    auto iter = m_providers.find(key);
    if (m_providers.end() != iter)
        return iter->second;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SpecificationContentProviderPtr> ContentCache::GetProviders(IConnectionCR connection) const
    {
    bvector<SpecificationContentProviderPtr> providers;
    for (auto pair : m_providers)
        {
        ContentProviderKey const& key = pair.first;
        if (key.GetConnectionId().Equals(connection.GetId()))
            providers.push_back(pair.second);
        }
    return providers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasRelatedSetting(ContentProviderCR provider, Utf8CP settingId)
    {
    bvector<Utf8String> const& relatedSettingIds = provider.GetContext().GetRelatedSettingIds();
    for (Utf8StringCR relatedSettingId : relatedSettingIds)
        {
        if (relatedSettingId.Equals(settingId))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SpecificationContentProviderPtr> ContentCache::GetProviders(Utf8CP rulesetId, Utf8CP settingId) const
    {
    bvector<SpecificationContentProviderPtr> providers;
    for (auto pair : m_providers)
        {
        ContentProviderKey const& key = pair.first;
        if (key.GetRulesetId().Equals(rulesetId) && HasRelatedSetting(*pair.second, settingId))
            providers.push_back(pair.second);
        }
    return providers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCache::CacheProvider(ContentProviderKey key, SpecificationContentProviderR provider)
    {
    m_providers[key] = &provider;
    }
