/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsItemTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "id": "id",
        "label": "label",
        "type": "options",
        "defaultValue": "defaultValue"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    UserSettingsItem item;
    EXPECT_TRUE(item.ReadJson(json));
    EXPECT_STREQ("id", item.GetId().c_str());
    EXPECT_STREQ("label", item.GetLabel().c_str());
    EXPECT_STREQ("options", item.GetOptions().c_str());
    EXPECT_STREQ("defaultValue", item.GetDefaultValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "id": "id",
        "label": "label"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    UserSettingsItem item;
    EXPECT_TRUE(item.ReadJson(json));
    EXPECT_STREQ("id", item.GetId().c_str());
    EXPECT_STREQ("label", item.GetLabel().c_str());
    EXPECT_STREQ("", item.GetOptions().c_str());
    EXPECT_STREQ("", item.GetDefaultValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, WriteToJson)
    {
    UserSettingsItem item("id", "label", "opts", "default");
    Json::Value json = item.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "id": "id",
        "label": "label",
        "type": "opts",
        "defaultValue": "default"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadsFromXml)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadsFromXmlWithDefaultValues)
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadFromJsonFailsWhenIdIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "label": "label"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    UserSettingsItem item;
    EXPECT_FALSE(item.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadFromJsonFailsWhenLabelIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "id": "id"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    UserSettingsItem item;
    EXPECT_FALSE(item.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadFromXmlFailsWhenIdIsNotSpecified)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadFromXmlFailsWhenLabelIsNotSpecified)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, WriteToXml)
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "5d07077af81450bb9502c24c96799400";

    // Make sure that introducing additional attributes with default values don't affect the hash
    UserSettingsItem defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    UserSettingsItem copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsGroupTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "label": "category",
        "vars": [{
            "id": "id",
            "label": "label"
        }],
        "nestedGroups": [{
            "label": "nestedCategory",
            "vars": []
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    UserSettingsGroup group;
    ASSERT_TRUE(group.ReadJson(json));
    EXPECT_STREQ("category", group.GetCategoryLabel().c_str());
    EXPECT_EQ(1, group.GetSettingsItems().size());
    EXPECT_STREQ("id", group.GetSettingsItems()[0]->GetId().c_str());
    EXPECT_STREQ("label", group.GetSettingsItems()[0]->GetLabel().c_str());
    EXPECT_STREQ("", group.GetSettingsItems()[0]->GetOptions().c_str());
    EXPECT_STREQ("", group.GetSettingsItems()[0]->GetDefaultValue().c_str());
    EXPECT_EQ(1, group.GetSettingsItems().size());
    EXPECT_STREQ("nestedCategory", group.GetNestedSettings()[0]->GetCategoryLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    UserSettingsGroup group;
    EXPECT_TRUE(group.ReadJson(json));
    EXPECT_STREQ("", group.GetCategoryLabel().c_str());
    EXPECT_EQ(0, group.GetSettingsItems().size());
    EXPECT_EQ(0, group.GetSettingsItems().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, WriteToJson)
    {
    UserSettingsGroup group("label");
    group.AddNestedSettings(*new UserSettingsGroup("nested"));
    group.AddSettingsItem(*new UserSettingsItem("id", "label", "opts", "default"));
    Json::Value json = group.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "label": "label",
        "nestedGroups": [{
            "label": "nested"
        }],
        "vars": [{
            "id": "id",
            "label": "label",
            "type": "opts",
            "defaultValue": "default"
        }]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, LoadsFromXml)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, LoadsFromXmlWithDefaultValues)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, WriteToXml)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsGroupTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "63ec6237c6eb7091a63a06385b24f366";

    // Make sure that introducing additional attributes with default values don't affect the hash
    UserSettingsGroup defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    UserSettingsGroup copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
