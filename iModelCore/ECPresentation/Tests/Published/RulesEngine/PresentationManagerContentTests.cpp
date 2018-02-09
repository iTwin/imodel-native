/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RulesEngine/PresentationManagerContentTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationManagerTests.h"
#include "../../../Source/RulesDriven/RulesEngine/LocalizationHelper.h"
#include "../../../Localization/Xliffs/ECPresentation.xliff.h"
#include "../../NonPublished/RulesEngine/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                04/2015
//=======================================================================================
struct RulesDrivenECPresentationManagerContentTests : RulesDrivenECPresentationManagerTests
{
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;
    ECSchemaCP m_schema;
    
    void SetUp() override
        {
        RulesDrivenECPresentationManagerTests::SetUp();

        m_schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
        ASSERT_TRUE(nullptr != m_schema);
        
        m_widgetClass = m_schema->GetClassCP("Widget");
        m_gadgetClass = m_schema->GetClassCP("Gadget");
        m_sprocketClass = m_schema->GetClassCP("Sprocket");
        }
};

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ReturnsValidDescriptorBasedOnSelectedClasses)
    {    
    // set up selection
    SelectionInfo selection({m_gadgetClass, m_widgetClass});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(2, descriptor->GetSelectClasses().size());

    EXPECT_EQ(m_gadgetClass, &descriptor->GetSelectClasses()[0].GetSelectClass());
    EXPECT_FALSE(descriptor->GetSelectClasses()[0].IsSelectPolymorphic());

    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[1].GetSelectClass());
    EXPECT_FALSE(descriptor->GetSelectClasses()[1].IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AllPropertiesOfOneSelectedNode)
    {
    // insert some widget instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance1);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    
    NavNodePtr widgetNode = TestNodesHelper::CreateInstanceNode(*widgetInstance);
    NavNodePtr gadgetNode = TestNodesHelper::CreateInstanceNode(*gadgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&widgetNode->GetKey());
    keys.push_back(&gadgetNode->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "Widget", false));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*widgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "WrongSchemaName", "", false));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
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
    
    NavNodePtr nodeE = TestNodesHelper::CreateInstanceNode(*classEInstance);
    NavNodePtr nodeF = TestNodesHelper::CreateInstanceNode(*classFInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&nodeE->GetKey());
    keys.push_back(&nodeF->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    
    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "ClassE", true));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("SelectedNodeInstances_AcceptablePolymorphically");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*instance2);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node1->GetKey());
    keys.push_back(&node2->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfTheSameClass");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*instance2);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node1->GetKey());
    keys.push_back(&node2->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfDifferentClasses");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*widget);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*gadget);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node1->GetKey());
    keys.push_back(&node2->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
    
    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet({widget.get(), gadget.get()}, *content, false);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetSortingField("Widget_Gadget_MyID");

    // get the content with descriptor override
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *ovr, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted
    RulesEngineTestHelpers::ValidateContentSet({gadget.get(), widget.get()}, *content, true);
    
    // change the order from ascending to descending
    ovr->SetSortDirection(SortDirection::Descending);

    // get the content with the changed sorting order
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *ovr, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted in descending order
    RulesEngineTestHelpers::ValidateContentSet({widget.get(), gadget.get()}, *content, true);
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetSortingField("ClassQ_IntEnum");

    // get the content with descriptor override
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *ovr, selection, PageOptions(), options.GetJson()).get();
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
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(7, descriptor->GetVisibleFields().size());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->RemoveField("Widget_IntProperty");

    // get the content with descriptor override
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *ovr, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the IntProperty field has been removed
    EXPECT_EQ(6, content->GetDescriptor().GetVisibleFields().size());
    EXPECT_TRUE(content->GetDescriptor().GetAllFields().end() == std::find_if(content->GetDescriptor().GetAllFields().begin(), content->GetDescriptor().GetAllFields().end(),
        [](ContentDescriptor::Field const* field){return field->GetName().Equals("Widget_IntProperty");}));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->RemoveField("Sprocket_Gadget");

    // get the content with descriptor override
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *ovr, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the Gadget field has been removed
    EXPECT_EQ(2, content->GetDescriptor().GetVisibleFields().size());

    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ContentSetItemCPtr record = content->GetContentSet().Get(0);
    rapidjson::Document json = record->AsJson();
    EXPECT_FALSE(json["Values"].HasMember("Sprocket_Gadget"));
    EXPECT_FALSE(json["DisplayValues"].HasMember("Sprocket_Gadget"));
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
        
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*instance2);
    NavNodePtr node3 = TestNodesHelper::CreateInstanceNode(*instance3);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node1->GetKey());
    keys.push_back(&node2->GetKey());
    keys.push_back(&node3->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options("DescriptorOverride_WithFilters");
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
    
    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet({instance1.get(), instance2.get(), instance3.get()}, *content);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFilterExpression("Widget_IntProperty > 1 or Widget_DoubleProperty < 0");

    // get the content with descriptor override
    content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *ovr, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    RulesEngineTestHelpers::ValidateContentSet({instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_ReturnsValidDescriptorWhichDoesNotDependOnSelectedClasses)
    {    
    // set up selection
    SelectionInfo selection({m_gadgetClass});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[0].GetSelectClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[0].IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_ClassNames_ReturnsInstanceOfDefinedClass)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(NavNodeKeyList()));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentInstancesOfSpecificClasses_ClassNames_ReturnsInstanceOfDefinedClass");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(NavNodeKeyList()));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "this.IntProperty=2", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentInstancesOfSpecificClasses_InstanceFilter");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(NavNodeKeyList()));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentInstancesOfSpecificClasses_ArePolymorphic");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
    
    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({classEInstance.get(), classFInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsValidDescriptorBasedOnSelectedClasses)
    {    
    ECEntityClassCP sprocketClass = GetClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
    ECRelationshipClassCP rel1 = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCP rel3 = GetClass("RulesEngineTest", "WidgetsHaveGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP rel4 = GetClass("RulesEngineTest", "WidgetsHaveGadgets2")->GetRelationshipClassCP();
    ECRelationshipClassCP rel5 = GetClass("RulesEngineTest", "GadgetHasSprocket")->GetRelationshipClassCP();
    ECRelationshipClassCP rel6 = GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    // set up selection
    SelectionInfo selection({m_gadgetClass});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "", ""));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(6, descriptor->GetSelectClasses().size());

    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[0].GetSelectClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[0].IsSelectPolymorphic());
    EXPECT_EQ(m_gadgetClass, descriptor->GetSelectClasses()[0].GetPrimaryClass());
    EXPECT_EQ(1, descriptor->GetSelectClasses()[0].GetPathToPrimaryClass().size());
    EXPECT_EQ(rel1, descriptor->GetSelectClasses()[0].GetPathToPrimaryClass()[0].GetRelationship());

    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[1].GetSelectClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[1].IsSelectPolymorphic());
    EXPECT_EQ(m_gadgetClass, descriptor->GetSelectClasses()[1].GetPrimaryClass());
    EXPECT_EQ(1, descriptor->GetSelectClasses()[1].GetPathToPrimaryClass().size());
    EXPECT_EQ(rel2, descriptor->GetSelectClasses()[1].GetPathToPrimaryClass()[0].GetRelationship());

    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[2].GetSelectClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[2].IsSelectPolymorphic());
    EXPECT_EQ(m_gadgetClass, descriptor->GetSelectClasses()[2].GetPrimaryClass());
    EXPECT_EQ(1, descriptor->GetSelectClasses()[2].GetPathToPrimaryClass().size());
    EXPECT_EQ(rel3, descriptor->GetSelectClasses()[2].GetPathToPrimaryClass()[0].GetRelationship());

    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[3].GetSelectClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[3].IsSelectPolymorphic());
    EXPECT_EQ(m_gadgetClass, descriptor->GetSelectClasses()[3].GetPrimaryClass());
    EXPECT_EQ(1, descriptor->GetSelectClasses()[3].GetPathToPrimaryClass().size());
    EXPECT_EQ(rel4, descriptor->GetSelectClasses()[3].GetPathToPrimaryClass()[0].GetRelationship());

    EXPECT_EQ(sprocketClass, &descriptor->GetSelectClasses()[4].GetSelectClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[4].IsSelectPolymorphic());
    EXPECT_EQ(m_gadgetClass, descriptor->GetSelectClasses()[4].GetPrimaryClass());
    EXPECT_EQ(1, descriptor->GetSelectClasses()[4].GetPathToPrimaryClass().size());
    EXPECT_EQ(rel5, descriptor->GetSelectClasses()[4].GetPathToPrimaryClass()[0].GetRelationship());

    EXPECT_EQ(sprocketClass, &descriptor->GetSelectClasses()[5].GetSelectClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[5].IsSelectPolymorphic());
    EXPECT_EQ(m_gadgetClass, descriptor->GetSelectClasses()[5].GetPrimaryClass());
    EXPECT_EQ(1, descriptor->GetSelectClasses()[5].GetPathToPrimaryClass().size());
    EXPECT_EQ(rel6, descriptor->GetSelectClasses()[5].GetPathToPrimaryClass()[0].GetRelationship());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance)
    {
    // insert some widget & gadget instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCR relationshipWidgetHasGadgets = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadgetInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("CustomID")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadgets, *widgetInstance, *gadgetInstance2);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*widgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
    
    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({gadgetInstance1.get(), gadgetInstance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance_BackwardsDirection)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*gadgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, "", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance_BackwardsDirection");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({widgetInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsRelatedInstancesPolymorphically)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipClassAHasBAndC = *m_schema->GetClassCP("ClassAHasBAndC")->GetRelationshipClassCP();
    ECEntityClassCP classA = GetClass("RulesEngineTest", "ClassA")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("RulesEngineTest", "ClassB")->GetEntityClassCP();
    ECEntityClassCP classC = GetClass("RulesEngineTest", "ClassC")->GetEntityClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("A")); });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("B")); });
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("C")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipClassAHasBAndC, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipClassAHasBAndC, *instanceA, *instanceC);
    
    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instanceA));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, "RulesEngineTest:ClassAHasBAndC", "RulesEngineTest:BaseOfBAndC"));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    BeInt64Id instanceAId = ECInstanceId::FromString(instanceA->GetInstanceId().c_str());

    rapidjson::Document item1 = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = item1["Values"];
    EXPECT_STREQ("B", values1[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    EXPECT_EQ(instanceAId.GetValueUnchecked(), values1[descriptor->GetVisibleFields()[1]->GetName().c_str()].GetInt64());

    rapidjson::Document item2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR values2 = item2["Values"];
    EXPECT_STREQ("C", values2[descriptor->GetVisibleFields()[0]->GetName().c_str()].GetString());
    EXPECT_EQ(instanceAId.GetValueUnchecked(), values2[descriptor->GetVisibleFields()[1]->GetName().c_str()].GetInt64());
    }


/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_RelationshipClassNames_ReturnsInvalidContentWhenRelationshipDoesNotExist)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*widgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "RulesEngineTest:DoesNotExist", ""));
    rules->AddPresentationRule(*rule);

    // note: query builder asserts when the class / relationships is not found - ignore that
    IGNORE_BE_ASSERT();
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentRelatedInstances_RelationshipClassNames_ReturnsInvalidContentWhenRelationshipDoesNotExist");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_FALSE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_RelationshipClassNames)
    {
    // insert some widget & gadget instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCR relationshipWidgetHasGadgets = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadgetInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadgets, *widgetInstance, *gadgetInstance2);
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*widgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "RulesEngineTest:WidgetHasGadget", ""));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentRelatedInstances_RelationshipClassNames");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
        
    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({gadgetInstance1.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_RelationshipClassNames_BackwardsDirection)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*gadgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadget", ""));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentRelatedInstances_RelationshipClassNames_BackwardsDirection");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({widgetInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_InstanceFilter)
    {
    // insert some widget & gadget instances
    ECRelationshipClassCR relationshipWidgetHasGadgets = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Gadget1"));});
    IECInstancePtr gadgetInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Gadget2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadgets, *widgetInstance, *gadgetInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadgets, *widgetInstance, *gadgetInstance2);
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*widgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "this.Description=\"Gadget2\"", RequiredRelationDirection_Both, "", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options("ContentRelatedInstances_InstanceFilter");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({gadgetInstance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_Recursive)
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

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance2);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Both, "", "RulesEngineTest:ClassN"));
    rules->AddPresentationRule(*rule);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({instance1.get(),instance3.get(),instance4.get(),instance5.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_RecursiveWithMultipleRelationships)
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

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance1);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({instance2.get(),instance3.get(),instance4.get(),instance5.get(),instance6.get(),instance7.get(),instance8.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis               11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_RecursiveWithMultipleSelectClasses, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_RecursiveWithMultipleSelectClasses)
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

    // set up selection
    NavNodeKeyList keys = {ECInstanceNodeKey::Create(*parentA), ECInstanceNodeKey::Create(*parentB)};
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
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
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    ECEntityClassCP classH = GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance) { instance.SetValue("PropertyF", ECValue(1000));});
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) { instance.SetValue("PropertyF", ECValue(2000));});

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instanceF));
    keys.push_back(ECInstanceNodeKey::Create(*instanceH));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(7, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("1000", jsonValues["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues["ClassF_ClassH_IntProperty"].IsNull());
    EXPECT_EQ(1000, jsonValues["ClassF_ClassH_PropertyF"].GetInt());
    EXPECT_TRUE(jsonValues["ClassF_ClassH_LongProperty"].IsNull());
    EXPECT_TRUE(jsonValues["ClassH_PointProperty"].IsNull());
    EXPECT_TRUE(jsonValues["ClassH_Point2dProperty"].IsNull());
    EXPECT_TRUE(jsonValues["ClassF_ClassH_ClassD"].IsNull());

    rapidjson::Document jsonDoc1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues1 = jsonDoc1["Values"];
    EXPECT_STREQ("2000", jsonValues1["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues1["ClassF_ClassH_IntProperty"].IsNull());
    EXPECT_EQ(2000, jsonValues1["ClassF_ClassH_PropertyF"].GetInt());
    EXPECT_TRUE(jsonValues1["ClassF_ClassH_LongProperty"].IsNull());
    EXPECT_FALSE(jsonValues1["ClassH_PointProperty"].IsNull());
    EXPECT_TRUE(jsonValues["ClassH_Point2dProperty"].IsNull());
    EXPECT_TRUE(jsonValues1["ClassF_ClassH_ClassD"].IsNull());
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
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(&node->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);
        
    // request for content
    RulesDrivenECPresentationManager::ContentOptions options("ContentSerialization");
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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

        if (name == "Widget_Description")
            {
            EXPECT_STREQ("MyDescription", value[name.c_str()].GetString());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("string", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name == "Widget_MyID")
            {
            EXPECT_STREQ("MyID", value[name.c_str()].GetString());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("string", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name == "Widget_IntProperty")
            {
            EXPECT_EQ(9, value[name.c_str()].GetInt());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("int", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name == "Widget_BoolProperty")
            {
            EXPECT_TRUE(value[name.c_str()].GetBool());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("boolean", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name == "Widget_DoubleProperty")
            {
            EXPECT_DOUBLE_EQ(7.0, value[name.c_str()].GetDouble());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("double", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name == "Widget_LongProperty")
            {
            EXPECT_EQ(123, value[name.c_str()].GetInt64());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("long", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name == "Widget_DateProperty")
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
// @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesCustomPropertyCategorySupplierIfSet)
    {    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);
    
    // override the category supplier
    TestCategorySupplier categorySupplier(ContentDescriptor::Category("CustomName", "Custom label", "Custom description", 0));
    m_manager->SetCategorySupplier(&categorySupplier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options("UsesCustomPropertyCategorySupplierIfSet");

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson()).get();
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

    // set up selection
    NavNodePtr gadgetNode = TestNodesHelper::CreateInstanceNode(*gadget);
    NavNodePtr widgetNode = TestNodesHelper::CreateInstanceNode(*widget2);
    NavNodeKeyList keys;
    keys.push_back(&gadgetNode->GetKey());
    keys.push_back(&widgetNode->GetKey());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Description,MyID", 1000, true));
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(3, descriptor->GetVisibleFields().size()); // Gadget.Description, related Widget.MyID, Widget.MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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

    ECInstanceId gadgetId, widgetId;
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceKey gadgetKey(m_gadgetClass->GetId(), gadgetId);
    ECInstanceKey widgetKey(m_widgetClass->GetId(), widgetId);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(5, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID, Widget.IntProperty

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    
    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp0(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);
    
    ContentSetItem::FieldProperty fp1(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp2(*descriptor->GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);
    
    ContentSetItem::FieldProperty fp3(*descriptor->GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
    ASSERT_EQ(1, widgetMyIdFieldInstanceKeys.size());
    EXPECT_EQ(widgetKey, widgetMyIdFieldInstanceKeys[0]);
    
    ContentSetItem::FieldProperty fp4(*descriptor->GetVisibleFields()[4]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> widgetIntpropertyFieldInstanceKeys = record->GetPropertyValueKeys(fp4);
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
    ECInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);
    ECInstanceKey sprocketKey = RulesEngineTestHelpers::GetInstanceKey(*sprocket);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID + Sprocket.MyID, Gadget.Description + Sprocket.MyID, Gadget.Widget, Sprocket.Gadget

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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

    bvector<ECInstanceKey> gadgetMyidFieldInstanceKeys = record0->GetPropertyValueKeys(fp00);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);
    
    bvector<ECInstanceKey> sprocketMyidFieldInstanceKeys = record0->GetPropertyValueKeys(fp01);
    EXPECT_TRUE(sprocketMyidFieldInstanceKeys.empty());
    
    bvector<ECInstanceKey> gadgetDescriptionFieldInstanceKeys = record0->GetPropertyValueKeys(fp10);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);
    
    bvector<ECInstanceKey> sprocketDescriptionFieldInstanceKeys = record0->GetPropertyValueKeys(fp11);
    EXPECT_TRUE(sprocketDescriptionFieldInstanceKeys.empty());

    bvector<ECInstanceKey> gadgetWidgetFieldInstanceKeys = record0->GetPropertyValueKeys(fp20);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    bvector<ECInstanceKey> sprocketGadgetFieldInstanceKeys = record0->GetPropertyValueKeys(fp30);
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
    ECInstanceKey gadgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*gadget1);
    ECInstanceKey gadgetKey2 = RulesEngineTestHelpers::GetInstanceKey(*gadget2);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    
    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp0(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(2, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetMyidFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetMyidFieldInstanceKeys[1]);
    
    ContentSetItem::FieldProperty fp1(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(2, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetDescriptionFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetDescriptionFieldInstanceKeys[1]);

    ContentSetItem::FieldProperty fp2(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
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
    ECInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);
    ECInstanceKey sprocketKey = RulesEngineTestHelpers::GetInstanceKey(*sprocket);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID + Sprocket.MyID, Gadget.Description + Sprocket.MyID, Gadget.Widget, Sprocket.Gadget
    
    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    
    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp00(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp00);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);
    
    ContentSetItem::FieldProperty fp01(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 1);
    bvector<ECInstanceKey> sprocketMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp01);
    ASSERT_EQ(1, sprocketMyidFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketMyidFieldInstanceKeys[0]);
    
    ContentSetItem::FieldProperty fp10(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp10);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);
    
    ContentSetItem::FieldProperty fp11(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 1);
    bvector<ECInstanceKey> sprocketDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp11);
    ASSERT_EQ(1, sprocketDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp20(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp20);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp30(*content->GetDescriptor().GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> sprocketGadgetFieldInstanceKeys = record->GetPropertyValueKeys(fp30);
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
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetInstanceId(), rel));
        });
    
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with Sprocket MyID and Description), Sprocket.Gadget, Gadget.MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    
    ContentSetItem::FieldProperty fp(*descriptor->GetVisibleFields()[8]->AsPropertiesField(), 0);

    ContentSetItemCPtr widgetRecord = contentSet.Get(0);
    bvector<ECInstanceKey> widgetKeys = widgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(widgetKeys.empty());
    EXPECT_TRUE(widgetRecord->AsJson()["Values"][fp.GetField().GetName().c_str()].IsNull());
    
    ContentSetItemCPtr sprocketRecord = contentSet.Get(1);
    bvector<ECInstanceKey> sprocketKeys = sprocketRecord->GetPropertyValueKeys(fp);
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
    
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with Sprocket MyID and Description), Gadget.MyID, Sprocket.Gadget
    
    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItem::FieldProperty fp(*content->GetDescriptor().GetVisibleFields()[7]->AsPropertiesField(), 0);
    ContentSetItemCPtr record = contentSet.Get(0);
    bvector<ECInstanceKey> keys = record->GetPropertyValueKeys(fp);
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
    
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(10, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with MyID and Description of Gadget and Sprocket), Gadget.Widget, Sprocket.Gadget, Gadget.MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());
    
    ContentSetItem::FieldProperty fp(*descriptor->GetVisibleFields()[9]->AsPropertiesField(), 0);

    ContentSetItemCPtr widgetRecord = contentSet.Get(0);
    bvector<ECInstanceKey> widgetKeys = widgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(widgetKeys.empty());
    
    ContentSetItemCPtr gadgetRecord = contentSet.Get(1);
    bvector<ECInstanceKey> gadgetKeys = gadgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(gadgetKeys.empty());

    ContentSetItemCPtr sprocketRecord = contentSet.Get(2);
    bvector<ECInstanceKey> sprocketKeys = sprocketRecord->GetPropertyValueKeys(fp);
    ASSERT_EQ(1, sprocketKeys.size());
    EXPECT_EQ(ECInstanceKey(m_gadgetClass->GetId(), ECInstanceId()), sprocketKeys[0]);
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::PropertyPane, 
        selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // Element.ElementProperty + <BaseRelatedClass.RelatedProperty, DerivedRelatedClass.RelatedProperty>

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    
    ContentSetItem::FieldProperty fp(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
        
    ContentSetItemCPtr record = contentSet.Get(0);
    bvector<ECInstanceKey> keys = record->GetPropertyValueKeys(fp);
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
    ECInstanceId gadgetId;
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());
    ECInstanceKey gadgetKey(m_gadgetClass->GetId(), gadgetId);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    
    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp0(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);
    
    ContentSetItem::FieldProperty fp1(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldProperty fp2(*descriptor->GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);
    
    ContentSetItem::FieldProperty fp3(*descriptor->GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
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

    ECInstanceId gadgetId1;
    ECInstanceId::FromString(gadgetId1, gadget1->GetInstanceId().c_str());
    ECInstanceKey gadgetKey1(m_gadgetClass->GetId(), gadgetId1);
    
    ECInstanceId gadgetId2;
    ECInstanceId::FromString(gadgetId2, gadget2->GetInstanceId().c_str());
    ECInstanceKey gadgetKey2(m_gadgetClass->GetId(), gadgetId2);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), 
        nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    
    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldProperty fp0(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(2, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetMyidFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetMyidFieldInstanceKeys[1]);
    
    ContentSetItem::FieldProperty fp1(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(2, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetDescriptionFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetDescriptionFieldInstanceKeys[1]);

    ContentSetItem::FieldProperty fp2(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(2, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetWidgetFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetWidgetFieldInstanceKeys[1]);
    
    ContentSetItem::FieldProperty fp3(*content->GetDescriptor().GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_SetsDisplayLabelPropertyInstanceLabelOverride)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_SetsDisplayLabelPropertyInstanceLabelOverride", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabel().c_str());
    }

TEST_F(RulesDrivenECPresentationManagerContentTests, SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreEqual)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabel().c_str());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreDifferent)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label 1"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Custom label 2"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_MultipleInstances()).c_str(), contentSet.Get(0)->GetDisplayLabel().c_str());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_MultipleInstances()).c_str(), contentSet.Get(0)->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsDisplayLabelPropertyWhenMergingRecordsAndClassesAreDifferent)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_MultipleInstances()).c_str(), contentSet.Get(0)->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsClassWhenMergingRecordsAndClassesAreEqual)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
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
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("PointProperty", 1000, true));
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // selection
    SelectionInfo selection("Test", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    
    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values = recordJson["Values"];
    RapidJsonValueCR pointValue = values[descriptor->GetVisibleFields()[0]->GetName().c_str()];
    EXPECT_TRUE(pointValue.IsObject());
    EXPECT_DOUBLE_EQ(1.0, pointValue["x"].GetDouble());
    EXPECT_DOUBLE_EQ(2.0, pointValue["y"].GetDouble());
    EXPECT_DOUBLE_EQ(3.0, pointValue["z"].GetDouble());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, DisplayLabelFieldsGetCreatedForRecordsfromDifferentSpecifications)
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); // no display label in the descriptor

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(9, content->GetDescriptor().GetVisibleFields().size()); // content created with a descriptor that has a display label

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("Test Widget", values1[content->GetDescriptor().GetVisibleFields()[0]->GetName().c_str()].GetString());

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR values2 = recordJson2["Values"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadget).c_str(), values2[content->GetDescriptor().GetVisibleFields()[0]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_DisplayLabelFieldsGetCreatedForRecordsFromDifferentSpecifications)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Widget"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_DisplayLabelFieldsGetCreatedForRecordsFromDifferentSpecifications", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); // no display label in the descriptor

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(9, content->GetDescriptor().GetVisibleFields().size()); // content created with a descriptor that has a display label

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("Test Widget", values1[content->GetDescriptor().GetVisibleFields()[0]->GetName().c_str()].GetString());


    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR values2 = recordJson2["Values"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadget).c_str(), values2[content->GetDescriptor().GetVisibleFields()[0]->GetName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesSuppliedECPropertyFormatterToFormatPrimitiveECPropertyValue)
    {
    TestPropertyFormatter formatter;
    m_manager->SetECPropertyFormatter(&formatter);
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

    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(7, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, SelectionInfo(), PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    ASSERT_TRUE(displayValues.IsObject());
   
    Utf8CP fieldName = "Widget_MyID";
    ASSERT_TRUE(displayValues.HasMember(fieldName));
    ASSERT_TRUE(displayValues[fieldName].IsString());
    EXPECT_STREQ("_Test 1_", displayValues[fieldName].GetString());

    fieldName = "Widget_Description";
    ASSERT_TRUE(displayValues.HasMember(fieldName));
    ASSERT_TRUE(displayValues[fieldName].IsString());
    EXPECT_STREQ("_Test 2_", displayValues[fieldName].GetString());

    fieldName = "Widget_IntProperty";
    ASSERT_TRUE(displayValues.HasMember(fieldName));
    ASSERT_TRUE(displayValues[fieldName].IsString());
    EXPECT_STREQ("_3_", displayValues[fieldName].GetString());

    fieldName = "Widget_BoolProperty";
    ASSERT_TRUE(displayValues.HasMember(fieldName));
    ASSERT_TRUE(displayValues[fieldName].IsString());
    EXPECT_STREQ("_True_", displayValues[fieldName].GetString());

    fieldName = "Widget_DoubleProperty";
    ASSERT_TRUE(displayValues.HasMember(fieldName));
    ASSERT_TRUE(displayValues[fieldName].IsString());
    EXPECT_STREQ("_4_", displayValues[fieldName].GetString());
    
    fieldName = "Widget_LongProperty";
    ASSERT_TRUE(displayValues.HasMember(fieldName));
    ASSERT_TRUE(displayValues[fieldName].IsString());
    EXPECT_STREQ("_123_", displayValues[fieldName].GetString());
    
    fieldName = "Widget_DateProperty";
    ASSERT_TRUE(displayValues.HasMember(fieldName));
    ASSERT_TRUE(displayValues[fieldName].IsString());
    EXPECT_STREQ("_2017-05-30T00:00:00.000Z_", displayValues[fieldName].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesSuppliedECPropertyFormatterToFormatPropertyLabels)
    {
    TestPropertyFormatter formatter;
    m_manager->SetECPropertyFormatter(&formatter);

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, SelectionInfo(), options.GetJson()).get();
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
* @bsitest                                      Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsCachedWhenParametersEqual)
    {
    // set up selection
    SelectionInfo selection("aaa", true, *NavNodeKeyListContainer::Create({
        ECInstanceNodeKey::Create(m_widgetClass->GetId(), ECInstanceId((uint64_t)1)),
        ECInstanceNodeKey::Create(m_widgetClass->GetId(), ECInstanceId((uint64_t)2))
        }));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();

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
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(project2.GetECDb(), nullptr, selection, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_ContentDisplayType)
    {
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Graphics, selection, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::Grid, selection, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_Provider)
    {
    // set up selection 1
    SelectionInfo selection1("A", false, *NavNodeKeyListContainer::Create());
    
    // set up selection 2
    SelectionInfo selection2("B", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection1, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection2, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_SubSelection)
    {
    // set up selection 1
    SelectionInfo selection1("", false, *NavNodeKeyListContainer::Create());
    
    // set up selection 2
    SelectionInfo selection2("", true, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection1, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection2, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_Keys)
    {
    // set up selection 1
    SelectionInfo selection1("", false, *NavNodeKeyListContainer::Create({
        ECInstanceNodeKey::Create(m_widgetClass->GetId(), ECInstanceId((uint64_t)1)),
        ECInstanceNodeKey::Create(m_widgetClass->GetId(), ECInstanceId((uint64_t)2))
        }));
    
    // set up selection 2
    SelectionInfo selection2("", false, *NavNodeKeyListContainer::Create({
        ECInstanceNodeKey::Create(m_widgetClass->GetId(), ECInstanceId((uint64_t)3)),
        ECInstanceNodeKey::Create(m_widgetClass->GetId(), ECInstanceId((uint64_t)4))
        }));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection1, options.GetJson()).get();
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection2, options.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_RulesetId)
    {
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options1.GetJson()).get();
    RulesDrivenECPresentationManager::ContentOptions options2(rules2->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options2.GetJson()).get();

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsRemovedFromCacheAfterConnectionClose)
    {
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();

    // simulate re-opening
    IConnectionCP connection = m_connections.GetConnection(s_project->GetECDb());
    m_connections.NotifyConnectionClosed(*connection);
    m_connections.NotifyConnectionOpened(s_project->GetECDb());

    // request again
    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
        
    // verify the two objects aren't equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentModifierAppliesHiddenPropertiesSpecification)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) { 
        instance.SetValue("Description", ECValue("TestDescription")); 
        instance.SetValue("MyID", ECValue("TestID"));
        });

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Gadget");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Description,Widget", 1000, false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Gadget.MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); //Gadget_Widget_MyId, Gadget_Widget_Description, Gadget_Widget, Widget_IntProperty, Widget_BoolProperty, Widget_DoubleProperty, Widget_LongProperty, Widget_Date, CalculatedProperty_0

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    ECEntityClassCP classB = GetClass("RulesEngineTest", "ClassB")->GetEntityClassCP();
    ECEntityClassCP classC = GetClass("RulesEngineTest", "ClassC")->GetEntityClassCP();
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("B")); });
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("C")); });
    
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); //ShouldBe:BaseOfBAndC_MyId, CalculatedProperty_0 Is:ClassB_ClassC_MyId, ClassB_ClassC_A, CalculatedProperty_0

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("B", jsonValues["ClassB_ClassC_MyID"].GetString());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues["ClassB_ClassC_A"].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_STREQ("C", jsonValues2["ClassB_ClassC_MyID"].GetString());
    EXPECT_STREQ("C", jsonValues2["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues["ClassB_ClassC_A"].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_ContentModifierIsAppliedToOnlyOneChildClassPolimorphically)
    {
    // set up data set
    ECEntityClassCP classB = GetClass("RulesEngineTest", "ClassB")->GetEntityClassCP();
    ECEntityClassCP classC = GetClass("RulesEngineTest", "ClassC")->GetEntityClassCP();
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, 
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("B")); });
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, 
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("C")); });
    
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); //BaseOfBAndC_MyId, ClassB_ClassC_A, CalculatedProperty_0

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("B", jsonValues["BaseOfBAndC_ClassB_ClassC_MyID"].GetString());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues["ClassB_ClassC_A"].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_STREQ("C", jsonValues2["BaseOfBAndC_ClassB_ClassC_MyID"].GetString());
    EXPECT_STREQ("C", jsonValues2["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues2["ClassB_ClassC_A"].IsNull());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*gadgetInstance)}));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); //Widget_MyId,Widget_Description, Widget_IntProperty, Widget_BoolProperty, Widget_DoubleProperty, Widget_LongProperty, Widget_Date, CalculatedProperty_0

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_TRUE(jsonValues["Widget_Description"].IsNull());
    EXPECT_STREQ("WidgetID", jsonValues["Widget_MyID"].GetString());
    EXPECT_TRUE(jsonValues["Widget_IntProperty"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_BoolProperty"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_DoubleProperty"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_LongProperty"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_DateProperty"].IsNull());
    EXPECT_STREQ("WidgetID", jsonValues["CalculatedProperty_0"].GetString());
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
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*instanceD)}));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    BeInt64Id instanceDId = ECInstanceId::FromString(instanceD->GetInstanceId().c_str());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues["ClassE_ClassF_ClassG_ClassH_IntProperty"].IsNull());
    EXPECT_TRUE(jsonValues["ClassF_ClassH_PropertyF"].IsNull());
    EXPECT_TRUE(jsonValues["ClassE_ClassF_ClassG_ClassH_LongProperty"].IsNull());
    EXPECT_TRUE(jsonValues["ClassG_D"].IsNull());
    EXPECT_TRUE(jsonValues["ClassH_PointProperty"].IsNull());
    EXPECT_TRUE(jsonValues["ClassH_Point2dProperty"].IsNull());
    EXPECT_EQ(instanceDId.GetValueUnchecked(), jsonValues["ClassE_ClassF_ClassG_ClassH_ClassD"].GetInt64());

    rapidjson::Document jsonDoc1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues1 = jsonDoc1["Values"];
    EXPECT_STREQ("1000", jsonValues1["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues1["ClassE_ClassF_ClassG_ClassH_IntProperty"].IsNull());
    EXPECT_EQ(1000, jsonValues1["ClassF_ClassH_PropertyF"].GetInt());
    EXPECT_TRUE(jsonValues1["ClassE_ClassF_ClassG_ClassH_LongProperty"].IsNull());
    EXPECT_TRUE(jsonValues1["ClassG_D"].IsNull());
    EXPECT_TRUE(jsonValues1["ClassH_PointProperty"].IsNull());
    EXPECT_TRUE(jsonValues1["ClassH_Point2dProperty"].IsNull());
    EXPECT_EQ(instanceDId.GetValueUnchecked(), jsonValues1["ClassE_ClassF_ClassG_ClassH_ClassD"].GetInt64());

    rapidjson::Document jsonDoc2 = contentSet.Get(2)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_TRUE(jsonValues2["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues2["ClassE_ClassF_ClassG_ClassH_IntProperty"].IsNull());
    EXPECT_TRUE(jsonValues2["ClassF_ClassH_PropertyF"].IsNull());
    EXPECT_TRUE(jsonValues2["ClassE_ClassF_ClassG_ClassH_LongProperty"].IsNull());
    EXPECT_TRUE(jsonValues2["ClassG_D"].IsNull());
    EXPECT_TRUE(jsonValues2["ClassH_PointProperty"].IsNull());
    EXPECT_TRUE(jsonValues2["ClassH_Point2dProperty"].IsNull());
    EXPECT_EQ(instanceDId.GetValueUnchecked(), jsonValues2["ClassE_ClassF_ClassG_ClassH_ClassD"].GetInt64());
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
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*instanceD)}));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();

    //Calculated property is not applied
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    BeInt64Id instanceDId = ECInstanceId::FromString(instanceD->GetInstanceId().c_str());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_EQ(12345, jsonValues["ClassE_IntProperty"].GetInt());
    EXPECT_TRUE(jsonValues["ClassE_LongProperty"].IsNull());
    EXPECT_EQ(instanceDId.GetValueUnchecked(), jsonValues["ClassE_ClassD"].GetInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ContentModifierAppliesCalculatedPropertiesSpecification)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    
    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*widgetInstance));
    keys.push_back(ECInstanceNodeKey::Create(*gadgetInstance));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); //Gadget_Widget_MyId, Gadget_Widget_Description, Gadget_Widget, Widget_IntProperty, Widget_BoolProperty, Widget_DoubleProperty, Widget_LongProperty, Widget_Date, CalculatedProperty_0

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
        
    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_TRUE(jsonValues["Widget_Gadget_Description"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_Gadget_MyID"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_IntProperty"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_BoolProperty"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_DoubleProperty"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_LongProperty"].IsNull());
    EXPECT_TRUE(jsonValues["Widget_DateProperty"].IsNull());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());
    EXPECT_TRUE(jsonValues["Gadget_Widget"].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_TRUE(jsonValues2["Widget_Gadget_Description"].IsNull());
    EXPECT_STREQ("GadgetID", jsonValues2["Widget_Gadget_MyID"].GetString());
    EXPECT_TRUE(jsonValues2["Widget_IntProperty"].IsNull());
    EXPECT_TRUE(jsonValues2["Widget_BoolProperty"].IsNull());
    EXPECT_TRUE(jsonValues2["Widget_DoubleProperty"].IsNull());
    EXPECT_TRUE(jsonValues2["Widget_LongProperty"].IsNull());
    EXPECT_TRUE(jsonValues2["Widget_DateProperty"].IsNull());
    EXPECT_STREQ("GadgetID", jsonValues2["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues2["Gadget_Widget"].IsNull());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_AppliesDisplayedPropertiesSpecification)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AppliesDisplayedPropertiesSpecification)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*widget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_AppliesDisplayedPropertiesSpecification)
    {
    // set up the dataset
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadgetInstance));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecification* spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "", "RulesEngineTest:Widget");
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentAppliesDisplayedPropertiesSpecificationWhenPriorityIsHigher)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID")); });

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 900, false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentAppliesHiddenPropertiesSpecificationWhenPriorityIsHigher)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID,Description", 900, true));
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Description", 1000, false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); //Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentAppliesHiddenPropertiesSpecificationWhenPrioritiesEqual)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("IntProperty", ECValue(1));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Description,LongProperty", 1000, false));
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Description,IntProperty,LongProperty", 1000, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_IntProperty

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentAppliesDisplayedPropertiesSpecificationFromBaseClass)
    {
    // set up the dataset
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance) {
        instance.SetValue("IntProperty", ECValue(10));
        });

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:ClassF", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("IntProperty", 1500, true)); // base class specification
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("IntProperty,PropertyF", 1000, false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); //ClassF_IntProperty

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, HidesBaseClassPropertiesWhenUsingDisplayedPropertiesSpecificationInContentModifierAndRequestingDerivedClass)
    {
    // set up the dataset
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("PropertyF", ECValue(456));
        });

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    modifier->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("IntProperty", 1000, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassE_IntProperty, ClassF_PropertyF

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentModifierAppliesDisplayedPropertiesSpecification)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Widget");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentModifierAppliesDisplayPropertiesSpecificationWhenPriorityIsHigher)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Widget");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));
    modifier->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 900, false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentModifierAppliesHiddenPropertiesSpecificationWhenPriorityIsHigher)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Widget");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID,Description", 900, true));
    modifier->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Description", 1000, false));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecificationApliesPropertyEditorsSpecification)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("TestID")); });

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyEditor(*new PropertyEditorsSpecification("IntProperty", "IntEditor"));
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("IntProperty", 1000, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Widget_IntProperty
    
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("IntEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, AppliesPropertyEditorsSpecification_GetOneFieldWhenPropertiesAndEditorsAreSimilar)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadget));
    keys.push_back(ECInstanceNodeKey::Create(*widget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyEditor(*new PropertyEditorsSpecification("MyID", "IDEditor"));
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // Gadget_Widget_MyID
    
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("IDEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, AppliesPropertyEditorsSpecification_GetDifferentFieldsWhenPropertiesAndEditorsAreDifferent)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*widget));
    keys.push_back(ECInstanceNodeKey::Create(*gadget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyEditor(*new PropertyEditorsSpecification("MyID", "IDEditor"));
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Gadget");
    modifier->AddPropertyEditor(*new PropertyEditorsSpecification("MyID", "GadgetEditor"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // Widget_MyID, Gadget_MyID

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("IDEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[1]->GetEditor());
    EXPECT_STREQ("GadgetEditor", descriptor->GetVisibleFields()[1]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, AppliesPropertyEditorsSpecificationFromBaseClassOnDerivedClass)
    {
    // set up the dataset
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instanceE));
    keys.push_back(ECInstanceNodeKey::Create(*instanceF));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("IntProperty", 1000, true));

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassE");
    modifier->AddPropertyEditor(*new PropertyEditorsSpecification("IntProperty", "IntEditor"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassE_ClassF_IntProperty
    
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("IntEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesNotApplyPropertyEditorsSpecificationFromDerivedClassOnBaseClass)
    {
    // set up the dataset
    ECEntityClassCP classE = GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCP classF = GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    ECEntityClassCP classH = GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr instanceH = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH);

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instanceH));
    keys.push_back(ECInstanceNodeKey::Create(*instanceF));
    keys.push_back(ECInstanceNodeKey::Create(*instanceE));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("IntProperty", 1000, true));

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "ClassF");
    modifier->AddPropertyEditor(*new PropertyEditorsSpecification("IntProperty", "ClassFIntEditor"));
    rules->AddPresentationRule(*modifier);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassE_IntProperty, ClassH_ClassF_IntProperty
    
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("ClassFIntEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    
    ASSERT_TRUE(nullptr == descriptor->GetVisibleFields()[1]->GetEditor());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadgetInstance));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_EQ(3, descriptor->GetVisibleFields().size());

    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues["Gadget_Widget"].GetString());

    ECInstanceNodeKeyPtr widgetKey = ECInstanceNodeKey::Create(*widgetInstance);
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(widgetKey->GetInstanceId().GetValue(), values["Gadget_Widget"].GetInt64());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadgetInstance));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_EQ(3, descriptor->GetVisibleFields().size());

    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues["Gadget_Widget"].GetString());

    ECInstanceNodeKeyPtr widgetKey = ECInstanceNodeKey::Create(*widgetInstance);
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(widgetKey->GetInstanceId().GetValue(), values["Gadget_Widget"].GetInt64());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instanceB));
    keys.push_back(ECInstanceNodeKey::Create(*instanceC));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ECInstanceNodeKeyPtr classAKey = ECInstanceNodeKey::Create(*instanceA);

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceA).c_str(), displayValues["ClassB_ClassC_A"].GetString());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(classAKey->GetInstanceId().GetValue(), values["ClassB_ClassC_A"].GetInt64());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR displayValues1 = recordJson1["DisplayValues"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceA).c_str(), displayValues1["ClassB_ClassC_A"].GetString());
    RapidJsonValueCR values1 = recordJson1["Values"];
    EXPECT_EQ(classAKey->GetInstanceId().GetValue(), values1["ClassB_ClassC_A"].GetInt64());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadgetInstance));
    keys.push_back(ECInstanceNodeKey::Create(*sprocketInstance));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_EQ(4, descriptor->GetVisibleFields().size());

    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ECInstanceNodeKeyPtr widgetKey = ECInstanceNodeKey::Create(*widgetInstance);
    ECInstanceNodeKeyPtr gadgetKey = ECInstanceNodeKey::Create(*gadgetInstance);

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widgetInstance).c_str(), displayValues["Gadget_Widget"].GetString());
    EXPECT_TRUE(displayValues["Sprocket_Gadget"].IsNull());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(widgetKey->GetInstanceId().GetValue(), values["Gadget_Widget"].GetInt64());
    EXPECT_TRUE(values["Sprocket_Gadget"].IsNull());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR displayValues1 = recordJson1["DisplayValues"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadgetInstance).c_str(), displayValues1["Sprocket_Gadget"].GetString());
    EXPECT_TRUE(displayValues1["Gadget_Widget"].IsNull());
    RapidJsonValueCR values1 = recordJson1["Values"];
    EXPECT_EQ(gadgetKey->GetInstanceId().GetValue(), values1["Sprocket_Gadget"].GetInt64());
    EXPECT_TRUE(values1["Gadget_Widget"].IsNull());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instanceB));
    keys.push_back(ECInstanceNodeKey::Create(*instanceB2));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ECInstanceNodeKeyPtr instanceAKey = ECInstanceNodeKey::Create(*instanceA);
    ECInstanceNodeKeyPtr instanceA2BaseKey = ECInstanceNodeKey::Create(*instanceA2Base);

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceA).c_str(), displayValues["ClassB_ClassB2_A"].GetString());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(instanceAKey->GetInstanceId().GetValue(), values["ClassB_ClassB2_A"].GetInt64());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR displayValues1 = recordJson1["DisplayValues"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceA2Base).c_str(), displayValues1["ClassB_ClassB2_A"].GetString());
    RapidJsonValueCR values1 = recordJson1["Values"];
    EXPECT_EQ(instanceA2BaseKey->GetInstanceId().GetValue(), values1["ClassB_ClassB2_A"].GetInt64());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instanceB2));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ECInstanceNodeKeyPtr instanceA2Key = ECInstanceNodeKey::Create(*instanceA2);

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceA2).c_str(), displayValues["ClassB2_A"].GetString());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(instanceA2Key->GetInstanceId().GetValue(), values["ClassB2_A"].GetInt64());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ECInstanceNodeKeyPtr widgetKey = ECInstanceNodeKey::Create(*widget);

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues["Gadget_Widget"].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(widgetKey->GetInstanceId().GetValue(), values["Gadget_Widget"].GetInt64());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ECInstanceNodeKeyPtr widgetKey = ECInstanceNodeKey::Create(*widget);

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues["Gadget_Widget"].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(widgetKey->GetInstanceId().GetValue(), values["Gadget_Widget"].GetInt64());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*widget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_EQ(3, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ECInstanceNodeKeyPtr widgetKey = ECInstanceNodeKey::Create(*widget);

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues["Gadget_Widget"].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(widgetKey->GetInstanceId().GetValue(), values["Gadget_Widget"].GetInt64());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*widget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_EQ(3, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ECInstanceNodeKeyPtr widgetKey = ECInstanceNodeKey::Create(*widget);

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ("WidgetID", displayValues["Gadget_Widget"].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(widgetKey->GetInstanceId().GetValue(), values["Gadget_Widget"].GetInt64());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "",
        "RulesEngineTest:ClassE,ClassG", false));
    rules->AddPresentationRule(*contentRule);

    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    
    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    EXPECT_TRUE(recordJson1["Values"]["ClassG_D"].IsNull());
    EXPECT_TRUE(recordJson1["DisplayValues"]["ClassG_D"].IsNull());

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*relatedInstance).GetInstanceId().GetValue(), recordJson2["Values"]["ClassG_D"].GetUint64());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*relatedInstance).c_str(), recordJson2["DisplayValues"]["ClassG_D"].GetString());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(R"({
        "ValueFormat": "Array",
        "TypeName": "int[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "int"
            }
        })");
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"(
        {
        "MyClass_ArrayProperty": [2, 1]
        })");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    
    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"(
        {
        "MyClass_ArrayProperty": [3, 4, 5]
        })");
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(R"({
        "ValueFormat": "Array",
        "TypeName": "point3d[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "point3d"
            }
        })");
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"(
        {
        "MyClass_PointsArrayProperty": [
            {"x": 0, "y": 0, "z": 0}, 
            {"x": 1, "y": 1, "z": 1}
        ]})");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    
    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"(
        {
        "MyClass_PointsArrayProperty": [
            {"x": 3, "y": 3, "z": 3},
            {"x": 4, "y": 4, "z": 4},
            {"x": 5, "y": 5, "z": 5}
        ]})");
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
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

    // set up selection
    SelectionInfo elementSelection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*elementInstance)}));
    SelectionInfo derivedSelection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*derivedInstance)}));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, elementSelection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ContentDescriptorCPtr descriptor2 = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, derivedSelection, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsValid());

    // merge descriptors
    ContentDescriptorPtr mergedDescriptor = ContentDescriptor::Create(*descriptor);
    mergedDescriptor->MergeWith(*descriptor2);
    ASSERT_TRUE(mergedDescriptor.IsValid());
    EXPECT_EQ(mergedDescriptor->GetAllFields().size(), descriptor->GetAllFields().size());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"(
        {
        "MyClassA_MyClassB_ArrayProperty": [2, 1]
        })");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    
    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"(
        {
        "MyClassA_MyClassB_ArrayProperty": [3, 4, 5]
        })");
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"(
        {
        "MyClassA_MyClassB_ArrayProperty": [2, 1]
        })");
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged("MyClassA_MyClassB_ArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()["MyClassA_MyClassB_ArrayProperty"].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()["MyClassA_MyClassB_ArrayProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("MyClassA_MyClassB_ArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"(
        {
        "MyClass_ArrayProperty": [2, 1]
        })");
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged("MyClass_ArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_STREQ(varies_string.c_str(), recordJson["Values"]["MyClass_ArrayProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("MyClass_ArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_STREQ(varies_string.c_str(), recordJson["Values"]["MyClass_ArrayProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("MyClass_ArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()["ClassA_ArrayProperty"].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()["ClassA_ArrayProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("ClassA_ArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(R"({
        "ValueFormat": "Array",
        "TypeName": "MyEnum[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "MyEnum"
            }
        })");
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"(
        {
        "MyClass_EnumsArray": [2, 1]
        })");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    rapidjson::Document expectedDisplayValues1;
    expectedDisplayValues1.Parse(R"(
        {
        "MyClass_EnumsArray": ["Two", "One"]
        })");
    EXPECT_EQ(expectedDisplayValues1, recordJson1["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["DisplayValues"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"(
        {
        "MyClass_EnumsArray": [1, 2, 3]
        })");
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    rapidjson::Document expectedDisplayValues2;
    expectedDisplayValues2.Parse(R"(
        {
        "MyClass_EnumsArray": ["One", "Two", "Three"]
        })");
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
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(R"({
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
        })");
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"({
        "MyClass_StructProperty": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    
    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"({
        "MyClass_StructProperty": {
           "IntProperty": 456,
           "StringProperty": "def"
           }
        })");
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
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [structClass](IECInstanceR instance)
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"({
        "MyClassA_MyClassB_StructProperty": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    
    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"({
        "MyClassA_MyClassB_StructProperty": {
           "IntProperty": 456,
           "StringProperty": "def"
           }
        })");
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "MyClassA_MyClassB_StructProperty": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })");
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged("MyClassA_MyClassB_StructProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()["MyClassA_MyClassB_StructProperty"].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()["MyClassA_MyClassB_StructProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("MyClassA_MyClassB_StructProperty"));
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
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "MyClass_StructProperty": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })");
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged("MyClass_StructProperty"));
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
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_STREQ(varies_string.c_str(), recordJson["Values"]["MyClass_StructProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("MyClass_StructProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    EXPECT_TRUE(record->GetValues()["ClassA_StructProperty"].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()["ClassA_StructProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("ClassA_StructProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(R"({
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
        })");
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"({
        "MyClass_StructArrayProperty": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    
    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"({
        "MyClass_StructArrayProperty": [{
           "IntProperty": 456,
           "StringProperty": "def"
           },{
           "IntProperty": 789,
           "StringProperty": "ghi"
           }]
        })");
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"({
        "MyClassA_MyClassB_StructArrayProperty": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    
    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"({
        "MyClassA_MyClassB_StructArrayProperty": [{
           "IntProperty": 456,
           "StringProperty": "def"
           },{
           "IntProperty": 789,
           "StringProperty": "ghi"
           }]
        })");
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "MyClassA_MyClassB_StructArrayProperty": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })");
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged("MyClassA_MyClassB_StructArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()["MyClassA_MyClassB_StructArrayProperty"].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()["MyClassA_MyClassB_StructArrayProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("MyClassA_MyClassB_StructArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "MyClass_StructArrayProperty": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })");
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged("MyClass_StructArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_STREQ(varies_string.c_str(), recordJson["Values"]["MyClass_StructArrayProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("MyClass_StructArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_STREQ(varies_string.c_str(), recordJson["Values"]["MyClass_StructArrayProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("MyClass_StructArrayProperty"));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *mergingDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    EXPECT_TRUE(record->GetValues()["ClassA_StructArrayProperty"].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()["ClassA_StructArrayProperty"].GetString());
    EXPECT_TRUE(record->IsMerged("ClassA_StructArrayProperty"));
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
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructProperty.IntProperty", 2);
        instance.SetValue("StructProperty.IntProperty", ECValue(1), 0);
        instance.SetValue("StructProperty.IntProperty", ECValue(2), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(R"({
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
        })");
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(R"({
        "MyClass_StructProperty": {
           "IntProperty": [1, 2]
           }
        })");
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    
    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(R"({
        "MyClass_StructProperty": {
           "IntProperty": [4, 5, 6]
           }
        })");
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FormatsPrimitiveArrayPropertyValues, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, FormatsPrimitiveArrayPropertyValues)
    {
    TestPropertyFormatter formatter;
    m_manager->SetECPropertyFormatter(&formatter);

    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(R"(
        {
        "MyClass_ArrayProperty": ["_2_", "_1_"]
        })");
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FormatsStructPropertyValues, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, FormatsStructPropertyValues)
    {
    TestPropertyFormatter formatter;
    m_manager->SetECPropertyFormatter(&formatter);

    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(R"({
        "MyClass_StructProperty": {
           "IntProperty": "_123_",
           "StringProperty": "_abc_"
           }
        })");
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
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
        instance.SetValue("Parent", ECValue(RulesEngineTestHelpers::GetInstanceKey(*parent).GetInstanceId(), rel));
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
        instance.SetValue("Parent", ECValue(RulesEngineTestHelpers::GetInstanceKey(*parent).GetInstanceId(), rel));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ParentClass_ParentProperty, Nested<5 ChildClass properties>
    
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(R"({
        "ValueFormat": "Struct",
        "TypeName": "ChildClass",
        "Members": [{
            "Name": "ChildClass_IntProperty",
            "Label": "IntProperty",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "int"
                }
            },{
            "Name": "ChildClass_StringProperty",
            "Label": "StringProperty",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "string"
                }
            },{
            "Name": "ChildClass_ArrayProperty",
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
            "Name": "ChildClass_StructProperty",
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
            "Name": "ChildClass_StructArrayProperty",
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
        })");
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[1]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "ParentClass_ParentProperty": null,
        "ParentClass_ChildClass": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "ChildClass_IntProperty": 1,
                "ChildClass_StringProperty": "111",
                "ChildClass_ArrayProperty": [2, 1],
                "ChildClass_StructProperty": {
                    "IntProperty": 123,
                    "StringProperty": "abc"
                    },
                "ChildClass_StructArrayProperty": [{
                   "IntProperty": 123,
                   "StringProperty": "abc"
                   }]
                },
            "DisplayValues": {
                "ChildClass_IntProperty": 1,
                "ChildClass_StringProperty": "111",
                "ChildClass_ArrayProperty": [2, 1],
                "ChildClass_StructProperty": {
                    "IntProperty": 123,
                    "StringProperty": "abc"
                    },
                "ChildClass_StructArrayProperty": [{
                   "IntProperty": 123,
                   "StringProperty": "abc"
                   }]
                }
            },{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "ChildClass_IntProperty": 2,
                "ChildClass_StringProperty": "222",
                "ChildClass_ArrayProperty": [3],
                "ChildClass_StructProperty": {
                    "IntProperty": 456,
                    "StringProperty": "def"
                    },
                "ChildClass_StructArrayProperty": [{
                   "IntProperty": 456,
                   "StringProperty": "def"
                   },{
                   "IntProperty": 789,
                   "StringProperty": "ghi"
                   }]
                },
            "DisplayValues": {
                "ChildClass_IntProperty": 2,
                "ChildClass_StringProperty": "222",
                "ChildClass_ArrayProperty": [3],
                "ChildClass_StructProperty": {
                    "IntProperty": 456,
                    "StringProperty": "def"
                    },
                "ChildClass_StructArrayProperty": [{
                   "IntProperty": 456,
                   "StringProperty": "def"
                   },{
                   "IntProperty": 789,
                   "StringProperty": "ghi"
                   }]
                }
            }]
        })");
    expectedValues["ParentClass_ChildClass"][0]["PrimaryKeys"][0]["ECClassId"].SetString(childClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ParentClass_ChildClass"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(child1->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ParentClass_ChildClass"][1]["PrimaryKeys"][0]["ECClassId"].SetString(childClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ParentClass_ChildClass"][1]["PrimaryKeys"][0]["ECInstanceId"].SetString(child2->GetInstanceId().c_str(), expectedValues.GetAllocator());

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
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetInstanceId(), rel));
        });
    IECInstancePtr instanceE12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("LongProperty", ECValue((int64_t)222));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetInstanceId(), rel));
        });
    IECInstancePtr instanceE13 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("LongProperty", ECValue((int64_t)333));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetInstanceId(), rel));
        });
    IECInstancePtr instanceD2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceE21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
        });
    IECInstancePtr instanceE22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("LongProperty", ECValue((int64_t)222));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
        });
    IECInstancePtr instanceE23 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("LongProperty", ECValue((int64_t)333));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
        });
    IECInstancePtr instanceD3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceE31 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetInstanceId(), rel));
        });
    IECInstancePtr instanceE32 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("LongProperty", ECValue((int64_t)222));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetInstanceId(), rel));
        });
    IECInstancePtr instanceE33 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("LongProperty", ECValue((int64_t)333));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetInstanceId(), rel));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassD_StringProperty, Array<ClassE_IntProperty + ClassE_LongProperty>
    
    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "ClassD_StringProperty": null,
        "ClassD_ClassE": [{
            "PrimaryKeys": [{"ECClassId":"", "ECInstanceId":""}, {"ECClassId":"", "ECInstanceId":""}, {"ECClassId":"", "ECInstanceId":""}],
            "Values": {
                "ClassE_IntProperty": 1,
                "ClassE_LongProperty": 111
                },
            "DisplayValues": {
                "ClassE_IntProperty": 1,
                "ClassE_LongProperty": 111
                }
            },{
            "PrimaryKeys": [{"ECClassId":"", "ECInstanceId":""}, {"ECClassId":"", "ECInstanceId":""}, {"ECClassId":"", "ECInstanceId":""}],
            "Values": {
                "ClassE_IntProperty": 2,
                "ClassE_LongProperty": 222
                },
            "DisplayValues": {
                "ClassE_IntProperty": 2,
                "ClassE_LongProperty": 222
                }
            },{
            "PrimaryKeys": [{"ECClassId":"", "ECInstanceId":""}, {"ECClassId":"", "ECInstanceId":""}, {"ECClassId":"", "ECInstanceId":""}],
            "Values": {
                "ClassE_IntProperty": 3,
                "ClassE_LongProperty": 333
                },
            "DisplayValues": {
                "ClassE_IntProperty": 3,
                "ClassE_LongProperty": 333
                }
            }]
        })");
    expectedValues["ClassD_ClassE"][0]["PrimaryKeys"][0]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(instanceE11->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][0]["PrimaryKeys"][1]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][0]["PrimaryKeys"][1]["ECInstanceId"].SetString(instanceE21->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][0]["PrimaryKeys"][2]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][0]["PrimaryKeys"][2]["ECInstanceId"].SetString(instanceE31->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][1]["PrimaryKeys"][0]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][1]["PrimaryKeys"][0]["ECInstanceId"].SetString(instanceE12->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][1]["PrimaryKeys"][1]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][1]["PrimaryKeys"][1]["ECInstanceId"].SetString(instanceE22->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][1]["PrimaryKeys"][2]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][1]["PrimaryKeys"][2]["ECInstanceId"].SetString(instanceE32->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][2]["PrimaryKeys"][0]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][2]["PrimaryKeys"][0]["ECInstanceId"].SetString(instanceE13->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][2]["PrimaryKeys"][1]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][2]["PrimaryKeys"][1]["ECInstanceId"].SetString(instanceE23->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][2]["PrimaryKeys"][2]["ECClassId"].SetString(classE->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["ClassD_ClassE"][2]["PrimaryKeys"][2]["ECInstanceId"].SetString(instanceE33->GetInstanceId().c_str(), expectedValues.GetAllocator());

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
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetInstanceId(), rel));
        });
    IECInstancePtr instanceD2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassD_StringProperty, Array<ClassE_IntProperty + ClassE_LongProperty>
    
    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "ClassD_StringProperty": null,
        "ClassD_ClassE": ""
        })");
    expectedValues["ClassD_ClassE"].SetString(varies_string.c_str(), expectedValues.GetAllocator());

    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_TRUE(contentSet.Get(0)->IsMerged("ClassD_ClassE"));
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
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("LongProperty", ECValue((int64_t)222));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("LongProperty", ECValue((int64_t)333));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD1).GetInstanceId(), rel));
        });
    IECInstancePtr instanceD2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(4));
        instance.SetValue("LongProperty", ECValue((int64_t)444));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(5));
        instance.SetValue("LongProperty", ECValue((int64_t)555));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(6));
        instance.SetValue("LongProperty", ECValue((int64_t)666));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD2).GetInstanceId(), rel));
        });
    IECInstancePtr instanceD3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(7));
        instance.SetValue("LongProperty", ECValue((int64_t)777));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(8));
        instance.SetValue("LongProperty", ECValue((int64_t)888));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetInstanceId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(9));
        instance.SetValue("LongProperty", ECValue((int64_t)999));
        instance.SetValue("ClassD", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceD3).GetInstanceId(), rel));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassD_StringProperty, Array<ClassE_IntProperty + ClassE_LongProperty>
    
    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "ClassD_StringProperty": null,
        "ClassD_ClassE": ""
        })");
    expectedValues["ClassD_ClassE"].SetString(varies_string.c_str(), expectedValues.GetAllocator());

    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_TRUE(contentSet.Get(0)->IsMerged("ClassD_ClassE"));
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
        instance.SetValue("Widget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*widget).GetInstanceId(), rel_WG));
        });
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetInstanceId(), rel_GS));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); // 7 Widget properties, Gadget 
    
    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Widget_Description": null,
        "Widget_MyID": null,
        "Widget_IntProperty": null,
        "Widget_BoolProperty": null,
        "Widget_DoubleProperty": null,
        "Widget_LongProperty": null,
        "Widget_DateProperty": null,
        "Widget_Gadget": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "Gadget_MyID": null,
                "Gadget_Description": null,
                "Gadget_Sprocket": [{
                    "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
                    "Values": {
                        "Sprocket_Description": null,
                        "Sprocket_MyID": null
                        }
                    }]
                },
            "DisplayValues": {
                "Gadget_MyID": null,
                "Gadget_Description": null,
                "Gadget_Sprocket": [{
                    "DisplayValues": {
                        "Sprocket_Description": null,
                        "Sprocket_MyID": null
                        }
                    }]
                }
            }]
        })");
    expectedValues["Widget_Gadget"][0]["PrimaryKeys"][0]["ECClassId"].SetString(m_gadgetClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Widget_Gadget"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(gadget->GetInstanceId().c_str(), expectedValues.GetAllocator());
    expectedValues["Widget_Gadget"][0]["Values"]["Gadget_Sprocket"][0]["PrimaryKeys"][0]["ECClassId"].SetString(m_sprocketClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Widget_Gadget"][0]["Values"]["Gadget_Sprocket"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(sprocket->GetInstanceId().c_str(), expectedValues.GetAllocator());

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
        instance.SetValue("Widget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*widget).GetInstanceId(), rel_WG));
        });
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetInstanceId(), rel_GS));
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); // 7 Widget properties, Sprocket 
    
    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Widget_Description": null,
        "Widget_MyID": null,
        "Widget_IntProperty": null,
        "Widget_BoolProperty": null,
        "Widget_DoubleProperty": null,
        "Widget_LongProperty": null,
        "Widget_DateProperty": null,
        "Widget_Gadget_Sprocket": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "Sprocket_Description": null,
                "Sprocket_MyID": null
                },
            "DisplayValues": {
                "Sprocket_Description": null,
                "Sprocket_MyID": null
                }
            }]
        })");
    expectedValues["Widget_Gadget_Sprocket"][0]["PrimaryKeys"][0]["ECClassId"].SetString(m_sprocketClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Widget_Gadget_Sprocket"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(sprocket->GetInstanceId().c_str(), expectedValues.GetAllocator());

    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, RequestingDescriptorWithClassIdsAllowsUsingSelectedNodeECExpressionSymbol)
    {    
    // set up selection
    ECClassCP classF = GetClass("RulesEngineTest", "ClassF");
    SelectionInfo selection({classF});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("SelectedNode.ECInstance.IsOfClass(\"ClassE\", \"RulesEngineTest\") ANDALSO SelectedNode.ClassName = \"ClassF\"", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(m_widgetClass, &descriptor->GetSelectClasses()[0].GetSelectClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, RequestingDescriptorWithClassIdsAndUsingSelectedNodeECInstanceSymbolFailsGracefully)
    {    
    // set up selection
    SelectionInfo selection({m_widgetClass});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("SelectedNode.ECInstance.IntProperty = 123", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                       Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerContentTests, RelatedPropertiesSpecification_GetCorrectFieldDisplayLabelWhenRelationshipMeaningIsSetToSameInstance)
    {    
    // set up selection
    ECRelationshipClassCP widgetHasGadget = GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadget, *widget, *gadget);
    
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
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
    // set up selection
    ECRelationshipClassCP widgetHasGadget = GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadget, *widget, *gadget);

    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*gadget));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
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
    // set up selection
    ECClassCP classK = GetClass("RulesEngineTest", "ClassK");
    ECClassCP classL = GetClass("RulesEngineTest", "ClassL");
    IECInstancePtr instanceK = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classK);
    IECInstancePtr instanceL = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classL);

    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instanceK));
    keys.push_back(ECInstanceNodeKey::Create(*instanceL));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification();
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_STREQ("ClassK_LengthProperty", descriptor->GetVisibleFields()[0]->GetName().c_str());
    EXPECT_STREQ("ClassL_LengthProperty", descriptor->GetVisibleFields()[1]->GetName().c_str());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    bvector<ContentDescriptor::Field*> fieldVectorCopy = descriptor->GetAllFields();
    // hide all fields except Widget_MyID
    for (ContentDescriptor::Field const* field : fieldVectorCopy)
        {
        if (!field->GetName().Equals("Widget_MyID"))
            overridenDescriptor->RemoveField(field->GetName().c_str());
        }
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // validate descriptor
    EXPECT_EQ(1, overridenDescriptor->GetVisibleFields().size()); // Widget_MyID

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *overridenDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR values1 = contentSet.Get(0)->GetValues();
    ASSERT_EQ(1, values1.MemberCount());
    EXPECT_STREQ("Test1", values1["Widget_MyID"].GetString());
    RapidJsonValueCR values2 = contentSet.Get(1)->GetValues();
    ASSERT_EQ(1, values2.MemberCount());
    EXPECT_STREQ("Test2", values2["Widget_MyID"].GetString());
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

    // set up selection
    NavNodeKeyList keys;
    keys.push_back(ECInstanceNodeKey::Create(*instance1));
    keys.push_back(ECInstanceNodeKey::Create(*instance2));
    keys.push_back(ECInstanceNodeKey::Create(*instance3));
    keys.push_back(ECInstanceNodeKey::Create(*instance4));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(8, descriptor->GetVisibleFields().size()); //Gadget_Widget_MyId, Gadget_Widget_Description, Gadget_Widget, Widget_IntProperty, Widget_BoolProperty, Widget_DoubleProperty, Widget_LongProperty, Widget_Date
    
    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    bvector<ContentDescriptor::Field*> fieldVectorCopy = descriptor->GetAllFields();
    // hide all fields except Widget_MyID
    for (ContentDescriptor::Field const* field : fieldVectorCopy)
        {
        if (!field->GetName().Equals("Gadget_Widget_MyID"))
            overridenDescriptor->RemoveField(field->GetName().c_str());
        }
    ASSERT_EQ(1, overridenDescriptor->GetVisibleFields().size());
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *overridenDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());
        
    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetValues();
    EXPECT_STREQ("GadgetID", jsonValues["Gadget_Widget_MyID"].GetString());
    RapidJsonValueCR jsonValues2 = contentSet.Get(1)->GetValues();
    EXPECT_STREQ("WidgetID", jsonValues2["Gadget_Widget_MyID"].GetString());
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

    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);

    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    ContentInstancesOfSpecificClassesSpecificationP gadgetSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", true);
    ContentInstancesOfSpecificClassesSpecificationP sprocketSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Sprocket", true);
    gadgetSpec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Widget", 1500, true));
    sprocketSpec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Gadget", 1500, true));
    rule->AddSpecification(*gadgetSpec);
    rule->AddSpecification(*sprocketSpec);

    // validate content descriptor
    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());

    ContentDescriptorCPtr descriptor = RulesDrivenECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = RulesDrivenECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ECInstanceNodeKeyPtr widgetKey = ECInstanceNodeKey::Create(*widget);
    ECInstanceNodeKeyPtr gadgetKey = ECInstanceNodeKey::Create(*gadget);

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetValues();
    EXPECT_EQ(widgetKey->GetInstanceId().GetValue(), jsonValues["Gadget_Widget"].GetUint64());
    EXPECT_TRUE(jsonValues["Sprocket_Gadget"].IsNull());
    RapidJsonValueCR jsonValues2 = contentSet.Get(1)->GetValues();
    EXPECT_EQ(gadgetKey->GetInstanceId().GetValue(), jsonValues2["Sprocket_Gadget"].GetUint64());
    EXPECT_TRUE(jsonValues2["Gadget_Widget"].IsNull());
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

    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create a ruleset
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", true);
    m_locater->AddRuleSet(*ruleSet);

    ruleSet->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    ruleSet->AddPresentationRule(*rule);
    ContentInstancesOfSpecificClassesSpecificationP gadgetSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", true);
    gadgetSpec->AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("Widget", 1500, true));
    rule->AddSpecification(*gadgetSpec);

    RulesDrivenECPresentationManager::ContentOptions options(ruleSet->GetRuleSetId().c_str());

    // validate content descriptor
    ContentDescriptorCPtr descriptor = RulesDrivenECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // modify content descriptor
    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // request for content
    ContentCPtr content = RulesDrivenECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *overridenDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("WidgetID", jsonValues["Gadget_Widget"].GetString());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*instanceA), ECInstanceNodeKey::Create(*instanceB)}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*rootElement), ECInstanceNodeKey::Create(*childElement)}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID,Description"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadget).c_str(), contentSet.Get(0)->GetDisplayLabel().c_str());
    EXPECT_STREQ("WidgetID", contentSet.Get(1)->GetDisplayLabel().c_str());
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
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID,Description"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "Description,MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget,Sprocket", false);
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());
    EXPECT_STREQ("GadgetDescription", contentSet.Get(0)->GetDisplayLabel().c_str());
    EXPECT_STREQ("WidgetID", contentSet.Get(1)->GetDisplayLabel().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*sprocket).c_str(), contentSet.Get(2)->GetDisplayLabel().c_str());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("WidgetDescription", contentSet.Get(0)->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_WhenNoPropertiesSpecified_OverrideAs_ClassName_BriefcaseId_LocalId)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), contentSet.Get(0)->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_AssertsWhenNoClasesSpecified_OverrideAsECInstanceDisplaylabel)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverrideSetsDisplayLabelProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    IGNORE_BE_ASSERT();
    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), contentSet.Get(0)->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_OverridesInstanceLabelAsFirstNotEmptyParameter)
    {
    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_OverridesInstanceLabelAsFirstNotEmptyParameter", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "Description,MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("WidgetID", contentSet.Get(0)->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_SetsDisplayLabelPropertyByDefaultAsECInstanceDisplaylabel)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_SetsDisplayLabelPropertyByDefaultAsECInstanceDisplaylabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "Description,MyID"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);
    
    // options
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    
    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), contentSet.Get(0)->GetDisplayLabel().c_str());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*instanceC)}));

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(2, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassB_StringProperty", contentSet.Get(0)->GetDisplayLabel().c_str());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(2, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceA).c_str(), contentSet.Get(0)->GetDisplayLabel().c_str());
    EXPECT_STREQ("ClassB_StringProperty", contentSet.Get(1)->GetDisplayLabel().c_str());
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
    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(3, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    EXPECT_STREQ("ClassA_BaseStringProperty", contentSet.Get(0)->GetDisplayLabel().c_str());
    EXPECT_STREQ("ClassB_BaseStringProperty", contentSet.Get(1)->GetDisplayLabel().c_str());
    EXPECT_STREQ("ClassC_StringProperty", contentSet.Get(2)->GetDisplayLabel().c_str());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(3, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *modifiedDescriptor, selection, PageOptions(), options.GetJson()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassC_BaseStringProperty", contentSet.Get(0)->GetDisplayLabel().c_str());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("ClassB_StringProperty", jsonValues["ClassC_A"].GetString());
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

    // set up selection
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

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
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();;
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();;
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("ClassC_StringProperty", jsonValues["ClassD_A"].GetString());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_MyAspect

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Element_MyAspect": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "MyAspect_AspectName": "my aspect"
                },
            "DisplayValues": {
                "MyAspect_AspectName": "my aspect"
                }
            }]
        })");
    expectedValues["Element_MyAspect"][0]["PrimaryKeys"][0]["ECClassId"].SetString(aspectClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Element_MyAspect"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(aspect->GetInstanceId().c_str(), expectedValues.GetAllocator());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create());

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Element_ElementName": "my element 1",
        "Element_MyAspectA": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                },
            "DisplayValues": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                }
            }]
        })");
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECClassId"].SetString(aspectAClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(aspect1->GetInstanceId().c_str(), expectedValues.GetAllocator());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*element1)}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Element_ElementName": "my element 1",
        "Element_MyAspectA": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                },
            "DisplayValues": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                }
            }]
        })");
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECClassId"].SetString(aspectAClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(aspect1->GetInstanceId().c_str(), expectedValues.GetAllocator());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*model1)}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Element_ElementName": "my element 1",
        "Element_MyAspectA": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                },
            "DisplayValues": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                }
            }]
        })");
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECClassId"].SetString(aspectAClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(aspect1->GetInstanceId().c_str(), expectedValues.GetAllocator());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*model)}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Element_ElementName": "my element 1",
        "Element_MyAspectA": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                },
            "DisplayValues": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                }
            }]
        })");
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECClassId"].SetString(aspectAClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(aspect1->GetInstanceId().c_str(), expectedValues.GetAllocator());
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
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create({ECInstanceNodeKey::Create(*element1)}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = IECPresentationManager::GetManager().GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(4, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA, Element_MyAspectB, Element_MyAspectC

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).get();
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Element_ElementName": "my element 2",
        "Element_MyAspectA": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                },
            "DisplayValues": {
                "MyAspectA_Aspect_A_Name": "my aspect a"
                }
            }],
        "Element_MyAspectB": [],
        "Element_MyAspectC": []
        })");
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECClassId"].SetString(aspectAClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Element_MyAspectA"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(aspect2->GetInstanceId().c_str(), expectedValues.GetAllocator());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(R"({
        "Element_ElementName": "my element 3",
        "Element_MyAspectA": [],
        "Element_MyAspectB": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "MyAspectB_Aspect_B_Name": "my aspect b"
                },
            "DisplayValues": {
                "MyAspectB_Aspect_B_Name": "my aspect b"
                }
            }],
        "Element_MyAspectC": []
        })");
    expectedValues["Element_MyAspectB"][0]["PrimaryKeys"][0]["ECClassId"].SetString(aspectBClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Element_MyAspectB"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(aspect3->GetInstanceId().c_str(), expectedValues.GetAllocator());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(R"({
        "Element_ElementName": "my element 4",
        "Element_MyAspectA": [],
        "Element_MyAspectB": [],
        "Element_MyAspectC": [{
            "PrimaryKeys": [{"ECClassId": "", "ECInstanceId": ""}],
            "Values": {
                "MyAspectC_Aspect_C_Name": "my aspect c"
                },
            "DisplayValues": {
                "MyAspectC_Aspect_C_Name": "my aspect c"
                }
            }]
        })");
    expectedValues["Element_MyAspectC"][0]["PrimaryKeys"][0]["ECClassId"].SetString(aspectCClass->GetId().ToString().c_str(), expectedValues.GetAllocator());
    expectedValues["Element_MyAspectC"][0]["PrimaryKeys"][0]["ECInstanceId"].SetString(aspect4->GetInstanceId().c_str(), expectedValues.GetAllocator());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }
