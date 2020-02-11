/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../PresentationManagerIntegrationTests.h"
#include "../../../../Source/RulesDriven/RulesEngine/LocalizationHelper.h"
#include "../../../../Localization/Xliffs/ECPresentation.xliff.h"
#include "../../RulesEngine/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                04/2015
//=======================================================================================
struct RulesDrivenECPresentationManagerContentTests : PresentationManagerIntegrationTests
{
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;
    ECSchemaCP m_schema;

    void SetUp() override
        {
        PresentationManagerIntegrationTests::SetUp();

        m_schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
        ASSERT_TRUE(nullptr != m_schema);

        m_widgetClass = m_schema->GetClassCP("Widget");
        m_gadgetClass = m_schema->GetClassCP("Gadget");
        m_sprocketClass = m_schema->GetClassCP("Sprocket");
        }

    void CloseConnection(IConnectionCR connection)
        {
        static_cast<TestConnectionManager*>(m_connections)->NotifyConnectionClosed(connection);
        }
};

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ReturnsValidDescriptorBasedOnSelectedClasses)
    {
    // set up input
    KeySetPtr input = KeySet::Create(bvector<ECClassInstanceKey>{ECClassInstanceKey(m_gadgetClass, ECInstanceId()), ECClassInstanceKey(m_widgetClass, ECInstanceId())});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(2, descriptor->GetSelectClasses().size());

    EXPECT_EQ(m_gadgetClass, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());
    EXPECT_FALSE(descriptor->GetSelectClasses()[0].GetSelectClass().IsSelectPolymorphic());

    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[1].GetSelectClass().GetClass());
    EXPECT_FALSE(descriptor->GetSelectClasses()[1].GetSelectClass().IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_ReturnsValidDescriptorBasedOnSelectedClassesWhenJoiningPolymorphicallyRelatedProperties, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect">
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ReturnsValidDescriptorBasedOnSelectedClassesWhenJoiningPolymorphicallyRelatedProperties)
    {
    ECEntityClassCP elementClass = GetClass("Element")->GetEntityClassCP();
    ECEntityClassCP aspectClass = GetClass("ElementUniqueAspect")->GetEntityClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();

    // set up dataset
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element, *aspect);

    // set up selection
    KeySetPtr input = KeySet::Create({elementClass});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rule->GetSpecifications().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        elementOwnsUniqueAspectRelationship->GetFullName(), aspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(elementClass, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields.front()->IsNestedContentField());
    EXPECT_STREQ(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass), fields.front()->GetName().c_str());
    ContentDescriptor::NestedContentField const* field = fields.front()->AsNestedContentField();
    ASSERT_EQ(1, field->GetFields().size());
    EXPECT_STREQ(FIELD_NAME(aspectClass, "AspectName"), field->GetFields().front()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AllPropertiesOfOneSelectedNode)
    {
    // insert some widget instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(*instance1);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({instance1.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AcceptableClassNames_ReturnsInstanceOfDefinedClassName)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{widgetInstance, gadgetInstance});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "Widget", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({widgetInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AcceptableSchemaName_WrongSchemaName_ContentIsNotValid)
    {
    // insert widget instance
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(*widgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "WrongSchemaName", "", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_FALSE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AcceptablePolymorphically)
    {
    // insert some classE & classF instances
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    IECInstancePtr classEInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr classFInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{classEInstance, classFInstance});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "ClassE", true));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({classEInstance.get(), classFInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfTheSameClass)
    {
    // insert some widget instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({instance1.get(), instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfDifferentClasses)
    {
    // insert some widget instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({instance1.get(), instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_WithSortingFieldAndOrder)
    {
    // insert some widgets and gadgets
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{widget, gadget});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet({widget.get(), gadget.get()}, *content, false);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetSortingField(FIELD_NAME((bvector<ECClassCP>{m_gadgetClass, m_widgetClass}), "MyID"));

    // get the content with descriptor override
    content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted
    RulesEngineTestHelpers::ValidateContentSet({gadget.get(), widget.get()}, *content, true);

    // change the order from ascending to descending
    ovr->SetSortDirection(SortDirection::Descending);

    // get the content with the changed sorting order
    content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted in descending order
    RulesEngineTestHelpers::ValidateContentSet({widget.get(), gadget.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_SortByDisplayLabel, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_SortByDisplayLabel)
    {
    // insert some instances
    ECClassCP ecClass = GetClass("MyClass");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProperty", ECValue("a"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProperty", ECValue("b"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(0, false, ecClass->GetFullName(), "MyProperty"));

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::List, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    ASSERT_EQ(2, content->GetContentSet().GetSize());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetSortingField(descriptor->GetDisplayLabelField()->GetName().c_str());
    ovr->SetSortDirection(SortDirection::Ascending);

    // get the content with descriptor override
    content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    EXPECT_STREQ("a", content->GetContentSet().Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("b", content->GetContentSet().Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    // change the order from ascending to descending
    ovr->SetSortDirection(SortDirection::Descending);

    // get the content with the changed sorting order
    content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted in descending order
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    EXPECT_STREQ("b", content->GetContentSet().Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("a", content->GetContentSet().Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_SortingByEnumProperty)
    {
    // insert some instances
    ECEntityClassCP classQ = GetClass("RulesEngineTest", "ClassQ")->GetEntityClassCP();
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance){instance.SetValue("IntEnum", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance){instance.SetValue("IntEnum", ECValue(3));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassQ", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetSortingField(FIELD_NAME(classQ, "IntEnum"));

    // get the content with descriptor override
    ContentCPtr content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted
    // note: the sorting should be done by enum display values instead of enum value ids
    RulesEngineTestHelpers::ValidateContentSet({instance2.get(), instance1.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_RemovesPropertyField)
    {
    // insert a widget instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(*instance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(7, descriptor->GetVisibleFields().size());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->RemoveField(FIELD_NAME(m_widgetClass, "IntProperty"));

    // get the content with descriptor override
    ContentCPtr content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the IntProperty field has been removed
    EXPECT_EQ(6, content->GetDescriptor().GetVisibleFields().size());
    EXPECT_TRUE(content->GetDescriptor().GetAllFields().end() == std::find_if(content->GetDescriptor().GetAllFields().begin(), content->GetDescriptor().GetAllFields().end(),
        [&](ContentDescriptor::Field const* field){return field->GetName().Equals(FIELD_NAME(m_widgetClass, "IntProperty"));}));
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_RemovesNavigationField)
    {
    // insert a sprocket instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", m_sprocketClass->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->RemoveField(FIELD_NAME(m_sprocketClass, "Gadget"));

    // get the content with descriptor override
    ContentCPtr content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the Gadget field has been removed
    EXPECT_EQ(2, content->GetDescriptor().GetVisibleFields().size());

    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ContentSetItemCPtr record = content->GetContentSet().Get(0);
    rapidjson::Document json = record->AsJson();
    EXPECT_FALSE(json["Values"].HasMember(FIELD_NAME(m_sprocketClass, "Gadget")));
    EXPECT_FALSE(json["DisplayValues"].HasMember(FIELD_NAME(m_sprocketClass, "Gadget")));
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_WithFilters)
    {
    // insert some widget instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("DoubleProperty", ECValue(1.0));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2, instance3});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet({instance1.get(), instance2.get(), instance3.get()}, *content);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFilterExpression(Utf8PrintfString("%s > 1 or %s < 0", FIELD_NAME(m_widgetClass, "IntProperty"), FIELD_NAME(m_widgetClass, "DoubleProperty")).c_str());

    // get the content with descriptor override
    content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    RulesEngineTestHelpers::ValidateContentSet({instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Haroldas.Vitunskas              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_WithEscapedFilters)
    {
    // insert some widget instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("abc"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("%"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("abc%def"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2, instance3});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet({instance1.get(), instance2.get(), instance3.get()}, *content);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFilterExpression(Utf8String(FIELD_NAME(m_widgetClass, "MyID")).append(" LIKE \"%\\%%\"").c_str());

    // get the content with descriptor override
    content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    RulesEngineTestHelpers::ValidateContentSet({instance2.get(), instance3.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_FiltersByDisplayLabel, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_FiltersByDisplayLabel)
    {
    // insert some instances
    ECClassCP ecClass = GetClass("MyClass");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProperty", ECValue("a"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProperty", ECValue("b"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(0, false, ecClass->GetFullName(), "MyProperty"));

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::List, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    ASSERT_EQ(2, content->GetContentSet().GetSize());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFilterExpression(Utf8PrintfString("%s = \"b\"", descriptor->GetDisplayLabelField()->GetName().c_str()));

    // get the content with descriptor override
    content = m_manager->GetContent(*ovr, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    EXPECT_STREQ("b", content->GetContentSet().Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_ReturnsValidDescriptorWhichDoesNotDependOnSelectedClasses)
    {
    // set up input
    KeySetPtr input = KeySet::Create({ECClassInstanceKey(m_gadgetClass, ECInstanceId())});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[0].GetSelectClass().IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_ClassNames_ReturnsInstanceOfDefinedClass)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({widgetInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_InstanceFilter)
    {
    // insert some widget instances
    IECInstancePtr widgetInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr widgetInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "this.IntProperty=2", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({widgetInstance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_ArePolymorphic)
    {
    // insert some classE & classF instances
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    IECInstancePtr classEInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr classFInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({classEInstance.get(), classFInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance)
    {
    // insert some widget & gadget instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCR relationshipWidgetHasGadgets = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadgetInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("CustomID")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadgets, *widgetInstance, *gadgetInstance2);

    // set up input
    KeySetPtr input = KeySet::Create(*widgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({gadgetInstance1.get(), gadgetInstance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance_BackwardsDirection)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // set up input
    KeySetPtr input = KeySet::Create(*gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, "", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({widgetInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsRelatedInstancesPolymorphically, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="PropC" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="PropD" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsRelatedInstancesPolymorphically)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP rel = GetClass("A_Has_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) { instance.SetValue("PropC", ECValue("C")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);

    // set up input
    KeySetPtr input = KeySet::Create(*a);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(
        rel->GetFullName(), RequiredRelationDirection_Forward
    )})}));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ("PropC", descriptor->GetVisibleFields().front()->AsPropertiesField()->GetProperties().front().GetProperty().GetName().c_str());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    RulesEngineTestHelpers::ValidateContentSet({c.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelationshipClassNames_ReturnsInvalidContentWhenRelationshipDoesNotExist)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // set up input
    KeySetPtr input = KeySet::Create(*widgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "RulesEngineTest:DoesNotExist", ""));
    rules->AddPresentationRule(*rule);

    // note: query builder asserts when the class / relationships is not found - ignore that
    IGNORE_BE_ASSERT();

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_FALSE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelationshipClassNames)
    {
    // insert some widget & gadget instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCR relationshipWidgetHasGadgets = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadgetInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadgets, *widgetInstance, *gadgetInstance2);

    // set up input
    KeySetPtr input = KeySet::Create(*widgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "RulesEngineTest:WidgetHasGadget", ""));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({gadgetInstance1.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelationshipClassNames_BackwardsDirection)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // set up input
    KeySetPtr input = KeySet::Create(*gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadget", ""));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({widgetInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsSingleStepRelatedInstances, R"*(
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsSingleStepRelatedInstances)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(
        modelContainsElementsRelationship->GetFullName(), RequiredRelationDirection_Forward
    ))}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*model);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({element.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiStepRelatedInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiStepRelatedInstances)
    {
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP elementOwnsChildElementsRelationship = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();

    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e1, *e2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e2, *e3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e3, *e4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(
        elementOwnsChildElementsRelationship->GetFullName(), RequiredRelationDirection_Backward, elementClass->GetFullName(), 3
    ))}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*e4);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({e1.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipSingleStepRelatedInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipSingleStepRelatedInstances)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Backward),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Backward),
        })}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*c);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // request
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({a.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipMultiStepRelatedInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipMultiStepRelatedInstances)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBB = GetClass("B_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b1, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b2, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Backward),
        new RepeatableRelationshipStepSpecification(relationshipBB->GetFullName(), RequiredRelationDirection_Backward, "", 2),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Backward),
        })}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*c);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // request
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({a.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipInFront, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipInFront)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAA = GetClass("A_To_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a2, *a3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a3, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*a1);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // request
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({c1.get(), c2.get(), c3.get(), c4.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipInTheMiddle, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipInTheMiddle)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBB = GetClass("B_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b1, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b2, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBB->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*a);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // request
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({c1.get(), c2.get(), c3.get(), c4.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipAtTheEnd, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipAtTheEnd)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipCC = GetClass("C_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipCC->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        })}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*a);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // request
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({c1.get(), c2.get(), c3.get(), c4.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipsInMultiplePlaces, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipsInMultiplePlaces)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAA = GetClass("A_To_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipCC = GetClass("C_To_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a2, *a3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a3, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a3, *a4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a4, *b4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b4, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipCC->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        })}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*a1);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // request
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({c1.get(), c2.get(), c3.get(), c4.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsEmptyResultsWithRecursiveRelationship, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsEmptyResultsWithRecursiveRelationship)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipCC = GetClass("C_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipCC->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        })}));
    rules->AddPresentationRule(*rule);

    // options
    KeySetPtr input = KeySet::Create(*a);
    RulesDrivenECPresentationManager::ContentOptions options(BeTest::GetNameOfCurrentTest());

    // request
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_InstanceFilter)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadgets = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Gadget1"));});
    IECInstancePtr gadgetInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Gadget2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadgets, *widgetInstance, *gadgetInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadgets, *widgetInstance, *gadgetInstance2);

    // set up input
    KeySetPtr input = KeySet::Create(*widgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "this.Description=\"Gadget2\"", RequiredRelationDirection_Both, "", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({gadgetInstance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_Recursive)
    {
    ECEntityClassCP classN = GetClass("RulesEngineTest", "ClassN")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClass = GetClass("RulesEngineTest", "ClassNGroupsClassN")->GetRelationshipClassCP();

    // insert some  instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance4, *instance5);

    /*
    Relationship hierarchy:
            1
            |
            2
          /   \
         3     4
               |
               5
    */

    // set up input
    KeySetPtr input = KeySet::Create(*instance2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Both, "", "RulesEngineTest:ClassN"));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({instance1.get(),instance3.get(),instance4.get(),instance5.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RecursiveWithMultipleRelationships)
    {
    ECEntityClassCP classN = GetClass("RulesEngineTest", "ClassN")->GetEntityClassCP();
    ECEntityClassCP classO = GetClass("RulesEngineTest", "ClassO")->GetEntityClassCP();
    ECEntityClassCP classP = GetClass("RulesEngineTest", "ClassP")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClassNN = GetClass("RulesEngineTest", "ClassNGroupsClassN")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClassNO = GetClass("RulesEngineTest", "ClassNOwnsClassO")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClassOP = GetClass("RulesEngineTest", "ClassOHasClassP")->GetRelationshipClassCP();

    // insert some  instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classN, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    IECInstancePtr instance6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classO, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(6));});
    IECInstancePtr instance7 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classO, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(7));});
    IECInstancePtr instance8 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classP, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassNN, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassNN, *instance2, *instance3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassNN, *instance2, *instance4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassNN, *instance4, *instance5);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassNO, *instance5, *instance6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassNO, *instance5, *instance7);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassOP, *instance7, *instance8);

    /*
    Relationship hierarchy:
            1
            |       <-- ClassNGroupsClassN
            2
          /   \     <-- ClassNGroupsClassN
         3     4
               |    <-- ClassNGroupsClassN
               5
              / \   <-- ClassNOwnsClassO
             6   7
                 |  <-- ClassOHasClassP
                 8
    */

    // set up input
    KeySetPtr input = KeySet::Create(*instance1);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Forward,
        "RulesEngineTest:ClassNGroupsClassN,ClassNOwnsClassO,ClassOHasClassP", "RulesEngineTest:ClassN,ClassO,ClassP"));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({instance2.get(),instance3.get(),instance4.get(),instance5.get(),instance6.get(),instance7.get(),instance8.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_RecursiveWithMultipleSelectClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RecursiveWithMultipleSelectClasses)
    {
    ECEntityClassCP elementClass = GetClass("Element")->GetEntityClassCP();
    ECEntityClassCP elementAClass = GetClass("ElementA")->GetEntityClassCP();
    ECEntityClassCP elementBClass = GetClass("ElementB")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClass = GetRelationshipClass("ElementOwnsChildElements")->GetRelationshipClassCP();

    // insert some  instances
    IECInstancePtr parentA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass, [](IECInstanceR instance){instance.SetValue("ElementProperty", ECValue(1));});
    IECInstancePtr childA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementProperty", ECValue(2));});
    IECInstancePtr parentB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass, [](IECInstanceR instance){instance.SetValue("ElementProperty", ECValue(3));});
    IECInstancePtr childB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementProperty", ECValue(4));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *parentA, *childA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *parentB, *childB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{parentA, parentB});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Forward,
        relationshipClass->GetFullName(), elementClass->GetFullName()));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({childA.get(), childB.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
 * @betest                                       Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_CalculatedPropertiesValue)
    {
    // insert some widget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Test"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    ContentSpecificationP specification = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    specification->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label1", 1000, "\"Value\""));
    specification->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1100, "1+2"));
    specification->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label3", 1200, "this.MyID"));
    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(10, descriptor->GetVisibleFields().size());

    ASSERT_TRUE(descriptor->GetVisibleFields()[7]->IsCalculatedPropertyField());
    ASSERT_TRUE(descriptor->GetVisibleFields()[8]->IsCalculatedPropertyField());
    ASSERT_TRUE(descriptor->GetVisibleFields()[9]->IsCalculatedPropertyField());

    EXPECT_STREQ("label1", descriptor->GetVisibleFields()[7]->AsCalculatedPropertyField()->GetLabel().c_str());
    EXPECT_STREQ("label2", descriptor->GetVisibleFields()[8]->AsCalculatedPropertyField()->GetLabel().c_str());
    EXPECT_STREQ("label3", descriptor->GetVisibleFields()[9]->AsCalculatedPropertyField()->GetLabel().c_str());

    EXPECT_EQ(1000, descriptor->GetVisibleFields()[7]->GetPriority());
    EXPECT_EQ(1100, descriptor->GetVisibleFields()[8]->GetPriority());
    EXPECT_EQ(1200, descriptor->GetVisibleFields()[9]->GetPriority());

    EXPECT_STREQ("\"Value\"", descriptor->GetVisibleFields()[7]->AsCalculatedPropertyField()->GetValueExpression().c_str());
    EXPECT_STREQ("1+2", descriptor->GetVisibleFields()[8]->AsCalculatedPropertyField()->GetValueExpression().c_str());
    EXPECT_STREQ("this.MyID", descriptor->GetVisibleFields()[9]->AsCalculatedPropertyField()->GetValueExpression().c_str());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];

    EXPECT_STREQ("Value", jsonValues["CalculatedProperty_0"].GetString());
    EXPECT_STREQ("3", jsonValues["CalculatedProperty_1"].GetString());
    EXPECT_STREQ("Test", jsonValues["CalculatedProperty_2"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CalculatedPropertiesSpecificationAppliedForBaseClassAndDerived_CreatesOneField)
    {
    // set up the dataset
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    ECEntityClassCP classH = GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance) { instance.SetValue("PropertyF", ECValue(1000));});
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) { instance.SetValue("PropertyF", ECValue(2000));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceF, instanceH});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassF");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 900, "this.PropertyF"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(7, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("1000", jsonValues["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classE, "IntProperty")].IsNull());
    EXPECT_EQ(1000, jsonValues[FIELD_NAME(classF, "PropertyF")].GetInt());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classE, "LongProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classH, "PointProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classH, "Point2dProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classE, "ClassD")].IsNull());

    rapidjson::Document jsonDoc1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues1 = jsonDoc1["Values"];
    EXPECT_STREQ("2000", jsonValues1["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues1[FIELD_NAME(classE, "IntProperty")].IsNull());
    EXPECT_EQ(2000, jsonValues1[FIELD_NAME(classF, "PropertyF")].GetInt());
    EXPECT_TRUE(jsonValues1[FIELD_NAME(classE, "LongProperty")].IsNull());
    EXPECT_FALSE(jsonValues1[FIELD_NAME(classH, "PointProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classH, "Point2dProperty")].IsNull());
    EXPECT_TRUE(jsonValues1[FIELD_NAME(classE, "ClassD")].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentSerialization)
    {
    // insert widget instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("Description", ECValue("MyDescription"));
        instance.SetValue("MyID", ECValue("MyID"));
        instance.SetValue("IntProperty", ECValue(9));
        instance.SetValue("BoolProperty", ECValue(true));
        instance.SetValue("DoubleProperty", ECValue(7.0));
        instance.SetValue("LongProperty", ECValue((int64_t)123));
        instance.SetValue("DateProperty", ECValue(DateTime(2017, 5, 30)));
        });

    // set up input
    KeySetPtr input = KeySet::Create(*instance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // request for content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());
    rapidjson::Document contentJson = content->AsJson();
    EXPECT_TRUE(contentJson.IsObject());

    // validate descriptor
    ASSERT_TRUE(contentJson.HasMember("Descriptor"));
    RapidJsonValueCR descriptorJson = contentJson["Descriptor"];
    ASSERT_TRUE(descriptorJson.HasMember("Fields"));
    RapidJsonValueCR fields = descriptorJson["Fields"];
    ASSERT_TRUE(fields.IsArray());
    ASSERT_EQ(7, fields.Size());

    // validate content set
    ASSERT_TRUE(contentJson.HasMember("ContentSet"));
    ASSERT_TRUE(contentJson["ContentSet"].IsArray());
    ASSERT_EQ(1, contentJson["ContentSet"].Size());
    RapidJsonValueCR contentSetItem = contentJson["ContentSet"][0];
    EXPECT_TRUE(contentSetItem.IsObject());

    ASSERT_TRUE(contentSetItem.HasMember("PrimaryKeys"));
    RapidJsonValueCR primaryKeys = contentSetItem["PrimaryKeys"];
    ASSERT_TRUE(primaryKeys.IsArray() && 1 == primaryKeys.Size());
    EXPECT_STREQ(instance->GetClass().GetId().ToString().c_str(), primaryKeys[0]["ECClassId"].GetString());
    EXPECT_STREQ(instance->GetInstanceId().c_str(), primaryKeys[0]["ECInstanceId"].GetString());

    ASSERT_TRUE(contentSetItem.HasMember("Values"));
    RapidJsonValueCR value = contentSetItem["Values"];

    for(rapidjson::Value::ConstValueIterator field = fields.Begin(); field != fields.End(); field++)
        {
        Utf8String name = (*field)["Name"].GetString();
        ASSERT_TRUE(value.HasMember(name.c_str()));

        if (name.Equals(FIELD_NAME(m_widgetClass, "Description")))
            {
            EXPECT_STREQ("MyDescription", value[name.c_str()].GetString());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("string", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(m_widgetClass, "MyID")))
            {
            EXPECT_STREQ("MyID", value[name.c_str()].GetString());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("string", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(m_widgetClass, "IntProperty")))
            {
            EXPECT_EQ(9, value[name.c_str()].GetInt());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("int", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(m_widgetClass, "BoolProperty")))
            {
            EXPECT_TRUE(value[name.c_str()].GetBool());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("boolean", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(m_widgetClass, "DoubleProperty")))
            {
            EXPECT_DOUBLE_EQ(7.0, value[name.c_str()].GetDouble());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("double", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(m_widgetClass, "LongProperty")))
            {
            EXPECT_EQ(123, value[name.c_str()].GetInt64());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("long", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(m_widgetClass, "DateProperty")))
            {
            EXPECT_STREQ("2017-05-30T00:00:00.000", value[name.c_str()].GetString());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("dateTime", (*field)["Type"]["TypeName"].GetString());
            }
        else
            FAIL();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, RelatedPropertyValuesAreCorrectWhenSelectionIncludesInstanceOfRelatedInstanceClass)
    {
    // set up the dataset
    ECRelationshipClassCP widgetHasGadgetsRelationship = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test Gadget"));});
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Widget 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Widget 2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{gadget, widget2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));
    spec->AddPropertyOverride(*new PropertySpecification("Description", 1000, "", "", true));
    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(3, descriptor->GetVisibleFields().size()); // Gadget.Description, related Widget.MyID, Widget.MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_TRUE(values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].IsNull());
    EXPECT_STREQ("Test Gadget", values1[descriptor->GetVisibleFields()[1]->GetName().c_str()].GetString());
    EXPECT_STREQ("Test Widget 1", values1[descriptor->GetVisibleFields()[2]->GetName().c_str()].GetString());

    recordJson = contentSet.Get(1)->AsJson();
    RapidJsonValueCR values2 = recordJson["Values"];
    EXPECT_STREQ("Test Widget 2", values2[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    EXPECT_TRUE(values2[descriptor->GetVisibleFields()[1]->GetName().c_str()].IsNull());
    EXPECT_TRUE(values2[descriptor->GetVisibleFields()[2]->GetName().c_str()].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPropertyValueInstanceKeys)
    {
    // set up the dataset
    ECRelationshipClassCP widgetHasGadgetsRelationship = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget);

    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);
    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID,IntProperty", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(5, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID, Widget.IntProperty

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp0(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp1(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp2(*descriptor->GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp3(*descriptor->GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
    ASSERT_EQ(1, widgetMyIdFieldInstanceKeys.size());
    EXPECT_EQ(widgetKey, widgetMyIdFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp4(*descriptor->GetVisibleFields()[4]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> widgetIntpropertyFieldInstanceKeys = record->GetPropertyValueKeys(fp4);
    ASSERT_EQ(1, widgetIntpropertyFieldInstanceKeys.size());
    EXPECT_EQ(widgetKey, widgetIntpropertyFieldInstanceKeys[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPropertyValueInstanceKeysWhenColumnsAreMerged)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);
    ECClassInstanceKey sprocketKey = RulesEngineTestHelpers::GetInstanceKey(*sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget,Sprocket", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID + Sprocket.MyID, Gadget.Description + Sprocket.MyID, Gadget.Widget, Sprocket.Gadget

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ContentSetItem::FieldProperty fp00(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 0);
    ContentSetItem::FieldProperty fp01(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 1);
    ContentSetItem::FieldProperty fp10(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
    ContentSetItem::FieldProperty fp11(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 1);
    ContentSetItem::FieldProperty fp20(*descriptor->GetVisibleFields()[2]->AsPropertiesField(), 0);
    ContentSetItem::FieldProperty fp30(*descriptor->GetVisibleFields()[3]->AsPropertiesField(), 0);

    ContentSetItemCPtr record0 = contentSet.Get(0); // gadget

    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record0->GetPropertyValueKeys(fp00);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);

    bvector<ECClassInstanceKey> sprocketMyidFieldInstanceKeys = record0->GetPropertyValueKeys(fp01);
    EXPECT_TRUE(sprocketMyidFieldInstanceKeys.empty());

    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record0->GetPropertyValueKeys(fp10);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    bvector<ECClassInstanceKey> sprocketDescriptionFieldInstanceKeys = record0->GetPropertyValueKeys(fp11);
    EXPECT_TRUE(sprocketDescriptionFieldInstanceKeys.empty());

    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record0->GetPropertyValueKeys(fp20);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    bvector<ECClassInstanceKey> sprocketGadgetFieldInstanceKeys = record0->GetPropertyValueKeys(fp30);
    EXPECT_TRUE(sprocketGadgetFieldInstanceKeys.empty());

    ContentSetItemCPtr record1 = contentSet.Get(1); // sprocket

    gadgetMyidFieldInstanceKeys = record1->GetPropertyValueKeys(fp00);
    EXPECT_TRUE(gadgetMyidFieldInstanceKeys.empty());

    sprocketMyidFieldInstanceKeys = record1->GetPropertyValueKeys(fp01);
    ASSERT_EQ(1, sprocketMyidFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketMyidFieldInstanceKeys[0]);

    gadgetDescriptionFieldInstanceKeys = record1->GetPropertyValueKeys(fp10);
    EXPECT_TRUE(gadgetDescriptionFieldInstanceKeys.empty());

    sprocketDescriptionFieldInstanceKeys = record1->GetPropertyValueKeys(fp11);
    ASSERT_EQ(1, sprocketDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketDescriptionFieldInstanceKeys[0]);

    gadgetWidgetFieldInstanceKeys = record1->GetPropertyValueKeys(fp20);
    EXPECT_TRUE(gadgetWidgetFieldInstanceKeys.empty());

    sprocketGadgetFieldInstanceKeys = record1->GetPropertyValueKeys(fp30);
    ASSERT_EQ(1, sprocketGadgetFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketGadgetFieldInstanceKeys[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPropertyValueInstanceKeysWhenRowsAreMerged)
    {
    // set up the dataset
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    ECClassInstanceKey gadgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*gadget1);
    ECClassInstanceKey gadgetKey2 = RulesEngineTestHelpers::GetInstanceKey(*gadget2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp0(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(2, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetMyidFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetMyidFieldInstanceKeys[1]);

    ContentSetItem::FieldProperty fp1(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(2, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetDescriptionFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetDescriptionFieldInstanceKeys[1]);

    ContentSetItem::FieldProperty fp2(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(2, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetWidgetFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetWidgetFieldInstanceKeys[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPropertyValueInstanceKeysWhenRowsAndColumnsAreMerged)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);
    ECClassInstanceKey sprocketKey = RulesEngineTestHelpers::GetInstanceKey(*sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget,Sprocket", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID + Sprocket.MyID, Gadget.Description + Sprocket.MyID, Gadget.Widget, Sprocket.Gadget

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp00(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp00);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp01(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 1);
    bvector<ECClassInstanceKey> sprocketMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp01);
    ASSERT_EQ(1, sprocketMyidFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketMyidFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp10(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp10);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp11(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 1);
    bvector<ECClassInstanceKey> sprocketDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp11);
    ASSERT_EQ(1, sprocketDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp20(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp20);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp30(*content->GetDescriptor().GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> sprocketGadgetFieldInstanceKeys = record->GetPropertyValueKeys(fp30);
    ASSERT_EQ(1, sprocketGadgetFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketGadgetFieldInstanceKeys[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsRelatedPropertyValuesWhenSelectingFromMultipleClasses)
    {
    // set up the dataset
    ECRelationshipClassCP rel = GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        });
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        });
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Sprocket"));
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetId(), rel));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Sprocket", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with Sprocket MyID and Description), Sprocket.Gadget, Gadget.MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ContentSetItem::FieldProperty fp(*descriptor->GetVisibleFields()[8]->AsPropertiesField(), 0);

    ContentSetItemCPtr widgetRecord = contentSet.Get(0);
    bvector<ECClassInstanceKey> widgetKeys = widgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(widgetKeys.empty());
    EXPECT_TRUE(widgetRecord->AsJson()["Values"][fp.GetField().GetName().c_str()].IsNull());

    ContentSetItemCPtr sprocketRecord = contentSet.Get(1);
    bvector<ECClassInstanceKey> sprocketKeys = sprocketRecord->GetPropertyValueKeys(fp);
    ASSERT_EQ(1, sprocketKeys.size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget), sprocketKeys[0]);
    EXPECT_STREQ("Gadget", sprocketRecord->AsJson()["Values"][fp.GetField().GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsRelatedPropertyValuesWhenSelectingFromMultipleClassesAndFieldsAreMerged)
    {
    ECRelationshipClassCP rel_gs = GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_wg = GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        });
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_wg, *widget, *gadget1);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Sprocket"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_gs, *gadget2, *sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Sprocket", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Both,
        "RulesEngineTest:GadgetHasSprockets,WidgetHasGadget", "RulesEngineTest:Gadget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with Sprocket MyID and Description), Gadget.MyID, Sprocket.Gadget

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItem::FieldProperty fp(*content->GetDescriptor().GetVisibleFields()[7]->AsPropertiesField(), 0);
    ContentSetItemCPtr record = contentSet.Get(0);
    bvector<ECClassInstanceKey> keys = record->GetPropertyValueKeys(fp);
    ASSERT_EQ(2, keys.size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget1), keys[0]);
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget2), keys[1]);
    EXPECT_STREQ("Gadget", record->AsJson()["Values"][fp.GetField().GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsNullRelatedPropertyValuesWhenSelectingFromMultipleClassesWithNoRelationships)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        });
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        });
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Sprocket"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget,Sprocket", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(10, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with MyID and Description of Gadget and Sprocket), Gadget.Widget, Sprocket.Gadget, Gadget.MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    ContentSetItem::FieldProperty fp(*descriptor->GetVisibleFields()[9]->AsPropertiesField(), 0);

    ContentSetItemCPtr widgetRecord = contentSet.Get(0);
    bvector<ECClassInstanceKey> widgetKeys = widgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(widgetKeys.empty());

    ContentSetItemCPtr gadgetRecord = contentSet.Get(1);
    bvector<ECClassInstanceKey> gadgetKeys = gadgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(gadgetKeys.empty());

    ContentSetItemCPtr sprocketRecord = contentSet.Get(2);
    bvector<ECClassInstanceKey> sprocketKeys = sprocketRecord->GetPropertyValueKeys(fp);
    ASSERT_EQ(1, sprocketKeys.size());
    EXPECT_EQ(ECClassInstanceKey(m_gadgetClass, ECInstanceId()), sprocketKeys[0]);
    EXPECT_TRUE(sprocketRecord->AsJson()["Values"][fp.GetField().GetName().c_str()].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectsRelatedPropertyValuesWhenSelectingFromDerivedRelatedInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="BaseRelatedClass">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="RelatedProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="DerivedRelatedClass">
        <BaseClass>BaseRelatedClass</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasRelatedInstances" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="is of" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="classifies" polymorphic="true">
            <Class class="BaseRelatedClass"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsRelatedPropertyValuesWhenSelectingFromDerivedRelatedInstances)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseRelatedClass = GetClass("BaseRelatedClass");
    ECClassCP derivedRelatedClass = GetClass("DerivedRelatedClass");
    ECRelationshipClassCP rel = GetRelationshipClass("ElementHasRelatedInstances");

    // set up the dataset
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr relatedInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedRelatedClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element1, *relatedInstance1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr relatedInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseRelatedClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element2, *relatedInstance2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        rel->GetFullName(), baseRelatedClass->GetFullName(), "RelatedProperty", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::PropertyPane, 0, *KeySet::Create(), nullptr,
        options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // Element.ElementProperty + <BaseRelatedClass.RelatedProperty, DerivedRelatedClass.RelatedProperty>

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItem::FieldProperty fp(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);

    ContentSetItemCPtr record = contentSet.Get(0);
    bvector<ECClassInstanceKey> keys = record->GetPropertyValueKeys(fp);
    ASSERT_EQ(2, keys.size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*relatedInstance1), keys[0]);
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*relatedInstance2), keys[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNullPropertyValueInstanceKeyWhenThereIsNoRelatedInstance)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp0(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp1(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp2(*descriptor->GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp3(*descriptor->GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
    ASSERT_EQ(1, widgetMyIdFieldInstanceKeys.size());
    EXPECT_FALSE(widgetMyIdFieldInstanceKeys[0].IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNullPropertyValueInstanceKeyWhenThereIsNoRelatedInstance_MergedValuesCase)
    {
    // set up the dataset
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    ECClassInstanceKey gadgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*gadget1);
    ECClassInstanceKey gadgetKey2 = RulesEngineTestHelpers::GetInstanceKey(*gadget2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(),
        nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp0(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(2, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetMyidFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetMyidFieldInstanceKeys[1]);

    ContentSetItem::FieldProperty fp1(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(2, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetDescriptionFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetDescriptionFieldInstanceKeys[1]);

    ContentSetItem::FieldProperty fp2(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(2, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetWidgetFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetWidgetFieldInstanceKeys[1]);

    ContentSetItem::FieldProperty fp3(*content->GetDescriptor().GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
    ASSERT_EQ(2, widgetMyIdFieldInstanceKeys.size());
    EXPECT_FALSE(widgetMyIdFieldInstanceKeys[0].IsValid());
    EXPECT_FALSE(widgetMyIdFieldInstanceKeys[1].IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsDisplayLabelProperty)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesRelatedInstanceInLabelOverrideCondition)
    {
    // set up the dataset
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [] (IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget label")); });
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Gadget ID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\" ANDALSO gadgetAlias.MyID = \"Gadget ID\"", 1, "this.MyID", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification(*new RelationshipStepSpecification("RulesEngineTest:WidgetHasGadget", RequiredRelationDirection_Forward, "RulesEngineTest:Gadget")), "gadgetAlias"));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Widget label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_UsesRelatedInstanceInLabelOverrideExpression)
    {
    // set up the dataset
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [] (IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget label")); });
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Gadget label"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "gadget.MyID", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget", "gadget"));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Gadget label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesRelatedInstanceInFilterExpression, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesRelatedInstanceInFilterExpression)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP rel = GetClass("ModelContainsElements")->GetRelationshipClassCP();

    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR inst){inst.SetValue("ModelName", ECValue("a"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model1, *element1);

    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR inst){inst.SetValue("ModelName", ECValue("b"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model2, *element2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "model.ModelName = \"a\"", elementClass->GetFullName(), false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Backward, modelClass->GetFullName())), "model"));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*element1), contentSet[0]->GetKeys().front());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesMultiRelationshipRelatedInstanceInFilterExpression, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="ElementA" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementA" />
    <ECRelationshipClass typeName="A_To_B" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="refers" polymorphic="true">
            <Class class="ElementA"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is refered by" polymorphic="true">
            <Class class="ElementB" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementB" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesMultiRelationshipRelatedInstanceInFilterExpression)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementAClass = GetClass("ElementA");
    ECClassCP elementBClass = GetClass("ElementB");
    ECRelationshipClassCP rel1 = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR inst) {inst.SetValue("ModelName", ECValue("a")); });
    IECInstancePtr elementA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr elementB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel1, *model1, *elementA1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel2, *elementA1, *elementB1);

    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR inst) {inst.SetValue("ModelName", ECValue("b")); });
    IECInstancePtr elementA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr elementB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel1, *model2, *elementA2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel2, *elementA2, *elementB2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "model.ModelName = \"a\"", elementBClass->GetFullName(), false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(rel2->GetFullName(), RequiredRelationDirection_Backward),
        new RelationshipStepSpecification(rel1->GetFullName(), RequiredRelationDirection_Backward)
        }), "model"));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*elementB1), contentSet[0]->GetKeys().front());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HandlesManyEndOfTheRelationshipInRelatedInstanceSpecification, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Model" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, HandlesManyEndOfTheRelationshipInRelatedInstanceSpecification)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP rel = GetClass("ModelContainsElements")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR inst) {inst.SetValue("ElementName", ECValue("a")); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR inst) {inst.SetValue("ElementName", ECValue("a")); });
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR inst) {inst.SetValue("ElementName", ECValue("b")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "element.ElementName = \"a\"", modelClass->GetFullName(), false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward),
        }), "element"));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*model), contentSet[0]->GetKeys().front());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesContentWithRelatedInstancesDerivingFromMultipleBaseClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ISubModeledElement" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0">
                <AppliesToEntityClass>Element</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
        <BaseClass>ISubModeledElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
        <BaseClass>ISubModeledElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="LinkElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Model">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ModelA">
        <BaseClass>Model</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ModelB">
        <BaseClass>Model</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ModelModelsElement" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is contained by" polymorphic="true">
            <Class class="ISubModeledElement" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementHasLinks" strength="referencing" modifier="None">
        <Source multiplicity="(1..*)" roleLabel="has" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
            <Class class="LinkElement"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentWithRelatedInstancesDerivingFromMultipleBaseClasses)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP modelAClass = GetClass("ModelA");
    ECClassCP mixinClass = GetClass("ISubModeledElement");
    ECClassCP elementAClass = GetClass("ElementA");
    ECClassCP elementBClass = GetClass("ElementB");
    ECClassCP linkElementClass = GetClass("LinkElement");
    ECRelationshipClassCP rel_ModelContainsElements = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_ModelModelsElement = GetClass("ModelModelsElement")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_ElementHasLinks = GetClass("ElementHasLinks")->GetRelationshipClassCP();

    IECInstancePtr modelA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelAClass);
    IECInstancePtr modeledElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr containedElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr linkElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *linkElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_ModelContainsElements, *modelA, *containedElement);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_ModelModelsElement, *modelA, *modeledElement);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_ElementHasLinks, *modeledElement, *linkElement);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementBClass->GetFullName(), false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(rel_ModelContainsElements->GetFullName(), RequiredRelationDirection_Backward, modelClass->GetFullName()),
        new RelationshipStepSpecification(rel_ModelModelsElement->GetFullName(), RequiredRelationDirection_Forward, mixinClass->GetFullName()),
        new RelationshipStepSpecification(rel_ElementHasLinks->GetFullName(), RequiredRelationDirection_Forward, linkElementClass->GetFullName()),
        }), "link1", true));
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(rel_ModelContainsElements->GetFullName(), RequiredRelationDirection_Backward, modelClass->GetFullName()),
        new RelationshipStepSpecification(rel_ModelModelsElement->GetFullName(), RequiredRelationDirection_Forward, mixinClass->GetFullName()),
        new RelationshipStepSpecification(rel_ElementHasLinks->GetFullName(), RequiredRelationDirection_Forward, linkElementClass->GetFullName()),
        }), "link2", true));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*containedElement), contentSet[0]->GetKeys().front());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_SetsDisplayLabelProperty)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreEqual)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreEqual)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreEqual", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreDifferent)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label 1"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label 2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_MultipleInstances()).c_str(), contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreDifferent)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label 1"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label 2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreDifferent", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_MultipleInstances()).c_str(), contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsDisplayLabelPropertyWhenMergingRecordsAndClassesAreDifferent)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_MultipleInstances()).c_str(), contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsClassWhenMergingRecordsAndClassesAreEqual)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(m_widgetClass, contentSet.Get(0)->GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNullClassWhenMergingRecordsAndClassesDifferent)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(nullptr, contentSet.Get(0)->GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsPointPropertyContent)
    {
    // set up the dataset
    ECEntityClassCP ecClass = GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("PointProperty", ECValue(DPoint3d::From(1, 2, 3)));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassH", false);
    spec->AddPropertyOverride(*new PropertySpecification("PointProperty", 1000, "", "", true));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();

    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"(
        {
        "%s": {"x": 1.0, "y": 2.0, "z": 3.0}
        })", FIELD_NAME(ecClass, "PointProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"(
        {
        "%s": "X: 1.00 Y: 2.00 Z: 3.00"
        })", FIELD_NAME(ecClass, "PointProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, RecordFromDifferrentSpecificationsGetMerged)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("Description", ECValue("Test Gadget")); });
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("Description", ECValue("Test Widget")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification *specWidget = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    ContentInstancesOfSpecificClassesSpecification *specGadget = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false);
    rule->AddSpecification(*specWidget);
    rule->AddSpecification(*specGadget);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values = recordJson["Values"];

    Utf8PrintfString expectedValue(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());
    EXPECT_STREQ(expectedValue.c_str(), values[content->GetDescriptor().GetVisibleFields()[0]->GetName().c_str()].GetString());
    EXPECT_TRUE(contentSet.Get(0)->IsMerged(content->GetDescriptor().GetVisibleFields()[0]->GetName()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LabelOverride_DisplayLabelGetCreatedfromDifferentSpecifications)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Widget"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); // no display label in the descriptor

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(8, content->GetDescriptor().GetVisibleFields().size()); // content created with a descriptor that has a display label

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ("Test Widget", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_DisplayLabelGetCreatedFromDifferentSpecifications)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Widget"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); // no display label in the descriptor

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(8, content->GetDescriptor().GetVisibleFields().size()); // content created with a descriptor that has a display label

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ("Test Widget", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsCachedWhenParametersEqual)
    {
    // set up input
    KeySetPtr input = KeySet::Create(bvector<ECClassInstanceKey>{
        ECClassInstanceKey(m_widgetClass, ECInstanceId((uint64_t)1)),
        ECClassInstanceKey(m_widgetClass, ECInstanceId((uint64_t)2))
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_EQ(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_Connection)
    {
    // set up a different connection
    ECDbTestProject project2;
    project2.Create("ContentDescriptorIsNotCachedWhenParametersDifferent_Connection", "RulesEngineTest.01.00.ecschema.xml");
    IConnectionPtr connection2 = m_manager->GetConnections().CreateConnection(project2.GetECDb());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(project2.GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);

    CloseConnection(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_ContentDisplayType)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Graphics, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Grid, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_Provider)
    {
    // set up selection 1
    SelectionInfoCPtr selection1 = SelectionInfo::Create("A", false);

    // set up selection 2
    SelectionInfoCPtr selection2 = SelectionInfo::Create("B", false);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), selection1.get(), options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), selection2.get(), options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_SubSelection)
    {
    // set up selection 1
    SelectionInfoCPtr selection1 = SelectionInfo::Create("", false);

    // set up selection 2
    SelectionInfoCPtr selection2 = SelectionInfo::Create("", true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), selection1.get(), options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), selection2.get(), options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_Keys)
    {
    // set up input 1
    KeySetPtr input1 = KeySet::Create(bvector<ECClassInstanceKey>{
        ECClassInstanceKey(m_widgetClass, ECInstanceId((uint64_t)1)),
        ECClassInstanceKey(m_widgetClass, ECInstanceId((uint64_t)2))
        });

    // set up input 2
    KeySetPtr input2 = KeySet::Create(bvector<ECClassInstanceKey>{
        ECClassInstanceKey(m_widgetClass, ECInstanceId((uint64_t)3)),
        ECClassInstanceKey(m_widgetClass, ECInstanceId((uint64_t)4))
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input1, nullptr, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input2, nullptr, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_RulesetId)
    {
    // create the rule set 1
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("ContentDescriptorIsNotCachedWhenParametersDifferent_RulesetId_1", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules1);
    ContentRuleP rule1 = new ContentRule("", 1, false);
    rule1->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules1->AddPresentationRule(*rule1);

    // create the rule set 2
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("ContentDescriptorIsNotCachedWhenParametersDifferent_RulesetId_2", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules2);
    ContentRuleP rule2 = new ContentRule("", 1, false);
    rule2->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules2->AddPresentationRule(*rule2);

    // request
    RulesDrivenECPresentationManager::ContentOptions options1(rules1->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options1.GetJson()).get();
    RulesDrivenECPresentationManager::ContentOptions options2(rules2->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options2.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsRemovedFromCacheAfterConnectionClose)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    // simulate re-opening
    CloseConnection(*m_connections->GetConnection(s_project->GetECDb()));
    m_manager->GetConnections().CreateConnection(s_project->GetECDb());

    // request again
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    // verify the two objects aren't equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentModifierAppliesPropertyHidingOverride)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {
        instance.SetValue("Description", ECValue("TestDescription"));
        instance.SetValue("MyID", ECValue("TestID"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Gadget");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertyOverride(*new PropertySpecification("Widget", 1000, "", "", false));
    modifier->AddPropertyOverride(*new PropertySpecification("Description", 1000, "", "", false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Gadget.MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestID", values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentModifierAppliesRelatedPropertiesSpecification)
    {
    // set up the dataset
    ECRelationshipClassCP widgetHasGadgetsRelationship = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Gadget");
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets",
        "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    BeInt64Id widgetId = ECInstanceId::FromString(widget->GetInstanceId().c_str());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("GadgetID", values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    EXPECT_TRUE(values1[descriptor->GetVisibleFields()[1]->GetName().c_str()].IsNull());
    EXPECT_EQ(widgetId.GetValueUnchecked(), values1[descriptor->GetVisibleFields()[2]->GetName().c_str()].GetInt64());
    EXPECT_STREQ("WidgetID", values1[descriptor->GetVisibleFields()[3]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_ContentModifierAppliesCalculatedPropertiesSpecification)
    {
    // insert some widget instances
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget,Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Widget");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.MyID"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); //Gadget_Widget_MyId, Gadget_Widget_Description, Gadget_Widget, Widget_IntProperty, Widget_BoolProperty, Widget_DoubleProperty, Widget_LongProperty, Widget_Date, CalculatedProperty_0

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_STREQ("WidgetID", jsonValues2["CalculatedProperty_0"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_ContentModifierOnBaseClassPropertyIsAppliedToOnlyOneChildClass)
    {
    // set up data set
    ECClassCP baseOfBAndC = GetClass("RulesEngineTest", "BaseOfBAndC");
    ECEntityClassCP classB = GetClass("RulesEngineTest", "ClassB")->GetEntityClassCP();
    ECEntityClassCP classC = GetClass("RulesEngineTest", "ClassC")->GetEntityClassCP();
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("B")); });
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("C")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassB,ClassC", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassC");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.MyID"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("B", jsonValues[FIELD_NAME(baseOfBAndC, "MyID")].GetString());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME((bvector<ECClassCP>{classB, classC}), "A")].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_STREQ("C", jsonValues2[FIELD_NAME(baseOfBAndC, "MyID")].GetString());
    EXPECT_STREQ("C", jsonValues2["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME((bvector<ECClassCP>{classB, classC}), "A")].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_ContentModifierIsAppliedToOnlyOneChildClassPolymorphically)
    {
    // set up data set
    ECClassCP baseOfBAndC = GetClass("RulesEngineTest", "BaseOfBAndC");
    ECEntityClassCP classB = GetClass("RulesEngineTest", "ClassB")->GetEntityClassCP();
    ECEntityClassCP classC = GetClass("RulesEngineTest", "ClassC")->GetEntityClassCP();
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("B")); });
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("C")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:BaseOfBandC", true);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassC");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.MyID"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); //BaseOfBAndC_MyId, CalculatedProperty_0

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("B", jsonValues[FIELD_NAME(baseOfBAndC, "MyID")].GetString());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_STREQ("C", jsonValues2[FIELD_NAME(baseOfBAndC, "MyID")].GetString());
    EXPECT_STREQ("C", jsonValues2["CalculatedProperty_0"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_ContentModifierAppliesCalculatedPropertiesSpecification)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // set up input
    KeySetPtr input = KeySet::Create(*gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*rule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Widget");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.MyID"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "Description")].IsNull());
    EXPECT_STREQ("WidgetID", jsonValues[FIELD_NAME(m_widgetClass, "MyID")].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "IntProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "BoolProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "DoubleProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "LongProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "DateProperty")].IsNull());
    EXPECT_STREQ("WidgetID", jsonValues["CalculatedProperty_0"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Kilinskas                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_GetsRelatedValueThroughCalculatedProperty)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // set up input
    KeySetPtr input = KeySet::Create(*gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*rule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Widget");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 1200, "this.GetRelatedValue(\"RulesEngineTest:WidgetHasGadget\", \"Forward\", \"RulesEngineTest:Gadget\", \"MyID\")"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); //Widget_MyId,Widget_Description, Widget_IntProperty, Widget_BoolProperty, Widget_DoubleProperty, Widget_LongProperty, Widget_Date, CalculatedProperty_0

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "Description")].IsNull());
    EXPECT_STREQ("WidgetID", jsonValues[FIELD_NAME(m_widgetClass, "MyID")].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "IntProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "BoolProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "DoubleProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "LongProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "DateProperty")].IsNull());
    EXPECT_STREQ("GadgetID", jsonValues["CalculatedProperty_0"].GetString());
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_ContentModifierAppliesCalculatedPropertiesSpecificationPolymorphically)
    {
    // set up data set
    ECRelationshipClassCR relationshipClassDHasClassE = *m_schema->GetClassCP("ClassDHasClassE")->GetRelationshipClassCP();
    ECEntityClassCP classD = GetClass("RulesEngineTest", "ClassD")->GetEntityClassCP();
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    ECEntityClassCP classG = GetClass("RulesEngineTest", "ClassG")->GetEntityClassCP();
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance) { instance.SetValue("PropertyF", ECValue(1000)); });
    IECInstancePtr instanceG = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classG);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipClassDHasClassE, *instanceD, *instanceE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipClassDHasClassE, *instanceD, *instanceF);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipClassDHasClassE, *instanceD, *instanceG);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceD);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassE"));
    rules->AddPresentationRule(*rule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassF");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 900, "this.PropertyF"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(6, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    BeInt64Id instanceDId = ECInstanceId::FromString(instanceD->GetInstanceId().c_str());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_EQ(instanceDId.GetValueUnchecked(), jsonValues[FIELD_NAME(classE, "ClassD")].GetInt64());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classE, "IntProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classE, "LongProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classF, "PropertyF")].IsNull());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classG, "D")].IsNull());

    rapidjson::Document jsonDoc1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues1 = jsonDoc1["Values"];
    EXPECT_EQ(instanceDId.GetValueUnchecked(), jsonValues1[FIELD_NAME(classE, "ClassD")].GetInt64());
    EXPECT_TRUE(jsonValues1[FIELD_NAME(classE, "IntProperty")].IsNull());
    EXPECT_TRUE(jsonValues1[FIELD_NAME(classE, "LongProperty")].IsNull());
    EXPECT_EQ(1000, jsonValues1[FIELD_NAME(classF, "PropertyF")].GetInt());
    EXPECT_STREQ("1000", jsonValues1["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues1[FIELD_NAME(classG, "D")].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(2)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_EQ(instanceDId.GetValueUnchecked(), jsonValues2[FIELD_NAME(classE, "ClassD")].GetInt64());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(classE, "IntProperty")].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(classE, "LongProperty")].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(classF, "PropertyF")].IsNull());
    EXPECT_TRUE(jsonValues2["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(classG, "D")].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_ContentModifierDoesNotApplyCalculatedPropertyForNonExistingClass)
    {
    // insert widget
    ECRelationshipClassCR relationshipClassDHasClassE = *m_schema->GetClassCP("ClassDHasClassE")->GetRelationshipClassCP();
    ECEntityClassCP classD = GetClass("RulesEngineTest", "ClassD")->GetEntityClassCP();
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(12345)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipClassDHasClassE, *instanceD, *instanceE);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceD);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassE"));
    rules->AddPresentationRule(*rule);


    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "NonExistingClass");//There is no such class in database
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 900, "this.PropertyF"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    //NonExistingClass causes assertion failure
    IGNORE_BE_ASSERT()

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();

    //Calculated property is not applied
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    BeInt64Id instanceDId = ECInstanceId::FromString(instanceD->GetInstanceId().c_str());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_EQ(12345, jsonValues[FIELD_NAME(classE, "IntProperty")].GetInt());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classE, "LongProperty")].IsNull());
    EXPECT_EQ(instanceDId.GetValueUnchecked(), jsonValues[FIELD_NAME(classE, "ClassD")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ContentModifierAppliesCalculatedPropertiesSpecification)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{widgetInstance, gadgetInstance});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Gadget");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.MyID"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); //Gadget_Widget_MyId, Gadget_Widget_Description, Gadget_Widget, Widget_IntProperty, Widget_BoolProperty, Widget_DoubleProperty, Widget_LongProperty, Widget_Date, CalculatedProperty_0

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_TRUE(jsonValues[FIELD_NAME((bvector<ECClassCP>{m_gadgetClass, m_widgetClass}), "Description")].IsNull());
    EXPECT_STREQ("GadgetID", jsonValues[FIELD_NAME((bvector<ECClassCP>{m_gadgetClass, m_widgetClass}), "MyID")].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "IntProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "BoolProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "DoubleProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "LongProperty")].IsNull());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_widgetClass, "DateProperty")].IsNull());
    EXPECT_STREQ("GadgetID", jsonValues["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_gadgetClass, "Widget")].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_TRUE(jsonValues2[FIELD_NAME((bvector<ECClassCP>{m_gadgetClass, m_widgetClass}), "Description")].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME((bvector<ECClassCP>{m_gadgetClass, m_widgetClass}), "MyID")].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(m_widgetClass, "IntProperty")].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(m_widgetClass, "BoolProperty")].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(m_widgetClass, "DoubleProperty")].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(m_widgetClass, "LongProperty")].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(m_widgetClass, "DateProperty")].IsNull());
    EXPECT_TRUE(jsonValues2["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(m_gadgetClass, "Widget")].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsCorrectEnumValues)
    {
    // set up data set
    ECEntityClassCP classQ = GetClass("RulesEngineTest", "ClassQ")->GetEntityClassCP();
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance)
        {
        instance.SetValue("IntEnum", ECValue(2));
        instance.SetValue("StrEnum", ECValue("Three"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassQ", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values = recordJson["Values"];
    RapidJsonValueCR dispalyValues = recordJson["DisplayValues"];
    EXPECT_EQ(2, values[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetInt());
    EXPECT_STREQ("M", dispalyValues[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    EXPECT_STREQ("Three", values[descriptor->GetVisibleFields()[1]->GetName().c_str()].GetString());
    EXPECT_STREQ("1", dispalyValues[descriptor->GetVisibleFields()[1]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_AppliesWithContentInstancesOfSpecificClasses)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestID", values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_AppliesWithSelectedNodeInstances)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // set up input
    KeySetPtr input = KeySet::Create(*widget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestID", values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_AppliesWithContentRelatedInstances)
    {
    // set up the dataset
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // set up input
    KeySetPtr input = KeySet::Create(*gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecification* spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "", "RulesEngineTest:Widget");
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestID", values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesHigherPriorityDisplayOverride)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));
    spec->AddPropertyOverride(*new PropertySpecification("MyID", 900, "", "", false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestID", values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesHigherPriorityHideOverride)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("MyID", 900, "", "", true));
    spec->AddPropertyOverride(*new PropertySpecification("Description", 900, "", "", true));
    spec->AddPropertyOverride(*new PropertySpecification("Description", 1000, "", "", false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); //Widget_MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestID", values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesByOrderOfDefinitionIfPrioritiesEqual)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("IntProperty", ECValue(1));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Description", 1000, "", "", false));
    spec->AddPropertyOverride(*new PropertySpecification("LongProperty", 1000, "", "", false));
    spec->AddPropertyOverride(*new PropertySpecification("Description", 1000, "", "", true));
    spec->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "", "", true));
    spec->AddPropertyOverride(*new PropertySpecification("LongProperty", 1000, "", "", true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_IntProperty

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_EQ(1, values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesSpecificationOverrideIfPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesSpecificationOverrideIfPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "", false));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "", true));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(0, descriptor->GetVisibleFields().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesModifierOverSpecificationOverrideIfPriorityHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesModifierOverSpecificationOverrideIfPriorityHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "", false));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", "", true));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME(classA, "UserLabel"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", "", false) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "", true));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // ClassB_UserLabel
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", "", false) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", "", true));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassB_UserLabel, ClassB_ClassA_UserLabel
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Polymorphism_AppliesDisplayFromBaseClassOnDerivedClass)
    {
    // set up the dataset
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance) {
        instance.SetValue("IntProperty", ECValue(10));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassF", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("IntProperty", 1500, "", "", true)); // base class specification
    spec->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "", "", false));
    spec->AddPropertyOverride(*new PropertySpecification("PropertyF", 1000, "", "", false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); //ClassF_IntProperty

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_EQ(10, values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Polymorphism_AppliesHideFromBaseClassOnDerivedClass)
    {
    // set up the dataset
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("PropertyF", ECValue(456));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(),
        1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassF", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassE");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "", "", true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassE_IntProperty, ClassF_PropertyF

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(123, values[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetInt());
    EXPECT_EQ(456, values[descriptor->GetVisibleFields()[1]->GetName().c_str()].GetInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "", nullptr, new PropertyEditorSpecification("Custom Editor 1")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "", nullptr, new PropertyEditorSpecification("Custom Editor 2")));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("Custom Editor 1", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Priorities_AppliesModifierOverSpecificationOverrideWhenPriorityHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Priorities_AppliesModifierOverSpecificationOverrideWhenPriorityHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "", nullptr, new PropertyEditorSpecification("Custom Editor 1")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", "", nullptr, new PropertyEditorSpecification("Custom Editor 2")));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("Custom Editor 2", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", "", nullptr, new PropertyEditorSpecification("Custom Editor 1")) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "", nullptr, new PropertyEditorSpecification("Custom Editor 2")));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB_UserLabel
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[1]->GetEditor());
    EXPECT_STREQ("Custom Editor 1", descriptor->GetVisibleFields()[1]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", "", nullptr, new PropertyEditorSpecification("Custom Editor 1")) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", "", nullptr, new PropertyEditorSpecification("Custom Editor 2")));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB_UserLabel
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[1]->GetEditor());
    EXPECT_STREQ("Custom Editor 2", descriptor->GetVisibleFields()[1]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_FieldsMerging_GetOneFieldWhenPropertiesAndEditorsAreSimilar)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{widget, gadget});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true, new PropertyEditorSpecification("IDEditor")));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Gadget_Widget_MyID

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("IDEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_FieldsMerging_GetDifferentFieldsWhenPropertiesAndEditorsAreDifferent)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{widget, gadget});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true, new PropertyEditorSpecification("IDEditor")));

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Gadget");
    modifier->AddPropertyOverride(*new PropertySpecification("MyID", 1500, "", "", true, new PropertyEditorSpecification("GadgetEditor")));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // Widget_MyID, Gadget_MyID

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("GadgetEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[1]->GetEditor());
    EXPECT_STREQ("IDEditor", descriptor->GetVisibleFields()[1]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Polymorphism_AppliesFromBaseClassOnDerivedClass)
    {
    // set up the dataset
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceF, instanceE});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "", "", true, nullptr));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassE");
    modifier->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "", "", nullptr, new PropertyEditorSpecification("IntEditor")));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassE_ClassF_IntProperty

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("IntEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass)
    {
    // set up the dataset
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    ECEntityClassCP classH = GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceH, instanceF, instanceE});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "", "", true, nullptr));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassF");
    modifier->AddPropertyOverride(*new PropertySpecification("IntProperty", 1500, "", "", nullptr, new PropertyEditorSpecification("ClassFIntEditor")));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassE_IntProperty, ClassH_ClassF_IntProperty

    EXPECT_TRUE(nullptr == descriptor->GetVisibleFields()[0]->GetEditor());

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[1]->GetEditor());
    EXPECT_STREQ("ClassFIntEditor", descriptor->GetVisibleFields()[1]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty)
    {
    // set up the dataset
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsAspect")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), aspectClass->GetFullName(),
        {new PropertySpecification("UserLabel", 1, "", "", nullptr, new PropertyEditorSpecification("test editor"))}, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_Aspect
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsNestedContentField());
    ASSERT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetEditor());
    EXPECT_STREQ("test editor", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_FieldsMerging_GetOneFieldWhenPropertyLabelsAreEqual)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{widget, gadget});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "Custom Property Label", "", true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME((bvector<ECClassCP>{m_gadgetClass, m_widgetClass}), "MyID"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ("Custom Property Label", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_FieldsMerging_GetDifferentFieldsWhenPropertyLabelsAreDifferent)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{widget, gadget});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Gadget");
    modifier->AddPropertyOverride(*new PropertySpecification("MyID", 1000, "Custom Gadget Property Label", "", true));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // Widget_MyID, Gadget_MyID

    EXPECT_STREQ(FIELD_NAME(m_gadgetClass, "MyID"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ("Custom Gadget Property Label", descriptor->GetVisibleFields()[0]->GetLabel().c_str());

    EXPECT_STREQ(FIELD_NAME(m_widgetClass, "MyID"), descriptor->GetVisibleFields()[1]->GetName().c_str());
    EXPECT_STREQ("MyID", descriptor->GetVisibleFields()[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Polymorphism_FromBaseClassOnDerivedClass)
    {
    // set up the dataset
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceF, instanceE});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "", "", true));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassE");
    modifier->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "Custom ClassE Property Label", "", nullptr));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME(classE, "IntProperty"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ("Custom ClassE Property Label", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass)
    {
    // set up the dataset
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    ECEntityClassCP classH = GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceH, instanceF, instanceE});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "", "", true));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassF");
    modifier->AddPropertyOverride(*new PropertySpecification("IntProperty", 1000, "Custom ClassF Property Label", "", nullptr));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_STREQ(FIELD_NAME(classE, "IntProperty"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ("IntProperty", descriptor->GetVisibleFields()[0]->GetLabel().c_str());

    EXPECT_STREQ(FIELD_NAME(classE, "IntProperty"), descriptor->GetVisibleFields()[1]->GetName().c_str());
    EXPECT_STREQ("Custom ClassF Property Label", descriptor->GetVisibleFields()[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "Custom Label 1"));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "Custom Label 2"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassA_UserLabel
    EXPECT_STREQ("Custom Label 1", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Priorities_AppliesModifierOverSpecOverrideWhenPriorityIsHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Priorities_AppliesModifierOverSpecOverrideWhenPriorityIsHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "Custom Label 1"));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "Custom Label 2"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassA_UserLabel
    EXPECT_STREQ("Custom Label 2", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "Custom A Label 1") }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "Custom A Label 2"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB_UserLabel
    EXPECT_STREQ("UserLabel", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    EXPECT_STREQ("Custom A Label 1", descriptor->GetVisibleFields()[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityIsHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityIsHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "Custom A Label 1") }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "Custom A Label 2"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB_UserLabel
    EXPECT_STREQ("UserLabel", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    EXPECT_STREQ("Custom A Label 2", descriptor->GetVisibleFields()[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty)
    {
    // set up the dataset
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsAspect")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), aspectClass->GetFullName(),
        {new PropertySpecification("UserLabel", 1, "Custom Label")}, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_Aspect
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsNestedContentField());
    ASSERT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("Custom Label", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_FieldsMerging_GetOneFieldWhenPropertyCategoriesAreEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_FieldsMerging_GetOneFieldWhenPropertyCategoriesAreEqual)
    {
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    auto spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({ classA, classB }), false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("my_category", "My Category"));
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "my_category"));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "UserLabel"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ("my_category", descriptor->GetVisibleFields()[0]->GetCategory().GetName().c_str());
    EXPECT_STREQ("My Category", descriptor->GetVisibleFields()[0]->GetCategory().GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_FieldsMerging_GetDifferentFieldsWhenPropertyCategoriesAreDifferent, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_FieldsMerging_GetDifferentFieldsWhenPropertyCategoriesAreDifferent)
    {
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({ classA, classB }), false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier1 = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier1->AddPropertyCategory(*new PropertyCategorySpecification("my_category_1", "My Category 1"));
    modifier1->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "my_category_1"));
    rules->AddPresentationRule(*modifier1);

    ContentModifierP modifier2 = new ContentModifier(classB->GetSchema().GetName(), classB->GetName());
    modifier2->AddPropertyCategory(*new PropertyCategorySpecification("my_category_2", "My Category 2"));
    modifier2->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "my_category_2"));
    rules->AddPresentationRule(*modifier2);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_STREQ(FIELD_NAME(classA, "UserLabel"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ("my_category_1", descriptor->GetVisibleFields()[0]->GetCategory().GetName().c_str());
    EXPECT_STREQ("My Category 1", descriptor->GetVisibleFields()[0]->GetCategory().GetLabel().c_str());

    EXPECT_STREQ(FIELD_NAME(classB, "UserLabel"), descriptor->GetVisibleFields()[1]->GetName().c_str());
    EXPECT_STREQ("my_category_2", descriptor->GetVisibleFields()[1]->GetCategory().GetName().c_str());
    EXPECT_STREQ("My Category 2", descriptor->GetVisibleFields()[1]->GetCategory().GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Polymorphism_FromBaseClassOnDerivedClass, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="DerivedElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Polymorphism_FromBaseClassOnDerivedClass)
    {
    ECClassCP baseClass = GetClass("Element");
    ECClassCP derivedClass = GetClass("DerivedElement");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", derivedClass->GetFullName(), false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(baseClass->GetSchema().GetName(), baseClass->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("my_category", "My Category"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "my_category"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME(baseClass, "UserLabel"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ("my_category", descriptor->GetVisibleFields()[0]->GetCategory().GetName().c_str());
    EXPECT_STREQ("My Category", descriptor->GetVisibleFields()[0]->GetCategory().GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="DerivedElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass)
    {
    ECClassCP baseClass = GetClass("Element");
    ECClassCP derivedClass = GetClass("DerivedElement");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(derivedClass->GetSchema().GetName(), derivedClass->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("my_category", "My Category"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "my_category"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME(baseClass, "UserLabel"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STRNE("my_category", descriptor->GetVisibleFields()[0]->GetCategory().GetName().c_str());
    EXPECT_STREQ(FIELD_NAME(baseClass, "UserLabel"), descriptor->GetVisibleFields()[1]->GetName().c_str());
    EXPECT_STREQ("my_category", descriptor->GetVisibleFields()[1]->GetCategory().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "category1"));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "category2"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassA_UserLabel
    EXPECT_STREQ("category1", descriptor->GetVisibleFields()[0]->GetCategory().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Priorities_AppliesModifierOverSpecOverrideWhenPriorityIsHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Priorities_AppliesModifierOverSpecOverrideWhenPriorityIsHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "category1"));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", "category2"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassA_UserLabel
    EXPECT_STREQ("category2", descriptor->GetVisibleFields()[0]->GetCategory().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", "category1") }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", "category2"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB_UserLabel
    EXPECT_STREQ("category1", descriptor->GetVisibleFields()[1]->GetCategory().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityIsHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityIsHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", "", nullptr, nullptr) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", "category2"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB_UserLabel
    EXPECT_STREQ("category2", descriptor->GetVisibleFields()[1]->GetCategory().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty)
    {
    // set up the dataset
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsAspect")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("test_category", "Test Category"));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), aspectClass->GetFullName(),
        {new PropertySpecification("UserLabel", 1, "", "test_category")}, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_Aspect
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsNestedContentField());
    ASSERT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("test_category", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetCategory().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetNavigationPropertyValue)
    {
    // set up the dataset
    ECRelationshipClassCP widgetHasGadgets = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widgetInstance, *gadgetInstance);

    // set up input
    KeySetPtr input = KeySet::Create(*gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(3, descriptor->GetVisibleFields().size());

    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues[FIELD_NAME(m_gadgetClass, "Widget")].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*widgetInstance).GetId().GetValue(), values[FIELD_NAME(m_gadgetClass, "Widget")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#919256
* @bsitest                                      Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetNavigationPropertyValueWhenThereIsPropertyWithItsKeyFieldName, R"*(
    <ECEntityClass typeName="Element">
        <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
        <ECProperty propertyName="Model_Id" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetNavigationPropertyValueWhenThereIsPropertyWithItsKeyFieldName)
    {
    // set up the dataset
    ECRelationshipClassCP rel = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP elementClass = GetClass("Element");
    ECClassCP modelClass = GetClass("Model");
    IECInstancePtr modelInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr elementInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [&modelInstance, rel](IECInstanceR instance)
        {
        ECInstanceId modelId;
        ECInstanceId::FromString(modelId, modelInstance->GetInstanceId().c_str());
        instance.SetValue("Model", ECValue(modelId, rel));
        instance.SetValue("Model_Id", ECValue("Test"));
        });

    // set up input
    KeySetPtr input = KeySet::Create(*elementInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Model, Model_Id

    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*modelInstance).c_str(), displayValues[FIELD_NAME(elementClass, "Model")].GetString());
    EXPECT_STREQ("Test", displayValues[FIELD_NAME(elementClass, "Model_Id")].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*modelInstance).GetId().GetValue(), values[FIELD_NAME(elementClass, "Model")].GetInt64());
    EXPECT_STREQ("Test", values[FIELD_NAME(elementClass, "Model_Id")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetNavigationPropertyValue_InstanceLabelOverride)
    {
    // set up the dataset
    ECRelationshipClassCP widgetHasGadgets = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widgetInstance, *gadgetInstance);

    // set up input
    KeySetPtr input = KeySet::Create(*gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SelectedNodeInstance_GetNavigationPropertyValue_InstanceLabelOverride", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(3, descriptor->GetVisibleFields().size());

    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues[FIELD_NAME(m_gadgetClass, "Widget")].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*widgetInstance).GetId().GetValue(), values[FIELD_NAME(m_gadgetClass, "Widget")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetOneFieldForSimilarNavigationProperties)
    {
    // set up the dataset
    ECRelationshipClassCP classAHasBAndC = m_schema->GetClassCP("ClassAHasBAndC")->GetRelationshipClassCP();
    ECClassCP classA = m_schema->GetClassCP("ClassA");
    ECClassCP classB = m_schema->GetClassCP("ClassB");
    ECClassCP classC = m_schema->GetClassCP("ClassC");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasBAndC, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasBAndC, *instanceA, *instanceC);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceC});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), displayValues[FIELD_NAME((bvector<ECClassCP>{classB, classC}), "A")].GetString());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*instanceA).GetId().GetValue(), values[FIELD_NAME((bvector<ECClassCP>{classB, classC}), "A")].GetInt64());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR displayValues1 = recordJson1["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), displayValues1[FIELD_NAME((bvector<ECClassCP>{classB, classC}), "A")].GetString());
    RapidJsonValueCR values1 = recordJson1["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*instanceA).GetId().GetValue(), values1[FIELD_NAME((bvector<ECClassCP>{classB, classC}), "A")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetDifferentFieldsForDifferentNavigationProperties)
    {
    // set up the dataset
    ECRelationshipClassCP widgetHasGadgets = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP gadgetHasSprockets = m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widgetInstance, *gadgetInstance);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{gadgetInstance, sprocketInstance});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(4, descriptor->GetVisibleFields().size());

    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*widgetInstance).c_str(), displayValues[FIELD_NAME(m_gadgetClass, "Widget")].GetString());
    EXPECT_TRUE(displayValues[FIELD_NAME(m_sprocketClass, "Gadget")].IsNull());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*widgetInstance).GetId().GetValue(), values[FIELD_NAME(m_gadgetClass, "Widget")].GetInt64());
    EXPECT_TRUE(values[FIELD_NAME(m_sprocketClass, "Gadget")].IsNull());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR displayValues1 = recordJson1["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*gadgetInstance).c_str(), displayValues1[FIELD_NAME(m_sprocketClass, "Gadget")].GetString());
    EXPECT_TRUE(displayValues1[FIELD_NAME(m_gadgetClass, "Widget")].IsNull());
    RapidJsonValueCR values1 = recordJson1["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadgetInstance).GetId().GetValue(), values1[FIELD_NAME(m_sprocketClass, "Gadget")].GetInt64());
    EXPECT_TRUE(values1[FIELD_NAME(m_gadgetClass, "Widget")].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetCorrectValuesWhenNavigationPropertiesPointsToDifferentClassesButAreInTheSameField)
    {
    // set up the dataset
    ECClassCP classA = GetClass("RulesEngineTest", "ClassA");
    ECClassCP classB = GetClass("RulesEngineTest", "ClassB");
    ECClassCP classA2Base = GetClass("RulesEngineTest", "ClassA2Base");
    ECClassCP classB2 = GetClass("RulesEngineTest", "ClassB2");
    ECRelationshipClassCP classAHasBAndC = GetClass("RulesEngineTest", "ClassAHasBAndC")->GetRelationshipClassCP();
    ECRelationshipClassCP classA2HasB2 = GetClass("RulesEngineTest", "ClassA2BaseHasB2")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceA2Base = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA2Base);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasBAndC, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classA2HasB2, *instanceA2Base, *instanceB2);

    /*
    A ---ClassAHasBAndC---> B
    A2Base ---ClassA2BaseHasB2---> B2
    */

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceB2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), displayValues[FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")].GetString());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*instanceA).GetId().GetValue(), values[FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")].GetInt64());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR displayValues1 = recordJson1["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA2Base).c_str(), displayValues1[FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")].GetString());
    RapidJsonValueCR values1 = recordJson1["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*instanceA2Base).GetId().GetValue(), values1[FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetDerivedClassLabelWhenNavigationPropertyPointsToDerivedClass)
    {
    ECClassCP classA2 = GetClass("RulesEngineTest", "ClassA2");
    ECClassCP classB2 = GetClass("RulesEngineTest", "ClassB2");
    ECRelationshipClassCP classA2HasB2 = GetClass("RulesEngineTest", "ClassA2BaseHasB2")->GetRelationshipClassCP();
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classA2HasB2, *instanceA2, *instanceB2);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA2).c_str(), displayValues[FIELD_NAME(classB2, "A")].GetString());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*instanceA2).GetId().GetValue(), values[FIELD_NAME(classB2, "A")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetCorrectNavigationPropertiesValuesWhenRelatedPropertiesSpecificationIsApplied)
    {
    ECRelationshipClassCP widgetHasGadgets = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget);

    // set up input
    KeySetPtr input = KeySet::Create(*gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets",
        "RulesEngineTest:Widget", "", RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues[FIELD_NAME(m_gadgetClass, "Widget")].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*widget).GetId().GetValue(), values[FIELD_NAME(m_gadgetClass, "Widget")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetCorrectNavigationPropertiesValuesWhenRelatedPropertiesSpecificationIsApplied_InstanceLabelOverride)
    {
    ECRelationshipClassCP widgetHasGadgets = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget);

    // set up input
    KeySetPtr input = KeySet::Create(*gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SelectedNodeInstance_GetCorrectNavigationPropertiesValuesWhenRelatedPropertiesSpecificationIsApplied_InstanceLabelOverride", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets",
        "RulesEngineTest:Widget", "", RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues[FIELD_NAME(m_gadgetClass, "Widget")].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*widget).GetId().GetValue(), values[FIELD_NAME(m_gadgetClass, "Widget")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_GetRelatedInstanceNavigationPropertyValue)
    {
    ECRelationshipClassCP widgetHasGadgets = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget);

    // set up input
    KeySetPtr input = KeySet::Create(*widget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecificationP spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "RulesEngineTest:WidgetHasGadgets", "");
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(3, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues[FIELD_NAME(m_gadgetClass, "Widget")].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*widget).GetId().GetValue(), values[FIELD_NAME(m_gadgetClass, "Widget")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_GetRelatedInstanceNavigationPropertyValue_InstanceLabelOverride)
    {
    ECRelationshipClassCP widgetHasGadgets = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget);

    // set up input
    KeySetPtr input = KeySet::Create(*widget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ContentRelatedInstances_GetRelatedInstanceNavigationPropertyValue_InstanceLabelOverride", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecificationP spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "RulesEngineTest:WidgetHasGadgets", "");
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(3, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues[FIELD_NAME(m_gadgetClass, "Widget")].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*widget).GetId().GetValue(), values[FIELD_NAME(m_gadgetClass, "Widget")].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDerivedClassNavigationPropertyWhenSelectingFromBaseClassAndDerivedClass)
    {
    ECRelationshipClassCP rel = GetClass("RulesEngineTest", "ClassGUsesClassD")->GetRelationshipClassCP();
    ECClassCP classE = GetClass("RulesEngineTest", "ClassE");
    ECClassCP classD = GetClass("RulesEngineTest", "ClassD");
    ECClassCP classG = GetClass("RulesEngineTest", "ClassG");
    IECInstancePtr relatedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr baseInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classG);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *relatedInstance, *derivedInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "",
        "RulesEngineTest:ClassE,ClassG", false));
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    EXPECT_TRUE(recordJson1["Values"][FIELD_NAME(classG, "D")].IsNull());
    EXPECT_TRUE(recordJson1["DisplayValues"][FIELD_NAME(classG, "D")].IsNull());

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*relatedInstance).GetId().GetValue(), recordJson2["Values"][FIELD_NAME(classG, "D")].GetUint64());
    EXPECT_STREQ(GetDefaultDisplayLabel(*relatedInstance).c_str(), recordJson2["DisplayValues"][FIELD_NAME(classG, "D")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPrimitiveArrayPropertyValue, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPrimitiveArrayPropertyValue)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(3), 0);
        instance.SetValue("ArrayProperty", ECValue(4), 1);
        instance.SetValue("ArrayProperty", ECValue(5), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Array",
        "TypeName": "int[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "int"
            }
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME(ecClass, "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": [3, 4, 5]
        })", FIELD_NAME(ecClass, "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPointsArrayPropertyValue, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="PointsArrayProperty" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPointsArrayPropertyValue)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("PointsArrayProperty", 2);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(0, 0, 0)), 0);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(1, 1, 1)), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("PointsArrayProperty", 3);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(3, 3, 3)), 0);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(4, 4, 4)), 1);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(5, 5, 5)), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Array",
        "TypeName": "point3d[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "point3d"
            }
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": [
            {"x": 0, "y": 0, "z": 0},
            {"x": 1, "y": 1, "z": 1}
        ]})", FIELD_NAME(ecClass, "PointsArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    rapidjson::Document expectedDisplayValues1;
    expectedDisplayValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": ["X: 0.00 Y: 0.00 Z: 0.00", "X: 1.00 Y: 1.00 Z: 1.00"]
        })", FIELD_NAME(ecClass, "PointsArrayProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues1, recordJson1["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["DisplayValues"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": [
            {"x": 3, "y": 3, "z": 3},
            {"x": 4, "y": 4, "z": 4},
            {"x": 5, "y": 5, "z": 5}
        ]})", FIELD_NAME(ecClass, "PointsArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    rapidjson::Document expectedDisplayValues2;
    expectedDisplayValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": ["X: 3.00 Y: 3.00 Z: 3.00", "X: 4.00 Y: 4.00 Z: 4.00", "X: 5.00 Y: 5.00 Z: 5.00"]
        })", FIELD_NAME(ecClass, "PointsArrayProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues2, recordJson2["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesDescriptorsWithSimilarNavigationProperties, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="ChildElement" relationshipName="ElementContainsElements" direction="Backward"/>
    </ECEntityClass>
    <ECEntityClass typeName="DerivedElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesDescriptorsWithSimilarNavigationProperties)
    {
    // set up data set
    ECClassCP element = GetClass("Element");
    ECClassCP derivedElement = GetClass("DerivedElement");

    IECInstancePtr elementInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *element);
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedElement);

    // set up input
    KeySetPtr elementInput = KeySet::Create(*elementInstance);
    KeySetPtr derivedInput = KeySet::Create(*derivedInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptors
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *elementInput, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *derivedInput, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsValid());

    // merge descriptors
    ContentDescriptorPtr mergedDescriptor = ContentDescriptor::Create(*descriptor);
    mergedDescriptor->MergeWith(*descriptor2);
    ASSERT_TRUE(mergedDescriptor.IsValid());
    EXPECT_EQ(mergedDescriptor->GetAllFields().size(), descriptor->GetAllFields().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntMergePropertiesOfSameNameAndTypeWhenValueKindDoesntMatch, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntMergePropertiesOfSameNameAndTypeWhenValueKindDoesntMatch)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Prop", ECValue(1));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.AddArrayElements("Prop", 2);
        instance.SetValue("Prop", ECValue(2), 0);
        instance.SetValue("Prop", ECValue(3), 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": 1,
        "%s": null
        })", FIELD_NAME(classA, "Prop"), FIELD_NAME(classB, "Prop")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": null,
        "%s": [2, 3]
        })", FIELD_NAME(classA, "Prop"), FIELD_NAME(classB, "Prop")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyFieldsOfDifferentClasses, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyFieldsOfDifferentClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(3), 0);
        instance.SetValue("ArrayProperty", ECValue(4), 1);
        instance.SetValue("ArrayProperty", ECValue(5), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": [3, 4, 5]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        instance.SetValue("ArrayProperty", ECValue(3), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenSomeValuesAreNull, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenSomeValuesAreNull)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 1);
        instance.SetValue("ArrayProperty", ECValue(1), 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyValueWhenValuesEqual, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyValueWhenValuesEqual)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME(ecClass, "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged(FIELD_NAME(ecClass, "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyValueWhenArraySizesAreDifferent, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyValueWhenArraySizesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        instance.SetValue("ArrayProperty", ECValue(3), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyValueWhenValuesInArrayAreDifferent, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyValueWhenValuesInArrayAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(1), 0);
        instance.SetValue("ArrayProperty", ECValue(2), 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyValueWhenClassesAreDifferent, R"*(
    <ECEntityClass typeName="ClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyValueWhenClassesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME(classA, "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME(classA, "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(classA, "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsEnumsArrayPropertyValue, R"*(
    <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="True" displayLabel="My Enum">
        <ECEnumerator value="1" displayLabel="One" />
        <ECEnumerator value="2" displayLabel="Two" />
        <ECEnumerator value="3" displayLabel="Three" />
    </ECEnumeration>
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="EnumsArray" typeName="MyEnum" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsEnumsArrayPropertyValue)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("EnumsArray", 2);
        instance.SetValue("EnumsArray", ECValue(2), 0);
        instance.SetValue("EnumsArray", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("EnumsArray", 3);
        instance.SetValue("EnumsArray", ECValue(1), 0);
        instance.SetValue("EnumsArray", ECValue(2), 1);
        instance.SetValue("EnumsArray", ECValue(3), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Array",
        "TypeName": "MyEnum[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "MyEnum"
            }
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME(ecClass, "EnumsArray")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    rapidjson::Document expectedDisplayValues1;
    expectedDisplayValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": ["Two", "One"]
        })", FIELD_NAME(ecClass, "EnumsArray")).c_str());
    EXPECT_EQ(expectedDisplayValues1, recordJson1["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["DisplayValues"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": [1, 2, 3]
        })", FIELD_NAME(ecClass, "EnumsArray")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    rapidjson::Document expectedDisplayValues2;
    expectedDisplayValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": ["One", "Two", "Three"]
        })", FIELD_NAME(ecClass, "EnumsArray")).c_str());
    EXPECT_EQ(expectedDisplayValues2, recordJson2["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsStructPropertyValue, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsStructPropertyValue)
    {
    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Struct",
        "TypeName": "MyStruct",
        "Members": [{
            "Name": "IntProperty",
            "Label": "IntProperty",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "int"
                }
            },{
            "Name": "StringProperty",
            "Label": "StringProperty",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "string"
                }
            }]
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 456,
           "StringProperty": "def"
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyFieldsOfDifferentClasses, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyFieldsOfDifferentClasses)
    {
    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 456,
           "StringProperty": "def"
           }
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")).c_str());
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyValuesWhenValuesAreEqual, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyValuesWhenValuesAreEqual)
    {
    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged(FIELD_NAME(ecClass, "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyValuesWhenValuesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyValuesWhenValuesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "StructProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "StructProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyValueWhenClassesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="ClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyValueWhenClassesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    EXPECT_TRUE(record->GetValues()[FIELD_NAME(classA, "StructProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME(classA, "StructProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(classA, "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsStructArrayPropertyValue, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsStructArrayPropertyValue)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 2);

        IECInstancePtr structInstance1 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance1->SetValue("IntProperty", ECValue(456));
        structInstance1->SetValue("StringProperty", ECValue("def"));
        ECValue structValue1;
        structValue1.SetStruct(structInstance1.get());
        instance.SetValue("StructArrayProperty", structValue1, 0);

        IECInstancePtr structInstance2 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance2->SetValue("IntProperty", ECValue(789));
        structInstance2->SetValue("StringProperty", ECValue("ghi"));
        ECValue structValue2;
        structValue2.SetStruct(structInstance2.get());
        instance.SetValue("StructArrayProperty", structValue2, 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Array",
        "TypeName": "MyStruct[]",
        "MemberType": {
            "ValueFormat": "Struct",
            "TypeName": "MyStruct",
            "Members": [{
                "Name": "IntProperty",
                "Label": "IntProperty",
                "Type": {
                    "ValueFormat": "Primitive",
                    "TypeName": "int"
                    }
                },{
                "Name": "StringProperty",
                "Label": "StringProperty",
                "Type": {
                    "ValueFormat": "Primitive",
                    "TypeName": "string"
                    }
                }]
            }
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })", FIELD_NAME(ecClass, "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 456,
           "StringProperty": "def"
           },{
           "IntProperty": 789,
           "StringProperty": "ghi"
           }]
        })", FIELD_NAME(ecClass, "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyFieldsOfDifferentClasses, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyFieldsOfDifferentClasses)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 2);

        IECInstancePtr structInstance1 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance1->SetValue("IntProperty", ECValue(456));
        structInstance1->SetValue("StringProperty", ECValue("def"));
        ECValue structValue1;
        structValue1.SetStruct(structInstance1.get());
        instance.SetValue("StructArrayProperty", structValue1, 0);

        IECInstancePtr structInstance2 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance2->SetValue("IntProperty", ECValue(789));
        structInstance2->SetValue("StringProperty", ECValue("ghi"));
        ECValue structValue2;
        structValue2.SetStruct(structInstance2.get());
        instance.SetValue("StructArrayProperty", structValue2, 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 456,
           "StringProperty": "def"
           },{
           "IntProperty": 789,
           "StringProperty": "ghi"
           }]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(456));
        structInstance->SetValue("StringProperty", ECValue("def"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyValuesWhenValuesAreEqual, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyValuesWhenValuesAreEqual)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })", FIELD_NAME(ecClass, "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged(FIELD_NAME(ecClass, "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyValuesWhenArraySizesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyValuesWhenArraySizesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 2);

        IECInstancePtr structInstance1 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance1->SetValue("IntProperty", ECValue(123));
        structInstance1->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue1;
        structValue1.SetStruct(structInstance1.get());
        instance.SetValue("StructArrayProperty", structValue1, 0);

        IECInstancePtr structInstance2 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance2->SetValue("IntProperty", ECValue(456));
        structInstance2->SetValue("StringProperty", ECValue("def"));
        ECValue structValue2;
        structValue2.SetStruct(structInstance2.get());
        instance.SetValue("StructArrayProperty", structValue2, 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "StructArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "StructArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyValuesWhenArrayValuesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyValuesWhenArrayValuesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(456));
        structInstance->SetValue("StringProperty", ECValue("def"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "StructArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "StructArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyValueWhenClassesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="ClassA">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyValueWhenClassesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", GetClassNamesList({classA, classB}), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    EXPECT_TRUE(record->GetValues()[FIELD_NAME(classA, "StructArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME(classA, "StructArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(classA, "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsStructWithArrayPropertyValue, R"*(
    <ECStructClass typeName="MyStruct">
        <ECArrayProperty propertyName="IntProperty" typeName="int" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsStructWithArrayPropertyValue)
    {
    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("StructProperty.IntProperty", 2);
        instance.SetValue("StructProperty.IntProperty", ECValue(1), 0);
        instance.SetValue("StructProperty.IntProperty", ECValue(2), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("StructProperty.IntProperty", 3);
        instance.SetValue("StructProperty.IntProperty", ECValue(4), 0);
        instance.SetValue("StructProperty.IntProperty", ECValue(5), 1);
        instance.SetValue("StructProperty.IntProperty", ECValue(6), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Struct",
        "TypeName": "MyStruct",
        "Members": [{
            "Name": "IntProperty",
            "Label": "IntProperty",
            "Type": {
                "ValueFormat": "Array",
                "TypeName": "int[]",
                "MemberType": {
                    "ValueFormat": "Primitive",
                    "TypeName": "int"
                    }
                }
            }]
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": [1, 2]
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": [4, 5, 6]
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsStructWithNullArrayAndStructPropertyValues, R"*(
    <ECStructClass typeName="MyStructA">
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECStructClass typeName="MyStructB">
        <ECArrayProperty propertyName="IntProperties" typeName="int" />
        <ECStructProperty propertyName="StructProperty" typeName="MyStructA" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStructB" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsStructWithNullArrayAndStructPropertyValues)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.StringProperty", ECValue("test"));
        });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": {
           "StringProperty": "test",
           "StructProperty": null,
           "IntProperties": null
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedInstancesAsArrays, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="ChildClass">
        <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren" direction="Backward" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="ParentClass">
        <ECProperty propertyName="ParentProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ParentHasChildren" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="ClassD Has ClassE" polymorphic="False">
            <Class class="ParentClass" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ClassE Has ClassD" polymorphic="False">
            <Class class="ChildClass" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedInstancesAsArrays)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP parentClass = GetClass("ParentClass");
    ECClassCP childClass = GetClass("ChildClass");
    ECRelationshipClassCP rel = GetRelationshipClass("ParentHasChildren");
    IECInstancePtr parent = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *parentClass);
    IECInstancePtr child1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *childClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Parent", ECValue(RulesEngineTestHelpers::GetInstanceKey(*parent).GetId(), rel));
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("StringProperty", ECValue("111"));

        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);

        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));

        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    IECInstancePtr child2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *childClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Parent", ECValue(RulesEngineTestHelpers::GetInstanceKey(*parent).GetId(), rel));
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("StringProperty", ECValue("222"));

        instance.AddArrayElements("ArrayProperty", 1);
        instance.SetValue("ArrayProperty", ECValue(3), 0);

        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));

        instance.AddArrayElements("StructArrayProperty", 2);

        IECInstancePtr structInstance1 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance1->SetValue("IntProperty", ECValue(456));
        structInstance1->SetValue("StringProperty", ECValue("def"));
        ECValue structValue1;
        structValue1.SetStruct(structInstance1.get());
        instance.SetValue("StructArrayProperty", structValue1, 0);

        IECInstancePtr structInstance2 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance2->SetValue("IntProperty", ECValue(789));
        structInstance2->SetValue("StringProperty", ECValue("ghi"));
        ECValue structValue2;
        structValue2.SetStruct(structInstance2.get());
        instance.SetValue("StructArrayProperty", structValue2, 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", parentClass->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(),
        childClass->GetFullName(), "", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ParentClass_ParentProperty, Nested<5 ChildClass properties>

    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Struct",
        "TypeName": "ChildClass",
        "Members": [{
            "Name": "%s",
            "Label": "IntProperty",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "int"
                }
            },{
            "Name": "%s",
            "Label": "StringProperty",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "string"
                }
            },{
            "Name": "%s",
            "Label": "ArrayProperty",
            "Type": {
                "ValueFormat": "Array",
                "TypeName": "int[]",
                "MemberType": {
                    "ValueFormat": "Primitive",
                    "TypeName": "int"
                    }
                }
            },{
            "Name": "%s",
            "Label": "StructProperty",
            "Type": {
                "ValueFormat": "Struct",
                "TypeName": "MyStruct",
                "Members": [{
                    "Name": "IntProperty",
                    "Label": "IntProperty",
                    "Type": {
                        "ValueFormat": "Primitive",
                        "TypeName": "int"
                        }
                    },{
                    "Name": "StringProperty",
                    "Label": "StringProperty",
                    "Type": {
                        "ValueFormat": "Primitive",
                        "TypeName": "string"
                        }
                    }]
                }
            },{
            "Name": "%s",
            "Label": "StructArrayProperty",
            "Type": {
                "ValueFormat": "Array",
                "TypeName": "MyStruct[]",
                "MemberType": {
                    "ValueFormat": "Struct",
                    "TypeName": "MyStruct",
                    "Members": [{
                        "Name": "IntProperty",
                        "Label": "IntProperty",
                        "Type": {
                            "ValueFormat": "Primitive",
                            "TypeName": "int"
                            }
                        },{
                        "Name": "StringProperty",
                        "Label": "StringProperty",
                        "Type": {
                            "ValueFormat": "Primitive",
                            "TypeName": "string"
                            }
                        }]
                    }
                }
            }]
        })",
        FIELD_NAME(childClass, "IntProperty"), FIELD_NAME(childClass, "StringProperty"),
        FIELD_NAME(childClass, "ArrayProperty"), FIELD_NAME(childClass, "StructProperty"),
        FIELD_NAME(childClass, "StructArrayProperty")).c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[1]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 1,
                "%s": "111",
                "%s": [2, 1],
                "%s": {
                    "IntProperty": 123,
                    "StringProperty": "abc"
                    },
                "%s": [{
                   "IntProperty": 123,
                   "StringProperty": "abc"
                   }]
                },
            "DisplayValues": {
                "%s": "1",
                "%s": "111",
                "%s": ["2", "1"],
                "%s": {
                    "IntProperty": "123",
                    "StringProperty": "abc"
                    },
                "%s": [{
                   "IntProperty": "123",
                   "StringProperty": "abc"
                   }]
                },
            "MergedFieldNames": []
            },{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 2,
                "%s": "222",
                "%s": [3],
                "%s": {
                    "IntProperty": 456,
                    "StringProperty": "def"
                    },
                "%s": [{
                   "IntProperty": 456,
                   "StringProperty": "def"
                   },{
                   "IntProperty": 789,
                   "StringProperty": "ghi"
                   }]
                },
            "DisplayValues": {
                "%s": "2",
                "%s": "222",
                "%s": ["3"],
                "%s": {
                    "IntProperty": "456",
                    "StringProperty": "def"
                    },
                "%s": [{
                   "IntProperty": "456",
                   "StringProperty": "def"
                   },{
                   "IntProperty": "789",
                   "StringProperty": "ghi"
                   }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(parentClass, "ParentProperty"), NESTED_CONTENT_FIELD_NAME(parentClass, childClass),
        childClass->GetId().ToString().c_str(), child1->GetInstanceId().c_str(),
        FIELD_NAME(childClass, "IntProperty"), FIELD_NAME(childClass, "StringProperty"), FIELD_NAME(childClass, "ArrayProperty"),
        FIELD_NAME(childClass, "StructProperty"), FIELD_NAME(childClass, "StructArrayProperty"),
        FIELD_NAME(childClass, "IntProperty"), FIELD_NAME(childClass, "StringProperty"), FIELD_NAME(childClass, "ArrayProperty"),
        FIELD_NAME(childClass, "StructProperty"), FIELD_NAME(childClass, "StructArrayProperty"),
        childClass->GetId().ToString().c_str(), child2->GetInstanceId().c_str(),
        FIELD_NAME(childClass, "IntProperty"), FIELD_NAME(childClass, "StringProperty"), FIELD_NAME(childClass, "ArrayProperty"),
        FIELD_NAME(childClass, "StructProperty"), FIELD_NAME(childClass, "StructArrayProperty"),
        FIELD_NAME(childClass, "IntProperty"), FIELD_NAME(childClass, "StringProperty"), FIELD_NAME(childClass, "ArrayProperty"),
        FIELD_NAME(childClass, "StructProperty"), FIELD_NAME(childClass, "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenEqual)
    {
    // set up data set
    ECEntityClassCP classD = GetClass("RulesEngineTest", "ClassD")->GetEntityClassCP();
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECRelationshipClassCP rel = GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();
    IECInstancePtr instanceD1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceE11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetId(), rel));
        });
    IECInstancePtr instanceE12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("LongProperty", ECValue((int64_t)222));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetId(), rel));
        });
    IECInstancePtr instanceE13 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("LongProperty", ECValue((int64_t)333));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetId(), rel));
        });
    IECInstancePtr instanceD2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceE21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });
    IECInstancePtr instanceE22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("LongProperty", ECValue((int64_t)222));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });
    IECInstancePtr instanceE23 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("LongProperty", ECValue((int64_t)333));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });
    IECInstancePtr instanceD3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceE31 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetId(), rel));
        });
    IECInstancePtr instanceE32 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("LongProperty", ECValue((int64_t)222));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetId(), rel));
        });
    IECInstancePtr instanceE33 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("LongProperty", ECValue((int64_t)333));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetId(), rel));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassD", false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:ClassDHasClassE",
        "RulesEngineTest:ClassE", "", RelationshipMeaning::RelatedInstance));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassD_StringProperty, Array<ClassE_IntProperty + ClassE_LongProperty>

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}, {"ECClassId":"%s", "ECInstanceId":"%s"}, {"ECClassId":"%s", "ECInstanceId":"%s"}],
            "Values": {
                "%s": 1,
                "%s": 111
                },
            "DisplayValues": {
                "%s": "1",
                "%s": "111"
                },
            "MergedFieldNames": []
            },{
            "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}, {"ECClassId":"%s", "ECInstanceId":"%s"}, {"ECClassId":"%s", "ECInstanceId":"%s"}],
            "Values": {
                "%s": 2,
                "%s": 222
                },
            "DisplayValues": {
                "%s": "2",
                "%s": "222"
                },
            "MergedFieldNames": []
            },{
            "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}, {"ECClassId":"%s", "ECInstanceId":"%s"}, {"ECClassId":"%s", "ECInstanceId":"%s"}],
            "Values": {
                "%s": 3,
                "%s": 333
                },
            "DisplayValues": {
                "%s": "3",
                "%s": "333"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(classD, "StringProperty"), NESTED_CONTENT_FIELD_NAME(classD, classE),
        classE->GetId().ToString().c_str(), instanceE11->GetInstanceId().c_str(),
        classE->GetId().ToString().c_str(), instanceE21->GetInstanceId().c_str(),
        classE->GetId().ToString().c_str(), instanceE31->GetInstanceId().c_str(),
        FIELD_NAME(classE, "IntProperty"), FIELD_NAME(classE, "LongProperty"), FIELD_NAME(classE, "IntProperty"), FIELD_NAME(classE, "LongProperty"),
        classE->GetId().ToString().c_str(), instanceE12->GetInstanceId().c_str(),
        classE->GetId().ToString().c_str(), instanceE22->GetInstanceId().c_str(),
        classE->GetId().ToString().c_str(), instanceE32->GetInstanceId().c_str(),
        FIELD_NAME(classE, "IntProperty"), FIELD_NAME(classE, "LongProperty"), FIELD_NAME(classE, "IntProperty"), FIELD_NAME(classE, "LongProperty"),
        classE->GetId().ToString().c_str(), instanceE13->GetInstanceId().c_str(),
        classE->GetId().ToString().c_str(), instanceE23->GetInstanceId().c_str(),
        classE->GetId().ToString().c_str(), instanceE33->GetInstanceId().c_str(),
        FIELD_NAME(classE, "IntProperty"), FIELD_NAME(classE, "LongProperty"), FIELD_NAME(classE, "IntProperty"), FIELD_NAME(classE, "LongProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(contentSet.Get(0)->IsMerged("ClassD_ClassE"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenSizesNotEqual)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECEntityClassCP classD = GetClass("RulesEngineTest", "ClassD")->GetEntityClassCP();
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECRelationshipClassCP rel = GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();
    IECInstancePtr instanceD1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetId(), rel));
        });
    IECInstancePtr instanceD2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassD", false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:ClassDHasClassE",
        "RulesEngineTest:ClassE", "", RelationshipMeaning::RelatedInstance));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassD_StringProperty, Array<ClassE_IntProperty + ClassE_LongProperty>

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": "%s"
        })",
        FIELD_NAME(classD, "StringProperty"), NESTED_CONTENT_FIELD_NAME(classD, classE),
        varies_string.c_str()).c_str());

    EXPECT_EQ(expectedValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    EXPECT_TRUE(contentSet.Get(0)->IsMerged(NESTED_CONTENT_FIELD_NAME(classD, classE)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenValuesNotEqual)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECEntityClassCP classD = GetClass("RulesEngineTest", "ClassD")->GetEntityClassCP();
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECRelationshipClassCP rel = GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();
    IECInstancePtr instanceD1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("LongProperty", ECValue((int64_t)222));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("LongProperty", ECValue((int64_t)333));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetId(), rel));
        });
    IECInstancePtr instanceD2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(4));
        instance.SetValue("LongProperty", ECValue((int64_t)444));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(5));
        instance.SetValue("LongProperty", ECValue((int64_t)555));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(6));
        instance.SetValue("LongProperty", ECValue((int64_t)666));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetId(), rel));
        });
    IECInstancePtr instanceD3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(7));
        instance.SetValue("LongProperty", ECValue((int64_t)777));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(8));
        instance.SetValue("LongProperty", ECValue((int64_t)888));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(9));
        instance.SetValue("LongProperty", ECValue((int64_t)999));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetId(), rel));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassD", false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:ClassDHasClassE",
        "RulesEngineTest:ClassE", "", RelationshipMeaning::RelatedInstance));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassD_StringProperty, Array<ClassE_IntProperty + ClassE_LongProperty>

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": "%s"
        })",
        FIELD_NAME(classD, "StringProperty"), NESTED_CONTENT_FIELD_NAME(classD, classE),
        varies_string.c_str()).c_str());

    EXPECT_EQ(expectedValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    EXPECT_TRUE(contentSet.Get(0)->IsMerged(NESTED_CONTENT_FIELD_NAME(classD, classE)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedNestedInstancesAsArraysWhenShowingIntermediateProperties)
    {
    // set up data set
    ECRelationshipClassCP rel_WG = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_GS = GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Widget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*widget).GetId(), rel_WG));
        });
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetId(), rel_GS));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel_WG->GetFullName(), m_gadgetClass->GetFullName(), "", RelationshipMeaning::RelatedInstance));
    spec->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel_GS->GetFullName(), m_sprocketClass->GetFullName(), "", RelationshipMeaning::RelatedInstance));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); // 7 Widget properties, Gadget

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": null,
                "%s": null,
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": null,
                        "%s": null
                        },
                    "DisplayValues": {
                        "%s": null,
                        "%s": null
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": null,
                "%s": null,
                "%s": [{
                    "DisplayValues": {
                        "%s": null,
                        "%s": null
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(m_widgetClass, "Description"), FIELD_NAME(m_widgetClass, "MyID"), FIELD_NAME(m_widgetClass, "IntProperty"),
        FIELD_NAME(m_widgetClass, "BoolProperty"), FIELD_NAME(m_widgetClass, "DoubleProperty"), FIELD_NAME(m_widgetClass, "LongProperty"),
        FIELD_NAME(m_widgetClass, "DateProperty"), NESTED_CONTENT_FIELD_NAME(m_widgetClass, m_gadgetClass),
        m_gadgetClass->GetId().ToString().c_str(), gadget->GetInstanceId().c_str(),
        FIELD_NAME(m_gadgetClass, "MyID"), FIELD_NAME(m_gadgetClass, "Description"), NESTED_CONTENT_FIELD_NAME(m_gadgetClass, m_sprocketClass),
        m_sprocketClass->GetId().ToString().c_str(), sprocket->GetInstanceId().c_str(),
        FIELD_NAME(m_sprocketClass, "Description"), FIELD_NAME(m_sprocketClass, "MyID"), FIELD_NAME(m_sprocketClass, "Description"), FIELD_NAME(m_sprocketClass, "MyID"),
        FIELD_NAME(m_gadgetClass, "MyID"), FIELD_NAME(m_gadgetClass, "Description"), NESTED_CONTENT_FIELD_NAME(m_gadgetClass, m_sprocketClass),
        FIELD_NAME(m_sprocketClass, "Description"), FIELD_NAME(m_sprocketClass, "MyID")).c_str());

    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedNestedInstancesAsArraysWhenSkippingIntermediateProperties)
    {
    // set up data set
    ECRelationshipClassCP rel_WG = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_GS = GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Widget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*widget).GetId(), rel_WG));
        });
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetId(), rel_GS));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel_WG->GetFullName(), m_gadgetClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    spec->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel_GS->GetFullName(), m_sprocketClass->GetFullName(), "", RelationshipMeaning::RelatedInstance));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); // 7 Widget properties, Sprocket

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": null,
                "%s": null
                },
            "DisplayValues": {
                "%s": null,
                "%s": null
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(m_widgetClass, "Description"), FIELD_NAME(m_widgetClass, "MyID"), FIELD_NAME(m_widgetClass, "IntProperty"),
        FIELD_NAME(m_widgetClass, "BoolProperty"), FIELD_NAME(m_widgetClass, "DoubleProperty"), FIELD_NAME(m_widgetClass, "LongProperty"),
        FIELD_NAME(m_widgetClass, "DateProperty"), NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{m_widgetClass, m_gadgetClass}), m_sprocketClass),
        m_sprocketClass->GetId().ToString().c_str(), sprocket->GetInstanceId().c_str(),
        FIELD_NAME(m_sprocketClass, "Description"), FIELD_NAME(m_sprocketClass, "MyID"),
        FIELD_NAME(m_sprocketClass, "Description"), FIELD_NAME(m_sprocketClass, "MyID")).c_str());

    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, RequestingDescriptorWithClassIdsAllowsUsingSelectedNodeECExpressionSymbol)
    {
    // set up input
    ECClassCP classF = GetClass("RulesEngineTest", "ClassF");
    KeySetPtr input = KeySet::Create({ECClassInstanceKey(classF, ECInstanceId())});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("SelectedNode.ECInstance.IsOfClass(\"ClassE\", \"RulesEngineTest\") ANDALSO SelectedNode.ClassName = \"ClassF\"", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, RequestingDescriptorWithClassIdsAndUsingSelectedNodeECInstanceSymbolFailsGracefully)
    {
    // set up input
    NavNodeKeyList keys;
    KeySetPtr input = KeySet::Create({ECClassInstanceKey(m_widgetClass, ECInstanceId())});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("SelectedNode.ECInstance.IntProperty = 123", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, RelatedPropertiesSpecification_GetCorrectFieldDisplayLabelWhenRelationshipMeaningIsSetToSameInstance)
    {
    // set up input
    ECRelationshipClassCP widgetHasGadget = GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadget, *widget, *gadget);

    KeySetPtr input = KeySet::Create(*gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification();
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget",
        "IntProperty", RelationshipMeaning::SameInstance));
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(4, descriptor->GetVisibleFields().size());

    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsPropertiesField());
    EXPECT_STREQ("MyID", descriptor->GetVisibleFields()[0]->AsPropertiesField()->GetLabel().c_str());

    ASSERT_TRUE(descriptor->GetVisibleFields()[1]->IsPropertiesField());
    EXPECT_STREQ("Description", descriptor->GetVisibleFields()[1]->AsPropertiesField()->GetLabel().c_str());

    ASSERT_TRUE(descriptor->GetVisibleFields()[2]->IsPropertiesField());
    EXPECT_STREQ("Widget", descriptor->GetVisibleFields()[2]->AsPropertiesField()->GetLabel().c_str());

    ASSERT_TRUE(descriptor->GetVisibleFields()[3]->IsPropertiesField());
    EXPECT_STREQ("IntProperty", descriptor->GetVisibleFields()[3]->AsPropertiesField()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, RelatedPropertiesSpecification_GetCorrectFieldDisplayLabelWhenRelationshipMeaningIsSetToRelatedInstance)
    {
    // set up input
    ECRelationshipClassCP widgetHasGadget = GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadget, *widget, *gadget);

    KeySetPtr input = KeySet::Create(*gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification();
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget",
        "IntProperty", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_EQ(4, descriptor->GetVisibleFields().size());

    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsPropertiesField());
    EXPECT_STREQ("MyID", descriptor->GetVisibleFields()[0]->AsPropertiesField()->GetLabel().c_str());

    ASSERT_TRUE(descriptor->GetVisibleFields()[1]->IsPropertiesField());
    EXPECT_STREQ("Description", descriptor->GetVisibleFields()[1]->AsPropertiesField()->GetLabel().c_str());

    ASSERT_TRUE(descriptor->GetVisibleFields()[2]->IsPropertiesField());
    EXPECT_STREQ("Widget", descriptor->GetVisibleFields()[2]->AsPropertiesField()->GetLabel().c_str());

    ASSERT_TRUE(descriptor->GetVisibleFields()[3]->IsPropertiesField());
    EXPECT_STREQ("IntProperty", descriptor->GetVisibleFields()[3]->AsPropertiesField()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDifferentFieldsIfPropertiesHaveDifferentKindOfQuantities)
    {
    // set up input
    ECClassCP classK = GetClass("RulesEngineTest", "ClassK");
    ECClassCP classL = GetClass("RulesEngineTest", "ClassL");
    IECInstancePtr instanceK = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classK);
    IECInstancePtr instanceL = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classL);

    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceK, instanceL});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification();
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_STREQ(FIELD_NAME(classK, "LengthProperty"), descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ(FIELD_NAME(classL, "LengthProperty"), descriptor->GetVisibleFields()[1]->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctValues)
    {
    // set up the dataset
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    bvector<ContentDescriptor::Field*> fieldVectorCopy = descriptor->GetAllFields();
    // hide all fields except Widget_MyID
    for (ContentDescriptor::Field const* field : fieldVectorCopy)
        {
        if (!field->GetName().EndsWithI("MyID"))
            overridenDescriptor->RemoveField(field->GetName().c_str());
        }
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // validate descriptor
    EXPECT_EQ(1, overridenDescriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = m_manager->GetContent(*overridenDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR values1 = contentSet.Get(0)->GetValues();
    ASSERT_EQ(1, values1.MemberCount());
    EXPECT_STREQ("Test1", values1[FIELD_NAME(m_widgetClass, "MyID")].GetString());
    RapidJsonValueCR values2 = contentSet.Get(1)->GetValues();
    ASSERT_EQ(1, values2.MemberCount());
    EXPECT_STREQ("Test2", values2[FIELD_NAME(m_widgetClass, "MyID")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Kilinskas                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctValuesFromRelatedInstancesWithSeveralRelationships)
    {
    ECRelationshipClassCP widgetHasGadget = m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCP widgetHasGadgets = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){ instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test2"));});
    IECInstancePtr gadget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget3);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadget, *widget, *gadget2);

    KeySetPtr keyset = KeySet::Create();
    keyset->Add(m_widgetClass, ECInstanceId(ECInstanceId::FromString(widget->GetInstanceId().c_str())));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecification* spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection::RequiredRelationDirection_Both,"", m_gadgetClass->GetFullName());
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *keyset, nullptr, options.GetJson()).get();

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    bvector<ContentDescriptor::Field*> fieldVectorCopy = descriptor->GetAllFields();
    // hide all fields except Gadget_MyID
    for (ContentDescriptor::Field const* field : fieldVectorCopy)
        {
        if (!field->GetName().EndsWithI("MyID"))
            overridenDescriptor->RemoveField(field->GetName().c_str());
        }
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // validate descriptor
    EXPECT_EQ(1, overridenDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*overridenDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR values1 = contentSet.Get(0)->GetValues();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({"%s": "Test1"})", FIELD_NAME(m_gadgetClass, "MyID")).c_str());
    EXPECT_EQ(expectedValues1, values1)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(values1);

    RapidJsonValueCR values2 = contentSet.Get(1)->GetValues();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({"%s": "Test2"})", FIELD_NAME(m_gadgetClass, "MyID")).c_str());
    EXPECT_EQ(expectedValues2, values2)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(values2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Kilinskas                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctValuesOfCalculatedProperty)
    {
    //set up data
    ECRelationshipClassCP widgetHasGadgets = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID",ECValue("Test1"));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test2"));});
    IECInstancePtr gadget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    CalculatedPropertiesSpecification* calcProp1 = new CalculatedPropertiesSpecification("CalculatedMyID", 1000, "this.MyID+\"Calculated\"");
    ContentInstancesOfSpecificClassesSpecification* specificSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", m_gadgetClass->GetFullName(), false);
    specificSpec->AddCalculatedProperty(*calcProp1);
    contentRule->AddSpecification(*specificSpec);
    CalculatedPropertiesSpecification* calcProp2 = new CalculatedPropertiesSpecification("CalculatedMyID", 1000, "this.MyID+\"Calculated\"");
    ContentRelatedInstancesSpecification* relatedSpec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection::RequiredRelationDirection_Both, widgetHasGadgets->GetFullName(), m_gadgetClass->GetFullName());
    relatedSpec->AddCalculatedProperty(*calcProp2);
    contentRule->AddSpecification(*relatedSpec);
    SelectedNodeInstancesSpecification* selectedSpec = new SelectedNodeInstancesSpecification();
    CalculatedPropertiesSpecification* calcProp3 = new CalculatedPropertiesSpecification("CalculatedMyID", 1000, "this.MyID+\"Calculated\"");
    selectedSpec->AddCalculatedProperty(*calcProp3);
    contentRule->AddSpecification(*selectedSpec);

    rules->AddPresentationRule(*contentRule);

    KeySetPtr keys = KeySet::Create(bvector<IECInstancePtr>{widget, gadget1, gadget2, gadget3});

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *keys, nullptr, options.GetJson()).get();
    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    bvector<ContentDescriptor::Field*> fieldVectorCopy = descriptor->GetAllFields();
    // hide all fields except CalculatedProperty_0
    for (ContentDescriptor::Field const* field : fieldVectorCopy)
        {
        if (!field->GetName().Equals("CalculatedProperty_0"))
            overridenDescriptor->RemoveField(field->GetName().c_str());
        }
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // validate descriptor
    EXPECT_EQ(1, overridenDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*overridenDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR values1 = contentSet.Get(0)->GetValues();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({"CalculatedProperty_0": "Test1Calculated"})").c_str());
    EXPECT_EQ(expectedValues1, values1)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(values1);

    RapidJsonValueCR values2 = contentSet.Get(1)->GetValues();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({"CalculatedProperty_0": "Test2Calculated"})").c_str());
    EXPECT_EQ(expectedValues2, values2)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(values2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Kilinskas                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctValuesOfDisplayLabel)
    {
    ECRelationshipClassCP widgetHasGadgets = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test2"));});
    IECInstancePtr gadget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* specificSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", m_gadgetClass->GetFullName(), false);
    contentRule->AddSpecification(*specificSpec);
    ContentRelatedInstancesSpecification* relatedSpec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection::RequiredRelationDirection_Both, widgetHasGadgets->GetFullName(), m_gadgetClass->GetFullName());
    contentRule->AddSpecification(*relatedSpec);
    SelectedNodeInstancesSpecification* selectedSpec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*selectedSpec);
    rules->AddPresentationRule(*contentRule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, m_gadgetClass->GetFullName(), "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, m_widgetClass->GetFullName(), "MyID"));

    KeySetPtr keys = KeySet::Create(bvector<IECInstancePtr>{widget, gadget1, gadget2, gadget3});

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *keys, nullptr, options.GetJson()).get();
    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    bvector<ContentDescriptor::Field*> fieldVectorCopy = descriptor->GetAllFields();
    // remove all fields except DisplayLabel
    for (ContentDescriptor::Field const* field : fieldVectorCopy)
        {
        if (!field->IsDisplayLabelField())
            overridenDescriptor->RemoveField(field->GetName().c_str());
        }
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);
    overridenDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    // validate descriptor
    EXPECT_EQ(0, overridenDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*overridenDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ("Test1", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Test2",  contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, GetDistinctValuesFromMergedField)
    {
    // insert some widget & gadget instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2, instance3, instance4});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); //Gadget_Widget_MyId, Gadget_Widget_Description, Gadget_Widget, Widget_IntProperty, Widget_BoolProperty, Widget_DoubleProperty, Widget_LongProperty, Widget_Date

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    bvector<ContentDescriptor::Field*> fieldVectorCopy = descriptor->GetAllFields();
    // hide all fields except Widget_MyID
    for (ContentDescriptor::Field const* field : fieldVectorCopy)
        {
        if (!field->GetName().EndsWithI("MyID"))
            overridenDescriptor->RemoveField(field->GetName().c_str());
        }
    ASSERT_EQ(1, overridenDescriptor->GetVisibleFields().size());
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // request for content
    ContentCPtr content = m_manager->GetContent(*overridenDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetValues();
    EXPECT_STREQ("GadgetID", jsonValues[FIELD_NAME((bvector<ECClassCP>{m_gadgetClass, m_widgetClass}), "MyID")].GetString());
    RapidJsonValueCR jsonValues2 = contentSet.Get(1)->GetValues();
    EXPECT_STREQ("WidgetID", jsonValues2[FIELD_NAME((bvector<ECClassCP>{m_gadgetClass, m_widgetClass}), "MyID")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetsContentDescriptorWithNavigationPropertiesFromDifferentContentSpecifications)
    {
    // prepare dataset
    ECRelationshipClassCP widgetHasGadgets = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP gadgetHasSprockets = GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprockets, *gadget, *sprocket);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);

    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    ContentInstancesOfSpecificClassesSpecificationP gadgetSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", true);
    ContentInstancesOfSpecificClassesSpecificationP sprocketSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Sprocket", true);
    gadgetSpec->AddPropertyOverride(*new PropertySpecification("Widget", 1500, "", "", true));
    sprocketSpec->AddPropertyOverride(*new PropertySpecification("Gadget", 1500, "", "", true));
    rule->AddSpecification(*gadgetSpec);
    rule->AddSpecification(*sprocketSpec);

    // validate content descriptor
    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());

    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetValues();
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*widget).GetId().GetValue(), jsonValues[FIELD_NAME(m_gadgetClass, "Widget")].GetUint64());
    EXPECT_TRUE(jsonValues[FIELD_NAME(m_sprocketClass, "Gadget")].IsNull());

    RapidJsonValueCR jsonValues2 = contentSet.Get(1)->GetValues();
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetId().GetValue(), jsonValues2[FIELD_NAME(m_sprocketClass, "Gadget")].GetUint64());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(m_gadgetClass, "Widget")].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctNavigationProperties)
    {
    // prepare dataset
    ECRelationshipClassCP widgetHasGadgets = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgets, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgets, *widget1, *gadget);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);

    ruleSet->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    ContentInstancesOfSpecificClassesSpecificationP gadgetSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", true);
    gadgetSpec->AddPropertyOverride(*new PropertySpecification("Widget", 1500, "", "", true));
    rule->AddSpecification(*gadgetSpec);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());

    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // modify content descriptor
    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // request for content
    ContentCPtr content = m_manager->GetContent(*overridenDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("WidgetID", jsonValues[FIELD_NAME(m_gadgetClass, "Widget")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_HierarchyIsCached)
    {
    // prepare dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECClassInstanceKey widgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*widget1);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedECClassGroupingNode_HierarchyIsCached", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget", false));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], widgetKey1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_HierarchyCacheIsClear)
    {
    // prepare dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECClassInstanceKey widgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*widget1);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedECClassGroupingNode_HierarchyCacheIsClear", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget", false));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();
    // clear cache
    m_locater->InvalidateCache(ruleSet->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*ruleSet);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], widgetKey1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_SelectedNodeIsNotRootNode)
    {
    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECClassInstanceKey widgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*widget1);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedECClassGroupingNode_SelectedNodeIsNotRootNode", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rules
    // gadget rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    // sprocket rule
    ChildNodeRuleP childRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree);
    ruleSet->AddPresentationRule(*childRule);
    InstanceNodesOfSpecificClassesSpecification* childRuleSpec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Sprocket", false);
    childRule->AddSpecification(*childRuleSpec);
    // widget rule
    ChildNodeRuleP nestedChildRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree);
    nestedChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget", false));
    childRuleSpec->AddNestedRule(*nestedChildRule);


    /* Create the following hierarchy:
            + gadget
            +--+ Sprocket
            |  +--+ widget ECClassGroupingNode   <- selected node
            |     +--- widget instance
            |     +--- widget1 instace
    */

    // cache hierarchy
    // get gadget node
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    // get sprocket node
    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());

    // get widget class grouping node
    childNodes = m_manager->GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, childNodes[0]->GetType().c_str());

    // save widget class grouping node key
    NavNodeKeyCPtr selectedNode = childNodes[0]->GetKey();
    // clear cache
    m_locater->InvalidateCache(ruleSet->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*ruleSet);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], widgetKey1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_NewInstanceIsInsertedAndHierarchyCacheIsClear)
    {
    // prepare dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedECClassGroupingNode_NewInstanceIsInsertedAndHierarchyCacheIsClear", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget", false));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());

    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();
    // clear cache
    m_locater->InvalidateCache(ruleSet->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*ruleSet);

    // insert another widget
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ECClassInstanceKey widgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*widget1);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], widgetKey1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_OneInstanceIsDeletedAndHierarchyCacheIsClear)
    {
    // prepare dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedECClassGroupingNode_OneInstanceIsDeletedAndHierarchyCacheIsClear", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget", false));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();
    // clear cache
    m_locater->InvalidateCache(ruleSet->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*ruleSet);

    // delete one widget
    RulesEngineTestHelpers::DeleteInstance(s_project->GetECDb(), *widget1);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedDisplayLabelGroupingNode_HierarchyIsCached)
    {
    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget"));});
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget"));});

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECClassInstanceKey widgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*widget1);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedDisplayLabelGroupingNode_HierarchyIsCached", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    ruleSet->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", "RulesEngineTest:Widget,Gadget", false));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str()); // gadget instance
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, rootNodes[1]->GetType().c_str()); // widgets display grouping node

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[1]->GetKey();

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], widgetKey1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedDisplayLabelGroupingNode_HierarchyCacheIsEmpty)
    {
    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget"));});
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget"));});

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECClassInstanceKey widgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*widget1);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedDisplayLabelGroupingNode_HierarchyCacheIsEmpty", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    ruleSet->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", "RulesEngineTest:Widget,Gadget", false));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str()); // gadget instance
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, rootNodes[1]->GetType().c_str()); // widgets display grouping node

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[1]->GetKey();

    // clear caches
    m_locater->InvalidateCache(ruleSet->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*ruleSet);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], widgetKey1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysWhenDisplayLabelGroupingNodeIsCreated)
    {
    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget"));});

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedDisplayLabelGroupingNode_HierarchyCacheIsEmpty", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    ruleSet->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", "RulesEngineTest:Widget,Gadget", false));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str()); // gadget instance
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str()); // widget node

    // save widget node key
    NavNodeKeyCPtr selectedNode = rootNodes[1]->GetKey();

    // clear caches
    m_locater->InvalidateCache(ruleSet->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*ruleSet);

    // insert another widget DisplayGroupingNode is created
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget"));});

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysWhenDisplayLabelGroupingNodeIsRemoved)
    {
    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget"));});
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget"));});

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedDisplayLabelGroupingNode_HierarchyCacheIsEmpty", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    ruleSet->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", "RulesEngineTest:Widget,Gadget", false));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str()); // gadget instance
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, rootNodes[1]->GetType().c_str()); // widgets display grouping node

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[1]->GetKey();

    // clear caches
    m_locater->InvalidateCache(ruleSet->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*ruleSet);

    // delete one widget
    RulesEngineTestHelpers::DeleteInstance(s_project->GetECDb(), *widget1);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECPropertyGroupingNode_HierarchyIsCached)
    {
    // prepare dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECClassInstanceKey widgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*widget1);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedECPropertyGroupingNode_HierarchyIsCached", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget,Gadget", false));
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    ruleSet->AddPresentationRule(*groupingRule);
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "IntProperty"));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], widgetKey1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECPropertyGroupingNode_HierarchyCacheIsEmpty)
    {
    // prepare dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});

    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECClassInstanceKey widgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*widget1);

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("GetKeysForSelectedECPropertyGroupingNode_HierarchyCacheIsEmpty", 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    ruleSet->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget,Gadget", false));
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    ruleSet->AddPresentationRule(*groupingRule);
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "IntProperty"));

    // cache hierarchy
    RulesDrivenECPresentationManager::NavigationOptions navOptions(ruleSet->GetRuleSetId().c_str());
    NavNodesContainer rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), navOptions.GetJson()).get();
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    // clear caches
    m_locater->InvalidateCache(ruleSet->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*ruleSet);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());
    // validate content descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*selectedNode), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], widgetKey);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], widgetKey1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntLoadCompositeContentIfInstanceDoesntHaveCompositeProperty, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECProperty propertyName="BIntProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntLoadCompositeContentIfInstanceDoesntHaveCompositeProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB});

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntTakeContentTwiceForTheSameInstance, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="StructProperty" typeName="int" />
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward" description="The parent element that owns this element.">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntTakeContentTwiceForTheSameInstance)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP elementOwnsChildElement = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();
    IECInstancePtr rootElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr childElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElement, *rootElement, *childElement);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection::RequiredRelationDirection_Forward,
        elementOwnsChildElement->GetFullName(), elementClass->GetFullName()));
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{rootElement, childElement});

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 2 content set items
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_OverridesInstanceLabelOfSpecifiedClass)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID,Description"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*gadget).c_str(), contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("WidgetID", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_OverridesInstanceLabelOfSpecifiedClassesWithDifferentProperties)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("GadgetDescription"));});
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("SprocketID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID,Description"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "Description,MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget,Sprocket", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());
    EXPECT_STREQ("GadgetDescription", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("WidgetID", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*sprocket).c_str(), contentSet.Get(2)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_AppliedByPriority)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("WidgetID"));
        instance.SetValue("Description", ECValue("WidgetDescription"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(2, true, "RulesEngineTest:Widget", "Description"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget,Sprocket", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("WidgetDescription", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_WhenNoPropertiesSpecified_FallsBackToLabelOverrides)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"LabelOverride\"", ""));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("LabelOverride", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_AssertsWhenNoClasesSpecified_FallsBackToLabelOverrides)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"LabelOverride\"", ""));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    IGNORE_BE_ASSERT();
    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("LabelOverride", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_HandlesDefaultBisRulesCorrectly, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement" displayLabel="Geometric Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="CustomElement" displayLabel="Custom Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="CustomGeometricElement" displayLabel="Custom Geometric Element">
        <BaseClass>GeometricElement</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_HandlesDefaultBisRulesCorrectly)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP geometricElementClass = GetClass("GeometricElement");
    ECClassCP customElementClass = GetClass("CustomElement");
    ECClassCP customGeometricElementClass = GetClass("CustomGeometricElement");

    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("UserLabel", ECValue("UserLabel"));
        });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("CodeValue", ECValue("CodeValue"));
        });
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass);
    IECInstancePtr geometricElement1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customGeometricElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("CodeValue", ECValue("CodeValue"));
        });
    IECInstancePtr geometricElement2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customGeometricElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("UserLabel", ECValue("UserLabel"));
        });
    IECInstancePtr geometricElement3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customGeometricElementClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, geometricElementClass->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("CodeValue"),
        new InstanceLabelOverrideCompositeValueSpecification(
            {
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverridePropertyValueSpecification("UserLabel"), true),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideCompositeValueSpecification(
                {
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("[")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideBriefcaseIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("-")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("]"))
                }, ""))
            }, " "),
        new InstanceLabelOverrideCompositeValueSpecification(
            {
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideClassLabelValueSpecification(), true),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideCompositeValueSpecification(
                {
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("[")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideBriefcaseIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("-")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("]"))
                }, ""))
            }, " ")
        }));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, elementClass->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("UserLabel"),
        new InstanceLabelOverridePropertyValueSpecification("CodeValue"),
        new InstanceLabelOverrideCompositeValueSpecification(
            {
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideClassLabelValueSpecification(), true),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideCompositeValueSpecification(
                {
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("[")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideBriefcaseIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("-")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("]"))
                }, ""))
            })
        }));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(6, contentSet.GetSize());

    EXPECT_STREQ("UserLabel", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("CodeValue", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Element [0-3]", contentSet.Get(2)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ("CodeValue", contentSet.Get(3)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("UserLabel [0-5]", contentSet.Get(4)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Geometric Element [0-6]", contentSet.Get(5)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceLabelIsOverridenByParentClassProperty, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <ECNavigationProperty propertyName="A" relationshipName="ClassCUsesClassA" direction="Forward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassCUsesClassA" strength="referencing" strengthDirection="backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassC Uses ClassA" polymorphic="True">
            <Class class="ClassC" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassA is used by ClassC" polymorphic="True">
            <Class class="ClassA" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, RelatedInstanceLabelIsOverridenByParentClassProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassB_StringProperty"));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    ECRelationshipClassCP classCUsesClassA = GetClass("ClassCUsesClassA")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classCUsesClassA, *instanceB, *instanceC);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceLabelIsOverridenByParentClassProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, classCUsesClassA->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassB_StringProperty", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingBaseClassPolymorphicallyInstanceLabelOverrideAppliedToSpecifiedClass, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingBaseClassPolymorphicallyInstanceLabelOverrideAppliedToSpecifiedClass)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassB_StringProperty"));});
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassA_StringProperty"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SelectingBaseClassPolymorphicallyInstanceLabelOverrideAppliedToSpecifiedClass", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "BaseStringProperty"));
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("ClassB_StringProperty", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingBaseClassPolymorphicallyChildrenLabelsAreOverriden, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
        <ECProperty propertyName="ClassC_String" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingBaseClassPolymorphicallyChildrenLabelsAreOverriden)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassB_BaseStringProperty"));});
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassA_BaseStringProperty"));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance)
        {
        instance.SetValue("ClassC_String", ECValue("ClassC_StringProperty"));
        instance.SetValue("BaseStringProperty", ECValue("ClassC_BaseStringProperty"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SelectingBaseClassPolymorphicallyChildrenLabelsAreOverriden", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "ClassC_String"));
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    EXPECT_STREQ("ClassB_BaseStringProperty", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("ClassA_BaseStringProperty", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("ClassC_StringProperty", contentSet.Get(2)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelOverridesAppliedPolymorphically, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
        <ECProperty propertyName="ClassC_String" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelOverridesAppliedPolymorphically)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassC_BaseStringProperty"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SelectingBaseClassPolymorphicallyChildrenLabelsAreOverriden", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(2, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassC_BaseStringProperty", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelOverridesAppliedPolymorphically_MultipleInheritance, R"*(
    <ECEntityClass typeName="ClassA1">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassA2" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0">
                <AppliesToEntityClass>ClassB</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA1</BaseClass>
        <BaseClass>ClassA2</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelOverridesAppliedPolymorphically_MultipleInheritance)
    {
    // set up data set
    ECClassCP classA1 = GetClass("ClassA1");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassC_CodeValue"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA1->GetFullName(), "CodeValue"));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassC_CodeValue", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelOverridesAppliedPolymorphically_MultipleInheritance_TakesBaseClassProperty, R"*(
    <ECEntityClass typeName="ClassA1">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassA2" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0">
                <AppliesToEntityClass>ClassB</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA1</BaseClass>
        <BaseClass>ClassA2</BaseClass>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelOverridesAppliedPolymorphically_MultipleInheritance_TakesBaseClassProperty)
    {
    // set up data set
    ECClassCP classA1 = GetClass("ClassA1");
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassC_CodeValue"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA1->GetFullName(), "CodeValue"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "UserLabel"));
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(2, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassC_CodeValue", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NavigationPropertyLabelIsOverridenByTargetClassProperty, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <ECNavigationProperty propertyName="A" relationshipName="ClassCUsesClassA" direction="Forward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassCUsesClassA" strength="referencing" strengthDirection="backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassC Uses ClassA" polymorphic="True">
            <Class class="ClassC" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassA is used by ClassC" polymorphic="True">
            <Class class="ClassA" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, NavigationPropertyLabelIsOverridenByTargetClassProperty)
    {
    // set up data set
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassB_StringProperty"));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    ECRelationshipClassCP classCUsesClassA = GetClass("ClassCUsesClassA")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classCUsesClassA, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("NavigationPropertyLabelIsOverridenByTargetClassProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "BaseStringProperty"));
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("ClassB_StringProperty", jsonValues[FIELD_NAME(classC, "A")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NavigationPropertyLabelIsOverridenPolymorphically, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassD">
        <ECNavigationProperty propertyName="A" relationshipName="ClassDUsesClassA" direction="Forward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassDUsesClassA" strength="referencing" strengthDirection="backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassC Uses ClassA" polymorphic="True">
            <Class class="ClassD" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassA is used by ClassC" polymorphic="True">
            <Class class="ClassA" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, NavigationPropertyLabelIsOverridenPolymorphically)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classC = GetClass("ClassC");
    ECClassCP classD = GetClass("ClassD");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassC_StringProperty"));});
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    ECRelationshipClassCP classCUsesClassA = GetClass("ClassDUsesClassA")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classCUsesClassA, *instanceC, *instanceD);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("NavigationPropertyLabelIsOverridenPolymorphically", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classD->GetFullName(), false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("ClassC_StringProperty", jsonValues[FIELD_NAME(classD, "A")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelUsingInstanceLabelOverrideAndGetRelatedInstanceLabelExpression, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelUsingInstanceLabelOverrideAndGetRelatedInstanceLabelExpression)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassB_UserLabel"));});
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasClassB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "UserLabel"));
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Forward\", \"%s\")", classAHasClassB->GetFullName(), classB->GetFullName()), ""));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassB_UserLabel", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelUsingLabelOverrideAndGetRelatedInstanceLabelExpression, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelUsingLabelOverrideAndGetRelatedInstanceLabelExpression)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassB_UserLabel"));});
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasClassB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classB->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, "this.UserLabel", ""));
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Forward\", \"%s\")", classAHasClassB->GetFullName(), classB->GetFullName()), ""));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassB_UserLabel", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelBackwardUsingLabelOverrideAndGetRelatedInstanceLabelExpression, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelBackwardUsingLabelOverrideAndGetRelatedInstanceLabelExpression)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassB_UserLabel"));});
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasClassB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, "this.UserLabel", ""));
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classB->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Backward\", \"%s\")", classAHasClassB->GetFullName(), classA->GetFullName()), ""));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassA_UserLabel", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelUsingGetRelatedInstanceLabelExpression_NoRelatedLabelOverrideRulesFound_ReturnsNotSpecifiedString, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelUsingGetRelatedInstanceLabelExpression_NoRelatedLabelOverrideRulesFound_ReturnsNotSpecifiedString)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassB_UserLabel"));});
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasClassB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Forward\", \"%s\")", classAHasClassB->GetFullName(), classB->GetFullName()), ""));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    // Expecting "Not specified" label
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelUsingGetRelatedInstanceLabelExpression_NoRelatedInstanceFound_ReturnsNotSpecifiedString, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelUsingGetRelatedInstanceLabelExpression_NoRelatedInstanceFound_ReturnsNotSpecifiedString)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classB->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, "this.UserLabel", ""));
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Forward\", \"%s\")", classAHasClassB->GetFullName(), classB->GetFullName()), ""));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* make concrete aspect properties are included into the content when requesting it
* with a ContentInstancesOfSpecificClassesSpecification
* @bsitest                                      Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForInstancesOfSpecificClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForInstancesOfSpecificClasses)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element"));});
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance){instance.SetValue("AspectName", ECValue("my aspect"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseElementClass->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseElementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_MyAspect

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect"
                },
            "DisplayValues": {
                "%s": "my aspect"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(baseElementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "AspectName"), FIELD_NAME(aspectClass, "AspectName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsMultiStepPolymorphicallyRelatedPropertiesForInstancesOfSpecificClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementRefersToElement" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsMultiStepPolymorphicallyRelatedPropertiesForInstancesOfSpecificClasses)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    ECRelationshipClassCP elementRefersToElementRelationship = GetClass("ElementRefersToElement")->GetRelationshipClassCP();

    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("ElementName", ECValue("element 1")); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementRefersToElementRelationship, *element2, *element1);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance) {instance.SetValue("AspectName", ECValue("aspect")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(elementRefersToElementRelationship->GetFullName(), RequiredRelationDirection_Backward),
        new RelationshipStepSpecification(elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName()),
        }), PropertySpecificationsList(), RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "aspect"
                },
            "DisplayValues": {
                "%s": "aspect"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{elementClass, baseElementClass}), aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "AspectName"), FIELD_NAME(aspectClass, "AspectName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* make concrete element properties are included into the content when requesting it
* backward with a ContentInstancesOfSpecificClassesSpecification
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyBackwardRelatedPropertiesForInstancesOfSpecificClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyBackwardRelatedPropertiesForInstancesOfSpecificClasses)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element"));});
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance){instance.SetValue("AspectName", ECValue("my aspect"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseAspectClass->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseAspectClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseElementClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // rel_ElementUniqueAspect_MyElement_ElementName

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element"
        })", RELATED_FIELD_NAME(baseAspectClass, elementClass, "ElementName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* make sure we don't include aspects into the content if they're not actually related
* with the instances we're asking content for
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntLoadPolymorphicallyRelatedPropertiesWhenThereAreNoRelatedInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntLoadPolymorphicallyRelatedPropertiesWhenThereAreNoRelatedInstances)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element"));});
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance){instance.SetValue("AspectName", ECValue("my aspect"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseElementClass->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseElementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(0, descriptor->GetVisibleFields().size()); // aspect property field not included
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different aspects, but only one of them matches instance filter
* - make sure we don't include aspect of instance which doesn't match the filter
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForInstancesOfSpecificClassesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForInstancesOfSpecificClassesMatchingInstanceFilter)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "this.ElementName = \"my element 1\"",
        elementClass->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects of the
* selected elements are included into the content
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*element1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsMultiStepPolymorphicallyRelatedPropertiesForSelectedNodeInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementRefersToElement" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsMultiStepPolymorphicallyRelatedPropertiesForSelectedNodeInstances)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    ECRelationshipClassCP elementRefersToElementRelationship = GetClass("ElementRefersToElement")->GetRelationshipClassCP();

    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("ElementName", ECValue("element 1")); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementRefersToElementRelationship, *element2, *element1);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance) {instance.SetValue("AspectName", ECValue("aspect")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(elementRefersToElementRelationship->GetFullName(), RequiredRelationDirection_Backward),
        new RelationshipStepSpecification(elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName()),
        }), PropertySpecificationsList(), RelationshipMeaning::SameInstance, true));

    // options
    KeySetPtr input = KeySet::Create(*element1);
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "aspect"
                },
            "DisplayValues": {
                "%s": "aspect"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{elementClass, baseElementClass}), aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "AspectName"), FIELD_NAME(aspectClass, "AspectName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenRelatedClassesAreNotSpecified, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenRelatedClassesAreNotSpecified)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    // unused - ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        "", "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*element1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenRelationshipsAreNotSpecified, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenRelationshipsAreNotSpecified)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "",
        baseAspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*element1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects of the
* related selected elements are included into the content
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward,
        modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*model1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsMultiStepPolymorphicallyRelatedPropertiesForRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementRefersToElement" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsMultiStepPolymorphicallyRelatedPropertiesForRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    ECRelationshipClassCP elementRefersToElementRelationship = GetClass("ElementRefersToElement")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("ElementName", ECValue("element 1")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementRefersToElementRelationship, *element2, *element1);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance) {instance.SetValue("AspectName", ECValue("aspect")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward,
        modelContainsElementsRelationship->GetFullName(), baseElementClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(elementRefersToElementRelationship->GetFullName(), RequiredRelationDirection_Backward),
        new RelationshipStepSpecification(elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName()),
        }), PropertySpecificationsList(), RelationshipMeaning::SameInstance, true));

    // options
    KeySetPtr input = KeySet::Create(*model);
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "aspect"
                },
            "DisplayValues": {
                "%s": "aspect"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{elementClass, baseElementClass}), aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "AspectName"), FIELD_NAME(aspectClass, "AspectName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects whose
* elements match the instance filter are included into the content
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForRelatedNodeInstancesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForRelatedNodeInstancesMatchingInstanceFilter)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "this.ElementName = \"my element 1\"", RequiredRelationDirection_Forward,
        modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*model);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects whose
* elements are recursively related to the selected element are included into the content
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_LoadsPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectC">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_C_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectD">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_D_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_LoadsPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECClassCP aspectCClass = GetClass("MyAspectC");
    ECClassCP aspectDClass = GetClass("MyAspectD");
    ECRelationshipClassCP elementOwnsChildElementsRelationship = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 3"));});
    IECInstancePtr element4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 4"));});
    IECInstancePtr element5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 5"));});
    IECInstancePtr element6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 6"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass, [](IECInstanceR instance){instance.SetValue("Aspect_C_Name", ECValue("my aspect c"));});
    IECInstancePtr aspect6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectDClass, [](IECInstanceR instance){instance.SetValue("Aspect_D_Name", ECValue("my aspect d"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element1, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element2, *element3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element1, *element4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element5, *element6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element3, *aspect3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element4, *aspect4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element6, *aspect6);

    /* hierarchy:
            el1         el5
            / \          |
          el2  el4      el6
           |
          el3
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Forward,
        elementOwnsChildElementsRelationship->GetFullName(), elementClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*element1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(4, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA, Element_MyAspectB, Element_MyAspectC

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 2",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 3",
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect b"
                },
            "DisplayValues": {
                "%s": "my aspect b"
                },
            "MergedFieldNames": []
            }],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass),
        aspectBClass->GetId().ToString().c_str(), aspect3->GetInstanceId().c_str(),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"), FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 4",
        "%s": [],
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect c"
                },
            "DisplayValues": {
                "%s": "my aspect c"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass),
        aspectCClass->GetId().ToString().c_str(), aspect4->GetInstanceId().c_str(),
        FIELD_NAME(aspectCClass, "Aspect_C_Name"), FIELD_NAME(aspectCClass, "Aspect_C_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects whose
* elements are recursively related to the selected element are included into the content
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectC">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_C_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectD">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_D_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECClassCP aspectCClass = GetClass("MyAspectC");
    ECClassCP aspectDClass = GetClass("MyAspectD");
    ECRelationshipClassCP elementOwnsChildElementsRelationship = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 3"));});
    IECInstancePtr element4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 4"));});
    IECInstancePtr element5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 5"));});
    IECInstancePtr element6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 6"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass, [](IECInstanceR instance){instance.SetValue("Aspect_C_Name", ECValue("my aspect c"));});
    IECInstancePtr aspect6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectDClass, [](IECInstanceR instance){instance.SetValue("Aspect_D_Name", ECValue("my aspect d"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element1, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element2, *element3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element1, *element4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element5, *element6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element3, *aspect3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element4, *aspect4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element6, *aspect6);

    /* hierarchy:
            el1         el5
            / \          |
           /   \         |
        el2(A) el4(B)  el6(D)
          |
        el3(C)
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification(
        {
        new RepeatableRelationshipStepSpecification(elementOwnsChildElementsRelationship->GetFullName(), RequiredRelationDirection_Forward, elementClass->GetFullName(), 0),
        })}));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(
        elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName()
    )), PropertySpecificationsList(), RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*element1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA, Element_MyAspectB, Element_MyAspectC

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(4, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 2",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 3",
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect b"
                },
            "DisplayValues": {
                "%s": "my aspect b"
                },
            "MergedFieldNames": []
            }],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass),
        aspectBClass->GetId().ToString().c_str(), aspect3->GetInstanceId().c_str(),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"), FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(3)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 4",
        "%s": [],
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect c"
                },
            "DisplayValues": {
                "%s": "my aspect c"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass),
        aspectCClass->GetId().ToString().c_str(), aspect4->GetInstanceId().c_str(),
        FIELD_NAME(aspectCClass, "Aspect_C_Name"), FIELD_NAME(aspectCClass, "Aspect_C_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForInstancesOfSpecificClassesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForInstancesOfSpecificClassesMatchingInstanceFilter)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 1"));});
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 2"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "this.ModelName = \"my model 1\"",
        modelClass->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), modelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(), baseAspectClass->GetFullName(), "", RelationshipMeaning::RelatedInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Model_ModelName, Model_Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"), NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 1"));});
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 2"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), modelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(), baseAspectClass->GetFullName(), "", RelationshipMeaning::RelatedInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*model1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Model_ModelName, Model_Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"), NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsCategories" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Category" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Category">
        <ECProperty propertyName="CategoryName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="CategoryContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Category"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP categoryClass = GetClass("Category");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsCategoriesRelationship = GetClass("ModelContainsCategories")->GetRelationshipClassCP();
    ECRelationshipClassCP categoryContainsElementsRelationship = GetClass("CategoryContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr category1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *categoryClass, [](IECInstanceR instance){instance.SetValue("CategoryName", ECValue("my category 1"));});
    IECInstancePtr category2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *categoryClass, [](IECInstanceR instance){instance.SetValue("CategoryName", ECValue("my category 1"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsCategoriesRelationship, *model1, *category1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsCategoriesRelationship, *model2, *category2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *categoryContainsElementsRelationship, *category1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *categoryContainsElementsRelationship, *category2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward,
        modelContainsCategoriesRelationship->GetFullName(), categoryClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), categoryClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, categoryContainsElementsRelationship->GetFullName(), elementClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(), baseAspectClass->GetFullName(), "", RelationshipMeaning::RelatedInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*model1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Category_CategoryName, Category_Element_MyAspectA

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my category 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(categoryClass, "CategoryName"), NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{categoryClass, elementClass}), aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_LoadsNestedPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelOwnsSubModel" modifier="Sealed" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="owns sub" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Model"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectC">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_C_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectD">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_D_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_LoadsNestedPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECClassCP aspectCClass = GetClass("MyAspectC");
    ECClassCP aspectDClass = GetClass("MyAspectD");
    ECRelationshipClassCP modelOwnsSubModelRelationship = GetClass("ModelOwnsSubModel")->GetRelationshipClassCP();
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 1"));});
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 2"));});
    IECInstancePtr model3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 3"));});
    IECInstancePtr model4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 4"));});
    IECInstancePtr model5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 5"));});
    IECInstancePtr model6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 6"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 3"));});
    IECInstancePtr element4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 4"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass, [](IECInstanceR instance){instance.SetValue("Aspect_C_Name", ECValue("my aspect c"));});
    IECInstancePtr aspect6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectDClass, [](IECInstanceR instance){instance.SetValue("Aspect_D_Name", ECValue("my aspect d"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model1, *model2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model2, *model3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model1, *model4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model5, *model6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model3, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model4, *element3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model6, *element4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element3, *aspect4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element4, *aspect6);

    /* hierarchy:
             m1          m5
            / \          |
           m2  m4        m6
           |
           m3
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Forward,
        modelOwnsSubModelRelationship->GetFullName(), modelClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), modelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(), baseAspectClass->GetFullName(), "", RelationshipMeaning::RelatedInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*model1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(4, descriptor->GetVisibleFields().size()); // Model_ModelName, Model_Element_MyAspectA, Model_Element_MyAspectB, Model_Element_MyAspectC

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 2",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 3",
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect b"
                },
            "DisplayValues": {
                "%s": "my aspect b"
                },
            "MergedFieldNames": []
            }],
        "%s": []
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        aspectBClass->GetId().ToString().c_str(), aspect3->GetInstanceId().c_str(),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"), FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 4",
        "%s": [],
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect c"
                },
            "DisplayValues": {
                "%s": "my aspect c"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass),
        aspectCClass->GetId().ToString().c_str(), aspect4->GetInstanceId().c_str(),
        FIELD_NAME(aspectCClass, "Aspect_C_Name"), FIELD_NAME(aspectCClass, "Aspect_C_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelOwnsSubModel" modifier="Sealed" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="owns sub" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Model"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectC">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_C_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectD">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_D_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECClassCP aspectCClass = GetClass("MyAspectC");
    ECClassCP aspectDClass = GetClass("MyAspectD");
    ECRelationshipClassCP modelOwnsSubModelRelationship = GetClass("ModelOwnsSubModel")->GetRelationshipClassCP();
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 1"));});
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 2"));});
    IECInstancePtr model3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 3"));});
    IECInstancePtr model4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 4"));});
    IECInstancePtr model5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 5"));});
    IECInstancePtr model6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 6"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 3"));});
    IECInstancePtr element4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 4"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass, [](IECInstanceR instance){instance.SetValue("Aspect_C_Name", ECValue("my aspect c"));});
    IECInstancePtr aspect6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectDClass, [](IECInstanceR instance){instance.SetValue("Aspect_D_Name", ECValue("my aspect d"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model1, *model2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model2, *model3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model1, *model4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model5, *model6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model3, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model4, *element3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model6, *element4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element3, *aspect4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element4, *aspect6, nullptr, true);

    /* hierarchy:
             m1          m5
            / \          |
           m2  m4        m6
         / |   |         |
        /  m3 e3(a4)   e4(a6)
   e1(a2)  |
         e2(a3)

    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(
        modelOwnsSubModelRelationship->GetFullName(), RequiredRelationDirection_Forward, modelClass->GetFullName(), 0))}));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), modelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(modelContainsElementsRelationship->GetFullName(), RequiredRelationDirection_Forward, elementClass->GetFullName()),
        new RelationshipStepSpecification(elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName())
    }), PropertySpecificationsList(), RelationshipMeaning::RelatedInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(*model1);

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Model_ModelName, Model_Element_MyAspectA, Model_Element_MyAspectB, Model_Element_MyAspectC

    // request for content
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(4, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 1",
        "%s": [],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 2",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 3",
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect b"
                },
            "DisplayValues": {
                "%s": "my aspect b"
                },
            "MergedFieldNames": []
            }],
        "%s": []
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        aspectBClass->GetId().ToString().c_str(), aspect3->GetInstanceId().c_str(),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"), FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(3)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 4",
        "%s": [],
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect c"
                },
            "DisplayValues": {
                "%s": "my aspect c"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass),
        aspectCClass->GetId().ToString().c_str(), aspect4->GetInstanceId().c_str(),
        FIELD_NAME(aspectCClass, "Aspect_C_Name"), FIELD_NAME(aspectCClass, "Aspect_C_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesForMultipleInstancesOfSameClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsMultiAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementMultiAspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesForMultipleInstancesOfSameClasses)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("ElementMultiAspect");
    ECRelationshipClassCP elementOwnsMultipleAspectRelationship = GetClass("ElementOwnsMultiAspects")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element1, *aspect1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("def"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsMultipleAspectRelationship->GetFullName(),
        aspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{element1, element2});

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_ElementMultiAspect

    // set the "merge results" flag
    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                },
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 123,
                "%s": null
                },
            "DisplayValues": {
                "%s": "123",
                "%s": "%s"
                },
            "MergedFieldNames": [
                "%s"
                ]
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        aspectClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        varies_string.c_str(),
        FIELD_NAME(aspectClass, "StringProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesForMultipleInstancesOfDifferentClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsMultiAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementMultiAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="MyElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesForMultipleInstancesOfDifferentClasses)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementAClass = GetClass("MyElementA");
    ECClassCP elementBClass = GetClass("MyElementB");
    ECClassCP aspectClass = GetClass("ElementMultiAspect");
    ECRelationshipClassCP elementOwnsMultipleAspectRelationship = GetClass("ElementOwnsMultiAspects")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element1, *aspect1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("def"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseElementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsMultipleAspectRelationship->GetFullName(),
        aspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{element1, element2});

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_ElementMultiAspect

    // set the "merge results" flag
    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                },
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 123,
                "%s": null
                },
            "DisplayValues": {
                "%s": "123",
                "%s": "%s"
                },
            "MergedFieldNames": [
                "%s"
                ]
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(baseElementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        aspectClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        varies_string.c_str(),
        FIELD_NAME(aspectClass, "StringProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* Merges nested related properties into same field when nested ralationship ends
* matches (Both hierarchies ends with same related class (ElementMultiAspect)).
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedRelatedPropertiesForMultipleInstancesOfDifferentClasses, R"*(
    <ECEntityClass typeName="Model">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsMultiAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementMultiAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyModelA">
        <BaseClass>Model</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="MyModelB">
        <BaseClass>Model</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="MyElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="MyElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedRelatedPropertiesForMultipleInstancesOfDifferentClasses)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP baseModelClass = GetClass("Model");
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementAClass = GetClass("MyElementA");
    ECClassCP elementBClass = GetClass("MyElementB");
    ECClassCP modelAClass = GetClass("MyModelA");
    ECClassCP modelBClass = GetClass("MyModelB");
    ECClassCP aspectClass = GetClass("ElementMultiAspect");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsMultipleAspectRelationship = GetClass("ElementOwnsMultiAspects")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelAClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element1, *aspect1);
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelBClass);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("def"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element2, *aspect2);

    /* hierarchies:
        model1(MyModelA) -> element1(MyElementA) -> aspect1(ElementMultiAspect)
        model2(MyModelB) -> element2(MyElementB) -> aspect2(ElementMultiAspect)
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseModelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, modelContainsElementsRelationship->GetFullName(),
        baseElementClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsMultipleAspectRelationship->GetFullName(),
        aspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{model1, model2});

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Model_Element_ElementMultiAspect

    // set the "merge results" flag
    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                },
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 123,
                "%s": null
                },
            "DisplayValues": {
                "%s": "123",
                "%s": "%s"
                },
            "MergedFieldNames": [
                "%s"
                ]
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{baseModelClass, baseElementClass}), aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        aspectClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        varies_string.c_str(),
        FIELD_NAME(aspectClass, "StringProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesForMultipleInstancesOfSameClassesOnlySecondInstanceHasAspect, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsMultiAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementMultiAspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesForMultipleInstancesOfSameClassesOnlySecondInstanceHasAspect)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Varies()).c_str());

    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("ElementMultiAspect");
    ECRelationshipClassCP elementOwnsMultipleAspectRelationship = GetClass("ElementOwnsMultiAspects")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [] (IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsMultipleAspectRelationship->GetFullName(),
        aspectClass->GetFullName(), "", RelationshipMeaning::SameInstance, false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{element1, element2});

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *input, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*mergingDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    EXPECT_TRUE(record->GetValues()[NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass)].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass)].GetString());
    EXPECT_TRUE(record->IsMerged(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsDisplayLabelOfSingleInstance, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="DisplayLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsDisplayLabelOfSingleInstance)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("DisplayLabel", ECValue("abc"));
        });

    // create key
    ECInstanceKey key(elementClass->GetId(), RulesEngineTestHelpers::GetInstanceKey(*element).GetId());

    // get label
    LabelDefinitionCPtr label = m_manager->GetDisplayLabel(s_project->GetECDb(), key).get();

    // verify
    EXPECT_STREQ("abc", label->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsDisplayLabelOfMultipleInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="DisplayLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsDisplayLabelOfMultipleInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("DisplayLabel", ECValue("abc"));
        });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("DisplayLabel", ECValue("def"));
        });

    // create keys
    KeySetPtr keys = KeySet::Create(
        bvector<ECClassInstanceKey>{
        RulesEngineTestHelpers::GetInstanceKey(*element1),
        RulesEngineTestHelpers::GetInstanceKey(*element2)
        });

    // get label
    LabelDefinitionCPtr label = m_manager->GetDisplayLabel(s_project->GetECDb(), *keys).get();

    // verify
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_MultipleInstances()).c_str(), label->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas             09/2018
* VSTS#29087
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HandlesContentWithPolymorphicallyRelatedPropertiesAndRelatedInstanceUsedInInstanceFilterCorrectly,
R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class = "Aspect" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="DerivedAspect">
        <BaseClass>Aspect</BaseClass>
        <ECProperty propertyName="intProp" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="InfoAspect">
        <BaseClass>Aspect</BaseClass>
        <ECProperty propertyName="intProp" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, HandlesContentWithPolymorphicallyRelatedPropertiesAndRelatedInstanceUsedInInstanceFilterCorrectly)
    {
    // prepare the dataset
    ECRelationshipClassCP rel = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementHasAspect"));
    ECClassCP ecClassElement = GetSchema()->GetClassCP("Element");
    ECClassCP ecClassAspect = GetSchema()->GetClassCP("Aspect");
    ECClassCP ecClassInfoAspect = GetSchema()->GetClassCP("InfoAspect");

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClassElement);
    IECInstancePtr infoAspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClassInfoAspect, [](IECInstanceR instance) {instance.SetValue("intProp", ECValue(75)); });

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *infoAspect, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    ContentRuleP rule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecificationP specification = new ContentInstancesOfSpecificClassesSpecification(1, "b.intProp < 100", ecClassElement->GetFullName(), true);
    specification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), ecClassInfoAspect->GetFullName(), "b", true));
    specification->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), ecClassAspect->GetFullName(), "",
        RelationshipMeaning::RelatedInstance, true));

    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    // get content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // assert
    ASSERT_TRUE(content.IsValid());
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 75
                },
            "DisplayValues": {
                "%s": "75"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(ecClassElement, ecClassInfoAspect),
        ecClassInfoAspect->GetId().ToString().c_str(), infoAspect->GetInstanceId().c_str(),
        FIELD_NAME(ecClassInfoAspect, "intProp"), FIELD_NAME(ecClassInfoAspect, "intProp")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AppliesContentItemExtendedDataFromPresentationRules, R"*(
    <ECEntityClass typeName="MyClass1">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClass2">
        <BaseClass>MyClass1</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, AppliesContentItemExtendedDataFromPresentationRules)
    {
    // set up data set
    ECClassCP ecClass1 = GetClass("MyClass1");
    ECClassCP ecClass2 = GetClass("MyClass2");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass1, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("test value")); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass2);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass1->GetFullName(), true));

    ExtendedDataRule* ex1 = new ExtendedDataRule();
    ex1->AddItem("class_name", "ThisNode.ClassName");
    ex1->AddItem("property_value", "this.CodeValue");
    ex1->AddItem("is_MyClass", Utf8PrintfString("ThisNode.IsOfClass(\"%s\", \"%s\")", ecClass1->GetName().c_str(), ecClass1->GetSchema().GetName().c_str()));
    rules->AddPresentationRule(*ex1);

    ExtendedDataRule* ex2 = new ExtendedDataRule(Utf8PrintfString("ThisNode.IsOfClass(\"%s\", \"%s\")", ecClass2->GetName().c_str(), ecClass2->GetSchema().GetName().c_str()));
    ex2->AddItem("constant_bool", "true");
    ex2->AddItem("constant_int", "1");
    ex2->AddItem("constant_double", "1.23");
    ex2->AddItem("constant_string", "\"Red\"");
    rules->AddPresentationRule(*ex2);

    // get content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // assert
    ASSERT_TRUE(content.IsValid());
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonAccessor extendedData1 = contentSet[0]->GetUsersExtendedData();
    ASSERT_TRUE(extendedData1.GetJson().IsObject());
    ASSERT_EQ(3, extendedData1.GetJson().MemberCount());

    ASSERT_TRUE(extendedData1.GetJson().HasMember("class_name"));
    EXPECT_STREQ(ecClass1->GetName().c_str(), extendedData1.GetJson()["class_name"].GetString());

    ASSERT_TRUE(extendedData1.GetJson().HasMember("property_value"));
    EXPECT_STREQ("test value", extendedData1.GetJson()["property_value"].GetString());

    ASSERT_TRUE(extendedData1.GetJson().HasMember("is_MyClass"));
    EXPECT_TRUE(extendedData1.GetJson()["is_MyClass"].GetBool());

    RapidJsonAccessor extendedData2 = contentSet[1]->GetUsersExtendedData();
    ASSERT_TRUE(extendedData2.GetJson().IsObject());
    ASSERT_EQ(7, extendedData2.GetJson().MemberCount());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("class_name"));
    EXPECT_STREQ(ecClass2->GetName().c_str(), extendedData2.GetJson()["class_name"].GetString());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("property_value"));
    EXPECT_TRUE(extendedData2.GetJson()["property_value"].IsNull());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("is_MyClass"));
    EXPECT_TRUE(extendedData2.GetJson()["is_MyClass"].GetBool());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("constant_bool"));
    EXPECT_TRUE(extendedData2.GetJson()["constant_bool"].GetBool());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("constant_int"));
    EXPECT_EQ(1, extendedData2.GetJson()["constant_int"].GetInt());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("constant_double"));
    EXPECT_DOUBLE_EQ(1.23, extendedData2.GetJson()["constant_double"].GetDouble());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("constant_string"));
    EXPECT_STREQ("Red", extendedData2.GetJson()["constant_string"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesAutoExpandedNestedFieldForRelatedProperties,
    R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesAutoExpandedNestedFieldForRelatedProperties)
    {
    // set up data set
    ECClassCP element = GetClass("Element");
    ECClassCP aspect = GetClass("Aspect");

    ECRelationshipClassCP elementHasAspect = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementHasAspect"));

    IECInstancePtr elementInst = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *element);
    IECInstancePtr aspectInst = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspect, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("test")); });

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspect, *elementInst, *aspectInst, nullptr, true);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", element->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(element->GetSchema().GetName(), element->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspect->GetFullName(), "", "", RelationshipMeaning::RelatedInstance, false, true));

    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());

    ASSERT_TRUE(fields[0]->IsNestedContentField());
    EXPECT_TRUE(fields[0]->AsNestedContentField()->ShouldAutoExpand());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#177537
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CategorizesNestedContentFields,
    R"*(
    <PropertyCategory typeName="GeometryAttributes" />
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CategorizedProp" typeName="double" category="GeometryAttributes" />
        <ECProperty propertyName="UncategorizedProp" typeName="double" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CategorizesNestedContentFields)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP elementHasAspectRel = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementHasAspect"));

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect, nullptr, true);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectClass->GetFullName(), "", RelationshipMeaning::RelatedInstance));

    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields[0]->IsNestedContentField());
    EXPECT_STREQ(aspectClass->GetName().c_str(), fields[0]->GetCategory().GetName().c_str());
    ASSERT_EQ(2, fields[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ(FIELD_NAME(aspectClass, "CategorizedProp"), fields[0]->AsNestedContentField()->GetFields()[0]->GetName().c_str());
    EXPECT_STREQ("GeometryAttributes", fields[0]->AsNestedContentField()->GetFields()[0]->GetCategory().GetName().c_str());
    EXPECT_STREQ(FIELD_NAME(aspectClass, "UncategorizedProp"), fields[0]->AsNestedContentField()->GetFields()[1]->GetName().c_str());
    EXPECT_STREQ("", fields[0]->AsNestedContentField()->GetFields()[1]->GetCategory().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotIncludeHiddenProperties,
    R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="RegularProperty" typeName="string" />
        <ECProperty propertyName="HiddenProperty" typeName="string">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00" />
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesNotIncludeHiddenProperties)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true));

    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields[0]->IsPropertiesField());
    ASSERT_EQ(1, fields[0]->AsPropertiesField()->GetProperties().size());
    EXPECT_EQ(elementClass->GetPropertyP("RegularProperty"), &fields[0]->AsPropertiesField()->GetProperties()[0].GetProperty());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#202530
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotIncludeHiddenRelatedClassPropertiesUnlessSpecificallyAskedFor,
    R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="HiddenAspect1">
        <BaseClass>Aspect</BaseClass>
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="HiddenAspect2">
        <BaseClass>Aspect</BaseClass>
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="HiddenAspect3">
        <BaseClass>Aspect</BaseClass>
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Prop3" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Aspect4">
        <BaseClass>Aspect</BaseClass>
        <ECProperty propertyName="Prop4" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesNotIncludeHiddenRelatedClassPropertiesUnlessSpecificallyAskedFor)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectBaseClass = GetClass("Aspect");
    ECClassCP aspectClass1 = GetClass("HiddenAspect1");
    ECClassCP aspectClass2 = GetClass("HiddenAspect2");
    ECClassCP aspectClass3 = GetClass("HiddenAspect3");
    ECClassCP aspectClass4 = GetClass("Aspect4");
    ECRelationshipClassCP elementHasAspectRel = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementHasAspect"));

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass1);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass2);
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass3);
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect1, nullptr);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect2, nullptr);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect3, nullptr);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect4, nullptr, true);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true));

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectBaseClass->GetFullName(), "", RelationshipMeaning::RelatedInstance, true));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectClass2->GetFullName(), "", RelationshipMeaning::RelatedInstance, true));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectClass3->GetFullName(), "", RelationshipMeaning::RelatedInstance, false));

    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(3, fields.size());

    // HiddenAspect1.Prop1 is _not_ included because it's hidden and we're including it's base class - not it specifically
    // HiddenAspect2.Prop2 is included because we have a rule that specifically requests it polymorphically
    EXPECT_TRUE(fields.end() != std::find_if(fields.begin(), fields.end(), [&](ContentDescriptor::Field const* f){return f->GetName().Equals(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass2));}));
    // HiddenAspect3.Prop3 is included because we have a rule that specifically requests it non-polymorphically
    EXPECT_TRUE(fields.end() != std::find_if(fields.begin(), fields.end(), [&](ContentDescriptor::Field const* f){return f->GetName().Equals(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass3));}));
    // Aspect4.Prop4 is included because it's not hidden
    EXPECT_TRUE(fields.end() != std::find_if(fields.begin(), fields.end(), [&](ContentDescriptor::Field const* f){return f->GetName().Equals(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass4));}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECEntityClassCP> CreateNDerivedClasses(ECSchemaR schema, ECEntityClassCR baseClass, int numberOfChildClasses)
    {
    bvector<ECEntityClassCP> classes;
    for (int i = 0; i < numberOfChildClasses; ++i)
        {
        ECEntityClassP ecClass = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(ecClass, Utf8PrintfString("Class%d", i + 1)));
        EXPECT_EQ(ECObjectsStatus::Success, ecClass->AddBaseClass(baseClass));
        classes.push_back(ecClass);
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#224831
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentForBaseClassWhenDerivedClassIsCustomizedAndThereAreLotsOfDerivedClasses)
    {
    // set up the schema
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, BeTest::GetNameOfCurrentTest(), Utf8PrintfString("alias_%s", BeTest::GetNameOfCurrentTest()), 0, 0, 0));
    schema->AddReferencedSchema(*const_cast<ECSchemaP>(s_project->GetECDb().Schemas().GetSchema("ECDbMap")));
    ECEntityClassP baseClass = nullptr;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(baseClass, "BaseClass"));
    ECClassCP classMapCustomAttributeClass = GetClass("ECDbMap", "ClassMap");
    IECInstancePtr classMapCustomAttribute = classMapCustomAttributeClass->GetDefaultStandaloneEnabler()->CreateInstance();
    classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
    ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*classMapCustomAttribute));
    PrimitiveECPropertyP prop = nullptr;
    ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(prop, "Label", PRIMITIVETYPE_String));
    bvector<ECEntityClassCP> derivedClasses = CreateNDerivedClasses(*schema, *baseClass, 1000);
    ASSERT_EQ(SUCCESS, s_project->GetECDb().Schemas().ImportSchemas({ schema.get() }));

    // set up data set
    for (ECEntityClassCP ecClass : derivedClasses)
        RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, derivedClasses[0]->GetFullName(), "Label"));

    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    ASSERT_EQ(2, descriptor->GetSelectClasses().size());
    EXPECT_EQ(baseClass->GetId(), descriptor->GetSelectClasses()[0].GetSelectClass().GetClass().GetId());
    EXPECT_EQ(derivedClasses[0]->GetId(), descriptor->GetSelectClasses()[1].GetSelectClass().GetClass().GetId());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());

    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(1000, content->GetContentSet().GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#257477
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentForRelatedBaseClassWhenThereAreLotsOfDerivedClasses)
    {
    // set up the schema
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, BeTest::GetNameOfCurrentTest(), Utf8PrintfString("alias_%s", BeTest::GetNameOfCurrentTest()), 0, 0, 0));
    schema->AddReferencedSchema(*const_cast<ECSchemaP>(s_project->GetECDb().Schemas().GetSchema("ECDbMap")));
    ECEntityClassP inputClass = nullptr;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(inputClass, "InputClass"));
    ECEntityClassP baseClass = nullptr;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(baseClass, "BaseClass"));
    ECClassCP classMapCustomAttributeClass = GetClass("ECDbMap", "ClassMap");
    IECInstancePtr classMapCustomAttribute = classMapCustomAttributeClass->GetDefaultStandaloneEnabler()->CreateInstance();
    classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
    ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*classMapCustomAttribute));
    PrimitiveECPropertyP prop = nullptr;
    ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(prop, "Label", PRIMITIVETYPE_String));
    bvector<ECEntityClassCP> derivedClasses = CreateNDerivedClasses(*schema, *baseClass, 1000);
    ECRelationshipClassP relationshipClass = nullptr;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateRelationshipClass(relationshipClass, "Relationship", *inputClass, "Source", *baseClass, "Target"));
    ASSERT_EQ(SUCCESS, s_project->GetECDb().Schemas().ImportSchemas({schema.get()}));

    // insert just one of the derived class instances
    IECInstancePtr inputInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *inputClass);
    IECInstancePtr relatedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClasses[0]);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *inputInstance, *relatedInstance);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, relationshipClass->GetFullName(), ""));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, derivedClasses[0]->GetFullName(), "Label"));

    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(*inputInstance), nullptr, options.GetJson()).get();

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(derivedClasses[0]->GetId(), descriptor->GetSelectClasses()[0].GetSelectClass().GetClass().GetId());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(1, content->GetContentSet().GetSize());
    }

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                04/2015
//=======================================================================================
struct RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests : RulesDrivenECPresentationManagerContentTests
    {
    TestPropertyFormatter m_propertyFormatter;
    virtual void _ConfigureManagerParams(RulesDrivenECPresentationManager::Params& params) override
        {
        RulesDrivenECPresentationManagerContentTests::_ConfigureManagerParams(params);
        params.SetECPropertyFormatter(&m_propertyFormatter);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, UsesSuppliedECPropertyFormatterToFormatPrimitiveECPropertyValue)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Test 1"));
        instance.SetValue("Description", ECValue("Test 2"));
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("BoolProperty", ECValue(true));
        instance.SetValue("DoubleProperty", ECValue(4.0));
        instance.SetValue("LongProperty", ECValue((int64_t)123));
        instance.SetValue("DateProperty", ECValue(DateTime(2017, 5, 30)));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification *specInstance = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*specInstance);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(7, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    ASSERT_TRUE(displayValues.IsObject());

    Utf8String fieldName = FIELD_NAME(m_widgetClass, "MyID");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_Test 1_", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(m_widgetClass, "Description");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_Test 2_", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(m_widgetClass, "IntProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_3_", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(m_widgetClass, "BoolProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_True_", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(m_widgetClass, "DoubleProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_4_", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(m_widgetClass, "LongProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_123_", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(m_widgetClass, "DateProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_2017-05-30T00:00:00.000Z_", displayValues[fieldName.c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesSuppliedECPropertyFormatterToFormatNestedContentValue, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect">
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, UsesSuppliedECPropertyFormatterToFormatNestedContentValue)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("ElementUniqueAspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("StringProperty", ECValue("Test"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "",
        elementClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        rel->GetFullName(), aspectClass->GetFullName(), "", RelationshipMeaning::SameInstance));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "Test"
                },
            "DisplayValues": {
                "%s": "_Test_"
                },
            "MergedFieldNames": []
            }]
        })", NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "StringProperty"), FIELD_NAME(aspectClass, "StringProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, UsesSuppliedECPropertyFormatterToFormatPropertyLabels)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification *specInstance = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*specInstance);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(7, descriptor->GetVisibleFields().size());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    EXPECT_STREQ("_Description_", fields[0]->GetLabel().c_str());
    EXPECT_STREQ("_MyID_", fields[1]->GetLabel().c_str());
    EXPECT_STREQ("_IntProperty_", fields[2]->GetLabel().c_str());
    EXPECT_STREQ("_BoolProperty_", fields[3]->GetLabel().c_str());
    EXPECT_STREQ("_DoubleProperty_", fields[4]->GetLabel().c_str());
    EXPECT_STREQ("_LongProperty_", fields[5]->GetLabel().c_str());
    EXPECT_STREQ("_DateProperty_", fields[6]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FormatsPrimitiveArrayPropertyValues, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, FormatsPrimitiveArrayPropertyValues)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        ECValue nullECValue; nullECValue.SetIsNull(true);
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        instance.SetValue("ArrayProperty", nullECValue, 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"(
        {
        "%s": ["_2_", "_1_", null]
        })", FIELD_NAME(ecClass, "ArrayProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FormatsStructPropertyValues, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="DoubleProperty" typeName="double" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, FormatsStructPropertyValues)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        ECValue nullECValue; nullECValue.SetIsNull(true);
        instance.SetValue("StructProperty.DoubleProperty", nullECValue);
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "%s": {
           "DoubleProperty": null,
           "IntProperty": "_123_",
           "StringProperty": "_abc_"
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FormatsNestedStructPropertyValues, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="DoubleProperty" typeName="double" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, FormatsNestedStructPropertyValues)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("ElementUniqueAspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        ECValue nullECValue; nullECValue.SetIsNull(true);
        instance.SetValue("StructProperty.DoubleProperty", nullECValue);
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        rel->GetFullName(), aspectClass->GetFullName(), "", RelationshipMeaning::SameInstance));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = m_manager->GetContent(*modifiedDescriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": {
                    "DoubleProperty": null,
                    "IntProperty": 123,
                    "StringProperty": "abc"
                    }
                },
            "DisplayValues": {
                "%s": {
                    "DoubleProperty": null,
                    "IntProperty": "_123_",
                    "StringProperty": "_abc_"
                    }
                },
            "MergedFieldNames": []
            }]
        })", NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "StructProperty"), FIELD_NAME(aspectClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                04/2015
//=======================================================================================
struct RulesDrivenECPresentationManagerContentWithCustomCategorySupplierTests : RulesDrivenECPresentationManagerContentTests
    {
    TestCategorySupplier m_categorySupplier = TestCategorySupplier(ContentDescriptor::Category("CustomName", "Custom label", "Custom description", 0));

    virtual void _ConfigureManagerParams(RulesDrivenECPresentationManager::Params& params) override
        {
        RulesDrivenECPresentationManagerContentTests::_ConfigureManagerParams(params);
        params.SetCategorySupplier(&m_categorySupplier);
        }
    };

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentWithCustomCategorySupplierTests, UsesCustomPropertyCategorySupplierIfSet)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // make sure all fields have custom category
    ContentDescriptor::Category const& category1 = descriptor->GetVisibleFields()[0]->GetCategory();
    EXPECT_STREQ("Custom label", category1.GetLabel().c_str());
    EXPECT_STREQ("CustomName", category1.GetName().c_str());
    EXPECT_STREQ("Custom description", category1.GetDescription().c_str());

    ContentDescriptor::Category const& category2 = descriptor->GetVisibleFields()[1]->GetCategory();
    EXPECT_STREQ("Custom label", category2.GetLabel().c_str());
    EXPECT_STREQ("CustomName", category2.GetName().c_str());
    EXPECT_STREQ("Custom description", category2.GetDescription().c_str());

    ContentDescriptor::Category const& category3 = descriptor->GetVisibleFields()[2]->GetCategory();
    EXPECT_STREQ("Custom label", category3.GetLabel().c_str());
    EXPECT_STREQ("CustomName", category3.GetName().c_str());
    EXPECT_STREQ("Custom description", category3.GetDescription().c_str());
    }
