/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavNodeTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    RefCountedPtr<ECInstanceNodeKey> key1 = ECInstanceNodeKey::Create(ECClassId(), ECInstanceId(BeInt64Id(123)), {"a", "b"});
    rapidjson::Document json = key1->AsJson();
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECInstanceNodeKey const* key2 = navNodeKey->AsECInstanceNodeKey();
    ASSERT_NE(nullptr, key2);
    // Validate
    EXPECT_EQ(key1->GetECClassId(), key2->GetECClassId());
    EXPECT_EQ(key1->GetInstanceId(), key2->GetInstanceId());
    EXPECT_EQ(key1->GetPathFromRoot(), key2->GetPathFromRoot());
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
    json["PathFromRoot"][0] = "789";
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECInstanceNodeKey const* key = navNodeKey->AsECInstanceNodeKey();
    ASSERT_NE(nullptr, key);
    // Validate
    EXPECT_EQ(123, key->GetECClassId().GetValue());
    EXPECT_EQ(456, key->GetInstanceId().GetValue());
    ASSERT_EQ(1, key->GetPathFromRoot().size());
    EXPECT_STREQ("789", key->GetPathFromRoot()[0].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, NavNodeKey_RapidJsonSerializationRoundtrip)
    {
    // Serialize
    RefCountedPtr<NavNodeKey> key1 = NavNodeKey::Create("Type", {"123"});
    rapidjson::Document json = key1->AsJson();
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    // Validate
    EXPECT_EQ(key1->GetType(), navNodeKey->GetType());
    EXPECT_EQ(key1->GetPathFromRoot(), navNodeKey->GetPathFromRoot());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, NavNodeKey_JsonValueDeserialization)
    {
    // SetUp json
    Json::Value json;
    json["Type"] = "ECClassGroupingNode";
    json["ECClassId"] = 456;
    json["PathFromRoot"][0] = "123";
    // Deserialize
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(json);
    ASSERT_TRUE(navNodeKey.IsValid());
    ASSERT_TRUE(nullptr != navNodeKey->AsECClassGroupingNodeKey());
    // Validate
    EXPECT_EQ(456, navNodeKey->AsECClassGroupingNodeKey()->GetECClassId().GetValue());
    ASSERT_EQ(1, navNodeKey->GetPathFromRoot().size());
    EXPECT_STREQ("123", navNodeKey->GetPathFromRoot()[0].c_str());
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
