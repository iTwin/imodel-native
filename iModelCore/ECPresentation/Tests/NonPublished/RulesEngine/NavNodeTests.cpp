/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavNodeTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+===============+===============+===============+===============+===============+======*/
struct NavNodeTests : ::testing::Test
{
    static ECDbTestProject* s_project;

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("NavNodeTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    void TearDown() override
        {
        s_project->GetECDb().AbandonChanges();
        }
};
ECDbTestProject* NavNodeTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, ECInstanceNodeKey_RapidJsonSerializationRoundtrip)
    {
    // Serialize
    RefCountedPtr<ECInstanceNodeKey> key1 = ECInstanceNodeKey::Create(ECClassId(), ECInstanceId(BeInt64Id(123)));
    rapidjson::Document json = key1->AsJson();
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECInstanceNodeKey const* key2 = navNodeKey->AsECInstanceNodeKey();
    ASSERT_NE(nullptr, key2);
    // Validate
    EXPECT_EQ(key1->GetECClassId(), key2->GetECClassId());
    EXPECT_EQ(key1->GetInstanceId(), key2->GetInstanceId());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, ECInstanceNodeKey_JsonValueDeserialization)
    {
    // SetUp json
    Json::Value json;
    json["Type"] = "ECInstanceNode";
    json["ECClassId"] = 123;
    json["ECInstanceId"] = 456;
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECInstanceNodeKey const* key = navNodeKey->AsECInstanceNodeKey();
    ASSERT_NE(nullptr, key);
    // Validate
    EXPECT_EQ(123, key->GetECClassId().GetValue());
    EXPECT_EQ(456, key->GetInstanceId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, ECClassGroupingNodeKey_RapidJsonSerializationRoundtrip)
    {
    // Serialize
    RefCountedPtr<ECClassGroupingNodeKey> key1 = ECClassGroupingNodeKey::Create(123, ECClassId());
    rapidjson::Document json = key1->AsJson();
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECClassGroupingNodeKey const* key2 = navNodeKey->AsECClassGroupingNodeKey();
    ASSERT_NE(nullptr, key2);
    // Validate
    EXPECT_EQ(key1->GetECClassId(), key2->GetECClassId());
    EXPECT_EQ(key1->GetNodeId(), key2->GetNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, ECClassGroupingNodeKey_JsonValueDeserialization)
    {
    // SetUp json
    Json::Value json;
    json["Type"] = "ECClassGroupingNode";
    json["NodeId"] = 123;
    json["ECClassId"] = 456;
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECClassGroupingNodeKey const* key = navNodeKey->AsECClassGroupingNodeKey();
    ASSERT_NE(nullptr, key);
    // Validate
    EXPECT_EQ(123, key->GetNodeId());
    EXPECT_EQ(456, key->GetECClassId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, ECPropertyGroupingNodeKey_RapidJsonSerializationRoundtrip)
    {
    // Serialize
    RefCountedPtr<ECPropertyGroupingNodeKey> key1 = ECPropertyGroupingNodeKey::Create(123, ECClassId(), "testProperty", 2, nullptr);
    rapidjson::Document json = key1->AsJson();
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECPropertyGroupingNodeKey const* key2 = navNodeKey->AsECPropertyGroupingNodeKey();
    ASSERT_NE(nullptr, key2);
    // Validate
    EXPECT_EQ(key1->GetECClassId(), key2->GetECClassId());
    EXPECT_EQ(key1->GetNodeId(), key2->GetNodeId());
    EXPECT_STREQ(key1->GetPropertyName().c_str(), key2->GetPropertyName().c_str());
    EXPECT_EQ(key1->GetGroupingRangeIndex(), key2->GetGroupingRangeIndex());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, ECPropertyGroupingNodeKey_JsonValueDeserialization)
    {
    // SetUp json
    Json::Value json;
    json["Type"] = "ECPropertyGroupingNode";
    json["ECClassId"] = 123;
    json["NodeId"] = 456;
    json["PropertyName"] = "propertyName";
    json["RangeIndex"] = 1;
    Json::Value groupingJson;
    groupingJson["Value"] = "value";
    json["GroupingValue"] = groupingJson;
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECPropertyGroupingNodeKey const* key = navNodeKey->AsECPropertyGroupingNodeKey();
    ASSERT_NE(nullptr, key);
    // Validate
    EXPECT_EQ(123, key->GetECClassId().GetValue());
    EXPECT_EQ(456, key->GetNodeId());
    EXPECT_STREQ("propertyName", key->GetPropertyName().c_str());
    EXPECT_EQ(1, key->GetGroupingRangeIndex());
    ASSERT_NE(nullptr, key->GetGroupingValue());
    EXPECT_STREQ("value", key->GetGroupingValue()->FindMember("Value")->value.GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, DisplayLabelGroupingNodeKey_RapidJsonSerializationRoundtrip)
    {
    // Serialize
    RefCountedPtr<DisplayLabelGroupingNodeKey> key1 = DisplayLabelGroupingNodeKey::Create(123, "label");
    rapidjson::Document json = key1->AsJson();
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    DisplayLabelGroupingNodeKey const* key2 = navNodeKey->AsDisplayLabelGroupingNodeKey();
    ASSERT_NE(nullptr, key2);
    // Validate
    EXPECT_EQ(key1->GetNodeId(), key2->GetNodeId());
    EXPECT_STREQ(key1->GetLabel().c_str(), key2->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, DisplayLabelGroupingNodeKey_JsonValueDeserialization)
    {
    // SetUp json
    Json::Value json;
    json["Type"] = "";
    json["NodeId"] = 123;
    json["DisplayLabel"] = "label";
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    DisplayLabelGroupingNodeKey const* key2 = navNodeKey->AsDisplayLabelGroupingNodeKey();
    ASSERT_NE(nullptr, key2);
    // Validate
    EXPECT_EQ(123, key2->GetNodeId());
    EXPECT_STREQ("label", key2->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, NavNodeExtendedData_SetRelatedInstanceKey)
    {
    ECClassCP widget = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    NavNodeExtendedData::RelatedInstanceKey* relatedKey = new NavNodeExtendedData::RelatedInstanceKey(ECInstanceKey(widget->GetId(), ECInstanceId(BeInt64Id(123))), "key");
    NavNodeExtendedData extendedData;
    extendedData.SetRelatedInstanceKey(*relatedKey);

    EXPECT_EQ(1, extendedData.GetRelatedInstanceKeys().size());
    EXPECT_STREQ(relatedKey->GetAlias(), extendedData.GetRelatedInstanceKeys()[0].GetAlias());
    EXPECT_EQ(relatedKey->GetInstanceKey(), extendedData.GetRelatedInstanceKeys()[0].GetInstanceKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, NavNodeExtendedData_SetSkippedInstanceKey)
    {
    ECClassCP widget = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECInstanceKey* key = new ECInstanceKey(widget->GetId(), ECInstanceId(BeInt64Id(123)));
    NavNodeExtendedData extendedData;
    extendedData.SetSkippedInstanceKey(*key);

    EXPECT_EQ(1, extendedData.GetSkippedInstanceKeys().size());
    EXPECT_EQ(key->GetClassId(), extendedData.GetSkippedInstanceKeys()[0].GetClassId());
    EXPECT_EQ(key->GetInstanceId(), extendedData.GetSkippedInstanceKeys()[0].GetInstanceId());
    }