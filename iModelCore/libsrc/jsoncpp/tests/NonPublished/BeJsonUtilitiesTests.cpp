/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/LocalState.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <json/BeJsValue.h>

USING_NAMESPACE_BENTLEY

struct JsonLocalStateTests : ::testing::Test {};

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(JsonLocalStateTests, GetJsonValue_EmptyState_ReturnsNull)
    {
    RuntimeLocalState localState;
    JsonLocalState jsonLocalState(localState);

    EXPECT_EQ(Json::Value::GetNull(), jsonLocalState.GetJsonValue("Foo", "Boo"));

    EXPECT_EQ(0, localState.GetValues().size());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(JsonLocalStateTests, GetJsonValue_IntValueSet_ReturnsSameValue)
    {
    RuntimeLocalState localState;
    JsonLocalState jsonLocalState(localState);

    jsonLocalState.SaveJsonValue("Foo", "Boo", Json::Value(42));
    EXPECT_EQ(Json::Value(42), jsonLocalState.GetJsonValue("Foo", "Boo"));

    EXPECT_EQ(1, localState.GetValues().size());
    EXPECT_STREQ("42\n", localState.GetValue("Foo", "Boo").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(JsonLocalStateTests, GetJsonValue_ObjectValueSet_ReturnsSameValue)
    {
    RuntimeLocalState localState;
    JsonLocalState jsonLocalState(localState);

    Json::Value json;
    json["A"] = "B";
    jsonLocalState.SaveJsonValue("Foo", "Boo", json);
    EXPECT_EQ(json, jsonLocalState.GetJsonValue("Foo", "Boo"));

    EXPECT_EQ(1, localState.GetValues().size());
    EXPECT_STREQ("{\"A\":\"B\"}\n", localState.GetValue("Foo", "Boo").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(JsonLocalStateTests, SaveJsonValue_NullValueRemoved_RemovesValue)
    {
    RuntimeLocalState localState;
    JsonLocalState jsonLocalState(localState);

    jsonLocalState.SaveJsonValue("Foo", "Boo", Json::Value(42));
    EXPECT_EQ(1, localState.GetValues().size());

    jsonLocalState.SaveJsonValue("Foo", "Boo", Json::nullValue);

    EXPECT_EQ(0, localState.GetValues().size());
    EXPECT_STREQ("", localState.GetValue("Foo", "Boo").c_str());

    EXPECT_EQ(Json::Value::GetNull(), jsonLocalState.GetJsonValue("Foo", "Boo"));
    }

struct RuntimeJsonLocalStateTests : ::testing::Test {};

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, GetJsonValue_EmptyState_ReturnsNull)
    {
    RuntimeJsonLocalState jsonLocalState;

    EXPECT_EQ(Json::Value::GetNull(), jsonLocalState.GetJsonValue("Foo", "Boo"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, GetJsonValue_IntValueSet_ReturnsSameValue)
    {
    RuntimeJsonLocalState jsonLocalState;

    jsonLocalState.SaveJsonValue("Foo", "Boo", Json::Value(42));
    EXPECT_EQ(Json::Value(42), jsonLocalState.GetJsonValue("Foo", "Boo"));

    EXPECT_EQ(1, jsonLocalState.GetValues().size());
    EXPECT_STREQ("42\n", jsonLocalState.GetValue("Foo", "Boo").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, SaveJsonValue_NullValueRemoved_RemovesValue)
    {
    RuntimeJsonLocalState jsonLocalState;

    jsonLocalState.SaveJsonValue("Foo", "Boo", Json::Value(42));
    EXPECT_EQ(1, jsonLocalState.GetValues().size());

    jsonLocalState.SaveJsonValue("Foo", "Boo", Json::nullValue);

    EXPECT_EQ(0, jsonLocalState.GetValues().size());
    EXPECT_STREQ("", jsonLocalState.GetValue("Foo", "Boo").c_str());

    EXPECT_EQ(Json::Value::GetNull(), jsonLocalState.GetJsonValue("Foo", "Boo"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, CtorCopy_UsingNonEmpty_StatesDoNotShareMemory)
    {
    RuntimeJsonLocalState a;
    a.SaveJsonValue("Foo", "Boo", "A");

    RuntimeJsonLocalState b(a);
    b.SaveJsonValue("Foo", "Boo", "B");

    EXPECT_EQ(Json::Value("A"), a.GetJsonValue("Foo", "Boo"));
    EXPECT_EQ(Json::Value("B"), b.GetJsonValue("Foo", "Boo"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, CtorMove_UsingNonEmpty_StatesDoNotShareMemory)
    {
    alignas(RuntimeJsonLocalState) uint8_t buffer[sizeof(RuntimeJsonLocalState)];
    auto a = new (buffer) RuntimeJsonLocalState();
    a->SaveJsonValue("Foo", "Boo", "A");
    EXPECT_EQ(Json::Value("A"), a->GetJsonValue("Foo", "Boo"));

    RuntimeJsonLocalState b(std::move(*a));
    b.SaveJsonValue("Foo", "Boo", "B");

    a->~RuntimeJsonLocalState();
    memset(buffer, 0x0, sizeof(RuntimeJsonLocalState));

    EXPECT_EQ(Json::Value("B"), b.GetJsonValue("Foo", "Boo"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, OperCopyAssignment_UsingNonEmpty_StatesDoNotShareMemory)
    {
    RuntimeJsonLocalState a;
    a.SaveJsonValue("Foo", "Boo", "A");

    RuntimeJsonLocalState b = a;
    b.SaveJsonValue("Foo", "Boo", "B");

    EXPECT_EQ(Json::Value("A"), a.GetJsonValue("Foo", "Boo"));
    EXPECT_EQ(Json::Value("B"), b.GetJsonValue("Foo", "Boo"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, OperCopyAssignment_UsingEmptyTemp_StatesDoNotShareMemory)
    {
    RuntimeJsonLocalState b;
    b.SaveJsonValue("Foo", "Boo", "A");

    b = RuntimeJsonLocalState();

    EXPECT_EQ(Json::Value::GetNull(), b.GetJsonValue("Foo", "Boo"));

    b.SaveJsonValue("Foo", "Boo", "B");
    EXPECT_EQ(Json::Value("B"), b.GetJsonValue("Foo", "Boo"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, OperMoveAssignment_UsingNonEmpty_StatesDoNotShareMemory)
    {
    alignas(RuntimeJsonLocalState) uint8_t buffer[sizeof(RuntimeJsonLocalState)];
    auto a = new (buffer) RuntimeJsonLocalState();
    a->SaveJsonValue("Foo", "Boo", "A");
    EXPECT_EQ(Json::Value("A"), a->GetJsonValue("Foo", "Boo"));

    RuntimeJsonLocalState b = std::move(*a);
    b.SaveJsonValue("Foo", "Boo", "B");

    a->~RuntimeJsonLocalState();
    memset(buffer, 0x0, sizeof(RuntimeJsonLocalState));

    EXPECT_EQ(Json::Value("B"), b.GetJsonValue("Foo", "Boo"));
    }

struct StringifyTests : ::testing::Test {};
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StringifyTests, DoubleFormatting)
    {
    // jsoncpp previously used %#.16g to format doubles. We changed it to %$.17g for better precision.
    // They subsequently also bumped it to .17, and ditched the '#'.
    // '#' means "include decimal point even if no digits follow." That produces invalid JSON that JSON.parse() rightly rejects.
    // However, removing "#" can change the value type from double to integer. Lots of our code assumes it will remain a double
    // after round-tripping through string. So we fix by appending trailing zero if last character is decimal point.
    Json::Value val(17363459708551168.0);
    Utf8String str = Json::FastWriter().write(val);
    EXPECT_EQ(str, Utf8String("17363459708551168.0\n")) << str.c_str();
    EXPECT_TRUE(Json::Reader::Parse(str, val));
    EXPECT_TRUE(val.isDouble());
    EXPECT_EQ(val.asDouble(), 17363459708551168.0);

    val = Json::Value(1736345970855116.8);
    str = Json::FastWriter().write(val);
    EXPECT_EQ(str, Utf8String("1736345970855116.8\n")) << str.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GetConstMember, OnlyModifiesNonConstValue)
    {
    Json::Value val(Json::objectValue);
    val["a"] = "A";

    BeJsConst c(val);
    EXPECT_FALSE(c.hasMember("b"));
    EXPECT_TRUE(c["b"].isNull());
    EXPECT_FALSE(c.hasMember("b"));

    BeJsValue nc(val);
    EXPECT_TRUE(nc["b"].isNull());
    EXPECT_TRUE(nc.hasMember("b"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GetConstArrayMember, OnlyModifiesNonConstValue)
    {
    Json::Value val(Json::arrayValue);
    val[0] = 0;

    BeJsConst c(val);
    EXPECT_EQ(c.size(), 1);
    EXPECT_TRUE(c[1].isNull());
    EXPECT_EQ(c.size(), 1);

    BeJsValue nc(val);
    EXPECT_TRUE(nc[1].isNull());
    EXPECT_EQ(nc.size(), 2);
    }

