/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../../Source/Content/ContentCache.h"
#include "../../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define TEST_RELATED_SETTING "test related setting"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentCacheTests : ECPresentationTest
    {
    std::shared_ptr<TestNodeLocater> m_nodesLocater;
    TestCategorySupplier m_categorySupplier;
    ECExpressionsCache m_ecexpressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    IConnectionPtr m_connection;
    TestNodesFactory m_nodesFactory;

    ContentCache m_cache;

    ContentCacheTests()
        : m_connection(new TestEmptyConnection()), m_nodesFactory(*m_connection, "spec-id", "test"), m_categorySupplier(ContentDescriptor::Category("a", "b", "c", 0))
        {
        m_nodesLocater = std::make_shared<TestNodeLocater>();
        }

    SpecificationContentProviderPtr CreateProvider(RulesetVariables const& variables = RulesetVariables())
        {
        PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test");
        ContentProviderContextPtr context = ContentProviderContext::Create(*ruleset, "", 0, *NavNodeKeyListContainer::Create(),
            m_nodesLocater, m_categorySupplier, std::make_unique<RulesetVariables>(variables), m_ecexpressionsCache, m_relatedPathsCache,
            m_nodesFactory, nullptr);
        context->GetUsedVariablesListener().OnVariableUsed(TEST_RELATED_SETTING);
        return SpecificationContentProvider::Create(*context, ContentRuleInstanceKeysContainer());
        }

    SpecificationContentProviderP CacheProvider(ContentProviderKey& key, RulesetVariables const& variables = RulesetVariables())
        {
        SpecificationContentProviderPtr provider = CreateProvider(variables);
        m_cache.CacheProvider(key, *provider);
        return provider.get();
        }

    bvector<SpecificationContentProviderP> CacheProviderVariations(ContentProviderKey& key, int count)
        {
        RulesetVariables variables;
        bvector<SpecificationContentProviderP> providers;
        for (int i = 0; i < count; ++i)
            {
            variables.SetIntValue(TEST_RELATED_SETTING, i);
            SpecificationContentProviderPtr provider = CreateProvider(variables);
            m_cache.CacheProvider(key, *provider);
            providers.push_back(provider.get());
            }
        return providers;
        }

    bvector<bpair<ContentProviderKey, SpecificationContentProviderP>> CacheProviders(int count)
        {
        bvector<bpair<ContentProviderKey, SpecificationContentProviderP>> providers;
        for (int i = 0; i < count; ++i)
            {
            Utf8PrintfString rulesetId("ruleset_%d", i);
            ContentProviderKey key("connection id", rulesetId, "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr);
            SpecificationContentProviderPtr provider = CreateProvider();
            m_cache.CacheProvider(key, *provider);
            providers.push_back(bpair<ContentProviderKey, SpecificationContentProviderP>(key, provider.get()));
            }
        return providers;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, CachesProvider)
    {
    SelectionInfoPtr selectionInfo = SelectionInfo::Create("selection source name", false);
    ContentProviderKey key("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), selectionInfo.get());
    SpecificationContentProviderP provider = CacheProvider(key);
    EXPECT_EQ(provider, m_cache.GetProvider(key, RulesetVariables(provider->GetContext().GetRelatedRulesetVariables())).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, CachesMultipleProvidersWithDifferentKeys)
    {
    ContentProviderKey key1("connection id 1", "ruleset id 1", "display type 1", 0, ECPresentation::UnitSystem::Metric, *NavNodeKeyListContainer::Create(), nullptr);
    ContentProviderKey key2("connection id 2", "ruleset id 2", "display type 2", 1, ECPresentation::UnitSystem::BritishImperial, *NavNodeKeyListContainer::Create(), nullptr);
    SpecificationContentProviderP provider1 = CacheProvider(key1);
    SpecificationContentProviderP provider2 = CacheProvider(key2);
    EXPECT_EQ(provider1, m_cache.GetProvider(key1, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(key2, RulesetVariables(provider2->GetContext().GetRelatedRulesetVariables())).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, CachesMultipleProvidersWhenUnitSystemsAreDifferent)
    {
    ContentProviderKey key1("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Metric, *NavNodeKeyListContainer::Create(), nullptr);
    ContentProviderKey key2("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::UsCustomary, *NavNodeKeyListContainer::Create(), nullptr);
    SpecificationContentProviderP provider1 = CacheProvider(key1);
    SpecificationContentProviderP provider2 = CacheProvider(key2);
    EXPECT_EQ(provider1, m_cache.GetProvider(key1, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(key2, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, OverwritesProviderWhenKeysEqual)
    {
    SelectionInfoPtr selectionInfo = SelectionInfo::Create("selection source name", false);
    ContentProviderKey key("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), selectionInfo.get());
    CacheProvider(key);
    SpecificationContentProviderP provider2 = CacheProvider(key);
    EXPECT_EQ(provider2, m_cache.GetProvider(key, RulesetVariables(provider2->GetContext().GetRelatedRulesetVariables())).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, ClearsAllCache)
    {
    ECDb db;
    IConnectionPtr connection = new TestConnection(db);
    m_cache.CacheProvider(ContentProviderKey(connection->GetId(), "ruleset id 1", "display type 1", 0, ECPresentation::UnitSystem::UsCustomary, *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());
    m_cache.CacheProvider(ContentProviderKey(connection->GetId(), "ruleset id 2", "display type 2", 0, ECPresentation::UnitSystem::UsSurvey, *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());
    EXPECT_EQ(2, m_cache.GetProviders(*connection).size());
    m_cache.ClearCache();
    EXPECT_EQ(0, m_cache.GetProviders(*connection).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, ClearsCacheByConnection)
    {
    ECDb db1;
    IConnectionPtr connection1 = new TestConnection(db1);
    m_cache.CacheProvider(ContentProviderKey(connection1->GetId(), "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());

    ECDb db2;
    IConnectionPtr connection2 = new TestConnection(db2);
    m_cache.CacheProvider(ContentProviderKey(connection2->GetId(), "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());

    EXPECT_EQ(1, m_cache.GetProviders(*connection1).size());
    EXPECT_EQ(1, m_cache.GetProviders(*connection2).size());

    m_cache.ClearCache(*connection1);

    EXPECT_EQ(0, m_cache.GetProviders(*connection1).size());
    EXPECT_EQ(1, m_cache.GetProviders(*connection2).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, ClearsCacheByRulesetId)
    {
    m_cache.CacheProvider(ContentProviderKey("connection id", "ruleset id 1", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());
    m_cache.CacheProvider(ContentProviderKey("connection id", "ruleset id 2", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());

    EXPECT_EQ(1, m_cache.GetProviders("ruleset id 1", TEST_RELATED_SETTING).size());
    EXPECT_EQ(1, m_cache.GetProviders("ruleset id 2", TEST_RELATED_SETTING).size());

    m_cache.ClearCache("ruleset id 1");

    EXPECT_EQ(0, m_cache.GetProviders("ruleset id 1", TEST_RELATED_SETTING).size());
    EXPECT_EQ(1, m_cache.GetProviders("ruleset id 2", TEST_RELATED_SETTING).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, CachesProvidersDifferingJustBySelectionInfo)
    {
    // cache provider with no selection info
    ContentProviderKey keyNoSelectionInfo1("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr);
    SpecificationContentProviderP provider1 = CacheProvider(keyNoSelectionInfo1);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());

    // cache provider with selection info
    // the one with no selection info should still exist in the cache
    SelectionInfoPtr selectionInfo1 = SelectionInfo::Create("name1", true, 1);
    ContentProviderKey keyWithSelectionInfo("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), selectionInfo1.get());
    SpecificationContentProviderP provider2 = CacheProvider(keyWithSelectionInfo);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(keyWithSelectionInfo, RulesetVariables(provider2->GetContext().GetRelatedRulesetVariables())).get());

    // cache provider with selection info differing just by selection source name
    // it should replace the previous provider
    SelectionInfoPtr selectionInfo2 = SelectionInfo::Create("name2", true, 1);
    ContentProviderKey keyWithSelectionInfoWithDifferentName("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), selectionInfo2.get());
    SpecificationContentProviderP provider3 = CacheProvider(keyWithSelectionInfoWithDifferentName);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(keyWithSelectionInfo, RulesetVariables(provider2->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider3, m_cache.GetProvider(keyWithSelectionInfoWithDifferentName, RulesetVariables(provider3->GetContext().GetRelatedRulesetVariables())).get());

    // cache provider with selection info differing just by selection level
    // it should replace the previous provider
    SelectionInfoPtr selectionInfo3 = SelectionInfo::Create("name2", false, 1);
    ContentProviderKey keyWithSelectionInfoWithDifferentLevel("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), selectionInfo3.get());
    SpecificationContentProviderP provider4 = CacheProvider(keyWithSelectionInfoWithDifferentLevel);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(keyWithSelectionInfo, RulesetVariables(provider2->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider3, m_cache.GetProvider(keyWithSelectionInfoWithDifferentName, RulesetVariables(provider3->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider4, m_cache.GetProvider(keyWithSelectionInfoWithDifferentLevel, RulesetVariables(provider4->GetContext().GetRelatedRulesetVariables())).get());

    // cache provider with selection info differing just by timestamp
    // it should replace the previous provider
    SelectionInfoPtr selectionInfo4 = SelectionInfo::Create("name2", false, 2);
    ContentProviderKey keyWithSelectionInfoWithDifferentTimestamp("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), selectionInfo4.get());
    SpecificationContentProviderP provider5 = CacheProvider(keyWithSelectionInfoWithDifferentTimestamp);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(keyWithSelectionInfo, RulesetVariables(provider2->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider3, m_cache.GetProvider(keyWithSelectionInfoWithDifferentName, RulesetVariables(provider3->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider4, m_cache.GetProvider(keyWithSelectionInfoWithDifferentLevel, RulesetVariables(provider4->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider5, m_cache.GetProvider(keyWithSelectionInfoWithDifferentTimestamp, RulesetVariables(provider5->GetContext().GetRelatedRulesetVariables())).get());

    // cache another provider with no selection info
    // it should add this new provider in addition to keeping all cached ones
    INavNodeKeysContainerCPtr keys = NavNodeKeyListContainer::Create({NavNodeKey::Create("a", "", { "some_id" })});
    ContentProviderKey keyNoSelectionInfo2("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *keys, nullptr);
    SpecificationContentProviderP provider6 = CacheProvider(keyNoSelectionInfo2);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1, RulesetVariables(provider1->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(keyWithSelectionInfo, RulesetVariables(provider2->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider3, m_cache.GetProvider(keyWithSelectionInfoWithDifferentName, RulesetVariables(provider3->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider4, m_cache.GetProvider(keyWithSelectionInfoWithDifferentLevel, RulesetVariables(provider4->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider5, m_cache.GetProvider(keyWithSelectionInfoWithDifferentTimestamp, RulesetVariables(provider5->GetContext().GetRelatedRulesetVariables())).get());
    EXPECT_EQ(provider6, m_cache.GetProvider(keyNoSelectionInfo2, RulesetVariables(provider6->GetContext().GetRelatedRulesetVariables())).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, LimitsProviderVariationsCount)
    {
    ContentProviderKey key("connection id", "ruleset id", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr);
    bvector<SpecificationContentProviderP> providers = CacheProviderVariations(key, CONTENTCACHE_Provider_Variations_Limit);
    RulesetVariables oldestProviderVariables(providers[0]->GetContext().GetRelatedRulesetVariables());

    // cache one more provider variation
    RulesetVariables variables;
    variables.SetIntValue(TEST_RELATED_SETTING, CONTENTCACHE_Provider_Variations_Limit + 1);
    SpecificationContentProviderP newProvider = CacheProvider(key, variables);

    // check if new provider variation is in the cache
    EXPECT_EQ(newProvider, m_cache.GetProvider(key, variables).get());
    // check if oldest provider variation is removed
    EXPECT_TRUE(m_cache.GetProvider(key, oldestProviderVariables).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, RemovesOldestProviderWhenLimitIsExceeded)
    {
    auto providers = CacheProviders(CONTENTCACHE_Size);
    ContentProviderKey oldestProviderKey = providers[0].first;
    RulesetVariables oldestProviderVariables(providers[0].second->GetContext().GetRelatedRulesetVariables());

    // cache one more provider
    ContentProviderKey key("connection id", "different ruleset", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr);
    SpecificationContentProviderP newProvider = CacheProvider(key);

    // check if new provider is in the cache
    EXPECT_EQ(newProvider, m_cache.GetProvider(key, RulesetVariables(newProvider->GetContext().GetRelatedRulesetVariables())).get());
    // check if oldest provider is removed
    EXPECT_TRUE(m_cache.GetProvider(oldestProviderKey, oldestProviderVariables).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, UpdatesLastUsedTimeWhenProviderIsAccessed)
    {
    auto providers = CacheProviders(CONTENTCACHE_Size);
    ContentProviderKey oldestProviderKey = providers[0].first;
    RulesetVariables oldestProviderVariables(providers[0].second->GetContext().GetRelatedRulesetVariables());

    ContentProviderKey secondToOldestProviderKey = providers[1].first;
    RulesetVariables secondToOldestProviderVariables(providers[1].second->GetContext().GetRelatedRulesetVariables());

    // access oldest provider so it's laset used time should be updated
    EXPECT_EQ(providers[0].second, m_cache.GetProvider(oldestProviderKey, oldestProviderVariables).get());

    // cache one more provider
    ContentProviderKey key("connection id", "different ruleset", "display type", 0, ECPresentation::UnitSystem::Undefined, *NavNodeKeyListContainer::Create(), nullptr);
    SpecificationContentProviderP newProvider = CacheProvider(key);

    // check if new provider is in the cache
    EXPECT_EQ(newProvider, m_cache.GetProvider(key, RulesetVariables(newProvider->GetContext().GetRelatedRulesetVariables())).get());
    // check if oldest provider is in the cache
    EXPECT_EQ(providers[0].second, m_cache.GetProvider(oldestProviderKey, oldestProviderVariables).get());
    // check if second oldest provider was removed
    EXPECT_TRUE(m_cache.GetProvider(secondToOldestProviderKey, secondToOldestProviderVariables).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, RemovesOldestProviderWhenNewVariationIsCachedForSomeProvider)
    {
    auto providers = CacheProviders(CONTENTCACHE_Size);
    ContentProviderKey oldestProviderKey = providers[0].first;
    RulesetVariables oldestProviderVariables(providers[0].second->GetContext().GetRelatedRulesetVariables());

    ContentProviderKey newestProviderKey = providers.back().first;

    // cache one more variation for newestProvider
    RulesetVariables variables;
    variables.SetIntValue(TEST_RELATED_SETTING, CONTENTCACHE_Size + 1);
    SpecificationContentProviderP newProvider = CacheProvider(newestProviderKey, variables);

    // check if new provider is in the cache
    EXPECT_EQ(newProvider, m_cache.GetProvider(newestProviderKey, variables).get());

    // check if oldest provider is removed
    EXPECT_TRUE(m_cache.GetProvider(oldestProviderKey, oldestProviderVariables).IsNull());
    }
