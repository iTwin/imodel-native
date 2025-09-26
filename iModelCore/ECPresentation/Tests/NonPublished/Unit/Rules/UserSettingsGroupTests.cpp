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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json = item.WriteJson();
    BeJsDocument expected(R"({
        "id": "id",
        "label": "label",
        "type": "opts",
        "defaultValue": "default"
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsItemTests, LoadFromJsonFailsWhenIdIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "label": "label"
    })";
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    UserSettingsItem item;
    EXPECT_FALSE(item.ReadJson(json));
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json = group.WriteJson();
    BeJsDocument expected(R"({
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
    EXPECT_TRUE(expected.isExactEqual(json));
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
