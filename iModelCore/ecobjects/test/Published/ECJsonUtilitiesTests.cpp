/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECJsonUtilitiesTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

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

    jsonStr = "-10";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(uint64_t(-10)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(uint64_t(-10)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = "3.14";
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

    jsonStr = R"json("-10")json";
    ASSERT_EQ(SUCCESS, ParseJson(jsonCpp, jsonStr));
    EXPECT_EQ(BeInt64Id(uint64_t(-10)), ECJsonUtilities::JsonToId<BeInt64Id>(jsonCpp)) << jsonStr;

    ASSERT_EQ(SUCCESS, ParseJson(rapidJson, jsonStr));
    EXPECT_EQ(BeInt64Id(uint64_t(-10)), ECJsonUtilities::JsonToId<BeInt64Id>(rapidJson)) << jsonStr;

    jsonStr = R"json("3.14")json";
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

END_BENTLEY_ECN_TEST_NAMESPACE