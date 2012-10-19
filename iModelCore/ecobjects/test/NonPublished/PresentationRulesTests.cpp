/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/PresentationRulesTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

#include <ECPresentationRules/PresentationRules.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct PresentationRulesTests : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateChildNodeRule (ChildNodeRuleCR childNodeRule, WCharP condition, int priority, bool onlyIfNotHandled)
    {
    EXPECT_STREQ (condition,        childNodeRule.GetCondition ().c_str ());
    EXPECT_EQ    (priority,         childNodeRule.GetPriority ());
    EXPECT_EQ    (onlyIfNotHandled, childNodeRule.GetOnlyIfNotHandled ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateContentRule (ContentRuleCR contentRule, WCharP condition, int priority, bool onlyIfNotHandled)
    {
    EXPECT_STREQ (condition,        contentRule.GetCondition ().c_str ());
    EXPECT_EQ    (priority,         contentRule.GetPriority ());
    EXPECT_EQ    (onlyIfNotHandled, contentRule.GetOnlyIfNotHandled ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateImageIdOverride (ImageIdOverrideCR imageIdOverride, WCharP condition, WCharP imageIdExpression, int priority, bool onlyIfNotHandled)
    {
    EXPECT_STREQ (condition,         imageIdOverride.GetCondition ().c_str ());
    EXPECT_STREQ (imageIdExpression, imageIdOverride.GetImageId ().c_str ());
    EXPECT_EQ    (priority,          imageIdOverride.GetPriority ());
    EXPECT_EQ    (onlyIfNotHandled,  imageIdOverride.GetOnlyIfNotHandled ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, TestPresentationRuleSetCreation)
    {
    RootNodeRuleP rootNodeRule1 = new RootNodeRule (L"TestCondition1", 1, false, TargetTree_MainTree);
    RootNodeRuleP rootNodeRule2 = new RootNodeRule (L"TestCondition2", 3, true, TargetTree_MainTree);
    RootNodeRuleP rootNodeRule3 = new RootNodeRule (L"TestCondition3", 2, false, TargetTree_MainTree);
    ValidateChildNodeRule (*rootNodeRule1, L"TestCondition1", 1, false);
    ValidateChildNodeRule (*rootNodeRule2, L"TestCondition2", 3, true);
    ValidateChildNodeRule (*rootNodeRule3, L"TestCondition3", 2, false);

    ChildNodeRuleP childNodeRule1 = new ChildNodeRule (L"TestCondition1", 1, false, TargetTree_MainTree);
    ChildNodeRuleP childNodeRule2 = new ChildNodeRule (L"TestCondition2", 3, true, TargetTree_MainTree);
    ChildNodeRuleP childNodeRule3 = new ChildNodeRule (L"TestCondition3", 2, false, TargetTree_MainTree);
    ValidateChildNodeRule (*childNodeRule1, L"TestCondition1", 1, false);
    ValidateChildNodeRule (*childNodeRule2, L"TestCondition2", 3, true);
    ValidateChildNodeRule (*childNodeRule3, L"TestCondition3", 2, false);

    ContentRuleP contentRule1 = new ContentRule (L"TestCondition1", 1, false);
    ContentRuleP contentRule2 = new ContentRule (L"TestCondition2", 3, true);
    ContentRuleP contentRule3 = new ContentRule (L"TestCondition3", 2, false);
    ValidateContentRule (*contentRule1, L"TestCondition1", 1, false);
    ValidateContentRule (*contentRule2, L"TestCondition2", 3, true);
    ValidateContentRule (*contentRule3, L"TestCondition3", 2, false);

    ImageIdOverrideP imageIdOverride1 = new ImageIdOverride (L"TestCondition1", L"Expression1", 1, false);
    ImageIdOverrideP imageIdOverride2 = new ImageIdOverride (L"TestCondition2", L"Expression2", 3, true);
    ImageIdOverrideP imageIdOverride3 = new ImageIdOverride (L"TestCondition3", L"Expression3", 2, false);
    ValidateImageIdOverride (*imageIdOverride1, L"TestCondition1", L"Expression1", 1, false);
    ValidateImageIdOverride (*imageIdOverride2, L"TestCondition2", L"Expression2", 3, true);
    ValidateImageIdOverride (*imageIdOverride3, L"TestCondition3", L"Expression3", 2, false);

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance (L"File", L"BaseElmentSchema", false, 1, L"MyImage");
    EXPECT_STREQ (L"File", ruleSet->GetRuleSetId ().c_str ());
    EXPECT_STREQ (L"BaseElmentSchema", ruleSet->GetSupportedSchemas ().c_str ());
    EXPECT_FALSE (ruleSet->GetIsSupplemental ());
    EXPECT_EQ    (1, ruleSet->GetVersion ());

    ruleSet->GetRootNodesRules ().push_back (rootNodeRule1);
    ruleSet->GetRootNodesRules ().push_back (rootNodeRule2);
    ruleSet->GetRootNodesRules ().push_back (rootNodeRule3);

    ruleSet->GetChildNodesRules ().push_back (childNodeRule1);
    ruleSet->GetChildNodesRules ().push_back (childNodeRule2);
    ruleSet->GetChildNodesRules ().push_back (childNodeRule3);

    ruleSet->GetContentRules ().push_back (contentRule1);
    ruleSet->GetContentRules ().push_back (contentRule2);
    ruleSet->GetContentRules ().push_back (contentRule3);

    ruleSet->GetImageIdOverrides ().push_back (imageIdOverride1);
    ruleSet->GetImageIdOverrides ().push_back (imageIdOverride2);
    ruleSet->GetImageIdOverrides ().push_back (imageIdOverride3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, TestPresentationRuleSetLoadingFromXml)
    {
    WCharP ruleSetXmlString = L"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                              L"  <PresentationRuleSet"
                              L"    RuleSetId=\"Items\""
                              L"    Version=\"1\""
                              L"    SupportedSchemas=\"E:DgnFileSchema,DgnModelSchema,BaseElementSchema\""
                              L"    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                              L"    xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\">"
                              L"    <RootNodeRule>"
                              L"      <AllInstances GroupByClass='true' GroupByLabel='true' />"
                              L"    </RootNodeRule>"
                              L"    <RootNodeRule Condition='TestCondition' Priority='12' OnlyIfNotHandled='true'>"
                              L"      <AllInstances GroupByClass='true' GroupByLabel='true' />"
                              L"    </RootNodeRule>"
                              L"    <ChildNodeRule>"
                              L"      <AllRelatedInstances GroupByClass='true' GroupByLabel='true' />"
                              L"    </ChildNodeRule>"
                              L"    <ChildNodeRule Condition='ParentNode.IsSearchNode' Priority='58' OnlyIfNotHandled='true'>"
                              L"      <SearchResultInstances GroupByClass='true' GroupByLabel='true' />"
                              L"    </ChildNodeRule>"
                              L"    <ContentRule>"
                              L"    </ContentRule>"
                              L"    <ContentRule Condition='ParentNode.IsClassNode' Priority='97' OnlyIfNotHandled='true'>"
                              L"    </ContentRule>"
                              L"    <ImageIdOverride ImageId='NewImageId1'/>"
                              L"    <ImageIdOverride Condition='ParentNode.IsClassGroupingNode' ImageId='NewImageId2' Priority='0' OnlyIfNotHandled='true'/>"
                              L"  </PresentationRuleSet>";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromXmlString (ruleSetXmlString);
    
    EXPECT_FALSE (ruleSet.IsNull ());
    EXPECT_STREQ (L"Items", ruleSet->GetRuleSetId ().c_str ());
    EXPECT_STREQ (L"E:DgnFileSchema,DgnModelSchema,BaseElementSchema", ruleSet->GetSupportedSchemas ().c_str ());
    EXPECT_FALSE (ruleSet->GetIsSupplemental ());
    EXPECT_EQ    (1, ruleSet->GetVersion ());

    //Check for root node rules
    int rootRulesCount = 0;
    for (RootNodeRuleList::const_iterator iter = ruleSet->GetRootNodesRules ().begin (); iter != ruleSet->GetRootNodesRules ().end (); ++iter)
        {
        ++rootRulesCount;
        if (1 == rootRulesCount)
            ValidateChildNodeRule (**iter, L""/*default*/, 1000/*default*/, false/*default*/);
        else
            ValidateChildNodeRule (**iter, L"TestCondition", 12, true);
        }
    EXPECT_EQ (2, rootRulesCount);

    //Check for child node rules
    int childNodeRulesCount = 0;
    for (ChildNodeRuleList::const_iterator iter = ruleSet->GetChildNodesRules ().begin (); iter != ruleSet->GetChildNodesRules ().end (); ++iter)
        {
        ++childNodeRulesCount;
        if (1 == childNodeRulesCount)
            ValidateChildNodeRule (**iter, L""/*default*/, 1000/*default*/, false/*default*/);
        else
            ValidateChildNodeRule (**iter, L"ParentNode.IsSearchNode", 58, true);
        }
    EXPECT_EQ (2, childNodeRulesCount);

    //Check for content rules
    int contentRulesCount = 0;
    for (ContentRuleList::const_iterator iter = ruleSet->GetContentRules ().begin (); iter != ruleSet->GetContentRules ().end (); ++iter)
        {
        ++contentRulesCount;
        if (1 == contentRulesCount)
            ValidateContentRule (**iter, L""/*default*/, 1000/*default*/, false/*default*/);
        else
            ValidateContentRule (**iter, L"ParentNode.IsClassNode", 97, true);
        }
    EXPECT_EQ (2, contentRulesCount);

    //Check for ImageIdOverride rules
    int imageIdOverrideCount = 0;
    for (ImageIdOverrideList::const_iterator iter = ruleSet->GetImageIdOverrides ().begin (); iter != ruleSet->GetImageIdOverrides ().end (); ++iter)
        {
        ++imageIdOverrideCount;
        if (1 == imageIdOverrideCount)
            ValidateImageIdOverride (**iter, L""/*default*/, L"NewImageId1", 1000/*default*/, false/*default*/);
        else
            ValidateImageIdOverride (**iter, L"ParentNode.IsClassGroupingNode", L"NewImageId2", 0, true);
        }
    EXPECT_EQ (2, imageIdOverrideCount);

    }

END_BENTLEY_ECOBJECT_NAMESPACE
