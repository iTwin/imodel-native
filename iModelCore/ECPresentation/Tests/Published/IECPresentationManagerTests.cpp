/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/IECPresentationManagerTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/ECPresentation/TestECPresentationManager.h>
#include "../NonPublished/RulesEngine/ECDbTestProject.h"
#include "../NonPublished/RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2016
+===============+===============+===============+===============+===============+======*/
struct IECPresentationManagerTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    TestECPresentationManager* m_manager;
    ECClassId m_widgetClassId;
    ECClassId m_gadgetClassId;
    ECClassId m_sprocketClassId;
    
    IECPresentationManagerTests() : m_manager(nullptr) {}
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("IECPresentationManagerTests", "RulesEngineTest.01.00.ecschema.xml");
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        delete s_project;
        s_project = nullptr;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetUp() override
        {
        ECPresentationTest::SetUp();
        Localization::Init();

        m_manager = new TestECPresentationManager(m_connections);
        IECPresentationManager::RegisterImplementation(m_manager);

        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());

        m_widgetClassId = s_project->GetECDb().Schemas().GetClassId("RulesEngineTest", "Widget");
        m_gadgetClassId = s_project->GetECDb().Schemas().GetClassId("RulesEngineTest", "Gadget");
        m_sprocketClassId = s_project->GetECDb().Schemas().GetClassId("RulesEngineTest", "Sprocket");
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void TearDown() override
        {
        IECPresentationManager::RegisterImplementation(nullptr);
        delete m_manager;
        Localization::Terminate();
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static uint64_t CreateNodeId() {static uint64_t id = 0; return ++id;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateInstanceNode(ECInstanceKey instanceKey, Utf8CP label)
        {
        ECClassCP ecClass = s_project->GetECDb().Schemas().GetClass(instanceKey.GetClassId());
        if (nullptr == ecClass)
            {
            BeAssert(false);
            return nullptr;
            }
        TestNavNodePtr node = TestNodesHelper::CreateInstanceNode(*m_connection, *ecClass, instanceKey.GetInstanceId());
        node->SetNodeId(CreateNodeId());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*m_connection, *node, bvector<Utf8String>{std::to_string(node->GetNodeId()).c_str()}));
        node->SetLabel(label);
        return node;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Aidas.Vaisknoras                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateTreeNode(uint64_t nodeId, NavNodeCPtr parent)
        {
        TestNavNodePtr node = TestNavNode::Create(*m_connection);
        node->SetNodeId(nodeId);
        bvector<Utf8String> path = (parent.IsNull()) ? bvector<Utf8String>() : parent->GetKey()->GetPathFromRoot();
        path.push_back(std::to_string(node->GetNodeId()).c_str());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*m_connection, *node, path));
        if (parent.IsValid())
            node->SetParentNodeId(parent->GetNodeId());
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateInstanceNode(ECClassId classId, ECInstanceId instanceId, Utf8CP label)
        {
        return CreateInstanceNode(ECInstanceKey(classId, instanceId), label);
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateClassGroupingNode(ECClassId classId, Utf8CP label, bvector<ECInstanceKey> const& groupedKeys)
        {
        ECClassCP ecClass = s_project->GetECDb().Schemas().GetClass(classId);
        if (nullptr == ecClass)
            {
            BeAssert(false);
            return nullptr;
            }
        TestNavNodePtr node = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClass, label);
        node->SetNodeId(CreateNodeId());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*m_connection, *node, bvector<Utf8String>{std::to_string(node->GetNodeId()).c_str()}));
        NavNodeExtendedData(*node).SetGroupedInstanceKeys(groupedKeys);
        return node;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreatePropertyGroupingNode(ECClassId classId, Utf8CP propertyName, Utf8CP label, int rangeIndex, rapidjson::Value const* groupingValueP, bvector<ECInstanceKey> const& groupedKeys)
        {
        ECClassCP ecClass = s_project->GetECDb().Schemas().GetClass(classId);
        if (nullptr == ecClass)
            {
            BeAssert(false);
            return nullptr;
            }
        ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
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
        TestNavNodePtr node = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClass, *ecProperty, label, groupingValue, -1 != rangeIndex);
        node->SetNodeId(CreateNodeId());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*m_connection, *node, bvector<Utf8String>{std::to_string(node->GetNodeId()).c_str()}));
        NavNodeExtendedData(*node).SetGroupedInstanceKeys(groupedKeys);
        return node;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestNavNodePtr CreateLabelGroupingNode(Utf8CP label, bvector<ECInstanceKey> const& groupedKeys)
        {
        TestNavNodePtr node = TestNodesHelper::CreateLabelGroupingNode(*m_connection, label);
        node->SetNodeId(CreateNodeId());
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*m_connection, *node, bvector<Utf8String>{std::to_string(node->GetNodeId()).c_str()}));
        NavNodeExtendedData(*node).SetGroupedInstanceKeys(groupedKeys);
        return node;
        }
    };
ECDbTestProject* IECPresentationManagerTests::s_project = nullptr;

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(IECPresentationManagerTests, GetNodesPath_InstancesHierarchy)
    {
    // create the hierarchy
    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)1), "A"));
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)2), "B"));
    NavNodeCPtr node1 = hierarchy[nullptr].back();
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)1), "B_1"));
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)2), "B_2"));
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)3), "B_3"));
    NavNodeCPtr node2 = *(hierarchy[node1].begin() + 1);
    hierarchy[node2].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)3), "B_2_1"));
    hierarchy[node2].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)4), "B_2_2"));
    NavNodeCPtr node3 = hierarchy[node2].front();
    m_manager->SetHierarchy(hierarchy);

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
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };

    // test
    NodesPathElement path = m_manager->GetNodesPath(s_project->GetECDb(), keysPath).get();

    NodesPathElement const* curr = &path;
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_2", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_2_1", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(IECPresentationManagerTests, GetNodesPath_InstancesHierarchyWithGrouping)
    {
    // need to override HasChild function to get this working
    m_manager->SetHasChildHandler([](IConnectionCR, NavNodeCR parent, ECInstanceKeyCR childKey, JsonValueCR) -> bool
        {
        bvector<ECInstanceKey> keys = NavNodeExtendedData(parent).GetGroupedInstanceKeys();
        return keys.end() != std::find(keys.begin(), keys.end(), childKey);
        });

    ECInstanceKey instanceKey1(m_widgetClassId, ECInstanceId((uint64_t)3));
    ECInstanceKey instanceKey2(m_widgetClassId, ECInstanceId((uint64_t)4));
    ECInstanceKey instanceKey3(m_gadgetClassId, ECInstanceId((uint64_t)1));
    ECInstanceKey instanceKey4(m_widgetClassId, ECInstanceId((uint64_t)5));
    ECInstanceKey instanceKey5(m_widgetClassId, ECInstanceId((uint64_t)6));
    ECInstanceKey instanceKey6(m_widgetClassId, ECInstanceId((uint64_t)7));

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
    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateClassGroupingNode(m_widgetClassId, "A", {instanceKey1, instanceKey2, instanceKey3, instanceKey4, instanceKey5}));
    hierarchy[nullptr].push_back(CreateClassGroupingNode(m_gadgetClassId, "B", {instanceKey6}));
    NavNodeCPtr node1 = hierarchy[nullptr].front();
    hierarchy[node1].push_back(CreatePropertyGroupingNode(m_widgetClassId, "IntProperty", "A_1", 1, nullptr, {instanceKey4}));
    hierarchy[node1].push_back(CreatePropertyGroupingNode(m_widgetClassId, "BoolProperty", "A_2", 2, nullptr, {instanceKey1, instanceKey2, instanceKey3}));
    hierarchy[node1].push_back(CreatePropertyGroupingNode(m_widgetClassId, "DoubleProperty", "A_3", 3, nullptr, {instanceKey5}));
    NavNodeCPtr node2 = *(hierarchy[node1].begin() + 1);
    hierarchy[node2].push_back(CreateLabelGroupingNode("A_2_1", {instanceKey3}));
    hierarchy[node2].push_back(CreateLabelGroupingNode("A_2_2", {instanceKey1, instanceKey2}));
    NavNodeCPtr node3 = hierarchy[node2].back();
    hierarchy[node3].push_back(CreateInstanceNode(instanceKey1, "A_2_2_1"));
    hierarchy[node3].push_back(CreateInstanceNode(instanceKey2, "A_2_2_2"));
    NavNodeCPtr node4 = hierarchy[node3].front();
    m_manager->SetHierarchy(hierarchy);

    // create the keys path
    bvector<ECInstanceKey> keysPath = {node4->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()};

    // test
    NodesPathElement path = m_manager->GetNodesPath(s_project->GetECDb(), keysPath).get();

    NodesPathElement const* curr = &path;
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());
    
    curr = &curr->GetChildren().front();
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A_2", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());
    
    curr = &curr->GetChildren().front();
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A_2_2", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());
    
    curr = &curr->GetChildren().front();
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A_2_2_1", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(IECPresentationManagerTests, GetNodesPath_Multiple_ReturnsTwoSeparatePathsWhenTheyDontIntersect)
    {
    // create the hierarchy
    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)1), "A"));
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)2), "B"));
    NavNodeCPtr node1_1 = hierarchy[nullptr].front();
    hierarchy[node1_1].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)1), "A_1"));
    hierarchy[node1_1].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)2), "A_2"));
    hierarchy[node1_1].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)3), "A_3"));
    NavNodeCPtr node1_2 = hierarchy[node1_1].back();
    hierarchy[node1_2].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)3), "A_2_1"));
    hierarchy[node1_2].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)4), "A_2_2"));
    NavNodeCPtr node2_1 = hierarchy[nullptr].back();
    hierarchy[node2_1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)1), "B_1"));
    hierarchy[node2_1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)2), "B_2"));
    hierarchy[node2_1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)3), "B_3"));
    NavNodeCPtr node2_2 = *(hierarchy[node2_1].begin() + 1);
    hierarchy[node2_2].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)3), "B_2_1"));
    hierarchy[node2_2].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)4), "B_2_2"));
    NavNodeCPtr node2_3 = hierarchy[node2_2].front();
    m_manager->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 = 
        {
        node1_1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node1_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath2 = 
        {
        node2_1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node2_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node2_3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodesPath(s_project->GetECDb(), keysPaths, -1).get();
    ASSERT_EQ(2, paths.size());

    // 1st branch
    NodesPathElement const* curr = &paths[0];
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(2, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("A_3", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(0, curr->GetChildren().size());
    
    // 2nd branch
    curr = &paths[1];
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_2", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    EXPECT_EQ(0, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_2_1", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(IECPresentationManagerTests, GetNodesPath_Multiple_ReturnsMergedPathWhenPathsIntersect)
    {
    // create the hierarchy
    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)1), "A"));
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)2), "B"));
    NavNodeCPtr node1 = hierarchy[nullptr].back();
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)1), "B_1"));
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)2), "B_2"));
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)3), "B_3"));
    NavNodeCPtr node1_2 = hierarchy[node1].front();
    hierarchy[node1_2].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)3), "B_1_1"));
    hierarchy[node1_2].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)4), "B_1_2"));
    NavNodeCPtr node1_3 = hierarchy[node1_2].front();
    NavNodeCPtr node2_2 = hierarchy[node1].back();
    hierarchy[node2_2].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)3), "B_3_1"));
    hierarchy[node2_2].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)4), "B_3_2"));
    NavNodeCPtr node21_3 = hierarchy[node2_2].back();
    NavNodeCPtr node22_3 = hierarchy[node2_2].front();
    m_manager->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 = 
        {
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node1_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath2 = 
        {
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node1_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node1_3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath3 = 
        {
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node2_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node21_3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath4 = 
        {
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node2_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node22_3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2, keysPath3, keysPath4};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodesPath(s_project->GetECDb(), keysPaths, -1).get();
    ASSERT_EQ(1, paths.size());

    NodesPathElement const* curr = &paths[0];    
    EXPECT_EQ(1, curr->GetIndex());
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabel().c_str());
    ASSERT_EQ(2, curr->GetChildren().size());

    NodesPathElement const* curr1 = &curr->GetChildren()[0];
    EXPECT_EQ(0, curr1->GetIndex());
    ASSERT_TRUE(curr1->GetNode().IsValid());
    EXPECT_STREQ("B_1", curr1->GetNode()->GetLabel().c_str());
    ASSERT_EQ(1, curr1->GetChildren().size());

    curr1 = &curr1->GetChildren().front();
    EXPECT_EQ(0, curr1->GetIndex());
    ASSERT_TRUE(curr1->GetNode().IsValid());
    EXPECT_STREQ("B_1_1", curr1->GetNode()->GetLabel().c_str());
    ASSERT_EQ(0, curr1->GetChildren().size());
    
    NodesPathElement const* curr2 = &curr->GetChildren()[1];
    EXPECT_EQ(2, curr2->GetIndex());
    ASSERT_TRUE(curr2->GetNode().IsValid());
    EXPECT_STREQ("B_3", curr2->GetNode()->GetLabel().c_str());
    ASSERT_EQ(2, curr2->GetChildren().size());

    NodesPathElement const* curr21 = &curr2->GetChildren()[0];
    EXPECT_EQ(1, curr21->GetIndex());
    ASSERT_TRUE(curr21->GetNode().IsValid());
    EXPECT_STREQ("B_3_2", curr21->GetNode()->GetLabel().c_str());
    ASSERT_EQ(0, curr21->GetChildren().size());
    
    NodesPathElement const* curr22 = &curr2->GetChildren()[1];
    EXPECT_EQ(0, curr22->GetIndex());
    ASSERT_TRUE(curr22->GetNode().IsValid());
    EXPECT_STREQ("B_3_1", curr22->GetNode()->GetLabel().c_str());
    ASSERT_EQ(0, curr22->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                12/2016
//---------------------------------------------------------------------------------------
TEST_F(IECPresentationManagerTests, GetNodesPath_Multiple_MarksTheSpecifiedPath)
    {
    // create the hierarchy
    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)1), "A"));
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)2), "B"));
    NavNodeCPtr node1 = hierarchy[nullptr].back();
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)1), "B_1"));
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)2), "B_2"));
    hierarchy[node1].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t)3), "B_3"));
    NavNodeCPtr node1_2 = hierarchy[node1].front();
    hierarchy[node1_2].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)3), "B_1_1"));
    hierarchy[node1_2].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)4), "B_1_2"));
    NavNodeCPtr node1_3 = hierarchy[node1_2].front();
    NavNodeCPtr node2_2 = hierarchy[node1].back();
    hierarchy[node2_2].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t)3), "B_3_1"));
    hierarchy[node2_2].push_back(CreateInstanceNode(m_sprocketClassId, ECInstanceId((uint64_t)4), "B_3_2"));
    NavNodeCPtr node21_3 = hierarchy[node2_2].back();
    NavNodeCPtr node22_3 = hierarchy[node2_2].front();
    m_manager->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 = 
        {
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node1_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath2 = 
        {
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node1_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node1_3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath3 = 
        {
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node2_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node21_3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath4 = 
        {
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node2_2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(), 
        node22_3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2, keysPath3, keysPath4};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodesPath(s_project->GetECDb(), keysPaths, 2).get();
    ASSERT_EQ(1, paths.size());

    NodesPathElement const* curr = &paths[0];
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabel().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(2, curr->GetChildren().size());

    curr = &curr->GetChildren().back();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_3", curr->GetNode()->GetLabel().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(2, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_3_2", curr->GetNode()->GetLabel().c_str());
    EXPECT_TRUE(curr->IsMarked());
    ASSERT_EQ(0, curr->GetChildren().size());
    }


//---------------------------------------------------------------------------------------
// @betest                                      Aidas.Kilinskas                06/2018
//---------------------------------------------------------------------------------------
TEST_F(IECPresentationManagerTests, GetNodesPath_Multiple_MarksTheSpecifiedPathWhenItIsFoundFirst)
    {
    // create the hierarchy
    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t) 1), "B"));
    NavNodeCPtr node = hierarchy[nullptr].back();
    hierarchy[node].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t) 2), "B_1"));
    NavNodeCPtr node1 = hierarchy[node].front();
    hierarchy[node1].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t) 3), "B_1_1"));
    NavNodeCPtr node1_1 = hierarchy[node1].front();

    m_manager->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 =
        {
        node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(),
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath2 =
        {
        node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(),
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(),
        node1_1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };

    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodesPath(s_project->GetECDb(), keysPaths, 0).get();
    ASSERT_EQ(1, paths.size());

    NodesPathElement const* curr = &paths[0];
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabel().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_1", curr->GetNode()->GetLabel().c_str());
    EXPECT_TRUE(curr->IsMarked());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_1_1", curr->GetNode()->GetLabel().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Aidas.Kilinskas                06/2018
//---------------------------------------------------------------------------------------
TEST_F(IECPresentationManagerTests, GetNodesPath_Multiple_MarksTheSpecifiedPathWhenItIsFoundNotFirst)
    {
    // create the hierarchy
    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t) 1), "B"));
    NavNodeCPtr node = hierarchy[nullptr].back();
    hierarchy[node].push_back(CreateInstanceNode(m_gadgetClassId, ECInstanceId((uint64_t) 2), "B_1"));
    NavNodeCPtr node1 = hierarchy[node].front();
    hierarchy[node1].push_back(CreateInstanceNode(m_widgetClassId, ECInstanceId((uint64_t) 3), "B_1_1"));
    NavNodeCPtr node1_1 = hierarchy[node1].front();

    m_manager->SetHierarchy(hierarchy);

    // create the key paths
    bvector<ECInstanceKey> keysPath1 =
        {
        node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(),
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(),
        node1_1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey()
        };
    bvector<ECInstanceKey> keysPath2 =
        {
        node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(),
        node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey(),
        };

    bvector<bvector<ECInstanceKey>> keysPaths = {keysPath1, keysPath2};

    // test
    bvector<NodesPathElement> paths = m_manager->GetNodesPath(s_project->GetECDb(), keysPaths, 1).get();
    ASSERT_EQ(1, paths.size());

    NodesPathElement const* curr = &paths[0];
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B", curr->GetNode()->GetLabel().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_1", curr->GetNode()->GetLabel().c_str());
    EXPECT_TRUE(curr->IsMarked());
    ASSERT_EQ(1, curr->GetChildren().size());

    curr = &curr->GetChildren().front();
    ASSERT_TRUE(curr->GetNode().IsValid());
    EXPECT_STREQ("B_1_1", curr->GetNode()->GetLabel().c_str());
    EXPECT_FALSE(curr->IsMarked());
    ASSERT_EQ(0, curr->GetChildren().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Aidas.Vaiksnoras                10/2017
//---------------------------------------------------------------------------------------
TEST_F(IECPresentationManagerTests, GetFilteredNodesPaths)
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
    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateTreeNode(1, nullptr));
    NavNodeCPtr node1 = hierarchy[nullptr].back();
    hierarchy[node1].push_back(CreateTreeNode(2, node1));
    hierarchy[node1].push_back(CreateTreeNode(3, node1));
    hierarchy[node1].push_back(CreateTreeNode(4, node1));
    NavNodeCPtr node3 = *(hierarchy[node1].begin() + 1);
    hierarchy[node3].push_back(CreateTreeNode(5, node3));
    hierarchy[node3].push_back(CreateTreeNode(6, node3));
    NavNodeCPtr node6 = *(hierarchy[node3].begin() + 1);
    hierarchy[node6].push_back(CreateTreeNode(7, node6));
    hierarchy[node6].push_back(CreateTreeNode(8, node6));
    m_manager->SetHierarchy(hierarchy);


    m_manager->SetGetFilteredNodesPathsHandler([&](IConnectionCR, Utf8CP filterText, JsonValueCR options)
        {
        bvector<NavNodeCPtr> nodelist;
        nodelist.push_back(*(hierarchy[node1].begin()));   //2
        nodelist.push_back(*(hierarchy[node3].begin()));   //5
        nodelist.push_back(*(hierarchy[node6].begin()));   //7
        return nodelist;
        });

    bvector<NodesPathElement> paths = m_manager->GetFilteredNodesPaths(s_project->GetECDb(), "T", Json::Value()).get();

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
TEST_F(IECPresentationManagerTests, GetFilteredNodesPaths_CountFilterTextOccurances)
    {
    TestNavNodePtr nodeP = CreateTreeNode(1, nullptr);
    nodeP->SetLabel("Some label with dgn");
    TestNavNodePtr nodeC1 = CreateTreeNode(2, nodeP);
    nodeC1->SetLabel("dgn in the front");
    TestNavNodePtr nodeC2 = CreateTreeNode(3, nodeP);
    nodeC2->SetLabel("Capital DgN and second _dgn");

    Hierarchy hierarchy;
    hierarchy[nullptr].push_back(nodeP);
    hierarchy[nodeP].push_back(nodeC1);
    hierarchy[nodeP].push_back(nodeC2);
    m_manager->SetHierarchy(hierarchy);

    m_manager->SetGetFilteredNodesPathsHandler([&](IConnectionCR, Utf8CP filterText, JsonValueCR options)
    {
        bvector<NavNodeCPtr> nodelist;
        nodelist.push_back(nodeP);
        nodelist.push_back(nodeC1);
        nodelist.push_back(nodeC2);
        return nodelist;
    });

    bvector<NodesPathElement> paths = m_manager->GetFilteredNodesPaths(s_project->GetECDb(), "dGn", Json::Value()).get();

    EXPECT_EQ(1, paths[0].GetFilteringData().GetOccurances());
    EXPECT_EQ(3, paths[0].GetFilteringData().GetChildrenOccurances());

    EXPECT_EQ(1, paths[0].GetChildren()[0].GetFilteringData().GetOccurances());
    EXPECT_EQ(2, paths[0].GetChildren()[1].GetFilteringData().GetOccurances());
    }
