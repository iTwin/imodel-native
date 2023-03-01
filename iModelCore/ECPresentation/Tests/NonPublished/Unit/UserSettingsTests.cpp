/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/UserSettings.h>
#include "../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UserSettingsTests : ECPresentationTest
    {
    TestLocalState m_localState;
    UserSettingsManager* m_settingsManager;

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        BeFileName temporaryDirectory;
        BeTest::GetHost().GetTempDir(temporaryDirectory);

        m_settingsManager = new UserSettingsManager(temporaryDirectory);
        m_settingsManager->SetLocalState(&m_localState);
        }

    void TearDown() override
        {
        delete m_settingsManager;
        }

    UserSettings& GetSettings() {return m_settingsManager->GetSettings("UserSettingsTests");}
    };

template <typename T>
BeJsDocument convertToBeJsDocument(T value)
    {
    BeJsDocument doc;
    BeJsValue json = doc;
    json = value;
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, GetsStringValueFromLocalState)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        return convertToBeJsDocument("GetsStringValueFromLocalState");
        });
    ASSERT_STREQ("GetsStringValueFromLocalState", GetSettings().GetSettingValue("test").c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, GetsIntValueFromLocalState)
    {
    BeJsDocument json = convertToBeJsDocument(999);
    m_localState.SetGetHandler([&json](Utf8CP ns, Utf8CP id)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        return convertToBeJsDocument(999);
        });
    ASSERT_EQ(999, GetSettings().GetSettingIntValue("test"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, GetsIntValuesFromLocalState)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        BeJsDocument json;
        json[0] = 123;
        json[1] = 456;
        json[2] = 789;
        EXPECT_STREQ("UserSettingsTests:test", id);
        return json;
        });

    bvector<int64_t> values = GetSettings().GetSettingIntValues("test");
    ASSERT_EQ(3, values.size());
    EXPECT_EQ(123, values[0]);
    EXPECT_EQ(456, values[1]);
    EXPECT_EQ(789, values[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, GetsBoolValueFromLocalState)
    {
    BeJsDocument json = convertToBeJsDocument(true);
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        return convertToBeJsDocument(true);
        });
    ASSERT_EQ(true, GetSettings().GetSettingBoolValue("test"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsStringValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isString());
        EXPECT_STREQ("SetsStringValueToLocalState", value.asCString());
        didSave = true;
        });
    GetSettings().SetSettingValue("test", "SetsStringValueToLocalState");
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsDefaultStringValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isString());
        EXPECT_STREQ("", value.asCString());
        didSave = true;
        });
    GetSettings().SetSettingValue("test", "");
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsIntValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isNumeric());
        EXPECT_EQ(666, value.asInt());
        didSave = true;
        });
    GetSettings().SetSettingIntValue("test", 666);
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsDefaultIntValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isNumeric());
        EXPECT_EQ(0, value.asInt());
        didSave = true;
        });
    GetSettings().SetSettingIntValue("test", 0);
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsIntValuesToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isArray());
        ASSERT_EQ(3, value.size());
        EXPECT_EQ(123, value[0].asInt());
        EXPECT_EQ(456, value[1].asInt());
        EXPECT_EQ(789, value[2].asInt());
        didSave = true;
        });
    GetSettings().SetSettingIntValues("test", {123, 456, 789});
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsDefaultIntValuesToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isArray());
        ASSERT_EQ(0, value.size());
        didSave = true;
        });
    GetSettings().SetSettingIntValues("test", {});
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsBoolValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isBool());
        EXPECT_EQ(true, value.asBool());
        didSave = true;
        });
    GetSettings().SetSettingBoolValue("test", true);
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsDefaultBoolValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isBool());
        EXPECT_EQ(false, value.asBool());
        didSave = true;
        });
    GetSettings().SetSettingBoolValue("test", false);
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, InitializesFromRules)
    {
    UserSettingsGroupList rules;
    UserSettingsGroupP group1 = new UserSettingsGroup("Group1");
    group1->AddSettingsItem(*new UserSettingsItem("Item1_Id", "Item1_Label", "YesNo", "true"));
    group1->AddSettingsItem(*new UserSettingsItem("Item2_Id", "Item2_Label", "ShowHide", "false"));
    UserSettingsGroupP group2 = new UserSettingsGroup("Group2");
    group2->AddSettingsItem(*new UserSettingsItem("Item3_Id", "Item3_Label", "StringValue", "DefaultStringValue"));
    group2->AddSettingsItem(*new UserSettingsItem("Item4_Id", "Item4_Label", "IntValue", "999"));
    rules.push_back(group1);
    rules.push_back(group2);

    bset<Utf8String> expectedKeys;
    expectedKeys.insert("UserSettingsTests:Item1_Id");
    expectedKeys.insert("UserSettingsTests:Item2_Id");
    expectedKeys.insert("UserSettingsTests:Item3_Id");
    expectedKeys.insert("UserSettingsTests:Item4_Id");

    BeJsDocument localStateValues;
    m_localState.SetGetHandler([&localStateValues](Utf8CP ns, Utf8CP id){
        BeJsDocument json;
        json.From(localStateValues[id]);
        return json;
        });
    m_localState.SetSaveHandler([&localStateValues, &expectedKeys](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        auto iter = expectedKeys.find(id);
        ASSERT_TRUE(expectedKeys.end() != iter);
        expectedKeys.erase(iter);
        localStateValues[id].From(value);
        });

    GetSettings().InitFrom(rules);
    ASSERT_TRUE(expectedKeys.empty());

    BeJsDocument expectedPresentationInfo;
    expectedPresentationInfo[0]["Label"] = "Group1";
    expectedPresentationInfo[0]["Items"][0]["Id"] = "Item1_Id";
    expectedPresentationInfo[0]["Items"][0]["Label"] = "Item1_Label";
    expectedPresentationInfo[0]["Items"][0]["Options"] = "YesNo";
    expectedPresentationInfo[0]["Items"][0]["Value"] = true;
    expectedPresentationInfo[0]["Items"][1]["Id"] = "Item2_Id";
    expectedPresentationInfo[0]["Items"][1]["Label"] = "Item2_Label";
    expectedPresentationInfo[0]["Items"][1]["Options"] = "ShowHide";
    expectedPresentationInfo[0]["Items"][1]["Value"] = false;
    expectedPresentationInfo[1]["Label"] = "Group2";
    expectedPresentationInfo[1]["Items"][0]["Id"] = "Item3_Id";
    expectedPresentationInfo[1]["Items"][0]["Label"] = "Item3_Label";
    expectedPresentationInfo[1]["Items"][0]["Options"] = "StringValue";
    expectedPresentationInfo[1]["Items"][0]["Value"] = "DefaultStringValue";
    expectedPresentationInfo[1]["Items"][1]["Id"] = "Item4_Id";
    expectedPresentationInfo[1]["Items"][1]["Label"] = "Item4_Label";
    expectedPresentationInfo[1]["Items"][1]["Options"] = "IntValue";
    expectedPresentationInfo[1]["Items"][1]["Value"] = 999;

    BeJsConst actualInfo = GetSettings().GetPresentationInfo();
    EXPECT_TRUE(expectedPresentationInfo.isExactEqual(actualInfo))
        << "Expected: " << expectedPresentationInfo.Stringify().c_str() << "\r\n"
        << "Actual:   " << actualInfo.Stringify().c_str();

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, InitializesFromNestedRules)
    {
    UserSettingsGroupList rules;
    UserSettingsGroupP group = new UserSettingsGroup("Group");
    UserSettingsGroupP nestedGroup = new UserSettingsGroup("NestedGroup");
    group->AddNestedSettings(*nestedGroup);
    nestedGroup->AddSettingsItem(*new UserSettingsItem("Item1_Id", "Item1_Label", "", ""));
    nestedGroup->AddSettingsItem(*new UserSettingsItem("Item2_Id", "Item2_Label", "", ""));
    rules.push_back(group);

    bset<Utf8String> expectedKeys;
    expectedKeys.insert("UserSettingsTests:Item1_Id");
    expectedKeys.insert("UserSettingsTests:Item2_Id");

    BeJsDocument localStateValues;
    m_localState.SetGetHandler([&localStateValues](Utf8CP ns, Utf8CP id){
        BeJsDocument json;
        json.From(localStateValues[id]);
        return json;
        });
    m_localState.SetSaveHandler([&localStateValues, &expectedKeys](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        auto iter = expectedKeys.find(id);
        ASSERT_TRUE(expectedKeys.end() != iter);
        expectedKeys.erase(iter);
        localStateValues[id].From(value);
        });

    GetSettings().InitFrom(rules);
    ASSERT_TRUE(expectedKeys.empty());

    BeJsDocument expectedPresentationInfo;
    expectedPresentationInfo[0]["Label"] = "Group";
    expectedPresentationInfo[0]["NestedGroups"][0]["Label"] = "NestedGroup";
    expectedPresentationInfo[0]["NestedGroups"][0]["Items"][0]["Id"] = "Item1_Id";
    expectedPresentationInfo[0]["NestedGroups"][0]["Items"][0]["Label"] = "Item1_Label";
    expectedPresentationInfo[0]["NestedGroups"][0]["Items"][0]["Options"] = "TrueFalse";
    expectedPresentationInfo[0]["NestedGroups"][0]["Items"][0]["Value"] = false;
    expectedPresentationInfo[0]["NestedGroups"][0]["Items"][1]["Id"] = "Item2_Id";
    expectedPresentationInfo[0]["NestedGroups"][0]["Items"][1]["Label"] = "Item2_Label";
    expectedPresentationInfo[0]["NestedGroups"][0]["Items"][1]["Options"] = "TrueFalse";
    expectedPresentationInfo[0]["NestedGroups"][0]["Items"][1]["Value"] = false;

    BeJsConst actualInfo = GetSettings().GetPresentationInfo();
    EXPECT_TRUE(expectedPresentationInfo.isExactEqual(actualInfo))
        << "Expected: " << expectedPresentationInfo.Stringify().c_str() << "\r\n"
        << "Actual:   " << actualInfo.Stringify().c_str();

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, InitializesFromRulesWithDefaults)
    {
    UserSettingsGroupList rules;
    UserSettingsGroupP group = new UserSettingsGroup("Group");
    group->AddSettingsItem(*new UserSettingsItem("Item1_Id", "Item1_Label", "YesNo", ""));
    group->AddSettingsItem(*new UserSettingsItem("Item2_Id", "Item2_Label", "ShowHide", ""));
    group->AddSettingsItem(*new UserSettingsItem("Item3_Id", "Item3_Label", "StringValue", ""));
    group->AddSettingsItem(*new UserSettingsItem("Item4_Id", "Item4_Label", "IntValue", ""));
    group->AddSettingsItem(*new UserSettingsItem("Item5_Id", "Item5_Label", "", ""));
    rules.push_back(group);

    BeJsDocument localStateValues;
    m_localState.SetGetHandler([&localStateValues](Utf8CP ns, Utf8CP id){
        BeJsDocument json;
        json.From(localStateValues[id]);
        return json;
        });
    m_localState.SetSaveHandler([&localStateValues](Utf8CP ns, Utf8CP id, BeJsConst value)
        {
        if (0 == strcmp("UserSettingsTests:Item1_Id", id) || 0 == strcmp("UserSettingsTests:Item2_Id", id) || 0 == strcmp("UserSettingsTests:Item5_Id", id))
            {
            EXPECT_TRUE(value.isBool());
            EXPECT_FALSE(value.asBool());
            }
        else if (0 == strcmp("UserSettingsTests:Item3_Id", id))
            {
            EXPECT_TRUE(value.isString());
            EXPECT_STREQ("", value.asCString());
            }
        else if (0 == strcmp("UserSettingsTests:Item4_Id", id))
            {
            EXPECT_TRUE(value.isNumeric());
            EXPECT_EQ(0, value.asInt());
            }
        localStateValues[id].From(value);
        });

    GetSettings().InitFrom(rules);

    // additionally, check if Item5_Id has the "options" value "TrueFalse"
    BeJsConst presentationInfo = GetSettings().GetPresentationInfo();
    ASSERT_STREQ("TrueFalse", presentationInfo[0]["Items"][4]["Options"].asCString());

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, InitializingFromRulesDoesntOverwriteLocalStateValues)
    {
    UserSettingsGroupList rules;
    UserSettingsGroupP group = new UserSettingsGroup("Group");
    group->AddSettingsItem(*new UserSettingsItem("SettingId", "ItemLabel", "StringValue", "DefaultValue"));
    rules.push_back(group);

    BeJsDocument localStateValues;
    m_localState.SetGetHandler([&localStateValues](Utf8CP ns, Utf8CP id){
        BeJsDocument json;
        json.From(localStateValues[id]);
        return json;
        });
    m_localState.SetSaveHandler([&localStateValues](Utf8CP ns, Utf8CP id, BeJsConst value){localStateValues[id].From(value);});
    localStateValues["UserSettingsTests:SettingId"] = "PersistedValue";

    GetSettings().InitFrom(rules);
    Utf8String value = GetSettings().GetSettingValue("SettingId");
    EXPECT_STREQ("PersistedValue", value.c_str());

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, HasValueReturnsFalseWhenTheresNoValue)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        BeJsDocument json;
        return json;
        });
    ASSERT_FALSE(GetSettings().HasSetting("test"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, HasValueReturnsTrueWhenTheresAValue)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        if (0 == strcmp("UserSettingsTests:bool", id))
            return convertToBeJsDocument(true);

        if (0 == strcmp("UserSettingsTests:int", id))
            return convertToBeJsDocument(9);

        if (0 == strcmp("UserSettingsTests:string", id))
            return convertToBeJsDocument("setting");

        EXPECT_FALSE(true);
        return convertToBeJsDocument(false);
        });

    ASSERT_TRUE(GetSettings().HasSetting("bool"));
    ASSERT_TRUE(GetSettings().HasSetting("int"));
    ASSERT_TRUE(GetSettings().HasSetting("string"));
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UserSettingsRoundtripTests : UserSettingsTests
    {
    ECPresentation::JsonLocalState m_localState;
    UserSettingsRoundtripTests () : m_localState(std::make_shared<RuntimeLocalState>()) {}
    void SetUp() override
        {
        UserSettingsTests::SetUp();
        m_settingsManager->SetLocalState(&m_localState);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, BoolValue)
    {
    EXPECT_FALSE(GetSettings().GetSettingBoolValue("bool_value")); // default

    GetSettings().SetSettingBoolValue("bool_value", false);
    EXPECT_FALSE(GetSettings().GetSettingBoolValue("bool_value")); // verify

    GetSettings().SetSettingBoolValue("bool_value", true);
    EXPECT_TRUE(GetSettings().GetSettingBoolValue("bool_value")); // verify
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, StringValue)
    {
    EXPECT_STREQ("", GetSettings().GetSettingValue("string_value").c_str()); // default

    GetSettings().SetSettingValue("string_value", "test");
    EXPECT_STREQ("test", GetSettings().GetSettingValue("string_value").c_str()); // verify
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, IntValue)
    {
    EXPECT_EQ(0, GetSettings().GetSettingIntValue("int_value")); // default

    GetSettings().SetSettingIntValue("int_value", 123);
    EXPECT_EQ(123, GetSettings().GetSettingIntValue("int_value")); // verify
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, IntValues)
    {
    EXPECT_EQ(bvector<int64_t>(), GetSettings().GetSettingIntValues("int_values")); // default

    GetSettings().SetSettingIntValues("int_values", {123, 456, 789});
    EXPECT_EQ(bvector<int64_t>({123, 456, 789}), GetSettings().GetSettingIntValues("int_values")); // verify
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, ValueAsJson)
    {
    BeJsDocument doc;
    BeJsValue json = doc;
    EXPECT_TRUE(json.isExactEqual(GetSettings().GetSettingValueAsJson("int_values"))); // No value found

    json = true;
    GetSettings().SetSettingBoolValue("bool_value", true);
    EXPECT_TRUE(json.isExactEqual(GetSettings().GetSettingValueAsJson("bool_value"))); // verify

    json = "test";
    GetSettings().SetSettingValue("string_value", "test");
    EXPECT_TRUE(json.isExactEqual(GetSettings().GetSettingValueAsJson("string_value"))); // verify

    json = 123;
    GetSettings().SetSettingIntValue("int_value", 123);
    EXPECT_TRUE(json.isExactEqual(GetSettings().GetSettingValueAsJson("int_value"))); // verify

    json.SetEmptyArray();
    json[0] = 123;
    json[1] = 456;
    json[2] = 789;
    GetSettings().SetSettingIntValues("int_values", {123, 456, 789});
    EXPECT_TRUE(json.isExactEqual(GetSettings().GetSettingValueAsJson("int_values"))); // verify
    }
