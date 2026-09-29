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
struct HierarchyUpdateTests : UpdateTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsRootNodeAdded, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(HierarchyUpdateTests, DetectsRootNodeAdded)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    // insert instance
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsRootNodeRemoved, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(HierarchyUpdateTests, DetectsRootNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *a2, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *a2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupByClass_DetectsGroupedNodeRemoved, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(HierarchyUpdateTests, GroupByClass_DetectsGroupedNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1, a2 }),
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *a2, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *a2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1 }),
            {
            CreateInstanceNodeValidator({ a1 }),
            })
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupByClass_DetectsLastGroupedNodeRemoved, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(HierarchyUpdateTests, GroupByClass_DetectsLastGroupedNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1 }),
            {
            CreateInstanceNodeValidator({ a1 }),
            })
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *a1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *a1);

    // validate hierarchy post-update
    ValidateHierarchy(params, {});

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupByClass_DetectsFirstGroupedNodeInserted, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(HierarchyUpdateTests, GroupByClass_DetectsFirstGroupedNodeInserted)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params, {});

    // insert instance
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a1);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1 }),
            {
            CreateInstanceNodeValidator({ a1 }),
            })
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupByClass_DetectsGroupedNodeInserted, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(HierarchyUpdateTests, GroupByClass_DetectsGroupedNodeInserted)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1 }),
            {
            CreateInstanceNodeValidator({ a1 }),
            })
        });

    // insert instance
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1, a2 }),
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupByLabel_DetectsGroupedNodeRemoved, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LabelProp" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, GroupByLabel_DetectsGroupedNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); });
    IECInstancePtr a12 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); });
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); }, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a11, a12 }),
            {
            CreateInstanceNodeValidator({ a11 }),
            CreateInstanceNodeValidator({ a12 }),
            }),
        CreateInstanceNodeValidator({ a21 }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *a12, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *a12);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a11 }),
        CreateInstanceNodeValidator({ a21 }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupByLabel_DetectsSiblingNodeRemoved, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LabelProp" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, GroupByLabel_DetectsSiblingNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); });
    IECInstancePtr a12 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); });
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); }, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a11, a12 }),
            {
            CreateInstanceNodeValidator({ a11 }),
            CreateInstanceNodeValidator({ a12 }),
            }),
        CreateInstanceNodeValidator({ a21 }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *a21, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *a21);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a11 }),
        CreateInstanceNodeValidator({ a12 }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupByLabel_DetectsGroupedNodeInserted, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LabelProp" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, GroupByLabel_DetectsGroupedNodeInserted)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); });
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); }, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a11 }),
        CreateInstanceNodeValidator({ a21 }),
        });

    // insert instance
    IECInstancePtr a12 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); }, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a12);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a11, a12 }),
            {
            CreateInstanceNodeValidator({ a11 }),
            CreateInstanceNodeValidator({ a12 }),
            }),
        CreateInstanceNodeValidator({ a21 }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupByLabel_DetectsGroupedNodeLabelChanged, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LabelProp" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, GroupByLabel_DetectsGroupedNodeLabelChanged)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); });
    IECInstancePtr a12 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); });
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); }, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a11, a12 }),
            {
            CreateInstanceNodeValidator({ a11 }),
            CreateInstanceNodeValidator({ a12 }),
            }),
        CreateInstanceNodeValidator({ a21 }),
        });

    // update instance
    a12->SetValue("LabelProp", ECValue("3"));
    ECInstanceUpdater updater(m_db, *a12, nullptr);
    updater.Update(*a12);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *a12);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a11 }),
        CreateInstanceNodeValidator({ a21 }),
        CreateInstanceNodeValidator({ a12 }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_DetectsRelatedNodeInserted, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_DetectsRelatedNodeInserted)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // insert instance
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *b2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_DetectsRelatedNodeRemoved, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_DetectsRelatedNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *b2, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *b2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_DetectsRelatedNodeParentChange, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_DetectsRelatedNodeParentChange)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    ECInstanceKey relKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b }),
            }),
        CreateInstanceNodeValidator({ a2 }),
        });

    // change b node parent instance
    RulesEngineTestHelpers::DeleteInstance(m_db, relKey);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *b);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ b }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_HideIfNoChildren_DetectsChildNodeRemoved, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_HideIfNoChildren_DetectsChildNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *b1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *b1);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_HideIfNoChildren_DetectsChildNodeInserted, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_HideIfNoChildren_DetectsChildNodeInserted)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // insert instance
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *b2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_GroupByClass_DetectsRelatedNodeInserted, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_GroupByClass_DetectsRelatedNodeInserted)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b1 }),
                {
                CreateInstanceNodeValidator({ b1 }),
                })
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b2 }),
                {
                CreateInstanceNodeValidator({ b2 }),
                })
            }),
        });

    // insert instance
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b3, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *b3);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b1, b3 }),
                {
                CreateInstanceNodeValidator({ b1 }),
                CreateInstanceNodeValidator({ b3 }),
                })
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b2 }),
                {
                CreateInstanceNodeValidator({ b2 }),
                })
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_GroupByClass_DetectsRelatedNodeRemoved, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_GroupByClass_DetectsRelatedNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b3, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b1 }),
                {
                CreateInstanceNodeValidator({ b1 }),
                })
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b2, b3 }),
                {
                CreateInstanceNodeValidator({ b2 }),
                CreateInstanceNodeValidator({ b3 }),
                })
            }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *b3, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *b3);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b1 }),
                {
                CreateInstanceNodeValidator({ b1 }),
                })
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b2 }),
                {
                CreateInstanceNodeValidator({ b2 }),
                })
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_HideNodesInHierarchy_DetectsRelatedNodeInserted, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_HideNodesInHierarchy_DetectsRelatedNodeInserted)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b, *c1, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRuleAB = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRuleAB->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleAB);

    ChildNodeRule* childRuleBC = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRuleBC->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleBC);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        });

    // insert instance
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b, *c2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *c2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            CreateInstanceNodeValidator({ c2 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_HideNodesInHierarchy_DetectsRelatedNodeRemoved, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_HideNodesInHierarchy_DetectsRelatedNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b, *c1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b, *c2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRuleAB = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRuleAB->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleAB);

    ChildNodeRule* childRuleBC = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRuleBC->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleBC);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            CreateInstanceNodeValidator({ c2 }),
            }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *c2, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *c2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_HideNodesInHierarchy_DetectsNodeRemovedInHiddenLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_HideNodesInHierarchy_DetectsNodeRemovedInHiddenLevel)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b2, *c2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRuleAB = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRuleAB->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleAB);

    ChildNodeRule* childRuleBC = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRuleBC->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleBC);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c2 }),
            }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *b1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *b1);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c2 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_HideNodesInHierarchy_DetectsNodeInsertedInHiddenLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_HideNodesInHierarchy_DetectsNodeInsertedInHiddenLevel)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b1, *c1, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRuleAB = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRuleAB->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleAB);

    ChildNodeRule* childRuleBC = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRuleBC->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleBC);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        CreateInstanceNodeValidator({ a2 }),
        });

    // insert instance
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b2, *c2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *b2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c2 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_HideNodesInHierarchy_UnderHideIfNoChildren_DetectsRelatedNodeInserted, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_HideNodesInHierarchy_UnderHideIfNoChildren_DetectsRelatedNodeInserted)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b1, *c1, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRuleAB = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRuleAB->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleAB);

    ChildNodeRule* childRuleBC = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRuleBC->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleBC);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
    {
    ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
        {
        CreateInstanceNodeValidator({ c1 }),
        }),
    });

    // insert instance
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b2, *c2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *c2);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c2 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstances_HideNodesInHierarchy_UnderHideIfNoChildren_DetectsRelatedNodeRemoved, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, RelatedInstances_HideNodesInHierarchy_UnderHideIfNoChildren_DetectsRelatedNodeRemoved)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(m_db, *classC);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relBC, *b2, *c2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRuleAB = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRuleAB->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleAB);

    ChildNodeRule* childRuleBC = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRuleBC->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleBC);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c2 }),
            }),
        });

    // remove instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *c1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *c1);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ c2 }),
            }),
        });

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesParentWhenNotFinalizedChildDatasourceBecomesEmpty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(HierarchyUpdateTests, UpdatesParentWhenNotFinalizedChildDatasourceBecomesEmpty)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(m_db, *classC, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "", classB->GetFullName(), false));
    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.ClassName = \"B\"", 1, false);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*childRule1);
    rules->AddPresentationRule(*childRule2);

    // request root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // expect to have 1 children
    EXPECT_EQ(1, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()))));

    // remove the C instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *c, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *c);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // make sure we have 0 child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }


/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdateParentNodeWhenChildInstanceIsInserted_WithRulesetVariables, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(HierarchyUpdateTests, UpdateParentNodeWhenChildInstanceIsInserted_WithRulesetVariables)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "GetVariableBoolValue(\"show_nodes\")", classA->GetFullName(), false));
    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "GetVariableBoolValue(\"show_nodes\")", classB->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*childRule);

    // request root nodes
    m_manager->GetUserSettings().GetSettings(rules->GetRuleSetId()).SetSettingBoolValue("show_nodes", true);
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // expect to have 0 children
    EXPECT_EQ(0, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()))));

    // add the B instance
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *b);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // make sure we have 1 child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesRootNodeWhenChildrenCountIsAffectedByChangeInGrandChildrenLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(HierarchyUpdateTests, UpdatesRootNodeWhenChildrenCountIsAffectedByChangeInGrandChildrenLevel)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(m_db, *classC, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "", classB->GetFullName(), false));
    ChildNodeRule* grandChildRule = new ChildNodeRule("ParentNode.ClassName = \"B\"", 1, false);
    grandChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*childRule);
    rules->AddPresentationRule(*grandChildRule);

    // request root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); });
    ASSERT_EQ(1, grandChildNodes.GetSize());

    // remove C instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *c, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *c);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // make sure we have 0 child node
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // verify records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdateFilteredRootHierarchyLevel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, UpdateFilteredRootHierarchyLevel)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2));});
    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>(filter));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    // add an instance that doesn't match the filter
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(3)); }, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a3);

    // verify hierarchy didn't change
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    m_updateRecordsHandler->Clear();

    // add another instance matching the filter
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a4);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a4 }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdateFilteredChildHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdateFilteredChildHierarchyLevel)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);

    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";
    auto aChildrenFilterSetter = [&](AsyncHierarchyRequestParams& p)
        {
        NavNodeKeyCP parentKey = p.GetParentNode() ? p.GetParentNode()->GetKey().get() : p.GetParentNodeKey() ? p.GetParentNodeKey() : nullptr;
        if (parentKey && ContainerHelpers::Contains(parentKey->AsECInstanceNodeKey()->GetInstanceKeys(), [&](auto const& k){return k.GetClass()->GetName().Equals("A");}))
            p.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>(filter));
        };

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // add an instance that doesn't match the filter
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(3));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b3, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a.get(), b3.get() });

    // verify the hierarchy didn't change
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    m_updateRecordsHandler->Clear();

    // add another instance matching the filter
    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b4, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a.get(), b4.get() });

    // validate hierarchy post-update
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b4 }),
            }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesFilteredParentNodeWhenItHasHideIfNoChildrenFlagAndChildrenAreRemoved, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdatesFilteredParentNodeWhenItHasHideIfNoChildrenFlagAndChildrenAreRemoved)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);

    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";
    auto aChildrenFilterSetter = [&](AsyncHierarchyRequestParams& p)
        {
        NavNodeKeyCP parentKey = p.GetParentNode() ? p.GetParentNode()->GetKey().get() : p.GetParentNodeKey() ? p.GetParentNodeKey() : nullptr;
        if (parentKey && ContainerHelpers::Contains(parentKey->AsECInstanceNodeKey()->GetInstanceKeys(), [&](auto const& k){return k.GetClass()->GetName().Equals("A");}))
            p.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>(filter));
        };

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // remove the instance matching the filter
    RulesEngineTestHelpers::DeleteInstance(m_db, *b1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *b1);

    // validate hierarchy post-update
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        // notes:
        // - even though 'a' has 'hide if no children' flag and has no children, we still show it
        //   or otherwise there would be no way to clear the filter and get it back
        // - even though the node has no children, it still has the 'has children' flag set to
        //   'true' - we don't know the filter for 'a' node when creating it
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }), true,
            {
            }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesFilteredParentNodeWhenItHasHideIfNoChildrenFlagAndChildrenAreInserted, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdatesFilteredParentNodeWhenItHasHideIfNoChildrenFlagAndChildrenAreInserted)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1)); });
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 2";
    auto aChildrenFilterSetter = [&](AsyncHierarchyRequestParams& p)
        {
        NavNodeKeyCP parentKey = p.GetParentNode() ? p.GetParentNode()->GetKey().get() : p.GetParentNodeKey() ? p.GetParentNodeKey() : nullptr;
        if (parentKey && ContainerHelpers::Contains(parentKey->AsECInstanceNodeKey()->GetInstanceKeys(), [&](auto const& k){return k.GetClass()->GetName().Equals("A"); }))
            p.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>(filter));
        };

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }), true,
            {
            }),
        });

    // insert an instance matching the filter
    auto b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a.get(), b2.get() });

    // validate hierarchy post-update
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesFilteredHierarchyLevelWhenNodesUnderVirtualParentAreUpdated, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdatesFilteredHierarchyLevelWhenNodesUnderVirtualParentAreUpdated)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);

    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    auto groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classB->GetName(), "");
    groupingRule->AddGroup(*new PropertyGroup("", false, "Prop"));
    rules->AddPresentationRule(*groupingRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";
    auto aChildrenFilterSetter = [&](AsyncHierarchyRequestParams& p)
        {
        NavNodeKeyCP parentKey = p.GetParentNode() ? p.GetParentNode()->GetKey().get() : p.GetParentNodeKey() ? p.GetParentNodeKey() : nullptr;
        if (!parentKey)
            return;

        bool isParentAInstanceNode = parentKey->AsECInstanceNodeKey()
            && ContainerHelpers::Contains(parentKey->AsECInstanceNodeKey()->GetInstanceKeys(), [&](auto const& k){return k.GetClass()->GetName().Equals("A");});
        bool isParentAPropertyGroupingNode = parentKey->AsECPropertyGroupingNodeKey()
            && &parentKey->AsECPropertyGroupingNodeKey()->GetECClass() == classA;
        if (isParentAInstanceNode || isParentAPropertyGroupingNode)
            {
            p.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>(filter));
            }
        };

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // insert an instance matching the filter
    auto b3 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b3, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a.get(), b3.get() });

    // validate hierarchy post-update
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ b1.get(), b3.get() }, { ECValue(1) }),
                {
                CreateInstanceNodeValidator({ b1 }),
                CreateInstanceNodeValidator({ b3 }),
                }),
            }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesHierarchyLevelWhenNodesUnderFilteredParentAreUpdated, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdatesHierarchyLevelWhenNodesUnderFilteredParentAreUpdated)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a0 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(0));});

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>(filter));
    auto hierarchy = ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // insert another B instance
    auto b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b2, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a1.get(), b2.get() });

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdateHierarchyLevelFilteredWithMultipleFilters, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, UpdateHierarchyLevelFilteredWithMultipleFilters)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2)); });
    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up hierarchy level instance filter
    auto filter1 = std::make_shared<InstanceFilterDefinition>("this.Prop = 1");
    auto filter2 = std::make_shared<InstanceFilterDefinition>("this.Prop = 2");

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());

    params.SetInstanceFilter(filter1);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    params.SetInstanceFilter(filter2);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });

    params.SetInstanceFilter(nullptr);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        });

    // add an instance that doesn't match any of the filters
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(3)); }, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a3);

    // verify hierarchy changes
    params.SetInstanceFilter(filter1);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    params.SetInstanceFilter(filter2);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });

    params.SetInstanceFilter(nullptr);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        CreateInstanceNodeValidator({ a3 }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());

    m_updateRecordsHandler->Clear();

    // add another instance matching one of the filters
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); }, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a4);

    // verify hierarchy changes
    params.SetInstanceFilter(filter1);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a4 }),
        });

    params.SetInstanceFilter(filter2);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });

    params.SetInstanceFilter(nullptr);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        CreateInstanceNodeValidator({ a3 }),
        CreateInstanceNodeValidator({ a4 }),
        });

    // expect update records
    EXPECT_FALSE(m_updateRecordsHandler->GetFullUpdateRecords().empty());
    }
