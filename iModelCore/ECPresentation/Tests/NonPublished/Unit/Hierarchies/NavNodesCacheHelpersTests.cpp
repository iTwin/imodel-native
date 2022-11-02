/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavNodesCacheTests.h"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheHelpersTests : NodesCacheTests {};

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHelpersTests, HierarchyLevelExists_ReturnsCorrectValues)
    {
    // cache hierarchy level
    auto hlInfo = CacheHierarchyLevel(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());

    // returns true for existing hierarchy level
    EXPECT_TRUE(NodesCacheHelpers::HierarchyLevelExists(m_cache->GetDb(), hlInfo.GetId()));
    // return false for non-existing hierarchy level
    EXPECT_FALSE(NodesCacheHelpers::HierarchyLevelExists(m_cache->GetDb(), BeGuid(true)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHelpersTests, DataSourceExists_ReturnsCorrectValues)
    {
    // cache data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");

    // returns true for existing data source
    EXPECT_TRUE(NodesCacheHelpers::DataSourceExists(m_cache->GetDb(), rootInfo.second.GetId()));
    // return false for non-existing data source
    EXPECT_FALSE(NodesCacheHelpers::DataSourceExists(m_cache->GetDb(), BeGuid(true)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHelpersTests, RulesetExists_ReturnsCorrectValues)
    {
    // returns true for existing data source
    EXPECT_TRUE(NodesCacheHelpers::RulesetExists(m_cache->GetDb(), "ruleset_id"));
    // return false for non-existing data source
    EXPECT_FALSE(NodesCacheHelpers::RulesetExists(m_cache->GetDb(), "invalid_ruleset_id"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHelpersTests, NodeExists_ReturnsCorrectValues)
    {
    // cache data source and node
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(rootInfo, 1);

    // returns true for existing node
    EXPECT_TRUE(NodesCacheHelpers::NodeExists(m_cache->GetDb(), nodes[0]->GetNodeId()));
    // return false for non-existing node
    EXPECT_FALSE(NodesCacheHelpers::NodeExists(m_cache->GetDb(), BeGuid(true)));
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHelpersTests, GetNodePathHash_ReturnsCorrectHash)
    {
    // cache data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(rootInfo, 1);

    Utf8String cachedPathHash = NodesCacheHelpers::GetNodePathHash(m_cache->GetDb(), nodes[0]->GetNodeId());
    Utf8String nodePathHash = NavNodesHelper::NodeKeyHashPathToString(*nodes[0]->GetKey());
    EXPECT_STREQ(cachedPathHash.c_str(), nodePathHash.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHelpersTests, HierarchyLevelHasDataSources_ReturnsCorrectValues)
    {
    // cache root hierarchy level without data source
    auto hlInfo = CacheHierarchyLevel(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());
    EXPECT_FALSE(NodesCacheHelpers::HierarchyLevelHasDataSources(m_cache->GetDb(), hlInfo.GetId()));

    // cache data source for root hierarchyLevel
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    EXPECT_TRUE(NodesCacheHelpers::HierarchyLevelHasDataSources(m_cache->GetDb(), hlInfo.GetId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHelpersTests, GetChildHierarchyLevelIds_ReturnsCorrectValues)
    {
    EXPECT_TRUE(NodesCacheHelpers::GetChildHierarchyLevelIds(m_cache->GetDb(), true, BeGuid(), nullptr).empty());

    // cache root hierarchy level
    auto hlInfo = CacheHierarchyLevel(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());
    auto hierarchyLevels = NodesCacheHelpers::GetChildHierarchyLevelIds(m_cache->GetDb(), true, BeGuid(), nullptr);
    ASSERT_EQ(hierarchyLevels.size(), 1);
    EXPECT_EQ(hierarchyLevels[0], hlInfo.GetId());

    // cache root hierarchy level with different ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2"));
    auto hlInfoDifferentRuleset = CacheHierarchyLevel(m_connection->GetId(), "ruleset_id2", BeGuid(), BeGuid());
    hierarchyLevels = NodesCacheHelpers::GetChildHierarchyLevelIds(m_cache->GetDb(), true, BeGuid(), "ruleset_id2");
    ASSERT_EQ(hierarchyLevels.size(), 1);
    EXPECT_EQ(hierarchyLevels[0], hlInfoDifferentRuleset.GetId());

    // cache child hierarchy level
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(rootInfo, 1);
    BeGuid nodeId = nodes[0]->GetNodeId();
    auto childHlInfo = CacheHierarchyLevel(m_connection->GetId(), "ruleset_id", nodeId, nodeId);
    hierarchyLevels = NodesCacheHelpers::GetChildHierarchyLevelIds(m_cache->GetDb(), true, nodeId, nullptr);
    ASSERT_EQ(hierarchyLevels.size(), 1);
    EXPECT_EQ(hierarchyLevels[0], childHlInfo.GetId());

    // get all root hierarchy levels
    hierarchyLevels = NodesCacheHelpers::GetChildHierarchyLevelIds(m_cache->GetDb(), true, BeGuid(), nullptr);
    EXPECT_EQ(hierarchyLevels.size(), 2);
    }
