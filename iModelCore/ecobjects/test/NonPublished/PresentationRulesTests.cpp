/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/PresentationRulesTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECPresentationRules/PresentationRules.h>
using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PresentationRulesTests : ECTestFixture
    {
    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
    void ValidateChildNodeRule(ChildNodeRuleCR childNodeRule, Utf8P condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
        {
        EXPECT_STREQ(condition, childNodeRule.GetCondition().c_str());
        EXPECT_EQ(priority, childNodeRule.GetPriority());
        EXPECT_EQ(onlyIfNotHandled, childNodeRule.GetOnlyIfNotHandled());
        //EXPECT_EQ    ((int)targetTree,  (int)childNodeRule.GetTargetTree ());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Eligijus.Mauragas               06/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ValidateContentRule(ContentRuleCR contentRule, Utf8P condition, int priority, bool onlyIfNotHandled)
        {
        EXPECT_STREQ(condition, contentRule.GetCondition().c_str());
        EXPECT_EQ(priority, contentRule.GetPriority());
        EXPECT_EQ(onlyIfNotHandled, contentRule.GetOnlyIfNotHandled());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Eligijus.Mauragas               06/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ValidateImageIdOverride(ImageIdOverrideCR imageIdOverride, Utf8P condition, Utf8P imageIdExpression, int priority)
        {
        EXPECT_STREQ(condition, imageIdOverride.GetCondition().c_str());
        EXPECT_STREQ(imageIdExpression, imageIdOverride.GetImageId().c_str());
        EXPECT_EQ(priority, imageIdOverride.GetPriority());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Eligijus.Mauragas               10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ValidateLabelOverride(LabelOverrideCR labelOverride, Utf8P condition, int priority, Utf8P label, Utf8P description)
        {
        EXPECT_STREQ(condition, labelOverride.GetCondition().c_str());
        EXPECT_EQ(priority, labelOverride.GetPriority());
        EXPECT_STREQ(label, labelOverride.GetLabel().c_str());
        EXPECT_STREQ(description, labelOverride.GetDescription().c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Eligijus.Mauragas               10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ValidateStyleOverride(StyleOverrideCR labelOverride, Utf8P condition, int priority, Utf8P foreColor, Utf8P backColor, Utf8P fontStyle)
        {
        EXPECT_STREQ(condition, labelOverride.GetCondition().c_str());
        EXPECT_EQ(priority, labelOverride.GetPriority());
        EXPECT_STREQ(foreColor, labelOverride.GetForeColor().c_str());
        EXPECT_STREQ(backColor, labelOverride.GetBackColor().c_str());
        EXPECT_STREQ(fontStyle, labelOverride.GetFontStyle().c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Eligijus.Mauragas               10/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ValidateLocalizationResourceKeyDefinition(LocalizationResourceKeyDefinitionCR definition, int priority, Utf8P id, Utf8P key, Utf8P defaultValue)
        {
        EXPECT_EQ(priority, definition.GetPriority());
        EXPECT_STREQ(id, definition.GetId().c_str());
        EXPECT_STREQ(key, definition.GetKey().c_str());
        EXPECT_STREQ(defaultValue, definition.GetDefaultValue().c_str());
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Muhammad.Hassan                     07/16
    //+---------------+---------------+---------------+---------------+---------------+------
    void ValidateCheckBoxRule(CheckBoxRuleCR checkBoxRule, Utf8CP condition, int priority, bool ifNotHandled, Utf8CP propertyName, bool useInversedPropertyVal, bool defaultVal)
        {
        EXPECT_STREQ(condition, checkBoxRule.GetCondition().c_str());
        EXPECT_EQ(priority, checkBoxRule.GetPriority());
        EXPECT_EQ(ifNotHandled, checkBoxRule.GetOnlyIfNotHandled());
        EXPECT_STREQ(propertyName, checkBoxRule.GetPropertyName().c_str());
        EXPECT_EQ(useInversedPropertyVal, checkBoxRule.GetUseInversedPropertyValue());
        EXPECT_EQ(defaultVal, checkBoxRule.GetDefaultValue());
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Muhammad.Hassan                     07/16
    //+---------------+---------------+---------------+---------------+---------------+------
    void ValidateRenameNodeRule(RenameNodeRuleCR renameNodeRule, Utf8CP condition, int priority)
        {
        EXPECT_STREQ(condition, renameNodeRule.GetCondition().c_str());
        EXPECT_EQ(priority, renameNodeRule.GetPriority());
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Muhammad.Hassan                     07/16
    //+---------------+---------------+---------------+---------------+---------------+------
    void ValidateSortingRule(SortingRuleCR sortingRule, Utf8CP condition, int priority, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName, bool sortAscending, bool doNotSort, bool isPolymorphic)
        {
        EXPECT_STREQ(condition, sortingRule.GetCondition().c_str());
        EXPECT_EQ(priority, sortingRule.GetPriority());
        EXPECT_STREQ(schemaName, sortingRule.GetSchemaName().c_str());
        EXPECT_STREQ(className, sortingRule.GetClassName().c_str());
        EXPECT_STREQ(propertyName, sortingRule.GetPropertyName().c_str());
        EXPECT_EQ(sortAscending, sortingRule.GetSortAscending());
        EXPECT_EQ(doNotSort, sortingRule.GetDoNotSort());
        EXPECT_EQ(isPolymorphic, sortingRule.GetIsPolymorphic());
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Muhammad.Hassan                     07/16
    //+---------------+---------------+---------------+---------------+---------------+------
    void ValidateSettingsGroup(UserSettingsGroupCR userSettingsGroup, Utf8CP categoryLabel, int priority)
        {
        EXPECT_STREQ(categoryLabel, userSettingsGroup.GetCategoryLabel().c_str());
        EXPECT_EQ(priority, userSettingsGroup.GetPriority());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, TestPresentationRuleSetCreation)
    {
    RootNodeRuleP rootNodeRule1 = new RootNodeRule("TestCondition1", 1, false, TargetTree_MainTree, false);
    RootNodeRuleP rootNodeRule2 = new RootNodeRule("TestCondition2", 3, true, TargetTree_MainTree, false);
    RootNodeRuleP rootNodeRule3 = new RootNodeRule("TestCondition3", 2, false, TargetTree_MainTree, false);
    ValidateChildNodeRule(*rootNodeRule1, "TestCondition1", 1, false, TargetTree_MainTree);
    ValidateChildNodeRule(*rootNodeRule2, "TestCondition2", 3, true, TargetTree_MainTree);
    ValidateChildNodeRule(*rootNodeRule3, "TestCondition3", 2, false, TargetTree_MainTree);

    ChildNodeRuleP childNodeRule1 = new ChildNodeRule("TestCondition1", 1, false, TargetTree_MainTree);
    ChildNodeRuleP childNodeRule2 = new ChildNodeRule("TestCondition2", 3, true, TargetTree_MainTree);
    ChildNodeRuleP childNodeRule3 = new ChildNodeRule("TestCondition3", 2, false, TargetTree_MainTree);
    ValidateChildNodeRule(*childNodeRule1, "TestCondition1", 1, false, TargetTree_MainTree);
    ValidateChildNodeRule(*childNodeRule2, "TestCondition2", 3, true, TargetTree_MainTree);
    ValidateChildNodeRule(*childNodeRule3, "TestCondition3", 2, false, TargetTree_MainTree);

    ContentRuleP contentRule1 = new ContentRule("TestCondition1", 1, false);
    ContentRuleP contentRule2 = new ContentRule("TestCondition2", 3, true);
    ContentRuleP contentRule3 = new ContentRule("TestCondition3", 2, false);
    ValidateContentRule(*contentRule1, "TestCondition1", 1, false);
    ValidateContentRule(*contentRule2, "TestCondition2", 3, true);
    ValidateContentRule(*contentRule3, "TestCondition3", 2, false);

    ImageIdOverrideP imageIdOverride1 = new ImageIdOverride("TestCondition1", 1, "Expression1");
    ImageIdOverrideP imageIdOverride2 = new ImageIdOverride("TestCondition2", 3, "Expression2");
    ImageIdOverrideP imageIdOverride3 = new ImageIdOverride("TestCondition3", 2, "Expression3");
    ValidateImageIdOverride(*imageIdOverride1, "TestCondition1", "Expression1", 1);
    ValidateImageIdOverride(*imageIdOverride2, "TestCondition2", "Expression2", 3);
    ValidateImageIdOverride(*imageIdOverride3, "TestCondition3", "Expression3", 2);

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("File", 1, 0, false, "Supplemental", "BaseElmentSchema", "MyImage", true);
    EXPECT_STREQ("File", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("BaseElmentSchema", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_FALSE(ruleSet->GetIsSupplemental());
    EXPECT_STREQ("Supplemental", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_EQ(1, ruleSet->GetVersionMajor());
    EXPECT_EQ(0, ruleSet->GetVersionMinor());

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

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromXmlString(ruleSetXmlString);

    EXPECT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("E:DgnFileSchema,DgnModelSchema,BaseElementSchema", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_FALSE(ruleSet->GetIsSupplemental());
    EXPECT_EQ(5, ruleSet->GetVersionMajor());
    EXPECT_EQ(3, ruleSet->GetVersionMinor());

    //Check for root node rules
    int rootRulesCount = 0;
    for (RootNodeRuleList::const_iterator iter = ruleSet->GetRootNodesRules().begin(); iter != ruleSet->GetRootNodesRules().end(); ++iter)
        {
        ++rootRulesCount;
        if (1 == rootRulesCount)
            ValidateChildNodeRule(**iter, ""/*default*/, 1000/*default*/, false/*default*/, TargetTree_MainTree);
        else
            ValidateChildNodeRule(**iter, "TestCondition", 12, true, TargetTree_MainTree);
        }
    EXPECT_EQ(2, rootRulesCount);

    //Check for child node rules
    int childNodeRulesCount = 0;
    for (ChildNodeRuleList::const_iterator iter = ruleSet->GetChildNodesRules().begin(); iter != ruleSet->GetChildNodesRules().end(); ++iter)
        {
        ++childNodeRulesCount;
        if (1 == childNodeRulesCount)
            ValidateChildNodeRule(**iter, ""/*default*/, 1000/*default*/, false/*default*/, TargetTree_MainTree);
        else
            ValidateChildNodeRule(**iter, "ParentNode.IsSearchNode", 58, true, TargetTree_MainTree);
        }
    EXPECT_EQ(2, childNodeRulesCount);

    //Check for content rules
    int contentRulesCount = 0;
    for (ContentRuleList::const_iterator iter = ruleSet->GetContentRules().begin(); iter != ruleSet->GetContentRules().end(); ++iter)
        {
        ++contentRulesCount;
        if (1 == contentRulesCount)
            ValidateContentRule(**iter, ""/*default*/, 1000/*default*/, false/*default*/);
        else
            ValidateContentRule(**iter, "ParentNode.IsClassNode", 97, true);
        }
    EXPECT_EQ(2, contentRulesCount);

    //Check for ImageIdOverride rules
    int imageIdOverrideCount = 0;
    for (ImageIdOverrideList::const_iterator iter = ruleSet->GetImageIdOverrides().begin(); iter != ruleSet->GetImageIdOverrides().end(); ++iter)
        {
        ++imageIdOverrideCount;
        if (1 == imageIdOverrideCount)
            ValidateImageIdOverride(**iter, ""/*default*/, "NewImageId1", 1000/*default*/);
        else
            ValidateImageIdOverride(**iter, "ParentNode.IsClassGroupingNode", "NewImageId2", 0);
        }
    EXPECT_EQ(2, imageIdOverrideCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, TestPresentationRules)
    {
    //Create PresentationRuleSet and rules usin non-default values, to make sure it saves and loads XML correctly.
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet", 2, 1, true, "Supplemental", "DummySchemaName", "MyImage", true);
    ASSERT_TRUE(nullptr != ruleSet.get());

    RootNodeRuleP rootNodeRule = new RootNodeRule("TestCondition1", 1, true, TargetTree_Both, false);
    ASSERT_TRUE(nullptr != rootNodeRule);
    ruleSet->AddPresentationRule(*rootNodeRule);

    ChildNodeRuleP childNodeRule = new ChildNodeRule("TestCondition2", 2, true, TargetTree_Both);
    ASSERT_TRUE(nullptr != childNodeRule);
    ruleSet->AddPresentationRule(*childNodeRule);

    ContentRuleP contentRule = new ContentRule("TestCondition3", 3, true);
    ASSERT_TRUE(nullptr != contentRule);
    ruleSet->AddPresentationRule(*contentRule);

    ruleSet->AddPresentationRule(*new ImageIdOverride("TestCondition4", 4, "ImageIdOverrideTestValue"));
    ruleSet->AddPresentationRule(*new LabelOverride("TestCondition5", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue"));
    ruleSet->AddPresentationRule(*new StyleOverride("TestCondition6", 6, "Blue", "Red", "Bold"));
    ruleSet->AddPresentationRule(*new LocalizationResourceKeyDefinition(7, "UniqueId:)", "LocalizedStringAccessKey", "ThisIsTheValueIfItFails"));

    GroupingRuleP groupingRule = new GroupingRule("TestCondition7", 8, true, "DummySchemaName", "DummyClassName", "ContextMenuCondition", "ContextMenuLabel", "SettingsId");
    ClassGroupP classGroup = new ClassGroup("ContextMenuLabel", true, "SchemaName", "BaseClassName");
    groupingRule->GetGroupsR().push_back(classGroup);
    PropertyGroupP propertyGroup = new PropertyGroup("ImageId", "ContextMenuLabel", true, "PropertyName");
    propertyGroup->GetRangesR().push_back(new PropertyRangeGroupSpecification("Label", "ImageId", "FromValue", "ToValue"));
    groupingRule->GetGroupsR().push_back(propertyGroup);
    ruleSet->AddPresentationRule(*groupingRule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("checkBoxCondition", 9, true, "checkBoxProperty", false, false);
    ASSERT_TRUE(nullptr != checkBoxRule);

    RenameNodeRuleP renameNodeRule = new RenameNodeRule("RenameRuleCondition", 10);
    ASSERT_TRUE(nullptr != renameNodeRule);

    SortingRuleP sortingRule = new SortingRule("SortingCondition", 11, "DummySchemaName", "DummyClassName", "DummyPropertyName", false, true, false);
    ASSERT_TRUE(nullptr != sortingRule);

    UserSettingsGroupP userSettingsGroup = new UserSettingsGroup("UserCategoryLabel");
    ASSERT_TRUE(nullptr != userSettingsGroup);

    ruleSet->AddPresentationRule(*checkBoxRule);
    ruleSet->AddPresentationRule(*renameNodeRule);
    ruleSet->AddPresentationRule(*sortingRule);
    ruleSet->AddPresentationRule(*userSettingsGroup);

    //Serialize RuleSet to string and deserialize from the same string.
    Utf8String serializedRuleSet = ruleSet->WriteToXmlString();
    PresentationRuleSetPtr loadedRuleSet = PresentationRuleSet::ReadFromXmlString(serializedRuleSet.c_str());

    //Compare two PresentationRuleSets to check whether it serialized and loaded corectly.
    EXPECT_FALSE(loadedRuleSet.IsNull());
    EXPECT_STREQ("TestRuleSet", loadedRuleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("DummySchemaName", loadedRuleSet->GetSupportedSchemas().c_str());
    EXPECT_TRUE(loadedRuleSet->GetIsSupplemental());
    EXPECT_STREQ("Supplemental", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_EQ(2, loadedRuleSet->GetVersionMajor());
    EXPECT_EQ(1, loadedRuleSet->GetVersionMinor());
    EXPECT_STREQ("MyImage", loadedRuleSet->GetPreferredImage().c_str());

    //Check for root node rules
    int rootRulesCount = 0;
    for (RootNodeRuleList::const_iterator iter = ruleSet->GetRootNodesRules().begin(); iter != ruleSet->GetRootNodesRules().end(); ++iter)
        {
        ++rootRulesCount;
        ValidateChildNodeRule(**iter, "TestCondition1", 1, true, TargetTree_Both);
        }
    EXPECT_EQ(1, rootRulesCount);

    //Check for child node rules
    int childNodeRulesCount = 0;
    for (ChildNodeRuleList::const_iterator iter = ruleSet->GetChildNodesRules().begin(); iter != ruleSet->GetChildNodesRules().end(); ++iter)
        {
        ++childNodeRulesCount;
        ValidateChildNodeRule(**iter, "TestCondition2", 2, true, TargetTree_Both);
        }
    EXPECT_EQ(1, childNodeRulesCount);

    //Check for content rules
    int contentRulesCount = 0;
    for (ContentRuleList::const_iterator iter = ruleSet->GetContentRules().begin(); iter != ruleSet->GetContentRules().end(); ++iter)
        {
        ++contentRulesCount;
        ValidateContentRule(**iter, "TestCondition3", 3, true);
        }
    EXPECT_EQ(1, contentRulesCount);

    //Check for ImageIdOverride rules
    int imageIdOverrideCount = 0;
    for (ImageIdOverrideList::const_iterator iter = ruleSet->GetImageIdOverrides().begin(); iter != ruleSet->GetImageIdOverrides().end(); ++iter)
        {
        ++imageIdOverrideCount;
        ValidateImageIdOverride(**iter, "TestCondition4", "ImageIdOverrideTestValue", 4);
        }
    EXPECT_EQ(1, imageIdOverrideCount);

    //Check for LabelOverride rules
    int labelOverrideCount = 0;
    for (LabelOverrideList::const_iterator iter = ruleSet->GetLabelOverrides().begin(); iter != ruleSet->GetLabelOverrides().end(); ++iter)
        {
        ++labelOverrideCount;
        ValidateLabelOverride(**iter, "TestCondition5", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue");
        }
    EXPECT_EQ(1, labelOverrideCount);

    //Check for StyleOverride rules
    int styleOverrideCount = 0;
    for (StyleOverrideList::const_iterator iter = ruleSet->GetStyleOverrides().begin(); iter != ruleSet->GetStyleOverrides().end(); ++iter)
        {
        ++styleOverrideCount;
        ValidateStyleOverride(**iter, "TestCondition6", 6, "Blue", "Red", "Bold");
        }
    EXPECT_EQ(1, styleOverrideCount);

    //Check for ValidateLocalizationResourceKeyDefinition rules
    int localizationResourceKeyDefinitionCount = 0;
    for (LocalizationResourceKeyDefinitionList::const_iterator iter = ruleSet->GetLocalizationResourceKeyDefinitions().begin(); iter != ruleSet->GetLocalizationResourceKeyDefinitions().end(); ++iter)
        {
        ++localizationResourceKeyDefinitionCount;
        ValidateLocalizationResourceKeyDefinition(**iter, 7, "UniqueId:)", "LocalizedStringAccessKey", "ThisIsTheValueIfItFails");
        }
    EXPECT_EQ(1, localizationResourceKeyDefinitionCount);

    // check checkbox rule
    int checkBoxRuleCount = 0;
    for (CheckBoxRuleList::const_iterator iter = ruleSet->GetCheckBoxRules().begin(); iter != ruleSet->GetCheckBoxRules().end(); ++iter)
        {
        ++checkBoxRuleCount;
        ValidateCheckBoxRule(**iter, "checkBoxCondition", 9, true, "checkBoxProperty", false, false);
        }
    EXPECT_EQ(1, checkBoxRuleCount);

    // check RenameNodeRule
    int renameNodeRuleCount = 0;
    for (RenameNodeRuleList::const_iterator iter = ruleSet->GetRenameNodeRules().begin(); iter != ruleSet->GetRenameNodeRules().end(); ++iter)
        {
        ++renameNodeRuleCount;
        ValidateRenameNodeRule(**iter, "RenameRuleCondition", 10);
        }
    EXPECT_EQ(1, renameNodeRuleCount);

    // check sorting rule
    int sortingRuleCount = 0;
    for (SortingRuleList::const_iterator iter = ruleSet->GetSortingRules().begin(); iter != ruleSet->GetSortingRules().end(); ++iter)
        {
        ++sortingRuleCount;
        ValidateSortingRule(**iter, "SortingCondition", 11, "DummySchemaName", "DummyClassName", "DummyPropertyName", false, true, false);
        }
    EXPECT_EQ(1, sortingRuleCount);

    // check for userSettingGroup
    int settingsRulesCount = 0;
    for (UserSettingsGroupList::const_iterator iter = ruleSet->GetUserSettings().begin(); iter != ruleSet->GetUserSettings().end(); ++iter)
        {
        ++settingsRulesCount;
        ValidateSettingsGroup(**iter, "UserCategoryLabel", 1000/*Default*/);
        }
    EXPECT_EQ(1, settingsRulesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRulesTests, AddPresentationRule_SortsByPriority)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet", 1, 0, false, "", "", "", true);
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());

    RootNodeRuleP rootNodeRule = new RootNodeRule("TestCondition1", 1, true, TargetTree_Both, false);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ContentRuleTest, GetSetContentRule)
    {
    ContentRuleP contentRule = new ContentRule("TestCondition3", 3, true);
    ASSERT_TRUE(nullptr != contentRule);
    contentRule->SetCustomControl("MyDisplay");

    ASSERT_STREQ("MyDisplay", contentRule->GetCustomControl().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ImageIdOverrideTest, GetSetImageId)
    {
    ImageIdOverrideP imageIdOverride = new ImageIdOverride("TestCondition4", 4, "ImageIdOverrideTestValue");
    ASSERT_TRUE(nullptr != imageIdOverride);

    ImageIdOverrideP imageIdOverride1 = new ImageIdOverride();
    imageIdOverride1->SetCondition("TestCondition4");
    imageIdOverride1->SetImageId("ImageIdOverrideTestValue");

    ASSERT_STREQ(imageIdOverride->GetCondition().c_str(), imageIdOverride1->GetCondition().c_str());
    ASSERT_STREQ(imageIdOverride->GetImageId().c_str(), imageIdOverride1->GetImageId().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(LabelOverrideTest, GetLabelOverrideAttributes)
    {
    LabelOverrideP lo = new LabelOverride("TestCondition5", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue");
    ASSERT_TRUE(nullptr != lo);

    LabelOverrideP lo1 = new LabelOverride();
    lo1->SetCondition("TestCondition5");
    lo1->SetLabel("LabelOverrideLabelValue");
    lo1->SetDescription("LabelOverrideDescriptionValue");

    ASSERT_STREQ(lo->GetCondition().c_str(), lo1->GetCondition().c_str());
    ASSERT_STREQ(lo->GetLabel().c_str(), lo1->GetLabel().c_str());
    ASSERT_STREQ(lo->GetDescription().c_str(), lo1->GetDescription().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(StyleOverrideTest, GetSetForeColor)
    {
    StyleOverrideP styleOverride = new StyleOverride();
    ASSERT_TRUE(nullptr != styleOverride);
    styleOverride->SetForeColor("White");

    ASSERT_STREQ("White", styleOverride->GetForeColor().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(UserSettingsItemTest, CreateUserSettingsItem)
    {
    UserSettingsItemP userSettingsItem = new UserSettingsItem("UserId", "UserLabel", "UserSettingOption", "UserDefaultValue");
    ASSERT_TRUE(nullptr != userSettingsItem);

    ASSERT_STREQ("UserId", userSettingsItem->GetId().c_str());
    ASSERT_STREQ("UserLabel", userSettingsItem->GetLabel().c_str());
    ASSERT_STREQ("UserSettingOption", userSettingsItem->GetOptions().c_str());
    ASSERT_STREQ("UserDefaultValue", userSettingsItem->GetDefaultValue().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(GroupingRulesTest, TestGroupingRules)
    {
    GroupingRuleP groupRuleset = new GroupingRule();
    groupRuleset->SetCondition("GroupTestCondition");
    groupRuleset->SetContextMenuCondition("ContextMenuCondition");

    ClassGroupP classGroup = new ClassGroup("ClassMenuLabel", true, "DummySchemaName", "DummyBaseClass");
    ASSERT_TRUE(nullptr != classGroup);

    PropertyGroupP propertyGroup = new PropertyGroup("PropertyContextMenuLabel", "DummyImageId", true, "DummyPropertyName");
    ASSERT_TRUE(nullptr != propertyGroup);
    propertyGroup->GetRangesR().push_back(new PropertyRangeGroupSpecification("PropertySpecificationLabel", "PropertySpecificationImageId", "1", "5"));

    SameLabelInstanceGroupP sameLabelInstanceGroup = new SameLabelInstanceGroup("sameLabelContextMenuLabel");
    ASSERT_TRUE(nullptr != sameLabelInstanceGroup);

    groupRuleset->GetGroupsR().push_back(classGroup);
    groupRuleset->GetGroupsR().push_back(propertyGroup);
    groupRuleset->GetGroupsR().push_back(sameLabelInstanceGroup);

    ASSERT_STREQ("GroupTestCondition", groupRuleset->GetCondition().c_str());
    ASSERT_STREQ("ContextMenuCondition", groupRuleset->GetContextMenuCondition().c_str());
    ASSERT_STREQ("", groupRuleset->GetContextMenuLabel().c_str());

    GroupList groupsList = groupRuleset->GetGroups();
    for (auto group : groupsList)
        {
        ASSERT_STREQ("", group->GetDefaultLabel().c_str());
        Utf8StringCR menuLabel = group->GetContextMenuLabel();
        if (menuLabel != "ClassMenuLabel" && menuLabel != "PropertyContextMenuLabel" && menuLabel != "sameLabelContextMenuLabel")
            ASSERT_TRUE(false) << group->GetContextMenuLabel().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
void ValidateDisplaySpecifications(DisplayRelatedItemsSpecificationCR displaySpec, bool logicalChildren, int nestedDepth, Utf8CP relationshipClasses)
    {
    EXPECT_EQ(logicalChildren, displaySpec.GetLogicalChildren());
    EXPECT_EQ(nestedDepth, displaySpec.GetNestingDepth());
    EXPECT_STREQ(relationshipClasses, displaySpec.GetRelationshipClasses().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(SelectedNodeInstanceTest, VerifyDisplayRelatedItemsSpecifications)
    {
    SelectedNodeInstancesSpecification spec(1, false, "SchemaComplex", "Class1", false);
    DisplayRelatedItemsSpecificationP displaySpec = new DisplayRelatedItemsSpecification();
    spec.GetDisplayRelatedItems().push_back(new DisplayRelatedItemsSpecification(false, 0, ""));
    spec.GetDisplayRelatedItems().push_back(displaySpec);

    int displayRelatedItemsCount = 0;
    for (DisplayRelatedItemsSpecificationList::const_iterator iter = spec.GetDisplayRelatedItems().begin(); iter != spec.GetDisplayRelatedItems().end(); ++iter)
        {
        ++displayRelatedItemsCount;
        ValidateDisplaySpecifications(**iter, false, 0, "");
        }
    ASSERT_EQ(2, displayRelatedItemsCount);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
