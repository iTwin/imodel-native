/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/RulesPreprocessorTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/ExtendedData.h>
#include "../../../Source/RulesDriven/RulesEngine/RulesPreprocessor.h"
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "TestNavNode.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2015
+===============+===============+===============+===============+===============+======*/
struct RulesPreprocessorTests : ::testing::Test
{
protected:
    ECDb                  m_connection;
    TestRuleSetLocaterPtr m_locater;
    RuleSetLocaterManager m_locaterManager;
    TestUserSettings      m_userSettings;
    ECExpressionsCache    m_expressionsCache;

public:
    void SetUp() override
        {
        m_locater = TestRuleSetLocater::Create();
        m_locaterManager.RegisterLocater(*m_locater);
        }

    void TearDown() override
        {
        m_locaterManager.UnregisterLocater(*m_locater);
        m_locater = nullptr;
        }
};

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRules_FindsRulesetWithHighestVersion)
    {
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("test", 1, 1, false, "", "", "", false);
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("test", 1, 2, false, "", "", "", false);
    m_locater->AddRuleSet(*rules1);
    m_locater->AddRuleSet(*rules2);
    
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager);
    ASSERT_EQ(foundRules, rules2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRules_DoesntFindRulesetWithVersionTooHigh)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 2, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    ASSERT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRules_DoesntFindSupplementalRuleset)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, true, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    ASSERT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRules_Supplementation_KeepsPrimaryRulesetProperties)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary", 1, 0, false, "", "supported_schema", "preferred_image", true);
    primary->SetSearchClasses("class1;class2");
    primary->SetExtendedData("test");
    PresentationRuleSetPtr supplemental = PresentationRuleSet::CreateInstance("supplementing", 1, 1, true, "a", "", "", false);
    m_locater->AddRuleSet(*primary);
    m_locater->AddRuleSet(*supplemental);
    
    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager);
    ASSERT_STREQ(primary->GetRuleSetId().c_str(), rules->GetRuleSetId().c_str());
    ASSERT_EQ(primary->GetVersionMajor(), rules->GetVersionMajor());
    ASSERT_EQ(primary->GetVersionMinor(), rules->GetVersionMinor());
    ASSERT_FALSE(rules->GetIsSupplemental());
    ASSERT_STREQ("", rules->GetSupplementationPurpose().c_str());
    ASSERT_STREQ(primary->GetSupportedSchemas().c_str(), rules->GetSupportedSchemas().c_str());
    ASSERT_STREQ(primary->GetPreferredImage().c_str(), rules->GetPreferredImage().c_str());
    ASSERT_EQ(primary->GetIsSearchEnabled(), rules->GetIsSearchEnabled());
    ASSERT_STREQ(primary->GetSearchClasses().c_str(), rules->GetSearchClasses().c_str());
    ASSERT_STREQ(primary->GetExtendedData().c_str(), rules->GetExtendedData().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddRulesToRuleset(PresentationRuleSetR ruleset)
    {
    ruleset.AddPresentationRule(*new RootNodeRule());
    ruleset.AddPresentationRule(*new ChildNodeRule());
    ruleset.AddPresentationRule(*new ContentRule());
    ruleset.AddPresentationRule(*new ImageIdOverride());
    ruleset.AddPresentationRule(*new LabelOverride());
    ruleset.AddPresentationRule(*new StyleOverride());
    ruleset.AddPresentationRule(*new GroupingRule());
    ruleset.AddPresentationRule(*new LocalizationResourceKeyDefinition());
    ruleset.AddPresentationRule(*new UserSettingsGroup());
    ruleset.AddPresentationRule(*new CheckBoxRule());
    ruleset.AddPresentationRule(*new RenameNodeRule());
    ruleset.AddPresentationRule(*new SortingRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRules_Supplementation_MergesRules)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary", 1, 0, false, "", "", "", false);
    AddRulesToRuleset(*primary);
    PresentationRuleSetPtr supplemental = PresentationRuleSet::CreateInstance("supplementing", 1, 1, true, "a", "", "", false);
    AddRulesToRuleset(*supplemental);
    m_locater->AddRuleSet(*primary);
    m_locater->AddRuleSet(*supplemental);

    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager);
    ASSERT_EQ(2, rules->GetRootNodesRules().size());
    ASSERT_EQ(2, rules->GetChildNodesRules().size());
    ASSERT_EQ(2, rules->GetContentRules().size());
    ASSERT_EQ(2, rules->GetImageIdOverrides().size());
    ASSERT_EQ(2, rules->GetLabelOverrides().size());
    ASSERT_EQ(2, rules->GetStyleOverrides().size());
    ASSERT_EQ(2, rules->GetGroupingRules().size());
    ASSERT_EQ(2, rules->GetLocalizationResourceKeyDefinitions().size());
    ASSERT_EQ(2, rules->GetUserSettings().size());
    ASSERT_EQ(2, rules->GetCheckBoxRules().size());
    ASSERT_EQ(2, rules->GetRenameNodeRules().size());
    ASSERT_EQ(2, rules->GetSortingRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRules_Supplementation_MergesRulesWithHigherVersion)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary", 1, 0, false, "", "", "", false);
    AddRulesToRuleset(*primary);
    PresentationRuleSetPtr supplemental1 = PresentationRuleSet::CreateInstance("supplementing1", 1, 0, true, "a", "", "", false);
    AddRulesToRuleset(*supplemental1);
    PresentationRuleSetPtr supplemental2 = PresentationRuleSet::CreateInstance("supplementing2", 1, 1, true, "a", "", "", false);
    AddRulesToRuleset(*supplemental2);
    AddRulesToRuleset(*supplemental2);
    m_locater->AddRuleSet(*primary);
    m_locater->AddRuleSet(*supplemental1);
    m_locater->AddRuleSet(*supplemental2);

    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager);
    ASSERT_EQ(3, rules->GetRootNodesRules().size());
    ASSERT_EQ(3, rules->GetChildNodesRules().size());
    ASSERT_EQ(3, rules->GetContentRules().size());
    ASSERT_EQ(3, rules->GetImageIdOverrides().size());
    ASSERT_EQ(3, rules->GetLabelOverrides().size());
    ASSERT_EQ(3, rules->GetStyleOverrides().size());
    ASSERT_EQ(3, rules->GetGroupingRules().size());
    ASSERT_EQ(3, rules->GetLocalizationResourceKeyDefinitions().size());
    ASSERT_EQ(3, rules->GetUserSettings().size());
    ASSERT_EQ(3, rules->GetCheckBoxRules().size());
    ASSERT_EQ(3, rules->GetRenameNodeRules().size());
    ASSERT_EQ(3, rules->GetSortingRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_NoConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new RootNodeRule("", 1, false, TargetTree_MainTree, false));
    rules->GetRootNodesRules()[0]->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    
    RulesPreprocessor::RootNodeRuleParameters params(m_connection, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_RulesSortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("", 2, false, TargetTree_MainTree, false);
    rule->SetStopFurtherProcessing(true);
    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(m_connection, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
    ASSERT_EQ(0, specs.size()); // rule with higher priority has no specs
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_SpecsSortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new RootNodeRule("", 1, false, TargetTree_MainTree, false));
    rules->GetRootNodesRules()[0]->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(2, false, false, false, false, false, ""));
    rules->GetRootNodesRules()[0]->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(3, false, false, false, false, false, ""));
    rules->GetRootNodesRules()[0]->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, false, ""));
    
    RulesPreprocessor::RootNodeRuleParameters params(m_connection, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
    ASSERT_EQ(3, specs.size());
    ASSERT_EQ(3, specs[0].GetPriority());
    ASSERT_EQ(2, specs[1].GetPriority());
    ASSERT_EQ(1, specs[2].GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    RootNodeRule* rule = new RootNodeRule("1 = 2", 1, false, TargetTree_MainTree, false);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("1 = 1", 1, false, TargetTree_MainTree, false);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(m_connection, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithTargetTree)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, true, false, false, true, true, "1"));
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("", 1, false, TargetTree_SelectionTree, false);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, true, false, false, true, true, "2"));
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, true, false, false, true, true, "3"));
    rules->AddPresentationRule(*rule);
    
    // note: using "supported schemas" as a way to know which rule the specification came from
    RulesPreprocessor::RootNodeRuleParameters paramsMainTree(m_connection, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(paramsMainTree);
    ASSERT_EQ(2, specs.size());
    ASSERT_STREQ("1", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("3", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());
    
    RulesPreprocessor::RootNodeRuleParameters paramsSelectionTree(m_connection, *rules, TargetTree_SelectionTree, m_userSettings, nullptr, m_expressionsCache);
    specs = RulesPreprocessor::GetRootNodeSpecifications(paramsSelectionTree);
    ASSERT_EQ(2, specs.size());
    ASSERT_STREQ("2", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("3", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());
    
    RulesPreprocessor::RootNodeRuleParameters paramsBothTrees(m_connection, *rules, TargetTree_Both, m_userSettings, nullptr, m_expressionsCache);
    specs = RulesPreprocessor::GetRootNodeSpecifications(paramsBothTrees);
    ASSERT_EQ(1, specs.size());
    ASSERT_STREQ("3", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithOnlyIfNotHandled)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("", 1, true, TargetTree_MainTree, false);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(m_connection, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
    ASSERT_EQ(0, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithStopFurtherProcessing)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);

    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);

    rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->SetStopFurtherProcessing(true);
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(m_connection, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_IncludesSpecsFromSubConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    
    SubCondition* subcondition = new SubCondition("1 = 1");
    subcondition->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    rule->GetSubConditionsR().push_back(subcondition);

    SubCondition* subcondition2 = new SubCondition();
    subcondition2->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    subcondition->GetSubConditionsR().push_back(subcondition2);

    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(m_connection, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
    ASSERT_EQ(2, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetChildNodeSpecifications)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->GetChildNodesRules()[0]->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::ChildNodeRuleParameters params(m_connection, *node, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    ChildNodeRuleSpecificationsList specs = RulesPreprocessor::GetChildNodeSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetChildNodeSpecifications_BySpecificationId)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    RootNodeRule* rule = new RootNodeRule();

    ChildNodeRule* stopRule = new ChildNodeRule();
    stopRule->SetStopFurtherProcessing(true);

    AllInstanceNodesSpecification* spec1 = new AllInstanceNodesSpecification();
    ChildNodeRule* nestedRule = new ChildNodeRule();
    nestedRule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, false, "one"));
    nestedRule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, false, "two"));
    spec1->GetNestedRules().push_back(nestedRule);
    spec1->GetNestedRules().push_back(stopRule);
    rule->GetSpecificationsR().push_back(spec1);

    AllInstanceNodesSpecification* spec2 = new AllInstanceNodesSpecification();
    nestedRule = new ChildNodeRule();
    nestedRule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, false, "three"));
    spec2->GetNestedRules().push_back(nestedRule);
    spec2->GetNestedRules().push_back(new ChildNodeRule(*stopRule));
    rule->GetSpecificationsR().push_back(spec2);

    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationId(spec1->GetId());
    RulesPreprocessor::ChildNodeRuleParameters params1(m_connection, *node, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    ChildNodeRuleSpecificationsList specs = RulesPreprocessor::GetChildNodeSpecifications(params1);

    // all sub-specs of the the spec that generated the node
    ASSERT_EQ(2, specs.size()); 
    // use "supported schemas" string to identify expected specs
    ASSERT_STREQ("one", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("two", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());

    nodeExtendedDataWriter.SetSpecificationId(spec2->GetId());
    RulesPreprocessor::ChildNodeRuleParameters params2(m_connection, *node, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    specs = RulesPreprocessor::GetChildNodeSpecifications(params2);
    // all sub-specs of the the spec that generated the node
    ASSERT_EQ(1, specs.size()); 
    ASSERT_STREQ("three", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetChildNodeSpecifications_BySpecificationIdWithRequested)
    {
    AllInstanceNodesSpecification* spec = new AllInstanceNodesSpecification();
    // set values using individual methods.
    spec->SetPriority(1);
    spec->SetAlwaysReturnsChildren(false);
    spec->SetHideNodesInHierarchy(false);
    spec->SetHideIfNoChildren(false);
    spec->SetGroupByClass(false);
    spec->SetGroupByLabel(false);
    spec->SetSupportedSchemas("requested");

    ChildNodeRule* nestedRule = new ChildNodeRule();
    nestedRule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, false, "one"));
    nestedRule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, false, "two"));
    spec->GetNestedRules().push_back(nestedRule);
    ChildNodeRule* stopRule = new ChildNodeRule();
    stopRule->SetStopFurtherProcessing(true);
    spec->GetNestedRules().push_back(stopRule);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    ChildNodeRule* rule = new ChildNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification());
    rule->GetSpecificationsR().push_back(spec);
    rules->AddPresentationRule(*rule);

    TestNavNodePtr node = TestNavNode::Create();
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationId(spec->GetId());
    nodeExtendedDataWriter.SetRequestedSpecification(true);

    RulesPreprocessor::ChildNodeRuleParameters params(m_connection, *node, *rules, TargetTree_MainTree, m_userSettings, nullptr, m_expressionsCache);
    ChildNodeRuleSpecificationsList specs = RulesPreprocessor::GetChildNodeSpecifications(params);
    // all sub-specs of the the spec that generated the node
    ASSERT_EQ(3, specs.size());
    // use "supported schemas" string to identify expected specs
    ASSERT_STREQ("requested", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("one", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("two", (static_cast<AllInstanceNodesSpecificationCP>(&specs[2].GetSpecification()))->GetSupportedSchemas().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLabelOverride_WithoutCondition)
    {
    LabelOverride* rule = new LabelOverride("", 1, "GetLabelOverride_WithoutCondition", "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
    ASSERT_TRUE(nullptr != labelOverride); 
    ASSERT_STREQ("GetLabelOverride_WithoutCondition", labelOverride->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLabelOverride_WithoutLabelAndDescription)
    {
    LabelOverride* rule = new LabelOverride("", 1, "", "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
    ASSERT_TRUE(nullptr == labelOverride); 
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLabelOverride_WithMatchingCondition)
    {
    LabelOverride* rule = new LabelOverride("Not ThisNode.IsInstanceNode", 1, "GetLabelOverride_WithMatchingCondition", "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
    ASSERT_TRUE(nullptr != labelOverride); 
    ASSERT_STREQ("GetLabelOverride_WithMatchingCondition", labelOverride->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLabelOverride_WithNoMatchingCondition)
    {
    LabelOverride* rule = new LabelOverride("ThisNode.IsInstanceNode", 1, "GetLabelOverride_WithNoMatchingCondition", "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
    ASSERT_TRUE(nullptr == labelOverride); 
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetStyleOverride_WithoutCondition)
    {
    StyleOverrideP rule = new StyleOverride("", 1, "FC", "BC", "FS");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    StyleOverrideCP styleOverride = RulesPreprocessor::GetStyleOverride(params);
    ASSERT_TRUE(nullptr != styleOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetStyleOverride_WithMatchingCondition)
    {
    StyleOverrideP rule = new StyleOverride("Not ThisNode.IsInstanceNode", 1, "FC", "BC", "FS");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    StyleOverrideCP styleOverride = RulesPreprocessor::GetStyleOverride(params);
    ASSERT_TRUE(nullptr != styleOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetStyleOverride_WithNoMatchingCondition)
    {
    StyleOverrideP rule = new StyleOverride("ThisNode.IsInstanceNode", 1, "FC", "BC", "FS");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    StyleOverrideCP styleOverride = RulesPreprocessor::GetStyleOverride(params);
    ASSERT_TRUE(nullptr == styleOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetSortingRules_SortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    SortingRule* rule1 = new SortingRule("", 1, "", "", "", true, false, false);
    SortingRule* rule2 = new SortingRule("", 3, "", "", "", true, false, false);
    SortingRule* rule3 = new SortingRule("", 2, "", "", "", true, false, false);
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);
    rules->AddPresentationRule(*rule3);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0, m_connection, *rules, m_userSettings, nullptr, m_expressionsCache);
    bvector<SortingRuleCP> sortingRules = RulesPreprocessor::GetSortingRules(params);
    ASSERT_EQ(3, sortingRules.size()); 
    EXPECT_EQ(3, sortingRules[0]->GetPriority());
    EXPECT_EQ(2, sortingRules[1]->GetPriority());
    EXPECT_EQ(1, sortingRules[2]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetSortingRules_VerifiesCondition)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    SortingRule* rule1 = new SortingRule("ParentNode.IsInstanceNode", 1, "", "", "", true, false, false);
    SortingRule* rule2 = new SortingRule("ParentNode.IsPropertyGroupingNode", 2, "", "", "", true, false, false);
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);
    
    TestNavNodePtr node = TestNavNode::Create();
    node->SetType(NAVNODE_TYPE_ECPropertyGroupingNode);
    
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0, m_connection, *rules, m_userSettings, nullptr, m_expressionsCache);
    bvector<SortingRuleCP> sortingRules = RulesPreprocessor::GetSortingRules(params);
    ASSERT_EQ(1, sortingRules.size()); 
    EXPECT_EQ(2, sortingRules[0]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetGroupingRules_SortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    GroupingRule* rule1 = new GroupingRule("", 1, false, "", "", "", "", "");
    GroupingRule* rule2 = new GroupingRule("", 3, false, "", "", "", "", "");
    GroupingRule* rule3 = new GroupingRule("", 2, false, "", "", "", "", "");
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);
    rules->AddPresentationRule(*rule3);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0, m_connection, *rules, m_userSettings, nullptr, m_expressionsCache);
    bvector<GroupingRuleCP> groupingRules = RulesPreprocessor::GetGroupingRules(params);
    ASSERT_EQ(3, groupingRules.size()); 
    EXPECT_EQ(3, groupingRules[0]->GetPriority());
    EXPECT_EQ(2, groupingRules[1]->GetPriority());
    EXPECT_EQ(1, groupingRules[2]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetGroupingRules_VerifiesCondition)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    GroupingRule* rule1 = new GroupingRule("ParentNode.IsInstanceNode", 1, false, "", "", "", "", "");
    GroupingRule* rule2 = new GroupingRule("ParentNode.IsPropertyGroupingNode", 2, false, "", "", "", "", "");
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);
    
    TestNavNodePtr node = TestNavNode::Create();
    node->SetType(NAVNODE_TYPE_ECPropertyGroupingNode);
    
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0, m_connection, *rules, m_userSettings, nullptr, m_expressionsCache);
    bvector<GroupingRuleCP> groupingRules = RulesPreprocessor::GetGroupingRules(params);
    ASSERT_EQ(1, groupingRules.size()); 
    EXPECT_EQ(2, groupingRules[0]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetGroupingRules_OnlyIfNotHandledFlagHandled)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    GroupingRule* rule1 = new GroupingRule("", 1, true, "", "", "", "", "");
    GroupingRule* rule2 = new GroupingRule("", 2, false, "", "", "", "", "");
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);
    
    RulesPreprocessor::AggregateCustomizationRuleParameters params(nullptr, 0, m_connection, *rules, m_userSettings, nullptr, m_expressionsCache);
    bvector<GroupingRuleCP> groupingRules = RulesPreprocessor::GetGroupingRules(params);
    ASSERT_EQ(1, groupingRules.size()); 
    EXPECT_EQ(2, groupingRules[0]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLocalizationResourceKeyDefinion_WithMatchingId)
    {
    LocalizationResourceKeyDefinitionP definition = new LocalizationResourceKeyDefinition(1, "test_id", "key", "default_value");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*definition);
    
    LocalizationResourceKeyDefinitionCP foundDefinition = RulesPreprocessor::GetLocalizationResourceKeyDefinition("test_id", *rules);
    ASSERT_TRUE(foundDefinition == definition);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLocalizationResourceKeyDefinion_WithoutMatchingId)
    {
    LocalizationResourceKeyDefinitionP definition = new LocalizationResourceKeyDefinition(1, "test_id", "key", "default_value");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*definition);
    
    LocalizationResourceKeyDefinitionCP foundDefinition = RulesPreprocessor::GetLocalizationResourceKeyDefinition("does_not_exist", *rules);
    ASSERT_TRUE(nullptr == foundDefinition); 
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetImageIdOverride_WithoutCondition)
    {
    ImageIdOverrideP rule = new ImageIdOverride("", 1, "ImageId");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    ImageIdOverrideCP imageOverride = RulesPreprocessor::GetImageIdOverride(params);
    ASSERT_TRUE(nullptr != imageOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetImageIdOverride_WithMatchingCondition)
    {
    ImageIdOverrideP rule = new ImageIdOverride("Not ThisNode.IsInstanceNode", 1, "ImageId");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    ImageIdOverrideCP imageOverride = RulesPreprocessor::GetImageIdOverride(params);
    ASSERT_TRUE(nullptr != imageOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetImageIdOverride_WithNoMatchingCondition)
    {
    ImageIdOverrideP rule = new ImageIdOverride("ThisNode.IsInstanceNode", 1, "ImageId");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    ImageIdOverrideCP imageOverride = RulesPreprocessor::GetImageIdOverride(params);
    ASSERT_TRUE(nullptr == imageOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetCheckboxRule_WithoutCondition)
    {
    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "PropertyValue", false, false, "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    CheckBoxRuleCP checkboxRule = RulesPreprocessor::GetCheckboxRule(params);
    ASSERT_TRUE(nullptr != checkboxRule);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetCheckboxRule_WithMatchingCondition)
    {
    CheckBoxRuleP rule = new CheckBoxRule("Not ThisNode.IsInstanceNode", 1, false, "PropertyValue", false, false, "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    CheckBoxRuleCP checkboxRule = RulesPreprocessor::GetCheckboxRule(params);
    ASSERT_TRUE(nullptr != checkboxRule);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetCheckboxRule_WithNoMatchingCondition)
    {
    CheckBoxRuleP rule = new CheckBoxRule("ThisNode.IsInstanceNode", 1, false, "PropertyValue", false, false, "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule);
    
    TestNavNodePtr node = TestNavNode::Create();
    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    CheckBoxRuleCP checkboxRule = RulesPreprocessor::GetCheckboxRule(params);
    ASSERT_TRUE(nullptr == checkboxRule);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_NoConditions)
    {

    NavNodePtr node = TestNavNode::Create();
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(&node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ContentRule("", 1, false));
    
    RulesPreprocessor::ContentRuleParameters params(m_connection, *NavNodeKeyListContainer::Create(selectedNodeKeys), "", "", false, *rules, m_userSettings, nullptr, m_expressionsCache, TestNodeLocater(*node));
    ContentRuleSpecificationsList specs = RulesPreprocessor::GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_RulesSortedByPriority)
    {
    NavNodePtr node = TestNavNode::Create();
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(&node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    ContentRuleP rule1 = new ContentRule("", 1, true);
    rules->AddPresentationRule(*rule1);
    ContentRuleP rule2 = new ContentRule("", 2, true);
    rules->AddPresentationRule(*rule2);
    
    RulesPreprocessor::ContentRuleParameters params(m_connection, *NavNodeKeyListContainer::Create(selectedNodeKeys), "", "", false, *rules, m_userSettings, nullptr, m_expressionsCache, TestNodeLocater(*node));
    ContentRuleSpecificationsList specs = RulesPreprocessor::GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    ASSERT_EQ(rule2, &(*specs.begin()).GetRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_WithConditions)
    {
    NavNodePtr node = TestNavNode::Create();
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(&node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    ContentRuleP rule1 = new ContentRule("1 = 2", 1, false);
    rules->AddPresentationRule(*rule1);
    ContentRuleP rule2 = new ContentRule("1 = 1", 1, false);
    rules->AddPresentationRule(*rule2);
    
    RulesPreprocessor::ContentRuleParameters params(m_connection, *NavNodeKeyListContainer::Create(selectedNodeKeys), "", "", false, *rules, m_userSettings, nullptr, m_expressionsCache, TestNodeLocater(*node));
    ContentRuleSpecificationsList specs = RulesPreprocessor::GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    ASSERT_EQ(rule2, &(*specs.begin()).GetRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_ReturnsMultipleRulesIfOnlyIfNotHandledIsFalse)
    {
    NavNodePtr node = TestNavNode::Create();
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(&node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ContentRule("", 1, false));
    rules->AddPresentationRule(*new ContentRule("", 1, false));
    
    RulesPreprocessor::ContentRuleParameters params(m_connection, *NavNodeKeyListContainer::Create(selectedNodeKeys), "", "", false, *rules, m_userSettings, nullptr, m_expressionsCache, TestNodeLocater(*node));
    ContentRuleSpecificationsList specs = RulesPreprocessor::GetContentSpecifications(params);
    ASSERT_EQ(2, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_ReturnsOneRuleIfOnlyIfNotHandledIsTrue)
    {
    NavNodePtr node = TestNavNode::Create();
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(&node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ContentRule("", 1, true));
    rules->AddPresentationRule(*new ContentRule("", 1, true));
    
    RulesPreprocessor::ContentRuleParameters params(m_connection, *NavNodeKeyListContainer::Create(selectedNodeKeys), "", "", false, *rules, m_userSettings, nullptr, m_expressionsCache, TestNodeLocater(*node));
    ContentRuleSpecificationsList specs = RulesPreprocessor::GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                 04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedGroupingRules_DoesNotFindRulesDefinedDeeperThanSpecification_ReturnsSortedByNestingLevel)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 0, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->AddPresentationRule(*new GroupingRule("", 1, false, "TestSchemaName1", "", "", "", ""));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    ChildNodeRule* nestedChildNodeRule = new ChildNodeRule("", 1, false, TargetTree_MainTree);
    nestedChildNodeRule->GetCustomizationRulesR().push_back(new GroupingRule("", 1, false, "TestSchemaName3", "", "", "", ""));
    spec->GetNestedRules().push_back(nestedChildNodeRule);
    rules->GetChildNodesRules()[0]->GetSpecificationsR().push_back(spec);
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new GroupingRule("", 1, false, "TestSchemaName2", "", "", "", ""));

    RulesPreprocessor::AggregateCustomizationRuleParameters params(nullptr, spec->GetId(), m_connection, *rules, m_userSettings, nullptr, m_expressionsCache);
    bvector<GroupingRuleCP> groupingRules = RulesPreprocessor::GetGroupingRules(params);
    // Returns only root level and nested level customization rules, because specification nested rule doesn't have specification 
    ASSERT_EQ(2, groupingRules.size());
    EXPECT_EQ("TestSchemaName2", groupingRules[0]->GetSchemaName());      //root level
    EXPECT_EQ("TestSchemaName1", groupingRules[1]->GetSchemaName());      //nested level 1 level
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                 04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedGroupingRules_FindsMostlyNestedRule_ReturnsSortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 0, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->AddPresentationRule(*new GroupingRule("", 3, false, "TestSchemaName1", "", "", "", ""));
    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");

    AllInstanceNodesSpecification* nestedSpec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    ChildNodeRule* nestedChildNodeRule = new ChildNodeRule("", 1, false, TargetTree_MainTree);
    nestedChildNodeRule->GetCustomizationRulesR().push_back(new GroupingRule("", 2, false, "TestSchemaName2", "", "", "", ""));
    nestedChildNodeRule->GetSpecificationsR().push_back(nestedSpec);
    spec->GetNestedRules().push_back(nestedChildNodeRule);
    rules->GetChildNodesRules()[0]->GetSpecificationsR().push_back(spec);
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new GroupingRule("", 1, false, "TestSchemaName3", "", "", "", ""));

    RulesPreprocessor::AggregateCustomizationRuleParameters params(nullptr, nestedSpec->GetId(), m_connection, *rules, m_userSettings, nullptr, m_expressionsCache);
    bvector<GroupingRuleCP> groupingRules = RulesPreprocessor::GetGroupingRules(params);
    // Returns all rules from all nested levels
    ASSERT_EQ(3, groupingRules.size());
    EXPECT_EQ(3, groupingRules[0]->GetPriority());      //root level
    EXPECT_EQ(2, groupingRules[1]->GetPriority());      //nested level 1 level
    EXPECT_EQ(1, groupingRules[2]->GetPriority());      //nested level 2 level
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                 04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedCustomizationRules_SameScope_SamePriorities_ReturnsFirstRule)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 0, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->GetSpecificationsR().push_back(spec);
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new LabelOverride("", 5, "LabelOverrideLabelValue1", "LabelOverrideDescriptionValue"));
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new LabelOverride("", 5, "LabelOverrideLabelValue2", "LabelOverrideDescriptionValue2"));

    TestNavNodePtr node = TestNavNode::Create();
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationId(spec->GetId());

    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
    // Same scope, conditions match, same prority - returns first declaired
    ASSERT_TRUE(nullptr != labelOverride);
    EXPECT_STREQ("LabelOverrideLabelValue1", labelOverride->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                 04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedCustomizationRules_SameScope_DifferentPriorities_ReturnsByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 0, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->GetSpecificationsR().push_back(spec);
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new ImageIdOverride("", 1, "ImageIdOverrideTestValue1"));
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new ImageIdOverride("", 3, "ImageIdOverrideTestValue2"));

    TestNavNodePtr node = TestNavNode::Create();
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationId(spec->GetId());

    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    ImageIdOverrideCP imageOverride = RulesPreprocessor::GetImageIdOverride(params);
    // Same scope - returns highest priority
    ASSERT_TRUE(nullptr != imageOverride);
    EXPECT_STREQ("ImageIdOverrideTestValue2", imageOverride->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                 04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedCustomizationRules_DifferentScopes_DifferentPriorities_ReturnsHighestPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 0, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->AddPresentationRule(*new StyleOverride("", 2, "Green", "Red", "Bold"));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->GetSpecificationsR().push_back(spec);
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new StyleOverride("", 1, "Blue", "Red", "Bold"));

    TestNavNodePtr node = TestNavNode::Create();
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationId(spec->GetId());

    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    StyleOverrideCP styleOverride = RulesPreprocessor::GetStyleOverride(params);
    // Different Scopes - returns highest priority
    ASSERT_TRUE(nullptr != styleOverride);
    EXPECT_STREQ("Green", styleOverride->GetForeColor().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                 04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedCustomizationRules_DifferentScopes_SamePriority_ReturnsMostlyNested)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 0, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->AddPresentationRule(*new CheckBoxRule("", 1, false, "CheckBoxPropertyName2", false, false, ""));
    
    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->GetSpecificationsR().push_back(spec);
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new CheckBoxRule("", 1, false, "CheckBoxPropertyName1", false, false, ""));

    TestNavNodePtr node = TestNavNode::Create(); 
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationId(spec->GetId());

    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    CheckBoxRuleCP checkBox = RulesPreprocessor::GetCheckboxRule(params);
    // Different scopes, same priority - return mostly nested
    ASSERT_TRUE(nullptr != checkBox);
    EXPECT_STREQ("CheckBoxPropertyName1", checkBox->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                 04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetCustomizationRule_WithConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 0, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->GetSpecificationsR().push_back(spec);
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new LabelOverride("2=3", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue"));
    rules->GetChildNodesRules()[0]->GetCustomizationRulesR().push_back(new LabelOverride("2=2", 1, "LabelOverrideLabelValue2", "LabelOverrideDescriptionValue2"));

    TestNavNodePtr node = TestNavNode::Create();
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationId(spec->GetId());

    RulesPreprocessor::CustomizationRuleParameters params(m_connection, *node, nullptr, *rules, m_userSettings, nullptr, m_expressionsCache);
    LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
    // Same scope, conditions do not match, different priorities - return condition true
    ASSERT_TRUE(nullptr != labelOverride);
    EXPECT_STREQ("LabelOverrideLabelValue2", labelOverride->GetLabel().c_str());
    }
