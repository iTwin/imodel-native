/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct RulesPreprocessorTests : ECPresentationTest
{
protected:
    static ECDbTestProject* s_project;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    TestRuleSetLocaterPtr m_locater;
    RuleSetLocaterManager m_locaterManager;
    TestUserSettings m_userSettings;
    ECExpressionsCache m_expressionsCache;
    ECSqlStatementCache m_statementsCache;
    Utf8String m_locale;

public:
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("RulesPreprocessorTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }

    RulesPreprocessorTests() : m_locaterManager(m_connections), m_statementsCache(10) {}

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        m_locater = TestRuleSetLocater::Create();
        m_locaterManager.RegisterLocater(*m_locater);
        m_locale = "test locale";
        }

    void TearDown() override
        {
        m_locaterManager.UnregisterLocater(*m_locater);
        m_locater = nullptr;
        }

    RulesPreprocessor GetTestRulesPreprocessor(PresentationRuleSetCR ruleset)
        {
        return RulesPreprocessor(m_connections, *m_connection, ruleset, m_locale, m_userSettings, nullptr, 
            m_expressionsCache, m_statementsCache);
        }
};
ECDbTestProject* RulesPreprocessorTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_FindsRulesetWithHighestVersion)
    {
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("test", 1, 1, false, "", "", "", false);
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("test", 1, 2, false, "", "", "", false);
    m_locater->AddRuleSet(*rules1);
    m_locater->AddRuleSet(*rules2);
    
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    ASSERT_EQ(foundRules, rules2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_DoesntFindRulesetWithVersionTooHigh)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 2, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    ASSERT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_DoesntFindSupplementalRuleset)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, true, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    ASSERT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_KeepsPrimaryRulesetProperties)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary", 1, 0, false, "", "", "preferred_image", true);
    primary->SetSearchClasses("class1;class2");
    primary->SetExtendedData("test");
    PresentationRuleSetPtr supplemental = PresentationRuleSet::CreateInstance("supplementing", 1, 1, true, "a", "", "", false);
    m_locater->AddRuleSet(*primary);
    m_locater->AddRuleSet(*supplemental);
    
    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    ASSERT_TRUE(rules.IsValid());
    EXPECT_STREQ(primary->GetRuleSetId().c_str(), rules->GetRuleSetId().c_str());
    EXPECT_EQ(primary->GetVersionMajor(), rules->GetVersionMajor());
    EXPECT_EQ(primary->GetVersionMinor(), rules->GetVersionMinor());
    EXPECT_FALSE(rules->GetIsSupplemental());
    EXPECT_STREQ("", rules->GetSupplementationPurpose().c_str());
    EXPECT_STREQ(primary->GetSupportedSchemas().c_str(), rules->GetSupportedSchemas().c_str());
    EXPECT_STREQ(primary->GetPreferredImage().c_str(), rules->GetPreferredImage().c_str());
    EXPECT_EQ(primary->GetIsSearchEnabled(), rules->GetIsSearchEnabled());
    EXPECT_STREQ(primary->GetSearchClasses().c_str(), rules->GetSearchClasses().c_str());
    EXPECT_STREQ(primary->GetExtendedData().c_str(), rules->GetExtendedData().c_str());
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
    ruleset.AddPresentationRule(*new InstanceLabelOverride());
    ruleset.AddPresentationRule(*new StyleOverride());
    ruleset.AddPresentationRule(*new GroupingRule());
    ruleset.AddPresentationRule(*new LocalizationResourceKeyDefinition());
    ruleset.AddPresentationRule(*new UserSettingsGroup());
    ruleset.AddPresentationRule(*new CheckBoxRule());
    ruleset.AddPresentationRule(*new SortingRule());
    ruleset.AddPresentationRule(*new ContentModifier());
    ruleset.AddPresentationRule(*new ExtendedDataRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_MergesRules)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary", 1, 0, false, "", "", "", false);
    AddRulesToRuleset(*primary);
    PresentationRuleSetPtr supplemental = PresentationRuleSet::CreateInstance("supplementing", 1, 1, true, "a", "", "", false);
    AddRulesToRuleset(*supplemental);
    m_locater->AddRuleSet(*primary);
    m_locater->AddRuleSet(*supplemental);

    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    ASSERT_TRUE(rules.IsValid());
    EXPECT_EQ(2, rules->GetRootNodesRules().size());
    EXPECT_EQ(2, rules->GetChildNodesRules().size());
    EXPECT_EQ(2, rules->GetContentRules().size());
    EXPECT_EQ(2, rules->GetImageIdOverrides().size());
    EXPECT_EQ(2, rules->GetLabelOverrides().size());
    EXPECT_EQ(2, rules->GetInstanceLabelOverrides().size());
    EXPECT_EQ(2, rules->GetStyleOverrides().size());
    EXPECT_EQ(2, rules->GetGroupingRules().size());
    EXPECT_EQ(2, rules->GetLocalizationResourceKeyDefinitions().size());
    EXPECT_EQ(2, rules->GetUserSettings().size());
    EXPECT_EQ(2, rules->GetCheckBoxRules().size());
    EXPECT_EQ(2, rules->GetSortingRules().size());
    EXPECT_EQ(2, rules->GetContentModifierRules().size());
    EXPECT_EQ(2, rules->GetExtendedDataRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_MergesRulesWithHigherVersion)
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

    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    ASSERT_TRUE(rules.IsValid());
    EXPECT_EQ(3, rules->GetRootNodesRules().size());
    EXPECT_EQ(3, rules->GetChildNodesRules().size());
    EXPECT_EQ(3, rules->GetContentRules().size());
    EXPECT_EQ(3, rules->GetImageIdOverrides().size());
    EXPECT_EQ(3, rules->GetLabelOverrides().size());
    EXPECT_EQ(3, rules->GetInstanceLabelOverrides().size());
    EXPECT_EQ(3, rules->GetStyleOverrides().size());
    EXPECT_EQ(3, rules->GetGroupingRules().size());
    EXPECT_EQ(3, rules->GetLocalizationResourceKeyDefinitions().size());
    EXPECT_EQ(3, rules->GetUserSettings().size());
    EXPECT_EQ(3, rules->GetCheckBoxRules().size());
    EXPECT_EQ(3, rules->GetSortingRules().size());
    EXPECT_EQ(3, rules->GetContentModifierRules().size());
    EXPECT_EQ(3, rules->GetExtendedDataRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_UsesOnlySupplementalRulesetsWithSupportedSchemasOrEmptySupportedSchemas)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary", 1, 0, false, "", "", "", false);
    AddRulesToRuleset(*primary);
    m_locater->AddRuleSet(*primary);

    PresentationRuleSetPtr supplemental1 = PresentationRuleSet::CreateInstance("supplementing1", 1, 0, true, "a", "RulesEngineTest", "", false);
    supplemental1->AddPresentationRule(*new LabelOverride());
    m_locater->AddRuleSet(*supplemental1);

    PresentationRuleSetPtr supplemental2 = PresentationRuleSet::CreateInstance("supplementing2", 1, 0, true, "b", "UnsupportedSchema", "", false);
    supplemental2->AddPresentationRule(*new ImageIdOverride());
    m_locater->AddRuleSet(*supplemental2);
    
    PresentationRuleSetPtr supplemental3 = PresentationRuleSet::CreateInstance("supplementing3", 1, 0, true, "c", "", "", false);
    supplemental3->AddPresentationRule(*new StyleOverride());
    m_locater->AddRuleSet(*supplemental3);

    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    EXPECT_EQ(1, rules->GetRootNodesRules().size());
    EXPECT_EQ(1, rules->GetChildNodesRules().size());
    EXPECT_EQ(1, rules->GetContentRules().size());
    EXPECT_EQ(1, rules->GetImageIdOverrides().size());
    EXPECT_EQ(2, rules->GetLabelOverrides().size());
    EXPECT_EQ(1, rules->GetInstanceLabelOverrides().size());
    EXPECT_EQ(2, rules->GetStyleOverrides().size());
    EXPECT_EQ(1, rules->GetGroupingRules().size());
    EXPECT_EQ(1, rules->GetLocalizationResourceKeyDefinitions().size());
    EXPECT_EQ(1, rules->GetUserSettings().size());
    EXPECT_EQ(1, rules->GetCheckBoxRules().size());
    EXPECT_EQ(1, rules->GetSortingRules().size());
    EXPECT_EQ(1, rules->GetContentModifierRules().size());
    EXPECT_EQ(1, rules->GetExtendedDataRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_NoConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new RootNodeRule("", 1, false, TargetTree_MainTree, false));
    rules->GetRootNodesRules()[0]->AddSpecification(*new AllInstanceNodesSpecification());
    
    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_RulesSortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("", 2, false, TargetTree_MainTree, false);
    rule->SetStopFurtherProcessing(true);
    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
    ASSERT_EQ(0, specs.size()); // rule with higher priority has no specs
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_SpecsSortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new RootNodeRule("", 1, false, TargetTree_MainTree, false));
    rules->GetRootNodesRules()[0]->AddSpecification(*new AllInstanceNodesSpecification(2, false, false, false, false, false, ""));
    rules->GetRootNodesRules()[0]->AddSpecification(*new AllInstanceNodesSpecification(3, false, false, false, false, false, ""));
    rules->GetRootNodesRules()[0]->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, ""));
    
    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
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
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("1 = 1", 1, false, TargetTree_MainTree, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithTargetTree)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, true, "1"));
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("", 1, false, TargetTree_SelectionTree, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, true, "2"));
    rules->AddPresentationRule(*rule);
    rule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, true, "3"));
    rules->AddPresentationRule(*rule);
    
    // note: using "supported schemas" as a way to know which rule the specification came from
    RulesPreprocessor preprocessor = GetTestRulesPreprocessor(*rules);
    RulesPreprocessor::RootNodeRuleParameters paramsMainTree(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = preprocessor.GetRootNodeSpecifications(paramsMainTree);
    ASSERT_EQ(2, specs.size());
    ASSERT_STREQ("1", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("3", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());
    
    RulesPreprocessor::RootNodeRuleParameters paramsSelectionTree(TargetTree_SelectionTree);
    specs = preprocessor.GetRootNodeSpecifications(paramsSelectionTree);
    ASSERT_EQ(2, specs.size());
    ASSERT_STREQ("2", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("3", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());
    
    RulesPreprocessor::RootNodeRuleParameters paramsBothTrees(TargetTree_Both);
    specs = preprocessor.GetRootNodeSpecifications(paramsBothTrees);
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
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
    ASSERT_EQ(0, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithStopFurtherProcessing)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);

    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);

    rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->SetStopFurtherProcessing(true);
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
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
    subcondition->AddSpecification(*new AllInstanceNodesSpecification());
    rule->AddSubCondition(*subcondition);

    SubCondition* subcondition2 = new SubCondition();
    subcondition2->AddSpecification(*new AllInstanceNodesSpecification());
    subcondition->AddSubCondition(*subcondition2);

    rules->AddPresentationRule(*rule);
    
    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
    ASSERT_EQ(2, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetChildNodeSpecifications)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->GetChildNodesRules()[0]->AddSpecification(*new AllInstanceNodesSpecification());
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::ChildNodeRuleParameters params(*node, TargetTree_MainTree);
    ChildNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetChildNodeSpecifications(params);
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
    nestedRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "one"));
    nestedRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "two"));
    spec1->AddNestedRule(*nestedRule);
    spec1->AddNestedRule(*stopRule);
    rule->AddSpecification(*spec1);

    AllInstanceNodesSpecification* spec2 = new AllInstanceNodesSpecification();
    nestedRule = new ChildNodeRule();
    nestedRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "three"));
    spec2->AddNestedRule(*nestedRule);
    spec2->AddNestedRule(*new ChildNodeRule(*stopRule));
    rule->AddSpecification(*spec2);

    rules->AddPresentationRule(*rule);
    RulesPreprocessor preprocessor = GetTestRulesPreprocessor(*rules);

    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationHash(spec1->GetHash());
    RulesPreprocessor::ChildNodeRuleParameters params1(*node, TargetTree_MainTree);
    ChildNodeRuleSpecificationsList specs = preprocessor.GetChildNodeSpecifications(params1);

    // all sub-specs of the the spec that generated the node
    ASSERT_EQ(2, specs.size()); 
    // use "supported schemas" string to identify expected specs
    ASSERT_STREQ("one", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("two", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());

    nodeExtendedDataWriter.SetSpecificationHash(spec2->GetHash());
    RulesPreprocessor::ChildNodeRuleParameters params2(*node, TargetTree_MainTree);
    specs = preprocessor.GetChildNodeSpecifications(params2);
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
    spec->SetHasChildren(ChildrenHint::Unknown);
    spec->SetHideNodesInHierarchy(false);
    spec->SetHideIfNoChildren(false);
    spec->SetGroupByClass(false);
    spec->SetGroupByLabel(false);
    spec->SetSupportedSchemas("requested");

    ChildNodeRule* nestedRule = new ChildNodeRule();
    nestedRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "one"));
    nestedRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "two"));
    spec->AddNestedRule(*nestedRule);
    ChildNodeRule* stopRule = new ChildNodeRule();
    stopRule->SetStopFurtherProcessing(true);
    spec->AddNestedRule(*stopRule);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    ChildNodeRule* rule = new ChildNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationHash(spec->GetHash());
    nodeExtendedDataWriter.SetRequestedSpecification(true);

    RulesPreprocessor::ChildNodeRuleParameters params(*node, TargetTree_MainTree);
    ChildNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetChildNodeSpecifications(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    StyleOverrideCP styleOverride = GetTestRulesPreprocessor(*rules).GetStyleOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    StyleOverrideCP styleOverride = GetTestRulesPreprocessor(*rules).GetStyleOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    StyleOverrideCP styleOverride = GetTestRulesPreprocessor(*rules).GetStyleOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0);
    bvector<SortingRuleCP> sortingRules = GetTestRulesPreprocessor(*rules).GetSortingRules(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection, NAVNODE_TYPE_ECPropertyGroupingNode);
    NavNodeExtendedData extendedData(*node);
    extendedData.SetPropertyName("TestProperty");
    
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0);
    bvector<SortingRuleCP> sortingRules = GetTestRulesPreprocessor(*rules).GetSortingRules(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0);
    bvector<GroupingRuleCP> groupingRules = GetTestRulesPreprocessor(*rules).GetGroupingRules(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection, NAVNODE_TYPE_ECPropertyGroupingNode);
    NavNodeExtendedData extendedData(*node);
    extendedData.SetPropertyName("TestProperty");
    
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0);
    bvector<GroupingRuleCP> groupingRules = GetTestRulesPreprocessor(*rules).GetGroupingRules(params);
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
    
    RulesPreprocessor::AggregateCustomizationRuleParameters params(nullptr, 0);
    bvector<GroupingRuleCP> groupingRules = GetTestRulesPreprocessor(*rules).GetGroupingRules(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    ImageIdOverrideCP imageOverride = GetTestRulesPreprocessor(*rules).GetImageIdOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    ImageIdOverrideCP imageOverride = GetTestRulesPreprocessor(*rules).GetImageIdOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    ImageIdOverrideCP imageOverride = GetTestRulesPreprocessor(*rules).GetImageIdOverride(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    CheckBoxRuleCP checkboxRule = GetTestRulesPreprocessor(*rules).GetCheckboxRule(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    CheckBoxRuleCP checkboxRule = GetTestRulesPreprocessor(*rules).GetCheckboxRule(params);
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
    
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    CheckBoxRuleCP checkboxRule = GetTestRulesPreprocessor(*rules).GetCheckboxRule(params);
    ASSERT_TRUE(nullptr == checkboxRule);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetExtendedDataRules)
    {
    ExtendedDataRuleP rule1 = new ExtendedDataRule("1 = 1");
    ExtendedDataRuleP rule2 = new ExtendedDataRule("2 = 3");
    ExtendedDataRuleP rule3 = new ExtendedDataRule("4 = 4");
    ExtendedDataRuleP rule4 = new ExtendedDataRule("5 = 6");

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);

    RootNodeRuleP navigationRule = new RootNodeRule();
    rules->AddPresentationRule(*navigationRule);

    InstanceNodesOfSpecificClassesSpecificationP navigationSpec = new InstanceNodesOfSpecificClassesSpecification();
    navigationRule->AddSpecification(*navigationSpec);
    navigationRule->AddCustomizationRule(*rule3);
    navigationRule->AddCustomizationRule(*rule4);

    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeExtendedData(*node).SetSpecificationHash(navigationSpec->GetHash());

    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    bvector<ExtendedDataRuleCP> extendedDataRules = GetTestRulesPreprocessor(*rules).GetExtendedDataRules(params);
    ASSERT_EQ(2, extendedDataRules.size());
    // note: order is not important
    EXPECT_NE(extendedDataRules.end(), std::find(extendedDataRules.begin(), extendedDataRules.end(), rule1));
    EXPECT_NE(extendedDataRules.end(), std::find(extendedDataRules.begin(), extendedDataRules.end(), rule3));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_NoConditions)
    {
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ContentRule("", 1, false));
    
    TestNodeLocater nodeLocater(*node);
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, &nodeLocater);
    ContentRuleInputKeysList specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_RulesSortedByPriority)
    {
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    ContentRuleP rule1 = new ContentRule("", 1, true);
    rules->AddPresentationRule(*rule1);
    ContentRuleP rule2 = new ContentRule("", 2, true);
    rules->AddPresentationRule(*rule2);
    
    TestNodeLocater nodeLocater(*node);
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, &nodeLocater);
    ContentRuleInputKeysList specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    ASSERT_EQ(rule2, &(*specs.begin()).GetRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_WithConditions)
    {
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    ContentRuleP rule1 = new ContentRule("1 = 2", 1, false);
    rules->AddPresentationRule(*rule1);
    ContentRuleP rule2 = new ContentRule("1 = 1", 1, false);
    rules->AddPresentationRule(*rule2);
    TestNodeLocater nodeLocater(*node);
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, &nodeLocater);
    ContentRuleInputKeysList specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    ASSERT_EQ(rule2, &(*specs.begin()).GetRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_ReturnsMultipleRulesIfOnlyIfNotHandledIsFalse)
    {
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ContentRule("", 1, false));
    rules->AddPresentationRule(*new ContentRule("", 1, false));
    
    TestNodeLocater nodeLocater(*node);
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, &nodeLocater);
    ContentRuleInputKeysList specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(2, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_ReturnsOneRuleIfOnlyIfNotHandledIsTrue)
    {
    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false);
    rules->AddPresentationRule(*new ContentRule("", 1, true));
    rules->AddPresentationRule(*new ContentRule("", 1, true));
    
    TestNodeLocater nodeLocater(*node);
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, &nodeLocater);
    ContentRuleInputKeysList specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
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
    nestedChildNodeRule->AddCustomizationRule(*new GroupingRule("", 1, false, "TestSchemaName3", "", "", "", ""));
    spec->AddNestedRule(*nestedChildNodeRule);
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new GroupingRule("", 1, false, "TestSchemaName2", "", "", "", ""));

    RulesPreprocessor::AggregateCustomizationRuleParameters params(nullptr, spec->GetHash());
    bvector<GroupingRuleCP> groupingRules = GetTestRulesPreprocessor(*rules).GetGroupingRules(params);
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
    nestedChildNodeRule->AddCustomizationRule(*new GroupingRule("", 2, false, "TestSchemaName2", "", "", "", ""));
    nestedChildNodeRule->AddSpecification(*nestedSpec);
    spec->AddNestedRule(*nestedChildNodeRule);
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new GroupingRule("", 1, false, "TestSchemaName3", "", "", "", ""));

    RulesPreprocessor::AggregateCustomizationRuleParameters params(nullptr, nestedSpec->GetHash());
    bvector<GroupingRuleCP> groupingRules = GetTestRulesPreprocessor(*rules).GetGroupingRules(params);
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
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new LabelOverride("", 5, "LabelOverrideLabelValue1", "LabelOverrideDescriptionValue"));
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new LabelOverride("", 5, "LabelOverrideLabelValue2", "LabelOverrideDescriptionValue2"));

    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationHash(spec->GetHash());

    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
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
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new ImageIdOverride("", 1, "ImageIdOverrideTestValue1"));
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new ImageIdOverride("", 3, "ImageIdOverrideTestValue2"));

    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationHash(spec->GetHash());

    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    ImageIdOverrideCP imageOverride = GetTestRulesPreprocessor(*rules).GetImageIdOverride(params);
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
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new StyleOverride("", 1, "Blue", "Red", "Bold"));

    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationHash(spec->GetHash());

    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    StyleOverrideCP styleOverride = GetTestRulesPreprocessor(*rules).GetStyleOverride(params);
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
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new CheckBoxRule("", 1, false, "CheckBoxPropertyName1", false, false, ""));

    TestNavNodePtr node = TestNavNode::Create(*m_connection); 
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationHash(spec->GetHash());

    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    CheckBoxRuleCP checkBox = GetTestRulesPreprocessor(*rules).GetCheckboxRule(params);
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
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new LabelOverride("2=3", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue"));
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new LabelOverride("2=2", 1, "LabelOverrideLabelValue2", "LabelOverrideDescriptionValue2"));

    TestNavNodePtr node = TestNavNode::Create(*m_connection);
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetSpecificationHash(spec->GetHash());

    RulesPreprocessor::CustomizationRuleParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    // Same scope, conditions do not match, different priorities - return condition true
    ASSERT_TRUE(nullptr != labelOverride);
    EXPECT_STREQ("LabelOverrideLabelValue2", labelOverride->GetLabel().c_str());
    }
