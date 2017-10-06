/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/PresentationRulesHashingTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                09/2017
+===============+===============+===============+===============+===============+======*/
struct PresentationRulesHashingTests : ::testing::Test
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesHashingTests, DifferentSpecificationsOfTheSameRuleHasDifferentHashes)
    {
    // create rules
    ChildNodeRuleP rule1 = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ChildNodeRuleP rule2 = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);

    // expect rules hashes to be equal
    Utf8String rule1Hash = rule1->GetHash();
    Utf8String rule2Hash = rule2->GetHash();
    EXPECT_STREQ(rule1Hash.c_str(), rule2Hash.c_str());

    InstanceNodesOfSpecificClassesSpecificationP spec1 = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestClass", true);
    InstanceNodesOfSpecificClassesSpecificationP spec2 = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestClass2", true);
    rule1->AddSpecification(*spec1);
    rule2->AddSpecification(*spec2);

    // now rules hashes should be different
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
TEST_F(PresentationRulesHashingTests, SameSpecificationsHasSameHash)
    {
    // create rules
    ChildNodeRuleP rule1 = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);
    ChildNodeRuleP rule2 = new ChildNodeRule("TestCondition", 100, true, RuleTargetTree::TargetTree_Both);

    // expect rules hashes to be equal
    Utf8String rule1Hash = rule1->GetHash();
    Utf8String rule2Hash = rule2->GetHash();
    EXPECT_STREQ(rule1Hash.c_str(), rule2Hash.c_str());

    InstanceNodesOfSpecificClassesSpecificationP spec1 = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestClass", true);
    InstanceNodesOfSpecificClassesSpecificationP spec2 = new InstanceNodesOfSpecificClassesSpecification(1, true, true, true, true, true, true, "", "TestClass", true);
    rule1->AddSpecification(*spec1);
    rule2->AddSpecification(*spec2);

    // rules hashes should be still equal
    rule1Hash = rule1->GetHash();
    rule2Hash = rule2->GetHash();
    EXPECT_STREQ(rule1Hash.c_str(), rule2Hash.c_str());

    // expect specifications hashes to be equal
    Utf8String s1 = spec1->GetHash();
    Utf8String s2 = spec2->GetHash();
    EXPECT_STREQ(s1.c_str(), s2.c_str());
    }

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
