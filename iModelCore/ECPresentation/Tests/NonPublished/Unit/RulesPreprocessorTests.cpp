/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/ExtendedData.h>
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>
#include "../../../Source/Shared/RulesPreprocessor.h"
#include "../Helpers/TestNavNode.h"
#include "../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesPreprocessorTests : ECPresentationTest
{
protected:
    static ECDbTestProject* s_project;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    TestRuleSetLocaterPtr m_locater;
    RuleSetLocaterManager m_locaterManager;
    RulesetVariables m_rulesetVariables;
    ECExpressionsCache m_expressionsCache;

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

    RulesPreprocessorTests() : m_locaterManager(m_connections) {}

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        m_locater = TestRuleSetLocater::Create();
        m_locaterManager.RegisterLocater(*m_locater);
        }

    void TearDown() override
        {
        m_locaterManager.UnregisterLocater(*m_locater);
        m_locater = nullptr;
        }

    RulesPreprocessor GetTestRulesPreprocessor(PresentationRuleSetCR ruleset)
        {
        return RulesPreprocessor(m_connections, *m_connection, ruleset, m_rulesetVariables, nullptr,
            m_expressionsCache);
        }
};
ECDbTestProject* RulesPreprocessorTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_FindsRulesetWithHighestVersion)
    {
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("test");
    rules1->SetRulesetVersion(Version(1, 1, 0));
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("test");
    rules2->SetRulesetVersion(Version(1, 2, 0));
    m_locater->AddRuleSet(*rules1);
    m_locater->AddRuleSet(*rules2);

    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    ASSERT_EQ(foundRules, rules2);
    ASSERT_EQ(foundRules->GetHash(), rules2->GetHash());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_DoesntFindRulesetSchemaWithVersionTooHigh)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->SetRulesetSchemaVersion(Version(PRESENTATION_RULES_SCHEMA_VERSION_MAJOR + 1, 0, 0));
    m_locater->AddRuleSet(*rules);
    ASSERT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_DoesntFindSupplementalRuleset)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->SetIsSupplemental(true);
    m_locater->AddRuleSet(*rules);
    ASSERT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, ReturnOnlySupportedRulesets_ThroughSupportedSchemas_ReturnsRulesetWithoutSupportedSchemas)
    {
    m_locater->AddRuleSet(*PresentationRuleSet::CreateInstance("EmptySupportedSchemas"));
    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    EXPECT_TRUE(rules.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, ReturnOnlySupportedRulesets_ThroughSupportedSchemas_ReturnsRulesetWhenAllSchemasSupported)
    {
    auto ruleset = PresentationRuleSet::CreateInstance("SupportedSchema");
    ruleset->SetSupportedSchemas("RulesEngineTest");
    m_locater->AddRuleSet(*ruleset);
    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    EXPECT_TRUE(rules.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, ReturnOnlySupportedRulesets_ThroughSupportedSchemas_DoesntReturnRulesetIfNoneOfTheSchemasAreSupported)
    {
    auto ruleset = PresentationRuleSet::CreateInstance("UnsupportedSchema");
    ruleset->SetSupportedSchemas("UnsupportedSchema1,UnsupportedSchema2");
    m_locater->AddRuleSet(*ruleset);
    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    EXPECT_TRUE(rules.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, ReturnOnlySupportedRulesets_ThroughSupportedSchemas_DoesntReturnRulesetIfAtLeastOneSchemaIsNotSupported)
    {
    auto ruleset = PresentationRuleSet::CreateInstance("SupportedAndNotSupportedSchemas");
    ruleset->SetSupportedSchemas("RulesEngineTest,UnsupportedSchema");
    m_locater->AddRuleSet(*ruleset);
    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    EXPECT_TRUE(rules.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, ReturnOnlySupportedRulesets_ThroughRequiredSchemasSchemas_ReturnsRulesetWithoutRequiredSchemas)
    {
    m_locater->AddRuleSet(*PresentationRuleSet::CreateInstance("test"));
    EXPECT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, ReturnOnlySupportedRulesets_ThroughRequiredSchemasSchemas_ReturnsRulesetWithAllRequiredSchemas)
    {
    auto ruleset = PresentationRuleSet::CreateInstance("test");
    ruleset->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest"));
    m_locater->AddRuleSet(*ruleset);
    EXPECT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, ReturnOnlySupportedRulesets_ThroughRequiredSchemasSchemas_DoesntReturnRulesetWithoutRequiredSchema)
    {
    auto ruleset = PresentationRuleSet::CreateInstance("test");
    ruleset->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("DoesNotExist"));
    m_locater->AddRuleSet(*ruleset);
    EXPECT_TRUE(RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_KeepsPrimaryRulesetProperties)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary");
    primary->SetRulesetSchemaVersion(Version(1, 2, 3));
    primary->SetRulesetVersion(Version(4, 5, 6));
    primary->SetSearchClasses("class1;class2");
    primary->SetExtendedData("test");
    m_locater->AddRuleSet(*primary);

    PresentationRuleSetPtr supplemental = PresentationRuleSet::CreateInstance("supplementing");
    m_locater->AddRuleSet(*supplemental);

    PresentationRuleSetPtr rules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);
    ASSERT_TRUE(rules.IsValid());
    EXPECT_STREQ(primary->GetRuleSetId().c_str(), rules->GetRuleSetId().c_str());
    EXPECT_EQ(primary->GetRulesetSchemaVersion(), rules->GetRulesetSchemaVersion());
    EXPECT_EQ(primary->GetRulesetVersion(), rules->GetRulesetVersion());
    EXPECT_FALSE(rules->GetIsSupplemental());
    EXPECT_STREQ("", rules->GetSupplementationPurpose().c_str());
    EXPECT_STREQ(primary->GetSupportedSchemas().c_str(), rules->GetSupportedSchemas().c_str());
    EXPECT_STREQ(primary->GetPreferredImage().c_str(), rules->GetPreferredImage().c_str());
    EXPECT_EQ(primary->GetIsSearchEnabled(), rules->GetIsSearchEnabled());
    EXPECT_STREQ(primary->GetSearchClasses().c_str(), rules->GetSearchClasses().c_str());
    EXPECT_STREQ(primary->GetExtendedData().c_str(), rules->GetExtendedData().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddRulesToRuleset(PresentationRuleSetR ruleset, int priority = 0)
    {
    // different priority for RootNodeRule and ChildNodeRule, so they won't be merged together as the same rules after ruleset supplemention
    ruleset.AddPresentationRule(*new RootNodeRule("", priority, false, TargetTree_MainTree, false));
    ruleset.AddPresentationRule(*new ChildNodeRule("", priority, false, TargetTree_MainTree));
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
    ruleset.AddPresentationRule(*new NodeArtifactsRule());
    ruleset.AddPresentationRule(*new DefaultPropertyCategoryOverride());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_MergesRules)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary");
    AddRulesToRuleset(*primary, 1);
    PresentationRuleSetPtr supplemental = PresentationRuleSet::CreateInstance("supplementing");
    supplemental->SetIsSupplemental(true);
    AddRulesToRuleset(*supplemental, 2);
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
    EXPECT_EQ(2, rules->GetNodeArtifactRules().size());
    EXPECT_EQ(2, rules->GetDefaultPropertyCategoryOverrides().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_MergesRulesWithHigherVersion)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary");
    AddRulesToRuleset(*primary, 1);
    m_locater->AddRuleSet(*primary);

    PresentationRuleSetPtr supplemental1 = PresentationRuleSet::CreateInstance("supplementing1");
    supplemental1->SetIsSupplemental(true);
    supplemental1->SetRulesetVersion(Version(1, 0, 0));
    AddRulesToRuleset(*supplemental1, 2);
    m_locater->AddRuleSet(*supplemental1);

    PresentationRuleSetPtr supplemental2 = PresentationRuleSet::CreateInstance("supplementing2");
    supplemental2->SetIsSupplemental(true);
    supplemental2->SetRulesetVersion(Version(1, 1, 0));
    AddRulesToRuleset(*supplemental2, 3);
    AddRulesToRuleset(*supplemental2, 4);
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
    EXPECT_EQ(3, rules->GetDefaultPropertyCategoryOverrides().size());
    EXPECT_EQ(3, rules->GetSortingRules().size());
    EXPECT_EQ(3, rules->GetContentModifierRules().size());
    EXPECT_EQ(3, rules->GetExtendedDataRules().size());
    EXPECT_EQ(3, rules->GetNodeArtifactRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_UsesOnlySupplementalRulesetsWithSupportedSchemasOrEmptySupportedSchemas)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary");
    AddRulesToRuleset(*primary);
    m_locater->AddRuleSet(*primary);

    PresentationRuleSetPtr supplemental1 = PresentationRuleSet::CreateInstance("supplementing1");
    supplemental1->SetSupplementationPurpose("a");
    supplemental1->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest"));
    supplemental1->AddPresentationRule(*new LabelOverride());
    m_locater->AddRuleSet(*supplemental1);

    PresentationRuleSetPtr supplemental2 = PresentationRuleSet::CreateInstance("supplementing2");
    supplemental2->SetSupplementationPurpose("b");
    supplemental2->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("UnsupportedSchema"));
    supplemental2->AddPresentationRule(*new ImageIdOverride());
    m_locater->AddRuleSet(*supplemental2);

    PresentationRuleSetPtr supplemental3 = PresentationRuleSet::CreateInstance("supplementing3");
    supplemental3->SetSupplementationPurpose("c");
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
    EXPECT_EQ(1, rules->GetNodeArtifactRules().size());
    EXPECT_EQ(1, rules->GetDefaultPropertyCategoryOverrides().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_Supplementation_UsesOnlySupplementalRulesetsWithSchemaRequirementsMet)
    {
    PresentationRuleSetPtr primary = PresentationRuleSet::CreateInstance("primary");
    AddRulesToRuleset(*primary);
    m_locater->AddRuleSet(*primary);

    PresentationRuleSetPtr supplemental1 = PresentationRuleSet::CreateInstance("supplementing1");
    supplemental1->SetSupplementationPurpose("a");
    supplemental1->SetSupportedSchemas("RulesEngineTest");
    supplemental1->AddPresentationRule(*new LabelOverride());
    m_locater->AddRuleSet(*supplemental1);

    PresentationRuleSetPtr supplemental2 = PresentationRuleSet::CreateInstance("supplementing2");
    supplemental2->SetSupplementationPurpose("b");
    supplemental2->SetSupportedSchemas("UnsupportedSchema");
    supplemental2->AddPresentationRule(*new ImageIdOverride());
    m_locater->AddRuleSet(*supplemental2);

    PresentationRuleSetPtr supplemental3 = PresentationRuleSet::CreateInstance("supplementing3");
    supplemental3->SetSupplementationPurpose("c");
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
    EXPECT_EQ(1, rules->GetNodeArtifactRules().size());
    EXPECT_EQ(1, rules->GetDefaultPropertyCategoryOverrides().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_NoConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new RootNodeRule("", 1, false, TargetTree_MainTree, false));
    rules->GetRootNodesRules()[0]->AddSpecification(*new AllInstanceNodesSpecification());

    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_RulesSortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_SpecsSortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetRootNodeSpecifications_WithRequiredSchemas)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");

    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", Version(1, 0, 0)));
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);

    rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", Version(2, 0, 0)));
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);

    rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rules->AddPresentationRule(*rule);

    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
    ASSERT_EQ(2, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithTargetTree)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithOnlyIfNotHandled)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_WithStopFurtherProcessing)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");

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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetRootNodeSpecifications_IncludesSpecsFromSubConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    RootNodeRule* rule = new RootNodeRule("", 1, false, TargetTree_MainTree, false);
    rules->AddPresentationRule(*rule);

    // with truthy condition
    SubCondition* subcondition1 = new SubCondition("1 = 1");
    subcondition1->AddSpecification(*new AllInstanceNodesSpecification());
    rule->AddSubCondition(*subcondition1);

    // without falsy condition
    SubCondition* subcondition2 = new SubCondition("1 = 2");
    subcondition2->AddSpecification(*new AllInstanceNodesSpecification());
    rule->AddSubCondition(*subcondition2);

    // with passing schema requirement
    SubCondition* subcondition3 = new SubCondition();
    subcondition3->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", Version(1, 0, 0)));
    subcondition3->AddSpecification(*new AllInstanceNodesSpecification());
    rule->AddSubCondition(*subcondition3);

    // with failing schema requirement
    SubCondition* subcondition4 = new SubCondition();
    subcondition4->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", Version(2, 0, 0)));
    subcondition4->AddSpecification(*new AllInstanceNodesSpecification());
    rule->AddSubCondition(*subcondition4);

    // without condition or schema requirements
    SubCondition* subcondition5 = new SubCondition();
    subcondition5->AddSpecification(*new AllInstanceNodesSpecification());
    rule->AddSubCondition(*subcondition5);

    RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
    RootNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetRootNodeSpecifications(params);
    ASSERT_EQ(3, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetChildNodeSpecifications)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->GetChildNodesRules()[0]->AddSpecification(*new AllInstanceNodesSpecification());

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::ChildNodeRuleParameters params(*node, TargetTree_MainTree);
    ChildNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetChildNodeSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetChildNodeSpecifications_BySpecificationId)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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

    auto node1 = TestNodesFactory(*m_connection, spec1->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");
    RulesPreprocessor::ChildNodeRuleParameters params1(*node1, TargetTree_MainTree);
    ChildNodeRuleSpecificationsList specs = preprocessor.GetChildNodeSpecifications(params1);

    // all sub-specs of the the spec that generated the node
    ASSERT_EQ(2, specs.size());
    // use "supported schemas" string to identify expected specs
    ASSERT_STREQ("one", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("two", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());

    auto node2 = TestNodesFactory(*m_connection, spec2->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");
    RulesPreprocessor::ChildNodeRuleParameters params2(*node2, TargetTree_MainTree);
    specs = preprocessor.GetChildNodeSpecifications(params2);
    // all sub-specs of the the spec that generated the node
    ASSERT_EQ(1, specs.size());
    ASSERT_STREQ("three", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetChildNodeSpecifications_FindsChildNodeSpecificationById)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    auto rootRule1 = new RootNodeRule();
    auto rootSpec1 = new AllInstanceNodesSpecification();

    auto nestedRule1 = new ChildNodeRule();
    nestedRule1->SetCondition("FALSE");
    auto nestedSpec1 = new AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    auto deeplyNestedRule1 = new ChildNodeRule();
    deeplyNestedRule1->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "two"));
    nestedSpec1->AddNestedRule(*deeplyNestedRule1);
    auto deeplyNestedRule2 = new ChildNodeRule();
    deeplyNestedRule2->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "three"));
    nestedSpec1->AddNestedRule(*deeplyNestedRule2);
    nestedRule1->AddSpecification(*nestedSpec1);
    rootSpec1->AddNestedRule(*nestedRule1);

    auto nestedRule2 = new ChildNodeRule();
    nestedRule2->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "four"));
    rootSpec1->AddNestedRule(*nestedRule2);

    rootRule1->AddSpecification(*rootSpec1);
    rules->AddPresentationRule(*rootRule1);

    auto rootRule2 = new ChildNodeRule();
    rootRule2->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "five"));
    rules->AddPresentationRule(*rootRule2);

    RulesPreprocessor preprocessor = GetTestRulesPreprocessor(*rules);

    auto node1 = TestNodesFactory(*m_connection, nestedSpec1->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");
    RulesPreprocessor::ChildNodeRuleParameters params1(*node1, TargetTree_MainTree);
    ChildNodeRuleSpecificationsList specs = preprocessor.GetChildNodeSpecifications(params1);

    // all sub-specs of the the spec that generated the node plus specs from root level
    ASSERT_EQ(3, specs.size());
    // use "supported schemas" string to identify expected specs
    ASSERT_STREQ("two", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("three", (static_cast<AllInstanceNodesSpecificationCP>(&specs[1].GetSpecification()))->GetSupportedSchemas().c_str());
    ASSERT_STREQ("five", (static_cast<AllInstanceNodesSpecificationCP>(&specs[2].GetSpecification()))->GetSupportedSchemas().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    ChildNodeRule* rule = new ChildNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification());
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    auto node = TestNodesFactory(*m_connection, spec->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");
    NavNodeExtendedData nodeExtendedDataWriter(*node);
    nodeExtendedDataWriter.SetRequestedSpecification(true);

    RulesPreprocessor::ChildNodeRuleParameters params(*node, TargetTree_MainTree);
    ChildNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetChildNodeSpecifications(params);
    // all sub-specs of the the spec that generated the node
    ASSERT_EQ(1, specs.size());
    // use "supported schemas" string to identify expected specs
    ASSERT_STREQ("requested", (static_cast<AllInstanceNodesSpecificationCP>(&specs[0].GetSpecification()))->GetSupportedSchemas().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetChildNodeSpecifications_FindsRulesFromCorrectContexts)
    {
    auto rootNodeRule = new RootNodeRule();
    auto rootNodeSpec = new CustomNodeSpecification(1, false, "T_ROOT", "Root", "", "");
    rootNodeRule->AddSpecification(*rootNodeSpec);

    auto nestedChildNodeRule = new ChildNodeRule();
    rootNodeSpec->AddNestedRule(*nestedChildNodeRule);
    auto childNode1Spec = new CustomNodeSpecification(1, false, "T_CHILD_1", "Child1", "", "");
    auto childNode2Spec = new CustomNodeSpecification(1, false, "T_CHILD_2", "Child2", "", "");
    nestedChildNodeRule->AddSpecification(*childNode1Spec);
    nestedChildNodeRule->AddSpecification(*childNode2Spec);

    auto nestedChild2NodeRule = new ChildNodeRule();
    childNode2Spec->AddNestedRule(*nestedChild2NodeRule);
    auto childNode21Spec = new CustomNodeSpecification(1, false, "T_CHILD_2_1", "Child2_1", "", "");
    nestedChild2NodeRule->AddSpecification(*childNode21Spec);

    auto rootChildNodeRule = new ChildNodeRule();
    auto rootChildNodeSpec = new CustomNodeSpecification(1, false, "T_CHILD", "Child", "", "");
    rootChildNodeRule->AddSpecification(*rootChildNodeSpec);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rootNodeRule);
    rules->AddPresentationRule(*rootChildNodeRule);

    auto parentNode = TestNodesFactory(*m_connection, childNode2Spec->GetHash(), "test").CreateCustomNode(nullptr, "Child2", "", "", "T_CHILD_2");
    RulesPreprocessor::ChildNodeRuleParameters params(*parentNode, TargetTree_MainTree);
    ChildNodeRuleSpecificationsList specs = GetTestRulesPreprocessor(*rules).GetChildNodeSpecifications(params);
    // only rootChildeNodeRule and nestedChild2NodeRule should be used for createing 'Child2' children
    ASSERT_EQ(2, specs.size());
    EXPECT_TRUE(specs.end() != std::find_if(specs.begin(), specs.end(), [&](auto const& spec) {return &spec.GetSpecification() == childNode21Spec; }));
    EXPECT_TRUE(specs.end() != std::find_if(specs.begin(), specs.end(), [&](auto const& spec) {return &spec.GetSpecification() == rootChildNodeSpec; }));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLabelOverride_WithoutCondition)
    {
    LabelOverride* rule = new LabelOverride("", 1, "GetLabelOverride_WithoutCondition", "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    ASSERT_TRUE(nullptr != labelOverride);
    ASSERT_STREQ("GetLabelOverride_WithoutCondition", labelOverride->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLabelOverride_WithoutLabelAndDescription)
    {
    LabelOverride* rule = new LabelOverride("", 1, "", "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    ASSERT_TRUE(nullptr == labelOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLabelOverride_WithMatchingCondition)
    {
    LabelOverride* rule = new LabelOverride("Not ThisNode.IsInstanceNode", 1, "GetLabelOverride_WithMatchingCondition", "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    ASSERT_TRUE(nullptr != labelOverride);
    ASSERT_STREQ("GetLabelOverride_WithMatchingCondition", labelOverride->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLabelOverride_WithNoMatchingCondition)
    {
    LabelOverride* rule = new LabelOverride("ThisNode.IsInstanceNode", 1, "GetLabelOverride_WithNoMatchingCondition", "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    ASSERT_TRUE(nullptr == labelOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetLabelOverride_WithPassingSchemaRequirement)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    LabelOverride* rule = new LabelOverride("", 1, BeTest::GetNameOfCurrentTest(), "");
    rule->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", Version(1, 0, 0)));
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    ASSERT_TRUE(rule == labelOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetLabelOverride_WithFailingSchemaRequirement)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    LabelOverride* rule = new LabelOverride("", 1, BeTest::GetNameOfCurrentTest(), "");
    rule->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", Version(2, 0, 0)));
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    ASSERT_TRUE(nullptr == labelOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetStyleOverride_WithoutCondition)
    {
    StyleOverrideP rule = new StyleOverride("", 1, "FC", "BC", "FS");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    StyleOverrideCP styleOverride = GetTestRulesPreprocessor(*rules).GetStyleOverride(params);
    ASSERT_TRUE(nullptr != styleOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetStyleOverride_WithMatchingCondition)
    {
    StyleOverrideP rule = new StyleOverride("Not ThisNode.IsInstanceNode", 1, "FC", "BC", "FS");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    StyleOverrideCP styleOverride = GetTestRulesPreprocessor(*rules).GetStyleOverride(params);
    ASSERT_TRUE(nullptr != styleOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetStyleOverride_WithNoMatchingCondition)
    {
    StyleOverrideP rule = new StyleOverride("ThisNode.IsInstanceNode", 1, "FC", "BC", "FS");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    StyleOverrideCP styleOverride = GetTestRulesPreprocessor(*rules).GetStyleOverride(params);
    ASSERT_TRUE(nullptr == styleOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetSortingRules_SortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    SortingRule* rule1 = new SortingRule("", 1, "", "", "", true, false, false);
    SortingRule* rule2 = new SortingRule("", 3, "", "", "", true, false, false);
    SortingRule* rule3 = new SortingRule("", 2, "", "", "", true, false, false);
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);
    rules->AddPresentationRule(*rule3);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0);
    bvector<SortingRuleCP> sortingRules = GetTestRulesPreprocessor(*rules).GetSortingRules(params);
    ASSERT_EQ(3, sortingRules.size());
    EXPECT_EQ(3, sortingRules[0]->GetPriority());
    EXPECT_EQ(2, sortingRules[1]->GetPriority());
    EXPECT_EQ(1, sortingRules[2]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetSortingRules_VerifiesCondition)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    SortingRule* rule1 = new SortingRule("ParentNode.IsInstanceNode", 1, "", "", "", true, false, false);
    SortingRule* rule2 = new SortingRule("ParentNode.IsPropertyGroupingNode", 2, "", "", "", true, false, false);
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, NAVNODE_TYPE_ECPropertyGroupingNode, "TestLabel", "");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetPropertyName("TestProperty");

    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0);
    bvector<SortingRuleCP> sortingRules = GetTestRulesPreprocessor(*rules).GetSortingRules(params);
    ASSERT_EQ(1, sortingRules.size());
    EXPECT_EQ(2, sortingRules[0]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetGroupingRules_SortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    GroupingRule* rule1 = new GroupingRule("", 1, false, "", "", "", "", "");
    GroupingRule* rule2 = new GroupingRule("", 3, false, "", "", "", "", "");
    GroupingRule* rule3 = new GroupingRule("", 2, false, "", "", "", "", "");
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);
    rules->AddPresentationRule(*rule3);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0);
    bvector<GroupingRuleCP> groupingRules = GetTestRulesPreprocessor(*rules).GetGroupingRules(params);
    ASSERT_EQ(3, groupingRules.size());
    EXPECT_EQ(3, groupingRules[0]->GetPriority());
    EXPECT_EQ(2, groupingRules[1]->GetPriority());
    EXPECT_EQ(1, groupingRules[2]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetGroupingRules_VerifiesCondition)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    GroupingRule* rule1 = new GroupingRule("ParentNode.IsInstanceNode", 1, false, "", "", "", "", "");
    GroupingRule* rule2 = new GroupingRule("ParentNode.IsPropertyGroupingNode", 2, false, "", "", "", "", "");
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, NAVNODE_TYPE_ECPropertyGroupingNode, "TestLabel", "");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetPropertyName("TestProperty");

    RulesPreprocessor::AggregateCustomizationRuleParameters params(node.get(), 0);
    bvector<GroupingRuleCP> groupingRules = GetTestRulesPreprocessor(*rules).GetGroupingRules(params);
    ASSERT_EQ(1, groupingRules.size());
    EXPECT_EQ(2, groupingRules[0]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetGroupingRules_OnlyIfNotHandledFlagHandled)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLocalizationResourceKeyDefinion_WithMatchingId)
    {
    LocalizationResourceKeyDefinitionP definition = new LocalizationResourceKeyDefinition(1, "test_id", "key", "default_value");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*definition);

    LocalizationResourceKeyDefinitionCP foundDefinition = RulesPreprocessor::GetLocalizationResourceKeyDefinition("test_id", *rules);
    ASSERT_TRUE(foundDefinition == definition);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetLocalizationResourceKeyDefinion_WithoutMatchingId)
    {
    LocalizationResourceKeyDefinitionP definition = new LocalizationResourceKeyDefinition(1, "test_id", "key", "default_value");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*definition);

    LocalizationResourceKeyDefinitionCP foundDefinition = RulesPreprocessor::GetLocalizationResourceKeyDefinition("does_not_exist", *rules);
    ASSERT_TRUE(nullptr == foundDefinition);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetImageIdOverride_WithoutCondition)
    {
    ImageIdOverrideP rule = new ImageIdOverride("", 1, "ImageId");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    ImageIdOverrideCP imageOverride = GetTestRulesPreprocessor(*rules).GetImageIdOverride(params);
    ASSERT_TRUE(nullptr != imageOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetImageIdOverride_WithMatchingCondition)
    {
    ImageIdOverrideP rule = new ImageIdOverride("Not ThisNode.IsInstanceNode", 1, "ImageId");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    ImageIdOverrideCP imageOverride = GetTestRulesPreprocessor(*rules).GetImageIdOverride(params);
    ASSERT_TRUE(nullptr != imageOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetImageIdOverride_WithNoMatchingCondition)
    {
    ImageIdOverrideP rule = new ImageIdOverride("ThisNode.IsInstanceNode", 1, "ImageId");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    ImageIdOverrideCP imageOverride = GetTestRulesPreprocessor(*rules).GetImageIdOverride(params);
    ASSERT_TRUE(nullptr == imageOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetCheckboxRule_WithoutCondition)
    {
    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "PropertyValue", false, false, "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    CheckBoxRuleCP checkboxRule = GetTestRulesPreprocessor(*rules).GetCheckboxRule(params);
    ASSERT_TRUE(nullptr != checkboxRule);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetCheckboxRule_WithMatchingCondition)
    {
    CheckBoxRuleP rule = new CheckBoxRule("Not ThisNode.IsInstanceNode", 1, false, "PropertyValue", false, false, "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    CheckBoxRuleCP checkboxRule = GetTestRulesPreprocessor(*rules).GetCheckboxRule(params);
    ASSERT_TRUE(nullptr != checkboxRule);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetCheckboxRule_WithNoMatchingCondition)
    {
    CheckBoxRuleP rule = new CheckBoxRule("ThisNode.IsInstanceNode", 1, false, "PropertyValue", false, false, "");
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    CheckBoxRuleCP checkboxRule = GetTestRulesPreprocessor(*rules).GetCheckboxRule(params);
    ASSERT_TRUE(nullptr == checkboxRule);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetDefaultPropertyCategoryOverride_ReturnsNullWhenThereAreNoOverrides)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    auto result = GetTestRulesPreprocessor(*rules).GetDefaultPropertyCategoryOverride();
    EXPECT_TRUE(nullptr == result);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetDefaultPropertyCategoryOverride_ReturnsOverrideWithHighestPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new DefaultPropertyCategoryOverride(*new PropertyCategorySpecification("1", "1"), 100));
    rules->AddPresentationRule(*new DefaultPropertyCategoryOverride(*new PropertyCategorySpecification("2", "2"), 200));
    auto result = GetTestRulesPreprocessor(*rules).GetDefaultPropertyCategoryOverride();
    EXPECT_STREQ("2", result->GetSpecification().GetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetExtendedDataRules)
    {
    ExtendedDataRuleP rule1 = new ExtendedDataRule("1 = 1");
    ExtendedDataRuleP rule2 = new ExtendedDataRule("2 = 3");
    ExtendedDataRuleP rule3 = new ExtendedDataRule("4 = 4");
    ExtendedDataRuleP rule4 = new ExtendedDataRule("5 = 6");

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);

    RootNodeRuleP navigationRule = new RootNodeRule();
    rules->AddPresentationRule(*navigationRule);

    InstanceNodesOfSpecificClassesSpecificationP navigationSpec = new InstanceNodesOfSpecificClassesSpecification();
    navigationRule->AddSpecification(*navigationSpec);
    navigationRule->AddCustomizationRule(*rule3);
    navigationRule->AddCustomizationRule(*rule4);

    auto node = TestNodesFactory(*m_connection, navigationSpec->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");

    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    bvector<ExtendedDataRuleCP> extendedDataRules = GetTestRulesPreprocessor(*rules).GetExtendedDataRules(params);
    ASSERT_EQ(2, extendedDataRules.size());
    // note: order is not important
    EXPECT_NE(extendedDataRules.end(), std::find(extendedDataRules.begin(), extendedDataRules.end(), rule1));
    EXPECT_NE(extendedDataRules.end(), std::find(extendedDataRules.begin(), extendedDataRules.end(), rule3));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetExtendedDataRules_OnlyIfNotHandledFlagHandled)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    ExtendedDataRuleP rule1 = new ExtendedDataRule("", 1, true, {});
    ExtendedDataRuleP rule2 = new ExtendedDataRule("", 2, false, {});
    ExtendedDataRuleP rule3 = new ExtendedDataRule("", 3, true, {});
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);
    rules->AddPresentationRule(*rule3);

    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    bvector<ExtendedDataRuleCP> extendedDataRules = GetTestRulesPreprocessor(*rules).GetExtendedDataRules(params);
    ASSERT_EQ(2, extendedDataRules.size());
    EXPECT_EQ(rule3, extendedDataRules[0]);
    EXPECT_EQ(rule2, extendedDataRules[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNodeArtifactRules)
    {
    NodeArtifactsRuleP rule1 = new NodeArtifactsRule("1 = 1");
    NodeArtifactsRuleP rule2 = new NodeArtifactsRule("2 = 3");
    NodeArtifactsRuleP rule3 = new NodeArtifactsRule("4 = 4");
    NodeArtifactsRuleP rule4 = new NodeArtifactsRule("5 = 6");

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*rule1);
    rules->AddPresentationRule(*rule2);

    RootNodeRuleP navigationRule = new RootNodeRule();
    rules->AddPresentationRule(*navigationRule);

    InstanceNodesOfSpecificClassesSpecificationP navigationSpec = new InstanceNodesOfSpecificClassesSpecification();
    navigationRule->AddSpecification(*navigationSpec);
    navigationRule->AddCustomizationRule(*rule3);
    navigationRule->AddCustomizationRule(*rule4);

    auto node = TestNodesFactory(*m_connection, navigationSpec->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");

    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    bvector<NodeArtifactsRuleCP> artifactRules = GetTestRulesPreprocessor(*rules).GetNodeArtifactRules(params);
    ASSERT_EQ(2, artifactRules.size());
    // note: order is not important
    EXPECT_NE(artifactRules.end(), std::find(artifactRules.begin(), artifactRules.end(), rule1));
    EXPECT_NE(artifactRules.end(), std::find(artifactRules.begin(), artifactRules.end(), rule3));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_NoConditions)
    {
    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ContentRule("", 1, false));
    TestNodeLabelCalculator testNodeLabelCalculator("test");
    TestNodeLocater nodeLocater(*node);
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, testNodeLabelCalculator, &nodeLocater);
    ContentRuleInputKeysContainer specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_RulesSortedByPriority)
    {
    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    ContentRuleP rule1 = new ContentRule("", 1, true);
    rules->AddPresentationRule(*rule1);
    ContentRuleP rule2 = new ContentRule("", 2, true);
    rules->AddPresentationRule(*rule2);

    TestNodeLabelCalculator testNodeLabelCalculator("test");
    TestNodeLocater nodeLocater(*node);
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, testNodeLabelCalculator, &nodeLocater);
    ContentRuleInputKeysContainer specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    ASSERT_EQ(rule2, &(*specs.begin()).GetRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_WithConditions)
    {
    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    ContentRuleP rule1 = new ContentRule("1 = 2", 1, false);
    rules->AddPresentationRule(*rule1);
    ContentRuleP rule2 = new ContentRule("1 = 1", 1, false);
    rules->AddPresentationRule(*rule2);
    TestNodeLocater nodeLocater(*node);
    TestNodeLabelCalculator testNodeLabelCalculator("test");
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, testNodeLabelCalculator, &nodeLocater);
    ContentRuleInputKeysContainer specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    ASSERT_EQ(rule2, &(*specs.begin()).GetRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetContentSpecifications_WithSchemaRequirements)
    {
    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");

    ContentRuleP rule1 = new ContentRule("", 1, false);
    rule1->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", Version(1, 0, 0)));
    rules->AddPresentationRule(*rule1);

    ContentRuleP rule2 = new ContentRule("", 1, false);
    rule2->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", Version(2, 0, 0)));
    rules->AddPresentationRule(*rule2);

    ContentRuleP rule3 = new ContentRule("", 1, false);
    rule3->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("RulesEngineTest", nullptr, Version(1, 0, 0)));
    rules->AddPresentationRule(*rule3);

    TestNodeLocater nodeLocater(*node);
    TestNodeLabelCalculator testNodeLabelCalculator("test");
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, testNodeLabelCalculator, &nodeLocater);
    ContentRuleInputKeysContainer specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    ASSERT_EQ(rule1, &(*specs.begin()).GetRule());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_ReturnsMultipleRulesIfOnlyIfNotHandledIsFalse)
    {
    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ContentRule("", 1, false));
    rules->AddPresentationRule(*new ContentRule("", 1, false));

    TestNodeLocater nodeLocater(*node);
    TestNodeLabelCalculator testNodeLabelCalculator("test");
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, testNodeLabelCalculator, &nodeLocater);
    ContentRuleInputKeysContainer specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(2, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesPreprocessorTests, GetContentSpecifications_ReturnsOneRuleIfOnlyIfNotHandledIsTrue)
    {
    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "TestLabel", "");
    NavNodeKeyList selectedNodeKeys;
    selectedNodeKeys.push_back(node->GetKey());

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ContentRule("", 1, true));
    rules->AddPresentationRule(*new ContentRule("", 1, true));

    TestNodeLocater nodeLocater(*node);
    TestNodeLabelCalculator testNodeLabelCalculator("test");
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(selectedNodeKeys), "", nullptr, testNodeLabelCalculator, &nodeLocater);
    ContentRuleInputKeysContainer specs = GetTestRulesPreprocessor(*rules).GetContentSpecifications(params);
    ASSERT_EQ(1, specs.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedGroupingRules_DoesNotFindRulesDefinedDeeperThanSpecification_ReturnsSortedByNestingLevel)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedGroupingRules_FindsMostlyNestedRule_ReturnsSortedByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedCustomizationRules_SameScope_SamePriorities_ReturnsFirstRule)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new LabelOverride("", 5, "LabelOverrideLabelValue1", "LabelOverrideDescriptionValue"));
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new LabelOverride("", 5, "LabelOverrideLabelValue2", "LabelOverrideDescriptionValue2"));

    auto node = TestNodesFactory(*m_connection, spec->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");

    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    // Same scope, conditions match, same prority - returns first declaired
    ASSERT_TRUE(nullptr != labelOverride);
    EXPECT_STREQ("LabelOverrideLabelValue1", labelOverride->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedCustomizationRules_SameScope_DifferentPriorities_ReturnsByPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new ImageIdOverride("", 1, "ImageIdOverrideTestValue1"));
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new ImageIdOverride("", 3, "ImageIdOverrideTestValue2"));

    auto node = TestNodesFactory(*m_connection, spec->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");

    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    ImageIdOverrideCP imageOverride = GetTestRulesPreprocessor(*rules).GetImageIdOverride(params);
    // Same scope - returns highest priority
    ASSERT_TRUE(nullptr != imageOverride);
    EXPECT_STREQ("ImageIdOverrideTestValue2", imageOverride->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedCustomizationRules_DifferentScopes_DifferentPriorities_ReturnsHighestPriority)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->AddPresentationRule(*new StyleOverride("", 2, "Green", "Red", "Bold"));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new StyleOverride("", 1, "Blue", "Red", "Bold"));

    auto node = TestNodesFactory(*m_connection, spec->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");

    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    StyleOverrideCP styleOverride = GetTestRulesPreprocessor(*rules).GetStyleOverride(params);
    // Different Scopes - returns highest priority
    ASSERT_TRUE(nullptr != styleOverride);
    EXPECT_STREQ("Green", styleOverride->GetForeColor().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetNestedCustomizationRules_DifferentScopes_SamePriority_ReturnsMostlyNested)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));
    rules->AddPresentationRule(*new CheckBoxRule("", 1, false, "CheckBoxPropertyName2", false, false, ""));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new CheckBoxRule("", 1, false, "CheckBoxPropertyName1", false, false, ""));

    auto node = TestNodesFactory(*m_connection, spec->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");

    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    CheckBoxRuleCP checkBox = GetTestRulesPreprocessor(*rules).GetCheckboxRule(params);
    // Different scopes, same priority - return mostly nested
    ASSERT_TRUE(nullptr != checkBox);
    EXPECT_STREQ("CheckBoxPropertyName1", checkBox->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetCustomizationRule_WithConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("test");
    rules->AddPresentationRule(*new ChildNodeRule("", 1, false, TargetTree_MainTree));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    rules->GetChildNodesRules()[0]->AddSpecification(*spec);
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new LabelOverride("2=3", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue"));
    rules->GetChildNodesRules()[0]->AddCustomizationRule(*new LabelOverride("2=2", 1, "LabelOverrideLabelValue2", "LabelOverrideDescriptionValue2"));

    auto node = TestNodesFactory(*m_connection, spec->GetHash(), "").CreateCustomNode(nullptr, "TestLabel", "", "", "TestType");

    RulesPreprocessor::CustomizationRuleByNodeParameters params(*node, nullptr);
    LabelOverrideCP labelOverride = GetTestRulesPreprocessor(*rules).GetLabelOverride(params);
    // Same scope, conditions do not match, different priorities - return condition true
    ASSERT_TRUE(nullptr != labelOverride);
    EXPECT_STREQ("LabelOverrideLabelValue2", labelOverride->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    rules->AddPresentationRule(*new ChildNodeRule());
    rules->AddPresentationRule(*new ChildNodeRule());

    rules->AddPresentationRule(*new RootNodeRule());
    rules->AddPresentationRule(*new RootNodeRule());

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());
    EXPECT_EQ(1, foundRules->GetRootNodesRules().size());
    EXPECT_EQ(1, foundRules->GetChildNodesRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging_DoNotMergeNodeRulesWithDifferentConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    rules->AddPresentationRule(*new RootNodeRule("Test1", 0, true, TargetTree_MainTree, false));
    rules->AddPresentationRule(*new RootNodeRule("Test2", 0, true, TargetTree_MainTree, false));

    rules->AddPresentationRule(*new ChildNodeRule("Test1", 1, false, TargetTree_MainTree));
    rules->AddPresentationRule(*new ChildNodeRule("Test2", 1, false, TargetTree_MainTree));

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());
    EXPECT_EQ(2, foundRules->GetRootNodesRules().size());
    EXPECT_EQ(2, foundRules->GetChildNodesRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging_MergeNodeRulesWithDifferentSpecifications)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    RootNodeRuleP rootRule1 = new RootNodeRule();
    RootNodeRuleP rootRule2 = new RootNodeRule();
    rules->AddPresentationRule(*rootRule1);
    rules->AddPresentationRule(*rootRule2);

    rootRule1->AddSpecification(*new AllRelatedInstanceNodesSpecification(1, true, true, true, true, true, true, 0, "Test1"));
    rootRule2->AddSpecification(*new AllRelatedInstanceNodesSpecification(1, false, false, false, false, false, false, 0, "Test2"));

    ChildNodeRuleP childRule1 = new ChildNodeRule();
    ChildNodeRuleP childRule2 = new ChildNodeRule();
    rules->AddPresentationRule(*childRule1);
    rules->AddPresentationRule(*childRule2);

    childRule1->AddSpecification(*new AllInstanceNodesSpecification(1, true, true, true, true, true, "Test1"));
    childRule2->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "Test2"));

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());

    EXPECT_EQ(1, foundRules->GetRootNodesRules().size());
    RootNodeRuleP foundRootRule = foundRules->GetRootNodesRules().front();
    EXPECT_EQ(2, foundRootRule->GetSpecifications().size());

    EXPECT_EQ(1, foundRules->GetChildNodesRules().size());
    ChildNodeRuleP foundChildRule = foundRules->GetChildNodesRules().front();
    EXPECT_EQ(2, foundChildRule->GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging_MergeNodeRulesWithDuplicateSpecifications)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    RootNodeRuleP rootRule1 = new RootNodeRule();
    RootNodeRuleP rootRule2 = new RootNodeRule();
    rules->AddPresentationRule(*rootRule1);
    rules->AddPresentationRule(*rootRule2);

    rootRule1->AddSpecification(*new AllRelatedInstanceNodesSpecification());
    rootRule2->AddSpecification(*new AllRelatedInstanceNodesSpecification());

    ChildNodeRuleP childRule1 = new ChildNodeRule();
    ChildNodeRuleP childRule2 = new ChildNodeRule();
    rules->AddPresentationRule(*childRule1);
    rules->AddPresentationRule(*childRule2);

    childRule1->AddSpecification(*new AllInstanceNodesSpecification());
    childRule2->AddSpecification(*new AllInstanceNodesSpecification());

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());

    EXPECT_EQ(1, foundRules->GetRootNodesRules().size());
    RootNodeRuleP foundRootRule = foundRules->GetRootNodesRules().front();
    EXPECT_EQ(1, foundRootRule->GetSpecifications().size());

    EXPECT_EQ(1, foundRules->GetChildNodesRules().size());
    ChildNodeRuleP foundChildRule = foundRules->GetChildNodesRules().front();
    EXPECT_EQ(1, foundChildRule->GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging_MergeNodeRulesWithDifferentSubConditions)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    ChildNodeRuleP childRule1 = new ChildNodeRule();
    ChildNodeRuleP childRule2 = new ChildNodeRule();
    rules->AddPresentationRule(*childRule1);
    rules->AddPresentationRule(*childRule2);

    SubConditionP subcondition1 = new SubCondition("TestCondition1");
    subcondition1->AddSpecification(*new AllInstanceNodesSpecification());

    SubConditionP subcondition2 = new SubCondition("TestCondition2");
    subcondition2->AddSpecification(*new AllInstanceNodesSpecification());

    childRule1->AddSubCondition(*subcondition1);
    childRule2->AddSubCondition(*subcondition2);

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());

    EXPECT_EQ(1, foundRules->GetChildNodesRules().size());
    ChildNodeRuleP foundChildRule = foundRules->GetChildNodesRules().front();
    ASSERT_EQ(2, foundChildRule->GetSubConditions().size());

    EXPECT_EQ(1, foundChildRule->GetSubConditions()[0]->GetSpecifications().size());
    EXPECT_EQ(1, foundChildRule->GetSubConditions()[1]->GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging_MergeSubConditionsWithSameSpecifications)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    ChildNodeRuleP childRule1 = new ChildNodeRule();
    ChildNodeRuleP childRule2 = new ChildNodeRule();
    rules->AddPresentationRule(*childRule1);
    rules->AddPresentationRule(*childRule2);

    SubConditionP subcondition1 = new SubCondition("TestCondition1");
    subcondition1->AddSpecification(*new AllInstanceNodesSpecification());

    SubConditionP subcondition2 = new SubCondition("TestCondition1");
    subcondition2->AddSpecification(*new AllInstanceNodesSpecification());

    childRule1->AddSubCondition(*subcondition1);
    childRule2->AddSubCondition(*subcondition2);

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());

    EXPECT_EQ(1, foundRules->GetChildNodesRules().size());
    ChildNodeRuleP foundChildRule = foundRules->GetChildNodesRules().front();
    ASSERT_EQ(1, foundChildRule->GetSubConditions().size());

    EXPECT_EQ(1, foundChildRule->GetSubConditions()[0]->GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging_MergeSubConditionsWithDifferentSpecifications)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    ChildNodeRuleP childRule1 = new ChildNodeRule();
    ChildNodeRuleP childRule2 = new ChildNodeRule();
    rules->AddPresentationRule(*childRule1);
    rules->AddPresentationRule(*childRule2);

    SubConditionP subcondition1 = new SubCondition("TestCondition1");
    subcondition1->AddSpecification(*new AllInstanceNodesSpecification(1, true, true, true, true, true, "Test1"));

    SubConditionP subcondition2 = new SubCondition("TestCondition1");
    subcondition2->AddSpecification(*new AllInstanceNodesSpecification(1, true, true, true, true, true, "Test2"));

    childRule1->AddSubCondition(*subcondition1);
    childRule2->AddSubCondition(*subcondition2);

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());

    EXPECT_EQ(1, foundRules->GetChildNodesRules().size());
    ChildNodeRuleP foundChildRule = foundRules->GetChildNodesRules().front();
    ASSERT_EQ(1, foundChildRule->GetSubConditions().size());

    EXPECT_EQ(2, foundChildRule->GetSubConditions()[0]->GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging_MergeDuplicateSpecificationsWithNestedRules)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    RootNodeRuleP rootRule1 = new RootNodeRule();
    RootNodeRuleP rootRule2 = new RootNodeRule();

    ChildNodeSpecificationP specification1 = new AllInstanceNodesSpecification();
    ChildNodeSpecificationP specification2 = new AllInstanceNodesSpecification();

    ChildNodeRuleP nestedRule1 = new ChildNodeRule();
    ChildNodeRuleP nestedRule2 = new ChildNodeRule();

    specification1->AddNestedRule(*nestedRule1);
    specification2->AddNestedRule(*nestedRule2);

    rootRule1->AddSpecification(*specification1);
    rootRule2->AddSpecification(*specification2);

    rules->AddPresentationRule(*rootRule1);
    rules->AddPresentationRule(*rootRule2);

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());

    ASSERT_EQ(1, foundRules->GetRootNodesRules().size());
    RootNodeRuleP foundRootRule = foundRules->GetRootNodesRules().front();
    ASSERT_EQ(1, foundRootRule->GetSpecifications().size());
    ChildNodeSpecificationP foundSpecification = foundRootRule->GetSpecifications().front();
    EXPECT_EQ(1, foundSpecification->GetNestedRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesPreprocessorTests, GetPresentationRuleSet_DuplicateRulesMerging_DoNotMergeNodeRulesWithCustomizationRules)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Test");
    ChildNodeRuleP childRule1 = new ChildNodeRule();
    ChildNodeRuleP childRule2 = new ChildNodeRule();
    rules->AddPresentationRule(*childRule1);
    rules->AddPresentationRule(*childRule2);

    GroupingRuleP groupRule = new GroupingRule();
    childRule1->AddCustomizationRule(*groupRule);

    RootNodeRuleP rootRule1 = new RootNodeRule();
    RootNodeRuleP rootRule2 = new RootNodeRule();
    rules->AddPresentationRule(*rootRule1);
    rules->AddPresentationRule(*rootRule2);

    ImageIdOverrideP imageRule = new ImageIdOverride();
    rootRule2->AddCustomizationRule(*imageRule);

    m_locater->AddRuleSet(*rules);
    PresentationRuleSetPtr foundRules = RulesPreprocessor::GetPresentationRuleSet(m_locaterManager, *m_connection);

    ASSERT_TRUE(foundRules.IsValid());

    EXPECT_EQ(2, foundRules->GetRootNodesRules().size());
    EXPECT_EQ(2, foundRules->GetChildNodesRules().size());
    }
