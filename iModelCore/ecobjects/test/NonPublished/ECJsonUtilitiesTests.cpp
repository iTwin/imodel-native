/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <Bentley/BeId.h>
#include <ostream>
#include <limits>
#include <type_traits>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECJsonUtilitiesTestFixture : ECTestFixture
    {

protected:
    static BentleyStatus ParseJson(BeJsDocument& json, Utf8StringCR jsonStr) { json.Parse(jsonStr); return json.hasParseError() ? ERROR : SUCCESS; }
    static BentleyStatus ParseJson(rapidjson::Document& json, Utf8StringCR jsonStr) { return json.Parse<0>(jsonStr.c_str()).HasParseError() ? ERROR : SUCCESS; }
    template <typename T>
    static void runTests(BeJsDocument& json, T placeholder)
        {
        json["test"] = static_cast<T>(2);
        EXPECT_EQ(BeInt64Id(2), ECJsonUtilities::JsonToId<BeInt64Id>(json["test"]));
        json["test"] = static_cast<T>(-1);
        EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(json["test"]));

        json["test"] = static_cast<T>(std::numeric_limits<T>::max());
        // A max float / double is outside the range of a uint64, so we should get an invalid id.
        if constexpr (std::is_same<T, double>::value || std::is_same<T, float>::value)
            EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(json["test"]));
        else 
            EXPECT_EQ(BeInt64Id(std::numeric_limits<T>::max()), ECJsonUtilities::JsonToId<BeInt64Id>(json["test"]));
        }
    template <typename T>
    static void runTests(rapidjson::Document& json, T placeholder)
        {
        json.SetObject();
        json.AddMember("test", static_cast<T>(2), json.GetAllocator());
        EXPECT_EQ(BeInt64Id(2), ECJsonUtilities::JsonToId<BeInt64Id>(json["test"]));
        json.RemoveMember("test");
        json.AddMember("test", static_cast<T>(-1), json.GetAllocator());
        EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(json["test"]));
        json.RemoveMember("test");

        json.AddMember("test", static_cast<T>(std::numeric_limits<T>::max()), json.GetAllocator());
        // A max float / double is outside the range of a uint64, so we should get an invalid id.
        if constexpr (std::is_same<T, double>::value || std::is_same<T, float>::value)
            EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(json["test"]));
        else 
            EXPECT_EQ(BeInt64Id(std::numeric_limits<T>::max()), ECJsonUtilities::JsonToId<BeInt64Id>(json["test"]));
        json.SetNull();
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeInt64Id id, std::ostream* os) { *os << id.GetValueUnchecked(); }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToId)
    {
    BeJsDocument jsonDoc;
    rapidjson::Document rapidJson;

    runTests(jsonDoc, int64_t(0));
    runTests(jsonDoc, int32_t(0));
    runTests(jsonDoc, double(0)); 
    runTests(jsonDoc, float(0));

    runTests(rapidJson, int64_t(0));
    runTests(rapidJson, int32_t(0));
    runTests(rapidJson, double(0));
    runTests(rapidJson, float(0));

    //Ids formatted numerically in JSON
    Utf8CP jsonStr = "1234";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = "-10"; // Not a valid value for an Id, but failing the method is not worth the overhead. So the test documents the behavior of the API
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = "3.14";  // Not a valid value for an Id, but failing the method is not worth the overhead. So the test documents the behavior of the API
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr << " floating numbers are given defaultValue of 0.";

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr << " floating numbers are given defaultValue of 0.";

    //Ids formatted as decimal strings in JSON
    jsonStr = R"json("1234")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("1099511627775")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1099511627775)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1099511627775)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("-10")json";  // Negative numbers are invalid Ids.

    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("3.14")json";  // Floating point numbers are invalid Ids.

    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr << " floating numbers are given defaultValue of 0.";

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(0), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr << " floating numbers are given defaultValue of 0.";

    //Ids formatted as hexadecimal strings in JSON
    jsonStr = R"json("0x123")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(0x123)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(0x123)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("0xFFFFFFFFFF")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(0xFFFFFFFFFF)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonDoc)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(0xFFFFFFFFFF)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, IdToJson)
    {
    BeJsDocument jsonDoc;
    rapidjson::Document rapidJson;

    BeInt64Id id(UINT64_C(1234));

    ASSERT_EQ(SUCCESS, ECJsonUtilities::IdToJson(jsonDoc, id)) << id.ToString();
    ASSERT_TRUE(jsonDoc.isString()) << id.ToString();
    ASSERT_STRCASEEQ("0x4d2", jsonDoc.asCString()) << id.ToString();

    ASSERT_EQ(SUCCESS, ECJsonUtilities::IdToJson(rapidJson, id, rapidJson.GetAllocator())) << id.ToString();
    ASSERT_TRUE(rapidJson.IsString()) << id.ToString();
    ASSERT_STRCASEEQ("0x4d2", rapidJson.GetString()) << id.ToString();

    id = BeInt64Id(UINT64_C(1099511627775));

    ASSERT_EQ(SUCCESS, ECJsonUtilities::IdToJson(jsonDoc, id)) << id.ToString();
    ASSERT_TRUE(jsonDoc.isString()) << id.ToString();
    ASSERT_STRCASEEQ("0xffffffffff", jsonDoc.asCString()) << id.ToString();

    ASSERT_EQ(SUCCESS, ECJsonUtilities::IdToJson(rapidJson, id, rapidJson.GetAllocator())) << id.ToString();
    ASSERT_TRUE(rapidJson.IsString()) << id.ToString();
    ASSERT_STRCASEEQ("0xffffffffff", rapidJson.GetString()) << id.ToString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToInt64)
    {
    BeJsDocument jsonDoc;
    rapidjson::Document rapidJson;

    Utf8CP jsonStr = nullptr;
    int64_t val;

    //formatted numerically in JSON
    jsonStr = "1234";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr;
    EXPECT_EQ(INT64_C(1234), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(1234), val) << jsonStr;

    jsonStr = "-10";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr;
    EXPECT_EQ(INT64_C(-10), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(-10), val) << jsonStr;

    jsonStr = "3.14";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr << " floating numbers are rounded";
    EXPECT_EQ(INT64_C(3), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr << " floating numbers are rounded";
    EXPECT_EQ(INT64_C(3), val) << jsonStr;

    //formatted as decimal strings in JSON
    jsonStr = R"json("1234")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr;
    EXPECT_EQ(INT64_C(1234), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(1234), val) << jsonStr;

    jsonStr = R"json("1099511627775")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr;
    EXPECT_EQ(INT64_C(1099511627775), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(1099511627775), val) << jsonStr;

    jsonStr = R"json("-10")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr;
    EXPECT_EQ(INT64_C(-10), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(-10), val) << jsonStr;

    jsonStr = R"json("3.14")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr << " floating numbers are rounded";
    EXPECT_EQ(INT64_C(3), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr << " floating numbers are rounded";
    EXPECT_EQ(INT64_C(3), val) << jsonStr;

    //Ids formatted as hexadecimal strings in JSON
    jsonStr = R"json("0x123")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr;
    EXPECT_EQ(INT64_C(0x123), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(0x123), val) << jsonStr;

    jsonStr = R"json("0xFFFFFFFFFF")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonDoc)) << jsonStr;
    EXPECT_EQ(INT64_C(0xFFFFFFFFFF), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(0xFFFFFFFFFF), val) << jsonStr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, Int64ToJson)
    {
    BeJsDocument jsonDoc;
    rapidjson::Document rapidJson;

    int64_t val = INT64_C(1234);

    ECJsonUtilities::Int64ToJson(jsonDoc, val, ECJsonInt64Format::AsNumber);
    // TODO: add isIntegral() function
    // ASSERT_TRUE(jsonDoc.isIntegral()) << val;
    ASSERT_EQ(val, jsonDoc.asInt64()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(rapidJson.IsInt64()) << val;
    ASSERT_EQ(val, rapidJson.GetInt64()) << val;


    ECJsonUtilities::Int64ToJson(jsonDoc, val, ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(jsonDoc.isString()) << val;
    ASSERT_STRCASEEQ("1234", jsonDoc.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("1234", rapidJson.GetString()) << val;

    ECJsonUtilities::Int64ToJson(jsonDoc, val, ECJsonInt64Format::AsHexadecimalString);
    ASSERT_TRUE(jsonDoc.isString()) << val;
    ASSERT_STRCASEEQ("0x4d2", jsonDoc.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsHexadecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("0x4d2", rapidJson.GetString()) << val;

    val = INT64_C(0xffffffffff);

    ECJsonUtilities::Int64ToJson(jsonDoc, val, ECJsonInt64Format::AsNumber);
    // TODO: add isIntegral() function
    // ASSERT_TRUE(jsonDoc.isIntegral()) << val;
    ASSERT_EQ(val, jsonDoc.asInt64()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(rapidJson.IsInt64()) << val;
    ASSERT_EQ(val, rapidJson.GetInt64()) << val;


    ECJsonUtilities::Int64ToJson(jsonDoc, val, ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(jsonDoc.isString()) << val;
    ASSERT_STRCASEEQ("1099511627775", jsonDoc.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("1099511627775", rapidJson.GetString()) << val;

    ECJsonUtilities::Int64ToJson(jsonDoc, val, ECJsonInt64Format::AsHexadecimalString);
    ASSERT_TRUE(jsonDoc.isString()) << val;
    ASSERT_STRCASEEQ("0xffffffffff", jsonDoc.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsHexadecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("0xffffffffff", rapidJson.GetString()) << val;


    val = INT64_C(-10);

    ECJsonUtilities::Int64ToJson(jsonDoc, val, ECJsonInt64Format::AsNumber);
    // TODO: add isIntegral() function
    // ASSERT_TRUE(jsonDoc.isIntegral()) << val;
    ASSERT_EQ(val, jsonDoc.asInt64()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(rapidJson.IsInt64()) << val;
    ASSERT_EQ(val, rapidJson.GetInt64()) << val;

    ECJsonUtilities::Int64ToJson(jsonDoc, val, ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(jsonDoc.isString()) << val;
    ASSERT_STRCASEEQ("-10", jsonDoc.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("-10", rapidJson.GetString()) << val;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToDateTime)
    {
    BeJsDocument jsonDoc;
    rapidjson::Document rapidJson;

    Utf8CP jsonStr = nullptr;
    DateTime dt;

    jsonStr = R"json("2017-11-20T10:45:32.111Z")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonDoc)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    jsonStr = R"json("2017-11-20 10:45:32.111Z")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonDoc)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    jsonStr = R"json("2017-11-20T10:45:32.111")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonDoc)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    jsonStr = R"json("2017-11-20 10:45:32.111")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonDoc)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    jsonStr = R"json("2017-11-20")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonDoc, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonDoc)) << jsonStr;
    EXPECT_EQ(DateTime(2017, 11, 20), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(2017, 11, 20), dt) << jsonStr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, DateTimeToJson)
    {
    BeJsDocument jsonDoc;
    rapidjson::Document rapidJson;

    DateTime dt(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111);

    ECJsonUtilities::DateTimeToJson(jsonDoc, dt);
    ASSERT_TRUE(jsonDoc.isString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20T10:45:32.111Z", jsonDoc.asCString()) << dt.ToString();

    ECJsonUtilities::DateTimeToJson(rapidJson, dt, rapidJson.GetAllocator());
    ASSERT_TRUE(rapidJson.IsString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20T10:45:32.111Z", rapidJson.GetString()) << dt.ToString();

    dt = DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111);

    ECJsonUtilities::DateTimeToJson(jsonDoc, dt);
    ASSERT_TRUE(jsonDoc.isString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20T10:45:32.111", jsonDoc.asCString()) << dt.ToString();

    ECJsonUtilities::DateTimeToJson(rapidJson, dt, rapidJson.GetAllocator());
    ASSERT_TRUE(rapidJson.IsString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20T10:45:32.111", rapidJson.GetString()) << dt.ToString();

    dt = DateTime(2017, 11, 20);

    ECJsonUtilities::DateTimeToJson(jsonDoc, dt);
    ASSERT_TRUE(jsonDoc.isString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20", jsonDoc.asCString()) << dt.ToString();

    ECJsonUtilities::DateTimeToJson(rapidJson, dt, rapidJson.GetAllocator());
    ASSERT_TRUE(rapidJson.IsString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20", rapidJson.GetString()) << dt.ToString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToBinary)
    {
    BeJsDocument jsonDocVal;
    BeJsValue jsonDoc(jsonDocVal);

    rapidjson::Document rapidJsonVal;
    BeJsValue rapidJson(rapidJsonVal);

    bvector<Byte> expectedByteVector { 11,32, 234, 112, 222, 212, 0, 33, 22, 55 };
    ByteStream expectedByteStream(expectedByteVector.data(), (int) expectedByteVector.size());

    bvector<Byte> actualByteVector;
    ByteStream actualByteStream;

    jsonDoc.SetBinary(expectedByteVector);
    EXPECT_EQ(SUCCESS, jsonDoc.GetBinary(actualByteVector));
    EXPECT_EQ(0, memcmp(expectedByteVector.data(), actualByteVector.data(), expectedByteVector.size()));
    jsonDoc.SetBinary(expectedByteStream.data(), expectedByteStream.size());
    jsonDoc.GetBinary(actualByteStream);
    EXPECT_EQ(0, memcmp(expectedByteStream.GetData(), actualByteStream.GetData(), expectedByteStream.GetSize()));

    actualByteVector.clear();
    actualByteStream.Clear();
    rapidJson.SetBinary(expectedByteVector);
    EXPECT_EQ(SUCCESS, rapidJson.GetBinary(actualByteVector));
    EXPECT_EQ(0, memcmp(expectedByteVector.data(), actualByteVector.data(), expectedByteVector.size()));
    rapidJson.GetBinary(actualByteStream);
    EXPECT_EQ(0, memcmp(expectedByteStream.GetData(), actualByteStream.GetData(), expectedByteStream.GetSize()));
    EXPECT_EQ(jsonDoc, rapidJson);

    jsonDoc = "bad";
    jsonDoc.From(rapidJson);
    EXPECT_EQ(jsonDoc, rapidJson);

    rapidJson = "bad";
    rapidJson.From(jsonDoc);
    EXPECT_EQ(jsonDoc, rapidJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECJsonUtilitiesTestFixture, JsonToPoint2d)
    {
    double x = 3.14;
    double y = -101.0;
    DPoint2d point2d = DPoint2d::From(x, y);

    // JSON Objects
    {
    Json::Value objJsonCpp(Json::ValueType::objectValue);
    objJsonCpp[ECJsonSystemNames::Point::X()] = x;
    objJsonCpp[ECJsonSystemNames::Point::Y()] = y;
    DPoint2d convertedObjJsonCpp;
    ASSERT_EQ(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedObjJsonCpp, objJsonCpp));
    EXPECT_EQ(point2d, convertedObjJsonCpp);

    rapidjson::Document objRapidJson(rapidjson::Type::kObjectType);
    objRapidJson.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::X()), x, objRapidJson.GetAllocator());
    objRapidJson.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::Y()), y, objRapidJson.GetAllocator());
    DPoint2d convertedObjRapidJson;
    ASSERT_EQ(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedObjRapidJson, objRapidJson));
    EXPECT_EQ(point2d, convertedObjRapidJson);
    }

    // JSON Arrays
    {
    Json::Value arrJsonCppValid(Json::ValueType::arrayValue);
    arrJsonCppValid[0u] = x;
    arrJsonCppValid[1u] = y;
    DPoint2d convertedArrJsonCppValid;
    ASSERT_EQ(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedArrJsonCppValid, arrJsonCppValid));
    EXPECT_EQ(point2d, convertedArrJsonCppValid);

    rapidjson::Document arrRapidJsonValid(rapidjson::Type::kArrayType);
    arrRapidJsonValid.PushBack(x, arrRapidJsonValid.GetAllocator());
    arrRapidJsonValid.PushBack(y, arrRapidJsonValid.GetAllocator());
    DPoint2d convertedArrRapidJsonValid;
    ASSERT_EQ(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedArrRapidJsonValid, arrRapidJsonValid));
    EXPECT_EQ(point2d, convertedArrRapidJsonValid);

    // Json arrays with length != 2 should fail.
    Json::Value arrJsonCppLenTooShort(Json::ValueType::arrayValue);
    arrJsonCppLenTooShort[0u] = x;
    DPoint2d convertedArrJsonCppLenTooShort;
    ASSERT_NE(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedArrJsonCppLenTooShort, arrJsonCppLenTooShort));

    rapidjson::Document arrRapidJsonLenTooShort(rapidjson::Type::kArrayType);
    arrRapidJsonLenTooShort.PushBack(x, arrRapidJsonLenTooShort.GetAllocator());
    DPoint2d convertedArrRapidJsonLenTooShort;
    ASSERT_NE(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedArrRapidJsonLenTooShort, arrRapidJsonLenTooShort));

    // Json arrays with length != 2 should fail.
    Json::Value arrJsonCppLenTooLong(Json::ValueType::arrayValue);
    arrJsonCppLenTooLong[0u] = x;
    arrJsonCppLenTooLong[1u] = y;
    arrJsonCppLenTooLong[2u] = (double)0xBAD;
    DPoint2d convertedArrJsonCppLenTooLong;
    ASSERT_NE(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedArrJsonCppLenTooLong, arrJsonCppLenTooLong));

    rapidjson::Document arrRapidJsonLenTooLong(rapidjson::Type::kArrayType);
    arrRapidJsonLenTooLong.PushBack(x, arrRapidJsonLenTooLong.GetAllocator());
    arrRapidJsonLenTooLong.PushBack(y, arrRapidJsonLenTooLong.GetAllocator());
    arrRapidJsonLenTooLong.PushBack((double)0xBAD, arrRapidJsonLenTooLong.GetAllocator());
    DPoint2d convertedArrRapidJsonLenTooLong;
    ASSERT_NE(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedArrRapidJsonLenTooLong, arrRapidJsonLenTooLong));

    // null values should fail.
    Json::Value nullJsonCpp(Json::nullValue);
    DPoint2d convertedNullJsonCpp;
    EXPECT_NE(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedNullJsonCpp, nullJsonCpp));

    rapidjson::Document nullRapidJson(rapidjson::Type::kNullType);
    DPoint2d convertedNullRapidJson;
    ASSERT_NE(SUCCESS, ECJsonUtilities::JsonToPoint2d(convertedNullRapidJson, nullRapidJson));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECJsonUtilitiesTestFixture, JsonToPoint3d)
    {
    double x = 3.14;
    double y = -101.0;
    double z = 0.25;
    DPoint3d point3d = DPoint3d::From(x, y, z);

    // JSON Objects
    {
    Json::Value objJsonCpp(Json::ValueType::objectValue);
    objJsonCpp[ECJsonSystemNames::Point::X()] = x;
    objJsonCpp[ECJsonSystemNames::Point::Y()] = y;
    objJsonCpp[ECJsonSystemNames::Point::Z()] = z;
    DPoint3d convertedObjJsonCpp;
    ASSERT_EQ(SUCCESS, ECJsonUtilities::JsonToPoint3d(convertedObjJsonCpp, objJsonCpp));
    EXPECT_EQ(point3d, convertedObjJsonCpp);

    rapidjson::Document objRapidJson(rapidjson::Type::kObjectType);
    objRapidJson.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::X()), x, objRapidJson.GetAllocator());
    objRapidJson.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::Y()), y, objRapidJson.GetAllocator());
    objRapidJson.AddMember(rapidjson::StringRef(ECJsonSystemNames::Point::Z()), z, objRapidJson.GetAllocator());
    DPoint3d convertedObjRapidJson;
    ASSERT_EQ(SUCCESS, ECJsonUtilities::JsonToPoint3d(convertedObjRapidJson, objRapidJson));
    EXPECT_EQ(point3d, convertedObjRapidJson);
    }

    // JSON Arrays
    {
    Json::Value arrJsonCpp(Json::ValueType::arrayValue);
    arrJsonCpp[0u] = x;
    arrJsonCpp[1u] = y;
    arrJsonCpp[2u] = z;
    DPoint3d convertedArrJsonCpp;
    ASSERT_EQ(SUCCESS, ECJsonUtilities::JsonToPoint3d(convertedArrJsonCpp, arrJsonCpp));
    EXPECT_EQ(point3d, convertedArrJsonCpp);

    // Json arrays with length != 3 should fail.
    Json::Value arrJsonCppLenTooShort(Json::ValueType::arrayValue);
    arrJsonCppLenTooShort[0u] = x;
    arrJsonCppLenTooShort[1u] = y;
    DPoint3d convertedArrJsonCppLenTooShort;
    ASSERT_NE(SUCCESS, ECJsonUtilities::JsonToPoint3d(convertedArrJsonCppLenTooShort, arrJsonCppLenTooShort));

    rapidjson::Document arrRapidJsonValid(rapidjson::Type::kArrayType);
    arrRapidJsonValid.PushBack(x, arrRapidJsonValid.GetAllocator());
    arrRapidJsonValid.PushBack(y, arrRapidJsonValid.GetAllocator());
    arrRapidJsonValid.PushBack(z, arrRapidJsonValid.GetAllocator());
    DPoint3d convertedArrRapidJsonValid;
    ASSERT_EQ(SUCCESS, ECJsonUtilities::JsonToPoint3d(convertedArrRapidJsonValid, arrRapidJsonValid));
    EXPECT_EQ(point3d, convertedArrRapidJsonValid);

    // Json arrays with length != 3 should fail.
    Json::Value arrJsonCppLenTooLong(Json::ValueType::arrayValue);
    arrJsonCppLenTooLong[0u] = x;
    arrJsonCppLenTooLong[1u] = y;
    arrJsonCppLenTooLong[2u] = y;
    arrJsonCppLenTooLong[3u] = (double)0xBAD;
    DPoint3d convertedArrJsonCppLenTooLong;
    ASSERT_NE(SUCCESS, ECJsonUtilities::JsonToPoint3d(convertedArrJsonCppLenTooLong, arrJsonCppLenTooLong));

    rapidjson::Document arrRapidJsonLenTooLong(rapidjson::Type::kArrayType);
    arrRapidJsonLenTooLong.PushBack(x, arrRapidJsonLenTooLong.GetAllocator());
    arrRapidJsonLenTooLong.PushBack(y, arrRapidJsonLenTooLong.GetAllocator());
    arrRapidJsonLenTooLong.PushBack(z, arrRapidJsonLenTooLong.GetAllocator());
    arrRapidJsonLenTooLong.PushBack((double)0xBAD, arrRapidJsonLenTooLong.GetAllocator());
    DPoint3d convertedArrRapidJsonLenTooLong;
    ASSERT_NE(SUCCESS, ECJsonUtilities::JsonToPoint3d(convertedArrRapidJsonLenTooLong, arrRapidJsonLenTooLong));

    // null values should fail.
    Json::Value nullJsonCpp(Json::nullValue);
    DPoint3d convertedNullJsonCpp;
    EXPECT_NE(SUCCESS, ECJsonUtilities::JsonToPoint3d(convertedNullJsonCpp, nullJsonCpp));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECJsonUtilitiesTestFixture, IGeometryIModelJsonRoundTrip)
    {
    {
    Json::Value lineSegmentObj(Json::ValueType::objectValue);
    Json::Value lineSegments(Json::ValueType::arrayValue);
    Json::Value lineSegment(Json::ValueType::arrayValue);
    lineSegment[0u] = -21908.999;
    lineSegment[1u] = 4111.625;
    lineSegment[2u] = 0.0;

    lineSegments[0u] = lineSegment;

    Json::Value lineSegment2(Json::ValueType::arrayValue);
    lineSegment2[0u] = -22956.749;
    lineSegment2[1u] = 4111.625;
    lineSegment2[2u] = 0.0;

    lineSegments[1u] = lineSegment2;

    lineSegmentObj["lineSegment"] = lineSegments;

    IGeometryPtr geom = ECJsonUtilities::JsonToIGeometry(lineSegmentObj);
    ASSERT_TRUE(geom.IsValid()) << "The iModelJson IGeometry format should be properly read by ECJsonUtilities::JsonToIGeometry";
    ICurvePrimitivePtr geomCur = geom->GetAsICurvePrimitive();
    ASSERT_TRUE(geomCur.IsValid());
    ASSERT_EQ(ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line, geomCur->GetCurvePrimitiveType());

    Json::Value retJson;
    ECJsonUtilities::IGeometryToIModelJson(retJson, *geom);
    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(retJson, lineSegmentObj)) << "Expected:\n" + lineSegmentObj.ToString() + "\nBut was:\n" + retJson.ToString();;

    // DgnJs
    Json::Value dgnJs;
    ECJsonUtilities::IGeometryToJson(dgnJs, *geom);

    IGeometryPtr geom2 = ECJsonUtilities::JsonToIGeometry(dgnJs);
    ASSERT_TRUE(geom2.IsValid()) << "The DgnJson format of IGeometry should be able to be read by ECJsonUtilities::JsonToIGeometry";
    ICurvePrimitivePtr geomCur2 = geom2->GetAsICurvePrimitive();
    ASSERT_TRUE(geomCur2.IsValid());
    ASSERT_EQ(ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line, geomCur2->GetCurvePrimitiveType());

    EXPECT_TRUE(geom2->IsSameStructureAndGeometry(*geom)) << "The roundtripped geometry should be the same as the original";
    }

    {// rapid json
    rapidjson::Document lineSegmentObj(rapidjson::Type::kObjectType);
    rapidjson::Document lineSegments(rapidjson::Type::kArrayType);

    rapidjson::Document lineSegment(rapidjson::Type::kArrayType);
    lineSegment.PushBack(-21908.999, lineSegment.GetAllocator());
    lineSegment.PushBack(4111.625, lineSegment.GetAllocator());
    lineSegment.PushBack(0.0, lineSegment.GetAllocator());

    lineSegments.PushBack(lineSegment, lineSegments.GetAllocator());

    rapidjson::Document lineSegment2(rapidjson::Type::kArrayType);
    lineSegment2.PushBack(-22956.749, lineSegment.GetAllocator());
    lineSegment2.PushBack(4111.625, lineSegment.GetAllocator());
    lineSegment2.PushBack(0.0, lineSegment.GetAllocator());

    lineSegments.PushBack(lineSegment2, lineSegments.GetAllocator());

    lineSegmentObj.AddMember(rapidjson::StringRef("lineSegment"), lineSegments, lineSegmentObj.GetAllocator());

    IGeometryPtr geom = ECJsonUtilities::JsonToIGeometry(lineSegmentObj);
    ASSERT_TRUE(geom.IsValid()) << "The iModelJson IGeometry format should be properly read by ECJsonUtilities::JsonToIGeometry";
    ICurvePrimitivePtr geomCur = geom->GetAsICurvePrimitive();
    ASSERT_TRUE(geomCur.IsValid());
    ASSERT_EQ(ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line, geomCur->GetCurvePrimitiveType());

    rapidjson::Document retJson;
    ECJsonUtilities::IGeometryToIModelJson(retJson, *geom.get(), retJson.GetAllocator());

    // DgnJs
    rapidjson::Document dgnJs;
    ECJsonUtilities::IGeometryToJson(dgnJs, *geom.get(), dgnJs.GetAllocator());

    IGeometryPtr geom2 = ECJsonUtilities::JsonToIGeometry(dgnJs);
    ASSERT_TRUE(geom2.IsValid()) << "The DgnJson format of IGeometry should be able to be read by ECJsonUtilities::JsonToIGeometry";
    ICurvePrimitivePtr geomCur2 = geom2->GetAsICurvePrimitive();
    ASSERT_TRUE(geomCur2.IsValid());
    ASSERT_EQ(ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line, geomCur2->GetCurvePrimitiveType());

    EXPECT_TRUE(geom2->IsSameStructureAndGeometry(*geom)) << "The roundtripped geometry should be the same as the original";
    }
    }

//=======================================================================================
// @bsistruct
//=======================================================================================
struct JsonECInstanceConverterTestFixture : ECJsonUtilitiesTestFixture
    {
protected:
    BeJsDocument m_jsDoc;
    rapidjson::Document m_rapidJson;

    static IECInstancePtr CreateTestInstance(ECSchemaR schema, PrimitiveType propertyType)
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProperty;
        schema.CreateEntityClass(testClass, "TestClass");
        testClass->CreatePrimitiveProperty(testProperty, "TestProperty", propertyType);

        auto customAttributeEnabler = testClass->GetDefaultStandaloneEnabler ();
        return customAttributeEnabler->CreateInstance();
        }

    void ParsePropertyJson(Utf8CP value)
        {
        Utf8PrintfString raw(Utf8Chars(u8R"*({ "TestProperty": %s })*"), value);
        ASSERT_EQ(SUCCESS, ParseJson(m_jsDoc, raw));
        ASSERT_EQ(SUCCESS, ParseJson(m_rapidJson, raw));
        }

    void ParseJsonString(Utf8CP jsonString, BentleyStatus expectedStatus = SUCCESS)
        {
        if (SUCCESS == expectedStatus)
            {
            ASSERT_EQ(SUCCESS, ParseJson(m_jsDoc, jsonString));
            ASSERT_EQ(SUCCESS, ParseJson(m_rapidJson, jsonString));
            }
        else
            {
            ASSERT_NE(SUCCESS, ParseJson(m_jsDoc, jsonString));
            ASSERT_NE(SUCCESS, ParseJson(m_rapidJson, jsonString));
            }
        }

    void AssertDoubleValue(IECInstanceCR instance, Utf8CP accessString, double d)
        {
        ECValue ecValue;
        ASSERT_EQ(ECObjectsStatus::Success, instance.GetValue(ecValue, accessString));
        ASSERT_EQ(ecValue.GetDouble(), d);
        }

    void AssertStringValue(IECInstanceCR instance, Utf8CP accessString, Utf8CP s)
        {
        ECValue ecValue;
        ASSERT_EQ(ECObjectsStatus::Success, instance.GetValue(ecValue, accessString));
        ASSERT_STREQ(ecValue.GetUtf8CP(), s);
        }

    void AssertNullValue(IECInstanceCR instance, Utf8CP accessString)
        {
        ECValue ecValue;
        ASSERT_EQ(ECObjectsStatus::Success, instance.GetValue(ecValue, accessString));
        ASSERT_TRUE(ecValue.IsNull());
        }

    void AssertEmptyArray(IECInstanceCR instance, Utf8CP accessString)
        {
        ECValue ecValue;
        ASSERT_EQ(ECObjectsStatus::Success, instance.GetValue(ecValue, accessString));
        ASSERT_TRUE(ecValue.IsArray());
        ASSERT_EQ(0, ecValue.GetArrayInfo().GetCount());
        }
    };

struct InSchemaClassLocater final : ECN::IECClassLocater
    {
    private:
        ECN::ECSchemaCR m_schema;

        ECN::ECClassCP _LocateClass(Utf8CP schemaName, Utf8CP className) override { return m_schema.GetClassCP(className); }
    public:
        explicit InSchemaClassLocater(ECN::ECSchemaCR schema) : m_schema(schema) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(JsonECInstanceConverterTestFixture, JsonToECInstance_DoubleProperty)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Schema", "sch", 1, 0, 0);
    InSchemaClassLocater classLocater(*schema);
    auto instance = CreateTestInstance(*schema, PRIMITIVETYPE_Double);

    auto GetPropertyValue = [](IECInstanceCR instance)
        {
        ECValue val;
        instance.GetValue(val, 1);
        return val.GetDouble();
        };

    ParsePropertyJson("100");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
    EXPECT_EQ(100, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(100, GetPropertyValue(*instance));

    ParsePropertyJson("1.111111");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
    EXPECT_EQ(1.111111, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.111111, GetPropertyValue(*instance));

    ParsePropertyJson("1000000000000000");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
    EXPECT_EQ(1.0e15, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e15, GetPropertyValue(*instance));

    ParsePropertyJson("1000000000000");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
    EXPECT_EQ(1.0e12, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e12, GetPropertyValue(*instance));

    ParsePropertyJson("1000000000");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
    EXPECT_EQ(1.0e9, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e9, GetPropertyValue(*instance));

    ParsePropertyJson("1.0e18");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
    EXPECT_EQ(1.0e18, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e18, GetPropertyValue(*instance));

    ParsePropertyJson("1.0e-24");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
    EXPECT_EQ(1.0e-24, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e-24, GetPropertyValue(*instance));

#ifdef NOT_NOW
    ParsePropertyJson("\"ABCDE\"");
        {
        DISABLE_ASSERTS
        EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
        EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
        }
#endif

    ParsePropertyJson("\"1.111111\"");
        {
        DISABLE_ASSERTS
        EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsDoc, classLocater));
        EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(JsonECInstanceConverterTestFixture, JsonToECInstance_Struct)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Schema", "sch", 1, 0, 0);
    InSchemaClassLocater classLocater(*schema);

    // construct struct instance
    ECStructClassP structClass;
        { 
        PrimitiveECPropertyP doubleProperty;
        PrimitiveECPropertyP stringProperty;
        schema->CreateStructClass(structClass, "TestStruct");
        structClass->CreatePrimitiveProperty(doubleProperty, "DoubleProperty", PRIMITIVETYPE_Double);
        structClass->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String);
        }
    IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(structInstance.IsValid());
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsDoc, classLocater)); // expect error when not initialized
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater)); // expect error when not initialized

    // construct entity instance with a struct property
    ECEntityClassP testClass;
        {
        StructECPropertyP structProperty;
        PrimitiveECPropertyP doubleProperty;
        PrimitiveECPropertyP stringProperty;
        schema->CreateEntityClass(testClass, "TestClass");
        testClass->CreateStructProperty(structProperty, "TestStruct", *structClass);
        testClass->CreatePrimitiveProperty(doubleProperty, "DoubleProperty", PRIMITIVETYPE_Double);
        testClass->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String);
        }
    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(testInstance.IsValid());
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater)); // expect error when not initialized
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater)); // expect error when not initialized

    // Test for expected JSON parse errors
    ParseJsonString(nullptr, ERROR);
    ParseJsonString("", ERROR);
    ParseJsonString("undefined", ERROR);

    //-------------------------------------------------------------------------
    // JSON --> struct instance tests
    //-------------------------------------------------------------------------

    ParseJsonString("null");
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsDoc, classLocater));
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));

    ParseJsonString("{}");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsDoc, classLocater));
    AssertNullValue(*structInstance, "doubleProperty");
    AssertNullValue(*structInstance, "stringProperty");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));
    AssertNullValue(*structInstance, "doubleProperty");
    AssertNullValue(*structInstance, "stringProperty");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": 1.1, "stringProperty": "S1" })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsDoc, classLocater));
    AssertDoubleValue(*structInstance, "doubleProperty", 1.1);
    AssertStringValue(*structInstance, "stringProperty", "S1");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*structInstance, "doubleProperty", 1.1);
    AssertStringValue(*structInstance, "stringProperty", "S1");

    //-------------------------------------------------------------------------
    // JSON --> entity instance with struct property tests
    //-------------------------------------------------------------------------

    ParseJsonString("null");
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));

    ParseJsonString("{}");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertNullValue(*testInstance, "doubleProperty");
    AssertNullValue(*testInstance, "stringProperty");
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertNullValue(*testInstance, "doubleProperty");
    AssertNullValue(*testInstance, "stringProperty");
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": null, "stringProperty": null })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertNullValue(*testInstance, "doubleProperty");
    AssertNullValue(*testInstance, "stringProperty");
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertNullValue(*testInstance, "doubleProperty");
    AssertNullValue(*testInstance, "stringProperty");
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": 1.1, "stringProperty": "S1" })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");
    AssertDoubleValue(*testInstance, "doubleProperty", 1.1);
    AssertStringValue(*testInstance, "stringProperty", "S1");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");
    AssertDoubleValue(*testInstance, "doubleProperty", 1.1);
    AssertStringValue(*testInstance, "stringProperty", "S1");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": 1.1, "stringProperty": "S1", "testStruct": null })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");
    AssertDoubleValue(*testInstance, "doubleProperty", 1.1);
    AssertStringValue(*testInstance, "stringProperty", "S1");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");
    AssertDoubleValue(*testInstance, "doubleProperty", 1.1);
    AssertStringValue(*testInstance, "stringProperty", "S1");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": 1.1, "stringProperty": "S1", "testStruct": { "doubleProperty": 2.2, "stringProperty": "S2" } })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 1.1);
    AssertStringValue(*testInstance, "stringProperty", "S1");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 2.2);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "S2");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 1.1);
    AssertStringValue(*testInstance, "stringProperty", "S1");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 2.2);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "S2");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(JsonECInstanceConverterTestFixture, JsonToECInstance_Struct_InitializedInstance)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Schema", "sch", 1, 0, 0);
    InSchemaClassLocater classLocater(*schema);

    // construct struct instance
    ECStructClassP structClass;
        { 
        PrimitiveECPropertyP doubleProperty;
        PrimitiveECPropertyP stringProperty;
        schema->CreateStructClass(structClass, "TestStruct");
        structClass->CreatePrimitiveProperty(doubleProperty, "DoubleProperty", PRIMITIVETYPE_Double);
        structClass->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String);
        }
    IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(structInstance.IsValid());
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsDoc, classLocater)); // expect error when not initialized
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater)); // expect error when not initialized

    // Initialize the struct instance with initial data
    ECValue ecValue;
    ecValue.SetDouble(22.2);
    (*structInstance).SetInternalValue("DoubleProperty", ecValue);
    ecValue.SetUtf8CP("TS.S1-Init");
    (*structInstance).SetInternalValue("StringProperty", ecValue);

    // construct entity instance with a struct property
    ECEntityClassP testClass;
        {
        StructECPropertyP structProperty;
        PrimitiveECPropertyP doubleProperty;
        PrimitiveECPropertyP stringProperty;
        schema->CreateEntityClass(testClass, "TestClass");
        testClass->CreateStructProperty(structProperty, "TestStruct", *structClass);
        testClass->CreatePrimitiveProperty(doubleProperty, "DoubleProperty", PRIMITIVETYPE_Double);
        testClass->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String);
        }
    
    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(testInstance.IsValid());
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater)); // expect error when not initialized
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater)); // expect error when not initialized

    // Initialize the test instance with initial data
    ecValue.SetDouble(0.6);
    (*testInstance).SetInternalValue("DoubleProperty", ecValue);
    ecValue.SetUtf8CP("S1-Init");
    (*testInstance).SetInternalValue("StringProperty", ecValue);
    ecValue.SetDouble(22.2);
    (*testInstance).SetInternalValue("TestStruct.DoubleProperty", ecValue);
    ecValue.SetUtf8CP("TS.S1-Init");
    (*testInstance).SetInternalValue("TestStruct.StringProperty", ecValue);

    // Test for expected JSON parse errors
    ParseJsonString(nullptr, ERROR);
    ParseJsonString("", ERROR);
    ParseJsonString("undefined", ERROR);

    //-------------------------------------------------------------------------
    // JSON --> struct instance tests
    //-------------------------------------------------------------------------

    ParseJsonString("null");
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsDoc, classLocater));
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));

    ParseJsonString("{}"); // properties should remain unchanged
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsDoc, classLocater));
    AssertDoubleValue(*structInstance, "doubleProperty", 22.2); 
    AssertStringValue(*structInstance, "stringProperty", "TS.S1-Init");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*structInstance, "doubleProperty", 22.2); 
    AssertStringValue(*structInstance, "stringProperty", "TS.S1-Init");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": 22.3, "stringProperty": "TS.S1-New" })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsDoc, classLocater));
    AssertDoubleValue(*structInstance, "doubleProperty", 22.3);
    AssertStringValue(*structInstance, "stringProperty", "TS.S1-New");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*structInstance, "doubleProperty", 22.3);
    AssertStringValue(*structInstance, "stringProperty", "TS.S1-New");

    //-------------------------------------------------------------------------
    // JSON --> entity instance with struct property tests
    //-------------------------------------------------------------------------

    ParseJsonString("null");
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));

    ParseJsonString("{}");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 0.6); 
    AssertStringValue(*testInstance, "stringProperty", "S1-Init");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 22.2);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "TS.S1-Init");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 0.6); 
    AssertStringValue(*testInstance, "stringProperty", "S1-Init");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 22.2);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "TS.S1-Init");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": null, "stringProperty": null })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertNullValue(*testInstance, "doubleProperty");
    AssertNullValue(*testInstance, "stringProperty");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 22.2);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "TS.S1-Init");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertNullValue(*testInstance, "doubleProperty");
    AssertNullValue(*testInstance, "stringProperty");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 22.2);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "TS.S1-Init");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": 4.2, "stringProperty": "S1-new-new" })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 4.2);
    AssertStringValue(*testInstance, "stringProperty", "S1-new-new");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 22.2);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "TS.S1-Init");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 4.2);
    AssertStringValue(*testInstance, "stringProperty", "S1-new-new");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 22.2);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "TS.S1-Init");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": 5.3, "stringProperty": "S1-new-new-new", "testStruct": null })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 5.3);
    AssertStringValue(*testInstance, "stringProperty", "S1-new-new-new");
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 5.3);
    AssertStringValue(*testInstance, "stringProperty", "S1-new-new-new");
    AssertNullValue(*testInstance, "testStruct");
    AssertNullValue(*testInstance, "testStruct.doubleProperty");
    AssertNullValue(*testInstance, "testStruct.stringProperty");

    ParseJsonString(Utf8Chars(u8R"*({ "doubleProperty": 111.1, "stringProperty": "S1", "testStruct": { "doubleProperty": 4.4, "stringProperty": "S2" } })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 111.1);
    AssertStringValue(*testInstance, "stringProperty", "S1");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 4.4);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "S2");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*testInstance, "doubleProperty", 111.1);
    AssertStringValue(*testInstance, "stringProperty", "S1");
    AssertDoubleValue(*testInstance, "testStruct.doubleProperty", 4.4);
    AssertStringValue(*testInstance, "testStruct.stringProperty", "S2");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(JsonECInstanceConverterTestFixture, JsonToECInstance_Array)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Schema", "sch", 1, 0, 0);
    InSchemaClassLocater classLocater(*schema);

    // construct struct instance
    ECStructClassP structClass;
        { 
        PrimitiveECPropertyP doubleProperty;
        PrimitiveECPropertyP stringProperty;
        schema->CreateStructClass(structClass, "TestStruct");
        structClass->CreatePrimitiveProperty(doubleProperty, "DoubleProperty", PRIMITIVETYPE_Double);
        structClass->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String);
        }

    ECEntityClassP testClass;
    PrimitiveArrayECPropertyP testIntegerArrayProperty;
    StructArrayECPropertyP testStructArrayProperty;
    schema->CreateEntityClass(testClass, "TestClass");
    testClass->CreatePrimitiveArrayProperty(testIntegerArrayProperty, "IntArrayProp", PRIMITIVETYPE_Integer);
    testClass->CreateStructArrayProperty(testStructArrayProperty, "StructArrayProp", *structClass);

    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(testInstance.IsValid());
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater)); // expect error when not initialized
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater)); // expect error when not initialized
    
    // Test for expected JSON parse errors
    ParseJsonString(nullptr, ERROR);
    ParseJsonString("", ERROR);
    ParseJsonString("undefined", ERROR);

    //-------------------------------------------------------------------------
    // JSON --> entity instance with array property tests
    //-------------------------------------------------------------------------

    ParseJsonString("null");
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));

    ParseJsonString("{}");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");

    ParseJsonString(Utf8Chars(u8R"*({ "IntArrayProp": null, "StructArrayProp": null })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");

    ParseJsonString(Utf8Chars(u8R"*({ "IntArrayProp": [], "StructArrayProp": [] })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");

    ParseJsonString(Utf8Chars(u8R"*({ "IntArrayProp": [4, 5, 6, 9, 12], "StructArrayProp": [{ "DoubleProperty": 3.41, "StringProperty": "NewVal1" }, { "DoubleProperty": 12.92, "StringProperty": null }] })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    {
    ECValue intArrayProp;
    ECValue structArrayProp;
    IECInstancePtr structInst;
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp"));
    ASSERT_EQ(intArrayProp.GetArrayInfo().GetCount(), 5);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 0));
    ASSERT_EQ(intArrayProp.GetInteger(), 4);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 1));
    ASSERT_EQ(intArrayProp.GetInteger(), 5);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 2));
    ASSERT_EQ(intArrayProp.GetInteger(), 6);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 3));
    ASSERT_EQ(intArrayProp.GetInteger(), 9);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 4));
    ASSERT_EQ(intArrayProp.GetInteger(), 12);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp"));
    ASSERT_EQ(structArrayProp.GetArrayInfo().GetCount(), 2);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 0));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 3.41);
    AssertStringValue(*structInst, "StringProperty", "NewVal1");
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 1));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 12.92);
    AssertNullValue(*structInst, "StringProperty");
    }
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    {
    ECValue intArrayProp;
    ECValue structArrayProp;
    IECInstancePtr structInst;
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp"));
    ASSERT_EQ(intArrayProp.GetArrayInfo().GetCount(), 5);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 0));
    ASSERT_EQ(intArrayProp.GetInteger(), 4);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 1));
    ASSERT_EQ(intArrayProp.GetInteger(), 5);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 2));
    ASSERT_EQ(intArrayProp.GetInteger(), 6);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 3));
    ASSERT_EQ(intArrayProp.GetInteger(), 9);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 4));
    ASSERT_EQ(intArrayProp.GetInteger(), 12);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp"));
    ASSERT_EQ(structArrayProp.GetArrayInfo().GetCount(), 2);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 0));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 3.41);
    AssertStringValue(*structInst, "StringProperty", "NewVal1");
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 1));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 12.92);
    AssertNullValue(*structInst, "StringProperty");
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(JsonECInstanceConverterTestFixture, JsonToECInstance_Array_InitializedInstance)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Schema", "sch", 1, 0, 0);
    InSchemaClassLocater classLocater(*schema);

    // construct struct instance
    ECStructClassP structClass;
        { 
        PrimitiveECPropertyP doubleProperty;
        PrimitiveECPropertyP stringProperty;
        schema->CreateStructClass(structClass, "TestStruct");
        structClass->CreatePrimitiveProperty(doubleProperty, "DoubleProperty", PRIMITIVETYPE_Double);
        structClass->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String);
        }

    ECEntityClassP testClass;
    PrimitiveArrayECPropertyP testIntegerArrayProperty;
    StructArrayECPropertyP testStructArrayProperty;
    schema->CreateEntityClass(testClass, "TestClass");
    testClass->CreatePrimitiveArrayProperty(testIntegerArrayProperty, "IntArrayProp", PRIMITIVETYPE_Integer);
    testClass->CreateStructArrayProperty(testStructArrayProperty, "StructArrayProp", *structClass);
    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(testInstance.IsValid());
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater)); // expect error when not initialized
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater)); // expect error when not initialized

    // Initialize the test instance with initial data
    ECValue ecValue;
    ecValue.SetInteger(9);
    (*testInstance).AddArrayElements("IntArrayProp", 3);
    (*testInstance).SetInternalValue("IntArrayProp", ecValue, 0);
    ecValue.SetInteger(16);
    (*testInstance).SetInternalValue("IntArrayProp", ecValue, 1);
    ecValue.SetInteger(25);
    (*testInstance).SetInternalValue("IntArrayProp", ecValue, 2);
    
    (*testInstance).AddArrayElements("StructArrayProp", 1);
    IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(structInstance.IsValid());
    ecValue.SetDouble(22.2);
    (*structInstance).SetInternalValue("DoubleProperty", ecValue);
    ecValue.SetUtf8CP("TS1-Init");
    (*structInstance).SetInternalValue("StringProperty", ecValue);
    ecValue.SetStruct(structInstance.get());
    (*testInstance).SetInternalValue("StructArrayProp", ecValue, 0);
    
    // Test for expected JSON parse errors
    ParseJsonString(nullptr, ERROR);
    ParseJsonString("", ERROR);
    ParseJsonString("undefined", ERROR);

    //-------------------------------------------------------------------------
    // JSON --> entity instance with array property tests
    //-------------------------------------------------------------------------

    ParseJsonString("null");
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));

    ParseJsonString("{}");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    {
    ECValue intArrayProp;
    ECValue structArrayProp;
    IECInstancePtr structInst;
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp"));
    ASSERT_EQ(intArrayProp.GetArrayInfo().GetCount(), 3);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 0));
    ASSERT_EQ(intArrayProp.GetInteger(), 9);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 1));
    ASSERT_EQ(intArrayProp.GetInteger(), 16);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 2));
    ASSERT_EQ(intArrayProp.GetInteger(), 25);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp"));
    ASSERT_EQ(structArrayProp.GetArrayInfo().GetCount(), 1);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 0));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 22.2);
    AssertStringValue(*structInst, "StringProperty", "TS1-Init");
    }
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    {
    ECValue intArrayProp;
    ECValue structArrayProp;
    IECInstancePtr structInst;
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp"));
    ASSERT_EQ(intArrayProp.GetArrayInfo().GetCount(), 3);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 0));
    ASSERT_EQ(intArrayProp.GetInteger(), 9);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 1));
    ASSERT_EQ(intArrayProp.GetInteger(), 16);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 2));
    ASSERT_EQ(intArrayProp.GetInteger(), 25);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp"));
    ASSERT_EQ(structArrayProp.GetArrayInfo().GetCount(), 1);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 0));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 22.2);
    AssertStringValue(*structInst, "StringProperty", "TS1-Init");
    }

    ParseJsonString(Utf8Chars(u8R"*({ "IntArrayProp": null, "StructArrayProp": null })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");
    
    ParseJsonString(Utf8Chars(u8R"*({ "IntArrayProp": [4, 5, 6, 9, 12], "StructArrayProp": [{ "DoubleProperty": 3.41, "StringProperty": "NewVal1" }, { "DoubleProperty": 12.92, "StringProperty": null }] })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    {
    ECValue intArrayProp;
    ECValue structArrayProp;
    IECInstancePtr structInst;
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp"));
    ASSERT_EQ(intArrayProp.GetArrayInfo().GetCount(), 5);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 0));
    ASSERT_EQ(intArrayProp.GetInteger(), 4);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 1));
    ASSERT_EQ(intArrayProp.GetInteger(), 5);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 2));
    ASSERT_EQ(intArrayProp.GetInteger(), 6);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 3));
    ASSERT_EQ(intArrayProp.GetInteger(), 9);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 4));
    ASSERT_EQ(intArrayProp.GetInteger(), 12);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp"));
    ASSERT_EQ(structArrayProp.GetArrayInfo().GetCount(), 2);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 0));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 3.41);
    AssertStringValue(*structInst, "StringProperty", "NewVal1");
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 1));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 12.92);
    AssertNullValue(*structInst, "StringProperty");
    }
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    {
    ECValue intArrayProp;
    ECValue structArrayProp;
    IECInstancePtr structInst;
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp"));
    ASSERT_EQ(intArrayProp.GetArrayInfo().GetCount(), 5);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 0));
    ASSERT_EQ(intArrayProp.GetInteger(), 4);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 1));
    ASSERT_EQ(intArrayProp.GetInteger(), 5);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 2));
    ASSERT_EQ(intArrayProp.GetInteger(), 6);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 3));
    ASSERT_EQ(intArrayProp.GetInteger(), 9);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(intArrayProp, "IntArrayProp", 4));
    ASSERT_EQ(intArrayProp.GetInteger(), 12);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp"));
    ASSERT_EQ(structArrayProp.GetArrayInfo().GetCount(), 2);
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 0));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 3.41);
    AssertStringValue(*structInst, "StringProperty", "NewVal1");
    ASSERT_EQ(ECObjectsStatus::Success, (*testInstance).GetValue(structArrayProp, "StructArrayProp", 1));
    structInst = structArrayProp.GetStruct();
    AssertDoubleValue(*structInst, "DoubleProperty", 12.92);
    AssertNullValue(*structInst, "StringProperty");
    }

    ParseJsonString(Utf8Chars(u8R"*({ "IntArrayProp": [], "StructArrayProp": [] })*"));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsDoc, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));
    AssertEmptyArray(*testInstance, "IntArrayProp");
    AssertEmptyArray(*testInstance, "StructArrayProp");
    }

END_BENTLEY_ECN_TEST_NAMESPACE