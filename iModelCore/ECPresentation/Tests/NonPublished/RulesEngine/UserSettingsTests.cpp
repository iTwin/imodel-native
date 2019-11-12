/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include "TestHelpers.h"
#include "TestLocalizationProvider.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
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

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, GetsStringValueFromLocalState)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        return "GetsStringValueFromLocalState";
        });
    ASSERT_STREQ("GetsStringValueFromLocalState", GetSettings().GetSettingValue("test").c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, GetsIntValueFromLocalState)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        return 999;
        });
    ASSERT_EQ(999, GetSettings().GetSettingIntValue("test"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, GetsIntValuesFromLocalState)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        Json::Value json(Json::arrayValue);
        json.append(123);
        json.append(456);
        json.append(789);
        return json;
        });

    bvector<int64_t> values = GetSettings().GetSettingIntValues("test");
    ASSERT_EQ(3, values.size());
    EXPECT_EQ(123, values[0]);
    EXPECT_EQ(456, values[1]);
    EXPECT_EQ(789, values[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, GetsBoolValueFromLocalState)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        return true;
        });
    ASSERT_EQ(true, GetSettings().GetSettingBoolValue("test"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsStringValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, JsonValueCR value)
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
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsIntValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, JsonValueCR value)
        {
        EXPECT_STREQ("UserSettingsTests:test", id);
        EXPECT_TRUE(value.isIntegral());
        EXPECT_EQ(666, value.asInt());
        didSave = true;
        });
    GetSettings().SetSettingIntValue("test", 666);
    ASSERT_TRUE(didSave);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsIntValuesToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, JsonValueCR value)
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
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, SetsBoolValueToLocalState)
    {
    bool didSave = false;
    m_localState.SetSaveHandler([&didSave](Utf8CP ns, Utf8CP id, JsonValueCR value)
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
* @bsitest                                      Grigas.Petraitis                01/2016
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
    
    Json::Value localStateValues;
    m_localState.SetGetHandler([&localStateValues](Utf8CP ns, Utf8CP id){return localStateValues[id];});
    m_localState.SetSaveHandler([&localStateValues, &expectedKeys](Utf8CP ns, Utf8CP id, JsonValueCR value)
        {
        auto iter = expectedKeys.find(id);
        ASSERT_TRUE(expectedKeys.end() != iter);
        expectedKeys.erase(iter);
        localStateValues[id] = value;
        });

    GetSettings().InitFrom(rules);
    ASSERT_TRUE(expectedKeys.empty());

    Json::Value expectedPresentationInfo;
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

    Json::Value actualInfo = GetSettings().GetPresentationInfo("locale");
    EXPECT_TRUE(expectedPresentationInfo == actualInfo)
        << "Expected: " << Json::StyledWriter().write(expectedPresentationInfo).c_str() << "\r\n"
        << "Actual:   " << Json::StyledWriter().write(actualInfo).c_str();

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
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

    Json::Value localStateValues;
    m_localState.SetGetHandler([&localStateValues](Utf8CP ns, Utf8CP id){return localStateValues[id];});
    m_localState.SetSaveHandler([&localStateValues, &expectedKeys](Utf8CP ns, Utf8CP id, JsonValueCR value)
        {
        auto iter = expectedKeys.find(id);
        ASSERT_TRUE(expectedKeys.end() != iter);
        expectedKeys.erase(iter);
        localStateValues[id] = value;
        });

    GetSettings().InitFrom(rules);
    ASSERT_TRUE(expectedKeys.empty());
    
    Json::Value expectedPresentationInfo;
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
    
    Json::Value actualInfo = GetSettings().GetPresentationInfo("locale");
    EXPECT_TRUE(expectedPresentationInfo == actualInfo)
        << "Expected: " << Json::StyledWriter().write(expectedPresentationInfo).c_str() << "\r\n"
        << "Actual:   " << Json::StyledWriter().write(actualInfo).c_str();

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
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
    
    Json::Value localStateValues;
    m_localState.SetGetHandler([&localStateValues](Utf8CP ns, Utf8CP id){return localStateValues[id];});
    m_localState.SetSaveHandler([&localStateValues](Utf8CP ns, Utf8CP id, JsonValueCR value)
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
            EXPECT_TRUE(value.isInt());
            EXPECT_EQ(0, value.asInt());
            }
        localStateValues[id] = value;
        });

    GetSettings().InitFrom(rules);

    // additionally, check if Item5_Id has the "options" value "TrueFalse"
    Json::Value presentationInfo = GetSettings().GetPresentationInfo("locale");
    ASSERT_STREQ("TrueFalse", presentationInfo[0]["Items"][4]["Options"].asCString());

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, InitializingFromRulesDoesntOverwriteLocalStateValues)
    {
    UserSettingsGroupList rules;
    UserSettingsGroupP group = new UserSettingsGroup("Group");
    group->AddSettingsItem(*new UserSettingsItem("SettingId", "ItemLabel", "StringValue", "DefaultValue"));
    rules.push_back(group);

    Json::Value localStateValues;
    m_localState.SetGetHandler([&localStateValues](Utf8CP ns, Utf8CP id){return localStateValues[id];});
    m_localState.SetSaveHandler([&localStateValues](Utf8CP ns, Utf8CP id, JsonValueCR value){localStateValues[id] = value;});
    localStateValues["UserSettingsTests:SettingId"] = "PersistedValue";

    GetSettings().InitFrom(rules);
    Utf8String value = GetSettings().GetSettingValue("SettingId");
    EXPECT_STREQ("PersistedValue", value.c_str());

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, LocalizesLabels)
    {
    TestLocalizationProvider localizationProvider;
    localizationProvider.SetHandler([](Utf8StringCR locale, Utf8StringCR id, Utf8StringR str)
        {
        EXPECT_STREQ("test locale", locale.c_str());
        if (id.Equals("Group:LabelId"))
            {
            str = "LocalizedGroupLabel";
            return true;
            }
        if (id.Equals("Item:LabelId"))
            {
            str = "LocalizedItemLabel";
            return true;
            }
        return false;
        });
    GetSettings().SetLocalizationProvider(&localizationProvider);

    UserSettingsGroupList rules;
    UserSettingsGroupP group = new UserSettingsGroup("@Group:LabelId@");
    group->AddSettingsItem(*new UserSettingsItem("ItemId", "@Item:LabelId@", "", ""));
    rules.push_back(group);
    
    GetSettings().InitFrom(rules);
    Json::Value presentationInfo = GetSettings().GetPresentationInfo("test locale");
    ASSERT_STREQ("LocalizedGroupLabel", presentationInfo[0]["Label"].asCString());
    ASSERT_STREQ("LocalizedItemLabel", presentationInfo[0]["Items"][0]["Label"].asCString());

    for (UserSettingsGroupCP rule : rules)
        delete rule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, HasValueReturnsFalseWhenTheresNoValue)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        return Json::Value::GetNull();
        });
    ASSERT_FALSE(GetSettings().HasSetting("test"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsTests, HasValueReturnsTrueWhenTheresAValue)
    {
    m_localState.SetGetHandler([](Utf8CP ns, Utf8CP id)
        {
        if (0 == strcmp("UserSettingsTests:bool", id))
            return Json::Value(true);

        if (0 == strcmp("UserSettingsTests:int", id))
            return Json::Value(9);

        if (0 == strcmp("UserSettingsTests:string", id))
            return Json::Value("setting");

        EXPECT_FALSE(true);
        return Json::Value::GetNull();
        });

    ASSERT_TRUE(GetSettings().HasSetting("bool"));
    ASSERT_TRUE(GetSettings().HasSetting("int"));
    ASSERT_TRUE(GetSettings().HasSetting("string"));
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct UserSettingsRoundtripTests : UserSettingsTests
    {  
    RuntimeJsonLocalState m_localState;
    void SetUp() override
        {
        UserSettingsTests::SetUp();        
        m_settingsManager->SetLocalState(&m_localState);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2017
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
* @bsitest                                      Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, StringValue)
    {
    EXPECT_STREQ("", GetSettings().GetSettingValue("string_value").c_str()); // default

    GetSettings().SetSettingValue("string_value", "test");
    EXPECT_STREQ("test", GetSettings().GetSettingValue("string_value").c_str()); // verify
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, IntValue)
    {
    EXPECT_EQ(0, GetSettings().GetSettingIntValue("int_value")); // default

    GetSettings().SetSettingIntValue("int_value", 123);
    EXPECT_EQ(123, GetSettings().GetSettingIntValue("int_value")); // verify
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, IntValues)
    {
    EXPECT_EQ(bvector<int64_t>(), GetSettings().GetSettingIntValues("int_values")); // default

    GetSettings().SetSettingIntValues("int_values", {123, 456, 789});
    EXPECT_EQ(bvector<int64_t>({123, 456, 789}), GetSettings().GetSettingIntValues("int_values")); // verify
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UserSettingsRoundtripTests, ValueAsJson)
    {
    EXPECT_EQ(Json::Value(Json::nullValue), GetSettings().GetSettingValueAsJson("int_values")); // No value found

    GetSettings().SetSettingBoolValue("bool_value", true);
    EXPECT_EQ(Json::Value(true), GetSettings().GetSettingValueAsJson("bool_value")); // verify

    GetSettings().SetSettingValue("string_value", "test");
    EXPECT_EQ(Json::Value("test"), GetSettings().GetSettingValueAsJson("string_value")); // verify

    GetSettings().SetSettingIntValue("int_value", 123);
    EXPECT_EQ(Json::Value(123), GetSettings().GetSettingValueAsJson("int_value")); // verify

    Json::Value json(Json::arrayValue);
    json.append(123);
    json.append(456);
    json.append(789);
    GetSettings().SetSettingIntValues("int_values", {123, 456, 789});
    EXPECT_EQ(json, GetSettings().GetSettingValueAsJson("int_values")); // verify
    }
