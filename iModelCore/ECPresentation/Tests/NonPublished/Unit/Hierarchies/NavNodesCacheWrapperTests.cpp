/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavNodesCacheTests.h"
#include "../../../../Source/Hierarchies/NavNodesCacheWrapper.h"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheWrapperTests : NodesCacheTests
    {
    std::shared_ptr<NodesCache> _CreateNodesCache(IConnectionR connection, BeFileNameCR tempDirectory) override;
    std::shared_ptr<NodesCacheWrapper> CreateWrapper(NodesCache& cache, BeGuidCR parentId = BeGuid());
    std::shared_ptr<NodesCacheWrapper> CreateWrapper(BeGuidCR parentId = BeGuid());

    bpair<HierarchyLevelIdentifier, DataSourceIdentifier> GetDataSourceInfo(IHierarchyCacheCR cache, Utf8CP connectionId, Utf8CP rulesetId, BeGuidCR virtualParentId, RulesetVariables const& variables = RulesetVariables(), Utf8StringCR instanceFilter = {})
        {
        BeGuid hlId = cache.FindHierarchyLevelId(connectionId, rulesetId, virtualParentId, BeGuid());
        EXPECT_TRUE(hlId.IsValid());
        HierarchyLevelIdentifier hlIdentifier(hlId, connectionId, rulesetId, virtualParentId, BeGuid());
        DataSourceIdentifier dsIdentifier = cache.FindDataSource(DataSourceIdentifier(hlId, { 0 }, instanceFilter), variables).GetIdentifier();
        EXPECT_TRUE(dsIdentifier.IsValid());
        return bpair<HierarchyLevelIdentifier, DataSourceIdentifier>(hlIdentifier, dsIdentifier);
        }

    NavNodePtr CacheNewNode(IHierarchyCacheR cache, DataSourceIdentifier const& dsIdentifier, Utf8CP label, NavNodeKeyCP parentKey = nullptr)
        {
        TestNodesFactory factory(*m_connection, "specificationIdentifier", TEST_RULESET_ID);
        auto node = factory.CreateCustomNode(parentKey, label, "description", "imageid", "TestType");
        cache.Cache(*node, dsIdentifier, 0, NodeVisibility::Visible);
        return node;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<NodesCacheWrapper> NodesCacheWrapperTests::CreateWrapper(NodesCache& cache, BeGuidCR parentId)
    {
    return std::make_unique<NodesCacheWrapper>(cache, parentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<NodesCacheWrapper> NodesCacheWrapperTests::CreateWrapper(BeGuidCR parentId)
    {
    return CreateWrapper(*m_cache, parentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<NodesCache> NodesCacheWrapperTests::_CreateNodesCache(IConnectionR connection, BeFileNameCR tempDirectory)
    {
    return NodesCache::Create(connection, tempDirectory, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::HybridDisk, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, FindHierarchyLevelId_ReturnsHierarchyLevelFromPersistedCache)
    {
    // cache hierarchy level
    auto hlInfo = CacheHierarchyLevel(m_connection->GetId(), TEST_RULESET_ID);

    auto wrapper = CreateWrapper();

    BeGuid cachedLevelId = wrapper->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid());
    EXPECT_TRUE(cachedLevelId.IsValid());
    EXPECT_EQ(hlInfo.GetId(), cachedLevelId);
    EXPECT_TRUE(nullptr == wrapper->GetMemoryCache());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, FindDataSource_ReturnsDataSourceFromPersistedCache)
    {
    // cache hierarchy level
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto node = CacheNewNode(*m_cache, rootInfo.second, "RootNode");

    auto wrapper = CreateWrapper();

    DataSourceInfo cacheDataSource = wrapper->FindDataSource(rootInfo.second, RulesetVariables(), 0);
    EXPECT_TRUE(cacheDataSource.GetIdentifier().IsValid());
    EXPECT_EQ(rootInfo.second.GetId(), cacheDataSource.GetIdentifier().GetId());
    EXPECT_TRUE(nullptr == wrapper->GetMemoryCache());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, ReturnsRootDataSourceFromPersistedCache)
    {
    // cache hierarchy level
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);

    auto wrapper = CreateWrapper();

    NavNodesProviderPtr cacheDataSource = wrapper->GetCombinedHierarchyLevel(*CreateContext(wrapper, rootInfo.first.GetCombined()), rootInfo.first.GetCombined());
    EXPECT_TRUE(cacheDataSource.IsValid());

    cacheDataSource = wrapper->GetHierarchyLevel(*CreateContext(wrapper, rootInfo.first.GetCombined()), rootInfo.first.GetId());
    EXPECT_TRUE(cacheDataSource.IsValid());

    auto iterator = wrapper->GetCachedDirectNodesIterator(*CreateContext(wrapper, rootInfo.first.GetCombined()), rootInfo.second);
    EXPECT_TRUE(cacheDataSource.IsValid());

    EXPECT_TRUE(nullptr == wrapper->GetMemoryCache());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, GetNode_ReturnsNodeFromPersistedCache)
    {
    // cache hierarchy level
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto node = CacheNewNode(*m_cache, rootInfo.second, "RootNode");

    auto wrapper = CreateWrapper();

    NavNodePtr cachedNode = wrapper->GetNode(node->GetNodeId());
    EXPECT_TRUE(cachedNode.IsValid());
    EXPECT_TRUE(cachedNode->Equals(*node));
    EXPECT_TRUE(nullptr == wrapper->GetMemoryCache());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, CacheHierarchyLevelAndPersist)
    {
    auto wrapper = CreateWrapper();

    auto hlInfo = CacheHierarchyLevel(*wrapper, m_connection->GetId(), TEST_RULESET_ID);

    BeGuid cachedHierarchyLevelId = m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid());
    EXPECT_FALSE(cachedHierarchyLevelId.IsValid());

    cachedHierarchyLevelId = wrapper->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid());
    EXPECT_TRUE(cachedHierarchyLevelId.IsValid());

    wrapper = nullptr;

    cachedHierarchyLevelId = m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid());
    EXPECT_TRUE(cachedHierarchyLevelId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, CacheDataSourceAndPersist)
    {
    CacheHierarchyLevel(*m_cache, m_connection->GetId(), TEST_RULESET_ID);

    auto wrapper = CreateWrapper();

    auto rootInfo = CacheDataSource(*wrapper, m_connection->GetId(), TEST_RULESET_ID);

    BeGuid cachedHierarchyLevelId = m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid());
    EXPECT_TRUE(cachedHierarchyLevelId.IsValid());

    DataSourceInfo cachedDataSource = m_cache->FindDataSource(rootInfo.second, RulesetVariables());
    EXPECT_FALSE(cachedDataSource.GetIdentifier().IsValid());

    cachedHierarchyLevelId = wrapper->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid());
    EXPECT_TRUE(cachedHierarchyLevelId.IsValid());
    cachedDataSource = wrapper->FindDataSource(rootInfo.second, RulesetVariables());
    EXPECT_TRUE(cachedDataSource.GetIdentifier().IsValid());

    wrapper = nullptr;

    cachedDataSource = m_cache->FindDataSource(rootInfo.second, RulesetVariables());
    EXPECT_TRUE(cachedDataSource.GetIdentifier().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, CacheNodeAndPersist)
    {
    auto rootInfo = CacheDataSource(*m_cache, m_connection->GetId(), TEST_RULESET_ID);

    auto wrapper = CreateWrapper();

    auto node = CacheNewNode(*wrapper, rootInfo.second, "RootNode");

    DataSourceInfo cachedDataSource = m_cache->FindDataSource(rootInfo.second, RulesetVariables());
    EXPECT_TRUE(cachedDataSource.GetIdentifier().IsValid());
    NavNodePtr cachedNode = m_cache->GetNode(node->GetNodeId());
    EXPECT_FALSE(cachedNode.IsValid());

    cachedNode = wrapper->GetNode(node->GetNodeId());
    EXPECT_TRUE(cachedNode.IsValid());

    wrapper = nullptr;

    cachedNode = m_cache->GetNode(node->GetNodeId());
    EXPECT_TRUE(cachedNode.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, UpdateDataSourceAndPersist)
    {
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);

    auto wrapper = CreateWrapper();

    DataSourceInfo cachedDataSource = wrapper->FindDataSource(rootInfo.second, RulesetVariables(), DataSourceInfo::Parts::PARTS_All);
    cachedDataSource.SetNodeTypes("UpdatedNodesType");
    cachedDataSource.SetSpecificationHash("UpdatedSpecificationHash");
    cachedDataSource.SetRelatedVariables(RulesetVariables({ RulesetVariableEntry("var_id", "var_value") }));
    bmap<ECClassId, bool> relatedClasses;
    relatedClasses.Insert(ECClassId((uint64_t)123), true);
    cachedDataSource.SetRelatedClasses(relatedClasses);

    wrapper->Update(cachedDataSource, DataSourceInfo::Parts::PARTS_All);

    DataSourceInfo oldDataSource = m_cache->FindDataSource(rootInfo.second, RulesetVariables(), DataSourceInfo::Parts::PARTS_All);
    EXPECT_STRNE(oldDataSource.GetNodeTypes().c_str(), cachedDataSource.GetNodeTypes().c_str());
    EXPECT_STRNE(oldDataSource.GetSpecificationHash().c_str(), cachedDataSource.GetSpecificationHash().c_str());
    EXPECT_FALSE(oldDataSource.GetRelatedVariables() == cachedDataSource.GetRelatedVariables());
    EXPECT_NE(oldDataSource.GetRelatedClasses(), cachedDataSource.GetRelatedClasses());

    wrapper = nullptr;

    DataSourceInfo updatedDataSource = m_cache->FindDataSource(rootInfo.second, RulesetVariables({ RulesetVariableEntry("var_id", "var_value") }), DataSourceInfo::Parts::PARTS_All);
    EXPECT_STREQ(updatedDataSource.GetNodeTypes().c_str(), cachedDataSource.GetNodeTypes().c_str());
    EXPECT_STREQ(updatedDataSource.GetSpecificationHash().c_str(), cachedDataSource.GetSpecificationHash().c_str());
    EXPECT_TRUE(updatedDataSource.GetRelatedVariables() == cachedDataSource.GetRelatedVariables());
    EXPECT_EQ(updatedDataSource.GetRelatedClasses(), cachedDataSource.GetRelatedClasses());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, CacheAdditionalDataSourceForRootHierarchyLevel)
    {
    RulesetVariables variables({ RulesetVariableEntry("var_id", "var_value") });
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, variables);

    auto wrapper = CreateWrapper();

    RulesetVariables otherVariables({ RulesetVariableEntry("var_id", "other_value") });
    auto otherDataSource = CacheDataSource(*wrapper, m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, otherVariables);

    DataSourceInfo cachedDataSource = m_cache->FindDataSource(otherDataSource.second, otherVariables);
    EXPECT_FALSE(cachedDataSource.GetIdentifier().IsValid());

    cachedDataSource = wrapper->FindDataSource(otherDataSource.second, otherVariables);
    EXPECT_TRUE(cachedDataSource.GetIdentifier().IsValid());

    wrapper = nullptr;

    DataSourceInfo firstDataSource = m_cache->FindDataSource(rootInfo.second, variables);
    EXPECT_TRUE(firstDataSource.GetIdentifier().IsValid());
    DataSourceInfo secondDataSource = m_cache->FindDataSource(otherDataSource.second, otherVariables);
    EXPECT_TRUE(secondDataSource.GetIdentifier().IsValid());
    EXPECT_NE(firstDataSource.GetIdentifier(), secondDataSource.GetIdentifier());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, CacheNodeForPersistedRootDataSourceWithNodes)
    {
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto node = CacheNewNode(*m_cache, rootInfo.second, "RootNode1");

    auto wrapper = CreateWrapper();

    CacheNewNode(*wrapper, rootInfo.second, "RootNode2", {});

    auto iterator = m_cache->GetCachedDirectNodesIterator(*CreateContext(rootInfo.first.GetCombined()), rootInfo.second);
    ASSERT_TRUE(nullptr != iterator);
    EXPECT_EQ(1, iterator->NodesCount());

    wrapper = nullptr;

    iterator = m_cache->GetCachedDirectNodesIterator(*CreateContext(rootInfo.first.GetCombined()), rootInfo.second);
    ASSERT_TRUE(nullptr != iterator);
    EXPECT_EQ(2, iterator->NodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheWrapperTests, CacheMultiLevelHierarchyAndPersist)
    {
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto node = CacheNewNode(*m_cache, rootInfo.second, "RootNode", {});

    auto wrapper = CreateWrapper();

    auto childInfo = CacheDataSource(*wrapper, m_connection->GetId(), TEST_RULESET_ID, node->GetNodeId());
    auto childNode = CacheNewNode(*wrapper, childInfo.second, "ChildNode", node->GetKey().get());

    auto grandchildInfo = CacheDataSource(*wrapper, m_connection->GetId(), TEST_RULESET_ID, childNode->GetNodeId());
    auto grandchildNode = CacheNewNode(*wrapper, grandchildInfo.second, "GrandchildNode", childNode->GetKey().get());

    DataSourceInfo childDataSourceInfo = m_cache->FindDataSource(childInfo.second, RulesetVariables());
    EXPECT_FALSE(childDataSourceInfo.GetIdentifier().IsValid());
    DataSourceInfo grandchildDataSourceInfo = m_cache->FindDataSource(grandchildInfo.second, RulesetVariables());
    EXPECT_FALSE(grandchildDataSourceInfo.GetIdentifier().IsValid());

    wrapper = nullptr;

    auto cachedChildInfo = GetDataSourceInfo(*m_cache, m_connection->GetId().c_str(), TEST_RULESET_ID, node->GetNodeId());
    auto childIterator = m_cache->GetCachedDirectNodesIterator(*CreateContext(cachedChildInfo.first.GetCombined()), cachedChildInfo.second);
    ASSERT_TRUE(nullptr != childIterator);
    ASSERT_EQ(1, childIterator->NodesCount());
    EXPECT_TRUE(childIterator->NextNode()->Equals(*childNode));
    EXPECT_TRUE(childIterator->NextNode().IsNull());

    auto cachedGrandchildInfo = GetDataSourceInfo(*m_cache, m_connection->GetId().c_str(), TEST_RULESET_ID, childNode->GetNodeId());
    auto grandchildIterator = m_cache->GetCachedDirectNodesIterator(*CreateContext(cachedGrandchildInfo.first.GetCombined()), cachedGrandchildInfo.second);
    ASSERT_TRUE(nullptr != grandchildIterator);
    ASSERT_EQ(1, grandchildIterator->NodesCount());
    EXPECT_TRUE(grandchildIterator->NextNode()->Equals(*grandchildNode));
    EXPECT_TRUE(grandchildIterator->NextNode().IsNull());
    }
