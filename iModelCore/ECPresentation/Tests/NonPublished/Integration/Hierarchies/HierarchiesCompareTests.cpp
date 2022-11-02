/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../PresentationManagerIntegrationTests.h"
#include "../../../../Source/PresentationManagerImpl.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestChangeRecordsHandler : IHierarchyChangeRecordsHandler
{
private:
    bvector<HierarchyChangeRecord> m_records;
protected:
    void _Start() override {m_records.clear();}
    void _Accept(HierarchyChangeRecord const& record) override {m_records.push_back(record);}
    void _Finish() override {}
public:
    bvector<HierarchyChangeRecord> const& GetRecords() const {return m_records;}
    void Clear() {m_records.clear();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchiesCompareTests : PresentationManagerIntegrationTests
    {
    std::shared_ptr<TestChangeRecordsHandler> m_changeRecordsHandler;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _ConfigureManagerParams(ECPresentationManager::Params& params) override
        {
        PresentationManagerIntegrationTests::_ConfigureManagerParams(params);
        params.SetMode(ECPresentationManager::Mode::ReadOnly);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetUp() override
        {
        m_changeRecordsHandler = std::make_shared<TestChangeRecordsHandler>();
        PresentationManagerIntegrationTests::SetUp();
        }
    };

enum Side { LHS, RHS };
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateRulesetName(Side side)
    {
    return Utf8PrintfString("%s:%s", BeTest::GetNameOfCurrentTest(), side == LHS ? "LHS" : side == RHS ? "RHS" : "?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
static CustomNodeSpecificationP CreateCustomNodeSpec(Utf8StringCR identifier)
    {
    auto spec = new CustomNodeSpecification(1, false, identifier, identifier, "", "");
    spec->SetHasChildren(ChildrenHint::Unknown);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchiesCompareTests, DetectsNoCustomNodeChangesWhenRulesetsAndVariablesAreEqual)
    {
    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_NODE"));
    lhs->AddPresentationRule(*rootRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));
    m_locater->AddRuleSet(*rhs);

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>())).wait();

    // verify
    ASSERT_TRUE(m_changeRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsNoECInstanceNodeChangesWhenRulesetsAndVariablesAreEqual, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(HierarchiesCompareTests, DetectsNoECInstanceNodeChangesWhenRulesetsAndVariablesAreEqual)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*rootRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));
    m_locater->AddRuleSet(*rhs);

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>())).wait();

    // verify
    ASSERT_TRUE(m_changeRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRuleset_DetectsRootNodeChangesWhenInstanceFilterIsModified, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, ByRuleset_DetectsRootNodeChangesWhenInstanceFilterIsModified)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(3));});

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = 1 OR this.Prop = 3", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = 2 OR this.Prop = 3", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsRule);

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>())).wait();

    // verify
    ASSERT_EQ(3, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *element1);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *element2);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *element3);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRuleset_DetectsChildNodeChangesWhenInstanceFilterIsModified, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, ByRuleset_DetectsChildNodeChangesWhenInstanceFilterIsModified)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2)); });

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRootRule->AddSpecification(*CreateCustomNodeSpec("T_NODE"));
    lhs->AddPresentationRule(*lhsRootRule);
    ChildNodeRule* lhsChildRule = new ChildNodeRule("ParentNode.Type = \"T_NODE\"", 1, false);
    lhsChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = 1", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsChildRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRootRule->AddSpecification(*CreateCustomNodeSpec("T_NODE"));
    rhs->AddPresentationRule(*rhsRootRule);
    ChildNodeRule* rhsChildRule = new ChildNodeRule("ParentNode.Type = \"T_NODE\"", 1, false);
    rhsChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = 2", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsChildRule);

    // get the nodes
    auto rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), lhs->GetRuleSetId(), RulesetVariables(), nullptr)));

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() })).wait();

    // verify
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *element1);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *element2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRuleset_DetectsClassGroupingNodeChangesWhenTheirChildNodesChange, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, ByRuleset_DetectsClassGroupingNodeChangesWhenTheirChildNodesChange)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2)); });

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false,
        "this.Prop = 1", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false,
        "this.Prop = 2", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsRule);

    // get the nodes
    auto rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), lhs->GetRuleSetId(), RulesetVariables(), nullptr)));

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() })).wait();

    // verify
    ASSERT_EQ(3, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_TRUE(m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    EXPECT_EQ(elementClass, &m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetKey()->AsECClassGroupingNodeKey()->GetECClass());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Delete, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *element1);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *element2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRuleset_DetectsLabelGroupingNodeChangesWhenTheirChildNodesChange, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="PropA" typeName="string" />
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, ByRuleset_DetectsLabelGroupingNodeChangesWhenTheirChildNodesChange)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("PropA", ECValue("A"));
        instance.SetValue("PropB", ECValue("B"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("PropA", ECValue("A"));
        instance.SetValue("PropB", ECValue("B"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("PropA", ECValue("C"));
        instance.SetValue("PropB", ECValue("C"));
        });

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true,
        "", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsRule);
    lhs->AddPresentationRule(*new InstanceLabelOverride(1, false, elementClass->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("PropA") }));

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true,
        "", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsRule);
    rhs->AddPresentationRule(*new InstanceLabelOverride(1, false, elementClass->GetFullName(), {new InstanceLabelOverridePropertyValueSpecification("PropB")}));

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>())).wait();

    // verify
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_TRUE(m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_STREQ("A", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_TRUE(m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_STREQ("B", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRuleset_DetectsPropertyGroupingNodeChangesWhenTheirChildNodesChange, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="PropA" typeName="string" />
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, ByRuleset_DetectsPropertyGroupingNodeChangesWhenTheirChildNodesChange)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("PropA", ECValue("A"));
        instance.SetValue("PropB", ECValue("B"));
        });

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true,
        "", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsRule);
    GroupingRule* lhsGroupingRule = new GroupingRule("", 1, false, elementClass->GetSchema().GetName(), elementClass->GetName(), "", "", "");
    lhsGroupingRule->AddGroup(*new PropertyGroup("", "", true, "PropA"));
    lhs->AddPresentationRule(*lhsGroupingRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true,
        "", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsRule);
    GroupingRule* rhsGroupingRule = new GroupingRule("", 1, false, elementClass->GetSchema().GetName(), elementClass->GetName(), "", "", "");
    rhsGroupingRule->AddGroup(*new PropertyGroup("", "", true, "PropB"));
    rhs->AddPresentationRule(*rhsGroupingRule);

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>())).wait();

    // verify
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_TRUE(m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode));
    EXPECT_STREQ("A", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_TRUE(m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode));
    EXPECT_STREQ("B", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRuleset_DetectsNoChangesOnCollapsedNodeWhenGrandchildrenAdded, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(HierarchiesCompareTests, ByRuleset_DetectsNoChangesOnCollapsedNodeWhenGrandchildrenAdded)
    {
    // set up the dataset
    ECClassCP classElement = GetClass("Element");
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement);

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_ROOT"));
    lhs->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*CreateCustomNodeSpec("T_CHILD_1"));
    lhs->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpec("T_CHILD_2"));
    lhs->AddPresentationRule(*childRule2);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);

    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));
    rhs->AddPresentationRule(*new ChildNodeRule(*childRule1));
    rhs->AddPresentationRule(*new ChildNodeRule(*childRule2));

    ChildNodeRule* grandChildRule1 = new ChildNodeRule("ParentNode.Type = \"T_CHILD_1\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classElement->GetFullName(), false));
    rhs->AddPresentationRule(*grandChildRule1);

    ChildNodeRule* grandChildRule2 = new ChildNodeRule("ParentNode.Type = \"T_CHILD_2\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classElement->GetFullName(), false));
    rhs->AddPresentationRule(*grandChildRule2);

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(0, m_changeRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRuleset_DetectsChildNodeUpdatesOnExpandedNodeWhenGrandchildrenAdded, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(HierarchiesCompareTests, ByRuleset_DetectsChildNodeUpdatesOnExpandedNodeWhenGrandchildrenAdded)
    {
    // set up the dataset
    ECClassCP classElement = GetClass("Element");
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement);

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_ROOT"));
    lhs->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*CreateCustomNodeSpec("T_CHILD_1"));
    lhs->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpec("T_CHILD_2"));
    lhs->AddPresentationRule(*childRule2);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);

    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));
    rhs->AddPresentationRule(*new ChildNodeRule(*childRule1));
    rhs->AddPresentationRule(*new ChildNodeRule(*childRule2));

    ChildNodeRule* grandChildRule1 = new ChildNodeRule("ParentNode.Type = \"T_CHILD_1\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classElement->GetFullName(), false));
    rhs->AddPresentationRule(*grandChildRule1);

    ChildNodeRule* grandChildRule2 = new ChildNodeRule("ParentNode.Type = \"T_CHILD_2\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classElement->GetFullName(), false));
    rhs->AddPresentationRule(*grandChildRule2);

    // get the nodes
    auto rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), lhs->GetRuleSetId(), RulesetVariables(), nullptr)));

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() })).wait();
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_1", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_2", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRuleset_DetectsChildNodeUpdatesAndGrandchildrenInsertsOnExpandedRootAndChildNodeWhenGrandchildrenAdded, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(HierarchiesCompareTests, ByRuleset_DetectsChildNodeUpdatesAndGrandchildrenInsertsOnExpandedRootAndChildNodeWhenGrandchildrenAdded)
    {
    // set up the dataset
    ECClassCP classElement = GetClass("Element");
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement);

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_ROOT"));
    lhs->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*CreateCustomNodeSpec("T_CHILD_1"));
    lhs->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpec("T_CHILD_2"));
    lhs->AddPresentationRule(*childRule2);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);

    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));
    rhs->AddPresentationRule(*new ChildNodeRule(*childRule1));
    rhs->AddPresentationRule(*new ChildNodeRule(*childRule2));

    ChildNodeRule* grandChildRule1 = new ChildNodeRule("ParentNode.Type = \"T_CHILD_1\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classElement->GetFullName(), false));
    rhs->AddPresentationRule(*grandChildRule1);

    ChildNodeRule* grandChildRule2 = new ChildNodeRule("ParentNode.Type = \"T_CHILD_2\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classElement->GetFullName(), false));
    rhs->AddPresentationRule(*grandChildRule2);

    // get the nodes
    auto rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), lhs->GetRuleSetId(), RulesetVariables(), nullptr)));
    auto childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), lhs->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));

    // compare
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey(), childNodes[0]->GetKey() })).wait();
    ASSERT_EQ(3, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_1", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_2", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchiesCompareTests, ByRulesetVariables_DetectsRootNodeChangesWhenVariableUsedInRuleCondition)
    {
    // create the ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);
    RootNodeRule* rule1 = new RootNodeRule("GetVariableBoolValue(\"use_first\")", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule1->AddSpecification(*CreateCustomNodeSpec("first"));
    ruleset->AddPresentationRule(*rule1);
    RootNodeRule* rule2 = new RootNodeRule("GetVariableBoolValue(\"use_second\")", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule2->AddSpecification(*CreateCustomNodeSpec("second"));
    ruleset->AddPresentationRule(*rule2);

    // compare no variables VS 'use_first'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("use_first", true) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_STREQ("first", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());

    // compare no variables VS 'use_first + use_second'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("use_first", true), RulesetVariableEntry("use_second", true) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    recordIndex = 0;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_STREQ("first", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(1, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_STREQ("second", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());

    // compare 'use_first=false' VS 'use_first=true + use_second=true'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("use_first", false) }),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("use_first", true), RulesetVariableEntry("use_second", true) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    recordIndex = 0;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_STREQ("first", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(1, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_STREQ("second", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());

    // compare 'use_first=true + use_second=false' VS 'use_first=false + use_second=true'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("use_first", true), RulesetVariableEntry("use_second", false) }),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("use_first", false), RulesetVariableEntry("use_second", true) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("first", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_STREQ("second", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRulesetVariables_DetectsRootNodeChangesWhenVariableUsedInInstanceFilter, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, ByRulesetVariables_DetectsRootNodeChangesWhenVariableUsedInInstanceFilter)
    {
    // set up the dataset
    ECClassCP classElement = GetClass("Element");
    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // create the ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = GetVariableIntValue(\"test\")", classElement->GetFullName(), false));
    ruleset->AddPresentationRule(*rule);

    // compare no variables VS 'test=0'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("test", (int64_t)3) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(0, m_changeRecordsHandler->GetRecords().size());

    // compare no variables VS 'test=2'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("test", (int64_t)2) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e2);

    // compare 'test=2' VS 'test=1'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("test", (int64_t)2) }),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("test", (int64_t)1) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e2);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e1);

    // compare 'test=1' VS 'test=0'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("test", (int64_t)1) }),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("test", (int64_t)0) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());
    recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e1);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRulesetVariables_DetectsRootNodeChangesWhenVariableUsedInCustomizationRuleCondition, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(HierarchiesCompareTests, ByRulesetVariables_DetectsRootNodeChangesWhenVariableUsedInCustomizationRuleCondition)
    {
    // set up the dataset
    ECClassCP classElement = GetClass("Element");
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement);

    // create the ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classElement->GetFullName(), false));
    ruleset->AddPresentationRule(*rule);
    ruleset->AddPresentationRule(*new StyleOverride("GetVariableBoolValue(\"customize\")", 1, "\"Red\"", "", ""));

    // compare no variables VS 'customize=false'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("customize", false) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(0, m_changeRecordsHandler->GetRecords().size());

    // compare no variables VS 'customize=true'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("customize", true) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e);

    // compare 'customize=true' VS 'customize=false'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("customize", true) }),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("customize", false) }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());
    recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRulesetVariables_DetectsRootNodeChangesWhenVariableUsedInCustomizationRuleValue, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(HierarchiesCompareTests, ByRulesetVariables_DetectsRootNodeChangesWhenVariableUsedInCustomizationRuleValue)
    {
    // set up the dataset
    ECClassCP classElement = GetClass("Element");
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement);

    // create the ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classElement->GetFullName(), false));
    ruleset->AddPresentationRule(*rule);
    ruleset->AddPresentationRule(*new StyleOverride("", 1, "GetVariableStringValue(\"color\")", "", ""));

    // compare no variables VS 'color=""'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("color", "") }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(0, m_changeRecordsHandler->GetRecords().size());

    // compare no variables VS 'color="Red"'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("color", "Red") }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e);

    // compare 'color="Blue"' VS 'color="Green"'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("color", "Blue") }),
        ruleset->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("color", "Green") }),
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());
    recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ByRulesetVariables_DetectsChangesOnlyForChildNodesOfExpandedParents_WhenVariablesUsedInChildrenInstanceFilter, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(HierarchiesCompareTests, ByRulesetVariables_DetectsChangesOnlyForChildNodesOfExpandedParents_WhenVariablesUsedInChildrenInstanceFilter)
    {
    // set up the dataset
    ECClassCP classElement = GetClass("Element");
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement);

    // create the ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_ROOT"));
    ruleset->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*CreateCustomNodeSpec("T_CHILD_1"));
    ruleset->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpec("T_CHILD_2"));
    ruleset->AddPresentationRule(*childRule2);

    ChildNodeRule* grandChildRule1 = new ChildNodeRule("ParentNode.Type = \"T_CHILD_1\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableBoolValue(\"show\")", classElement->GetFullName(), false));
    ruleset->AddPresentationRule(*grandChildRule1);

    ChildNodeRule* grandChildRule2 = new ChildNodeRule("ParentNode.Type = \"T_CHILD_2\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableBoolValue(\"show\")", classElement->GetFullName(), false));
    ruleset->AddPresentationRule(*grandChildRule2);

    // set up predefined variables
    RulesetVariables variablesShowFalse({RulesetVariableEntry("show", false)});
    RulesetVariables variablesShowTrue({RulesetVariableEntry("show", true)});

    // get the nodes
    auto rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), ruleset->GetRuleSetId(), RulesetVariables(), nullptr)));
    auto childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), ruleset->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));

    // compare no variables VS 'show=true' with no expanded nodes
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), variablesShowTrue,
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(0, m_changeRecordsHandler->GetRecords().size());

    // compare 'show=false' VS 'show=true' with expanded root node
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), variablesShowFalse,
        ruleset->GetRuleSetId(), variablesShowTrue,
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() })).wait();
    ASSERT_EQ(2, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_1", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_2", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    // compare 'show=false' VS 'show=true' with expanded root node and one of its children
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), variablesShowFalse,
        ruleset->GetRuleSetId(), variablesShowTrue,
        bvector<NavNodeKeyCPtr>{ childNodes[0]->GetKey(), rootNodes[0]->GetKey() })).wait();
    ASSERT_EQ(3, m_changeRecordsHandler->GetRecords().size());
    recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_1", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_changeRecordsHandler->GetRecords()[recordIndex].GetNode(), *e);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_2", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchiesCompareTests, ByRulesetVariables_DetectsChangesOnlyForChildNodesOfExpandedParents_WhenVariablesUsedInChildrenCondition)
    {
    // create the ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_ROOT_1"));
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_ROOT_2"));
    ruleset->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"T_ROOT_1\" ANDALSO GetVariableBoolValue(\"show\")", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*CreateCustomNodeSpec("T_CHILD_1"));
    ruleset->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"T_ROOT_2\" ANDALSO GetVariableBoolValue(\"show\")", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpec("T_CHILD_2"));
    ruleset->AddPresentationRule(*childRule2);

    // set up predefined variables
    RulesetVariables variablesShowFalse({ RulesetVariableEntry("show", false) });
    RulesetVariables variablesShowTrue({ RulesetVariableEntry("show", true) });

    // get the nodes
    auto rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), ruleset->GetRuleSetId(), RulesetVariables(), nullptr)));
    auto childNodes1 = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), ruleset->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    auto childNodes2 = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), ruleset->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())));

    // compare no variables VS 'show=false'
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), RulesetVariables(),
        ruleset->GetRuleSetId(), variablesShowFalse,
        bvector<NavNodeKeyCPtr>())).wait();
    ASSERT_EQ(0, m_changeRecordsHandler->GetRecords().size());

    // compare 'show=false' VS 'show=true' with one expanded root node
    m_changeRecordsHandler->Clear();
    m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        ruleset->GetRuleSetId(), variablesShowFalse,
        ruleset->GetRuleSetId(), variablesShowTrue,
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() })).wait();
    ASSERT_EQ(3, m_changeRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_ROOT_1", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_CHILD_1", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[recordIndex].GetPosition());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_STREQ("T_ROOT_2", m_changeRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().c_str());
    ++recordIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchiesCompareTests, GetsHierarchyUpdatesInMultipleRequests_AddedNodes)
    {
    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_ROOT"));
    lhs->AddPresentationRule(*rootRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);

    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*CreateCustomNodeSpec("T_CHILD_1"));
    rhs->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpec("T_CHILD_2"));
    rhs->AddPresentationRule(*childRule2);

    // get the nodes
    auto rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), lhs->GetRuleSetId(), RulesetVariables(), nullptr)));
    ASSERT_EQ(1, rootNodes.GetSize());

    // compare
    auto result = GetValidatedResponse(m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() }, nullptr, 1)));
    ASSERT_TRUE(nullptr != result);

    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());
    EXPECT_EQ(ChangeType::Update, m_changeRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("T_ROOT", m_changeRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());

    m_changeRecordsHandler->Clear();
    result = GetValidatedResponse(m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() }, result, 1)));
    ASSERT_TRUE(nullptr != result);

    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("T_CHILD_1", m_changeRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[0].GetPosition());

    m_changeRecordsHandler->Clear();
    result = GetValidatedResponse(m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() }, result, 1)));
    ASSERT_TRUE(nullptr == result);

    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("T_CHILD_2", m_changeRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_EQ(1, m_changeRecordsHandler->GetRecords()[0].GetPosition());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchiesCompareTests, GetsHierarchyUpdatesInMultipleRequests_AddedNodesInCorrectPositions)
    {
    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS));
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*CreateCustomNodeSpec("T_ROOT"));
    lhs->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*CreateCustomNodeSpec("T_CHILD_1"));
    lhs->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpec("T_CHILD_2"));
    lhs->AddPresentationRule(*childRule2);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS));
    m_locater->AddRuleSet(*rhs);

    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));

    ChildNodeRule* childRule3 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule3->AddSpecification(*CreateCustomNodeSpec("T_CHILD_3"));
    rhs->AddPresentationRule(*childRule3);

    rhs->AddPresentationRule(*new ChildNodeRule(*childRule1));

    ChildNodeRule* childRule4 = new ChildNodeRule("ParentNode.Type = \"T_ROOT\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule4->AddSpecification(*CreateCustomNodeSpec("T_CHILD_4"));
    rhs->AddPresentationRule(*childRule4);

    rhs->AddPresentationRule(*new ChildNodeRule(*childRule2));

    // get the nodes
    auto rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), lhs->GetRuleSetId(), RulesetVariables(), nullptr)));
    ASSERT_EQ(1, rootNodes.GetSize());

    // compare
    auto result = GetValidatedResponse(m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() }, nullptr, 1)));
    ASSERT_TRUE(nullptr != result);

    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("T_CHILD_3", m_changeRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_EQ(0, m_changeRecordsHandler->GetRecords()[0].GetPosition());

    m_changeRecordsHandler->Clear();
    result = GetValidatedResponse(m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() }, result, 1)));
    ASSERT_TRUE(nullptr != result);

    ASSERT_EQ(1, m_changeRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Insert, m_changeRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_STREQ("T_CHILD_4", m_changeRecordsHandler->GetRecords()[0].GetNode()->GetType().c_str());
    EXPECT_EQ(2, m_changeRecordsHandler->GetRecords()[0].GetPosition());

    m_changeRecordsHandler->Clear();
    result = GetValidatedResponse(m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(s_project->GetECDb(), m_changeRecordsHandler,
        lhs->GetRuleSetId(), RulesetVariables(),
        rhs->GetRuleSetId(), RulesetVariables(),
        bvector<NavNodeKeyCPtr>{ rootNodes[0]->GetKey() }, result, 1)));
    ASSERT_TRUE(nullptr == result);
    EXPECT_TRUE(m_changeRecordsHandler->GetRecords().empty());
    }
