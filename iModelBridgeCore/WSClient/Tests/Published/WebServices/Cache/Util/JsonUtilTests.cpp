/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/JsonUtilTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "JsonUtilTests.h"

#include "../../../../../Cache/Util/JsonUtil.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

using namespace rapidjson;

#define TEST_DeepCopy(sourceJsonStr) \
    { \
    auto source = ToRapidJson(sourceJsonStr); \
    Document target; \
    JsonUtil::DeepCopy(*source, target); \
    EXPECT_EQ(*source, target); \
    }

#define TEST_DeepCopyJson(source) \
    { \
    Document target; \
    JsonUtil::DeepCopy(source, target); \
    EXPECT_TRUE(source == target); \
    }

#define TEST_AreValuesEqual(a, b) \
    EXPECT_TRUE(JsonUtil::AreValuesEqual(a, b));

#define TEST_AreValuesNotEqual(a, b) \
    EXPECT_FALSE(JsonUtil::AreValuesEqual(a, b)); \
    EXPECT_FALSE(JsonUtil::AreValuesEqual(b, a));

TEST_F(JsonUtilTests, DeepCopy_Empty_Copies)
    {
    TEST_DeepCopy(R"({})");
    TEST_DeepCopy(R"([])");
    }

TEST_F(JsonUtilTests, DeepCopy_PrimitiveValues_Copies)
    {
    TEST_DeepCopyJson(Value());
    TEST_DeepCopyJson(Value("StringValue"));
    TEST_DeepCopyJson(Value(42));
    TEST_DeepCopyJson(Value(-42));
    TEST_DeepCopyJson(Value(6.28));
    TEST_DeepCopyJson(Value(-6.28));
    TEST_DeepCopyJson(Value(true));
    TEST_DeepCopyJson(Value(false));
    }

TEST_F(JsonUtilTests, DeepCopy_ObjectWithPrimitiveValues_Copies)
    {
    TEST_DeepCopy(R"({"Field" : null})");
    TEST_DeepCopy(R"({"Field" : "StringValue"})");
    TEST_DeepCopy(R"({"Field" : 42})");
    TEST_DeepCopy(R"({"Field" : -42})");
    TEST_DeepCopy(R"({"Field" : 6.28})");
    TEST_DeepCopy(R"({"Field" : -6.28})");
    TEST_DeepCopy(R"({"Field" : true})");
    TEST_DeepCopy(R"({"Field" : false})");
    }

TEST_F(JsonUtilTests, DeepCopy_ObjectWithObjects_Copies)
    {
    TEST_DeepCopy(R"({
        "A" : { 
            "Foo" : "Value"
            },
        "B" : { 
            "Foo" : 456
            }})");
    }

TEST_F(JsonUtilTests, DeepCopy_ObjectWithArray_Copies)
    {
    TEST_DeepCopy(R"({ "Foo" : [1, 2, 3]})");
    }

TEST_F(JsonUtilTests, DeepCopy_Array_Copies)
    {
    TEST_DeepCopy(R"([1, 2, 3])");
    TEST_DeepCopy(R"(["a", "b", "c"])");
    TEST_DeepCopy(R"([{ "Foo" : "A" }, { "Foo" : "B" }])");
    }

TEST_F(JsonUtilTests, DeepCopy_TargetObjectContainsUnrelatedValues_LeavesValues)
    {
    auto source = ToRapidJson(R"({"A" : "ValA"})");
    auto target = ToRapidJson(R"({"B" : "ValB"})");
    JsonUtil::DeepCopy(*source, *target);
    EXPECT_EQ(*ToRapidJson(R"({"A" : "ValA", "B" : "ValB"})"), *target);
    }

TEST_F(JsonUtilTests, DeepCopy_TargetObjectContainsSameFieldWithDifferentValue_OverridesValue)
    {
    auto source = ToRapidJson(R"({"Foo" : "SourceVal"})");
    auto target = ToRapidJson(R"({"Foo" : "TargetVal"})");
    JsonUtil::DeepCopy(*source, *target);
    EXPECT_EQ(*ToRapidJson(R"({"Foo" : "SourceVal"})"), *target);
    }

TEST_F(JsonUtilTests, DeepCopy_TargetObjectContainsSameFieldWithDifferentType_OverridesValue)
    {
    auto source = ToRapidJson(R"({"Foo" : "SourceVal"})");
    auto target = ToRapidJson(R"({"Foo" : 123})");
    JsonUtil::DeepCopy(*source, *target);
    EXPECT_EQ(*ToRapidJson(R"({"Foo" : "SourceVal"})"), *target);
    }

TEST_F(JsonUtilTests, DeepCopy_TargetObjectContainsSameFieldWithArray_OverridesArray)
    {
    auto source = ToRapidJson(R"({"Foo" : [4, 5, 6]})");
    auto target = ToRapidJson(R"({"Foo" : [7, 8]})");
    JsonUtil::DeepCopy(*source, *target);
    EXPECT_EQ(*ToRapidJson(R"({"Foo" : [4, 5, 6]})"), *target);
    }

TEST_F(JsonUtilTests, DeepCopy_SourceIsNullAndTargetIsObject_OverridesWithNull)
    {
    Document source;
    auto target = ToRapidJson(R"({"Foo" : "Booo"})");
    JsonUtil::DeepCopy(source, *target);
    EXPECT_EQ(Value(), *target);
    }

TEST_F(JsonUtilTests, AreValuesEqual_EqualPrimitiveValues_True)
    {
    TEST_AreValuesEqual(Value(kNullType), Value(kNullType));
    TEST_AreValuesEqual(Value(kTrueType), Value(kTrueType));
    TEST_AreValuesEqual(Value(kFalseType), Value(kFalseType));
    TEST_AreValuesEqual(Value(true), Value(true));
    TEST_AreValuesEqual(Value(false), Value(false));
    TEST_AreValuesEqual(Value(42), Value(42));
    TEST_AreValuesEqual(Value(-42), Value(-42));
    TEST_AreValuesEqual(Value(6.28), Value(6.28));
    TEST_AreValuesEqual(Value(-6.28), Value(-6.28));
    TEST_AreValuesEqual(Value("TestString"), Value("TestString"));
    TEST_AreValuesEqual(Value(""), Value(""));
    }

TEST_F(JsonUtilTests, AreValuesEqual_NotequalPrimitiveValues_False)
    {
    TEST_AreValuesNotEqual(Value(kFalseType), Value(kTrueType));
    TEST_AreValuesNotEqual(Value(true), Value(false));
    TEST_AreValuesNotEqual(Value(42), Value(222));
    TEST_AreValuesNotEqual(Value(-42), Value(-222));
    TEST_AreValuesNotEqual(Value(6.28), Value(3.14));
    TEST_AreValuesNotEqual(Value(-6.28), Value(-3.14));
    TEST_AreValuesNotEqual(Value("TestString"), Value("OtherString"));
    TEST_AreValuesNotEqual(Value(""), Value("Boo"));
    }

TEST_F(JsonUtilTests, AreValuesEqual_NotequalPrimitiveTypes_False)
    {
    TEST_AreValuesNotEqual(Value(kNullType), Value(42));
    TEST_AreValuesNotEqual(Value(kNullType), Value(3.14));
    TEST_AreValuesNotEqual(Value(kNullType), Value(false));
    TEST_AreValuesNotEqual(Value(kNullType), Value(true));
    TEST_AreValuesNotEqual(Value(kNullType), Value("Foo"));
    TEST_AreValuesNotEqual(Value(kNullType), Value(kObjectType));
    TEST_AreValuesNotEqual(Value(kNullType), Value(kArrayType));

    TEST_AreValuesNotEqual(Value(false), Value(42));
    TEST_AreValuesNotEqual(Value(false), Value(3.14));
    TEST_AreValuesNotEqual(Value(false), Value(kNullType));
    TEST_AreValuesNotEqual(Value(false), Value(true));
    TEST_AreValuesNotEqual(Value(false), Value("Foo"));
    TEST_AreValuesNotEqual(Value(false), Value(kObjectType));
    TEST_AreValuesNotEqual(Value(false), Value(kArrayType));

    TEST_AreValuesNotEqual(Value(true), Value(42));
    TEST_AreValuesNotEqual(Value(true), Value(3.14));
    TEST_AreValuesNotEqual(Value(true), Value(false));
    TEST_AreValuesNotEqual(Value(true), Value(kNullType));
    TEST_AreValuesNotEqual(Value(true), Value("Foo"));
    TEST_AreValuesNotEqual(Value(true), Value(kObjectType));
    TEST_AreValuesNotEqual(Value(true), Value(kArrayType));

    TEST_AreValuesNotEqual(Value(42), Value(kNullType));
    TEST_AreValuesNotEqual(Value(42), Value(3.14));
    TEST_AreValuesNotEqual(Value(42), Value(false));
    TEST_AreValuesNotEqual(Value(42), Value(true));
    TEST_AreValuesNotEqual(Value(42), Value("Foo"));
    TEST_AreValuesNotEqual(Value(42), Value(kObjectType));
    TEST_AreValuesNotEqual(Value(42), Value(kArrayType));

    TEST_AreValuesNotEqual(Value(6.14), Value(42));
    TEST_AreValuesNotEqual(Value(6.14), Value(kNullType));
    TEST_AreValuesNotEqual(Value(6.14), Value(false));
    TEST_AreValuesNotEqual(Value(6.14), Value(true));
    TEST_AreValuesNotEqual(Value(6.14), Value("Foo"));
    TEST_AreValuesNotEqual(Value(6.14), Value(kObjectType));
    TEST_AreValuesNotEqual(Value(6.14), Value(kArrayType));

    TEST_AreValuesNotEqual(Value("Foo"), Value(42));
    TEST_AreValuesNotEqual(Value("Foo"), Value(3.14));
    TEST_AreValuesNotEqual(Value("Foo"), Value(false));
    TEST_AreValuesNotEqual(Value("Foo"), Value(true));
    TEST_AreValuesNotEqual(Value("Foo"), Value(kNullType));
    TEST_AreValuesNotEqual(Value("Foo"), Value(kObjectType));
    TEST_AreValuesNotEqual(Value("Foo"), Value(kArrayType));

    TEST_AreValuesNotEqual(Value(kObjectType), Value(42));
    TEST_AreValuesNotEqual(Value(kObjectType), Value(3.14));
    TEST_AreValuesNotEqual(Value(kObjectType), Value(false));
    TEST_AreValuesNotEqual(Value(kObjectType), Value(true));
    TEST_AreValuesNotEqual(Value(kObjectType), Value("Foo"));
    TEST_AreValuesNotEqual(Value(kObjectType), Value(kNullType));
    TEST_AreValuesNotEqual(Value(kObjectType), Value(kArrayType));

    TEST_AreValuesNotEqual(Value(kArrayType), Value(42));
    TEST_AreValuesNotEqual(Value(kArrayType), Value(3.14));
    TEST_AreValuesNotEqual(Value(kArrayType), Value(false));
    TEST_AreValuesNotEqual(Value(kArrayType), Value(true));
    TEST_AreValuesNotEqual(Value(kArrayType), Value("Foo"));
    TEST_AreValuesNotEqual(Value(kArrayType), Value(kObjectType));
    TEST_AreValuesNotEqual(Value(kArrayType), Value(kNullType));
    }

TEST_F(JsonUtilTests, AreValuesEqual_EqualObjects_True)
    {
    TEST_AreValuesEqual(*ToRapidJson(R"({"Foo" : "Boo"})"), *ToRapidJson(R"({"Foo" : "Boo"})"));
    TEST_AreValuesEqual(*ToRapidJson(R"({"Foo" : {}})"), *ToRapidJson(R"({"Foo" : {}})"));
    }

TEST_F(JsonUtilTests, AreValuesEqual_NotEqualObjects_False)
    {
    TEST_AreValuesNotEqual(*ToRapidJson(R"({"A" : "Boo"})"), *ToRapidJson(R"({"B" : "Boo"})"));
    TEST_AreValuesNotEqual(*ToRapidJson(R"({"Foo" : 1})"), *ToRapidJson(R"({"Foo" : 2})"));
    }

TEST_F(JsonUtilTests, AreValuesEqual_EqualArrays_True)
    {
    TEST_AreValuesEqual(*ToRapidJson(R"([])"), *ToRapidJson(R"([])"));
    TEST_AreValuesEqual(*ToRapidJson(R"([1, 2, 3])"), *ToRapidJson(R"([1, 2, 3])"));
    }

TEST_F(JsonUtilTests, AreValuesEqual_NotEqualArrays_False)
    {
    TEST_AreValuesNotEqual(*ToRapidJson(R"([])"), *ToRapidJson(R"([1, 2])"));
    TEST_AreValuesNotEqual(*ToRapidJson(R"([1, 2])"), *ToRapidJson(R"([])"));
    TEST_AreValuesNotEqual(*ToRapidJson(R"([1, 2, 3])"), *ToRapidJson(R"([1, 2])"));
    }
