/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentCache.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ContentCache.h"
#include "ContentProviders.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentProviderKey::operator<(ContentProviderKey const& other) const
    {
    if (m_db < other.m_db)
        return true;

    if (m_db > other.m_db)
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
    
    return m_selectionInfo < other.m_selectionInfo;
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
void ContentCache::ClearCache(ECDbCR connection)
    {
    auto pred = [&](ContentProviderKey const& key){return &key.GetConnection() == &connection;};
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
bvector<SpecificationContentProviderPtr> ContentCache::GetProviders(ECDbCR connection) const
    {
    bvector<SpecificationContentProviderPtr> providers;
    for (auto pair : m_providers)
        {
        ContentProviderKey const& key = pair.first;
        if (&key.GetConnection() == &connection)
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
