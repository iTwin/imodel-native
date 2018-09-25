/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/Published/BeJsonUtilitiesTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/LocalState.h>
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY

struct JsonLocalStateTests : ::testing::Test {};

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(JsonLocalStateTests, GetJsonValue_EmptyState_ReturnsNull)
    {
    RuntimeLocalState localState;
    JsonLocalState jsonLocalState(localState);

    EXPECT_EQ(Json::Value::GetNull(), jsonLocalState.GetJsonValue("Foo", "Boo"));

    EXPECT_EQ(0, localState.GetValues().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
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
// @betest                                      Vincas.Razma
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
// @betest                                      Vincas.Razma
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
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, GetJsonValue_EmptyState_ReturnsNull)
    {
    RuntimeJsonLocalState jsonLocalState;

    EXPECT_EQ(Json::Value::GetNull(), jsonLocalState.GetJsonValue("Foo", "Boo"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
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
// @betest                                      Vincas.Razma
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
// @betest                                      Vincas.Razma
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
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, CtorMove_UsingNonEmpty_StatesDoNotShareMemory)
    {
    char buffer[sizeof(RuntimeJsonLocalState)];
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
// @betest                                      Vincas.Razma
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
// @betest                                      Vincas.Razma
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
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(RuntimeJsonLocalStateTests, OperMoveAssignment_UsingNonEmpty_StatesDoNotShareMemory)
    {
    char buffer[sizeof(RuntimeJsonLocalState)];
    auto a = new (buffer) RuntimeJsonLocalState();
    a->SaveJsonValue("Foo", "Boo", "A");
    EXPECT_EQ(Json::Value("A"), a->GetJsonValue("Foo", "Boo"));

    RuntimeJsonLocalState b = std::move(*a);
    b.SaveJsonValue("Foo", "Boo", "B");

    a->~RuntimeJsonLocalState();
    memset(buffer, 0x0, sizeof(RuntimeJsonLocalState));

    EXPECT_EQ(Json::Value("B"), b.GetJsonValue("Foo", "Boo"));
    }