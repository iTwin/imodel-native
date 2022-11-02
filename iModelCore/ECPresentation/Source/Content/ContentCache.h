/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationManager.h>
#include <deque>
#include "../RulesEngineTypes.h"
#include "ContentProviders.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define CONTENTCACHE_Size 100
#define CONTENTCACHE_Provider_Variations_Limit 5

/*=============================================================================**//**
* Key used for caching content providers.
* @bsiclass
+===============+===============+===============+===============+===============+==*/
struct ContentProviderKey
{
private:
    Utf8String m_connectionId;
    Utf8String m_rulesetId;
    Utf8String m_preferredDisplayType;
    int m_contentFlags;
    ECPresentation::UnitSystem m_unitSystem;
    INavNodeKeysContainerCPtr m_inputNodeKeys;
    SelectionInfoCPtr m_selectionInfo;
public:
    ContentProviderKey() {}
    ECPRESENTATION_EXPORT ContentProviderKey(Utf8String connectionId, Utf8String rulesetId, Utf8String displayType,
        int contentFlags, ECPresentation::UnitSystem, INavNodeKeysContainerCR inputNodeKeys,
        SelectionInfo const* selectionInfo);
    ECPRESENTATION_EXPORT ContentProviderKey(ContentProviderKey const& other);
    ECPRESENTATION_EXPORT ContentProviderKey(ContentProviderKey&& other);
    ECPRESENTATION_EXPORT ContentProviderKey& operator=(ContentProviderKey const& other);
    ECPRESENTATION_EXPORT ContentProviderKey& operator=(ContentProviderKey&& other);
    ECPRESENTATION_EXPORT bool operator<(ContentProviderKey const& other) const;
    ECPRESENTATION_EXPORT bool operator==(ContentProviderKey const& other) const;
    bool operator!=(ContentProviderKey const& other) const { return !operator==(other); }

    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    int GetContentFlags() const {return m_contentFlags;}
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
    ECPresentation::UnitSystem GetUnitSystem() const {return m_unitSystem;}
    INavNodeKeysContainerCR GetInputNodeKeys() const {return *m_inputNodeKeys;}
    SelectionInfo const* GetSelectionInfo() const {return m_selectionInfo.get();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentCacheEntry
{
private:
    ContentProviderKey m_key;
    SpecificationContentProviderPtr m_provider;
public:
    ContentCacheEntry(ContentProviderKey const& key, SpecificationContentProviderR provider)
        : m_key(key), m_provider(&provider)
        {}
    ContentProviderKey const& GetProviderKey() const { return m_key; }
    RulesetVariables GetRelatedVariables() const { return RulesetVariables(m_provider->GetContext().GetRelatedRulesetVariables()); }
    SpecificationContentProviderR GetProvider() const { return *m_provider; }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentCache : NonCopyableClass
{
private:
    std::deque<ContentCacheEntry> m_cacheEntries;
    size_t m_cacheSize;
    mutable BeMutex m_mutex;

private:
    void LimitProviderVariations(ContentProviderKey const& key);

public:
    ContentCache(size_t size = CONTENTCACHE_Size) : m_cacheSize(size) {}
    BeMutex& GetMutex() {return m_mutex;}
    void ClearCache() {BeMutexHolder lock(m_mutex); m_cacheEntries.clear();}
    ECPRESENTATION_EXPORT void ClearCache(IConnectionCR connection);
    ECPRESENTATION_EXPORT void ClearCache(Utf8StringCR rulesetId);
    ECPRESENTATION_EXPORT SpecificationContentProviderPtr GetProvider(ContentProviderKey const& key, RulesetVariables const& variables);
    ECPRESENTATION_EXPORT bvector<SpecificationContentProviderPtr> GetProviders(IConnectionCR) const;
    ECPRESENTATION_EXPORT bvector<SpecificationContentProviderPtr> GetProviders(Utf8CP rulesetId, Utf8CP settingId) const;
    ECPRESENTATION_EXPORT void CacheProvider(ContentProviderKey key, SpecificationContentProviderR provider);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
