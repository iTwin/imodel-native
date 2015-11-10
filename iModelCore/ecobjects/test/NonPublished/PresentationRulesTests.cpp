/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/PresentationRulesTests.cpp $
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
static void ValidateChildNodeRule (ChildNodeRuleCR childNodeRule, Utf8P condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
    {
    EXPECT_STREQ (condition,        childNodeRule.GetCondition ().c_str ());
    EXPECT_EQ    (priority,         childNodeRule.GetPriority ());
    EXPECT_EQ    (onlyIfNotHandled, childNodeRule.GetOnlyIfNotHandled ());
    //EXPECT_EQ    ((int)targetTree,  (int)childNodeRule.GetTargetTree ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateContentRule (ContentRuleCR contentRule, Utf8P condition, int priority, bool onlyIfNotHandled)
    {
    EXPECT_STREQ (condition,        contentRule.GetCondition ().c_str ());
    EXPECT_EQ    (priority,         contentRule.GetPriority ());
    EXPECT_EQ    (onlyIfNotHandled, contentRule.GetOnlyIfNotHandled ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateImageIdOverride (ImageIdOverrideCR imageIdOverride, Utf8P condition, Utf8P imageIdExpression, int priority)
    {
    EXPECT_STREQ (condition,         imageIdOverride.GetCondition ().c_str ());
    EXPECT_STREQ (imageIdExpression, imageIdOverride.GetImageId ().c_str ());
    EXPECT_EQ    (priority,          imageIdOverride.GetPriority ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateLabelOverride (LabelOverrideCR labelOverride, Utf8P condition, int priority, Utf8P label, Utf8P description)
    {
    EXPECT_STREQ (condition,         labelOverride.GetCondition ().c_str ());
    EXPECT_EQ    (priority,          labelOverride.GetPriority ());
    EXPECT_STREQ (label,             labelOverride.GetLabel ().c_str ());
    EXPECT_STREQ (description,       labelOverride.GetDescription ().c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateStyleOverride (StyleOverrideCR labelOverride, Utf8P condition, int priority, Utf8P foreColor, Utf8P backColor, Utf8P fontStyle)
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
static void ValidateLocalizationResourceKeyDefinition (LocalizationResourceKeyDefinitionCR definition, int priority, Utf8P id, Utf8P key, Utf8P defaultValue)
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
    RootNodeRuleP rootNodeRule1 = new RootNodeRule ("TestCondition1", 1, false, TargetTree_MainTree, false);
    RootNodeRuleP rootNodeRule2 = new RootNodeRule ("TestCondition2", 3, true, TargetTree_MainTree, false);
    RootNodeRuleP rootNodeRule3 = new RootNodeRule ("TestCondition3", 2, false, TargetTree_MainTree, false);
    ValidateChildNodeRule (*rootNodeRule1, "TestCondition1", 1, false, TargetTree_MainTree);
    ValidateChildNodeRule (*rootNodeRule2, "TestCondition2", 3, true, TargetTree_MainTree);
    ValidateChildNodeRule (*rootNodeRule3, "TestCondition3", 2, false, TargetTree_MainTree);

    ChildNodeRuleP childNodeRule1 = new ChildNodeRule ("TestCondition1", 1, false, TargetTree_MainTree);
    ChildNodeRuleP childNodeRule2 = new ChildNodeRule ("TestCondition2", 3, true, TargetTree_MainTree);
    ChildNodeRuleP childNodeRule3 = new ChildNodeRule ("TestCondition3", 2, false, TargetTree_MainTree);
    ValidateChildNodeRule (*childNodeRule1, "TestCondition1", 1, false, TargetTree_MainTree);
    ValidateChildNodeRule (*childNodeRule2, "TestCondition2", 3, true, TargetTree_MainTree);
    ValidateChildNodeRule (*childNodeRule3, "TestCondition3", 2, false, TargetTree_MainTree);

    ContentRuleP contentRule1 = new ContentRule ("TestCondition1", 1, false);
    ContentRuleP contentRule2 = new ContentRule ("TestCondition2", 3, true);
    ContentRuleP contentRule3 = new ContentRule ("TestCondition3", 2, false);
    ValidateContentRule (*contentRule1, "TestCondition1", 1, false);
    ValidateContentRule (*contentRule2, "TestCondition2", 3, true);
    ValidateContentRule (*contentRule3, "TestCondition3", 2, false);

    ImageIdOverrideP imageIdOverride1 = new ImageIdOverride ("TestCondition1", 1, "Expression1");
    ImageIdOverrideP imageIdOverride2 = new ImageIdOverride ("TestCondition2", 3, "Expression2");
    ImageIdOverrideP imageIdOverride3 = new ImageIdOverride ("TestCondition3", 2, "Expression3");
    ValidateImageIdOverride (*imageIdOverride1, "TestCondition1", "Expression1", 1);
    ValidateImageIdOverride (*imageIdOverride2, "TestCondition2", "Expression2", 3);
    ValidateImageIdOverride (*imageIdOverride3, "TestCondition3", "Expression3", 2);

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance ("File", 1, 0, false, "Supplemental", "BaseElmentSchema", "MyImage", true);
    EXPECT_STREQ ("File", ruleSet->GetRuleSetId ().c_str ());
    EXPECT_STREQ ("BaseElmentSchema", ruleSet->GetSupportedSchemas ().c_str ());
    EXPECT_FALSE (ruleSet->GetIsSupplemental ());
    EXPECT_STREQ ("Supplemental", ruleSet->GetSupplementationPurpose ().c_str ());
    EXPECT_EQ    (1, ruleSet->GetVersionMajor ());
    EXPECT_EQ    (0, ruleSet->GetVersionMinor ());
   
    ruleSet->AddPresentationRule(*rootNodeRule1);
    ruleSet->AddPresentationRule(*rootNodeRule2);
    ruleSet->AddPresentationRule(*rootNodeRule3);

    ruleSet->AddPresentationRule(*childNodeRule1);
    ruleSet->AddPresentationRule(*childNodeRule2);
    ruleSet->AddPresentationRule(*childNodeRule3);

    ruleSet->AddPresentationRule(*contentRule1);
    ruleSet->AddPresentationRule(*contentRule2);
    ruleSet->AddPresentationRule(*contentRule3);

    ruleSet->AddPresentationRule(*imageIdOverride1);
    ruleSet->AddPresentationRule(*imageIdOverride2);
    ruleSet->AddPresentationRule(*imageIdOverride3);
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
    EXPECT_STREQ ("Items", ruleSet->GetRuleSetId ().c_str ());
    EXPECT_STREQ ("E:DgnFileSchema,DgnModelSchema,BaseElementSchema", ruleSet->GetSupportedSchemas ().c_str ());
    EXPECT_FALSE (ruleSet->GetIsSupplemental ());
    EXPECT_EQ    (5, ruleSet->GetVersionMajor ());
    EXPECT_EQ    (3, ruleSet->GetVersionMinor ());

    //Check for root node rules
    int rootRulesCount = 0;
    for (RootNodeRuleList::const_iterator iter = ruleSet->GetRootNodesRules ().begin (); iter != ruleSet->GetRootNodesRules ().end (); ++iter)
        {
        ++rootRulesCount;
        if (1 == rootRulesCount)
            ValidateChildNodeRule (**iter, ""/*default*/, 1000/*default*/, false/*default*/, TargetTree_MainTree);
        else
            ValidateChildNodeRule (**iter, "TestCondition", 12, true, TargetTree_MainTree);
        }
    EXPECT_EQ (2, rootRulesCount);

    //Check for child node rules
    int childNodeRulesCount = 0;
    for (ChildNodeRuleList::const_iterator iter = ruleSet->GetChildNodesRules ().begin (); iter != ruleSet->GetChildNodesRules ().end (); ++iter)
        {
        ++childNodeRulesCount;
        if (1 == childNodeRulesCount)
            ValidateChildNodeRule (**iter, ""/*default*/, 1000/*default*/, false/*default*/, TargetTree_MainTree);
        else
            ValidateChildNodeRule (**iter, "ParentNode.IsSearchNode", 58, true, TargetTree_MainTree);
        }
    EXPECT_EQ (2, childNodeRulesCount);

    //Check for content rules
    int contentRulesCount = 0;
    for (ContentRuleList::const_iterator iter = ruleSet->GetContentRules ().begin (); iter != ruleSet->GetContentRules ().end (); ++iter)
        {
        ++contentRulesCount;
        if (1 == contentRulesCount)
            ValidateContentRule (**iter, ""/*default*/, 1000/*default*/, false/*default*/);
        else
            ValidateContentRule (**iter, "ParentNode.IsClassNode", 97, true);
        }
    EXPECT_EQ (2, contentRulesCount);

    //Check for ImageIdOverride rules
    int imageIdOverrideCount = 0;
    for (ImageIdOverrideList::const_iterator iter = ruleSet->GetImageIdOverrides ().begin (); iter != ruleSet->GetImageIdOverrides ().end (); ++iter)
        {
        ++imageIdOverrideCount;
        if (1 == imageIdOverrideCount)
            ValidateImageIdOverride (**iter, ""/*default*/, "NewImageId1", 1000/*default*/);
        else
            ValidateImageIdOverride (**iter, "ParentNode.IsClassGroupingNode", "NewImageId2", 0);
        }
    EXPECT_EQ (2, imageIdOverrideCount);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, TestPresentationRuleSetSavingToXml)
    {
    //Create PresentationRuleSet and rules usin non-default values, to make sure it saves and loads XML correctly.
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance ("TestRuleSet", 2, 1, true, "Supplemental", "DummySchemaName", "MyImage", true);
    
    RootNodeRuleP rootNodeRule = new RootNodeRule ("TestCondition1", 1, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(*rootNodeRule);

    ChildNodeRuleP childNodeRule = new ChildNodeRule ("TestCondition2", 2, true, TargetTree_Both);
    ruleSet->AddPresentationRule(*childNodeRule);

    ContentRuleP contentRule = new ContentRule ("TestCondition3", 3, true);
    ruleSet->AddPresentationRule(*contentRule);
    
    ruleSet->AddPresentationRule(*new ImageIdOverride("TestCondition4", 4, "ImageIdOverrideTestValue"));
    ruleSet->AddPresentationRule(*new LabelOverride("TestCondition5", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue"));
    ruleSet->AddPresentationRule(*new StyleOverride("TestCondition6", 6, "Blue", "Red", "Bold"));
    ruleSet->AddPresentationRule(*new LocalizationResourceKeyDefinition(7, "UniqueId:)", "LocalizedStringAccessKey", "ThisIsTheValueIfItFails"));
    
    GroupingRuleP groupingRule = new GroupingRule ("TestCondition7", 7, true, "DummySchemaName", "DummyClassName", "ContextMenuCondition", "ContextMenuLabel", "SettingsId");
    ClassGroupP classGroup = new ClassGroup ("ContextMenuLabel", true, "SchemaName", "BaseClassName");
    groupingRule->GetGroups ().push_back (classGroup);
    PropertyGroupP propertyGroup = new PropertyGroup ("ImageId", "ContextMenuLabel", true, "PropertyName");
    propertyGroup->GetRanges ().push_back (new PropertyRangeGroupSpecification ("Label", "ImageId", "FromValue", "ToValue"));
    groupingRule->GetGroups ().push_back (propertyGroup);
    ruleSet->AddPresentationRule(*groupingRule);

    //Serialize RuleSet to string and deserialize from the same string.
    Utf8String serializedRuleSet = ruleSet->WriteToXmlString ();
    PresentationRuleSetPtr loadedRuleSet = PresentationRuleSet::ReadFromXmlString (serializedRuleSet.c_str ());

    //Compare two PresentationRuleSets to check whether it serialized and loaded corectly.
    EXPECT_FALSE (loadedRuleSet.IsNull ());
    EXPECT_STREQ ("TestRuleSet", loadedRuleSet->GetRuleSetId ().c_str ());
    EXPECT_STREQ ("DummySchemaName", loadedRuleSet->GetSupportedSchemas ().c_str ());
    EXPECT_TRUE  (loadedRuleSet->GetIsSupplemental ());
    EXPECT_STREQ ("Supplemental", ruleSet->GetSupplementationPurpose ().c_str ());
    EXPECT_EQ    (2, loadedRuleSet->GetVersionMajor ());
    EXPECT_EQ    (1, loadedRuleSet->GetVersionMinor ());
    EXPECT_STREQ ("MyImage", loadedRuleSet->GetPreferredImage ().c_str ());

    //Check for root node rules
    int rootRulesCount = 0;
    for (RootNodeRuleList::const_iterator iter = ruleSet->GetRootNodesRules ().begin (); iter != ruleSet->GetRootNodesRules ().end (); ++iter)
        {
        ++rootRulesCount;
        ValidateChildNodeRule (**iter, "TestCondition1", 1, true, TargetTree_Both);
        }
    EXPECT_EQ (1, rootRulesCount);

    //Check for child node rules
    int childNodeRulesCount = 0;
    for (ChildNodeRuleList::const_iterator iter = ruleSet->GetChildNodesRules ().begin (); iter != ruleSet->GetChildNodesRules ().end (); ++iter)
        {
        ++childNodeRulesCount;
        ValidateChildNodeRule (**iter, "TestCondition2", 2, true, TargetTree_Both);
        }
    EXPECT_EQ (1, childNodeRulesCount);

    //Check for content rules
    int contentRulesCount = 0;
    for (ContentRuleList::const_iterator iter = ruleSet->GetContentRules ().begin (); iter != ruleSet->GetContentRules ().end (); ++iter)
        {
        ++contentRulesCount;
        ValidateContentRule (**iter, "TestCondition3", 3, true);
        }
    EXPECT_EQ (1, contentRulesCount);

    //Check for ImageIdOverride rules
    int imageIdOverrideCount = 0;
    for (ImageIdOverrideList::const_iterator iter = ruleSet->GetImageIdOverrides ().begin (); iter != ruleSet->GetImageIdOverrides ().end (); ++iter)
        {
        ++imageIdOverrideCount;
        ValidateImageIdOverride (**iter, "TestCondition4", "ImageIdOverrideTestValue", 4);
        }
    EXPECT_EQ (1, imageIdOverrideCount);

    //Check for LabelOverride rules
    int labelOverrideCount = 0;
    for (LabelOverrideList::const_iterator iter = ruleSet->GetLabelOverrides ().begin (); iter != ruleSet->GetLabelOverrides ().end (); ++iter)
        {
        ++labelOverrideCount;
        ValidateLabelOverride (**iter, "TestCondition5", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue");
        }
    EXPECT_EQ (1, labelOverrideCount);

    //Check for StyleOverride rules
    int styleOverrideCount = 0;
    for (StyleOverrideList::const_iterator iter = ruleSet->GetStyleOverrides ().begin (); iter != ruleSet->GetStyleOverrides ().end (); ++iter)
        {
        ++styleOverrideCount;
        ValidateStyleOverride (**iter, "TestCondition6", 6, "Blue", "Red", "Bold");
        }
    EXPECT_EQ (1, styleOverrideCount);
    
    //Check for ValidateLocalizationResourceKeyDefinition rules
    int localizationResourceKeyDefinitionCount = 0;
    for (LocalizationResourceKeyDefinitionList::const_iterator iter = ruleSet->GetLocalizationResourceKeyDefinitions ().begin (); iter != ruleSet->GetLocalizationResourceKeyDefinitions ().end (); ++iter)
        {
        ++localizationResourceKeyDefinitionCount;
        ValidateLocalizationResourceKeyDefinition (**iter, 7, "UniqueId:)", "LocalizedStringAccessKey", "ThisIsTheValueIfItFails");
        }
    EXPECT_EQ (1, localizationResourceKeyDefinitionCount);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, AddPresentationRule_SortsByPriority)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet", 1, 0, false, "", "", "", true);
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());

    RootNodeRuleP rootNodeRule = new RootNodeRule ("TestCondition1", 1, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(1, ruleSet->GetRootNodesRules().size());

    rootNodeRule = new RootNodeRule("TestCondition2", 1, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(2, ruleSet->GetRootNodesRules().size());
    EXPECT_STREQ("TestCondition1", ruleSet->GetRootNodesRules()[0]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition2", ruleSet->GetRootNodesRules()[1]->GetCondition().c_str());

    rootNodeRule = new RootNodeRule("TestCondition3", 2, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(3, ruleSet->GetRootNodesRules().size());
    EXPECT_STREQ("TestCondition3", ruleSet->GetRootNodesRules()[0]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition1", ruleSet->GetRootNodesRules()[1]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition2", ruleSet->GetRootNodesRules()[2]->GetCondition().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, RemovePresentationRule)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet", 1, 0, false, "", "", "", true);
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());

    RootNodeRuleP rootNodeRule = new RootNodeRule("TestCondition1", 1, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(1, ruleSet->GetRootNodesRules().size());

    ruleSet->RemovePresentationRule(*rootNodeRule);
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
