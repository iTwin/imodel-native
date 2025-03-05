/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentProviderTests.h"
#include "../../../../Source/Content/ContentQueryContracts.h"

ECDbTestProject* ContentProviderTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("ContentProviderTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderTests::SetUp()
    {
    ECPresentationTest::SetUp();
    s_project->GetECDb().AbandonChanges();

    m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    m_gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    m_sprocketClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket");

    m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_customFunctions = new CustomFunctionsInjector(m_connections, *m_connection);
    m_ruleset = PresentationRuleSet::CreateInstance("ContentProviderTests");

    m_nodesLocater = std::shared_ptr<TestNodeLocater>();
    m_context = ContentProviderContext::Create(*m_ruleset, ContentDisplayType::Undefined, 0,
        *NavNodeKeyListContainer::Create(), m_nodesLocater, m_categorySupplier,
        std::make_unique<RulesetVariables>(), m_expressionsCache, m_relatedPathsCache, m_nodesFactory, nullptr, nullptr);
    m_context->SetQueryContext(m_connections, *m_connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderTests::TearDown()
    {
    DELETE_AND_CLEAR(m_customFunctions);
    }
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, SelectedNodeInstances_AllPropertiesOfOneSelectedNode)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*m_connection, *instance1);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k){return ECInstanceKey(k.GetClass()->GetId(), k.GetId());});

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
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfTheSameClass)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance2);

    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });
    std::transform(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    ASSERT_EQ(2, provider->GetContentSetSize());

    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *descriptor);

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *descriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfDifferentClasses)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance2);

    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });
    std::transform(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(8, descriptor->GetVisibleFields().size());
    ASSERT_EQ(2, provider->GetContentSetSize());

    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *descriptor);

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *descriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, CreatesImageIdWhenSpecified)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*m_connection, *instance);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

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
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, CustomizesImageId)
    {
    m_ruleset->AddPresentationRule(*new ImageIdOverride("ThisNode.IsInstanceNode", 1, "\"TestImageId\""));

    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*m_connection, *instance);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

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
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, UsesRelatedInstanceInImageIdOverrideCondition)
    {
    ECRelationshipClassCR relationshipWidgetHasGadget = *s_project->GetECDb().Schemas().GetSchema("RulesEngineTest")->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Gadget ID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetInstance);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    m_ruleset->AddPresentationRule(*new ImageIdOverride("ThisNode.ClassName = \"Widget\" ANDALSO gadgetAlias.MyID = \"Gadget ID\"", 1, "\"TestImageId\""));

    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    spec->SetShowImages(true);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget", "gadgetAlias"));
    ContentRule rule;
    rule.AddSpecification(*spec);
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    ASSERT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*widgetInstance, *item, *descriptor, nullptr, "TestImageId");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, UsesRelatedInstanceInImageIdOverrideExpression)
    {
    ECRelationshipClassCR relationshipWidgetHasGadget = *s_project->GetECDb().Schemas().GetSchema("RulesEngineTest")->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Gadget ID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetInstance);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    m_ruleset->AddPresentationRule(*new ImageIdOverride("ThisNode.ClassName = \"Widget\"", 1, "gadgetAlias.MyID"));

    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    spec->SetShowImages(true);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget", "gadgetAlias"));
    ContentRule rule;
    rule.AddSpecification(*spec);
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(m_widgetClass->GetPropertyCount(), descriptor->GetVisibleFields().size());

    ASSERT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*widgetInstance, *item, *descriptor, nullptr, "Gadget ID");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, CreatesLabelWhenSpecified)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test MyID"));});

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*m_connection, *instance);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));
    m_context->SetContentFlags((int)ContentFlags::ShowLabels);

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    ContentRule rule;
    rule.AddSpecification(*spec);
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    ASSERT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance, *item, *descriptor, [](const auto&) { return "Test MyID"; });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, PagingUnsortedData)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance2);
    NavNodePtr node3 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance3);

    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    keys.push_back(node3->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });
    std::transform(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });
    std::transform(node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));
    provider->SetPageOptions(PageOptions(1, 5));

    ASSERT_EQ(2, provider->GetContentSetSize());

    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *provider->GetContentDescriptor());

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance3, *item, *provider->GetContentDescriptor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, PagingSortedData)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});

    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance2);
    NavNodePtr node3 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance3);

    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    keys.push_back(node3->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });
    std::transform(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });
    std::transform(node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));
    ASSERT_TRUE(nullptr != provider->GetContentDescriptor());

    ContentDescriptorPtr ovr = ContentDescriptor::Create(*provider->GetContentDescriptor());
    ovr->SetSortingField(FIELD_NAME(m_widgetClass, "IntProperty"));
    provider->SetContentDescriptor(*ovr);
    // result: instance3, instance1, instance2

    provider->SetPageOptions(PageOptions(1, 5));
    ASSERT_EQ(2, provider->GetContentSetSize());

    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *provider->GetContentDescriptor());

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *provider->GetContentDescriptor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentProviderTests, PageDoesNotExceedPageSize)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance1);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance2);
    NavNodePtr node3 = TestNodesHelper::CreateInstanceNode(*m_connection, *instance3);

    NavNodeKeyList keys;
    keys.push_back(node1->GetKey());
    keys.push_back(node2->GetKey());
    keys.push_back(node3->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node1->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });
    std::transform(node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node2->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });
    std::transform(node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node3->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));

    provider->SetPageOptions(PageOptions(0, 2));
    ASSERT_EQ(2, provider->GetContentSetSize());

    ContentSetItemPtr item;

    ASSERT_TRUE(provider->GetContentSetItem(item, 0));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance1, *item, *provider->GetContentDescriptor());

    ASSERT_TRUE(provider->GetContentSetItem(item, 1));
    RulesEngineTestHelpers::ValidateContentSetItem(*instance2, *item, *provider->GetContentDescriptor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentProviderTests, LoadsNestedContentFields)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetId(), rel));
        instance.SetValue("Description", ECValue("One"));
        });
    IECInstancePtr sprocket2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetId(), rel));
        instance.SetValue("Description", ECValue("Two"));
        });

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", m_gadgetClass->GetFullName(), false, false);
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

    Utf8CP fieldName = descriptor->GetVisibleFields()[3]->GetUniqueName().c_str();
    rapidjson::Document json = item->AsJson();
    RapidJsonValueCR value = json["Values"][fieldName];
    ASSERT_TRUE(value.IsArray());
    ASSERT_EQ(2, value.Size());

    ASSERT_TRUE(value[0].IsObject());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*sprocket1).GetId().GetValue(), BeRapidJsonUtilities::UInt64FromValue(value[0]["PrimaryKeys"][0]["ECInstanceId"]));
    EXPECT_STREQ("One", value[0]["Values"][FIELD_NAME(m_sprocketClass, "Description")].GetString());

    ASSERT_TRUE(value[1].IsObject());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*sprocket2).GetId().GetValue(), BeRapidJsonUtilities::UInt64FromValue(value[1]["PrimaryKeys"][0]["ECInstanceId"]));
    EXPECT_STREQ("Two", value[1]["Values"][FIELD_NAME(m_sprocketClass, "Description")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentProviderTests, AllowsUsingCustomFunctionsInInstanceFilterWithPolymorphicallyRelatedProperties)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&](IECInstanceR instance)
        {
        instance.SetValue("Widget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*widget).GetId(), rel));
        });

    ContentRule rule;
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1,
        "this.IsOfClass(\"Widget\", \"RulesEngineTest\")", m_widgetClass->GetFullName(), true, false);
    rule.AddSpecification(*spec);
    RelatedPropertiesSpecification* relatedPropertiesSpec = new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        rel->GetFullName(), m_gadgetClass->GetFullName(), "Description", RelationshipMeaning::RelatedInstance, true);
    spec->AddRelatedProperty(*relatedPropertiesSpec);

    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule));

    ContentDescriptorCP descriptor = provider->GetContentDescriptor();
    ASSERT_TRUE(nullptr != descriptor);
    EXPECT_EQ(7 + 1, descriptor->GetVisibleFields().size()); // 7 Widget properties + Gadget.Description
    EXPECT_EQ(1, provider->GetContentSetSize());

    ContentSetItemPtr item;
    EXPECT_TRUE(provider->GetContentSetItem(item, 0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentProviderTests, CloneContentProviderAndChangeCancelationToken)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavNodePtr node = TestNodesHelper::CreateInstanceNode(*m_connection, *instance1);

    NavNodeKeyList keys;
    keys.push_back(node->GetKey());
    m_context->SetInputKeys(*NavNodeKeyListContainer::Create(keys));

    bvector<ECInstanceKey> instanceKeys;
    std::transform(node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().begin(), node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().end(),
        std::back_inserter(instanceKeys), [](ECClassInstanceKeyCR k) {return ECInstanceKey(k.GetClass()->GetId(), k.GetId()); });

    ContentRule rule;
    rule.AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    SpecificationContentProviderPtr provider = SpecificationContentProvider::Create(*m_context, ContentRuleInstanceKeys(rule, instanceKeys));
    ICancelationTokenPtr cancelationToken = new TestCancelationToken([&](){return false;});
    provider->GetContextR().SetCancelationToken(cancelationToken.get());

    SpecificationContentProviderPtr clonedProvider = provider->Clone();

    // Update cancelation token
    ICancelationTokenPtr newCancelationToken = new TestCancelationToken([&](){return false;});
    clonedProvider->GetContextR().SetCancelationToken(newCancelationToken.get());

    // Check what only one cancelation token has changed
    ASSERT_NE(&provider->GetContext().GetCancelationToken(), &clonedProvider->GetContext().GetCancelationToken());
    }
