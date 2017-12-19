/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/UserSettingsGroupTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsGroupTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, UserSettingsItem_LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <UserSettingsItem Id="id" Label="label" Options="options" DefaultValue="defaultValue"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    UserSettingsItem item;
    EXPECT_TRUE(item.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("id", item.GetId().c_str());
    EXPECT_STREQ("label", item.GetLabel().c_str());
    EXPECT_STREQ("options", item.GetOptions().c_str());
    EXPECT_STREQ("defaultValue", item.GetDefaultValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, UserSettingsItem_LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = R"(
        <UserSettingsItem Id="id" Label="label"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    UserSettingsItem item;
    EXPECT_TRUE(item.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("id", item.GetId().c_str());
    EXPECT_STREQ("label", item.GetLabel().c_str());
    EXPECT_STREQ("", item.GetOptions().c_str());
    EXPECT_STREQ("", item.GetDefaultValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, UserSettingsItem_LoadFromXmlFailsWhenIdIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <UserSettingsItem Label="label"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    UserSettingsItem item;
    EXPECT_FALSE(item.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, UserSettingsItem_LoadFromXmlFailsWhenLabelIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <UserSettingsItem Id="id"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    UserSettingsItem item;
    EXPECT_FALSE(item.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, UserSettingsGroup_LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <UserSettings CategoryLabel="category">
            <UserSettingsItem Id="id" Label="label"/>
            <UserSettings CategoryLabel="nestedCategory"/>
        </UserSettings>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    UserSettingsGroup group;
    EXPECT_TRUE(group.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("category", group.GetCategoryLabel().c_str());
    EXPECT_EQ(1, group.GetSettingsItems().size());
    EXPECT_STREQ("id", group.GetSettingsItems()[0]->GetId().c_str());
    EXPECT_STREQ("label", group.GetSettingsItems()[0]->GetLabel().c_str());
    EXPECT_STREQ("", group.GetSettingsItems()[0]->GetOptions().c_str());
    EXPECT_STREQ("", group.GetSettingsItems()[0]->GetDefaultValue().c_str());
    EXPECT_EQ(1, group.GetNestedSettings().size());
    EXPECT_STREQ("nestedCategory", group.GetNestedSettings()[0]->GetCategoryLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, UserSettingsGroup_LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<UserSettings/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    UserSettingsGroup group;
    EXPECT_TRUE(group.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", group.GetCategoryLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, UserSettingsItem_WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    UserSettingsItem item("id", "label", "options", "defaultValue");
    item.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<UserSettingsItem Id="id" Label="label" Options="options" DefaultValue="defaultValue"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, UserSettingsGroup_WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    UserSettingsGroup group("category");
    group.AddSettingsItem(*new UserSettingsItem("id", "label", "options", "defaultValue"));
    group.AddNestedSettings(*new UserSettingsGroup("nestedCategory"));
    group.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<UserSettings Priority="1000" CategoryLabel="category">)"
                R"(<UserSettingsItem Id="id" Label="label" Options="options" DefaultValue="defaultValue"/>)"
                R"(<UserSettings Priority="1000" CategoryLabel="nestedCategory"/>)"
            R"(</UserSettings>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, ComputesCorrectHashes)
    {
    UserSettingsGroup group1("category");
    group1.AddSettingsItem(*new UserSettingsItem("id", "label", "options", "defaultValue"));
    group1.AddNestedSettings(*new UserSettingsGroup("nestedCategory"));
    UserSettingsGroup group2("category");
    group2.AddSettingsItem(*new UserSettingsItem("id", "label", "options", "defaultValue"));
    group2.AddNestedSettings(*new UserSettingsGroup("nestedCategory"));
    UserSettingsGroup group3("category");
    group3.AddSettingsItem(*new UserSettingsItem("id", "label", "options", "defaultValue"));

    // Hashes are same for goups with same items
    EXPECT_STREQ(group1.GetHash().c_str(), group2.GetHash().c_str());
    // Hashes differ for groups with different items
    EXPECT_STRNE(group1.GetHash().c_str(), group3.GetHash().c_str());
    }