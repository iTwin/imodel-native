/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentCache.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "RulesEngineTypes.h"
#include "ContentProviders.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=============================================================================**//**
* Key used for caching content providers.
* @bsiclass                                     Grigas.Petraitis            07/2016
+===============+===============+===============+===============+===============+==*/
struct ContentProviderKey
{
private:
    Utf8String m_connectionId;
    Utf8String m_rulesetId;
    Utf8String m_preferredDisplayType; 
    INavNodeKeysContainerCPtr m_inputNodeKeys;
    SelectionInfo const* m_selectionInfo;
public:
    ContentProviderKey() : m_selectionInfo(nullptr) {}
    ContentProviderKey(Utf8String connectionId, Utf8String rulesetId, Utf8String displayType, INavNodeKeysContainerCR inputNodeKeys, SelectionInfo const* selectionInfo);
    ContentProviderKey(ContentProviderKey const& other);
    ContentProviderKey(ContentProviderKey&& other);
    ContentProviderKey& operator=(ContentProviderKey const& other);
    ContentProviderKey& operator=(ContentProviderKey&& other);
    ~ContentProviderKey() {DELETE_AND_CLEAR(m_selectionInfo);}

    bool operator<(ContentProviderKey const& other) const;
    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct ContentCache : NonCopyableClass
{
private:
    bmap<ContentProviderKey, SpecificationContentProviderPtr> m_providers;

public:
    void ClearCache() {m_providers.clear();}
    void ClearCache(IConnectionCR connection);
    void ClearCache(Utf8StringCR rulesetId);
    SpecificationContentProviderPtr GetProvider(ContentProviderKey const& key) const;
    bvector<SpecificationContentProviderPtr> GetProviders(IConnectionCR) const;
    bvector<SpecificationContentProviderPtr> GetProviders(Utf8CP rulesetId, Utf8CP settingId) const;
    void CacheProvider(ContentProviderKey key, SpecificationContentProviderR provider);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
