/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/PresentationRulesHashingTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "../../NonPublished/RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                09/2017
+===============+===============+===============+===============+===============+======*/
struct PresentationRulesHashingTests : ECPresentationTest
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, SameSpecificationsInDifferentRulesHasDifferentHash)
    {
    // create rules
    ChildNodeRuleP rule1 = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ChildNodeRuleP rule2 = new ChildNodeRule("TestCondition1", 100, true, RuleTargetTree::TargetTree_Both);

    // expect rules hashes to be different
    Utf8String rule1Hash = rule1->GetHash();
    Utf8String rule2Hash = rule2->GetHash();
    EXPECT_STRNE(rule1Hash.c_str(), rule2Hash.c_str());

    InstanceNodesOfSpecificClassesSpecificationP spec1 = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestClass", true);
    InstanceNodesOfSpecificClassesSpecificationP spec2 = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestClass", true);
    rule1->AddSpecification(*spec1);
    rule2->AddSpecification(*spec2);

    // rules hashes should be different
    rule1Hash = rule1->GetHash();
    rule2Hash = rule2->GetHash();
    EXPECT_STRNE(rule1Hash.c_str(), rule2Hash.c_str());

    // expect specifications hashes to be different
    Utf8String s1 = spec1->GetHash();
    Utf8String s2 = spec2->GetHash();
    EXPECT_STRNE(s1.c_str(), s2.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, AddingNewSpecificationToRuleDoesntChangeOldSpecificationHash)
    {
    // create rule
    ChildNodeRuleP rule = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);

    // add specification
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestClass", true);
    rule->AddSpecification(*spec);

    // get hashes
    Utf8String ruleHash = rule->GetHash();
    Utf8String specHash = spec->GetHash();

    // add new specification
    AllInstanceNodesSpecificationP newSpec = new AllInstanceNodesSpecification(1, true, true, true, true, true, "TestSchema");
    rule->AddSpecification(*newSpec);

    // get hashes again
    Utf8String ruleHashNew = rule->GetHash();
    Utf8String specHashNew = spec->GetHash();

    // expect rule to have new hash
    EXPECT_STRNE(ruleHash.c_str(), ruleHashNew.c_str());
    
    // expect specification to have same hash
    EXPECT_STREQ(specHash.c_str(), specHashNew.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, RulesetsWithSameRulesAndSpecificationsHasSameHash)
    {
    // create rulesets
    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);

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
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, RulesetHasDifferentHashAfterSpecificationIsAdded)
    {
    // create ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);

    // create rule
    ChildNodeRuleP rule = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ruleset->AddPresentationRule(*rule);

    Utf8String rulesetHash = ruleset->GetHash();

    // add specification
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestClass", true);
    rule->AddSpecification(*spec);

    EXPECT_STRNE(ruleset->GetHash().c_str(), rulesetHash.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, RulesetHasDifferentHashAfterRuleIsAdded)
    {
    // create ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);

    // create rule
    ChildNodeRuleP rule = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ruleset->AddPresentationRule(*rule);

    Utf8String rulesetHash = ruleset->GetHash();

    // add one more rule
    ruleset->AddPresentationRule(*new ContentRule("", 1, false));

    EXPECT_STRNE(ruleset->GetHash().c_str(), rulesetHash.c_str());
    }
