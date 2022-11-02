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
struct ContentUpdateTests : UpdateTests
    {};

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesContentAfterECInstanceInsert, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceInsert)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));

    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());

    // insert one more instance
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instance2);

    // expect 2 records
    content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    ASSERT_STREQ(instance1->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());
    ASSERT_STREQ(instance2->GetInstanceId().c_str(), content->GetContentSet()[1]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceInsert", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesContentAfterECInstanceUpdate, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceUpdate)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));

    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());

    // notify about an update (even though we didn't change anything)
    m_eventsSource->NotifyECInstanceUpdated(m_db, *instance);

    // expect 1 record
    content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_STREQ(instance->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceUpdate", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(ContentUpdateTests, UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));

    // expect 2 records
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());
    ASSERT_TRUE(content->GetContentSet()[1].IsValid());

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *instance1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *instance1);

    // expect 1 record
    content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_STREQ(instance2->GetInstanceId().c_str(), content->GetContentSet()[0]->GetKeys()[0].GetId().ToString().c_str());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceDeleteWhenMoreInstancesExist", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (ContentUpdateTests, UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));

    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ASSERT_TRUE(content->GetContentSet()[0].IsValid());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *instance, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *instance);

    // expect 0 records
    content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));
    ASSERT_EQ(0, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ("UpdatesContentAfterECInstanceDeleteWhenNoMoreInstancesExist", m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithNoContent, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (ContentUpdateTests, UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithNoContent)
    {
    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(m_db, *GetClass("A"), nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // request content and expect none
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Graphics, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor1.IsNull());
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Grid, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor2.IsNull());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *instance, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *instance);

    // still expect no content
    descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Graphics, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor1.IsNull());
    descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Grid, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor2.IsNull());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithContent, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (ContentUpdateTests, UpdatesAllContentBasedOnOneRulesetButSendsOnlyOneNotification_WithContent)
    {
    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(m_db, *GetClass("A"), nullptr, true);

    // set up selection
    KeySetPtr inputKeys = KeySet::Create({RulesEngineTestHelpers::GetInstanceKey(*instance)});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Graphics, 0, *inputKeys)));
    ASSERT_TRUE(descriptor1.IsValid());
    ContentCPtr content1 = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor1)));
    ASSERT_EQ(1, content1->GetContentSet().GetSize());

    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Grid, 0, *inputKeys)));
    ASSERT_TRUE(descriptor2.IsValid());
    ContentCPtr content2 = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor2)));
    ASSERT_EQ(1, content2->GetContentSet().GetSize());

    // delete the instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *instance, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *instance);
    inputKeys = KeySet::Create();

    // expect no content in both cases
    descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Graphics, 0, *inputKeys)));
    ASSERT_TRUE(descriptor1.IsNull());
    descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Grid, 0, *inputKeys)));
    ASSERT_TRUE(descriptor2.IsNull());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    ASSERT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    ASSERT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InvalidatesWhenUserSettingChanges_UsedInRuleCondition, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (ContentUpdateTests, InvalidatesWhenUserSettingChanges_UsedInRuleCondition)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("1 = GetSettingIntValue(\"test\")", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request for descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));

    // expect the descriptor to be null because no rules applied
    ASSERT_TRUE(descriptor.IsNull());

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 content item
    descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    EXPECT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    EXPECT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InvalidatesWhenUserSettingChanges_UsedInInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (ContentUpdateTests, InvalidatesWhenUserSettingChanges_UsedInInstanceFilter)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "1 = GetSettingIntValue(\"test\")", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request for content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));

    // expect 0 records
    ASSERT_EQ(0, content->GetContentSet().GetSize());

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect 1 content item
    content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect one full update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetFullUpdateRecords().size());
    EXPECT_STREQ(rules->GetRuleSetId().c_str(), m_updateRecordsHandler->GetFullUpdateRecords()[0].GetRulesetId().c_str());
    EXPECT_EQ(FullUpdateRecord::UpdateTarget::Content, m_updateRecordsHandler->GetFullUpdateRecords()[0].GetUpdateTarget());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotInvalidateWhenUnusedUserSettingChanges, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (ContentUpdateTests, DoesNotInvalidateWhenUnusedUserSettingChanges)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request for content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));

    // expect 1 record
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // change a setting
    m_manager->GetUserSettings(rules->GetRuleSetId().c_str()).SetSettingIntValue("test", 1);

    // expect the content to be the same
    content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    // expect 0 update records
    ASSERT_EQ(0, m_updateRecordsHandler->GetFullUpdateRecords().size());
    }
