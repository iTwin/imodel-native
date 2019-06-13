/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <Bentley/BeId.h>
#include <ostream>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECJsonUtilitiesTestFixture : ECTestFixture 
    {

protected:
    static BentleyStatus ParseJson(Json::Value& json, Utf8StringCR jsonStr) { return Json::Reader::Parse(jsonStr, json) ? SUCCESS : ERROR; }
    static BentleyStatus ParseJson(rapidjson::Document& json, Utf8StringCR jsonStr) { return json.Parse<0>(jsonStr.c_str()).HasParseError() ? ERROR : SUCCESS; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeInt64Id id, std::ostream* os) { *os << id.GetValueUnchecked(); }

//----------------------------------------------------------------------------------------
// @bsimethod                                       Krischan.Eberle             11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToId)
    {
    Json::Value jsonCpp;
    rapidjson::Document rapidJson;

    //Ids formatted numerically in JSON
    Utf8CP jsonStr = "1234";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(1234)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = "12345678901231231231";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(12345678901231231231)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(UINT64_C(12345678901231231231)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

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
// @bsimethod                                       Krischan.Eberle             11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, IdToJson)
    {
    Json::Value jsonCpp;
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
// @bsimethod                                       Krischan.Eberle             11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToInt64)
    {
    Json::Value jsonCpp;
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

    jsonStr = "12345678901231231231";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, jsonCpp)) << jsonStr;
    EXPECT_EQ(INT64_C(12345678901231231231), val) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToInt64(val, rapidJson)) << jsonStr;
    EXPECT_EQ(INT64_C(12345678901231231231), val) << jsonStr;

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
// @bsimethod                                       Krischan.Eberle             11/2017
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
// @bsimethod                                       Krischan.Eberle             11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToDateTime)
    {
    Json::Value jsonCpp;
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
// @bsimethod                                       Krischan.Eberle             11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, DateTimeToJson)
    {
    Json::Value jsonCpp;
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
// @bsimethod                                       Krischan.Eberle             11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, JsonToBinary)
    {
    Json::Value jsonCpp;
    rapidjson::Document rapidJson;

    Utf8String expectedBase64Str;
    bvector<Byte> expectedByteVector;
    ByteStream expectedByteStream;
    {
    const int64_t expectedNumber = INT64_C(1234567890);
    Byte const* expectedBlob = (Byte const*) (&expectedNumber);
    const size_t expectedBlobSize = sizeof(expectedNumber);

    Base64Utilities::Encode(expectedBase64Str, expectedBlob, expectedBlobSize);

    Base64Utilities::Decode(expectedByteVector, expectedBase64Str);
    Base64Utilities::Decode(expectedByteStream, expectedBase64Str);
    }

    bvector<Byte> actualByteVector;
    ByteStream actualByteStream;

    Utf8String jsonStr;
    jsonStr.append("\"").append(expectedBase64Str).append("\"");

    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToBinary(actualByteVector, jsonCpp)) << jsonStr;
    EXPECT_EQ(0, memcmp(expectedByteVector.data(), actualByteVector.data(), expectedByteVector.size())) << jsonStr;
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToBinary(actualByteStream, jsonCpp)) << jsonStr;
    EXPECT_EQ(0, memcmp(expectedByteStream.GetData(), actualByteStream.GetData(), expectedByteStream.GetSize())) << jsonStr;

    actualByteVector.clear();
    actualByteStream.Clear();
    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToBinary(actualByteVector, rapidJson)) << jsonStr;
    EXPECT_EQ(0, memcmp(expectedByteVector.data(), actualByteVector.data(), expectedByteVector.size())) << jsonStr;
    EXPECT_EQ(SUCCESS, ECJsonUtilities::JsonToBinary(actualByteStream, rapidJson)) << jsonStr;
    EXPECT_EQ(0, memcmp(expectedByteStream.GetData(), actualByteStream.GetData(), expectedByteStream.GetSize())) << jsonStr;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                       Krischan.Eberle             11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECJsonUtilitiesTestFixture, BinaryToJson)
    {
    Json::Value jsonCpp;
    rapidjson::Document rapidJson;

    Utf8String expectedBase64Str;
    bvector<Byte> byteVector;
    ByteStream byteStream;
    {
    const int64_t expectedNumber = INT64_C(1234567890);
    Byte const* expectedBlob = (Byte const*) (&expectedNumber);
    const size_t expectedBlobSize = sizeof(expectedNumber);

    Base64Utilities::Encode(expectedBase64Str, expectedBlob, expectedBlobSize);

    Base64Utilities::Decode(byteVector, expectedBase64Str);
    Base64Utilities::Decode(byteStream, expectedBase64Str);
    }

    EXPECT_EQ(SUCCESS, ECJsonUtilities::BinaryToJson(jsonCpp, byteVector.data(), byteVector.size())) << expectedBase64Str;
    EXPECT_STRCASEEQ(expectedBase64Str.c_str(), jsonCpp.asCString()) << expectedBase64Str;
    EXPECT_EQ(SUCCESS, ECJsonUtilities::BinaryToJson(jsonCpp, byteStream.GetData(), byteStream.GetSize())) << expectedBase64Str;
    EXPECT_STRCASEEQ(expectedBase64Str.c_str(), jsonCpp.asCString()) << expectedBase64Str;

    EXPECT_EQ(SUCCESS, ECJsonUtilities::BinaryToJson(rapidJson, byteVector.data(), byteVector.size(), rapidJson.GetAllocator())) << expectedBase64Str;
    EXPECT_STRCASEEQ(expectedBase64Str.c_str(), rapidJson.GetString()) << expectedBase64Str;
    EXPECT_EQ(SUCCESS, ECJsonUtilities::BinaryToJson(rapidJson, byteStream.GetData(), byteStream.GetSize(), rapidJson.GetAllocator())) << expectedBase64Str;
    EXPECT_STRCASEEQ(expectedBase64Str.c_str(), rapidJson.GetString()) << expectedBase64Str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                02/2018
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
// @bsimethod                                    Victor.Cushman                02/2018
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

//=======================================================================================
// @bsistruct                                                   Bill.Goehrig     10/2018
//=======================================================================================
struct JsonECInstanceConverterTestFixture : ECJsonUtilitiesTestFixture
    {
protected:
    Json::Value m_jsonCpp;
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
// @bsimethod                                                   Bill.Goehrig     10/2018
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
        // FIXME: This should not be supported!
        // EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_jsonCpp, classLocater));
        EXPECT_NE(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, m_rapidJson, classLocater));
        }
    }

END_BENTLEY_ECN_TEST_NAMESPACE