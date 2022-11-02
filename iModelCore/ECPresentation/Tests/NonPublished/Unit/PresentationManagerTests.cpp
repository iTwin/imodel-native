/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../Helpers/StubRulesDrivenECPresentationManagerImpl.h"
#include "../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    ECPresentationManager* m_manager;

    RulesDrivenECPresentationManagerTests() : m_manager(nullptr) {}
    virtual ECPresentationManager::Impl* _CreateImpl(ECPresentationManager::Impl::Params const&) = 0;
    virtual void SetUp() override;
    virtual void TearDown() override;

    static void SetUpTestCase();
    static void TearDownTestCase();
    static ECDbR GetECDb() {return s_project->GetECDb();}

    IConnectionPtr GetConnection() {return m_manager->GetConnections().GetConnection(GetECDb());}
    ECClassCP GetClass(Utf8CP name) { return GetECDb().Schemas().GetClass("RulesEngineTest", name); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodePtr CreateInstanceNode(ECClassInstanceKey instanceKey, Utf8CP label)
        {
        TestNodesFactory nodesFactory(*GetConnection(), "spec-id", "ruleset-id");
        auto node = nodesFactory.CreateECInstanceNode(nullptr, instanceKey.GetClass()->GetId(), instanceKey.GetId(), label);
        node->SetNodeId(BeGuid(true));
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodePtr CreateTreeNode(BeGuidCR nodeId, NavNodeCPtr parent, Utf8CP label = "")
        {
        TestNodesFactory nodesFactory(*GetConnection(), "spec-id", "ruleset-id");
        NavNodePtr node = nodesFactory.CreateCustomNode(parent.IsValid() ? parent->GetKey().get() : nullptr, label, "", "", "test-type");
        node->SetNodeId(nodeId);
        if (parent.IsValid())
            node->SetParentNodeId(parent->GetNodeId());
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodePtr CreateInstanceNode(ECClassCR ecClass, ECInstanceId instanceId, Utf8CP label)
        {
        return CreateInstanceNode(ECClassInstanceKey(ecClass, instanceId), label);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodePtr CreateInstanceNode(ECInstanceKey instanceKey, Utf8CP label)
        {
        ECClassCP ecClass = GetECDb().Schemas().GetClass(instanceKey.GetClassId());
        return CreateInstanceNode(*ecClass, instanceKey.GetInstanceId(), label);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodePtr CreateClassGroupingNode(ECClassCR ecClass, Utf8CP label, bvector<ECInstanceKey> const& groupedKeys)
        {
        TestNodesFactory nodesFactory(*GetConnection(), "spec-id", "ruleset-id");
        NavNodePtr node = nodesFactory.CreateECClassGroupingNode(nullptr, ecClass, false, label, (uint64_t)groupedKeys.size());
        node->SetNodeId(BeGuid(true));
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodePtr CreatePropertyGroupingNode(ECClassCR ecClass, Utf8CP propertyName, Utf8CP label, bool isRangeGrouping, bvector<ECValue> const& groupedValues, bvector<ECInstanceKey> const& groupedKeys)
        {
        TestNodesFactory nodesFactory(*GetConnection(), "spec-id", "ruleset-id");
        ECPropertyCP ecProperty = ecClass.GetPropertyP(propertyName);
        if (nullptr == ecProperty)
            {
            BeAssert(false);
            return nullptr;
            }

        rapidjson::Document groupedValuesJson(rapidjson::kArrayType);
        for (ECValueCR value : groupedValues)
            groupedValuesJson.PushBack(ValueHelpers::GetJsonFromECValue(value, &groupedValuesJson.GetAllocator()), groupedValuesJson.GetAllocator());

        NavNodePtr node = nodesFactory.CreateECPropertyGroupingNode(nullptr, ecClass, *ecProperty, label, "", groupedValuesJson, isRangeGrouping, (uint64_t)groupedKeys.size());
        node->SetNodeId(BeGuid(true));
        return node;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodePtr CreateLabelGroupingNode(Utf8CP label, bvector<ECInstanceKey> const& groupedKeys)
        {
        TestNodesFactory nodesFactory(*GetConnection(), "spec-id", "ruleset-id");
        NavNodePtr node = nodesFactory.CreateDisplayLabelGroupingNode(nullptr, label, (uint64_t)groupedKeys.size());
        node->SetNodeId(BeGuid(true));
        return node;
        }
    };
ECDbTestProject* RulesDrivenECPresentationManagerTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("RulesDrivenECPresentationManagerTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::TearDownTestCase()
    {
    s_project->GetECDb().CloseDb();
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::SetUp()
    {
    m_manager = new ECPresentationManager(ECPresentationManager::Params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost())));

    ECPresentationManager::Params params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    params.SetConnections(std::make_shared<TestConnectionManager>());
    m_manager->SetImpl(*_CreateImpl(*m_manager->CreateImplParams(params)));

    m_manager->GetConnections().CreateConnection(GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerTests::TearDown()
    {
    DELETE_AND_CLEAR(m_manager);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerStubbedImplTests : RulesDrivenECPresentationManagerTests
    {
    StubRulesDrivenECPresentationManagerImpl* m_impl;

    virtual ECPresentationManager::Impl* _CreateImpl(ECPresentationManager::Impl::Params const& params) override
        {
        m_impl = new StubRulesDrivenECPresentationManagerImpl(params);
        return m_impl;
        }
    };

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
static ECInstanceKey GetFirstInstanceKey(ECInstancesNodeKey const& nodeKey)
    {
    ECClassInstanceKeyCR classKey = nodeKey.GetInstanceKeys().front();
    return ECInstanceKey(classKey.GetClass()->GetId(), classKey.GetId());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByInstanceKeys_InstancesHierarchy)
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
    bvector<bvector<ECInstanceKey>> keyPaths =
        {
            {
            GetFirstInstanceKey(*node1->GetKey()->AsECInstanceNodeKey()),
            GetFirstInstanceKey(*node2->GetKey()->AsECInstanceNodeKey()),
            GetFirstInstanceKey(*node3->GetKey()->AsECInstanceNodeKey())
            }
        };

    // test
    NodesPathElement path = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams::Create(GetECDb(), "", RulesetVariables(), keyPaths))).front();

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
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByInstanceKeys_InstancesHierarchyWithGrouping)
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
    hierarchy[node1].push_back(CreatePropertyGroupingNode(*GetClass("Widget"), "IntProperty", "A_1", true, {ECValue(1)}, {instanceKey4}));
    hierarchy[node1].push_back(CreatePropertyGroupingNode(*GetClass("Widget"), "BoolProperty", "A_2", true, {ECValue(2)}, {instanceKey1, instanceKey2, instanceKey3}));
    hierarchy[node1].push_back(CreatePropertyGroupingNode(*GetClass("Widget"), "DoubleProperty", "A_3", true, {ECValue(3)}, {instanceKey5}));
    NavNodePtr node2 = *(hierarchy[node1].begin() + 1);
    hierarchy[node2].push_back(CreateLabelGroupingNode("A_2_1", {instanceKey3}));
    hierarchy[node2].push_back(CreateLabelGroupingNode("A_2_2", {instanceKey1, instanceKey2}));
    NavNodePtr node3 = hierarchy[node2].back();
    hierarchy[node3].push_back(CreateInstanceNode(instanceKey1, "A_2_2_1"));
    hierarchy[node3].push_back(CreateInstanceNode(instanceKey2, "A_2_2_2"));
    NavNodePtr node4 = hierarchy[node3].front();
    m_impl->SetHierarchy(hierarchy);
    m_impl->SetNodeInstanceKeysProviderFactory([&]()
        {
        auto provider = std::make_unique<StubNodeInstanceKeysProvider>();
        provider->SetContainsFunc([&](NavNodeCR node, ECInstanceKeyCR key)
            {
            if (node.GetLabelDefinition().GetDisplayValue().Equals("A"))
                return key == instanceKey1 || key == instanceKey2 || key == instanceKey3 || key == instanceKey4 || key == instanceKey5;
            if (node.GetLabelDefinition().GetDisplayValue().Equals("A_1"))
                return key == instanceKey4;
            if (node.GetLabelDefinition().GetDisplayValue().Equals("A_2"))
                return key == instanceKey1 || key == instanceKey2 || key == instanceKey3;
            if (node.GetLabelDefinition().GetDisplayValue().Equals("A_2_1"))
                return key == instanceKey3;
            if (node.GetLabelDefinition().GetDisplayValue().Equals("A_2_2"))
                return key == instanceKey1 || key == instanceKey2;
            if (node.GetLabelDefinition().GetDisplayValue().Equals("A_3"))
                return key == instanceKey5;
            if (node.GetLabelDefinition().GetDisplayValue().Equals("B"))
                return key == instanceKey6;
            return false;
            });
        return provider;
        });

    // create the keys path
    bvector<bvector<ECInstanceKey>> keyPaths = {
        { GetFirstInstanceKey(*node4->GetKey()->AsECInstanceNodeKey()) }
        };

    // test
    NodesPathElement path = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams::Create(GetECDb(), "", RulesetVariables(), keyPaths))).front();

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
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByInstanceKeys_Multiple_ReturnsTwoSeparatePathsWhenTheyDontIntersect)
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
    bvector<NodesPathElement> paths = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams::Create(GetECDb(), "", RulesetVariables(), keysPaths)));
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
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByInstanceKeys_Multiple_ReturnsMergedPathWhenPathsIntersect)
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
    bvector<NodesPathElement> paths = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams::Create(GetECDb(), "", RulesetVariables(), keysPaths)));
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
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByInstanceKeys_Multiple_MarksTheSpecifiedPath)
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
    bvector<NodesPathElement> paths = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams::Create(GetECDb(), "", RulesetVariables(), keysPaths, 2)));
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
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByInstanceKeys_Multiple_MarksTheSpecifiedPathWhenItIsFoundFirst)
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
    bvector<NodesPathElement> paths = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams::Create(GetECDb(), "", RulesetVariables(), keysPaths, 0)));
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
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByInstanceKeys_Multiple_MarksTheSpecifiedPathWhenItIsFoundNotFirst)
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
    bvector<NodesPathElement> paths = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams::Create(GetECDb(), "", RulesetVariables(), keysPaths, 1)));
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
// @betest
//---------------------------------------------------------------------------------------
static bvector<BeGuid> GenerateIds(int count)
    {
    bvector<BeGuid> ids;
    for (int i = 0; i < count; ++i)
        ids.push_back(BeGuid(true));

    std::sort(ids.begin(), ids.end());
    return ids;
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByFilterText)
    {
    /* create the hierarchy
       assume underscored nodes are filtered nodes

             0
           / | \
         /   |  \
      _1     2   3
           /   \
          /     \
        _4       5
                / \
               /   \
             _6     7

    */
    bvector<BeGuid> nodeIds = GenerateIds(8);
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    hierarchy[nullptr].push_back(CreateTreeNode(nodeIds[0], nullptr));
    NavNodePtr node0 = hierarchy[nullptr].back();
    hierarchy[node0].push_back(CreateTreeNode(nodeIds[1], node0, "match"));
    hierarchy[node0].push_back(CreateTreeNode(nodeIds[2], node0));
    hierarchy[node0].push_back(CreateTreeNode(nodeIds[3], node0));
    NavNodePtr node2 = *(hierarchy[node0].begin() + 1);
    hierarchy[node2].push_back(CreateTreeNode(nodeIds[4], node2, "match"));
    hierarchy[node2].push_back(CreateTreeNode(nodeIds[5], node2));
    NavNodePtr node5 = *(hierarchy[node2].begin() + 1);
    hierarchy[node5].push_back(CreateTreeNode(nodeIds[6], node5, "match"));
    hierarchy[node5].push_back(CreateTreeNode(nodeIds[7], node5));
    m_impl->SetHierarchy(hierarchy);

    bvector<NodesPathElement> paths = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(GetECDb(), "", RulesetVariables(), "match")));

    /* Validate path hierarchy

             0
           / |
         /   |
       1     2
           /   \
          /     \
         4       5
                /
               /
              6

    */

    ASSERT_EQ(1, paths.size());

    EXPECT_EQ(nodeIds[0], paths[0].GetNode()->GetNodeId());
    ASSERT_EQ(2, paths[0].GetChildren().size());

    EXPECT_EQ(nodeIds[1], paths[0].GetChildren()[0].GetNode()->GetNodeId());
    EXPECT_EQ(nodeIds[2], paths[0].GetChildren()[1].GetNode()->GetNodeId());

    ASSERT_EQ(2, paths[0].GetChildren()[1].GetChildren().size());
    EXPECT_EQ(nodeIds[4], paths[0].GetChildren()[1].GetChildren()[0].GetNode()->GetNodeId());
    EXPECT_EQ(nodeIds[5], paths[0].GetChildren()[1].GetChildren()[1].GetNode()->GetNodeId());

    ASSERT_EQ(1, paths[0].GetChildren()[1].GetChildren()[1].GetChildren().size());
    EXPECT_EQ(nodeIds[6], paths[0].GetChildren()[1].GetChildren()[1].GetChildren()[0].GetNode()->GetNodeId());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByFilterText_CountFilterTextOccurancesWithLowercaseInput)
    {
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    bvector<BeGuid> nodeIds = GenerateIds(3);
    NavNodePtr root = CreateTreeNode(nodeIds[0], nullptr, "match");
    hierarchy[nullptr].push_back(root);
    hierarchy[root].push_back(CreateTreeNode(nodeIds[1], root, "Match"));
    hierarchy[root].push_back(CreateTreeNode(nodeIds[2], root, "match MATCH"));
    m_impl->SetHierarchy(hierarchy);

    bvector<NodesPathElement> paths = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(GetECDb(), "", RulesetVariables(), "match")));

    EXPECT_EQ(1, paths[0].GetFilteringData().GetOccurances());
    EXPECT_EQ(3, paths[0].GetFilteringData().GetChildrenOccurances());

    EXPECT_EQ(1, paths[0].GetChildren()[0].GetFilteringData().GetOccurances());
    EXPECT_EQ(2, paths[0].GetChildren()[1].GetFilteringData().GetOccurances());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RulesDrivenECPresentationManagerStubbedImplTests, GetNodePaths_ByFilterText_CountFilterTextOccurancesWithUppercaseInput)
    {
    StubRulesDrivenECPresentationManagerImpl::Hierarchy hierarchy;
    bvector<BeGuid> nodeIds = GenerateIds(3);
    NavNodePtr root = CreateTreeNode(nodeIds[0], nullptr, "match");
    hierarchy[nullptr].push_back(root);
    hierarchy[root].push_back(CreateTreeNode(nodeIds[1], root, "Match"));
    hierarchy[root].push_back(CreateTreeNode(nodeIds[2], root, "match MATCH"));
    m_impl->SetHierarchy(hierarchy);

    bvector<NodesPathElement> paths = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(GetECDb(), "", RulesetVariables(), "MATCH")));

    EXPECT_EQ(1, paths[0].GetFilteringData().GetOccurances());
    EXPECT_EQ(3, paths[0].GetFilteringData().GetChildrenOccurances());

    EXPECT_EQ(1, paths[0].GetChildren()[0].GetFilteringData().GetOccurances());
    EXPECT_EQ(2, paths[0].GetChildren()[1].GetFilteringData().GetOccurances());
    }