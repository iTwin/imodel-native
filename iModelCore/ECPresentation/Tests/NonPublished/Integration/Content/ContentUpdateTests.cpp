/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../PresentationManagerIntegrationTests.h"
#include "../../../../Source/RulesDriven/RulesEngine/PresentationManagerImpl.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct ContentUpdateTests : UpdateTests
    {};

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceInsert)
    {
    // insert a widget instance
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());

    // insert one more instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // expect 2 records
    content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget1->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());
    ASSERT_STREQ(widget2->GetInstanceId().c_str(), content->GetContentSet()[1]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceInsert", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceUpdate)
    {
    // insert a widget instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());

    // notify about an update (even though we didn't change anything)
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // expect 1 record
    content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceUpdate", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentUpdateTests, UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // expect 2 records
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    ASSERT_TRUE(content->GetContentSet()[1].IsValid());

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget1);

    // expect 1 record
    content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_STREQ(widget2->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist)
    {
    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // expect 0 records
    content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(0, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithNoContent)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // request content and expect none
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(m_db, ContentDisplayType::Graphics, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor1.IsNull());
    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(m_db, ContentDisplayType::Grid, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsNull());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // still expect no content
    descriptor1 = m_manager->GetContentDescriptor(m_db, ContentDisplayType::Graphics, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor1.IsNull());
    descriptor2 = m_manager->GetContentDescriptor(m_db, ContentDisplayType::Grid, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsNull());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithContent)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // set up selection
    KeySetPtr inputKeys = KeySet::Create({RulesEngineTestHelpers::GetInstanceKey(*widget)});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // request content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());

    ContentDescriptorCPtr descriptor1 = m_manager->GetContentDescriptor(m_db, ContentDisplayType::Graphics, 0, *inputKeys, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor1.IsValid());
    ContentCPtr content1 = m_manager->GetContent(*descriptor1, PageOptions()).get();
    ASSERT_EQ(1, content1->GetContentSet().GetSize());

    ContentDescriptorCPtr descriptor2 = m_manager->GetContentDescriptor(m_db, ContentDisplayType::Grid, 0, *inputKeys, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsValid());
    ContentCPtr content2 = m_manager->GetContent(*descriptor2, PageOptions()).get();
    ASSERT_EQ(1, content2->GetContentSet().GetSize());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);
    inputKeys = KeySet::Create();

    // expect no content in both cases
    descriptor1 = m_manager->GetContentDescriptor(m_db, ContentDisplayType::Graphics, 0, *inputKeys, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor1.IsNull());
    descriptor2 = m_manager->GetContentDescriptor(m_db, ContentDisplayType::Grid, 0, *inputKeys, nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor2.IsNull());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, InvalidatesWhenUserSettingChanges_UsedInRuleCondition)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("1 = GetSettingIntValue(\"test\")", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();

    // expect the descriptor to be null because no rules applied
    ASSERT_TRUE(descriptor.IsNull());

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 content item
    descriptor = m_manager->GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    EXPECT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    EXPECT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, InvalidatesWhenUserSettingChanges_UsedInInstanceFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "1 = GetSettingIntValue(\"test\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // expect 0 records
    ASSERT_EQ(0, content->GetContentSet().GetSize());

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 content item
    content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    EXPECT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    EXPECT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentUpdateTests, DoesNotInvalidateWhenUnusedUserSettingChanges)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect the content to be the same
    content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect 0 update records
    ASSERT_EQ(0, m_updateRecordsHandler->GetFullUpdateRecords().size());
    }
