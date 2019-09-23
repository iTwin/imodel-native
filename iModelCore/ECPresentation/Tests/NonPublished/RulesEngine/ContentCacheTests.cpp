/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/ContentCache.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define TEST_RELATED_SETTING "test related setting"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2018
+===============+===============+===============+===============+===============+======*/
struct ContentCacheTests : ECPresentationTest
    {
    TestNodeLocater m_nodesLocater;
    TestCategorySupplier m_categorySupplier;
    TestUserSettings m_userSettings;
    ECExpressionsCache m_ecexpressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    PolymorphicallyRelatedClassesCache m_polymorphicallyRelatedClassesCache;
    TestNodesFactory m_nodesFactory;

    ContentCache m_cache;

    ContentCacheTests()
        : m_nodesFactory("test"), m_categorySupplier(ContentDescriptor::Category("a", "b", "c", 0))
        {}

    SpecificationContentProviderPtr CreateProvider()
        {
        PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
        ContentProviderContextPtr context = ContentProviderContext::Create(*ruleset, "locale", "", 0, *NavNodeKeyListContainer::Create(),
            m_nodesLocater, m_categorySupplier, m_userSettings, m_ecexpressionsCache, m_relatedPathsCache,
            m_polymorphicallyRelatedClassesCache, m_nodesFactory, nullptr);
        context->GetUsedSettingsListener().OnUserSettingUsed(TEST_RELATED_SETTING);
        return SpecificationContentProvider::Create(*context, ContentRuleInstanceKeysList());
        }

    SpecificationContentProviderP CacheProvider(ContentProviderKey& key)
        {
        SpecificationContentProviderPtr provider = CreateProvider();
        m_cache.CacheProvider(key, *provider);
        return provider.get();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, CachesProvider)
    {
    SelectionInfoPtr selectionInfo = SelectionInfo::Create("selection source name", false);
    ContentProviderKey key("connection id", "ruleset id", "display type", 0, "", *NavNodeKeyListContainer::Create(), selectionInfo.get());
    SpecificationContentProviderP provider = CacheProvider(key);
    EXPECT_EQ(provider, m_cache.GetProvider(key).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, CachesMultipleProvidersWithDifferentKeys)
    {
    ContentProviderKey key1("connection id 1", "ruleset id 1", "display type 1", 0, "locale 1", *NavNodeKeyListContainer::Create(), nullptr);
    ContentProviderKey key2("connection id 2", "ruleset id 2", "display type 2", 1, "locale 2", *NavNodeKeyListContainer::Create(), nullptr);
    SpecificationContentProviderP provider1 = CacheProvider(key1);
    SpecificationContentProviderP provider2 = CacheProvider(key2);
    EXPECT_EQ(provider1, m_cache.GetProvider(key1).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(key2).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, CachesMultipleProvidersWhenLocalesAreDifferent)
    {
    ContentProviderKey key1("connection id", "ruleset id", "display type", 0, "a", *NavNodeKeyListContainer::Create(), nullptr);
    ContentProviderKey key2("connection id", "ruleset id", "display type", 0, "b", *NavNodeKeyListContainer::Create(), nullptr);
    SpecificationContentProviderP provider1 = CacheProvider(key1);
    SpecificationContentProviderP provider2 = CacheProvider(key2);
    EXPECT_EQ(provider1, m_cache.GetProvider(key1).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(key2).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, OverwritesProviderWhenKeysEqual)
    {
    SelectionInfoPtr selectionInfo = SelectionInfo::Create("selection source name", false);
    ContentProviderKey key("connection id", "ruleset id", "display type", 0, "", *NavNodeKeyListContainer::Create(), selectionInfo.get());
    CacheProvider(key);
    SpecificationContentProviderP provider2 = CacheProvider(key);
    EXPECT_EQ(provider2, m_cache.GetProvider(key).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, ClearsAllCache)
    {
    ECDb db;
    IConnectionPtr connection = new TestConnection(db);
    m_cache.CacheProvider(ContentProviderKey(connection->GetId(), "ruleset id 1", "display type 1", 0, "locale 1", *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());
    m_cache.CacheProvider(ContentProviderKey(connection->GetId(), "ruleset id 2", "display type 2", 0, "locale 2", *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());
    EXPECT_EQ(2, m_cache.GetProviders(*connection).size());
    m_cache.ClearCache();
    EXPECT_EQ(0, m_cache.GetProviders(*connection).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, ClearsCacheByConnection)
    {
    ECDb db1;
    IConnectionPtr connection1 = new TestConnection(db1);
    m_cache.CacheProvider(ContentProviderKey(connection1->GetId(), "ruleset id", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());

    ECDb db2;
    IConnectionPtr connection2 = new TestConnection(db2);
    m_cache.CacheProvider(ContentProviderKey(connection2->GetId(), "ruleset id", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());

    EXPECT_EQ(1, m_cache.GetProviders(*connection1).size());
    EXPECT_EQ(1, m_cache.GetProviders(*connection2).size());

    m_cache.ClearCache(*connection1);

    EXPECT_EQ(0, m_cache.GetProviders(*connection1).size());
    EXPECT_EQ(1, m_cache.GetProviders(*connection2).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, ClearsCacheByRulesetId)
    {
    m_cache.CacheProvider(ContentProviderKey("connection id", "ruleset id 1", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());
    m_cache.CacheProvider(ContentProviderKey("connection id", "ruleset id 2", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), nullptr), *CreateProvider());

    EXPECT_EQ(1, m_cache.GetProviders("ruleset id 1", TEST_RELATED_SETTING).size());
    EXPECT_EQ(1, m_cache.GetProviders("ruleset id 2", TEST_RELATED_SETTING).size());

    m_cache.ClearCache("ruleset id 1");

    EXPECT_EQ(0, m_cache.GetProviders("ruleset id 1", TEST_RELATED_SETTING).size());
    EXPECT_EQ(1, m_cache.GetProviders("ruleset id 2", TEST_RELATED_SETTING).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentCacheTests, ReplacesProvidersDifferingJustBySelectionInfo)
    {
    // cache provider with no selection info
    ContentProviderKey keyNoSelectionInfo1("connection id", "ruleset id", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), nullptr);
    SpecificationContentProviderP provider1 = CacheProvider(keyNoSelectionInfo1);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1).get());

    // cache provider with selection info
    // the one with no selection info should still exist in the cache
    SelectionInfoPtr selectionInfo1 = SelectionInfo::Create("name1", true, 1);
    ContentProviderKey keyWithSelectionInfo("connection id", "ruleset id", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), selectionInfo1.get());
    SpecificationContentProviderP provider2 = CacheProvider(keyWithSelectionInfo);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1).get());
    EXPECT_EQ(provider2, m_cache.GetProvider(keyWithSelectionInfo).get());

    // cache provider with selection info differing just by selection source name
    // it should replace the previous provider
    SelectionInfoPtr selectionInfo2 = SelectionInfo::Create("name2", true, 1);
    ContentProviderKey keyWithSelectionInfoWithDifferentName("connection id", "ruleset id", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), selectionInfo2.get());
    SpecificationContentProviderP provider3 = CacheProvider(keyWithSelectionInfoWithDifferentName);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1).get());
    EXPECT_TRUE(m_cache.GetProvider(keyWithSelectionInfo).IsNull());
    EXPECT_EQ(provider3, m_cache.GetProvider(keyWithSelectionInfoWithDifferentName).get());

    // cache provider with selection info differing just by selection level
    // it should replace the previous provider
    SelectionInfoPtr selectionInfo3 = SelectionInfo::Create("name2", false, 1);
    ContentProviderKey keyWithSelectionInfoWithDifferentLevel("connection id", "ruleset id", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), selectionInfo3.get());
    SpecificationContentProviderP provider4 = CacheProvider(keyWithSelectionInfoWithDifferentLevel);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1).get());
    EXPECT_TRUE(m_cache.GetProvider(keyWithSelectionInfoWithDifferentName).IsNull());
    EXPECT_EQ(provider4, m_cache.GetProvider(keyWithSelectionInfoWithDifferentLevel).get());

    // cache provider with selection info differing just by timestamp
    // it should replace the previous provider
    SelectionInfoPtr selectionInfo4 = SelectionInfo::Create("name2", false, 2);
    ContentProviderKey keyWithSelectionInfoWithDifferentTimestamp("connection id", "ruleset id", "display type", 0, "locale", *NavNodeKeyListContainer::Create(), selectionInfo4.get());
    SpecificationContentProviderP provider5 = CacheProvider(keyWithSelectionInfoWithDifferentTimestamp);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1).get());
    EXPECT_TRUE(m_cache.GetProvider(keyWithSelectionInfoWithDifferentLevel).IsNull());
    EXPECT_EQ(provider5, m_cache.GetProvider(keyWithSelectionInfoWithDifferentTimestamp).get());

    // cache another provider with no selection info
    // it should add this new provider in addition to keeping all cached ones
    INavNodeKeysContainerCPtr keys = NavNodeKeyListContainer::Create({NavNodeKey::Create("a", bvector<Utf8String>())});
    ContentProviderKey keyNoSelectionInfo2("connection id", "ruleset id", "display type", 0, "locale", *keys, nullptr);
    SpecificationContentProviderP provider6 = CacheProvider(keyNoSelectionInfo2);
    EXPECT_EQ(provider1, m_cache.GetProvider(keyNoSelectionInfo1).get());
    EXPECT_EQ(provider5, m_cache.GetProvider(keyWithSelectionInfoWithDifferentTimestamp).get());
    EXPECT_EQ(provider6, m_cache.GetProvider(keyNoSelectionInfo2).get());
    }
