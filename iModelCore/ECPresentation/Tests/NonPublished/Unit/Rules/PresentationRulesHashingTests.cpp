/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Rules/PresentationRules.h>
#include "../../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* TODO: Check if needed anymore
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationRulesHashingTests : ECPresentationTest
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, AddingNewSpecificationToRuleDoesntChangeOldSpecificationHash)
    {
    // create rule
    ChildNodeRule rule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);

    // add specification
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestSchema:TestClass", true);
    rule.AddSpecification(*spec);

    // get hashes
    Utf8String ruleHash = rule.GetHash();
    Utf8String specHash = spec->GetHash();

    // add new specification
    AllInstanceNodesSpecificationP newSpec = new AllInstanceNodesSpecification(1, true, true, true, true, true, "TestSchema");
    rule.AddSpecification(*newSpec);

    // get hashes again
    Utf8String ruleHashNew = rule.GetHash();
    Utf8String specHashNew = spec->GetHash();

    // expect rule to have new hash
    EXPECT_STRNE(ruleHash.c_str(), ruleHashNew.c_str());

    // expect specification to have same hash
    EXPECT_STREQ(specHash.c_str(), specHashNew.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, RulesetsWithSameRulesAndSpecificationsHasSameHash)
    {
    // create rulesets
    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("TestRuleset");
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("TestRuleset");

    // create rules
    ChildNodeRuleP rule1 = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ruleset1->AddPresentationRule(*rule1);
    ChildNodeRuleP rule2 = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ruleset2->AddPresentationRule(*rule2);

    // add specifications
    rule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "TestSchema:TestClass", false));
    rule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "TestSchema:TestClass", false));

    // expect rulesets to have same hash
    EXPECT_STREQ(ruleset1->GetHash().c_str(), ruleset2->GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, RulesetHasDifferentHashAfterSpecificationIsAdded)
    {
    // create ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("TestRuleset");

    // create rule
    ChildNodeRuleP rule = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ruleset->AddPresentationRule(*rule);

    Utf8String rulesetHash = ruleset->GetHash();

    // add specification
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestSchema:TestClass", true);
    rule->AddSpecification(*spec);

    EXPECT_STRNE(ruleset->GetHash().c_str(), rulesetHash.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, RulesetHasDifferentHashAfterRuleIsAdded)
    {
    // create ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("TestRuleset");

    // create rule
    ChildNodeRuleP rule = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ruleset->AddPresentationRule(*rule);

    Utf8String rulesetHash = ruleset->GetHash();

    // add one more rule
    ruleset->AddPresentationRule(*new ContentRule("", 1, false));

    EXPECT_STRNE(ruleset->GetHash().c_str(), rulesetHash.c_str());
    }
