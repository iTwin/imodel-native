/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentProviderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "ContentProviderTests.h"
#include "TestHelpers.h"

ECDbTestProject* ContentProviderTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("ContentProviderTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderTests::SetUp()
    {
    s_project->GetECDb().AbandonChanges();

    Localization::Init();

    m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    m_gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    m_sprocketClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket");

    m_customFunctions = new CustomFunctionsInjector(m_connections, s_project->GetECDb());
    m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_ruleset = PresentationRuleSet::CreateInstance("ContentProviderTests", 1, 0, false, "", "", "", false);

    m_context = ContentProviderContext::Create(*m_ruleset, true, ContentDisplayType::Undefined, *NavNodeKeyListContainer::Create(), m_nodesLocater, m_categorySupplier,
        m_settings, m_expressionsCache, m_relatedPathsCache, m_nodesFactory, nullptr);
    m_context->SetQueryContext(m_connections, *m_connection, m_statementCache, *m_customFunctions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderTests::TearDown()
    {
    DELETE_AND_CLEAR(m_customFunctions);
    Localization::Terminate();
    }
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, SelectedNodeInstances_AllPropertiesOfOneSelectedNode)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance1);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());

    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());
    
    ASSERT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *descriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfTheSameClass)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*instance2);
    
    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    instanceKeys.push_back(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    
    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());
    
    ASSERT_EQ(2, provider->GetContentSetSize());

    Json::Value json;
    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *descriptor);

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *descriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfDifferentClasses)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*instance2);
    
    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    instanceKeys.push_back(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    
    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(8, descriptor->GetVisibleFields().size());    
    ASSERT_EQ(2, provider->GetContentSetSize());

    Json::Value json;
    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *descriptor);

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *descriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, CreatesImageIdWhenSpecified)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance);
    
    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    
    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    spec->SetShowImages(true);
    ContentRule rule;
    rule.AddSpecification(*spec);
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());
    
    ASSERT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance, *item, *descriptor, nullptr, "ECInstanceImage://RulesEngineTest:Widget");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, CustomizesImageId)
    {
    m_ruleset->AddPresentationRule(*new ImageIdOverride("ThisNode.IsInstanceNode", 1, "\"TestImageId\""));

    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance);
    
    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    
    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    spec->SetShowImages(true);
    ContentRule rule;
    rule.AddSpecification(*spec);
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());
    
    ASSERT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance, *item, *descriptor, nullptr, "TestImageId");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, CreatesLabelWhenSpecified)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test MyID"));});
    
    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*instance);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    
    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    ContentRule rule;
    rule.AddSpecification(*spec);
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);

    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->AddContentFlag(ContentFlags::ShowLabels);

    provider->SetContentDescriptor(*ovr);
    ASSERT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance, *item, *ovr, "Test MyID");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, PagingUnsortedData)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*instance2);
    NavNodePtr node3 = TestNodesHelper::CreateInstanceNode(*instance3);
    
    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    keys.push_back(node3->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    instanceKeys.push_back(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    instanceKeys.push_back(node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    
    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));
    provider->SetPageOptions(PageOptions(1, 5));

    ASSERT_EQ(2, provider->GetContentSetSize());

    Json::Value json;
    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *provider->GetContentDescriptor());

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance3, *item, *provider->GetContentDescriptor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, PagingSortedData)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, 
        [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*instance2);
    NavNodePtr node3 = TestNodesHelper::CreateInstanceNode(*instance3);
    
    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    keys.push_back(node3->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    instanceKeys.push_back(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    instanceKeys.push_back(node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    
    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));
    ASSERT_TRUE(nullptr != provider->GetContentDescriptor());

    ContentDescriptorPtr ovr = ContentDescriptor::Create(*provider->GetContentDescriptor());
    ovr->SetSortingField("Widget_IntProperty");
    provider->SetContentDescriptor(*ovr);
    // result: instance3, instance1, instance2

    provider->SetPageOptions(PageOptions(1, 5));
    ASSERT_EQ(2, provider->GetContentSetSize());

    Json::Value json;
    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *provider->GetContentDescriptor());

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *provider->GetContentDescriptor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, PageDoesNotExceedPageSize)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*instance2);
    NavNodePtr node3 = TestNodesHelper::CreateInstanceNode(*instance3);
    
    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    keys.push_back(node3->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));
    
    bvector<ECInstanceKey> instanceKeys;
    instanceKeys.push_back(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    instanceKeys.push_back(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    instanceKeys.push_back(node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());

    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));
    
    provider->SetPageOptions(PageOptions(0, 2));
    ASSERT_EQ(2, provider->GetContentSetSize());

    Json::Value json;
    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *provider->GetContentDescriptor());

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *provider->GetContentDescriptor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentProviderTests, LoadsNestedContentFields)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetInstanceId(), rel));
        instance.SetValue("Description", ECValue("One"));
        });
    IECInstancePtr sprocket2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetInstanceId(), rel));
        instance.SetValue("Description", ECValue("Two"));
        });

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", m_gadgetClass->GetFullName(), false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        rel->GetFullName(), m_sprocketClass->GetFullName(), "Description", RelationshipMeaning::RelatedInstance));

    ContentRule rule;
    rule.AddSpecification(*spec);

    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    ASSERT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Sprocket.Description
    ASSERT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    ASSERT_TRUE(provider->GetContentSetItem(item, 0));

    Utf8CP fieldName = descriptor->GetVisibleFields()[3]->GetName().c_str();
    rapidjson::Document json = item->AsJson();
    RapidJsonValueCR value = json["Values"][fieldName];
    ASSERT_TRUE(value.IsArray());
    ASSERT_EQ(2, value.Size());
    
    ASSERT_TRUE(value[0].IsObject());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*sprocket1).GetInstanceId().GetValue(), BeRapidJsonUtilities::UInt64FromValue(value[0]["PrimaryKeys"][0]["ECInstanceId"]));
    EXPECT_STREQ("One", value[0]["Values"]["Sprocket_Description"].GetString());
    
    ASSERT_TRUE(value[1].IsObject());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*sprocket2).GetInstanceId().GetValue(), BeRapidJsonUtilities::UInt64FromValue(value[1]["PrimaryKeys"][0]["ECInstanceId"]));
    EXPECT_STREQ("Two", value[1]["Values"]["Sprocket_Description"].GetString());
    }
