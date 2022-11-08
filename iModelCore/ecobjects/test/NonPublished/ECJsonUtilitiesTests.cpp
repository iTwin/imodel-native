/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <Bentley/BeId.h>
#include <ostream>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECJsonUtilitiesTestFixture : ECTestFixture
    {

protected:
    static BentleyStatus ParseJson(BeJsDocument& json, Utf8StringCR jsonStr) { json.Parse(jsonStr); return json.hasParseError() ? ERROR : SUCCESS; }
    static BentleyStatus ParseJson(rapidjson::Document& json, Utf8StringCR jsonStr) { return json.Parse<0>(jsonStr.c_str()).HasParseError() ? ERROR : SUCCESS; }
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
    BeJsDocument jsonCpp;
    rapidjson::Document rapidJson;

    //Ids formatted numerically in JSON
    Utf8CP jsonStr = "1234";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = "-10"; // Not a valid value for an Id, but failing the method is not worth the overhead. So the test documents the behavior of the API
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(uint64_t(-10)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(uint64_t(-10)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = "3.14";  // Not a valid value for an Id, but failing the method is not worth the overhead. So the test documents the behavior of the API
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(3), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr << " floating numbers are rounded";

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(3), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr << " floating numbers are rounded";

    //Ids formatted as decimal strings in JSON
    jsonStr = R"json("1234")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("1099511627775")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1099511627775)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1099511627775)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("-10")json";  // Not a valid value for an Id, but failing the method is not worth the overhead. So the test documents the behavior of the API
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(uint64_t(-10)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(uint64_t(-10)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("3.14")json";  // Not a valid value for an Id, but failing the method is not worth the overhead. So the test documents the behavior of the API
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(3), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr << " floating numbers are rounded";

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(3), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr << " floating numbers are rounded";

    //Ids formatted as hexadecimal strings in JSON
    jsonStr = R"json("0x123")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(0x123)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(0x123)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("0xFFFFFFFFFF")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(0xFFFFFFFFFF)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(0xFFFFFFFFFF)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, IdToJson)
    {
    BeJsDocument jsonCpp;
    rapidjson::Document rapidJson;

    BeInt64Id id(UINT64_C(1234));

    ASSERT_EQ(SUCCESS, ECJsonUtilities::IdToJson(jsonCpp, id)) << id.ToString();
    ASSERT_TRUE(jsonCpp.isString()) << id.ToString();
    ASSERT_STRCASEEQ("0x4d2", jsonCpp.asCString()) << id.ToString();

    ASSERT_EQ(SUCCESS, ECJsonUtilities::IdToJson(rapidJson, id, rapidJson.GetAllocator())) << id.ToString();
    ASSERT_TRUE(rapidJson.IsString()) << id.ToString();
    ASSERT_STRCASEEQ("0x4d2", rapidJson.GetString()) << id.ToString();

    id = BeInt64Id(UINT64_C(1099511627775));

    ASSERT_EQ(SUCCESS, ECJsonUtilities::IdToJson(jsonCpp, id)) << id.ToString();
    ASSERT_TRUE(jsonCpp.isString()) << id.ToString();
    ASSERT_STRCASEEQ("0xffffffffff", jsonCpp.asCString()) << id.ToString();

    ASSERT_EQ(SUCCESS, ECJsonUtilities::IdToJson(rapidJson, id, rapidJson.GetAllocator())) << id.ToString();
    ASSERT_TRUE(rapidJson.IsString()) << id.ToString();
    ASSERT_STRCASEEQ("0xffffffffff", rapidJson.GetString()) << id.ToString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToInt64)
    {
    BeJsDocument jsonCpp;
    rapidjson::Document rapidJson;

    Utf8CP jsonStr = nullptr;
    int64_t val;

    //formatted numerically in JSON
    jsonStr = "1234";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr;
    EXPECT_EQ(INT64_C(1234), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(1234), val) << jsonStr;

    jsonStr = "-10";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr;
    EXPECT_EQ(INT64_C(-10), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(-10), val) << jsonStr;

    jsonStr = "3.14";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr << " floating numbers are rounded";
    EXPECT_EQ(INT64_C(3), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr << " floating numbers are rounded";
    EXPECT_EQ(INT64_C(3), val) << jsonStr;

    //formatted as decimal strings in JSON
    jsonStr = R"json("1234")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr;
    EXPECT_EQ(INT64_C(1234), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(1234), val) << jsonStr;

    jsonStr = R"json("1099511627775")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr;
    EXPECT_EQ(INT64_C(1099511627775), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(1099511627775), val) << jsonStr;

    jsonStr = R"json("-10")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr;
    EXPECT_EQ(INT64_C(-10), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(-10), val) << jsonStr;

    jsonStr = R"json("3.14")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr << " floating numbers are rounded";
    EXPECT_EQ(INT64_C(3), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr << " floating numbers are rounded";
    EXPECT_EQ(INT64_C(3), val) << jsonStr;

    //Ids formatted as hexadecimal strings in JSON
    jsonStr = R"json("0x123")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr;
    EXPECT_EQ(INT64_C(0x123), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(0x123), val) << jsonStr;

    jsonStr = R"json("0xFFFFFFFFFF")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr;
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
    Json::Value jsonCpp;
    rapidjson::Document rapidJson;

    int64_t val = INT64_C(1234);

    ECJsonUtilities::Int64ToJson(jsonCpp, val, ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(jsonCpp.isIntegral()) << val;
    ASSERT_EQ(val, jsonCpp.asInt64()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(rapidJson.IsInt64()) << val;
    ASSERT_EQ(val, rapidJson.GetInt64()) << val;


    ECJsonUtilities::Int64ToJson(jsonCpp, val, ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(jsonCpp.isString()) << val;
    ASSERT_STRCASEEQ("1234", jsonCpp.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("1234", rapidJson.GetString()) << val;

    ECJsonUtilities::Int64ToJson(jsonCpp, val, ECJsonInt64Format::AsHexadecimalString);
    ASSERT_TRUE(jsonCpp.isString()) << val;
    ASSERT_STRCASEEQ("0x4d2", jsonCpp.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsHexadecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("0x4d2", rapidJson.GetString()) << val;

    val = INT64_C(0xffffffffff);

    ECJsonUtilities::Int64ToJson(jsonCpp, val, ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(jsonCpp.isIntegral()) << val;
    ASSERT_EQ(val, jsonCpp.asInt64()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(rapidJson.IsInt64()) << val;
    ASSERT_EQ(val, rapidJson.GetInt64()) << val;


    ECJsonUtilities::Int64ToJson(jsonCpp, val, ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(jsonCpp.isString()) << val;
    ASSERT_STRCASEEQ("1099511627775", jsonCpp.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("1099511627775", rapidJson.GetString()) << val;

    ECJsonUtilities::Int64ToJson(jsonCpp, val, ECJsonInt64Format::AsHexadecimalString);
    ASSERT_TRUE(jsonCpp.isString()) << val;
    ASSERT_STRCASEEQ("0xffffffffff", jsonCpp.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsHexadecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("0xffffffffff", rapidJson.GetString()) << val;


    val = INT64_C(-10);

    ECJsonUtilities::Int64ToJson(jsonCpp, val, ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(jsonCpp.isIntegral()) << val;
    ASSERT_EQ(val, jsonCpp.asInt64()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsNumber);
    ASSERT_TRUE(rapidJson.IsInt64()) << val;
    ASSERT_EQ(val, rapidJson.GetInt64()) << val;

    ECJsonUtilities::Int64ToJson(jsonCpp, val, ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(jsonCpp.isString()) << val;
    ASSERT_STRCASEEQ("-10", jsonCpp.asCString()) << val;

    ECJsonUtilities::Int64ToJson(rapidJson, val, rapidJson.GetAllocator(), ECJsonInt64Format::AsDecimalString);
    ASSERT_TRUE(rapidJson.IsString()) << val;
    ASSERT_STRCASEEQ("-10", rapidJson.GetString()) << val;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToDateTime)
    {
    BeJsDocument jsonCpp;
    rapidjson::Document rapidJson;

    Utf8CP jsonStr = nullptr;
    DateTime dt;

    jsonStr = R"json("2017-11-20T10:45:32.111Z")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonCpp)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    jsonStr = R"json("2017-11-20 10:45:32.111Z")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonCpp)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    jsonStr = R"json("2017-11-20T10:45:32.111")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonCpp)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    jsonStr = R"json("2017-11-20 10:45:32.111")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonCpp)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, rapidJson)) << jsonStr;
    EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111), dt) << jsonStr;

    jsonStr = R"json("2017-11-20")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToDateTime(dt, jsonCpp)) << jsonStr;
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
    BeJsDocument jsonCpp;
    rapidjson::Document rapidJson;

    DateTime dt(DateTime::Kind::Utc, 2017, 11, 20, 10, 45, 32, 111);

    ECJsonUtilities::DateTimeToJson(jsonCpp, dt);
    ASSERT_TRUE(jsonCpp.isString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20T10:45:32.111Z", jsonCpp.asCString()) << dt.ToString();

    ECJsonUtilities::DateTimeToJson(rapidJson, dt, rapidJson.GetAllocator());
    ASSERT_TRUE(rapidJson.IsString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20T10:45:32.111Z", rapidJson.GetString()) << dt.ToString();

    dt = DateTime(DateTime::Kind::Unspecified, 2017, 11, 20, 10, 45, 32, 111);

    ECJsonUtilities::DateTimeToJson(jsonCpp, dt);
    ASSERT_TRUE(jsonCpp.isString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20T10:45:32.111", jsonCpp.asCString()) << dt.ToString();

    ECJsonUtilities::DateTimeToJson(rapidJson, dt, rapidJson.GetAllocator());
    ASSERT_TRUE(rapidJson.IsString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20T10:45:32.111", rapidJson.GetString()) << dt.ToString();

    dt = DateTime(2017, 11, 20);

    ECJsonUtilities::DateTimeToJson(jsonCpp, dt);
    ASSERT_TRUE(jsonCpp.isString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20", jsonCpp.asCString()) << dt.ToString();

    ECJsonUtilities::DateTimeToJson(rapidJson, dt, rapidJson.GetAllocator());
    ASSERT_TRUE(rapidJson.IsString()) << dt.ToString();
    ASSERT_STRCASEEQ("2017-11-20", rapidJson.GetString()) << dt.ToString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToBinary)
    {
    BeJsDocument jsonCppVal;
    BeJsValue jsonCpp(jsonCppVal);

    rapidjson::Document rapidJsonVal;
    BeJsValue rapidJson(rapidJsonVal);

    bvector<Byte> expectedByteVector { 11,32, 234, 112, 222, 212, 0, 33, 22, 55 };
    ByteStream expectedByteStream(expectedByteVector.data(), (int) expectedByteVector.size());

    bvector<Byte> actualByteVector;
    ByteStream actualByteStream;

    jsonCpp.SetBinary(expectedByteVector);
    EXPECT_EQ(SUCCESS, jsonCpp.GetBinary(actualByteVector));
    EXPECT_EQ(0, memcmp(expectedByteVector.data(), actualByteVector.data(), expectedByteVector.size()));
    jsonCpp.SetBinary(expectedByteStream.data(), expectedByteStream.size());
    jsonCpp.GetBinary(actualByteStream);
    EXPECT_EQ(0, memcmp(expectedByteStream.GetData(), actualByteStream.GetData(), expectedByteStream.GetSize()));

    actualByteVector.clear();
    actualByteStream.Clear();
    rapidJson.SetBinary(expectedByteVector);
    EXPECT_EQ(SUCCESS, rapidJson.GetBinary(actualByteVector));
    EXPECT_EQ(0, memcmp(expectedByteVector.data(), actualByteVector.data(), expectedByteVector.size()));
    rapidJson.GetBinary(actualByteStream);
    EXPECT_EQ(0, memcmp(expectedByteStream.GetData(), actualByteStream.GetData(), expectedByteStream.GetSize()));
    EXPECT_EQ(jsonCpp, rapidJson);

    jsonCpp = "bad";
    jsonCpp.From(rapidJson);
    EXPECT_EQ(jsonCpp, rapidJson);

    rapidJson = "bad";
    rapidJson.From(jsonCpp);
    EXPECT_EQ(jsonCpp, rapidJson);
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
    BeJsDocument m_jsonCpp;
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
        Utf8PrintfString raw(u8R"*({ "TestProperty": %s })*", value);
        ASSERT_EQ(SUCCESS, ParseJson(m_jsonCpp, raw));
        ASSERT_EQ(SUCCESS, ParseJson(m_rapidJson, raw));
        }

    void ParseJsonString(Utf8CP jsonString, BentleyStatus expectedStatus = SUCCESS)
        {
        if (SUCCESS == expectedStatus)
            {
            ASSERT_EQ(SUCCESS, ParseJson(m_jsonCpp, jsonString));
            ASSERT_EQ(SUCCESS, ParseJson(m_rapidJson, jsonString));
            }
        else
            {
            ASSERT_NE(SUCCESS, ParseJson(m_jsonCpp, jsonString));
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
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
    EXPECT_EQ(100, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(100, GetPropertyValue(*instance));

    ParsePropertyJson("1.111111");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
    EXPECT_EQ(1.111111, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.111111, GetPropertyValue(*instance));

    ParsePropertyJson("1000000000000000");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
    EXPECT_EQ(1.0e15, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e15, GetPropertyValue(*instance));

    ParsePropertyJson("1000000000000");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
    EXPECT_EQ(1.0e12, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e12, GetPropertyValue(*instance));

    ParsePropertyJson("1000000000");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
    EXPECT_EQ(1.0e9, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e9, GetPropertyValue(*instance));

    ParsePropertyJson("1.0e18");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
    EXPECT_EQ(1.0e18, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e18, GetPropertyValue(*instance));

    ParsePropertyJson("1.0e-24");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
    EXPECT_EQ(1.0e-24, GetPropertyValue(*instance));
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
    EXPECT_EQ(1.0e-24, GetPropertyValue(*instance));

#ifdef NOT_NOW
    ParsePropertyJson("\"ABCDE\"");
        {
        DISABLE_ASSERTS
        EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
        EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
        }
#endif

    ParsePropertyJson("\"1.111111\"");
        {
        DISABLE_ASSERTS
        EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
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
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsonCpp, classLocater)); // expect error when not initialized
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
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsonCpp, classLocater)); // expect error when not initialized
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater)); // expect error when not initialized

    // Test for expected JSON parse errors
    ParseJsonString(nullptr, ERROR);
    ParseJsonString("", ERROR);
    ParseJsonString("undefined", ERROR);

    //-------------------------------------------------------------------------
    // JSON --> struct instance tests
    //-------------------------------------------------------------------------

    ParseJsonString("null");
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsonCpp, classLocater));
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));

    ParseJsonString("{}");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsonCpp, classLocater));
    AssertNullValue(*structInstance, "doubleProperty");
    AssertNullValue(*structInstance, "stringProperty");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));
    AssertNullValue(*structInstance, "doubleProperty");
    AssertNullValue(*structInstance, "stringProperty");

    ParseJsonString(u8R"*({ "doubleProperty": 1.1, "stringProperty": "S1" })*");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_jsonCpp, classLocater));
    AssertDoubleValue(*structInstance, "doubleProperty", 1.1);
    AssertStringValue(*structInstance, "stringProperty", "S1");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*structInstance, m_rapidJson, classLocater));
    AssertDoubleValue(*structInstance, "doubleProperty", 1.1);
    AssertStringValue(*structInstance, "stringProperty", "S1");

    //-------------------------------------------------------------------------
    // JSON --> entity instance with struct property tests
    //-------------------------------------------------------------------------

    ParseJsonString("null");
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsonCpp, classLocater));
    EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_rapidJson, classLocater));

    ParseJsonString("{}");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsonCpp, classLocater));
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

    ParseJsonString(u8R"*({ "doubleProperty": null, "stringProperty": null })*");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsonCpp, classLocater));
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

    ParseJsonString(u8R"*({ "doubleProperty": 1.1, "stringProperty": "S1" })*");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsonCpp, classLocater));
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

    ParseJsonString(u8R"*({ "doubleProperty": 1.1, "stringProperty": "S1", "testStruct": null })*");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsonCpp, classLocater));
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

    ParseJsonString(u8R"*({ "doubleProperty": 1.1, "stringProperty": "S1", "testStruct": { "doubleProperty": 2.2, "stringProperty": "S2" } })*");
    EXPECT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*testInstance, m_jsonCpp, classLocater));
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

END_BENTLEY_ECN_TEST_NAMESPACE