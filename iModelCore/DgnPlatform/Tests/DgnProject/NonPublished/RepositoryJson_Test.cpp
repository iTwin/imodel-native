/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//========================================================================================
// @bsiclass                                    Shaun.Sewall                    01/2016
//========================================================================================
struct RepositoryJsonTests : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepositoryJsonTests, BeInt64Id)
    {
    BeBriefcaseBasedId briefcaseBasedId(BeBriefcaseId(99), 9999);
    BeInt64Id expectedId;

    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(briefcaseBasedId.ToString(BeInt64Id::UseHex::Yes))));
    ASSERT_EQ(expectedId.GetValue(), briefcaseBasedId.GetValue());
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(briefcaseBasedId.ToString(BeInt64Id::UseHex::No))));
    ASSERT_EQ(expectedId.GetValue(), briefcaseBasedId.GetValue());
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(briefcaseBasedId.GetValue())));
    ASSERT_EQ(expectedId.GetValue(), briefcaseBasedId.GetValue());
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value("42")));
    ASSERT_EQ(expectedId.GetValue(), 42);
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(42)));
    ASSERT_EQ(expectedId.GetValue(), 42);
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value("0X9C")));
    ASSERT_EQ(expectedId.GetValue(), 0x9c);
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(0X9C)));
    ASSERT_EQ(expectedId.GetValue(), 0x9c);
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value("0x8a")));
    ASSERT_EQ(expectedId.GetValue(), 0x8a);
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(0x8a)));
    ASSERT_EQ(expectedId.GetValue(), 0x8a);
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(0)));
    ASSERT_EQ(expectedId.GetValueUnchecked(), 0);
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(BeInt64Id().ToString(BeInt64Id::UseHex::Yes))));
    ASSERT_EQ(expectedId.GetValueUnchecked(), 0);
    ASSERT_TRUE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value(BeInt64Id().ToString(BeInt64Id::UseHex::No))));
    ASSERT_EQ(expectedId.GetValueUnchecked(), 0);

    ASSERT_FALSE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value()));
    ASSERT_FALSE(RepositoryJson::BeInt64IdFromJson(expectedId, Json::Value("")));

    Json::Value expectedJson(briefcaseBasedId.ToString(BeInt64Id::UseHex::Yes));
    Json::Value actualJson;
    RepositoryJson::BeInt64IdToJson(actualJson, briefcaseBasedId);
    ASSERT_EQ(expectedJson, actualJson);
    }
