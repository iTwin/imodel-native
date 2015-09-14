/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/NonPublished/PresentationRulesTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECPresentationRules/PresentationRules.h>
using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PresentationRulesTests : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateChildNodeRule (ChildNodeRuleCR childNodeRule, WCharP condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
    {
    EXPECT_STREQ (condition,        childNodeRule.GetCondition ().c_str ());
    EXPECT_EQ    (priority,         childNodeRule.GetPriority ());
    EXPECT_EQ    (onlyIfNotHandled, childNodeRule.GetOnlyIfNotHandled ());
    //EXPECT_EQ    ((int)targetTree,  (int)childNodeRule.GetTargetTree ());
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
static void ValidateImageIdOverride (ImageIdOverrideCR imageIdOverride, WCharP condition, WCharP imageIdExpression, int priority)
    {
    EXPECT_STREQ (condition,         imageIdOverride.GetCondition ().c_str ());
    EXPECT_STREQ (imageIdExpression, imageIdOverride.GetImageId ().c_str ());
    EXPECT_EQ    (priority,          imageIdOverride.GetPriority ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateLabelOverride (LabelOverrideCR labelOverride, WCharP condition, int priority, WCharP label, WCharP description)
    {
    EXPECT_STREQ (condition,         labelOverride.GetCondition ().c_str ());
    EXPECT_EQ    (priority,          labelOverride.GetPriority ());
    EXPECT_STREQ (label,             labelOverride.GetLabel ().c_str ());
    EXPECT_STREQ (description,       labelOverride.GetDescription ().c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateStyleOverride (StyleOverrideCR labelOverride, WCharP condition, int priority, WCharP foreColor, WCharP backColor, WCharP fontStyle)
    {
    EXPECT_STREQ (condition,         labelOverride.GetCondition ().c_str ());
    EXPECT_EQ    (priority,          labelOverride.GetPriority ());
    EXPECT_STREQ (foreColor,         labelOverride.GetForeColor ().c_str ());
    EXPECT_STREQ (backColor,         labelOverride.GetBackColor ().c_str ());
    EXPECT_STREQ (fontStyle,         labelOverride.GetFontStyle ().c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateLocalizationResourceKeyDefinition (LocalizationResourceKeyDefinitionCR definition, int priority, WCharP id, WCharP key, WCharP defaultValue)
    {
    EXPECT_EQ    (priority,          definition.GetPriority ());
    EXPECT_STREQ (id,                definition.GetId ().c_str ());
    EXPECT_STREQ (key,               definition.GetKey ().c_str ());
    EXPECT_STREQ (defaultValue,      definition.GetDefaultValue ().c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, TestPresentationRuleSetCreation)
    {
    RootNodeRuleP rootNodeRule1 = new RootNodeRule (L"TestCondition1", 1, false, TargetTree_MainTree, false);
    RootNodeRuleP rootNodeRule2 = new RootNodeRule (L"TestCondition2", 3, true, TargetTree_MainTree, false);
    RootNodeRuleP rootNodeRule3 = new RootNodeRule (L"TestCondition3", 2, false, TargetTree_MainTree, false);
    ValidateChildNodeRule (*rootNodeRule1, L"TestCondition1", 1, false, TargetTree_MainTree);
    ValidateChildNodeRule (*rootNodeRule2, L"TestCondition2", 3, true, TargetTree_MainTree);
    ValidateChildNodeRule (*rootNodeRule3, L"TestCondition3", 2, false, TargetTree_MainTree);

    ChildNodeRuleP childNodeRule1 = new ChildNodeRule (L"TestCondition1", 1, false, TargetTree_MainTree);
    ChildNodeRuleP childNodeRule2 = new ChildNodeRule (L"TestCondition2", 3, true, TargetTree_MainTree);
    ChildNodeRuleP childNodeRule3 = new ChildNodeRule (L"TestCondition3", 2, false, TargetTree_MainTree);
    ValidateChildNodeRule (*childNodeRule1, L"TestCondition1", 1, false, TargetTree_MainTree);
    ValidateChildNodeRule (*childNodeRule2, L"TestCondition2", 3, true, TargetTree_MainTree);
    ValidateChildNodeRule (*childNodeRule3, L"TestCondition3", 2, false, TargetTree_MainTree);

    ContentRuleP contentRule1 = new ContentRule (L"TestCondition1", 1, false);
    ContentRuleP contentRule2 = new ContentRule (L"TestCondition2", 3, true);
    ContentRuleP contentRule3 = new ContentRule (L"TestCondition3", 2, false);
    ValidateContentRule (*contentRule1, L"TestCondition1", 1, false);
    ValidateContentRule (*contentRule2, L"TestCondition2", 3, true);
    ValidateContentRule (*contentRule3, L"TestCondition3", 2, false);

    ImageIdOverrideP imageIdOverride1 = new ImageIdOverride (L"TestCondition1", 1, L"Expression1");
    ImageIdOverrideP imageIdOverride2 = new ImageIdOverride (L"TestCondition2", 3, L"Expression2");
    ImageIdOverrideP imageIdOverride3 = new ImageIdOverride (L"TestCondition3", 2, L"Expression3");
    ValidateImageIdOverride (*imageIdOverride1, L"TestCondition1", L"Expression1", 1);
    ValidateImageIdOverride (*imageIdOverride2, L"TestCondition2", L"Expression2", 3);
    ValidateImageIdOverride (*imageIdOverride3, L"TestCondition3", L"Expression3", 2);

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance (L"File", 1, 0, false, L"Supplemental", L"BaseElmentSchema", L"MyImage", true);
    EXPECT_STREQ (L"File", ruleSet->GetRuleSetId ().c_str ());
    EXPECT_STREQ (L"BaseElmentSchema", ruleSet->GetSupportedSchemas ().c_str ());
    EXPECT_FALSE (ruleSet->GetIsSupplemental ());
    EXPECT_STREQ (L"Supplemental", ruleSet->GetSupplementationPurpose ().c_str ());
    EXPECT_EQ    (1, ruleSet->GetVersionMajor ());
    EXPECT_EQ    (0, ruleSet->GetVersionMinor ());

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
    Utf8CP ruleSetXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                              "  <PresentationRuleSet"
                              "    RuleSetId=\"Items\""
                              "    VersionMajor=\"5\""
                              "    VersionMinor=\"3\""
                              "    SupportedSchemas=\"E:DgnFileSchema,DgnModelSchema,BaseElementSchema\""
                              "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                              "    xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\">"
                              "    <RootNodeRule>"
                              "      <AllInstances GroupByClass='true' GroupByLabel='true' />"
                              "    </RootNodeRule>"
                              "    <RootNodeRule Condition='TestCondition' Priority='12' OnlyIfNotHandled='true'>"
                              "      <AllInstances GroupByClass='true' GroupByLabel='true' />"
                              "    </RootNodeRule>"
                              "    <ChildNodeRule>"
                              "      <AllRelatedInstances GroupByClass='true' GroupByLabel='true' />"
                              "    </ChildNodeRule>"
                              "    <ChildNodeRule Condition='ParentNode.IsSearchNode' Priority='58' OnlyIfNotHandled='true'>"
                              "      <SearchResultInstances GroupByClass='true' GroupByLabel='true' />"
                              "    </ChildNodeRule>"
                              "    <ContentRule>"
                              "    </ContentRule>"
                              "    <ContentRule Condition='ParentNode.IsClassNode' Priority='97' OnlyIfNotHandled='true'>"
                              "    </ContentRule>"
                              "    <ImageIdOverride ImageId='NewImageId1'/>"
                              "    <ImageIdOverride Condition='ParentNode.IsClassGroupingNode' ImageId='NewImageId2' Priority='0'/>"
                              "  </PresentationRuleSet>";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromXmlString (ruleSetXmlString);
    
    EXPECT_FALSE (ruleSet.IsNull ());
    EXPECT_STREQ (L"Items", ruleSet->GetRuleSetId ().c_str ());
    EXPECT_STREQ (L"E:DgnFileSchema,DgnModelSchema,BaseElementSchema", ruleSet->GetSupportedSchemas ().c_str ());
    EXPECT_FALSE (ruleSet->GetIsSupplemental ());
    EXPECT_EQ    (5, ruleSet->GetVersionMajor ());
    EXPECT_EQ    (3, ruleSet->GetVersionMinor ());

    //Check for root node rules
    int rootRulesCount = 0;
    for (RootNodeRuleList::const_iterator iter = ruleSet->GetRootNodesRules ().begin (); iter != ruleSet->GetRootNodesRules ().end (); ++iter)
        {
        ++rootRulesCount;
        if (1 == rootRulesCount)
            ValidateChildNodeRule (**iter, L""/*default*/, 1000/*default*/, false/*default*/, TargetTree_MainTree);
        else
            ValidateChildNodeRule (**iter, L"TestCondition", 12, true, TargetTree_MainTree);
        }
    EXPECT_EQ (2, rootRulesCount);

    //Check for child node rules
    int childNodeRulesCount = 0;
    for (ChildNodeRuleList::const_iterator iter = ruleSet->GetChildNodesRules ().begin (); iter != ruleSet->GetChildNodesRules ().end (); ++iter)
        {
        ++childNodeRulesCount;
        if (1 == childNodeRulesCount)
            ValidateChildNodeRule (**iter, L""/*default*/, 1000/*default*/, false/*default*/, TargetTree_MainTree);
        else
            ValidateChildNodeRule (**iter, L"ParentNode.IsSearchNode", 58, true, TargetTree_MainTree);
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
            ValidateImageIdOverride (**iter, L""/*default*/, L"NewImageId1", 1000/*default*/);
        else
            ValidateImageIdOverride (**iter, L"ParentNode.IsClassGroupingNode", L"NewImageId2", 0);
        }
    EXPECT_EQ (2, imageIdOverrideCount);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, TestPresentationRuleSetSavingToXml)
    {
    //Create PresentationRuleSet and rules usin non-default values, to make sure it saves and loads XML correctly.
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance (L"TestRuleSet", 2, 1, true, L"Supplemental", L"DummySchemaName", L"MyImage", true);
    
    RootNodeRuleP rootNodeRule = new RootNodeRule (L"TestCondition1", 1, true, TargetTree_Both, false);
    ruleSet->GetRootNodesRules ().push_back (rootNodeRule);

    ChildNodeRuleP childNodeRule = new ChildNodeRule (L"TestCondition2", 2, true, TargetTree_Both);
    ruleSet->GetChildNodesRules ().push_back (childNodeRule);

    ContentRuleP contentRule = new ContentRule (L"TestCondition3", 3, true);
    ruleSet->GetContentRules ().push_back (contentRule);

    ruleSet->GetImageIdOverrides ().push_back (new ImageIdOverride (L"TestCondition4", 4, L"ImageIdOverrideTestValue"));
    ruleSet->GetLabelOverrides ().push_back (new LabelOverride (L"TestCondition5", 5, L"LabelOverrideLabelValue", L"LabelOverrideDescriptionValue"));
    ruleSet->GetStyleOverrides ().push_back (new StyleOverride (L"TestCondition6", 6, L"Blue", L"Red", L"Bold"));
    ruleSet->GetLocalizationResourceKeyDefinitions ().push_back (new LocalizationResourceKeyDefinition (7, L"UniqueId:)", L"LocalizedStringAccessKey", L"ThisIsTheValueIfItFails"));
    
    GroupingRuleP groupingRule = new GroupingRule (L"TestCondition7", 7, true, L"DummySchemaName", L"DummyClassName", L"ContextMenuCondition", L"ContextMenuLabel", L"SettingsId");
    ClassGroupP classGroup = new ClassGroup (L"ContextMenuLabel", true, L"SchemaName", L"BaseClassName");
    groupingRule->GetGroups ().push_back (classGroup);
    PropertyGroupP propertyGroup = new PropertyGroup (L"ImageId", L"ContextMenuLabel", true, L"PropertyName");
    propertyGroup->GetRanges ().push_back (new PropertyRangeGroupSpecification (L"Label", L"ImageId", L"FromValue", L"ToValue"));
    groupingRule->GetGroups ().push_back (propertyGroup);
    ruleSet->GetGroupingRules ().push_back (groupingRule);

    //Serialize RuleSet to string and deserialize from the same string.
    Utf8String serializedRuleSet = ruleSet->WriteToXmlString ();
    PresentationRuleSetPtr loadedRuleSet = PresentationRuleSet::ReadFromXmlString (serializedRuleSet.c_str ());

    //Compare two PresentationRuleSets to check whether it serialized and loaded corectly.
    EXPECT_FALSE (loadedRuleSet.IsNull ());
    EXPECT_STREQ (L"TestRuleSet", loadedRuleSet->GetRuleSetId ().c_str ());
    EXPECT_STREQ (L"DummySchemaName", loadedRuleSet->GetSupportedSchemas ().c_str ());
    EXPECT_TRUE  (loadedRuleSet->GetIsSupplemental ());
    EXPECT_STREQ (L"Supplemental", ruleSet->GetSupplementationPurpose ().c_str ());
    EXPECT_EQ    (2, loadedRuleSet->GetVersionMajor ());
    EXPECT_EQ    (1, loadedRuleSet->GetVersionMinor ());
    EXPECT_STREQ (L"MyImage", loadedRuleSet->GetPreferredImage ().c_str ());

    //Check for root node rules
    int rootRulesCount = 0;
    for (RootNodeRuleList::const_iterator iter = ruleSet->GetRootNodesRules ().begin (); iter != ruleSet->GetRootNodesRules ().end (); ++iter)
        {
        ++rootRulesCount;
        ValidateChildNodeRule (**iter, L"TestCondition1", 1, true, TargetTree_Both);
        }
    EXPECT_EQ (1, rootRulesCount);

    //Check for child node rules
    int childNodeRulesCount = 0;
    for (ChildNodeRuleList::const_iterator iter = ruleSet->GetChildNodesRules ().begin (); iter != ruleSet->GetChildNodesRules ().end (); ++iter)
        {
        ++childNodeRulesCount;
        ValidateChildNodeRule (**iter, L"TestCondition2", 2, true, TargetTree_Both);
        }
    EXPECT_EQ (1, childNodeRulesCount);

    //Check for content rules
    int contentRulesCount = 0;
    for (ContentRuleList::const_iterator iter = ruleSet->GetContentRules ().begin (); iter != ruleSet->GetContentRules ().end (); ++iter)
        {
        ++contentRulesCount;
        ValidateContentRule (**iter, L"TestCondition3", 3, true);
        }
    EXPECT_EQ (1, contentRulesCount);

    //Check for ImageIdOverride rules
    int imageIdOverrideCount = 0;
    for (ImageIdOverrideList::const_iterator iter = ruleSet->GetImageIdOverrides ().begin (); iter != ruleSet->GetImageIdOverrides ().end (); ++iter)
        {
        ++imageIdOverrideCount;
        ValidateImageIdOverride (**iter, L"TestCondition4", L"ImageIdOverrideTestValue", 4);
        }
    EXPECT_EQ (1, imageIdOverrideCount);

    //Check for LabelOverride rules
    int labelOverrideCount = 0;
    for (LabelOverrideList::const_iterator iter = ruleSet->GetLabelOverrides ().begin (); iter != ruleSet->GetLabelOverrides ().end (); ++iter)
        {
        ++labelOverrideCount;
        ValidateLabelOverride (**iter, L"TestCondition5", 5, L"LabelOverrideLabelValue", L"LabelOverrideDescriptionValue");
        }
    EXPECT_EQ (1, labelOverrideCount);

    //Check for StyleOverride rules
    int styleOverrideCount = 0;
    for (StyleOverrideList::const_iterator iter = ruleSet->GetStyleOverrides ().begin (); iter != ruleSet->GetStyleOverrides ().end (); ++iter)
        {
        ++styleOverrideCount;
        ValidateStyleOverride (**iter, L"TestCondition6", 6, L"Blue", L"Red", L"Bold");
        }
    EXPECT_EQ (1, styleOverrideCount);
    
    //Check for ValidateLocalizationResourceKeyDefinition rules
    int localizationResourceKeyDefinitionCount = 0;
    for (LocalizationResourceKeyDefinitionList::const_iterator iter = ruleSet->GetLocalizationResourceKeyDefinitions ().begin (); iter != ruleSet->GetLocalizationResourceKeyDefinitions ().end (); ++iter)
        {
        ++localizationResourceKeyDefinitionCount;
        ValidateLocalizationResourceKeyDefinition (**iter, 7, L"UniqueId:)", L"LocalizedStringAccessKey", L"ThisIsTheValueIfItFails");
        }
    EXPECT_EQ (1, localizationResourceKeyDefinitionCount);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
