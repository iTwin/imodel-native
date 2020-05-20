/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodesCache.h"
#include "ECDbTestProject.h"
#include "TestNavNode.h"
#include "TestHelpers.h"
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2017
+===============+===============+===============+===============+===============+======*/
struct NodesCacheTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;

    TestConnectionManager m_connections;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesProviderContextFactory m_nodesProviderContextFactory;
    IConnectionPtr m_connection;
    std::unique_ptr<NodesCache> m_cache;

    NodesCacheTests() : m_nodesProviderContextFactory(m_connections) {}

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        if (!s_project->GetECDb().IsDbOpen())
            s_project->Open("NodesCacheTests", Db::OpenParams(Db::OpenMode::ReadWrite));

        ReCreateNodesCache();
        }

    virtual void TearDown() override
        {
        m_connections.ClearConnections();
        m_cache = nullptr;
        s_project->GetECDb().AbandonChanges();
        }

    static ECDbR GetDb() {return s_project->GetECDb();}

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("NodesCacheTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        s_project->GetECDb().CloseDb();
        DELETE_AND_CLEAR(s_project);
        }

    void ReCreateProject()
        {
        if (nullptr != s_project)
            {
            m_connections.NotifyConnectionClosed(*m_connection);
            DELETE_AND_CLEAR(s_project);
            }

        s_project = new ECDbTestProject();
        s_project->Open("NodesCacheTests", Db::OpenParams(Db::OpenMode::ReadWrite));
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        }

    virtual std::unique_ptr<NodesCache> _CreateNodesCache(IConnectionR connection, BeFileNameCR tempDirectory)
        {
        return NodesCache::Create(connection, tempDirectory, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Memory, true);
        }

    void ReCreateNodesCache()
        {
        m_cache = nullptr;
        BeFileName temporaryDirectory;
        BeTest::GetHost().GetTempDir(temporaryDirectory);
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        m_cache = _CreateNodesCache(*m_connection, temporaryDirectory);
        m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id", 1, 0, false, "", "", "", false));
        m_nodesProviderContextFactory.SetNodesCache(m_cache.get());
        }

    NavNodesProviderContextPtr CreateContext(Utf8StringCR rulesetId, Utf8StringCR locale, uint64_t parentId, RulesetVariables vars = RulesetVariables()) const
        {
        return m_nodesProviderContextFactory.Create(*m_connection, rulesetId.c_str(), locale.c_str(), (0 != parentId) ? &parentId : nullptr, nullptr, -1, vars);
        }

    NavNodesProviderContextPtr CreateContext(CombinedHierarchyLevelIdentifier const& id, RulesetVariables vars = RulesetVariables()) const
        {
        return m_nodesProviderContextFactory.Create(*m_connection, id.GetRulesetId().c_str(), id.GetLocale().c_str(), id.GetPhysicalParentNodeId(), nullptr, -1, vars);
        }

    void InitNode(JsonNavNodeR, HierarchyLevelIdentifier const&);
    void FillWithNodes(bpair<HierarchyLevelIdentifier, DataSourceIdentifier> const&, bvector<JsonNavNodePtr> const&, bool createChildDataSources = false, bool areVirtual = false);
    bvector<JsonNavNodePtr> FillWithNodes(bpair<HierarchyLevelIdentifier, DataSourceIdentifier> const&, size_t count, bool createChildDataSources = false, bool areVirtual = false);
    bpair<HierarchyLevelIdentifier, DataSourceIdentifier> CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, Utf8StringCR locale, uint64_t virtualParentId = 0, bool finalize = true, RulesetVariables const& variables = RulesetVariables());
    bpair<HierarchyLevelIdentifier, DataSourceIdentifier> CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, Utf8StringCR locale, uint64_t virtualParentId, uint64_t physicalParentId, bool finalize = true, RulesetVariables const& variables = RulesetVariables());
    };
ECDbTestProject* NodesCacheTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::InitNode(JsonNavNodeR node, HierarchyLevelIdentifier const& info)
    {
    if (nullptr != info.GetPhysicalParentNodeId())
        node.SetParentNodeId(*info.GetPhysicalParentNodeId());

    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(info.GetConnectionId());
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    if (nullptr != info.GetVirtualParentNodeId())
        extendedData.SetVirtualParentId(*info.GetVirtualParentNodeId());

    JsonNavNodeCPtr parentNode = info.GetVirtualParentNodeId() ? m_cache->GetNode(*info.GetVirtualParentNodeId()) : nullptr;
    node.SetNodeKey(*NavNodesHelper::CreateNodeKey(*m_connection, node, parentNode.IsValid() ? parentNode->GetKey().get() : nullptr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::FillWithNodes(bpair<HierarchyLevelIdentifier, DataSourceIdentifier> const& info, bvector<JsonNavNodePtr> const& nodes, bool createChildDataSources, bool areVirtual)
    {
    for (size_t i = 0; i < nodes.size(); ++i)
        {
        InitNode(*nodes[i], info.first);
        m_cache->Cache(*nodes[i], info.second, (uint64_t)i, areVirtual ? NodeVisibility::Virtual : NodeVisibility::Visible);

        if (createChildDataSources)
            CacheDataSource(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), nodes[i]->GetNodeId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<JsonNavNodePtr> NodesCacheTests::FillWithNodes(bpair<HierarchyLevelIdentifier, DataSourceIdentifier> const& info, size_t count, bool createChildDataSources, bool areVirtual)
    {
    bvector<JsonNavNodePtr> nodes;
    for (size_t i = 0; i < count; ++i)
        {
        TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", std::to_string(i).c_str(), "descr");
        nodes.push_back(node);
        }
    FillWithNodes(info, nodes, createChildDataSources, areVirtual);
    return nodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, Utf8StringCR locale, uint64_t virtualParentId, uint64_t physicalParentId, bool finalize, RulesetVariables const& variables)
    {
    HierarchyLevelIdentifier hlInfo = m_cache->FindHierarchyLevel(connectionId.c_str(), rulesetId.c_str(), locale.c_str(), &virtualParentId);
    if (!hlInfo.IsValid())
        {
        hlInfo = HierarchyLevelIdentifier(connectionId, rulesetId, locale, physicalParentId, virtualParentId);
        m_cache->Cache(hlInfo);
        }
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceInfo dsInfo(DataSourceIdentifier(hlInfo.GetId(), { 0 }), variables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Cache(dsInfo);
    EXPECT_TRUE(dsInfo.GetIdentifier().IsValid());

    if (finalize)
        m_cache->FinalizeInitialization(dsInfo.GetIdentifier());

    return bpair<HierarchyLevelIdentifier, DataSourceIdentifier>(hlInfo, dsInfo.GetIdentifier());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, Utf8StringCR locale, uint64_t parentId, bool finalize, RulesetVariables const& variables)
    {
    return CacheDataSource(connectionId, rulesetId, locale, parentId, parentId, finalize, variables);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_Full)
    {
    // create a different ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));

    // cache
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    CacheDataSource(m_connection->GetId(), "ruleset_id2", "locale", 0);
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale2", 0);

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale2", nullptr).IsValid());

    // fully clear the cache
    m_cache->Clear();

    // verify everything's gone
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", "locale", nullptr).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale2", nullptr).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByRulesetId)
    {
    // create a different ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));

    // cache
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    CacheDataSource(m_connection->GetId(), "ruleset_id2", "locale", 0);

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", "locale", nullptr).IsValid());

    // clear "ruleset_id" hierarchy from cache
    m_cache->Clear("ruleset_id");

    // verify only "ruleset_id" hierarchy was removed
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", "locale", nullptr).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesHierarchyLevel)
    {
    // cache data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");

    // verify it got cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());

    // attempt to remove with invalid ruleset id and verify it didn't get removed
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(info.first.GetConnectionId(),
        "invalid", info.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());

    // attempt to remove with invalid locale and verify it didn't get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(info.first.GetConnectionId(),
        info.first.GetRulesetId(), "invalid", 0));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());

    // attempt to remove with invalid parent node and verify it didn't get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(info.first.GetConnectionId(),
        info.first.GetRulesetId(), info.first.GetLocale(), 1));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());

    // attempt to remove with valid parameters and verify it did get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(info.first.GetConnectionId(),
        info.first.GetRulesetId(), info.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_FALSE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesChildHierarchyLevel)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodes[0]->GetNodeId());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext("ruleset_id", "locale", nodes[0]->GetNodeId()), childInfo.first).IsValid());

    // remove child data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(childInfo.first.GetConnectionId(),
        childInfo.first.GetRulesetId(), childInfo.first.GetLocale(), nodes[0]->GetNodeId()));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext("ruleset_id", "locale", nodes[0]->GetNodeId()), childInfo.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesChildNodesWhenParentHierarchyLevelIsRemoved)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);

    // verify node is cached
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    // remove child data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(rootInfo.first.GetConnectionId(),
        rootInfo.first.GetRulesetId(), rootInfo.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesChildDataSourcesWhenParentDataSourceIsRemoved)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodes[0]->GetNodeId());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext("ruleset_id", "locale", nodes[0]->GetNodeId()), childInfo.first).IsValid());

    // remove root data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(rootInfo.first.GetConnectionId(),
        rootInfo.first.GetRulesetId(), rootInfo.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext("ruleset_id", "locale", nodes[0]->GetNodeId()), childInfo.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakePhysical)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1, false, true);

    // cache some children
    auto virtualNodeChildrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodes[0]->GetNodeId(), (uint64_t)0);
    auto childNodes = FillWithNodes(virtualNodeChildrenInfo, 1, false, true);

    // cache some grand-children
    auto virtualNodeGrandChildrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", childNodes[0]->GetNodeId(), (uint64_t)0);
    auto grandChildNodes = FillWithNodes(virtualNodeGrandChildrenInfo, 1, false, true);

    // verify node is virtual
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // verify child and grandchild nodes' physical parent is not set (root)
    EXPECT_EQ(0, m_cache->GetNode(childNodes[0]->GetNodeId())->GetParentNodeId());
    EXPECT_EQ(0, m_cache->GetNode(grandChildNodes[0]->GetNodeId())->GetParentNodeId());

    // make physical
    m_cache->MakePhysical(*nodes[0]);

    // verify node is physical
    EXPECT_EQ(NodeVisibility::Visible, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // verify child and grandchild nodes' physical parent is now set
    EXPECT_EQ(nodes[0]->GetNodeId(), m_cache->GetNode(childNodes[0]->GetNodeId())->GetParentNodeId());
    EXPECT_EQ(nodes[0]->GetNodeId(), m_cache->GetNode(grandChildNodes[0]->GetNodeId())->GetParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakeVirtual)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1, false, false);

    // cache some children
    auto virtualNodeChildrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodes[0]->GetNodeId(), nodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(virtualNodeChildrenInfo, 1, false, false);

    // cache some grand-children
    auto virtualNodeGrandChildrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", childNodes[0]->GetNodeId(), nodes[0]->GetNodeId());
    auto grandChildNodes = FillWithNodes(virtualNodeGrandChildrenInfo, 1, false, false);

    // verify node is physical
    EXPECT_EQ(NodeVisibility::Visible, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // verify child and grandchild nodes' physical parent is set to their closest physical parent node
    EXPECT_EQ(nodes[0]->GetNodeId(), m_cache->GetNode(childNodes[0]->GetNodeId())->GetParentNodeId());
    EXPECT_EQ(nodes[0]->GetNodeId(), m_cache->GetNode(grandChildNodes[0]->GetNodeId())->GetParentNodeId());

    // make virtual
    m_cache->MakeVirtual(*nodes[0]);

    // verify node is virtual
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // verify child and grandchild nodes' physical parent is updated
    EXPECT_EQ(nodes[0]->GetParentNodeId(), m_cache->GetNode(childNodes[0]->GetNodeId())->GetParentNodeId());
    EXPECT_EQ(nodes[0]->GetParentNodeId(), m_cache->GetNode(grandChildNodes[0]->GetNodeId())->GetParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateNode)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);

    // verify node's label is valid
    EXPECT_STREQ("0", m_cache->GetNode(nodes[0]->GetNodeId())->GetLabelDefinition().GetDisplayValue().c_str());

    // update
    nodes[0]->SetLabelDefinition(*LabelDefinition::Create("B"));
    m_cache->Update(nodes[0]->GetNodeId(), *nodes[0], IHierarchyCache::UPDATE_NodeAll);

    // verify node's label has changed
    EXPECT_STREQ("B", m_cache->GetNode(nodes[0]->GetNodeId())->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesFilter)
    {
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), "ruleset_id", "locale", 0, 0);
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    ECClassId usedClassId((uint64_t)1);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[usedClassId] = false;

    DataSourceInfo dsInfo(DataSourceIdentifier(hlInfo.GetId(), { 0 }), RulesetVariables(), DataSourceFilter(), usedClassIds, "", "");
    m_cache->Cache(dsInfo);
    EXPECT_TRUE(dsInfo.GetIdentifier().IsValid());

    // verify the filter is not applied and we find related hierarchy level
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(usedClassId, ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_EQ(1, related.size());

    // update the datasource with new filter
    bset<ECClassId> relationshipClassIds;
    relationshipClassIds.insert(widgetHasGadgetRelationship->GetId());
    dsInfo.SetFilter(DataSourceFilter(DataSourceFilter::RelatedInstanceInfo(relationshipClassIds, RequiredRelationDirection_Both, {ECInstanceKey(usedClassId, ECInstanceId((uint64_t)2))}), nullptr));
    m_cache->Update(dsInfo, DataSourceInfo::PART_Filter);

    // verify the filter is applied and we dont find related hierarchy level anymore
    related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_EQ(0, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedClassIds)
    {
    // cache the data source
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), "ruleset_id", "locale", 0, 0);
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceInfo dsInfo(DataSourceIdentifier(hlInfo.GetId(), {0}), RulesetVariables(), DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Cache(dsInfo);
    EXPECT_TRUE(dsInfo.GetIdentifier().IsValid());

    // verify we don't find related hierarchy level as the key is not related to the data source
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_EQ(0, related.size());

    // update the datasource related class ids
    bmap<ECN::ECClassId, bool> relatedClassIds;
    relatedClassIds[ECN::ECClassId((uint64_t)1)] = false;
    dsInfo.SetRelatedClasses(relatedClassIds);
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses);

    // verify we do find related hierarchy level this time as the data source is now related to the lookup keys
    related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_EQ(1, related.size());
    }

#ifdef wip_ruleset_variables
/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedSettings)
    {
    // cache the data source
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), "ruleset_id", "locale", 0, 0);
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceIdentifier dsInfo(hlInfo.GetId(), { 0 });
    m_cache->Cache(dsInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(dsInfo.IsValid());

    // verify we don't find related hierarchy level as data source is not related to any setting ids
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(0, related.size());

    // update the datasource related settings
    bvector<UserSettingEntry> settings = {UserSettingEntry("setting_id", Json::Value("value0"))};
    m_cache->Update(dsInfo, nullptr, nullptr, &settings);

    // verify we do find related hierarchy level this time as the data source is now related to the setting id
    related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(1, related.size());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemapNodeIds_RemapsDataSourcesWhenParentIsPhysical)
    {
    // create 2 root nodes
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto rootNodes = FillWithNodes(rootInfo, 2, false);

    // create a data source for root node 0
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());

    // verify rootNodes[0] has a datasource and rootNodes[1] doesnt
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(rootNodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(rootNodes[1]->GetNodeId()));

    // remap
    bmap<uint64_t, uint64_t> remapInfo;
    remapInfo.Insert(rootNodes[0]->GetNodeId(), rootNodes[1]->GetNodeId());
    m_cache->RemapNodeIds(remapInfo);

    // verify rootNodes[1] has a datasource and rootNodes[0] doesnt
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(rootNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(rootNodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemapNodeIds_RemapsDataSourcesWhenParentIsVirtual)
    {
    // create 2 root nodes
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto rootNodes = FillWithNodes(rootInfo, 2, false);
    m_cache->MakeVirtual(*rootNodes[0]);

    // create a data source for root node 0
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());

    // verify rootNodes[0] has a datasource and rootNodes[1] doesnt
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(rootNodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(rootNodes[1]->GetNodeId()));

    // remap
    bmap<uint64_t, uint64_t> remapInfo;
    remapInfo.Insert(rootNodes[0]->GetNodeId(), rootNodes[1]->GetNodeId());
    m_cache->RemapNodeIds(remapInfo);

    // verify rootNodes[1] has a datasource and rootNodes[0] doesnt
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(rootNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(rootNodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodesProvider_FindsRootNode)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // create the root nodes
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_A_test", "test descr"),
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_B_test", "test descr")
        };
    FillWithNodes(info, nodes);

    // filter
    NavNodesProviderPtr provider = m_cache->GetFilteredNodesProvider("A", *m_connection, info.first.GetRulesetId().c_str(), "locale");

    // verify filtered node
    EXPECT_EQ(1, provider->GetNodesCount());
    JsonNavNodePtr node;
    ASSERT_TRUE(provider->GetNode(node, 0));
    EXPECT_TRUE(node->Equals(*nodes[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodes_DoesntFindNodesFromDifferentLocales)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale1", 0);

    // create the root nodes
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_A_test", "test descr")
        };
    FillWithNodes(info, nodes);

    // filter
    NavNodesProviderPtr provider = m_cache->GetFilteredNodesProvider("A", *m_connection, info.first.GetRulesetId().c_str(), "locale");

    // verify filtered node
    EXPECT_EQ(0, provider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_ReturnsNodeWhichDoesntHaveChildHierarchyLevel)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);

    // expect the node to have undetermined children
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId())->DeterminedChildren());

    // expect provider to contain the node
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*m_connection,
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str());
    ASSERT_TRUE(provider.IsValid());
    EXPECT_EQ(1, provider->GetNodesCount());

    JsonNavNodePtr expectedNode;
    provider->GetNode(expectedNode, 0);
    EXPECT_EQ(expectedNode->GetNodeId(), nodes[0]->GetNodeId());
    EXPECT_TRUE(expectedNode->DeterminedChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_ReturnsNodeWhichHasUnitializedChildHierarchyLevel)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodes[0]->GetNodeId(), false);

    // expect the node to have undetermined children
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId())->DeterminedChildren());

    // expect provider to contain the node
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*m_connection, info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str());
    ASSERT_TRUE(provider.IsValid());
    ASSERT_EQ(1, provider->GetNodesCount());

    JsonNavNodePtr expectedNode;
    provider->GetNode(expectedNode, 0);
    EXPECT_EQ(expectedNode->GetNodeId(), nodes[0]->GetNodeId());
    EXPECT_TRUE(expectedNode->DeterminedChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_DoesNotReturnNodeThatHasInitializedHierarchyLevel)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1, true);

    // expect provider to be empty
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*m_connection,
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str());
    ASSERT_TRUE(provider.IsValid());
    EXPECT_EQ(0, provider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsNullsWhenEmpty)
    {
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached("test ruleset id", "test locale"));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(999));
    EXPECT_FALSE(m_cache->IsNodeCached(999));

    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "test ruleset id", "test locale", nullptr).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(1).IsValid());
    EXPECT_FALSE(m_cache->FindDataSource(DataSourceIdentifier(1, { 0 }), RulesetVariables()).GetIdentifier().IsValid());
    EXPECT_FALSE(m_cache->FindDataSource(999).GetIdentifier().IsValid());

    IGNORE_BE_ASSERT();

    auto context = CreateContext("test ruleset id", "test locale", 999);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), "test ruleset id", "test locale", 999)).IsNull());
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*context, HierarchyLevelIdentifier(1, m_connection->GetId(), "test ruleset id", "test locale", 0, 0)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(*context, DataSourceIdentifier(1, { 0 })).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(*context, 999).IsNull());
    EXPECT_TRUE(m_cache->GetNode(999).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootDataSource)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto context = CreateContext("test ruleset id", "test locale", 0);

    // verify the data source is cached
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));

    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetId()).IsValid());
    EXPECT_TRUE(m_cache->FindDataSource(DataSourceIdentifier(info.first.GetId(), { 0 }), RulesetVariables()).GetIdentifier().IsValid());

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), info.first.GetRulesetId(), info.first.GetLocale(), 0)).IsValid());
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*context, info.first).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(*context, info.second).IsValid());

    // verify it's not found when looking with invalid parameters
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached("invalid", info.first.GetLocale().c_str()));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), "invalid", info.first.GetLocale(), 0)).IsNull());

    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(info.first.GetRulesetId().c_str(), "invalid"));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), info.first.GetRulesetId(), "invalid", 0)).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDifferentCachedRootDataSourceForDifferentVariables)
    {
    RulesetVariables variables1({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });
    RulesetVariables variables2({ RulesetVariableEntry("var_id", rapidjson::Value("value2")) });

    // cache root data source
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0, true, variables1);
    // cache second root data source with different variables
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0, true, variables2);

    // verify that data sources where cached for same hierarchy level
    EXPECT_TRUE(info1.first == info2.first);
    // verify that data sources are different
    EXPECT_FALSE(info1.second == info2.second);

    // verify the hierarchy level is cached
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info1.first.GetRulesetId().c_str(), info1.first.GetLocale().c_str()));
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), info1.first.GetRulesetId().c_str(), info1.first.GetLocale().c_str(), nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info1.first.GetId()).IsValid());

    // verify that data sources are cached
    DataSourceInfo cachedSource1 = m_cache->FindDataSource(DataSourceIdentifier(info1.first.GetId(), { 0 }), variables1);
    EXPECT_TRUE(cachedSource1.GetIdentifier().IsValid());
    DataSourceInfo cachedSource2 = m_cache->FindDataSource(DataSourceIdentifier(info1.first.GetId(), { 0 }), variables2);
    EXPECT_TRUE(cachedSource2.GetIdentifier().IsValid());
    EXPECT_FALSE(cachedSource1.GetIdentifier() == cachedSource2.GetIdentifier());

    // verify hierarchy level is not found when looking with invalid variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), info1.first.GetLocale(), 0, RulesetVariables()),
        CombinedHierarchyLevelIdentifier(m_connection->GetId(), info1.first.GetRulesetId(), info1.first.GetLocale(), 0)).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto context = CreateContext(info.first.GetRulesetId(), info.first.GetLocale(), 0);

    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    InitNode(*node, info.first);

    // the root data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), info.first.GetLocale(), 0));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // the node's data source should also be empty
    cachedProvider = m_cache->GetDataSource(*context, node->GetNodeId());
    ASSERT_FALSE(cachedProvider.IsValid());

    // cache the node
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    // verify the node is cached
    cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), info.first.GetLocale(), 0));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(m_cache->IsNodeCached(node->GetNodeId()));
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId()).IsValid());

    // verify node's datasource is not empty
    cachedProvider = m_cache->GetDataSource(*context, node->GetNodeId());
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDifferentCachedChildDataSourceForDifferentRulesetVariables)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);
    uint64_t nodeId = nodes[0]->GetNodeId();
    auto context = CreateContext(info.first.GetRulesetId(), info.first.GetLocale(), nodeId);

    // child data source should not exist
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), info.first.GetLocale(), nodeId)).IsNull());

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodeId);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, childrenInfo.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildDataSource)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);
    uint64_t nodeId = nodes[0]->GetNodeId();
    auto context = CreateContext(info.first.GetRulesetId(), info.first.GetLocale(), nodeId);

    // child data source should not exist
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), info.first.GetLocale(), nodeId)).IsNull());

    // two child data sources with different variables
    RulesetVariables variables1({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });
    RulesetVariables variables2({ RulesetVariableEntry("var_id", rapidjson::Value("value2")) });

    auto childrenInfo1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodeId, true, variables1);
    auto childrenInfo2 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodeId, true, variables2);
    EXPECT_TRUE(childrenInfo1.first == childrenInfo2.first);
    EXPECT_FALSE(childrenInfo1.second == childrenInfo2.second);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodeId));

    NavNodesProviderPtr cachedSource1 = m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetRulesetId(), info.first.GetLocale(), nodeId, variables1), childrenInfo1.first);
    EXPECT_TRUE(cachedSource1.IsValid());

    NavNodesProviderPtr cachedSource2 = m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetRulesetId(), info.first.GetLocale(), nodeId, variables2), childrenInfo2.first);
    EXPECT_TRUE(cachedSource2.IsValid());
    EXPECT_TRUE(cachedSource1 != cachedSource2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);
    uint64_t nodeId = nodes[0]->GetNodeId();

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodeId);

    auto context = CreateContext(info.first.GetRulesetId(), info.first.GetLocale(), nodeId);

    // the data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), info.first.GetLocale(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // cache child node
    auto childNodes = FillWithNodes(childrenInfo, 1);

    // verify the child node is cached
    cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), info.first.GetLocale(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(m_cache->IsNodeCached(childNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->GetNode(childNodes[0]->GetNodeId()).IsValid());

    // verify we can get its parent data source
    cachedProvider = m_cache->GetDataSource(*context, childNodes[0]->GetNodeId());
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetNodeVisibility_ReturnsPhysicalNodeVisibility)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);

    // verify results
    EXPECT_EQ(NodeVisibility::Visible, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetNodeVisibility_ReturnsVirtualNodeVisibility)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1, false, true);

    // verify results
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsDataSourcesWithValidRulesetIds)
    {
    // cache root data source for the first ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id1", 1, 0, false, "", "", "", false));
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id1", "locale", 0);

    // cache root data source for the second ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id2", "locale", 0);

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), info1.first.GetLocale(), 0), info1.first);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(info1.first.GetRulesetId().c_str(), cached1->GetContext().GetRuleset().GetRuleSetId().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(*CreateContext(info2.first.GetRulesetId(), info2.first.GetLocale(), 0), info2.first);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_STREQ(info2.first.GetRulesetId().c_str(), cached2->GetContext().GetRuleset().GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsDataSourcesWithValidLocales)
    {
    // cache root data source for the first locale
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale1", 0);

    // cache root data source for the second locale
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale2", 0);

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), info1.first.GetLocale(), 0), info1.first);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(info1.first.GetLocale().c_str(), cached1->GetContext().GetLocale().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(*CreateContext(info2.first.GetRulesetId(), info2.first.GetLocale(), 0), info2.first);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_STREQ(info2.first.GetLocale().c_str(), cached2->GetContext().GetLocale().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsDataSourcesWithValidVariables)
    {
    RulesetVariables variables1({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });
    RulesetVariables variables2({ RulesetVariableEntry("var_id", rapidjson::Value("value2")) });

    // cache root data source for the first variables
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0, true, variables1);

    // cache root data source for the second variables
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0, true, variables2);

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), info1.first.GetLocale(), 0, variables1), info1.first);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_TRUE(variables1 == cached1->GetContext().GetRulesetVariables());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(*CreateContext(info2.first.GetRulesetId(), info2.first.GetLocale(), 0, variables2), info2.first);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_TRUE(variables2 == cached2->GetContext().GetRulesetVariables());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseForEmptySet)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto nodes = FillWithNodes(info, 1);

    bset<uint64_t> parentNodeIds;
    EXPECT_FALSE(m_cache->HasParentNode(nodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseForRootNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto nodes = FillWithNodes(info, 1);

    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(nodes[0]->GetNodeId() + 1);
    EXPECT_FALSE(m_cache->HasParentNode(nodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseIfListDoesntContainParentNodeId)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto rootNodes = FillWithNodes(info, 2);

    // cache child node for root node 0
    auto childNodesInfo = CacheDataSource(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), rootNodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(childNodesInfo, 1);

    // cache child data source for root node 1
    CacheDataSource(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(),
        info.first.GetLocale().c_str(), rootNodes[1]->GetNodeId());

    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(rootNodes[1]->GetNodeId());
    EXPECT_FALSE(m_cache->HasParentNode(childNodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsTrueIfListContainsImmediateParentNodeId)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto rootNodes = FillWithNodes(info, 2);

    // cache child node for root node 0
    auto childNodesInfo = CacheDataSource(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), rootNodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(childNodesInfo, 1);

    // cache child data source for root node 1
    CacheDataSource(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(),
        info.first.GetLocale().c_str(), rootNodes[1]->GetNodeId());

    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(rootNodes[0]->GetNodeId());
    EXPECT_TRUE(m_cache->HasParentNode(childNodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsTrueIfListContainsGrandParentNodeId)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto rootNodes = FillWithNodes(info, 1);

    // cache child node for the root node
    auto childNodesInfo = CacheDataSource(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), rootNodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(childNodesInfo, 1);

    // cache grandchild node for child node
    auto grandchildNodesInfo = CacheDataSource(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), childNodes[0]->GetNodeId());
    auto grandchildNodes = FillWithNodes(grandchildNodesInfo, 1);

    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(rootNodes[0]->GetNodeId());
    EXPECT_TRUE(m_cache->HasParentNode(grandchildNodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode)
    {
    // create a node in an open connection
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when key is invalid
    ECClassInstanceKey invalidKey(
        nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetClass(),
        ECInstanceId(nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetId().GetValue() + 100));
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *ECInstancesNodeKey::Create(invalidKey, { "wrong hash" }), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode_FromCorrectRuleset)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");

    // create a node with ruleset 1
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id_1", 1, 0, false, "", "", "", false));
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id_1", "locale", 0);
    JsonNavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    NavNodeExtendedData(*node1).SetRulesetId(info1.first.GetRulesetId().c_str());
    FillWithNodes(info1, { node1 });

    // create a node with ruleset 2
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id_2", 1, 0, false, "", "", "", false));
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id_2", "locale", 0);
    JsonNavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    NavNodeExtendedData(*node2).SetRulesetId(info2.first.GetRulesetId().c_str());
    FillWithNodes(info2, { node2 });

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id_1", "locale", *node1->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node1->Equals(*locatedNode));

    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id_2", "locale", *node2->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node2->Equals(*locatedNode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode_ForCorrectRulesetVariables)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");

    // create a node with ruleset variables 1
    RulesetVariables variables1({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0, true, variables1);
    JsonNavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    FillWithNodes(info1, { node1 });

    // create a node with ruleset 2
    RulesetVariables variables2({ RulesetVariableEntry("var_id", rapidjson::Value("value2")) });
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0, true, variables2);
    JsonNavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    FillWithNodes(info2, { node2 });

    // verify the node is found successfully with valid key and variables
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *node1->GetKey(), variables1);
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node1->Equals(*locatedNode));

    // verify the node is not found with invalid variables
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *node1->GetKey(), RulesetVariables());
    EXPECT_FALSE(locatedNode.IsValid());

    // verify the second node is found successfully with valid key and variables
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *node2->GetKey(), variables2);
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node2->Equals(*locatedNode));

    // verify the second node is not found with invalid variables
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *node2->GetKey(), RulesetVariables());
    EXPECT_FALSE(locatedNode.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECClassGroupingNode)
    {
    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateClassGroupingNode(*m_connection, *widgetClass1, "test label")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *ECClassGroupingNodeKey::Create(*widgetClass1, {"different hash"}, 1), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECPropertyValueGroupingNode)
    {
    rapidjson::Document groupingValue;
    groupingValue.SetInt(9);
    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty1 = widgetClass1->GetPropertyP("IntProperty");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", groupingValue, false)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *ECPropertyGroupingNodeKey::Create(*widgetClass1, "IntProperty", &groupingValue, {"different hash"}, 1), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECPropertyRangeGroupingNode)
    {
    rapidjson::Document groupingValue;
    groupingValue.SetInt(2);
    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty1 = widgetClass1->GetPropertyP("IntProperty");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", groupingValue, true)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *ECPropertyGroupingNodeKey::Create(*widgetClass1, "IntProperty", &groupingValue, {"different hash"}, 1), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesLabelGroupingNode)
    {
    // create a node in an open connection
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateLabelGroupingNode(*m_connection, "test label")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *NavNodeKey::Create("type", {"wrong hash"}), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesCustomNode)
    {
    // create a node in an open connection
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt matchSetInstanceId
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", "locale", *NavNodeKey::Create("type", {"wrong hash"}), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_AddsToQuickCacheWhenDataSourceReachesRequiredSize)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(info, 2);
    bvector<uint64_t> rootNodeIds;
    std::transform(rootNodes.begin(), rootNodes.end(), std::back_inserter(rootNodeIds), [](NavNodeCPtr node){return node->GetNodeId();});

    // cache child data source 1 (less than required size to add to quick cache)
    auto childrenInfo1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeIds[0]);
    FillWithNodes(childrenInfo1, NODESCACHE_QUICK_Boundary);
    NavNodesProviderPtr datasource1 = m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo1.first), CombinedHierarchyLevelIdentifier(childrenInfo1.first));
    EXPECT_EQ(NODESCACHE_QUICK_Boundary, datasource1->GetNodesCount());

    // cache child data source 2 (of required size to add to quick cache)
    auto childrenInfo2 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeIds[1]);
    FillWithNodes(childrenInfo2, NODESCACHE_QUICK_Boundary + 1);
    NavNodesProviderPtr datasource2 = m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo2.first), CombinedHierarchyLevelIdentifier(childrenInfo2.first));
    EXPECT_EQ(NODESCACHE_QUICK_Boundary + 1, datasource2->GetNodesCount());

    // attempt to add both providers to quick cache
    m_cache->CacheHierarchyLevel(childrenInfo1.first, *datasource1, RulesetVariables());
    m_cache->CacheHierarchyLevel(childrenInfo2.first, *datasource2, RulesetVariables());

    // verify only the second one got cached
    ASSERT_EQ(2, datasource2->GetRefCount());
    datasource2->Release();
    EXPECT_NE(datasource1.get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo1.first), childrenInfo1.first).get());
    EXPECT_EQ(datasource2.get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo2.first), childrenInfo2.first).get());
    datasource2->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_RemovesPreviousDataSourceIfNewOneIsAddedForTheSameHierarchyLevel)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(info, 2);
    uint64_t rootNodeId = rootNodes[0]->GetNodeId();

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeId);
    FillWithNodes(childrenInfo, NODESCACHE_QUICK_Boundary + 1);

    // both datasources point to the same data, but they're different instances
    NavNodesProviderPtr datasource1 = m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo.first), childrenInfo.first);
    NavNodesProviderPtr datasource2 = m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo.first), childrenInfo.first);

    // add the 1st provider to quick cache
    m_cache->CacheHierarchyLevel(childrenInfo.first, *datasource1, RulesetVariables());

    // verify 1st provider is in the quick cache, but not the 2nd one
    ASSERT_EQ(2, datasource1->GetRefCount());
    datasource1->Release();
    EXPECT_EQ(datasource1.get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo.first), childrenInfo.first).get());
    EXPECT_NE(datasource2.get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo.first), childrenInfo.first).get());
    datasource1->AddRef();

    // add the 2nd provider to quick cache
    m_cache->CacheHierarchyLevel(childrenInfo.first, *datasource2, RulesetVariables());

    // verify 2nd provider is in the quick cache, and the 1st one is removed
    ASSERT_EQ(2, datasource2->GetRefCount());
    datasource2->Release();
    EXPECT_NE(datasource1.get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo.first), childrenInfo.first).get());
    EXPECT_EQ(datasource2.get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo.first), childrenInfo.first).get());
    datasource2->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_RemovesLastUsedProvidersWhenMaxSizeIsReached)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(info, NODESCACHE_QUICK_Size + 1);
    bvector<uint64_t> rootNodeIds;
    std::transform(rootNodes.begin(), rootNodes.end(), std::back_inserter(rootNodeIds), [](NavNodeCPtr node){return node->GetNodeId();});

    // cache NODESCACHE_QUICK_Size providers
    bvector<NavNodesProviderPtr> providers;
    for (size_t i = 0; i < NODESCACHE_QUICK_Size; ++i)
        {
        auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeIds[i]);
        FillWithNodes(childrenInfo, NODESCACHE_QUICK_Boundary + 1);

        NavNodesProviderPtr provider = m_cache->GetCombinedHierarchyLevel(*CreateContext(childrenInfo.first), childrenInfo.first);
        m_cache->CacheHierarchyLevel(childrenInfo.first, *provider, RulesetVariables());
        providers.push_back(provider);
        }

    // verify all providers are in the quick cache
    for (size_t i = 0; i < providers.size(); ++i)
        {
        ASSERT_EQ(2, providers[i]->GetRefCount());
        providers[i]->Release();
        CombinedHierarchyLevelIdentifier providerInfo(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), rootNodeIds[i]);
        EXPECT_EQ(providers[i].get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(providerInfo), providerInfo).get());
        providers[i]->AddRef();
        }

    // add a new provider
    auto newInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeIds[NODESCACHE_QUICK_Size]);
    FillWithNodes(newInfo, NODESCACHE_QUICK_Boundary + 1);
    NavNodesProviderPtr provider = m_cache->GetCombinedHierarchyLevel(*CreateContext(newInfo.first), newInfo.first);
    m_cache->CacheHierarchyLevel(newInfo.first, *provider, RulesetVariables());

    // verify the first cached provider is now removed from cache and the new one is inserted
    CombinedHierarchyLevelIdentifier id0(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), rootNodeIds[0]);
    EXPECT_NE(providers[0].get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(id0), id0).get());
    for (size_t i = 1; i < providers.size(); ++i)
        {
        ASSERT_EQ(2, providers[i]->GetRefCount());
        providers[i]->Release();
        CombinedHierarchyLevelIdentifier idi(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), rootNodeIds[i]);
        EXPECT_EQ(providers[i].get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(idi), idi).get());
        providers[i]->AddRef();
        }

    ASSERT_EQ(2, provider->GetRefCount());
    provider->Release();
    CombinedHierarchyLevelIdentifier idLast(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), rootNodeIds[NODESCACHE_QUICK_Size]);
    EXPECT_EQ(provider.get(), m_cache->GetCombinedHierarchyLevel(*CreateContext(idLast), idLast).get());
    provider->AddRef();
    }

#ifdef wip_ruleset_variables
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsEmptyListWhenRulesetAndSettingIdsAreInvalid)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));

    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels("invalid", "any");
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsOnlyRelatedDataSources)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(rootInfo, 2);

    // cache unrelated datasource
    auto childInfo0 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    bvector<UserSettingEntry> settings0 = {UserSettingEntry("setting_0", Json::Value("value0"))};
    m_cache->Update(childInfo0.second, nullptr, nullptr, &settings0);

    // cache related datasource
    auto childInfo1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[1]->GetNodeId());
    bvector<UserSettingEntry> settings1 = {UserSettingEntry("setting_1", Json::Value("value1")) };
    m_cache->Update(childInfo1.second, nullptr, nullptr, &settings1);

    // verify the correct datasource is found
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels("ruleset_id", "setting_1");
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(childInfo1.first, related[0]);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenInstancesArentRelated)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassMatchesAndFilterIsNotSpecified)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), DataSourceFilter(), usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses);
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenClassMatchesButFilterDoesnt)
    {
    ECClassCP widgetHasGadget = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    ECClassId usedClassId((uint64_t)1);
    ECInstanceId usedInstanceId((uint64_t)1);

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadget->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, {ECInstanceKey(usedClassId, usedInstanceId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[usedClassId] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(usedClassId, ECInstanceId((uint64_t)456)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByInstanceId)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    ECClassId usedClassId(gadgetClass->GetId());
    ECInstanceId usedInstanceId((uint64_t)123);

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, {ECInstanceKey(usedClassId, usedInstanceId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(usedClassId, usedInstanceId));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByForwardRelatedInstances)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Forward, {ECInstanceKey(widgetClass->GetId(), widgetId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(gadgetClass->GetId(), gadgetId));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByBackwardRelatedInstances)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, {ECInstanceKey(gadgetClass->GetId(), gadgetId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByAnyDirectionRelatedInstances)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Both, {ECInstanceKey(gadgetClass->GetId(), gadgetId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByRelatedInstancesFollowingMultipleRelationships)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");
    ECClassCP widgetHasGadgetsRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets");

    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    relationshipIds.insert(widgetHasGadgetsRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, {ECInstanceKey(gadgetClass->GetId(), gadgetId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenChildNodeGroupedInstanceKeysMatchByClassId)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)2), ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenChildNodeGroupedInstanceKeysMatchByInstanceId)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenChildNodeGroupedInstanceKeysMatch)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenChildNodeGroupedInstanceKeysMatchButFilterDoesnt)
    {
    ECClassCP widgetHasGadget = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    ECClassId usedClassId((uint64_t)1);

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadget->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, {ECInstanceKey(usedClassId, ECInstanceId((uint64_t)123))}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[usedClassId] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetInstanceKey(ECInstanceKey(usedClassId, ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(usedClassId, ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ReturnsInvalidRootDatasourceForDifferentRulesetVariables)
    {
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value0"))});
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    DataSourceInfo dsInfo(info.second, relatedVariables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first, relatedVariables), info.first).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value1")) });
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first, differentVariables), info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ReturnsInvalidChildDatasourceCacheForDifferentRulesetVariables)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(1))});
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    DataSourceInfo dsInfo(childInfo.second, relatedVariables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first, relatedVariables), childInfo.first).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(10)) });
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first, differentVariables), childInfo.first).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsInvalidRootDatasourceForDIfferentRulesetVariables)
    {
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value0"))});
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    DataSourceInfo dsInfo(info.second, relatedVariables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(info.first, relatedVariables), info.first).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value1")) });
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext(info.first, differentVariables), info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsInvalidChildDatasourceForDifferentRulesetVariables)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(1))});
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    DataSourceInfo dsInfo(childInfo.second, relatedVariables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(childInfo.first, relatedVariables), childInfo.first).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(10)) });
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(childInfo.first, differentVariables), childInfo.first).IsNull());
    }

#ifdef wip_ruleset_variables
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_ClearsRootDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value0");
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value("value0"))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());

    // verify that valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value1");

    EXPECT_FALSE(m_cache->GetDataSource(info.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_DoesntClearRootDatasourceCacheIfRelatedSettingsValuesDidntChange)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(10))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());

    // verify what valid datasource is returned with same user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_ClearsChildDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 1);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(1))};
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    m_cache->Update(childInfo.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(childInfo.second).IsValid());

    // verify what valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetDataSource(childInfo.second).IsNull());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Savepoint_DiscardsChangesWhenCanceled)
    {
    auto savepoint = m_cache->CreateSavepoint();

    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto nodes = FillWithNodes(info, 1);

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    savepoint->Cancel();

    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Savepoint_DoesntDiscardChangesWhenNotCanceled)
    {
    auto savepoint = m_cache->CreateSavepoint();

    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto nodes = FillWithNodes(info, 1);

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    savepoint = nullptr;

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, OnRulesetVariablesUsed_DoesNotClearVariablesWhenThresholdIsNotExceeded)
    {
    // cache root hierarchy level
    HierarchyLevelIdentifier info(m_connection->GetId(), "ruleset_id", "locale", 0, 0);
    m_cache->Cache(info);
    ASSERT_TRUE(info.IsValid());

    RulesetVariables variables;
    bvector<DataSourceIdentifier> dataSourceIdentifiers;
    int cacheThreshold = atoi(NODESCACHE_VARIABLES_Threshold);
    // cache some data sources with different variables not exceeding threshold
    for (uint64_t i = 0; i < cacheThreshold; ++i)
        {
        variables.SetIntValue("var_id", i);
        DataSourceInfo ds(DataSourceIdentifier(info.GetId(), { i }), variables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
        // cache data source
        m_cache->Cache(ds);
        m_cache->FinalizeInitialization(ds.GetIdentifier());
        // check that data source exists
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info, variables), ds.GetIdentifier()).IsValid());
        dataSourceIdentifiers.push_back(ds.GetIdentifier());
        BeThreadUtilities::BeSleep(5); // sleep for 5 milliseconds to make sure the "last used" time differs
        }

    variables.SetIntValue("var_id", cacheThreshold);
    m_cache->OnRulesetVariablesUsed(variables, "var_id");

    // check that all data sources still exists
    for (uint64_t i = 0; i < cacheThreshold; ++i)
        {
        variables.SetIntValue("var_id", i);
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info, variables), dataSourceIdentifiers[i]).IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, OnRulesetVariablesUsed_ClearVariablesWhenThresholdIsExceeded)
    {
    // cache root hierarchy level
    HierarchyLevelIdentifier info(m_connection->GetId(), "ruleset_id", "locale", 0, 0);
    m_cache->Cache(info);
    ASSERT_TRUE(info.IsValid());

    RulesetVariables variables;
    bvector<DataSourceIdentifier> dataSourceIdentifiers;
    int cacheThreshold = atoi(NODESCACHE_VARIABLES_Threshold);
    // cache some data sources with different ruleset variables to exceed threshold
    for (uint64_t i = 0; i < cacheThreshold + 1; ++i)
        {
        variables.SetIntValue("var_id", i);
        DataSourceInfo ds(DataSourceIdentifier(info.GetId(), {i}), variables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
        // cache data source
        m_cache->Cache(ds);
        m_cache->FinalizeInitialization(ds.GetIdentifier());
        // check that data source exists
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info, variables), ds.GetIdentifier()).IsValid());
        dataSourceIdentifiers.push_back(ds.GetIdentifier());
        BeThreadUtilities::BeSleep(5); // sleep for 5 milliseconds to make sure the "last used" time differs
        }

    // remove data sources using oldest variables
    variables.SetIntValue("var_id", cacheThreshold);
    m_cache->OnRulesetVariablesUsed(variables, "ruleset_id");

    // check that data source with oldest variables is removed
    variables.SetIntValue("var_id", 0);
    EXPECT_FALSE(m_cache->GetDataSource(*CreateContext(info, variables), dataSourceIdentifiers[0]).IsValid());

    // check all data source exist except first
    for (uint64_t i = 1; i < cacheThreshold + 1; ++i)
        {
        variables.SetIntValue("var_id", i);
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info, variables), dataSourceIdentifiers[i]).IsValid());
        }
    }

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                09/2017
+===============+===============+===============+===============+===============+======*/
struct DiskNodesCacheTests : NodesCacheTests
    {
    bool m_cacheUpdateData = true;

    void TearDown() override
        {
        BeFileName cacheDb(m_cache->GetDb().GetDbFileName());
        NodesCacheTests::TearDown();
        cacheDb.BeDeleteFile();
        }

    virtual std::unique_ptr<NodesCache> _CreateNodesCache(IConnectionR connection, BeFileNameCR tempDir) override
        {
        return NodesCache::Create(connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, m_cacheUpdateData);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, CreatesNewDbFileIfCacheIsAlreadyInUse)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    auto secondCache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);

    Utf8CP firstCacheName = m_cache->GetDb().GetDbFileName();
    Utf8CP secondCacheName = secondCache->GetDb().GetDbFileName();

    EXPECT_STRNE(firstCacheName, secondCacheName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined (BENTLEYCONFIG_OS_LINUX) // informed Kyle/Caleb/Gintaras 9/28/18
TEST_F(DiskNodesCacheTests, ShareCachedHierarchiesBetweenSessions)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto nodes = FillWithNodes(info, 2, true);

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));

    // close cache
    m_cache = nullptr;

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache.get());

    // cache should not be cleaned
    CombinedHierarchyLevelIdentifier updatedInfo(m_connection->GetId(), info.first.GetRulesetId(),
        info.first.GetLocale(), 0);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(updatedInfo), updatedInfo).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, RecreatesCacheIfExistingCacheDoesntHaveUpdateData)
    {
    // create the cache in read-only mode
    m_cacheUpdateData = false;
    ReCreateNodesCache();

    // cache something
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());

    // re-create the cache in read-write mode
    m_cacheUpdateData = true;
    ReCreateNodesCache();

    // expect the data to be gone
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, DowngradesReadWriteCacheToReadOnly)
    {
    // verify this is a read write cache (gets created by default for these tests because m_cacheUpdateData = true)
    Utf8String value;
    EXPECT_EQ(BE_SQLITE_ROW, m_cache->GetDb().QueryProperty(value, PropertySpec("CachesUpdateData", "HierarchyCache")));
    EXPECT_STREQ("1", value.c_str());

    // re-open in read-only mode
    m_cacheUpdateData = false;
    ReCreateNodesCache();

    // expect the db to be downgraded
    EXPECT_EQ(BE_SQLITE_ROW, m_cache->GetDb().QueryProperty(value, PropertySpec("CachesUpdateData", "HierarchyCache")));
    EXPECT_STREQ("0", value.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Test situation when app is closed, ruleset is modified and app is turned on.
* In this case OnRulesetDisposed isn't called and only OnRulesetCreated is called.
* @bsitest                                      Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearSharedCacheIfRulesetWasModified)
    {
    // create ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);
    ruleset->AddPresentationRule(*new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false));
    m_cache->OnRulesetUsed(*ruleset);

    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), ruleset->GetRuleSetId(), "locale", 0);

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));

    // mock new session: app is turned on with modified ruleset and OnRulesetCreated is called
    ruleset->AddPresentationRule(*new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree));
    m_cache->OnRulesetUsed(*ruleset);

    // verify cache is cleared
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearsCacheIfCacheFileSizeExceedsLimit)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));

    // set cache limit
    m_cache->SetCacheFileSizeLimit(1);

    // close cache
    m_cache = nullptr;

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache.get());

    // verify cache is cleared
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearsOldestRulesetCacheWhenCacheFileSizeExceedsLimit)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    FillWithNodes(info, 30, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());

    // create second ruleset and cache root data source
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));

    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id2", "locale", 0);
    FillWithNodes(info2, 40, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info2.first), info2.first).IsValid());

    // set cache file size limit
    const_cast<Db&>(m_cache->GetDb()).SaveChanges();
    BeFileName cacheFile(m_cache->GetDb().GetDbFileName());
    uint64_t fileSize = 0;
    cacheFile.GetFileSize(fileSize);
    m_cache->SetCacheFileSizeLimit(fileSize - 1);

    // close cache
    m_cache = nullptr;

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache.get());

    // verify first ruleset cache was deleted
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());

    // verify second ruleset cache wasn't deleted
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info2.first), info2.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, DoesntClearCacheWhenCacheFileSizeAndLimitAreEqual)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    FillWithNodes(info, 30, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());

    // set cache file size limit
    const_cast<Db&>(m_cache->GetDb()).SaveChanges();
    BeFileName cacheFile(m_cache->GetDb().GetDbFileName());
    uint64_t fileSize = 0;
    cacheFile.GetFileSize(fileSize);
    m_cache->SetCacheFileSizeLimit(fileSize);

    // close cache
    m_cache = nullptr;

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache.get());

    // verify the cache is still valid
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearSharedCacheIfHierarchyWasModified)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first), info.first).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));

    // close cache
    m_cache = nullptr;

    // change connection file last modification time
    BeFileName projectPath(s_project->GetECDbPath());
    time_t modTime;
    projectPath.GetFileTime(nullptr, nullptr, &modTime);
    modTime -= 100;
    projectPath.SetFileTime(nullptr, &modTime);

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache.get());

    // cache should be empty for this connection
    CombinedHierarchyLevelIdentifier updatedInfo(m_connection->GetId(), info.first.GetRulesetId(),
        info.first.GetLocale(), 0);
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(updatedInfo), updatedInfo).IsValid());
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2018
+===============+===============+===============+===============+===============+======*/
struct DiskNodesCacheLocationTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;

    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    TestUserSettingsManager m_userSettings;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesProviderContextFactory m_nodesProviderContextFactory;
    BeFileName m_directory;
    Utf8String m_ecdbFileName;
    DiskNodesCacheLocationTests() : m_nodesProviderContextFactory(m_connections) {}
    void SetUp() override
        {
        BeTest::GetHost().GetOutputRoot(m_directory);
        m_directory.AppendToPath(L"DiskNodesCacheLocationTests");
        if (!m_directory.DoesPathExist())
            BeFileName::CreateNewDirectory(m_directory.c_str());
        m_ecdbFileName = Utf8String(BeFileName(s_project->GetECDbCR().GetDbFileName()).GetFileNameAndExtension().c_str());
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        }

    virtual void TearDown() override
        {
        m_connections.ClearConnections();
        s_project->GetECDb().AbandonChanges();
        }

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("DiskNodesCacheLocationTestsProject");
        }

    static void TearDownTestCase()
        {
        s_project->GetECDb().CloseDb();
        DELETE_AND_CLEAR(s_project);
        }
    };
ECDbTestProject* DiskNodesCacheLocationTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, CreatesCacheInSpecifiedDirectory)
    {
    BeFileName expectedPath = m_directory;
    Utf8PrintfString cacheName("%s.HierarchyCache.db", m_ecdbFileName.c_str());
    expectedPath.AppendSeparator().AppendUtf8(cacheName.c_str());

    auto cache = NodesCache::Create(*m_connection, m_directory, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    EXPECT_STREQ(expectedPath.GetNameUtf8().c_str(), cache->GetDb().GetDbFileName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, ReusesExistingCache)
    {
    BeFileName expectedPath = m_directory;
    Utf8PrintfString cacheName("%s.HierarchyCache.db", m_ecdbFileName.c_str());
    expectedPath.AppendSeparator().AppendUtf8(cacheName.c_str());

    {
    NodesCache::Create(*m_connection, m_directory, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    }
    EXPECT_TRUE(expectedPath.DoesPathExist());

    auto cache = NodesCache::Create(*m_connection, m_directory, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    EXPECT_STREQ(expectedPath.GetNameUtf8().c_str(), cache->GetDb().GetDbFileName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, CreatesSeparateCacheWhenExistingIsLocked)
    {
    BeFileName expectedPath1 = m_directory;
    Utf8PrintfString cacheName("%s.HierarchyCache.db", m_ecdbFileName.c_str());
    expectedPath1.AppendSeparator().AppendUtf8(cacheName.c_str());

    auto cache1 = NodesCache::Create(*m_connection, m_directory, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    EXPECT_STREQ(expectedPath1.GetNameUtf8().c_str(), cache1->GetDb().GetDbFileName());

    BeFileName cache2Path;
    {
        auto cache2 = NodesCache::Create(*m_connection, m_directory, m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
        cache2Path = BeFileName(cache2->GetDb().GetDbFileName());
        BeFileName expectedDirectory2 = BeFileName(m_directory).AppendSeparator();
        EXPECT_STREQ(expectedDirectory2.c_str(), cache2Path.GetDirectoryName().c_str());

        bvector<WString> nameParts;
        BeStringUtilities::Split(cache2Path.GetFileNameAndExtension().c_str(), L".", nullptr, nameParts);
        ASSERT_EQ(4, nameParts.size());
        EXPECT_STREQ(L"HierarchyCache", nameParts[1].c_str());
        EXPECT_EQ(SUCCESS, BeGuid().FromString(Utf8String(nameParts[2].c_str()).c_str()));
        EXPECT_STREQ(L"db", nameParts[3].c_str());
    }
    EXPECT_FALSE(cache2Path.DoesPathExist());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, CreatesCacheInTheSameDirectoryAsIModelIfDirectoryNotSpecified)
    {
    BeFileName expectedPath = BeFileName(s_project->GetECDbPath()).GetDirectoryName();
    Utf8PrintfString cacheName("%s.HierarchyCache.db", m_ecdbFileName.c_str());
    expectedPath.AppendUtf8(cacheName.c_str());

    auto cache = NodesCache::Create(*m_connection, BeFileName(), m_nodesFactory, m_nodesProviderContextFactory, NodesCacheType::Disk, true);
    EXPECT_STREQ(expectedPath.GetNameUtf8().c_str(), cache->GetDb().GetDbFileName());
    }
