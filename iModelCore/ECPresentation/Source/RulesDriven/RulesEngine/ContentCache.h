/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentCache.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    SelectionInfo m_selectionInfo;
public:
    ContentProviderKey() {}
    ContentProviderKey(Utf8String connectionId, Utf8String rulesetId, Utf8String displayType, SelectionInfo selectionInfo) 
        : m_connectionId(connectionId), m_rulesetId(rulesetId), m_preferredDisplayType(displayType), m_selectionInfo(selectionInfo)
        {}
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
