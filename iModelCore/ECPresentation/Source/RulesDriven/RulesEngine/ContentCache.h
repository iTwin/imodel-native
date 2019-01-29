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
    Utf8String m_locale;
    INavNodeKeysContainerCPtr m_inputNodeKeys;
    SelectionInfoCPtr m_selectionInfo;
public:
    ContentProviderKey() {}
    ECPRESENTATION_EXPORT ContentProviderKey(Utf8String connectionId, Utf8String rulesetId, Utf8String displayType, Utf8String locale, INavNodeKeysContainerCR inputNodeKeys, SelectionInfo const* selectionInfo);
    ECPRESENTATION_EXPORT ContentProviderKey(ContentProviderKey const& other);
    ECPRESENTATION_EXPORT ContentProviderKey(ContentProviderKey&& other);
    ECPRESENTATION_EXPORT ContentProviderKey& operator=(ContentProviderKey const& other);
    ECPRESENTATION_EXPORT ContentProviderKey& operator=(ContentProviderKey&& other);
    ECPRESENTATION_EXPORT bool operator<(ContentProviderKey const& other) const;

    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
    Utf8StringCR GetLocale() const {return m_locale;}
    SelectionInfo const* GetSelectionInfo() const {return m_selectionInfo.get();}
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
    ECPRESENTATION_EXPORT void ClearCache(IConnectionCR connection);
    ECPRESENTATION_EXPORT void ClearCache(Utf8StringCR rulesetId);
    ECPRESENTATION_EXPORT SpecificationContentProviderPtr GetProvider(ContentProviderKey const& key) const;
    ECPRESENTATION_EXPORT bvector<SpecificationContentProviderPtr> GetProviders(IConnectionCR) const;
    ECPRESENTATION_EXPORT bvector<SpecificationContentProviderPtr> GetProviders(Utf8CP rulesetId, Utf8CP settingId) const;
    ECPRESENTATION_EXPORT void CacheProvider(ContentProviderKey key, SpecificationContentProviderR provider);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
