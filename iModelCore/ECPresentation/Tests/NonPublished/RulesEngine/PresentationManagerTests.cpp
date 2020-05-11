/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "TestHelpers.h"
#include "StubRulesDrivenECPresentationManagerImpl.h"
#include "../../NonPublished/RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    RulesDrivenECPresentationManager* m_manager;

    RulesDrivenECPresentationManagerTests() : m_manager(nullptr) {}
    virtual RulesDrivenECPresentationManager::Impl* _CreateImpl(RulesDrivenECPresentationManager::Impl::Params const&) = 0;
    virtual void SetUp() override;
    virtual void TearDown() override;

    static void SetUpTestCase();
    static void TearDownTestCase();
    static ECDbR GetECDb() {return s_project->GetECDb();}

    IConnectionPtr GetConnection() {return m_manager->GetConnections().GetConnection(GetECDb());}
    ECClassCP GetClass(Utf8CP name) { return GetECDb().Schemas().GetClass("RulesEngineTest", name); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static uint64_t CreateNodeId() { static uint64_t id = 0; return ++id; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateInstanceNode(ECClassInstanceKey instanceKey, Utf8CP label)
        {
        TestNavNodePtr node = TestNodesHelper::CreateInstanceNode(*GetConnection(), *instanceKey.GetClass(), instanceKey.GetId());
        node->SetNodeId(CreateNodeId());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*GetConnection(), *node, bvector<Utf8String>{std::to_string(node->GetNodeId()).c_str()}, false));
        node->SetLabelDefinition(*LabelDefinition::Create(label));
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Aidas.Vaisknoras                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateTreeNode(uint64_t nodeId, NavNodeCPtr parent, Utf8CP label = "")
        {
        TestNavNodePtr node = TestNavNode::Create(*GetConnection());
        node->SetNodeId(nodeId);
        bvector<Utf8String> path = (parent.IsNull()) ? bvector<Utf8String>() : parent->GetKey()->GetHashPath();
        path.push_back(std::to_string(node->GetNodeId()).c_str());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*GetConnection(), *node, path, false));
        if (parent.IsValid())
            node->SetParentNodeId(parent->GetNodeId());
        node->SetLabelDefinition(*LabelDefinition::Create(label));
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateInstanceNode(ECClassCR ecClass, ECInstanceId instanceId, Utf8CP label)
        {
        return CreateInstanceNode(ECClassInstanceKey(ecClass, instanceId), label);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateInstanceNode(ECInstanceKey instanceKey, Utf8CP label)
        {
        ECClassCP ecClass = GetECDb().Schemas().GetClass(instanceKey.GetClassId());
        return CreateInstanceNode(*ecClass, instanceKey.GetInstanceId(), label);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateClassGroupingNode(ECClassCR ecClass, Utf8CP label, bvector<ECInstanceKey> const& groupedKeys)
        {
        TestNavNodePtr node = TestNodesHelper::CreateClassGroupingNode(*GetConnection(), ecClass, label);
        node->SetNodeId(CreateNodeId());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*GetConnection(), *node, bvector<Utf8String>{std::to_string(node->GetNodeId()).c_str()}, false));
        NavNodeExtendedData(*node).SetInstanceKeys(groupedKeys);
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreatePropertyGroupingNode(ECClassCR ecClass, Utf8CP propertyName, Utf8CP label, int rangeIndex, rapidjson::Value const* groupingValueP, bvector<ECInstanceKey> const& groupedKeys)
        {
        ECPropertyCP ecProperty = ecClass.GetPropertyP(propertyName);
        if (nullptr == ecProperty)
            {
            BeAssert(false);
            return nullptr;
            }
        rapidjson::Document groupingValue;
        if (nullptr != groupingValueP)
            groupingValue.CopyFrom(*groupingValueP, groupingValue.GetAllocator());
        else if (-1 != rangeIndex)
            groupingValue.SetInt(rangeIndex);
        TestNavNodePtr node = TestNodesHelper::CreatePropertyGroupingNode(*GetConnection(), ecClass, *ecProperty, label, groupingValue, -1 != rangeIndex);
        node->SetNodeId(CreateNodeId());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*GetConnection(), *node, bvector<Utf8String>{std::to_string(node->GetNodeId()).c_str()}, false));
        NavNodeExtendedData(*node).SetInstanceKeys(groupedKeys);
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateLabelGroupingNode(Utf8CP label, bvector<ECInstanceKey> const& groupedKeys)
        {
        TestNavNodePtr node = TestNodesHelper::CreateLabelGroupingNode(*GetConnection(), label);
        node->SetNodeId(CreateNodeId());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*GetConnection(), *node, bvector<Utf8String>{std::to_string(node->GetNodeId()).c_str()}, false));
        NavNodeExtendedData(*node).SetInstanceKeys(groupedKeys);
        return node;
        }
    };
ECDbTestProject* RulesDrivenECPresentationManagerTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("RulesDrivenECPresentationManagerTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::TearDownTestCase()
    {
    s_project->GetECDb().CloseDb();
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::SetUp()
    {
    m_manager = new RulesDrivenECPresentationManager(RulesDrivenECPresentationManager::Params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost())));

    RulesDrivenECPresentationManager::Params params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    params.SetConnections(std::make_shared<TestConnectionManager>());
    m_manager->SetImpl(*_CreateImpl(*m_manager->CreateImplParams(params)));

    m_manager->GetConnections().CreateConnection(GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::TearDown()
    {
    DELETE_AND_CLEAR(m_manager);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerStubbedImplTests : RulesDrivenECPresentationManagerTests
    {
    StubRulesDrivenECPresentationManagerImpl* m_impl;

    virtual RulesDrivenECPresentationManager::Impl* _CreateImpl(RulesDrivenECPresentationManager::Impl::Params const& params) override
        {
        m_impl = new StubRulesDrivenECPresentationManagerImpl(params);
        return m_impl;
        }
    };

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                11/2019
//---------------------------------------------------------------------------------------
static ECInstanceKey GetFirstInstanceKey(ECInstancesNodeKey const& nodeKey)
    {
    ECClassInstanceKeyCR classKey = nodeKey.GetInstanceKeys().front();
    return ECInstanceKey(classKey.GetClass()->GetId(), classKey.GetId());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePath_InstancesHierarchy)
    {
    // create the hierarchy
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)1), "A"));
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)2), "B"));
    NavNodePtr node1 = hierarchy[nullptr].back();
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)1), "B_1"));
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)2), "B_2"));
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)3), "B_3"));
    NavNodePtr node2 = *(hierarchy[node1].begin() + 1);
    hierarchy[node2].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)3), "B_2_1"));
    hierarchy[node2].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)4), "B_2_2"));
    NavNodePtr node3 = hierarchy[node2].front();
    m_impl->SetHierarchy(hierarchy);

    /*
    A (w1)
    B (w2)              *
        B_1 (g1)
        B_2 (g2)        *
            B_2_1 (w3)  *
            B_2_2 (s4)
        B_3 (g3)
    */

    // create the keys path
    bvector<ECInstanceKey> keysPath =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node2->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node3->GetKey()->AsECInstanceNodeKey())
        };

    // test
    NodesPathElement path = m_manager->GetNodePath(GetECDb(), keysPath).get();

    NodesPathElement const* curr = &path;
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_2", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_2_1", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePath_InstancesHierarchyWithGrouping)
    {
    ECInstanceKey instanceKey1(GetClass("Widget")->GetId(), ECInstanceId((uint64_t)3));
    ECInstanceKey instanceKey2(GetClass("Widget")->GetId(), ECInstanceId((uint64_t)4));
    ECInstanceKey instanceKey3(GetClass("Gadget")->GetId(), ECInstanceId((uint64_t)1));
    ECInstanceKey instanceKey4(GetClass("Widget")->GetId(), ECInstanceId((uint64_t)5));
    ECInstanceKey instanceKey5(GetClass("Widget")->GetId(), ECInstanceId((uint64_t)6));
    ECInstanceKey instanceKey6(GetClass("Widget")->GetId(), ECInstanceId((uint64_t)7));

    /* create the hierarchy

             A          B
           / | \
         /   |   \
       A_1  A_2  A_3
           /   \
          /     \
       A_2_1   A_2_2
                / \
               /   \
         A_2_2_1   A_2_2_2

    */
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateClassGroupingNode(*GetClass("Widget"), "A", {instanceKey1, instanceKey2, instanceKey3, instanceKey4, instanceKey5}));
    hierarchy[nullptr].push_back(CreateClassGroupingNode(*GetClass("Gadget"), "B", {instanceKey6}));
    NavNodePtr node1 = hierarchy[nullptr].front();
    hierarchy[node1].push_back(CreatePropertyGroupingNode(*GetClass("Widget"), "IntProperty", "A_1", 1, nullptr, {instanceKey4}));
    hierarchy[node1].push_back(CreatePropertyGroupingNode(*GetClass("Widget"), "BoolProperty", "A_2", 2, nullptr, {instanceKey1, instanceKey2, instanceKey3}));
    hierarchy[node1].push_back(CreatePropertyGroupingNode(*GetClass("Widget"), "DoubleProperty", "A_3", 3, nullptr, {instanceKey5}));
    NavNodePtr node2 = *(hierarchy[node1].begin() + 1);
    hierarchy[node2].push_back(CreateLabelGroupingNode("A_2_1", {instanceKey3}));
    hierarchy[node2].push_back(CreateLabelGroupingNode("A_2_2", {instanceKey1, instanceKey2}));
    NavNodePtr node3 = hierarchy[node2].back();
    hierarchy[node3].push_back(CreateInstanceNode(instanceKey1, "A_2_2_1"));
    hierarchy[node3].push_back(CreateInstanceNode(instanceKey2, "A_2_2_2"));
    NavNodePtr node4 = hierarchy[node3].front();
    m_impl->SetHierarchy(hierarchy);

    // create the keys path
    bvector<ECInstanceKey> keysPath = {GetFirstInstanceKey(*node4->GetKey()->AsECInstanceNodeKey())};

    // test
    NodesPathElement path = m_manager->GetNodePath(GetECDb(), keysPath).get();

    NodesPathElement const* curr = &path;
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A_2", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A_2_2", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A_2_2_1", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_Multiple_ReturnsTwoSeparatePathsWhenTheyDontIntersect)
    {
    // create the hierarchy
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)1), "A"));
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)2), "B"));
    NavNodePtr node1_1 = hierarchy[nullptr].front();
    hierarchy[node1_1].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)1), "A_1"));
    hierarchy[node1_1].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)2), "A_2"));
    hierarchy[node1_1].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)3), "A_3"));
    NavNodePtr node1_2 = hierarchy[node1_1].back();
    hierarchy[node1_2].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)3), "A_2_1"));
    hierarchy[node1_2].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)4), "A_2_2"));
    NavNodePtr node2_1 = hierarchy[nullptr].back();
    hierarchy[node2_1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)1), "B_1"));
    hierarchy[node2_1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)2), "B_2"));
    hierarchy[node2_1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)3), "B_3"));
    NavNodePtr node2_2 = *(hierarchy[node2_1].begin() + 1);
    hierarchy[node2_2].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)3), "B_2_1"));
    hierarchy[node2_2].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)4), "B_2_2"));
    NavNodePtr node2_3 = hierarchy[node2_2].front();
    m_impl->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 =
        {
        GetFirstInstanceKey(*node1_1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_2->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath2 =
        {
        GetFirstInstanceKey(*node2_1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node2_2->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node2_3->GetKey()->AsECInstanceNodeKey())
        };
    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodePaths(GetECDb(), keysPaths, -1).get();
    ASSERT_EQ(2, paths.size());

    // 1st branch
    NodesPathElement const* curr = &paths[0];
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(2, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A_3", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, curr->GetChildren().size());

    // 2nd branch
    curr = &paths[1];
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_2", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_2_1", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_Multiple_ReturnsMergedPathWhenPathsIntersect)
    {
    // create the hierarchy
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)1), "A"));
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)2), "B"));
    NavNodePtr node1 = hierarchy[nullptr].back();
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)1), "B_1"));
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)2), "B_2"));
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)3), "B_3"));
    NavNodePtr node1_2 = hierarchy[node1].front();
    hierarchy[node1_2].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)3), "B_1_1"));
    hierarchy[node1_2].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)4), "B_1_2"));
    NavNodePtr node1_3 = hierarchy[node1_2].front();
    NavNodePtr node2_2 = hierarchy[node1].back();
    hierarchy[node2_2].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)3), "B_3_1"));
    hierarchy[node2_2].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)4), "B_3_2"));
    NavNodePtr node21_3 = hierarchy[node2_2].back();
    NavNodePtr node22_3 = hierarchy[node2_2].front();
    m_impl->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_2->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath2 =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_2->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_3->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath3 =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node2_2->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node21_3->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath4 =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node2_2->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node22_3->GetKey()->AsECInstanceNodeKey())
        };
    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2, keysPath3, keysPath4};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodePaths(GetECDb(), keysPaths, -1).get();
    ASSERT_EQ(1, paths.size());

    NodesPathElement const* curr = &paths[0];
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(2, curr->GetChildren().size());

    NodesPathElement const* curr1 = &curr->GetChildren()[0];
    EXPECT_EQ(0, curr1->GetIndex());
    ASSERT_TRUE(curr1->GetNode().IsValid());
    EXPECT_STREQ("B_1", curr1->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, curr1->GetChildren().size());

    curr1 = &curr1->GetChildren().front();
    EXPECT_EQ(0, curr1->GetIndex());
    ASSERT_TRUE(curr1->GetNode().IsValid());
    EXPECT_STREQ("B_1_1", curr1->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, curr1->GetChildren().size());

    NodesPathElement const* curr2 = &curr->GetChildren()[1];
    EXPECT_EQ(2, curr2->GetIndex());
    ASSERT_TRUE(curr2->GetNode().IsValid());
    EXPECT_STREQ("B_3", curr2->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(2, curr2->GetChildren().size());

    NodesPathElement const* curr21 = &curr2->GetChildren()[0];
    EXPECT_EQ(1, curr21->GetIndex());
    ASSERT_TRUE(curr21->GetNode().IsValid());
    EXPECT_STREQ("B_3_2", curr21->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, curr21->GetChildren().size());

    NodesPathElement const* curr22 = &curr2->GetChildren()[1];
    EXPECT_EQ(0, curr22->GetIndex());
    ASSERT_TRUE(curr22->GetNode().IsValid());
    EXPECT_STREQ("B_3_1", curr22->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, curr22->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_Multiple_MarksTheSpecifiedPath)
    {
    // create the hierarchy
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)1), "A"));
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)2), "B"));
    NavNodePtr node1 = hierarchy[nullptr].back();
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)1), "B_1"));
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)2), "B_2"));
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t)3), "B_3"));
    NavNodePtr node1_2 = hierarchy[node1].front();
    hierarchy[node1_2].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)3), "B_1_1"));
    hierarchy[node1_2].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)4), "B_1_2"));
    NavNodePtr node1_3 = hierarchy[node1_2].front();
    NavNodePtr node2_2 = hierarchy[node1].back();
    hierarchy[node2_2].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t)3), "B_3_1"));
    hierarchy[node2_2].push_back(CreateInstanceNode(*GetClass("Sprocket"), ECInstanceId((uint64_t)4), "B_3_2"));
    NavNodePtr node21_3 = hierarchy[node2_2].back();
    NavNodePtr node22_3 = hierarchy[node2_2].front();
    m_impl->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_2->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath2 =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_2->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_3->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath3 =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node2_2->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node21_3->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath4 =
        {
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node2_2->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node22_3->GetKey()->AsECInstanceNodeKey())
        };
    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2, keysPath3, keysPath4};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodePaths(GetECDb(), keysPaths, 2).get();
    ASSERT_EQ(1, paths.size());

    NodesPathElement const* curr = &paths[0];
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(2, curr->GetChildren().size());

    curr = &curr->GetChildren().back();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_3", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(2, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_3_2", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_TRUE(curr->IsMarked());
    ASSERT_EQ(0, curr->GetChildren().size());
    }


//---------------------------------------------------------------------------------------
// @betest                                      Aidas.Kilinskas                06/2018
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_Multiple_MarksTheSpecifiedPathWhenItIsFoundFirst)
    {
    // create the hierarchy
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t) 1), "B"));
    NavNodePtr node = hierarchy[nullptr].back();
    hierarchy[node].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t) 2), "B_1"));
    NavNodePtr node1 = hierarchy[node].front();
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t) 3), "B_1_1"));
    NavNodePtr node1_1 = hierarchy[node1].front();
    m_impl->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 =
        {
        GetFirstInstanceKey(*node->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath2 =
        {
        GetFirstInstanceKey(*node->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_1->GetKey()->AsECInstanceNodeKey())
        };

    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodePaths(GetECDb(), keysPaths, 0).get();
    ASSERT_EQ(1, paths.size());

    NodesPathElement const* curr = &paths[0];
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_1", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_TRUE(curr->IsMarked());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_1_1", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Aidas.Kilinskas                06/2018
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_Multiple_MarksTheSpecifiedPathWhenItIsFoundNotFirst)
    {
    // create the hierarchy
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t) 1), "B"));
    NavNodePtr node = hierarchy[nullptr].back();
    hierarchy[node].push_back(CreateInstanceNode(*GetClass("Gadget"), ECInstanceId((uint64_t) 2), "B_1"));
    NavNodePtr node1 = hierarchy[node].front();
    hierarchy[node1].push_back(CreateInstanceNode(*GetClass("Widget"), ECInstanceId((uint64_t) 3), "B_1_1"));
    NavNodePtr node1_1 = hierarchy[node1].front();
    m_impl->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 =
        {
        GetFirstInstanceKey(*node->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1_1->GetKey()->AsECInstanceNodeKey())
        };
    bvector<ECInstanceKey> keysPath2 =
        {
        GetFirstInstanceKey(*node->GetKey()->AsECInstanceNodeKey()),
        GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
        };

    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodePaths(GetECDb(), keysPaths, 1).get();
    ASSERT_EQ(1, paths.size());

    NodesPathElement const* curr = &paths[0];
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_1", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_TRUE(curr->IsMarked());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_1_1", curr->GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Aidas.Vaiksnoras                10/2017
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetFilteredNodesPaths)
    {
    /* create the hierarchy
       assume underscored nodes are filtered nodes

             1
           / | \
         /   |  \
      _2     3   4
           /   \
          /     \
        _5       6
                / \
               /   \
             _7     8

    */
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateTreeNode(1, nullptr));
    NavNodePtr node1 = hierarchy[nullptr].back();
    hierarchy[node1].push_back(CreateTreeNode(2, node1, "match"));
    hierarchy[node1].push_back(CreateTreeNode(3, node1));
    hierarchy[node1].push_back(CreateTreeNode(4, node1));
    NavNodePtr node3 = *(hierarchy[node1].begin() + 1);
    hierarchy[node3].push_back(CreateTreeNode(5, node3, "match"));
    hierarchy[node3].push_back(CreateTreeNode(6, node3));
    NavNodePtr node6 = *(hierarchy[node3].begin() + 1);
    hierarchy[node6].push_back(CreateTreeNode(7, node6, "match"));
    hierarchy[node6].push_back(CreateTreeNode(8, node6));
    m_impl->SetHierarchy(hierarchy);

    bvector<NodesPathElement> paths = m_manager->GetFilteredNodePaths(GetECDb(), "match", Json::Value()).get();

    /* Validate path hierarchy

             1
           / |
         /   |
       2     3
           /   \
          /     \
         5       6
                /
               /
              7

    */

    ASSERT_EQ(1, paths.size());

    EXPECT_EQ(1, paths[0].GetNode()->GetNodeId());
    ASSERT_EQ(2, paths[0].GetChildren().size());

    EXPECT_EQ(2, paths[0].GetChildren()[0].GetNode()->GetNodeId());
    EXPECT_EQ(3, paths[0].GetChildren()[1].GetNode()->GetNodeId());

    ASSERT_EQ(2, paths[0].GetChildren()[1].GetChildren().size());
    EXPECT_EQ(5, paths[0].GetChildren()[1].GetChildren()[0].GetNode()->GetNodeId());
    EXPECT_EQ(6, paths[0].GetChildren()[1].GetChildren()[1].GetNode()->GetNodeId());

    ASSERT_EQ(1, paths[0].GetChildren()[1].GetChildren()[1].GetChildren().size());
    EXPECT_EQ(7, paths[0].GetChildren()[1].GetChildren()[1].GetChildren()[0].GetNode()->GetNodeId());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Elonas.Seviakovas               09/2018
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetFilteredNodesPaths_CountFilterTextOccurances)
    {
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    TestNavNodePtr root = CreateTreeNode(1, nullptr, "match");
    hierarchy[nullptr].push_back(root);
    hierarchy[root].push_back(CreateTreeNode(2, root, "Match"));
    hierarchy[root].push_back(CreateTreeNode(3, root, "match MATCH"));
    m_impl->SetHierarchy(hierarchy);

    bvector<NodesPathElement> paths = m_manager->GetFilteredNodePaths(GetECDb(), "match", Json::Value()).get();

    EXPECT_EQ(1, paths[0].GetFilteringData().GetOccurances());
    EXPECT_EQ(3, paths[0].GetFilteringData().GetChildrenOccurances());

    EXPECT_EQ(1, paths[0].GetChildren()[0].GetFilteringData().GetOccurances());
    EXPECT_EQ(2, paths[0].GetChildren()[1].GetFilteringData().GetOccurances());
    }
